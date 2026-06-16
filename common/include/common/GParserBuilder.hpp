/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here
// nvcc (CUDA host compiler) does not suppress warnings from system/third-party
// headers the way GCC does.  The three diagnostics below are Boost-internal
// false positives that are irrelevant to Geneva code:
//   #68-D  – integer conversion sign change    (boost/mpl/print.hpp)
//   #186-D – unsigned comparison with zero     (boost/mp11, via ptree/multi_index)
//   #191-D – meaningless cast qualifier        (boost/archive/detail/iserializer.hpp)
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GDefaultValueT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem::Common {

// Forward declaration
class GParserBuilder;

/******************************************************************************/
// Indicates whether help was requested using the -h or --help switch on the command line
constexpr bool GCL_HELP_REQUESTED = true;
constexpr bool GCL_NO_HELP_REQUESTED = false;

// Indicates whether implicit values are allowed (such as in --server vs. --server=true)
constexpr bool GCL_IMPLICIT_ALLOWED = true;
constexpr bool GCL_IMPLICIT_NOT_ALLOWED = false;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to store values for a single entity from different sources, such
 * as command line, configuration files or environment variables. The enum class
 * parameter_source holds the available parameter sources. These sources are
 * grouped in the order "command line", "environment variable", "configuration
 * file" and "network".
 */
template <typename parameter_type>
class GMultiSourceParameterT {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(default_value_) & BOOST_SERIALIZATION_NVP(parameter_values_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * Construction with a default value
	  */
    explicit GMultiSourceParameterT(parameter_type default_value)
      : default_value_(default_value) { /* nothing */
    }

    /***************************************************************************/
    // Defaulted constructors. destructors and assignment operators

    GMultiSourceParameterT(GMultiSourceParameterT<parameter_type> const &cp) = default;
    GMultiSourceParameterT(GMultiSourceParameterT<parameter_type> &&cp) noexcept = default;
    GMultiSourceParameterT<parameter_type> &
    operator=(GMultiSourceParameterT<parameter_type> const &cp) = default;
    GMultiSourceParameterT<parameter_type> &
    operator=(GMultiSourceParameterT<parameter_type> &&cp) noexcept = default;
    ~GMultiSourceParameterT() = default;

    /***************************************************************************/
    /**
	  * Allows to set the value associated with a given data source
	  */
    void set(Gem::Common::parameter_source data_source, parameter_type parameter_value) {
        parameter_values_.at(data_source) = parameter_value;
    }

    /***************************************************************************/
    /**
	  * Allows to check whether the value for a given data source was set
	  */
    bool isSet(Gem::Common::parameter_source data_source) {
        return parameter_values_.at(data_source).second;
    }

    /***************************************************************************/
    /**
	  * Retrieves the first stored value that has been set, in the order of
	  * appearance in parameter_values_, or alternatively the default value,
	  * if the value was not set from any source.
	  */
    parameter_type value() const {
        for(auto const &v_pair : parameter_values_) {
            if(v_pair.second) { // Value was set
                return *v_pair.second;
            }

            // Not set -- continue loop
        }

        // Nothing found
        return default_value_;
    }

    /***************************************************************************/
    /**
	  * Returns the value stored for a given data source. The function will throw
	  * when called for a parameter source not listed in parameter_values_.
	  */
    parameter_type value(Gem::Common::parameter_source data_source) {
        return parameter_values_.at(data_source);
    }

    /***************************************************************************/
    /**
	  * Automatic conversion for constant callers
	  */
    operator parameter_type() const { // NOLINT
        return value();
    }

private:
    /***************************************************************************/
    /**
    * Default constructor -- Only needed for (de-)serialization purposes
    */
    GMultiSourceParameterT() = default;

    /***************************************************************************/
    // Data

    parameter_type default_value_ =
        parameter_type(nullptr); // The default value to be returned when all else fails

    // Value retrieval will look at each entry of the map until it finds one that was set.
    // If none was set, the default value will be returned
    std::map<Gem::Common::parameter_source, std::optional<parameter_type>> parameter_values_{
        {Gem::Common::parameter_source::NETWORK, std::optional<parameter_type>()},
        {Gem::Common::parameter_source::COMMAND_LINE, std::optional<parameter_type>()},
        {Gem::Common::parameter_source::ENVIRONMENT_VARIABLE, std::optional<parameter_type>()},
        {Gem::Common::parameter_source::CONFIGURATION_FILE, std::optional<parameter_type>()},
        {Gem::Common::parameter_source::ASSIGNMENT, std::optional<parameter_type>()}
    };
};
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A manipulator object that allows to identify the id of the comment to be
 * added
 */
class commentLevel { // NOLINT(cppcoreguidelines-special-member-functions)
public:
    /** @brief Enforce setting of the comment level */
    explicit commentLevel(std::size_t);

    /*************************************************************************/
    // Defaulted or deleted functions functions

    commentLevel() = delete;

    commentLevel(commentLevel const &) = default;
    commentLevel(commentLevel &&) = delete; // enforce explicit settinf of comment level

    commentLevel &operator=(commentLevel const &) = default;
    commentLevel &operator=(commentLevel &&) = delete; // enforce explicit settinf of comment level

    /*************************************************************************/

    /** @brief Retrieves the current commentLevel */
    std::size_t getCommentLevel() const;

private:
    std::size_t comment_level_; ///< The id of the comment inside of GParsableI
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A manipulator object that increments the comment level
 */
class nextComment {
public:
    /** @brief The default constructor */
    nextComment() = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable parameters, to
 * which a call-back function has been assigned. It also stores some
 * information common to all parameter types.
 */
class GParsableI {
public:
    /** @brief A constructor for individual items */
    GParsableI(std::string const &, std::string const &);

    /** @brief A constructor for vectors */
    GParsableI(std::vector<std::string> const &, std::vector<std::string> const &);

    /** @brief The destructor */
    virtual ~GParsableI() = default;

    // Prevent copying, moving and default construction
    GParsableI() = delete;
    GParsableI(GParsableI const &) = delete;
    GParsableI(GParsableI &&) = delete;
    GParsableI &operator=(GParsableI const &) = delete;
    GParsableI &operator=(GParsableI &&) = delete;

    /** @brief Retrieves the option name at a given position */
    std::string optionName(std::size_t = 0) const;
    /** @brief Retrieves the comment that was assigned to this variable at a given position */
    std::string comment(std::size_t = 0) const;
    /** @brief Checks whether comments have indeed been registered */
    bool hasComments() const;
    /** @brief Retrieves the number of comments available */
    std::size_t numberOfComments() const;
    /** @brief Retrieves the number of option names registered for this parameter */
    std::size_t numberOfOptionNames() const;

    /***************************************************************************/
    /**
	  * Create a std::vector<T> from a single element
	  */
    template <typename T>
    static std::vector<T> makeVector(T const &item) {
        std::vector<T> result;
        result.push_back(item);
        return result;
    }

    /***************************************************************************/
    /**
	  * Create a std::vector<T> from two elements
	  */
    template <typename T>
    static std::vector<T> makeVector(T const &item1, T const &item2) {
        std::vector<T> result;
        result.push_back(item1);
        result.push_back(item2);
        return result;
    }

    /***************************************************************************/
    /**
	  * This function will forward all arguments to a newly created ostringstream
	  * and will then be added to the current comment_ entry.
	  */
    template <typename T>
    GParsableI &operator<<(T const &t) {
        std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
        oss << t;
        comment_.at(cl_) += oss.str();
        return *this;
    }

    /***************************************************************************/
    /** @brief Needed for std::ostringstream */
    GParsableI &operator<<(std::ostream &(*val)(std::ostream &));
    /** @brief Needed for std::ostringstream */
    GParsableI &operator<<(std::ios &(*val)(std::ios &));
    /** @brief Needed for std::ostringstream */
    GParsableI &operator<<(std::ios_base &(*val)(std::ios_base &));
    /** @brief Allows to indicate the current comment level */
    GParsableI &operator<<(commentLevel const &);
    /** @brief Allows to switch to the next comment level */
    GParsableI &operator<<(nextComment const &);

protected:
    /***************************************************************************/
    /** @brief Splits a comment into sub-tokens */
    std::vector<std::string> splitComment(std::string const &) const;

private:
    /***************************************************************************/
    std::vector<std::string> option_name_; ///< The name of this parameter
    std::vector<std::string> comment_;     ///< A comment assigned to this parameter

    std::size_t cl_; ///< The id of the current comment inside of the comment_ vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable file parameters, to
 * which a call-back function has been assigned. Note that this class cannot
 * be copied; copy and move are explicitly deleted.
 */
class GFileParsableI : public GParsableI {
    // We want GParserBuilder to be able to call our private load- and save functions
    friend class GParserBuilder;

public:
    /** @brief A constructor for individual items */
    GFileParsableI(std::string const &, std::string const &, bool);
    /** @brief A constructor for vectors */
    
    GFileParsableI(std::vector<std::string> const &, std::vector<std::string> const &, bool);

    /** @brief The destructor */
    ~GFileParsableI() override = default;

    // Prevent copying, moving and default construction
    GFileParsableI() = delete;
    GFileParsableI(GFileParsableI const &) = delete;
    GFileParsableI(GFileParsableI &&) = delete;
    GFileParsableI &operator=(GFileParsableI const &) = delete;
    GFileParsableI &operator=(GFileParsableI &&) = delete;

    /** @brief Checks whether this is an essential variable at a given position */
    bool isEssential() const;

    /** @brief Executes a stored callbacl function */
    void executeCallBackFunction();

    /** @brief Returns the top-level configuration-file (JSON) key this parameter
     *  occupies. By default this is the first option name -- single, vector and
     *  array parameters write their data directly under it. Combined parameters
     *  override this to return their JSON group label, under which their
     *  sub-options nest. Used by the unknown-key diagnostic to recognise valid
     *  top-level keys. */
    virtual std::string topLevelConfigKey() const {
        return GParsableI::optionName(0);
    }

private:
    /***************************************************************************/
    /** @brief Loads data from a property_tree object */
    virtual void load_from(boost::property_tree::ptree const &) = 0;

    /** @brief Saves data to a property tree object */
    virtual void save_to(boost::property_tree::ptree &) const = 0;

    /** @brief Executes a stored call-back function */
    virtual void executeCallBackFunction_() = 0;

    /***************************************************************************/

    bool is_essential_; ///< Indicates whether this is an essential variable
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for single parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template <typename parameter_type>
class GSingleParmT : public GFileParsableI {
    // We want GParserBuilder to be able to call the reset function
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class
	  */
    GSingleParmT(
        const std::string &option_name_var,
        const std::string &comment_var,
        const bool &is_essential_var,
        const parameter_type &def_val
    )
      : GFileParsableI(option_name_var, comment_var, is_essential_var)
      , def_val_(def_val)
      , par_(def_val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GSingleParmT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GSingleParmT() = delete;
    GSingleParmT(GSingleParmT<parameter_type> const &) = delete;
    GSingleParmT(GSingleParmT<parameter_type> &&) = delete;
    GSingleParmT<parameter_type> &operator=(GSingleParmT<parameter_type> const &) = delete;
    GSingleParmT<parameter_type> &operator=(GSingleParmT<parameter_type> &&) = delete;

protected:
    /***************************************************************************/
    /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par_, as its value will be overwritten
	  * as well. The reason is that configuration files will otherwise contain
	  * the "old" par_-value.
	  */
    void resetDefault(parameter_type const &def_val) {
        def_val_ = def_val;
        par_ = def_val;
    }

    /***************************************************************************/
    parameter_type def_val_; ///< Holds the parameter's default value
    parameter_type par_;     ///< Holds the individual parameter

private:
    /***************************************************************************/
    /** @brief Loads data from a property_tree object */
    void load_from(boost::property_tree::ptree const &) override = 0;

    /** @brief Saves data to a property tree object */
    void save_to(boost::property_tree::ptree &) const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps individual parsable file parameters, to which a callback
 * function has been assigned.
 */
template <typename parameter_type>
class GFileSingleParsableParameterT : public GSingleParmT<parameter_type> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class
	  */
    GFileSingleParsableParameterT(
        const std::string &option_name_var,
        const std::string &comment_var,
        const bool &is_essential_var,
        const parameter_type &def_val
    )
      : GSingleParmT<parameter_type>(
            option_name_var,
            comment_var,
            is_essential_var,
            def_val
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class, except
	  * for comments.
	  */
    GFileSingleParsableParameterT(const std::string &option_name_var, const parameter_type &def_val)
      : GSingleParmT<parameter_type>(
            option_name_var,
            std::string(),
            Gem::Common::VAR_IS_ESSENTIAL,
            def_val
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileSingleParsableParameterT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GFileSingleParsableParameterT() = delete;
    GFileSingleParsableParameterT(GFileSingleParsableParameterT<parameter_type> const &) = delete;
    GFileSingleParsableParameterT(GFileSingleParsableParameterT<parameter_type> &&) = delete;
    GFileSingleParsableParameterT<parameter_type> &
    operator=(GFileSingleParsableParameterT<parameter_type> const &) = delete;
    GFileSingleParsableParameterT<parameter_type> &
    operator=(GFileSingleParsableParameterT<parameter_type> &&) = delete;

    /***************************************************************************/
    /**
	  * Allows to register a call-back function with this object
	  *
	  * @param call_back The function to be executed
	  */
    void registerCallBackFunction(std::function<void(parameter_type)> call_back) {
        if(not call_back) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSingleParsableParameter::registerCallBackFunction(): Error" << '\n'
                << "Tried to register an empty call-back function" << '\n'
            );
        }

        call_back_func_ = call_back;
    }

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        GSingleParmT<parameter_type>::par_ = pt.get(
            (GParsableI::optionName(0) + ".value").c_str(),
            GSingleParmT<parameter_type>::def_val_
        );
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 1) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileSingleParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 1 comment but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
            if(not comments.empty()) {
                for(auto const &comment : comments) {
                    pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
                }
            }
        }

        pt.put(
            (GParsableI::optionName(0) + ".default").c_str(),
            GSingleParmT<parameter_type>::def_val_
        );
        pt.put((GParsableI::optionName(0) + ".value").c_str(), GSingleParmT<parameter_type>::par_);
    }

    /***************************************************************************/
    /**
	  * Executes a stored call-back function
	  */
    void executeCallBackFunction_() override {
        if(not call_back_func_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSingleParsableParameter::executeCallBackFunction_(): Error" << '\n'
                << "Tried to execute call-back function without a stored function" << '\n'
            );
        }

        // Execute the function
        call_back_func_(GSingleParmT<parameter_type>::par_);
    }

    /***************************************************************************/

    std::function<void(parameter_type)> call_back_func_; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual parameters. Instead of
 * executing a stored call-back function, executeCallBackFunction will assign
 * the parsed value to the reference.
 */
template <typename parameter_type>
class GFileReferenceParsableParameterT : public GSingleParmT<parameter_type> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class
	  */
    GFileReferenceParsableParameterT(
        parameter_type &stored_reference,
        std::string const &option_name_var,
        std::string const &comment_var,
        bool is_essential_var,
        parameter_type const &def_val
    )
      : GSingleParmT<parameter_type>(option_name_var, comment_var, is_essential_var, def_val)
      , stored_reference_(stored_reference) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class, except
	  * for comments.
	  */
    GFileReferenceParsableParameterT(
        parameter_type &stored_reference,
        std::string const &option_name_var,
        parameter_type const &def_val
    )
      : GSingleParmT<parameter_type>(
            option_name_var,
            std::string(),
            Gem::Common::VAR_IS_ESSENTIAL,
            def_val
        )
      , stored_reference_(stored_reference) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileReferenceParsableParameterT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GFileReferenceParsableParameterT() = delete;
    GFileReferenceParsableParameterT(GFileReferenceParsableParameterT<parameter_type> const &) =
        delete;
    GFileReferenceParsableParameterT(GFileReferenceParsableParameterT<parameter_type> &&) = delete;
    GFileReferenceParsableParameterT<parameter_type> &
    operator=(GFileReferenceParsableParameterT<parameter_type> const &) = delete;
    GFileReferenceParsableParameterT<parameter_type> &
    operator=(GFileReferenceParsableParameterT<parameter_type> &&) = delete;

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        GSingleParmT<parameter_type>::par_ = pt.get(
            (GParsableI::optionName(0) + ".value").c_str(),
            GSingleParmT<parameter_type>::def_val_
        );
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 1) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileReferenceParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 1 comment but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
            if(not comments.empty()) {
                for(auto const &comment : comments) {
                    pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
                }
            }
        }

        pt.put(
            (GParsableI::optionName(0) + ".default").c_str(),
            GSingleParmT<parameter_type>::def_val_
        );
        pt.put((GParsableI::optionName(0) + ".value").c_str(), GSingleParmT<parameter_type>::par_);
    }

    /***************************************************************************/
    /**
	  * Assigns the stored parameter to the reference
	  */
    void executeCallBackFunction_() override {
        stored_reference_ = GSingleParmT<parameter_type>::par_;
    }

    /***************************************************************************/

    parameter_type
        &stored_reference_; ///< Holds the reference to which the parsed value will be assigned
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for combined parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template <typename par_type0, typename par_type1>
class GCombinedParT : public GFileParsableI {
    // We want GParserBuilder to be able to call the reset function
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class
	  */
    GCombinedParT(
        std::string const &option_name_var0,
        std::string const &comment_var0,
        par_type0 const &def_val0,
        std::string const &option_name_var1,
        std::string const &comment_var1,
        par_type1 const &def_val1,
        bool const &is_essential_var,
        std::string combined_label
    )
      : GFileParsableI(
            GFileParsableI::makeVector(option_name_var0, option_name_var1),
            GFileParsableI::makeVector(comment_var0, comment_var1),
            is_essential_var
        )
      , par0_(def_val0)
      , def_val0_(def_val0)
      , par1_(def_val1)
      , def_val1_(def_val1)
      , combined_label_(std::move(combined_label)) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GCombinedParT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GCombinedParT() = delete;
    GCombinedParT(GCombinedParT<par_type0, par_type1> const &) = delete;
    GCombinedParT(GCombinedParT<par_type0, par_type1> &&) = delete;
    GCombinedParT<par_type0, par_type1> &
    operator=(GCombinedParT<par_type0, par_type1> const &) = delete;
    GCombinedParT<par_type0, par_type1> &operator=(GCombinedParT<par_type0, par_type1> &&) = delete;

    /***************************************************************************/
    /** @brief The combined parameter nests its sub-options under a single JSON
     *  group label, so that label -- not the individual sub-option names -- is the
     *  top-level configuration-file key. */
    std::string topLevelConfigKey() const override {
        return combined_label_;
    }

protected:
    /***************************************************************************/
    /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par1_ and par_2, as their values will
	  * be overwritten as well. The reason is that configuration files will otherwise
	  * contain the "old" par_-value.
	  */
    void resetDefault(par_type0 const &def_val0, par_type1 const &def_val1) {
        def_val0_ = def_val0;
        def_val1_ = def_val1;
        par0_ = def_val0;
        par1_ = def_val1;
    }

    /***************************************************************************/
    par_type0 par0_, def_val0_; ///< Holds the individual parameters and default values 0
    par_type1 par1_, def_val1_; ///< Holds the individual parameters and default values 1

    std::string combined_label_; ///< Holds a path label for the combined JSON path

private:
    /***************************************************************************/
    /** @brief Loads data from a property_tree object */
    void load_from(boost::property_tree::ptree const &) override = 0;

    /** @brief Saves data to a property tree object */
    void save_to(boost::property_tree::ptree &) const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps combined parsable file parameters, to which a callback
 * function has been assigned.
 */
template <typename par_type0, typename par_type1>
class GFileCombinedParsableParameterT : public GCombinedParT<par_type0, par_type1> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameters
	  */
    GFileCombinedParsableParameterT(
        std::string const &option_name_var0,
        std::string const &comment_var0,
        par_type0 const &def_val0,
        std::string const &option_name_var1,
        std::string const &comment_var1,
        par_type1 const &def_val1,
        bool is_essential_var,
        std::string const &combined_label
    )
      : GCombinedParT<par_type0, par_type1>(
            option_name_var0,
            comment_var0,
            def_val0,
            option_name_var1,
            comment_var1,
            def_val1,
            is_essential_var,
            combined_label
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameters
	  */
    GFileCombinedParsableParameterT(
        std::string const &option_name_var0,
        par_type0 const &def_val0,
        std::string const &option_name_var1,
        par_type1 const &def_val1,
        std::string const &combined_label
    )
      : GCombinedParT<par_type0, par_type1>(
            option_name_var0,
            std::string(),
            def_val0,
            option_name_var1,
            std::string(),
            def_val1,
            Gem::Common::VAR_IS_ESSENTIAL,
            combined_label
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileCombinedParsableParameterT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GFileCombinedParsableParameterT() = delete;
    GFileCombinedParsableParameterT(GFileCombinedParsableParameterT<par_type0, par_type1> const &) =
        delete;
    GFileCombinedParsableParameterT(GFileCombinedParsableParameterT<par_type0, par_type1> &&) =
        delete;
    GFileCombinedParsableParameterT<par_type0, par_type1> &
    operator=(GFileCombinedParsableParameterT<par_type0, par_type1> const &) = delete;
    GFileCombinedParsableParameterT<par_type0, par_type1> &
    operator=(GFileCombinedParsableParameterT<par_type0, par_type1> &&) = delete;

    /***************************************************************************/
    /**
	  * Allows to register a call-back function with this object
	  *
	  * @param call_back The function to be executed
	  */
    void registerCallBackFunction(std::function<void(par_type0, par_type1)> call_back) {
        if(not call_back) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileCombinedParsableParameterT::registerCallBackFunction(): Error"
                << '\n'
                << "Tried to register an empty call-back function" << '\n'
            );
        }

        call_back_func_ = call_back;
    }

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        GCombinedParT<par_type0, par_type1>::par0_ = pt.get(
            (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
             GParsableI::optionName(0) + ".value")
                .c_str(),
            GCombinedParT<par_type0, par_type1>::def_val0_
        );
        GCombinedParT<par_type0, par_type1>::par1_ = pt.get(
            (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
             GParsableI::optionName(1) + ".value")
                .c_str(),
            GCombinedParT<par_type0, par_type1>::def_val1_
        );
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 2) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileCombinedParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 2 comments but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments0 = GParsableI::splitComment(this->comment(0));
            if(not comments0.empty()) {
                for(auto const &comment : comments0) {
                    pt.add(
                        (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
                         GParsableI::optionName(0) + ".comment")
                            .c_str(),
                        comment.c_str()
                    );
                }
            }
        }
        pt.put(
            (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
             GParsableI::optionName(0) + ".default")
                .c_str(),
            GCombinedParT<par_type0, par_type1>::def_val0_
        );
        pt.put(
            (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
             GParsableI::optionName(0) + ".value")
                .c_str(),
            GCombinedParT<par_type0, par_type1>::par0_
        );

        if(this->hasComments()) {
            std::vector<std::string> comments1 = GParsableI::splitComment(this->comment(1));
            if(not comments1.empty()) {
                for(auto const &comment : comments1) {
                    pt.add(
                        (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
                         GParsableI::optionName(1) + ".comment")
                            .c_str(),
                        comment.c_str()
                    );
                }
            }
        }
        pt.put(
            (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
             GParsableI::optionName(1) + ".default")
                .c_str(),
            GCombinedParT<par_type0, par_type1>::def_val1_
        );
        pt.put(
            (GCombinedParT<par_type0, par_type1>::combined_label_ + "." +
             GParsableI::optionName(1) + ".value")
                .c_str(),
            GCombinedParT<par_type0, par_type1>::par1_
        );
    }

    /***************************************************************************/
    /**
	  * Executes a stored call-back function
	  */
    void executeCallBackFunction_() override {
        if(not call_back_func_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileCombinedParsableParameterT::executeCallBackFunction_(): Error"
                << '\n'
                << "Tried to execute call-back function without a stored function" << '\n'
            );
        }

        // Execute the function
        call_back_func_(
            GCombinedParT<par_type0, par_type1>::par0_,
            GCombinedParT<par_type0, par_type1>::par1_
        );
    }

    /***************************************************************************/

    std::function<void(par_type0, par_type1)> call_back_func_; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for vector parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template <typename parameter_type>
class GVectorParT : public GFileParsableI {
    // We want GParserBuilder to be able to call the reset function
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class
	  */
    GVectorParT(
        std::string const &option_name_var,
        std::string const &comment_var,
        std::vector<parameter_type> const &def_val,
        bool is_essential_var
    )
      : GFileParsableI(option_name_var, comment_var, is_essential_var)
      , def_val_cnt_(def_val)
      // Seed par_cnt_ with the defaults so writeConfigFile() can emit a "value"
      // for every "default" entry before any parsing has populated par_cnt_.
      // The previous version left par_cnt_ empty, and save_to() then iterated
      // def_val_cnt_ while dereferencing par_cnt_.cbegin() — UB whenever no
      // load_from() had run yet, observable as silent garbage values in the
      // generated config or a segfault depending on allocator layout.
      , par_cnt_(def_val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GVectorParT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GVectorParT() = delete;
    GVectorParT(GVectorParT<parameter_type> const &) = delete;
    GVectorParT(GVectorParT<parameter_type> &&) = delete;
    GVectorParT<parameter_type> &operator=(GVectorParT<parameter_type> const &) = delete;
    GVectorParT<parameter_type> &operator=(GVectorParT<parameter_type> &&) = delete;

protected:
    /***************************************************************************/
    /**
	  * Allows derived classes to reset the default value. Keeps par_cnt_ in
	  * lock-step with the new defaults so a subsequent writeConfigFile()
	  * before parsing still emits one "value" per "default" entry.
	  */
    void resetDefault(std::vector<parameter_type> const &def_val) {
        def_val_cnt_ = def_val;
        par_cnt_     = def_val;
    }

    /***************************************************************************/
    std::vector<parameter_type> def_val_cnt_; ///< Holds default values
    std::vector<parameter_type> par_cnt_;     ///< Holds the parsed parameters

private:
    /***************************************************************************/
    /** @brief Loads data from a property_tree object */
    void load_from(boost::property_tree::ptree const &) override = 0;

    /** @brief Saves data to a property tree object */
    void save_to(boost::property_tree::ptree &) const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a std::vector of values (obviously of identical type).
 * Note that this class does not enforce a given amount of parameters. However,
 * there needs to be at least one default value in the def_val vector, if
 * you plan to write out a parameter file.
 */
template <typename parameter_type>
class GFileVectorParsableParameterT : public GVectorParT<parameter_type> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameters
	  */
    GFileVectorParsableParameterT(
        std::string const &option_name_var,
        std::string const &comment_var,
        std::vector<parameter_type> const &def_val,
        bool is_essential_var
    )
      : GVectorParT<parameter_type>(
            option_name_var,
            comment_var,
            def_val,
            is_essential_var
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameters, except for comments
	  */
    GFileVectorParsableParameterT(
        std::string const &option_name_var,
        std::vector<parameter_type> const &def_val
    )
      : GVectorParT<parameter_type>(
            option_name_var,
            std::string(),
            def_val,
            Gem::Common::VAR_IS_ESSENTIAL
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileVectorParsableParameterT() override = default;

    // Prevent copying, moving and default construction
    GFileVectorParsableParameterT() = delete;
    GFileVectorParsableParameterT(GFileVectorParsableParameterT<parameter_type> const &) = delete;
    GFileVectorParsableParameterT(GFileVectorParsableParameterT<parameter_type> &&) = delete;
    GFileVectorParsableParameterT<parameter_type> &
    operator=(GFileVectorParsableParameterT<parameter_type> const &) = delete;
    GFileVectorParsableParameterT<parameter_type> &
    operator=(GFileVectorParsableParameterT<parameter_type> &&) = delete;

    /***************************************************************************/
    /**
	  * Allows to register a call-back function with this object
	  *
	  * @param call_back The function to be executed
	  */
    void registerCallBackFunction(std::function<void(std::vector<parameter_type>)> call_back) {
        if(not call_back) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileVectorParsableParameterT::registerCallBackFunction(): Error"
                << '\n'
                << "Tried to register an empty call-back function" << '\n'
            );
        }

        call_back_func_ = call_back;
    }

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        using namespace boost::property_tree;

        // Make sure the recipient vector is empty
        GVectorParT<parameter_type>::par_cnt_.clear();

        std::string ppath = GParsableI::optionName(0) + ".value";
        for(auto const &v : pt.get_child(ppath.c_str())) {
            GVectorParT<parameter_type>::par_cnt_.push_back(
                Gem::Common::from_string<parameter_type>(v.second.data())
            );
        }
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector. Note that there needs
	  * to be at least a single default value in it.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 1) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileVectorParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 1 comment but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
            if(not comments.empty()) {
                for(auto const &comment : comments) {
                    pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
                }
            }
        }

        // Do some error checking
        if(GVectorParT<parameter_type>::def_val_cnt_.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GVectorParsableParameter::save_to(): Error!" << '\n'
                << "You need to provide at least one default value" << '\n'
            );
        }

        // Add the value and default items
        auto par_it = GVectorParT<parameter_type>::par_cnt_.cbegin();
        for(auto const &def_val : GVectorParT<parameter_type>::def_val_cnt_) {
            pt.add((GParsableI::optionName(0) + ".default.item").c_str(), def_val);
            pt.add((GParsableI::optionName(0) + ".value.item").c_str(), *par_it);

            par_it++;
        }
    }

    /***************************************************************************/
    /**
	  * Executes a stored call-back function
	  */
    void executeCallBackFunction_() override {
        if(not call_back_func_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileVectorParsableParameterT::executeCallBackFunction_(): Error"
                << '\n'
                << "Tried to execute call-back function without a stored function" << '\n'
            );
        }

        // Execute the function
        call_back_func_(GVectorParT<parameter_type>::par_cnt_);
    }

    /***************************************************************************/

    std::function<void(std::vector<parameter_type>)>
        call_back_func_; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference std::vector of values (obviously of identical type).
 * Note that this class does not enforce a given amount of parameters. However,
 * there needs to be at least one default value in the def_val vector, if
 * you plan to write out a parameter file.
 */
template <typename parameter_type>
class GFileVectorReferenceParsableParameterT : public GVectorParT<parameter_type> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameters
	  */
    GFileVectorReferenceParsableParameterT(
        std::vector<parameter_type> &stored_reference,
        std::string const &option_name_var,
        std::string const &comment_var,
        std::vector<parameter_type> const &def_val,
        bool is_essential_var
    )
      : GVectorParT<parameter_type>(option_name_var, comment_var, def_val, is_essential_var)
      , stored_reference_(stored_reference) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameters, except for comments
	  */
    GFileVectorReferenceParsableParameterT(
        std::vector<parameter_type> &stored_reference,
        std::string const &option_name_var,
        std::vector<parameter_type> const &def_val
    )
      : GVectorParT<parameter_type>(
            option_name_var,
            std::string(),
            def_val,
            Gem::Common::VAR_IS_ESSENTIAL
        )
      , stored_reference_(stored_reference) { /* nothing */
    }

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GFileVectorReferenceParsableParameterT() = delete;
    GFileVectorReferenceParsableParameterT(
        GFileVectorReferenceParsableParameterT<parameter_type> const &
    ) = delete;
    GFileVectorReferenceParsableParameterT(
        GFileVectorReferenceParsableParameterT<parameter_type> &&
    ) = delete;
    GFileVectorReferenceParsableParameterT<parameter_type> &
    operator=(GFileVectorReferenceParsableParameterT<parameter_type> const &) = delete;
    GFileVectorReferenceParsableParameterT<parameter_type> &
    operator=(GFileVectorReferenceParsableParameterT<parameter_type> &&) = delete;

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileVectorReferenceParsableParameterT() override = default;

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        using namespace boost::property_tree;

        // Make sure the recipient vector is empty
        GVectorParT<parameter_type>::par_cnt_.clear();

        std::string ppath = GParsableI::optionName(0) + ".value";
        for(auto const &v : pt.get_child(ppath.c_str())) {
            GVectorParT<parameter_type>::par_cnt_.push_back(
                Gem::Common::from_string<parameter_type>(v.second.data())
            );
        }
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector. Note that there needs
	  * to be at least a single default value in it.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 1) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileVectorReferenceParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 1 comment but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
            if(not comments.empty()) {
                for(auto const &comment : comments) {
                    pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
                }
            }
        }

        // Do some error checking
        if(GVectorParT<parameter_type>::def_val_cnt_.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileVectorReferenceParsableParameterT::save_to(): Error!" << '\n'
                << "You need to provide at least one default value" << '\n'
            );
        }

        // Add the value and default items
        auto par_it = GVectorParT<parameter_type>::par_cnt_.cbegin();
        for(auto const &def_val : GVectorParT<parameter_type>::def_val_cnt_) {
            pt.add((GParsableI::optionName(0) + ".default.item").c_str(), def_val);
            pt.add((GParsableI::optionName(0) + ".value.item").c_str(), *par_it);

            par_it++;
        }
    }

    /***************************************************************************/
    /**
	  * Assigns the parsed parameters to the reference vector
	  */
    void executeCallBackFunction_() override {
        stored_reference_ = GVectorParT<parameter_type>::par_cnt_;
    }

    /***************************************************************************/

    std::vector<parameter_type> &stored_reference_; ///< Holds a reference to the target vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for array parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template <typename parameter_type, std::size_t N>
class GArrayParT : public GFileParsableI {
    // We want GParserBuilder to be able to call the reset function
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameter and sets values in the parent class
	  */
    GArrayParT(
        std::string const &option_name_var,
        std::string const &comment_var,
        std::array<parameter_type, N> const &def_val,
        bool is_essential_var
    )
      : GFileParsableI(option_name_var, comment_var, is_essential_var)
      , def_val_arr_(def_val)
      , par_arr_(def_val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GArrayParT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GArrayParT() = delete;
    GArrayParT(GArrayParT<parameter_type, N> const &) = delete;
    GArrayParT(GArrayParT<parameter_type, N> &&) = delete;
    GArrayParT<parameter_type, N> &operator=(GArrayParT<parameter_type, N> const &) = delete;
    GArrayParT<parameter_type, N> &operator=(GArrayParT<parameter_type, N> &&) = delete;

protected:
    /***************************************************************************/
    /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par_, as its value will be overwritten
	  * as well. The reason is that configuration files will otherwise contain
	  * the "old" par_-value.
	  */
    void resetDefault(std::array<parameter_type, N> const &def_val_arr) {
        def_val_arr_ = def_val_arr;
        par_arr_ = def_val_arr;
    }

    /***************************************************************************/
    std::array<parameter_type, N> def_val_arr_; ///< Holds default values
    std::array<parameter_type, N> par_arr_;     ///< Holds the parsed parameters

private:
    /***************************************************************************/
    /** @brief Loads data from a property_tree object */
    void load_from(boost::property_tree::ptree const &) override = 0;

    /** @brief Saves data to a property tree object */
    void save_to(boost::property_tree::ptree &) const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a std::array of values (obviously of identical type).
 * This class enforces a fixed number of items in the array.
 */
template <typename parameter_type, std::size_t N>
class GFileArrayParsableParameterT : public GArrayParT<parameter_type, N> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameters
	  */
    GFileArrayParsableParameterT(
        std::string const &option_name_var,
        std::string const &comment_var,
        std::array<parameter_type, N> const &def_val,
        bool is_essential_var
    )
      : GArrayParT<parameter_type, N>(
            option_name_var,
            comment_var,
            def_val,
            is_essential_var
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameters, except for comments
	  */
    GFileArrayParsableParameterT(
        std::string const &option_name_var,
        std::array<parameter_type, N> const &def_val
    )
      : GArrayParT<parameter_type, N>(
            option_name_var,
            std::string(),
            def_val,
            Gem::Common::VAR_IS_ESSENTIAL
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileArrayParsableParameterT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GFileArrayParsableParameterT() = delete;
    GFileArrayParsableParameterT(GFileArrayParsableParameterT<parameter_type, N> const &) = delete;
    GFileArrayParsableParameterT(GFileArrayParsableParameterT<parameter_type, N> &&) = delete;
    GFileArrayParsableParameterT<parameter_type, N> &
    operator=(GFileArrayParsableParameterT<parameter_type, N> const &) = delete;
    GFileArrayParsableParameterT<parameter_type, N> &
    operator=(GFileArrayParsableParameterT<parameter_type, N> &&) = delete;

    /***************************************************************************/
    /**
	  * Allows to register a call-back function with this object
	  *
	  * @param call_back The function to be executed
	  */
    void registerCallBackFunction(std::function<void(std::array<parameter_type, N>)> call_back) {
        if(not call_back) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileArrayParsableParameterT::registerCallBackFunction(): Error" << '\n'
                << "Tried to register an empty call-back function" << '\n'
            );
        }

        call_back_func_ = call_back;
    }

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        using namespace boost::property_tree;

        // We are looping over two arrays here, so a range-based for is unfortunately no option
        for(std::size_t i = 0; i < GArrayParT<parameter_type, N>::par_arr_.size(); i++) {
            GArrayParT<parameter_type, N>::par_arr_.at(i) = pt.get(
                (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
                GArrayParT<parameter_type, N>::def_val_arr_.at(i)
            );
        }
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 1) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileArrayParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 1 comment but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
            if(not comments.empty()) {
                for(auto const &comment : comments) {
                    pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
                }
            }
        }

        // Do some error checking
        if(GArrayParT<parameter_type, N>::def_val_arr_.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileArrayParsableParameterT::save_to(): Error!" << '\n'
                << "You need to provide at least one default value" << '\n'
            );
        }

        // Add the value and default items
        for(std::size_t i = 0; i < GArrayParT<parameter_type, N>::def_val_arr_.size(); i++) {
            pt.add(
                (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".default").c_str(),
                GArrayParT<parameter_type, N>::def_val_arr_.at(i)
            );
            pt.add(
                (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
                GArrayParT<parameter_type, N>::par_arr_.at(i)
            );
        }
    }

    /***************************************************************************/
    /**
	  * Executes a stored call-back function
	  */
    void executeCallBackFunction_() override {
        if(not call_back_func_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileArrayParsableParameterT::executeCallBackFunction_(): Error" << '\n'
                << "Tried to execute call-back function without a stored function" << '\n'
            );
        }

        // Execute the function
        call_back_func_(GArrayParT<parameter_type, N>::par_arr_);
    }

    /***************************************************************************/

    std::function<void(std::array<parameter_type, N>)>
        call_back_func_; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to a std::array of values (obviously of
 * identical type). This class enforces a fixed number of items in the array.
 */
template <typename parameter_type, std::size_t N>
class GFileArrayReferenceParsableParameterT : public GArrayParT<parameter_type, N> {
    // We want GParserBuilder to be able to call our load- and save functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * Initializes the parameters
	  */
    GFileArrayReferenceParsableParameterT(
        std::array<parameter_type, N> &stored_reference,
        std::string const &option_name_var,
        std::string const &comment_var,
        std::array<parameter_type, N> const &def_val,
        bool is_essential_var
    )
      : GArrayParT<parameter_type, N>(option_name_var, comment_var, def_val, is_essential_var)
      , stored_reference_(stored_reference) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the parameters, except for comments
	  */
    GFileArrayReferenceParsableParameterT(
        std::array<parameter_type, N> &stored_reference,
        std::string const &option_name_var,
        std::array<parameter_type, N> const &def_val
    )
      : GArrayParT<parameter_type, N>(
            option_name_var,
            std::string(),
            def_val,
            Gem::Common::VAR_IS_ESSENTIAL
        )
      , stored_reference_(stored_reference) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GFileArrayReferenceParsableParameterT() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GFileArrayReferenceParsableParameterT() = delete;
    GFileArrayReferenceParsableParameterT(
        GFileArrayReferenceParsableParameterT<parameter_type, N> const &
    ) = delete;
    GFileArrayReferenceParsableParameterT(
        GFileArrayReferenceParsableParameterT<parameter_type, N> &&
    ) = delete;
    GFileArrayReferenceParsableParameterT<parameter_type, N> &
    operator=(GFileArrayReferenceParsableParameterT<parameter_type, N> const &) = delete;
    GFileArrayReferenceParsableParameterT<parameter_type, N> &
    operator=(GFileArrayReferenceParsableParameterT<parameter_type, N> &&) = delete;

private:
    /***************************************************************************/
    /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
    void load_from(boost::property_tree::ptree const &pt) override {
        using namespace boost::property_tree;

        for(std::size_t i = 0; i < GArrayParT<parameter_type, N>::par_arr_.size(); i++) {
            GArrayParT<parameter_type, N>::par_arr_.at(i) = pt.get(
                (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
                GArrayParT<parameter_type, N>::def_val_arr_.at(i)
            );
        }
    }

    /***************************************************************************/
    /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector.
	  *
	  * @param pt The object to which data should be saved
	  */
    void save_to(boost::property_tree::ptree &pt) const override {
        // Check that we have the right number of comments
        if(this->hasComments()) {
            if(this->numberOfComments() != 1) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFileArrayReferenceParsableParameterT<>::save_to(): Error!" << '\n'
                    << "Expected 0 or 1 comment but got " << this->numberOfComments() << '\n'
                );
            }

            // Retrieve a list of sub-comments
            std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
            std::vector<std::string>::iterator c;
            if(not comments.empty()) {
                for(c = comments.begin(); c != comments.end(); ++c) {
                    pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
                }
            }
        }

        // Do some error checking
        if(GArrayParT<parameter_type, N>::def_val_arr_.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFileArrayReferenceParsableParameterT::save_to(): Error!" << '\n'
                << "You need to provide at least one default value" << '\n'
            );
        }

        // Add the value and default items
        for(std::size_t i = 0; i < GArrayParT<parameter_type, N>::def_val_arr_.size(); i++) {
            pt.add(
                (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".default").c_str(),
                GArrayParT<parameter_type, N>::def_val_arr_.at(i)
            );
            pt.add(
                (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
                GArrayParT<parameter_type, N>::par_arr_.at(i)
            );
        }
    }

    /***************************************************************************/
    /**
	  * Assigns the parsed parameters to the reference vector
	  */
    void executeCallBackFunction_() override {
        stored_reference_ = GArrayParT<parameter_type, N>::par_arr_;
    }

    /***************************************************************************/

    std::array<parameter_type, N> &stored_reference_; ///< Holds a reference to the target vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable command line parameters. Note
 * that this class cannot be copied; copy and move are explicitly deleted.
 */
class GCLParsableI : public GParsableI {
    // We want GParserBuilder to be able to call our private load- and save functions
    friend class GParserBuilder;

public:
    /** @brief A constructor for individual items */
    GCLParsableI(std::string const &, std::string const &);
    /** @brief A constructor for vectors */
    GCLParsableI(std::vector<std::string> const &, std::vector<std::string> const &);

    /** @brief The destructor */
    ~GCLParsableI() override = default;

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GCLParsableI() = delete;
    GCLParsableI(GCLParsableI const &) = delete;
    GCLParsableI(GCLParsableI &&) = delete;
    GCLParsableI &operator=(GCLParsableI const &) = delete;
    GCLParsableI &operator=(GCLParsableI &&) = delete;

protected:
    /** @brief Saves data to a property tree object */
    virtual void save_to(boost::program_options::options_description &) const = 0;

    /** @brief Returns the content of this object as a std::string */
    virtual std::string content() const = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual command line parameters.
 */
template <typename parameter_type>
class GCLReferenceParsableParameterT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GCLParsableI {
    // We want GParserBuilder to be able to call our private functions
    friend class GParserBuilder;

public:
    /***************************************************************************/
    /**
	  * A constructor that initializes the internal reference
	  */
    GCLReferenceParsableParameterT(
        parameter_type &stored_reference,
        std::string const &option_name_var,
        std::string const &comment_var,
        parameter_type def_val,
        bool implicit_allowed,
        parameter_type impl_val
    )
      : GCLParsableI(
            GCLParsableI::makeVector(option_name_var),
            GCLParsableI::makeVector(comment_var)
        )
      , stored_reference_(stored_reference)
      , def_val_(def_val)
      , implicit_allowed_(implicit_allowed)
      , impl_val_(impl_val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * A constructor that initializes the internal variiables, except for comments
	  */
    GCLReferenceParsableParameterT(
        parameter_type &stored_reference,
        std::string const &option_name_var,
        parameter_type def_val,
        bool implicit_allowed,
        parameter_type impl_val
    )
      : GCLParsableI(
            GCLParsableI::makeVector(option_name_var),
            GCLParsableI::makeVector(std::string())
        )
      , stored_reference_(stored_reference)
      , def_val_(def_val)
      , implicit_allowed_(implicit_allowed)
      , impl_val_(impl_val) { /* nothing */
    }

    /***************************************************************************/
    // Prevent copying, moving and default construction
    GCLReferenceParsableParameterT() = delete;
    GCLReferenceParsableParameterT(GCLReferenceParsableParameterT<parameter_type> const &) = delete;
    GCLReferenceParsableParameterT(GCLReferenceParsableParameterT<parameter_type> &&) = delete;
    GCLReferenceParsableParameterT<parameter_type> &
    operator=(GCLReferenceParsableParameterT<parameter_type> const &) = delete;
    GCLReferenceParsableParameterT<parameter_type> &
    operator=(GCLReferenceParsableParameterT<parameter_type> &&) = delete;

private:
    /***************************************************************************/
    /**
	  * Saves data to a property tree object
	  */
    void save_to(boost::program_options::options_description &desc) const override {
        namespace po = boost::program_options;
        if(GCL_IMPLICIT_ALLOWED == implicit_allowed_) {
            desc.add_options()(
                (this->optionName()).c_str(),
                po::value<parameter_type>(&stored_reference_)
                    ->implicit_value(impl_val_)
                    ->default_value(def_val_),
                (this->comment()).c_str()
            );
        }
        else { // GCL_IMPLICIT_NOT_ALLOWED
            desc.add_options()(
                (this->optionName()).c_str(),
                po::value<parameter_type>(&stored_reference_)->default_value(def_val_),
                (this->comment()).c_str()
            );
        }
    }

    /***************************************************************************/
    /**
	  * Returns the content of this object as a std::string
	  */
    std::string content() const override {
        std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)
        result << this->optionName() << " :\t" << stored_reference_ << "\t"
               << ((stored_reference_ != def_val_)
                       ? "default: " + Gem::Common::to_string(def_val_)
                       : std::string());
        return result.str();
    }

    /***************************************************************************/

    parameter_type
        &stored_reference_;  ///< Holds the reference to which the parsed value will be assigned
    parameter_type def_val_; ///< Holds the default value
    bool
        implicit_allowed_; ///< Indicates, whether implicit values (e.g. --server=true vs. --server) are allowed
    parameter_type impl_val_; ///< Holds an implicit value used if only the option name is given
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements a "parser builder", that allows to easily specify
 * the options that the parser should search for in a configuration file.
 * Results of the parsing process will be written directly into the supplied
 * variables. If not found, a default value will be used. Note that this
 * class assumes that the parameter_type can be streamed using operator<< or
 * operator>>
 */
class GParserBuilder {
public:
    /** @brief The default constructor */
    GParserBuilder();

    /** @brief The destructor */
    virtual ~GParserBuilder() = default;

    // Prevent copying and moving
    GParserBuilder(GParserBuilder const &) = delete;
    GParserBuilder(GParserBuilder &&) = delete;
    GParserBuilder &operator=(GParserBuilder const &) = delete;
    GParserBuilder &operator=(GParserBuilder &&) = delete;

    /** @brief Reads and parses a configuration file, applying the values to the registered options. Optionally hands the parsed ptree back via the second argument so callers can cache it. */
    bool parseConfigFile(std::filesystem::path const &, boost::property_tree::ptree * = nullptr);
    /** @brief Applies an already-parsed configuration ptree to the registered options (no file access); runs the optional unknown-key diagnostic. */
    void loadFromPtree(
        boost::property_tree::ptree const &,
        std::filesystem::path const & = {},
        bool run_unknown_key_check = true
    );
    /** @brief Writes out a configuration file */
    void
    writeConfigFile(std::filesystem::path const &, std::string const & = "", bool = true) const;
    /** @brief Globally enables/disables the unknown-configuration-key diagnostic (default: enabled; warns on config keys no registered parameter consumes). */
    static void setCheckUnknownKeys(bool);
    /** @brief Retrieves whether the unknown-configuration-key diagnostic is enabled */
    static bool checkUnknownKeys();
    /** @brief Globally selects whether an unknown configuration-file key is an error (true) or a warning (false, the default). Only takes effect when the check is enabled. Call once at startup. */
    static void setUnknownKeyIsError(bool);
    /** @brief Retrieves whether unknown configuration-file keys are treated as an error */
    static bool unknownKeyIsError();
    /** @brief Provides information on the number of file configuration options stored in this class */
    std::size_t numberOfFileOptions() const;

    /** @brief Parses the commandline for options */
    bool parseCommandLine(int, char **, bool = false);
    /** @brief Provides information on the number of command line configuration options stored in this class */
    std::size_t numberOfCLOptions() const;

    /***************************************************************************/
    /**
	  * Allows to retrieve a GFileParsableI-derivative by name and to convert it to
	  * the derived type. This allows us to selectively change properties of these
	  * objects.
	  */
    template <typename fileParsableDerivative>
    std::shared_ptr<fileParsableDerivative>
    file_at(std::string const &option_name) { // NOLINT(misc-unused-parameters)
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            return std::dynamic_pointer_cast<fileParsableDerivative>(*it);
        }

        return {};
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve a GCLParsableI-derivative by name and to convert it to
	  * the derived type. This allows us to selectively change properties of these
	  * objects.
	  */
    template <typename clParsableDerivative>
    std::shared_ptr<clParsableDerivative>
    cl_at(std::string const &option_name) { // NOLINT(misc-unused-parameters)
        auto it = std::find_if(
            cl_parameter_proxies_.begin(),
            cl_parameter_proxies_.end(),
            [&](std::shared_ptr<GCLParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != cl_parameter_proxies_.end()) {
            return std::dynamic_pointer_cast<clParsableDerivative>(*it);
        }

        return {};
    }

    /////////////////////////////////////////////////////////////////////////////
    /***************************************************************************/
    /**
	  * Adds a single parameter of configurable type to the collection. When
	  * this parameter has been read using parseConfigFile, a call-back
	  * function is executed.
	  */
    template <typename parameter_type>
    GParsableI &registerFileParameter(
        std::string const &option_name,
        parameter_type def_val,
        std::function<void(parameter_type)> call_back,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment = std::string()
    ) {
#ifdef DEBUG
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(single_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileSingleParsableParameterT<parameter_type>> single_parm_ptr;

        if(comment.empty()) {
            single_parm_ptr = std::make_shared<GFileSingleParsableParameterT<parameter_type>>(
                option_name,
                def_val
            );
        }
        else {
            single_parm_ptr = std::make_shared<GFileSingleParsableParameterT<parameter_type>>(
                option_name,
                comment,
                is_essential,
                def_val
            );
        }

        single_parm_ptr->registerCallBackFunction(call_back);

        // Add to the proxy store
        file_parameter_proxies_.push_back(single_parm_ptr);
        return *single_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Adds a parameter with a configurable type to the collection.
	  *
	  * @param option_name The name of the option
	  * @param parameter The parameter into which the value will be written
	  * @param def_val A default value to be used if the corresponding parameter was not found in the configuration file
	  * @param is_essential A boolean which indicates whether this is an essential or a secondary parameter
	  * @param comment A comment to be associated with the parameter in configuration files
	  */
    template <typename parameter_type>
    GParsableI &registerFileParameter(
        std::string const &option_name,
        parameter_type &parameter,
        parameter_type def_val,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment = std::string()
    ) {
#ifdef DEBUG
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(ref_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileReferenceParsableParameterT<parameter_type>> ref_parm_ptr;

        if(comment.empty()) {
            ref_parm_ptr = std::make_shared<GFileReferenceParsableParameterT<parameter_type>>(
                parameter,
                option_name,
                def_val
            );
        }
        else {
            ref_parm_ptr = std::make_shared<GFileReferenceParsableParameterT<parameter_type>>(
                parameter,
                option_name,
                comment,
                is_essential,
                def_val
            );
        }

        // Add to the proxy store
        file_parameter_proxies_.push_back(ref_parm_ptr);
        return *ref_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
    template <typename parameter_type>
    void resetFileParameterDefaults(std::string const &option_name, parameter_type def_val) {
        // Retrieve the parameter object with this name
        std::shared_ptr<GSingleParmT<parameter_type>> parm_object =
            file_at<GSingleParmT<parameter_type>>(option_name);

        // Check that we have indeed received an item
        if(not parm_object) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterObject::resetFileParameterDefaults(GSingleParmT): Error!"
                << '\n'
                << "Parameter object couldn't be found" << '\n'
            );
        }

        // Reset the default value
        parm_object->resetDefault(def_val);
    }

    /////////////////////////////////////////////////////////////////////////////
    /***************************************************************************/
    /**
	  * Adds two parameters of configurable types to the collection. When
	  * these parameters have been read using parseConfigFile, a call-back
	  * function will be executed.
	  */
    template <typename par_type1, typename par_type2>
    GParsableI &registerFileParameter(
        std::string const &option_name1,
        std::string const &option_name2,
        par_type1 def_val1,
        par_type2 def_val2,
        std::function<void(par_type1, par_type2)> call_back,
        std::string const &combined_label,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment1 = std::string(),
        std::string const &comment2 = std::string()
    ) {
#ifdef DEBUG
        // Check whether the option already exists
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name1);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(comb_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name1 << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2>> comb_parm_ptr;

        if(comment1.empty() && comment2.empty()) {
            comb_parm_ptr = std::make_shared<GFileCombinedParsableParameterT<par_type1, par_type2>>(
                option_name1,
                def_val1,
                option_name2,
                def_val2,
                combined_label
            );
        }
        else {
            comb_parm_ptr = std::make_shared<GFileCombinedParsableParameterT<par_type1, par_type2>>(
                option_name1,
                comment1,
                def_val1,
                option_name2,
                comment2,
                def_val2,
                is_essential,
                combined_label
            );
        }

        comb_parm_ptr->registerCallBackFunction(call_back);

        // Add to the proxy store
        file_parameter_proxies_.push_back(comb_parm_ptr);
        return *comb_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
    template <typename par_type1, typename par_type2>
    void resetFileParameterDefaults(
        std::string const &option_name1,
        par_type1 def_val1,
        par_type2 def_val2
    ) {
        // Retrieve the parameter object with this name
        std::shared_ptr<GCombinedParT<par_type1, par_type2>> parm_object =
            file_at<GCombinedParT<par_type1, par_type2>>(option_name1);

        // Check that we have indeed received an item
        if(not parm_object) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterObject::resetFileParameterDefaults(GCombinedParT): Error!"
                << '\n'
                << "Parameter object couldn't be found" << '\n'
            );
        }

        // Reset the default value
        parm_object->resetDefault(def_val1, def_val2);
    }

    /////////////////////////////////////////////////////////////////////////////
    /***************************************************************************/
    /**
	  * Adds a vector of configurable type to the collection, using a
	  * call-back function
	  */
    template <typename parameter_type>
    GParsableI &registerFileParameter(
        std::string const &option_name,
        std::vector<parameter_type> const &def_val,
        std::function<void(std::vector<parameter_type>)> call_back,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment = std::string()
    ) {
#ifdef DEBUG
        // Check whether the option already exists
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(vec_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileVectorParsableParameterT<parameter_type>> vec_parm_ptr;

        if(comment.empty()) {
            vec_parm_ptr = std::make_shared<GFileVectorParsableParameterT<parameter_type>>(
                option_name,
                def_val
            );
        }
        else {
            vec_parm_ptr = std::make_shared<GFileVectorParsableParameterT<parameter_type>>(
                option_name,
                comment,
                def_val,
                is_essential
            );
        }

        vec_parm_ptr->registerCallBackFunction(call_back);

        // Add to the proxy store
        file_parameter_proxies_.push_back(vec_parm_ptr);
        return *vec_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Adds a reference to a vector of configurable type to the collection
	  */
    template <typename parameter_type>
    GParsableI &registerFileParameter(
        std::string const &option_name,
        std::vector<parameter_type> &stored_reference,
        std::vector<parameter_type> const &def_val,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment = std::string()
    ) {
#ifdef DEBUG
        // Check whether the option already exists
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(vec_ref_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type>> vec_ref_parm_ptr;

        if(comment.empty()) {
            vec_ref_parm_ptr =
                std::make_shared<GFileVectorReferenceParsableParameterT<parameter_type>>(
                    stored_reference,
                    option_name,
                    def_val
                );
        }
        else {
            vec_ref_parm_ptr =
                std::make_shared<GFileVectorReferenceParsableParameterT<parameter_type>>(
                    stored_reference,
                    option_name,
                    comment,
                    def_val,
                    is_essential
                );
        }

        // Add to the proxy store
        file_parameter_proxies_.push_back(vec_ref_parm_ptr);
        return *vec_ref_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
    template <typename parameter_type>
    void resetFileParameterDefaults(
        std::string const &option_name,
        std::vector<parameter_type> const &def_val
    ) {
        // Retrieve the parameter object with this name
        std::shared_ptr<GVectorParT<parameter_type>> parm_object =
            file_at<GVectorParT<parameter_type>>(option_name);

        // Check that we have indeed received an item
        if(not parm_object) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterObject::resetFileParameterDefaults(GVectorParT): Error!"
                << '\n'
                << "Parameter object couldn't be found" << '\n'
            );
        }

        // Reset the default value
        parm_object->resetDefault(def_val);
    }

    /////////////////////////////////////////////////////////////////////////////
    /***************************************************************************/
    /**
	  * Adds an array of configurable type but fixed size to the collection.
	  * This allows to make sure that a given amount of configuration options
	  * must be available.
	  */
    template <typename parameter_type, std::size_t N>
    GParsableI &registerFileParameter(
        std::string const &option_name,
        std::array<parameter_type, N> const &def_val,
        std::function<void(std::array<parameter_type, N>)> call_back,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment = std::string()
    ) {
#ifdef DEBUG
        // Check whether the option already exists
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(array_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileArrayParsableParameterT<parameter_type, N>> array_parm_ptr;

        if(comment.empty()) {
            array_parm_ptr = std::make_shared<GFileArrayParsableParameterT<parameter_type, N>>(
                option_name,
                def_val
            );
        }
        else {
            array_parm_ptr = std::make_shared<GFileArrayParsableParameterT<parameter_type, N>>(
                option_name,
                comment,
                def_val,
                is_essential
            );
        }

        // Register the call back function
        array_parm_ptr->registerCallBackFunction(call_back);

        // Add to the proxy store
        file_parameter_proxies_.push_back(array_parm_ptr);
        return *array_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Adds a reference to an array of configurable type but fixed size
	  * to the file parameter collection
	  */
    template <typename parameter_type, std::size_t N>
    GParsableI &registerFileParameter(
        std::string const &option_name,
        std::array<parameter_type, N> &stored_reference,
        std::array<parameter_type, N> const &def_val,
        bool is_essential = Gem::Common::VAR_IS_ESSENTIAL,
        std::string const &comment = std::string()
    ) {
#ifdef DEBUG
        // Check whether the option already exists
        auto it = std::find_if(
            file_parameter_proxies_.begin(),
            file_parameter_proxies_.end(),
            [&](std::shared_ptr<GFileParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );
        if(it != file_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerFileParameter(array_ref_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type, N>>
            array_ref_parm_ptr;
        if(comment.empty()) {
            array_ref_parm_ptr =
                std::make_shared<GFileArrayReferenceParsableParameterT<parameter_type, N>>(
                    stored_reference,
                    option_name,
                    def_val
                );
        }
        else {
            array_ref_parm_ptr =
                std::make_shared<GFileArrayReferenceParsableParameterT<parameter_type, N>>(
                    stored_reference,
                    option_name,
                    comment,
                    def_val,
                    is_essential
                );
        }

        // Add to the proxy store
        file_parameter_proxies_.push_back(array_ref_parm_ptr);
        return *array_ref_parm_ptr;
    }

    /***************************************************************************/
    /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */

    template <typename parameter_type, std::size_t N>
    void resetFileParameterDefaults(
        std::string const &option_name,
        std::array<parameter_type, N> const &def_val
    ) {
        // Retrieve the parameter object with this name
        std::shared_ptr<GArrayParT<parameter_type, N>> parm_object =
            file_at<GArrayParT<parameter_type, N>>(option_name);

        // Check that we have indeed received an item
        if(not parm_object) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterObject::resetFileParameterDefaults(GArrayParT): Error!"
                << '\n'
                << "Parameter object couldn't be found" << '\n'
            );
        }

        // Reset the default value
        parm_object->resetDefault(def_val);
    }

    /////////////////////////////////////////////////////////////////////////////
    /***************************************************************************/
    /**
	  * Adds a reference to a configurable type to the command line parameters.
	  */
    template <typename parameter_type>
    GParsableI &registerCLParameter(
        std::string const &option_name,
        parameter_type &parameter,
        parameter_type const &def_val,
        std::string const &comment = std::string(),
        bool implicit_allowed = GCL_IMPLICIT_NOT_ALLOWED,
        parameter_type impl_val = GDefaultValueT<parameter_type>::value()
    ) {
#ifdef DEBUG
        // Check whether the option already exists
        auto it = std::find_if(
            cl_parameter_proxies_.begin(),
            cl_parameter_proxies_.end(),
            [&](std::shared_ptr<GCLParsableI> const &candidate_ptr) {
                return (candidate_ptr->GParsableI::optionName(0) == option_name);
            }
        );

        if(it != cl_parameter_proxies_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::registerCLParameter(ref_parm_ptr): Error!" << '\n'
                << "Parameter " << option_name << " has already been registered" << '\n'
            );
        }
#endif /* DEBUG */

        std::shared_ptr<GCLReferenceParsableParameterT<parameter_type>> ref_parm_ptr;

        if(comment.empty()) {
            ref_parm_ptr = std::make_shared<GCLReferenceParsableParameterT<parameter_type>>(
                parameter,
                option_name,
                def_val,
                implicit_allowed,
                impl_val
            );
        }
        else {
            ref_parm_ptr = std::make_shared<GCLReferenceParsableParameterT<parameter_type>>(
                parameter,
                option_name,
                comment,
                def_val,
                implicit_allowed,
                impl_val
            );
        }

        // Add to the proxy store
        cl_parameter_proxies_.push_back(ref_parm_ptr);
        return *ref_parm_ptr;
    }

private:
    /***************************************************************************/

    std::vector<std::shared_ptr<GFileParsableI>>
        file_parameter_proxies_; ///< Holds file parameter proxies
    std::vector<std::shared_ptr<GCLParsableI>>
        cl_parameter_proxies_; ///< Holds command line parameter proxies

    std::filesystem::path config_base_dir_{};

    static std::mutex
        configfile_parser_mutex_; ///< Synchronization of access to configuration files (may only happen serially)
    static bool
        unknown_key_is_error_; ///< If true, an unknown configuration-file key throws instead of warning (default: false)
    static bool
        check_unknown_keys_; ///< If true, a genuine config-file parse warns about keys no registered parameter consumes (default: true; group-aware, runs once per parse)
};

/******************************************************************************/
/**
 * A helper function that lets users configure a given object from a file.
 * The function assumes that the target object has a suitable addConfigurationOptions
 * function. The function will automatically generate the configuration file using
 * the mechanisms implemented in GParserBuilder, should the file not exist.
 */
template <typename conf_object_type>
void configureFromFile(conf_object_type &target_object, std::filesystem::path const &conf_file) {
    // Create a parser builder object. It will be destroyed at
    // the end of this scope and thus cannot cause trouble
    // due to registered call-backs and references
    Gem::Common::GParserBuilder gpb;

    // Add configuration options from the target object
    target_object.addConfigurationOptions(gpb);

    //----------------------------------------------------------------------------
    // Some error checking

    // Check whether path is a directory name rather than
    // a file. It is a severe error if this is the case.
    if(std::filesystem::is_directory(conf_file)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In configureFromFile(" << conf_file.string() << "): Error!" << '\n'
            << "Target is a directory rather than a file." << '\n'
        );
    }

    // Check whether the target directory exists. It is a
    // severe error if this is not the case.
    if(not std::filesystem::exists(conf_file.parent_path())) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In configureFromFile(" << conf_file << "): Error!" << '\n'
            << "Target has invalid parent path" << '\n'
        );
    }

    //----------------------------------------------------------------------------
    // Do the actual parsing
    gpb.parseConfigFile(conf_file);

    //----------------------------------------------------------------------------
}

/******************************************************************************/

} /* namespace Gem::Common */
