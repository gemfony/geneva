/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <array>
#include <map>

// Boost headers go here
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/utility.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/optional.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GDefaultValueT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"

namespace Gem {
namespace Common {

// Forward declaration
class GParserBuilder;

/******************************************************************************/
// Indicates whether help was requested using the -h or --help switch on the command line
const bool GCL_HELP_REQUESTED = true;
const bool GCL_NO_HELP_REQUESTED = false;

// Indicates whether implicit values are allowed (such as in --server vs. --server=true)
const bool GCL_IMPLICIT_ALLOWED = true;
const bool GCL_IMPLICIT_NOT_ALLOWED = false;

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
template<typename parameter_type>
class GMultiSourceParameterT {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_default_value)
		 & BOOST_SERIALIZATION_NVP(m_parameter_values);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
	 /**
	  * Construction with a default value
	  */
	 explicit GMultiSourceParameterT(parameter_type default_value)
	 	: m_default_value(default_value)
	 { /* nothing */ }

	 /***************************************************************************/
	 // Defaulted constructors. destructors and assignment operators

	 GMultiSourceParameterT(GMultiSourceParameterT<parameter_type> const& cp) = default;
	 GMultiSourceParameterT(GMultiSourceParameterT<parameter_type> && cp) = default;
	 GMultiSourceParameterT<parameter_type> &operator=(GMultiSourceParameterT<parameter_type> const& cp) = default;
	 GMultiSourceParameterT<parameter_type> &operator=(GMultiSourceParameterT<parameter_type> && cp) = default;
	 ~GMultiSourceParameterT() = default;

	 /***************************************************************************/
	 /**
	  * Allows to set the value associated with a given data source
	  */
    void set(
    	Gem::Common::parameter_source data_source
    	, parameter_type parameter_value
	 ) {
		 m_parameter_values.at(data_source) = parameter_value;
    }

	 /***************************************************************************/
	 /**
	  * Allows to check whether the value for a given data source was set
	  */
    bool isSet(Gem::Common::parameter_source data_source) {
    	return *(m_parameter_values.at(data_source).second);
    }

	 /***************************************************************************/
	 /**
	  * Retrieves the first stored value that has been set, in the order of
	  * appearance in m_parameter_values, or alternatively the default value,
	  * if the value was not set from any source.
	  */
    parameter_type value() const {
    	for(auto const& v_pair: m_parameter_values) {
    		if(v_pair.second) { // Value was set
    			return *v_pair.second;
    		}

    		// Not set -- continue loop
    	}

    	// Nothing found
    	return m_default_value;
    }

	 /***************************************************************************/
	 /**
	  * Returns the value stored for a given data source. The function will throw
	  * when called for a parameter source not listed in m_parameter_values.
	  */
  	 parameter_type value(Gem::Common::parameter_source data_source) {
  	 	 return m_parameter_values.at(data_source);
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
   GMultiSourceParameterT() : m_default_value(parameter_type(nullptr))
   { /* nothing */ }

   /***************************************************************************/
   // Data

   parameter_type m_default_value; // The default value to be returned when all else fails

   // Value retrieval will look at each entry of the map until it finds one that was set.
   // If none was set, the default value will be returned
	std::map<Gem::Common::parameter_source, boost::optional<parameter_type>> m_parameter_values {
		{ Gem::Common::parameter_source::NETWORK, boost::optional<parameter_type>() }
		, { Gem::Common::parameter_source::COMMAND_LINE, boost::optional<parameter_type>() }
		, { Gem::Common::parameter_source::ENVIRONMENT_VARIABLE, boost::optional<parameter_type>() }
		, { Gem::Common::parameter_source::CONFIGURATION_FILE, boost::optional<parameter_type>() }
		, { Gem::Common::parameter_source::ASSIGNMENT, boost::optional<parameter_type>() }
	};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Gives write access to a reference to parm_ a single time. When this has happened,
 * only an explicit reset allows to gain access to a parameter-reference again.
 * It is however possible to explicitly set the parameter.
 */
template<typename parameter_type>
class GOneTimeRefParameterT {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_parm)
		 & BOOST_SERIALIZATION_NVP(m_parm_dummy)
		 & BOOST_SERIALIZATION_NVP(m_parm_set);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The standard constructor
	  */
	 explicit GOneTimeRefParameterT(parameter_type const& def = parameter_type(0))
		 : m_parm(def)
		 , m_parm_dummy(def)
		 , m_parm_set(false)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GOneTimeRefParameterT(GOneTimeRefParameterT<parameter_type> const& cp)
		 : m_parm(cp.m_parm)
		 , m_parm_dummy(cp.m_parm_dummy)
		 , m_parm_set(cp.m_parm_set)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Assignment of another object of this type
	  */
	 GOneTimeRefParameterT<parameter_type> &operator=(GOneTimeRefParameterT<parameter_type> const& cp) {
		 m_parm = cp.m_parm;
		 m_parm_dummy = cp.m_parm_dummy;
		 m_parm_set = cp.m_parm_set;

		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Returns a reference to the parameter, if it hasn't been set. Otherwise
	  * it will return a reference to the dummy parameter.
	  */
	 parameter_type &reference() {
		 if (m_parm_set) {
			 return m_parm_dummy;
		 } else {
			 m_parm_set = true;
			 return m_parm;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether a parameter has already been set
	  */
	 bool parmSet() const {
		 return m_parm_set;
	 }

	 /***************************************************************************/
	 /**
	  * Explicit reset of the "dirty" flag
	  */
	 void reset() {
		 m_parm_set = false;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the parameter value
	  */
	 parameter_type value() const {
		 return m_parm;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to explicitly set the value of the parameter
	  */
	 void setValue(parameter_type const & parm) {
		 m_parm = parm;
		 m_parm_set = true;
	 }

	 /***************************************************************************/
	 /**
	  * Explicit assignment of a parameter_type value
	  */
	 GOneTimeRefParameterT<parameter_type> &operator=(parameter_type const & parm) {
		 this->setValue(parm);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Automatic conversion
	  */
	 operator parameter_type() { // NOLINT
		 return m_parm;
	 }

	 /***************************************************************************/
	 /**
	  * Automatic conversion for constant callers
	  */
	 operator parameter_type() const { // NOLINT
		 return m_parm;
	 }

private:
	 /***************************************************************************/

	 parameter_type m_parm; ///< Stores the actual setting
	 parameter_type m_parm_dummy; ///< Returned instead of parm_ if the latter has already been set
	 bool m_parm_set; ///< Set to true if the parameter has been set already
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A manipulator object that allows to identify the id of the comment to be
 * added
 */
class commentLevel {
public:
	 /** @brief The standard constructor */
	 explicit G_API_COMMON commentLevel(std::size_t);

	 commentLevel() = delete; ///< The default constructor -- intentionally undefined

	 /** @brief Retrieves the current commentLevel */
	 G_API_COMMON std::size_t getCommentLevel() const;

private:
	 std::size_t m_comment_level; ///< The id of the comment inside of GParsableI
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
	 G_API_COMMON nextComment() = default;
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
	 G_API_COMMON GParsableI(
	 	 std::string const &
		 , std::string const &
	 );

	 /** @brief A constructor for vectors */
	 G_API_COMMON GParsableI(
	 	 std::vector<std::string> const &
		 , std::vector<std::string> const &
	 );

	 /** @brief The destructor */
	 virtual G_API_COMMON ~GParsableI() = default;

	 // Prevent copying, moving and default construction
	 GParsableI() = delete;
	 GParsableI(GParsableI const &) = delete;
	 GParsableI(GParsableI&&) = delete;
	 GParsableI& operator=(GParsableI const&) = delete;
	 GParsableI& operator=(GParsableI&&) = delete;

	 /** @brief Retrieves the option name at a given position */
	 G_API_COMMON std::string optionName(std::size_t = 0) const;
	 /** @brief Retrieves the comment that was assigned to this variable at a given position */
	 G_API_COMMON std::string comment(std::size_t = 0) const;
	 /** @brief Checks whether comments have indeed been registered */
	 G_API_COMMON bool hasComments() const;
	 /** @brief Retrieves the number of comments available */
	 G_API_COMMON std::size_t numberOfComments() const;

	 /***************************************************************************/
	 /**
	  * Create a std::vector<T> from a single element
	  */
	 template<typename T>
	 static std::vector<T> makeVector(T const &item) {
		 std::vector<T> result;
		 result.push_back(item);
		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * Create a std::vector<T> from two elements
	  */
	 template<typename T>
	 static std::vector<T> makeVector(T const &item1, T const & item2) {
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
	 template<typename T>
	 GParsableI &operator<<(T const &t) {
		 std::ostringstream oss;
		 oss << t;
		 m_comment.at(m_cl) += oss.str();
		 return *this;
	 }

	 /***************************************************************************/
	 /** @brief Needed for std::ostringstream */
	 G_API_COMMON GParsableI &operator<<(std::ostream &( *val )(std::ostream &));
	 /** @brief Needed for std::ostringstream */
	 G_API_COMMON GParsableI &operator<<(std::ios &( *val )(std::ios &));
	 /** @brief Needed for std::ostringstream */
	 G_API_COMMON GParsableI &operator<<(std::ios_base &( *val )(std::ios_base &));
	 /** @brief Allows to indicate the current comment level */
	 G_API_COMMON GParsableI &operator<<(commentLevel const &);
	 /** @brief Allows to switch to the next comment level */
	 G_API_COMMON GParsableI &operator<<(nextComment const &);

protected:
	 /***************************************************************************/
	 /** @brief Splits a comment into sub-tokens */
	 G_API_COMMON std::vector<std::string> splitComment(std::string const &) const;

private:
	 /***************************************************************************/
	 std::vector<std::string> m_option_name; ///< The name of this parameter
	 std::vector<std::string> m_comment; ///< A comment assigned to this parameter

	 std::size_t m_cl; ///< The id of the current comment inside of the comment_ vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable file parameters, to
 * which a call-back function has been assigned. Note that this class cannot
 * be copied, as the parent class is derived from boost::noncopyable.
 */
class GFileParsableI : public GParsableI
{
	 // We want GParserBuilder to be able to call our private load- and save functions
	 friend class GParserBuilder;

public:
	 /** @brief A constructor for individual items */
	 G_API_COMMON GFileParsableI(
	 	 std::string const &
		 , std::string const &
		 , bool
	 );
	 /** @brief A constructor for vectors */
	 G_API_COMMON GFileParsableI(
	 	 std::vector<std::string> const &
		 , std::vector<std::string> const &
		 , bool
	 );

	 /** @brief The destructor */
	 G_API_COMMON ~GFileParsableI() override = default;

	 // Prevent copying, moving and default construction
	 GFileParsableI() = delete;
	 GFileParsableI(GFileParsableI const &) = delete;
	 GFileParsableI(GFileParsableI &&) = delete;
	 GFileParsableI& operator=(GFileParsableI const &) = delete;
	 GFileParsableI& operator=(GFileParsableI&&) = delete;

	 /** @brief Checks whether this is an essential variable at a given position */
	 G_API_COMMON bool isEssential() const;

	 /** @brief Executes a stored callbacl function */
	 G_API_COMMON void executeCallBackFunction();

private:
	 /***************************************************************************/
	 /** @brief Loads data from a property_tree object */
	 virtual G_API_COMMON void load_from(boost::property_tree::ptree const &) = 0;

	 /** @brief Saves data to a property tree object */
	 virtual G_API_COMMON void save_to(boost::property_tree::ptree &) const = 0;

	 /** @brief Executes a stored call-back function */
	 virtual G_API_COMMON void executeCallBackFunction_() = 0;

	 /***************************************************************************/

	 bool m_is_essential; ///< Indicates whether this is an essential variable
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for single parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename parameter_type>
class GSingleParmT
	: public GFileParsableI
{
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GSingleParmT(
		 const std::string &optionNameVar
		 , const std::string &commentVar
		 , const bool &isEssentialVar
		 , const parameter_type &def_val
	 )
		 : GFileParsableI(optionNameVar, commentVar, isEssentialVar)
		 , m_def_val(def_val)
		 , m_par(def_val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GSingleParmT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GSingleParmT() = delete;
	 GSingleParmT(GSingleParmT<parameter_type> const&) = delete;
	 GSingleParmT(GSingleParmT<parameter_type>&&) = delete;
	 GSingleParmT<parameter_type>& operator=(GSingleParmT<parameter_type> const&) = delete;
	 GSingleParmT<parameter_type>& operator=(GSingleParmT<parameter_type>&&) = delete;

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par_, as its value will be overwritten
	  * as well. The reason is that configuration files will otherwise contain
	  * the "old" par_-value.
	  */
	 void resetDefault(parameter_type const & def_val) {
		 m_def_val = def_val;
		 m_par = def_val;
	 }

	 /***************************************************************************/
	 parameter_type m_def_val; ///< Holds the parameter's default value
	 parameter_type m_par; ///< Holds the individual parameter

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
template<typename parameter_type>
class GFileSingleParsableParameterT
	: public GSingleParmT<parameter_type>
{
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GFileSingleParsableParameterT(
		 const std::string &optionNameVar
		 , const std::string &commentVar
		 , const bool &isEssentialVar
		 , const parameter_type &def_val
	 )
		 : GSingleParmT<parameter_type>(optionNameVar, commentVar, isEssentialVar, def_val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class, except
	  * for comments.
	  */
	 GFileSingleParsableParameterT(
		 const std::string &optionNameVar
		 , const parameter_type &def_val
	 )
		 : GSingleParmT<parameter_type>(
		 		optionNameVar
		 		, std::string()
		 		, Gem::Common::VAR_IS_ESSENTIAL
		 		, def_val
			)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GFileSingleParsableParameterT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileSingleParsableParameterT() = delete;
	 GFileSingleParsableParameterT(GFileSingleParsableParameterT<parameter_type> const&) = delete;
	 GFileSingleParsableParameterT(GFileSingleParsableParameterT<parameter_type>&&) = delete;
	 GFileSingleParsableParameterT<parameter_type>& operator=(GFileSingleParsableParameterT<parameter_type> const&) = delete;
	 GFileSingleParsableParameterT<parameter_type>& operator=(GFileSingleParsableParameterT<parameter_type>&&) = delete;

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(parameter_type)> callBack) {
		 if (not callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSingleParsableParameter::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 void load_from(boost::property_tree::ptree const & pt) override {
		 GSingleParmT<parameter_type>::m_par = pt.get((GParsableI::optionName(0) + ".value").c_str(),
			 GSingleParmT<parameter_type>::m_def_val);
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
	 void save_to(boost::property_tree::ptree &pt) const override {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileSingleParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 if (not comments.empty()) {
				 for(auto const& comment: comments) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
				 }
			 }
		 }

		 pt.put((GParsableI::optionName(0) + ".default").c_str(), GSingleParmT<parameter_type>::m_def_val);
		 pt.put((GParsableI::optionName(0) + ".value").c_str(), GSingleParmT<parameter_type>::m_par);
	 }


	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 void executeCallBackFunction_() override {
		 if (not m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSingleParsableParameter::executeCallBackFunction_(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GSingleParmT<parameter_type>::m_par);
	 }

	 /***************************************************************************/

	 std::function<void(parameter_type)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual parameters. Instead of
 * executing a stored call-back function, executeCallBackFunction will assign
 * the parsed value to the reference.
 */
template<typename parameter_type>
class GFileReferenceParsableParameterT
	: public GSingleParmT<parameter_type> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GFileReferenceParsableParameterT(
		 parameter_type &storedReference
		 , std::string const & optionNameVar
		 , std::string const & commentVar
		 , bool isEssentialVar
		 , parameter_type const & def_val
	 )
		 : GSingleParmT<parameter_type>(
		 		optionNameVar
		 		, commentVar
		 		, isEssentialVar
		 		, def_val
	 		)
	 		, m_stored_reference(storedReference)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class, except
	  * for comments.
	  */
	 GFileReferenceParsableParameterT(
		 parameter_type &storedReference
		 , std::string const & optionNameVar
		 , parameter_type const & def_val
	 )
		 : GSingleParmT<parameter_type>(
		 		optionNameVar
		 		, std::string()
		 		, Gem::Common::VAR_IS_ESSENTIAL
		 		, def_val
	 		)
	 		, m_stored_reference(storedReference)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GFileReferenceParsableParameterT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileReferenceParsableParameterT() = delete;
	 GFileReferenceParsableParameterT(GFileReferenceParsableParameterT<parameter_type> const&) = delete;
	 GFileReferenceParsableParameterT(GFileReferenceParsableParameterT<parameter_type>&&) = delete;
	 GFileReferenceParsableParameterT<parameter_type>& operator=(GFileReferenceParsableParameterT<parameter_type> const&) = delete;
	 GFileReferenceParsableParameterT<parameter_type>& operator=(GFileReferenceParsableParameterT<parameter_type>&&) = delete;

private:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 void load_from(boost::property_tree::ptree const & pt) override {
		 GSingleParmT<parameter_type>::m_par = pt.get((GParsableI::optionName(0) + ".value").c_str(),
			 GSingleParmT<parameter_type>::m_def_val);
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
	 void save_to(boost::property_tree::ptree &pt) const override {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileReferenceParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 if (not comments.empty()) {
				 for(auto const& comment: comments) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
				 }
			 }
		 }

		 pt.put((GParsableI::optionName(0) + ".default").c_str(), GSingleParmT<parameter_type>::m_def_val);
		 pt.put((GParsableI::optionName(0) + ".value").c_str(), GSingleParmT<parameter_type>::m_par);
	 }

	 /***************************************************************************/
	 /**
	  * Assigns the stored parameter to the reference
	  */
	 void executeCallBackFunction_() override {
		 m_stored_reference = GSingleParmT<parameter_type>::m_par;
	 }

	 /***************************************************************************/

	 parameter_type &m_stored_reference; ///< Holds the reference to which the parsed value will be assigned
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for combined parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename par_type0, typename par_type1>
class GCombinedParT
	: public GFileParsableI
{
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GCombinedParT(
	 	 std::string const & optionNameVar0
		 , std::string const & commentVar0
		 , par_type0 const & def_val0
		 , std::string const & optionNameVar1
		 , std::string const & commentVar1
		 , par_type1 const & def_val1
		 , bool const & isEssentialVar
		 , std::string combined_label
	 )
		 : GFileParsableI(
		 		GFileParsableI::makeVector(optionNameVar0, optionNameVar1)
		 		, GFileParsableI::makeVector(commentVar0, commentVar1)
		 		, isEssentialVar
			)
		 , m_par0(def_val0)
		 , m_def_val0(def_val0)
		 , m_par1(def_val1)
		 , m_def_val1(def_val1)
		 , m_combined_label(std::move(combined_label))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GCombinedParT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GCombinedParT() = delete;
	 GCombinedParT(GCombinedParT<par_type0, par_type1> const&) = delete;
	 GCombinedParT(GCombinedParT<par_type0, par_type1>&&) = delete;
	 GCombinedParT<par_type0, par_type1>& operator=(GCombinedParT<par_type0, par_type1> const&) = delete;
	 GCombinedParT<par_type0, par_type1>& operator=(GCombinedParT<par_type0, par_type1>&&) = delete;

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par1_ and par_2, as their values will
	  * be overwritten as well. The reason is that configuration files will otherwise
	  * contain the "old" par_-value.
	  */
	 void resetDefault(
	 	par_type0 const & def_val0
	 	, par_type1 const & def_val1
	 ) {
		 m_def_val0 = def_val0;
		 m_def_val1 = def_val1;
		 m_par0 = def_val0;
		 m_par1 = def_val1;
	 }

	 /***************************************************************************/
	 par_type0 m_par0, m_def_val0; ///< Holds the individual parameters and default values 0
	 par_type1 m_par1, m_def_val1; ///< Holds the individual parameters and default values 1

	 std::string m_combined_label; ///< Holds a path label for the combined JSON path

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
template<typename par_type0, typename par_type1>
class GFileCombinedParsableParameterT : public GCombinedParT<par_type0, par_type1>
{
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileCombinedParsableParameterT(
	 	 std::string const & optionNameVar0
		 , std::string const & commentVar0
		 , par_type0 const & defVal0
		 , std::string const & optionNameVar1
		 , std::string const & commentVar1
		 , par_type1 const & defVal1
		 , bool isEssentialVar
		 , std::string const & combined_label
	 )
		 : GCombinedParT<par_type0, par_type1>(
		 		optionNameVar0
		 		, commentVar0
		 		, defVal0
		 		, optionNameVar1
		 		, commentVar1
		 		, defVal1
		 		, isEssentialVar
		 		, combined_label
			)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileCombinedParsableParameterT(
	 	 std::string const & optionNameVar0
		 , par_type0 const & defVal0
		 , std::string const & optionNameVar1
		 , par_type1 const & defVal1
		 , std::string const & combined_label
	 )
		 : GCombinedParT<par_type0, par_type1>(
		 	  optionNameVar0
		 	  , std::string()
		 	  , defVal0
		 	  , optionNameVar1
		 	  , std::string()
		 	  , defVal1
		 	  , Gem::Common::VAR_IS_ESSENTIAL
		 	  , combined_label
		  )
    { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GFileCombinedParsableParameterT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileCombinedParsableParameterT() = delete;
	 GFileCombinedParsableParameterT(GFileCombinedParsableParameterT<par_type0, par_type1> const&) = delete;
	 GFileCombinedParsableParameterT(GFileCombinedParsableParameterT<par_type0, par_type1>&&) = delete;
	 GFileCombinedParsableParameterT<par_type0, par_type1>& operator=(GFileCombinedParsableParameterT<par_type0, par_type1> const&) = delete;
	 GFileCombinedParsableParameterT<par_type0, par_type1>& operator=(GFileCombinedParsableParameterT<par_type0, par_type1>&&) = delete;

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(par_type0, par_type1)> callBack) {
		 if (not callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileCombinedParsableParameterT::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 void load_from(boost::property_tree::ptree const &pt) override {
		 GCombinedParT<par_type0, par_type1>::m_par0 = pt.get(
			 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(0) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val0);
		 GCombinedParT<par_type0, par_type1>::m_par1 = pt.get(
			 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(1) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val1);
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
	 void save_to(boost::property_tree::ptree &pt) const override {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 2) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileCombinedParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 2 comments but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments0 = GParsableI::splitComment(this->comment(0));
			 if (not comments0.empty()) {
				 for(auto const& comment: comments0) {
					 pt.add(
						 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(0) + ".comment").c_str()
						 , comment.c_str()
					 );
				 }
			 }
		 }
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(0) + ".default").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val0);
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(0) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_par0);

		 if (this->hasComments()) {
			 std::vector<std::string> comments1 = GParsableI::splitComment(this->comment(1));
			 if (not comments1.empty()) {
				 for(auto const& comment: comments1) {
					 pt.add((GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(1) +
								".comment").c_str(), comment.c_str());
				 }
			 }
		 }
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(1) + ".default").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val1);
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::m_combined_label + "." + GParsableI::optionName(1) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_par1);
	 }

	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 void executeCallBackFunction_() override {
		 if (not m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileCombinedParsableParameterT::executeCallBackFunction_(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GCombinedParT<par_type0, par_type1>::m_par0, GCombinedParT<par_type0, par_type1>::m_par1);
	 }

	 /***************************************************************************/

	 std::function<void(par_type0, par_type1)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for vector parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename parameter_type>
class GVectorParT
	: public GFileParsableI
{
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GVectorParT(
	 	 std::string const & optionNameVar
		 , std::string const & commentVar
		 , std::vector<parameter_type> const &def_val
		 , bool isEssentialVar
	 )
		 : GFileParsableI(
		 optionNameVar, commentVar, isEssentialVar
	 ), m_def_val_vec(def_val), m_par_vec() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GVectorParT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GVectorParT() = delete;
	 GVectorParT(GVectorParT<parameter_type> const&) = delete;
	 GVectorParT(GVectorParT<parameter_type>&&) = delete;
	 GVectorParT<parameter_type>& operator=(GVectorParT<parameter_type> const&) = delete;
	 GVectorParT<parameter_type>& operator=(GVectorParT<parameter_type>&&) = delete;

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value.
	  */
	 void resetDefault(std::vector<parameter_type> const & def_val) {
		 m_def_val_vec = def_val;
	 }

	 /***************************************************************************/
	 std::vector<parameter_type> m_def_val_vec; ///< Holds default values
	 std::vector<parameter_type> m_par_vec; ///< Holds the parsed parameters

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
template<typename parameter_type>
class GFileVectorParsableParameterT
	: public GVectorParT<parameter_type>
{
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileVectorParsableParameterT(
	 	 std::string const & optionNameVar
		 , std::string const & commentVar
		 , std::vector<parameter_type> const & def_val
		 , bool isEssentialVar
	 )
		 : GVectorParT<parameter_type>(
		 		optionNameVar
		 		, commentVar
		 		, def_val
		 		, isEssentialVar
		 )
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileVectorParsableParameterT(
	 	 std::string const & optionNameVar
		 , std::vector<parameter_type> const & def_val
	 )
		 : GVectorParT<parameter_type>(
		 		optionNameVar
		 		, std::string()
		 		, def_val
		 		, Gem::Common::VAR_IS_ESSENTIAL
			)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GFileVectorParsableParameterT() override = default;

	 // Prevent copying, moving and default construction
	 GFileVectorParsableParameterT() = delete;
	 GFileVectorParsableParameterT(GFileVectorParsableParameterT<parameter_type> const&) = delete;
	 GFileVectorParsableParameterT(GFileVectorParsableParameterT<parameter_type>&&) = delete;
	 GFileVectorParsableParameterT<parameter_type>& operator=(GFileVectorParsableParameterT<parameter_type> const&) = delete;
	 GFileVectorParsableParameterT<parameter_type>& operator=(GFileVectorParsableParameterT<parameter_type>&&) = delete;

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(std::vector<parameter_type>)> callBack) {
		 if (not callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileVectorParsableParameterT::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
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
		 GVectorParT<parameter_type>::m_par_vec.clear();

		 std::string ppath = GParsableI::optionName(0) + ".value";
		 for(auto const& v: pt.get_child(ppath.c_str())) {
			 GVectorParT<parameter_type>::m_par_vec.push_back(boost::lexical_cast<parameter_type>(v.second.data()));
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
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileVectorParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 if (not comments.empty()) {
			 	 for(auto const& comment: comments) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
			 	 }
			 }
		 }

		 // Do some error checking
		 if (GVectorParT<parameter_type>::m_def_val_vec.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GVectorParsableParameter::save_to(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 auto par_it = GVectorParT<parameter_type>::m_par_vec.cbegin();
		 for(auto const& def_val: GVectorParT<parameter_type>::m_def_val_vec) {
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
		 if (not m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileVectorParsableParameterT::executeCallBackFunction_(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GVectorParT<parameter_type>::m_par_vec);
	 }

	 /***************************************************************************/

	 std::function<void(std::vector<parameter_type>)> m_call_back_func; ///< Holds the call-back function
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
template<typename parameter_type>
class GFileVectorReferenceParsableParameterT
	: public GVectorParT<parameter_type>
{
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileVectorReferenceParsableParameterT(
		 std::vector<parameter_type> &stored_reference
		 , std::string const & optionNameVar
		 , std::string const & commentVar
		 , std::vector<parameter_type> const &def_val
		 , bool isEssentialVar
	 )
		 : GVectorParT<parameter_type>(
		 		optionNameVar
		 		, commentVar
		 		, def_val
		 		, isEssentialVar
		  )
		  , m_stored_reference(stored_reference)
    { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileVectorReferenceParsableParameterT(
		 std::vector<parameter_type> &stored_reference
		 , std::string const & optionNameVar
		 , std::vector<parameter_type> const & def_val
	 )
		 : GVectorParT<parameter_type>(
		 		optionNameVar
		 		, std::string()
		 		, def_val
		 		, Gem::Common::VAR_IS_ESSENTIAL
	 		)
	 		, m_stored_reference(stored_reference)
	 { /* nothing */ }


	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileVectorReferenceParsableParameterT() = delete;
	 GFileVectorReferenceParsableParameterT(GFileVectorReferenceParsableParameterT<parameter_type> const&) = delete;
	 GFileVectorReferenceParsableParameterT(GFileVectorReferenceParsableParameterT<parameter_type>&&) = delete;
	 GFileVectorReferenceParsableParameterT<parameter_type>& operator=(GFileVectorReferenceParsableParameterT<parameter_type> const&) = delete;
	 GFileVectorReferenceParsableParameterT<parameter_type>& operator=(GFileVectorReferenceParsableParameterT<parameter_type>&&) = delete;

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
	 void load_from(boost::property_tree::ptree const & pt) override {
		 using namespace boost::property_tree;

		 // Make sure the recipient vector is empty
		 GVectorParT<parameter_type>::m_par_vec.clear();

		 std::string ppath = GParsableI::optionName(0) + ".value";
		 for(auto const& v: pt.get_child(ppath.c_str())) {
			 GVectorParT<parameter_type>::m_par_vec.push_back(boost::lexical_cast<parameter_type>(v.second.data()));
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
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileVectorReferenceParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 if (not comments.empty()) {
			 	 for(auto const &comment: comments) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
			 	 }
			 }
		 }

		 // Do some error checking
		 if (GVectorParT<parameter_type>::m_def_val_vec.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileVectorReferenceParsableParameterT::save_to(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }


		 // Add the value and default items
		 auto par_it = GVectorParT<parameter_type>::m_par_vec.cbegin();
		 for(auto const& def_val: GVectorParT<parameter_type>::m_def_val_vec) {
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
		 m_stored_reference = GVectorParT<parameter_type>::m_par_vec;
	 }

	 /***************************************************************************/

	 std::vector<parameter_type> &m_stored_reference; ///< Holds a reference to the target vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for array parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename parameter_type, std::size_t N>
class GArrayParT
	: public GFileParsableI
{
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GArrayParT(
	    std::string const & optionNameVar
		 , std::string const & commentVar
		 , std::array<parameter_type, N> const & def_val
		 , bool isEssentialVar
	 )
		 : GFileParsableI(optionNameVar, commentVar, isEssentialVar)
		 , m_def_val_arr(def_val)
		 , m_par_arr(def_val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GArrayParT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GArrayParT() = delete;
	 GArrayParT(GArrayParT<parameter_type, N> const&) = delete;
	 GArrayParT(GArrayParT<parameter_type, N>&&) = delete;
	 GArrayParT<parameter_type, N>& operator=(GArrayParT<parameter_type, N> const&) = delete;
	 GArrayParT<parameter_type, N>& operator=(GArrayParT<parameter_type, N>&&) = delete;

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par_, as its value will be overwritten
	  * as well. The reason is that configuration files will otherwise contain
	  * the "old" par_-value.
	  */
	 void resetDefault(std::array<parameter_type, N> const & def_val_arr) {
		 m_def_val_arr = def_val_arr;
		 m_par_arr = def_val_arr;
	 }

	 /***************************************************************************/
	 std::array<parameter_type, N> m_def_val_arr; ///< Holds default values
	 std::array<parameter_type, N> m_par_arr; ///< Holds the parsed parameters

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
template<typename parameter_type, std::size_t N>
class GFileArrayParsableParameterT : public GArrayParT<parameter_type, N>
{
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileArrayParsableParameterT(
	 	 std::string const & optionNameVar
		 , std::string const & commentVar
		 , std::array<parameter_type, N> const & def_val
		 , bool isEssentialVar
	 )
		 : GArrayParT<parameter_type, N>(
		 		optionNameVar
		 		, commentVar
		 		, def_val
		 		, isEssentialVar
			)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileArrayParsableParameterT(
	 	 std::string const & optionNameVar
		 , std::array<parameter_type, N> const & def_val
	 )
		 : GArrayParT<parameter_type, N>(
		 		optionNameVar
		 		, std::string()
		 		, def_val
		 		, Gem::Common::VAR_IS_ESSENTIAL
	 		)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GFileArrayParsableParameterT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileArrayParsableParameterT() = delete;
	 GFileArrayParsableParameterT(GFileArrayParsableParameterT<parameter_type, N> const&) = delete;
	 GFileArrayParsableParameterT(GFileArrayParsableParameterT<parameter_type, N>&&) = delete;
	 GFileArrayParsableParameterT<parameter_type, N>& operator=(GFileArrayParsableParameterT<parameter_type, N> const&) = delete;
	 GFileArrayParsableParameterT<parameter_type, N>& operator=(GFileArrayParsableParameterT<parameter_type, N>&&) = delete;

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(std::array<parameter_type, N>)> callBack) {
		 if (not callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayParsableParameterT::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
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
		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_par_arr.size(); i++) {
			 GArrayParT<parameter_type,N>::m_par_arr.at(i)
			 	= pt.get(
				 (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str()
				 , GArrayParT<parameter_type, N>::m_def_val_arr.at(i)
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
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileArrayParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 if (not comments.empty()) {
			 	 for (auto const& comment: comments) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), comment.c_str());
			 	 }
			 }
		 }

		 // Do some error checking
		 if (GArrayParT<parameter_type, N>::m_def_val_arr.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayParsableParameterT::save_to(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_def_val_arr.size(); i++) {
			 pt.add((GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".default").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val_arr.at(i));
			 pt.add((GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_par_arr.at(i));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 void executeCallBackFunction_() override {
		 if (not m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayParsableParameterT::executeCallBackFunction_(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GArrayParT<parameter_type, N>::m_par_arr);
	 }

	 /***************************************************************************/

	 std::function<void(std::array<parameter_type, N>)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to a std::array of values (obviously of
 * identical type). This class enforces a fixed number of items in the array.
 */
template<typename parameter_type, std::size_t N>
class GFileArrayReferenceParsableParameterT
	: public GArrayParT<parameter_type, N>
{
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileArrayReferenceParsableParameterT(
		 std::array<parameter_type, N> &stored_reference
		 , std::string const & optionNameVar
		 , std::string const & commentVar
		 , std::array<parameter_type, N> const & def_val
		 , bool isEssentialVar
	 )
		 : GArrayParT<parameter_type, N>(
		 		optionNameVar
		 		, commentVar
		 		, def_val
		 		, isEssentialVar
		   )
		 , m_stored_reference(stored_reference)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileArrayReferenceParsableParameterT(
		 std::array<parameter_type, N> &stored_reference
		 , std::string const & optionNameVar
		 , std::array<parameter_type, N> const & def_val
	 )
		 : GArrayParT<parameter_type, N>(
		 		optionNameVar
		 		, std::string()
		 		, def_val
		 		, Gem::Common::VAR_IS_ESSENTIAL
		  )
		  , m_stored_reference(stored_reference)
    { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GFileArrayReferenceParsableParameterT() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileArrayReferenceParsableParameterT() = delete;
	 GFileArrayReferenceParsableParameterT(GFileArrayReferenceParsableParameterT<parameter_type, N> const&) = delete;
	 GFileArrayReferenceParsableParameterT(GFileArrayReferenceParsableParameterT<parameter_type, N>&&) = delete;
	 GFileArrayReferenceParsableParameterT<parameter_type, N>& operator=(GFileArrayReferenceParsableParameterT<parameter_type, N> const&) = delete;
	 GFileArrayReferenceParsableParameterT<parameter_type, N>& operator=(GFileArrayReferenceParsableParameterT<parameter_type, N>&&) = delete;

private:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 void load_from(boost::property_tree::ptree const & pt) override {
		 using namespace boost::property_tree;

		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_par_arr.size(); i++) {
			 GArrayParT<parameter_type, N>::m_par_arr.at(i) = pt.get(
				 (GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val_arr.at(i));
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
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileArrayReferenceParsableParameterT<>::save_to(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (not comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 // Do some error checking
		 if (GArrayParT<parameter_type, N>::m_def_val_arr.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayReferenceParsableParameterT::save_to(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_def_val_arr.size(); i++) {
			 pt.add((GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".default").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val_arr.at(i));
			 pt.add((GParsableI::optionName(0) + "." + Gem::Common::to_string(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_par_arr.at(i));
		 }
	 }


	 /***************************************************************************/
	 /**
	  * Assigns the parsed parameters to the reference vector
	  */
	 void executeCallBackFunction_() override {
		 m_stored_reference = GArrayParT<parameter_type, N>::m_par_arr;
	 }

	 /***************************************************************************/

	 std::array<parameter_type, N> &m_stored_reference; ///< Holds a reference to the target vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable command line parameters. Note
 * that this class cannot be copied, as the parent class is derived from
 * boost::noncopyable.
 */
class GCLParsableI
	: public GParsableI
{
	 // We want GParserBuilder to be able to call our private load- and save functions
	 friend class GParserBuilder;

public:
	 /** @brief A constructor for individual items */
	 G_API_COMMON GCLParsableI(
		 std::string const &
		 , std::string const &
	 );
	 /** @brief A constructor for vectors */
	 G_API_COMMON GCLParsableI(
		 std::vector<std::string> const &
		 , std::vector<std::string> const &
	 );

	 /** @brief The destructor */
	 G_API_COMMON ~GCLParsableI() override = default;

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GCLParsableI() = delete;
	 GCLParsableI(GCLParsableI const&) = delete;
	 GCLParsableI(GCLParsableI&&) = delete;
	 GCLParsableI& operator=(GCLParsableI const&) = delete;
	 GCLParsableI& operator=(GCLParsableI&&) = delete;

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
template<typename parameter_type>
class GCLReferenceParsableParameterT
	: public GCLParsableI
{
	 // We want GParserBuilder to be able to call our private functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * A constructor that initializes the internal reference
	  *
	  * @param storedReference A reference to a variable in which parsed values should be stored
	  * @param defVal The default value of this variable
	  */
	 GCLReferenceParsableParameterT(
		 parameter_type &storedReference
		 , std::string const & optionNameVar
		 , std::string const & commentVar
		 , parameter_type defVal
		 , bool implicitAllowed
		 , parameter_type implVal
	 )
		 : GCLParsableI(
		 		GCLParsableI::makeVector(optionNameVar)
		 		, GCLParsableI::makeVector(commentVar)
	 		)
	    , m_stored_reference(storedReference)
	    , m_def_val(defVal)
	    , m_implicit_allowed(implicitAllowed)
	    , m_impl_val(implVal)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A constructor that initializes the internal variiables, except for comments
	  *
	  * @param storedReference A reference to a variable in which parsed values should be stored
	  * @param defVal The default value of this variable
	  */
	 GCLReferenceParsableParameterT(
		 parameter_type &storedReference
		 , std::string const & optionNameVar
		 , parameter_type defVal
		 , bool implicitAllowed
		 , parameter_type implVal
	 )
		 : GCLParsableI(
		 		GCLParsableI::makeVector(optionNameVar)
		 		, GCLParsableI::makeVector(std::string())
	 		)
	    , m_stored_reference(storedReference)
	    , m_def_val(defVal)
	    , m_implicit_allowed(implicitAllowed)
	    , m_impl_val(implVal)
	 { /* nothing */ }

	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GCLReferenceParsableParameterT() = delete;
	 GCLReferenceParsableParameterT(GCLReferenceParsableParameterT<parameter_type> const&) = delete;
	 GCLReferenceParsableParameterT(GCLReferenceParsableParameterT<parameter_type>&&) = delete;
	 GCLReferenceParsableParameterT<parameter_type>& operator=(GCLReferenceParsableParameterT<parameter_type> const&) = delete;
	 GCLReferenceParsableParameterT<parameter_type>& operator=(GCLReferenceParsableParameterT<parameter_type>&&) = delete;

private:
	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object
	  */
	 void save_to(boost::program_options::options_description &desc) const override {
		 namespace po = boost::program_options;
		 if (GCL_IMPLICIT_ALLOWED == m_implicit_allowed) {
			 desc.add_options()(
				 (this->optionName()).c_str(),
				 po::value<parameter_type>(&m_stored_reference)->implicit_value(m_impl_val)->default_value(m_def_val),
				 (this->comment()).c_str()
			 );
		 } else { // GCL_IMPLICIT_NOT_ALLOWED
			 desc.add_options()(
				 (this->optionName()).c_str(), po::value<parameter_type>(&m_stored_reference)->default_value(m_def_val),
				 (this->comment()).c_str()
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the content of this object as a std::string
	  */
	 std::string content() const override {
		 std::ostringstream result;
		 result << this->optionName() << " :\t" << m_stored_reference << "\t" <<
				  ((m_stored_reference != m_def_val) ? "default: " + Gem::Common::to_string(m_def_val) : std::string());
		 return result.str();
	 }

	 /***************************************************************************/

	 parameter_type &m_stored_reference; ///< Holds the reference to which the parsed value will be assigned
	 parameter_type m_def_val; ///< Holds the default value
	 bool m_implicit_allowed; ///< Indicates, whether implicit values (e.g. --server=true vs. --server) are allowed
	 parameter_type m_impl_val; ///< Holds an implicit value used if only the option name is given
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
	 G_API_COMMON GParserBuilder();

	 /** @brief The destructor */
	 virtual G_API_COMMON ~GParserBuilder() = default;

	 // Prevent copying and moving
	 GParserBuilder(GParserBuilder const&) = delete;
	 GParserBuilder(GParserBuilder&&) = delete;
	 GParserBuilder& operator=(GParserBuilder const&) = delete;
	 GParserBuilder& operator=(GParserBuilder&&) = delete;

	 /** @brief Tries to parse a given configuration file for a set of options */
	 G_API_COMMON bool parseConfigFile(boost::filesystem::path const &);
	 /** @brief Writes out a configuration file */
	 G_API_COMMON void writeConfigFile(boost::filesystem::path const &, std::string const & = "", bool = true) const;
	 /** @brief Provides information on the number of file configuration options stored in this class */
	 G_API_COMMON std::size_t numberOfFileOptions() const;

	 /** @brief Parses the commandline for options */
	 G_API_COMMON bool parseCommandLine(int, char **, bool = false);
	 /** @brief Provides information on the number of command line configuration options stored in this class */
	 G_API_COMMON std::size_t numberOfCLOptions() const;

	 /***************************************************************************/
	 /**
	  * Allows to retrieve a GFileParsableI-derivative by name and to convert it to
	  * the derived type. This allows us to selectively change properties of these
	  * objects.
	  */
	 template<typename fileParsableDerivative>
	 std::shared_ptr<fileParsableDerivative> file_at(std::string const& optionName) {
		 auto it = std::find_if(
			 m_file_parameter_proxies.begin()
			 , m_file_parameter_proxies.end()
			 , [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
			 		return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 	}
		 );
		 if (it != m_file_parameter_proxies.end()) {
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
	 template<typename clParsableDerivative>
	 std::shared_ptr<clParsableDerivative> cl_at(std::string const& optionName) {
		 auto it = std::find_if(
			 m_cl_parameter_proxies.begin()
			 , m_cl_parameter_proxies.end()
			 , [&](std::shared_ptr<GCLParsableI> const& candidate_ptr) {
			 		return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 	}
		 );
		 if (it != m_cl_parameter_proxies.end()) {
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
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
	 	 std::string const &optionName
		 , parameter_type def_val
		 , std::function<void(parameter_type)> callBack
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const &comment = std::string()
	 ) {
#ifdef DEBUG
		 auto it = std::find_if(
			 m_file_parameter_proxies.begin()
			 , m_file_parameter_proxies.end()
			 , [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
		 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(singleParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GFileSingleParsableParameterT<parameter_type>> singleParm_ptr;

		 if (comment.empty()) {
			 singleParm_ptr = std::shared_ptr<GFileSingleParsableParameterT<parameter_type>>(
				 new GFileSingleParsableParameterT<parameter_type>(
					 optionName, def_val
				 )
			 );
		 } else {
			 singleParm_ptr = std::shared_ptr<GFileSingleParsableParameterT<parameter_type>>(
				 new GFileSingleParsableParameterT<parameter_type>(
					 optionName, comment, isEssential, def_val
				 )
			 );
		 }

		 singleParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(singleParm_ptr);
		 return *singleParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a parameter with a configurable type to the collection.
	  *
	  * @param optionName The name of the option
	  * @param parameter The parameter into which the value will be written
	  * @param def_val A default value to be used if the corresponding parameter was not found in the configuration file
	  * @param isEssential A boolean which indicates whether this is an essential or a secondary parameter
	  * @param comment A comment to be associated with the parameter in configuration files
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
	 	 std::string const &optionName
		 , parameter_type &parameter
		 , parameter_type def_val
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const &comment = std::string()
	 ) {
#ifdef DEBUG
		 auto it = std::find_if(
			 m_file_parameter_proxies.begin()
			 , m_file_parameter_proxies.end()
			 , [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
		 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(refParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GFileReferenceParsableParameterT<parameter_type>> refParm_ptr;

		 if (comment.empty()) {
			 refParm_ptr = std::shared_ptr<GFileReferenceParsableParameterT<parameter_type>>(
				 new GFileReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, def_val
				 )
			 );
		 } else {
			 refParm_ptr = std::shared_ptr<GFileReferenceParsableParameterT<parameter_type>>(
				 new GFileReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, comment, isEssential, def_val
				 )
			 );
		 }

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(refParm_ptr);
		 return *refParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
	 template<typename parameter_type>
	 void resetFileParameterDefaults(
	 	 std::string const & optionName
		 , parameter_type def_val
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr<GSingleParmT<parameter_type>> parmObject
			 = file_at<GSingleParmT<parameter_type>>(optionName);

		 // Check that we have indeed received an item
		 if (not parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GSingleParmT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds two parameters of configurable types to the collection. When
	  * these parameters have been read using parseConfigFile, a call-back
	  * function will be executed.
	  */
	 template<typename par_type1, typename par_type2>
	 GParsableI &registerFileParameter(
	 	 std::string const & optionName1
		 , std::string const & optionName2
		 , par_type1 def_val1
		 , par_type2 def_val2
		 , std::function<void(par_type1, par_type2)> callBack
		 , std::string const & combined_label
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const & comment1 = std::string()
		 , std::string const & comment2 = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 auto it = std::find_if(
		 	m_file_parameter_proxies.begin()
		 	, m_file_parameter_proxies.end()
			, [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName1);
			 }
		 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(combParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName1 << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2>> combParm_ptr;

		 if (comment1.empty() && comment2.empty()) {
			 combParm_ptr = std::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2>>(
				 new GFileCombinedParsableParameterT<par_type1, par_type2>(
					 optionName1, def_val1, optionName2, def_val2, combined_label
				 )
			 );
		 } else {
			 combParm_ptr = std::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2>>(
				 new GFileCombinedParsableParameterT<par_type1, par_type2>(
					 optionName1, comment1, def_val1, optionName2, comment2, def_val2, isEssential, combined_label
				 )
			 );
		 }

		 combParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(combParm_ptr);
		 return *combParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
	 template<typename par_type1, typename par_type2>
	 void resetFileParameterDefaults(
	 	 std::string const & optionName1
		 , par_type1 def_val1
		 , par_type2 def_val2
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr <GCombinedParT<par_type1, par_type2>> parmObject
			 = file_at<GCombinedParT<par_type1, par_type2>>(optionName1);

		 // Check that we have indeed received an item
		 if (not parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GCombinedParT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val1, def_val2);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds a vector of configurable type to the collection, using a
	  * call-back function
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
	 	 std::string const &optionName
		 , std::vector<parameter_type> const & def_val
		 , std::function<void(std::vector<parameter_type>)> callBack
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const & comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 auto it = std::find_if(
		 	m_file_parameter_proxies.begin()
		 	, m_file_parameter_proxies.end()
			, [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
		 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(vecParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GFileVectorParsableParameterT<parameter_type>> vecParm_ptr;

		 if (comment.empty()) {
			 vecParm_ptr = std::shared_ptr<GFileVectorParsableParameterT<parameter_type>>(
				 new GFileVectorParsableParameterT<parameter_type>(
					 optionName, def_val
				 )
			 );
		 } else {
			 vecParm_ptr = std::shared_ptr<GFileVectorParsableParameterT<parameter_type>>(
				 new GFileVectorParsableParameterT<parameter_type>(
					 optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 vecParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(vecParm_ptr);
		 return *vecParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a reference to a vector of configurable type to the collection
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
	 	 std::string const & optionName
		 , std::vector<parameter_type> &stored_reference
		 , std::vector<parameter_type> const &def_val
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 auto it = std::find_if(
		 	m_file_parameter_proxies.begin()
		 	, m_file_parameter_proxies.end()
			, [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
	 	 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(vecRefParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */


		 std::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type>> vecRefParm_ptr;

		 if (comment.empty()) {
			 vecRefParm_ptr = std::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type>>(
				 new GFileVectorReferenceParsableParameterT<parameter_type>(
					 stored_reference, optionName, def_val
				 )
			 );
		 } else {
			 vecRefParm_ptr = std::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type>>(
				 new GFileVectorReferenceParsableParameterT<parameter_type>(
					 stored_reference, optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(vecRefParm_ptr);
		 return *vecRefParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
	 template<typename parameter_type>
	 void resetFileParameterDefaults(
	 	 std::string const & optionName
		 , std::vector<parameter_type> const & def_val
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr<GVectorParT<parameter_type>> parmObject
			 = file_at<GVectorParT<parameter_type>>(optionName);

		 // Check that we have indeed received an item
		 if (not parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GVectorParT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds an array of configurable type but fixed size to the collection.
	  * This allows to make sure that a given amount of configuration options
	  * must be available.
	  */
	 template<typename parameter_type, std::size_t N>
	 GParsableI &registerFileParameter(
	 	 std::string const & optionName
		 , std::array<parameter_type, N> const &def_val
		 , std::function<void(std::array<parameter_type, N>)> callBack
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const & comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 auto it = std::find_if(
		 	m_file_parameter_proxies.begin()
		 	, m_file_parameter_proxies.end()
			, [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
		 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(arrayParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GFileArrayParsableParameterT<parameter_type, N>> arrayParm_ptr;

		 if (comment.empty()) {
			 arrayParm_ptr = std::shared_ptr<GFileArrayParsableParameterT<parameter_type, N>>(
				 new GFileArrayParsableParameterT<parameter_type, N>(
					 optionName, def_val
				 )
			 );
		 } else {
			 arrayParm_ptr = std::shared_ptr<GFileArrayParsableParameterT<parameter_type, N>>(
				 new GFileArrayParsableParameterT<parameter_type, N>(
					 optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 // Register the call back function
		 arrayParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(arrayParm_ptr);
		 return *arrayParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a reference to an array of configurable type but fixed size
	  * to the file parameter collection
	  */
	 template<typename parameter_type, std::size_t N>
	 GParsableI &registerFileParameter(
	 	 std::string const & optionName
		 , std::array<parameter_type, N> &stored_reference
		 , std::array<parameter_type, N> const & def_val
		 , bool isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , std::string const & comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 auto it = std::find_if(
		 	m_file_parameter_proxies.begin()
		 	, m_file_parameter_proxies.end()
			, [&](std::shared_ptr<GFileParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
		 );
		 if(it != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(arrayRefParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type, N>> arrayRefParm_ptr;
		 if (comment.empty()) {
			 arrayRefParm_ptr = std::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type, N>>(
				 new GFileArrayReferenceParsableParameterT<parameter_type, N>(
					 stored_reference, optionName, def_val
				 )
			 );
		 } else {
			 arrayRefParm_ptr = std::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type, N>>(
				 new GFileArrayReferenceParsableParameterT<parameter_type, N>(
					 stored_reference, optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(arrayRefParm_ptr);
		 return *arrayRefParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */

	 template<typename parameter_type, std::size_t N>
	 void resetFileParameterDefaults(
	 	 std::string const & optionName
		 , std::array<parameter_type, N> const & def_val
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr<GArrayParT<parameter_type, N>> parmObject
			 = file_at<GArrayParT<parameter_type, N>>(optionName);

		 // Check that we have indeed received an item
		 if (not parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GArrayParT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds a reference to a configurable type to the command line parameters.
	  *
	  * @param optionName The name of the option
	  * @param parameter The parameter into which the value will be written
	  * @param def_val A default value to be used if the corresponding parameter was not found in the configuration file
	  * @param comment A comment to be associated with the parameter in configuration files
	  */
	 template<typename parameter_type>
	 GParsableI &registerCLParameter(
	 	 std::string const & optionName
		 , parameter_type &parameter
		 , parameter_type const & def_val
		 , std::string const & comment = std::string()
		 , bool implicitAllowed = GCL_IMPLICIT_NOT_ALLOWED
		 , parameter_type impl_val = GDefaultValueT<parameter_type>::value()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
	 	 auto it = std::find_if(
	 	 	m_cl_parameter_proxies.begin()
	 	 	, m_cl_parameter_proxies.end()
			, [&](std::shared_ptr<GCLParsableI> const& candidate_ptr) {
				 return (candidate_ptr->GParsableI::optionName(0) == optionName);
			 }
		 );

		 if(it != m_cl_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerCLParameter(refParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr<GCLReferenceParsableParameterT<parameter_type>> refParm_ptr;

		 if (comment.empty()) {
			 refParm_ptr = std::shared_ptr<GCLReferenceParsableParameterT<parameter_type>>(
				 new GCLReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, def_val, implicitAllowed, impl_val
				 )
			 );
		 } else {
			 refParm_ptr = std::shared_ptr<GCLReferenceParsableParameterT<parameter_type>>(
				 new GCLReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, comment, def_val, implicitAllowed, impl_val
				 )
			 );
		 }

		 // Add to the proxy store
		 m_cl_parameter_proxies.push_back(refParm_ptr);
		 return *refParm_ptr;
	 }

private:
	 /***************************************************************************/

	 std::vector<std::shared_ptr<GFileParsableI>> m_file_parameter_proxies; ///< Holds file parameter proxies
	 std::vector<std::shared_ptr<GCLParsableI>> m_cl_parameter_proxies;   ///< Holds command line parameter proxies

	 boost::filesystem::path m_config_base_dir{};

	 static std::mutex m_configfile_parser_mutex; ///< Synchronization of access to configuration files (may only happen serially)
};

/******************************************************************************/
/**
 * A helper function that lets users configure a given object from a file.
 * The function assumes that the target object has a suitable addConfigurationOptions
 * function. The function will automatically generate the configuration file using
 * the mechanisms implemented in GParserBuilder, should the file not exist.
 *
 * @param path Name and path to a configuration file
 */
template <typename conf_object_type>
void configureFromFile(
	conf_object_type& target_object
	, boost::filesystem::path const & conf_file
) {
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
	if(boost::filesystem::is_directory(conf_file)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In configureFromFile(" << conf_file.string() << "): Error!" << std::endl
				<< "Target is a directory rather than a file." << std::endl
		);
	}

	// Check whether the target directory exists. It is a
	// severe error if this is not the case.
	if(not boost::filesystem::exists(conf_file.parent_path())) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In configureFromFile(" << conf_file << "): Error!" << std::endl
				<< "Target has invalid parent path" << std::endl
		);
	}

	//----------------------------------------------------------------------------
	// Do the actual parsing
	gpb.parseConfigFile(conf_file);

	//----------------------------------------------------------------------------
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
