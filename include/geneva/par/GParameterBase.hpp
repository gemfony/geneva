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

// Standard header files go here
#include <any>
#include <concepts>
#include <random>

// Boost header files go here

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "geneva/par/GMutableParameterI.hpp"
#include "geneva/GObject.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The purpose of this class is to provide a common base for all parameter classes, so
 * that a GParameterSet can be built from different parameter types. The class also
 * defines the interface that needs to be implemented by parameter classes.
 */
class GParameterBase
  : public GObject
  , public GMutableParameterI {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject) &
            BOOST_SERIALIZATION_NVP(adaptionsActive_) &
            BOOST_SERIALIZATION_NVP(randomInitializationBlocked_) &
            BOOST_SERIALIZATION_NVP(parameterName_);
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GParameterBase() = default;
    GParameterBase(GParameterBase const &) = default;
    GParameterBase(GParameterBase &&) = default;

    ~GParameterBase() override = default;

    GParameterBase &operator=(GParameterBase const &) = default;
    GParameterBase &operator=(GParameterBase &&) = default;

    /*********************************************************************/

    /** @brief The adaption interface */
    std::size_t adapt(Gem::Hap::GRandomBase &) override;

    /** @brief Update adaptors depending on the number of iterations without improvement */
    bool updateAdaptorsOnStall(std::size_t);

    /** @brief Retrieves information from an adaptor on a given property */
    void queryAdaptor(
        const std::string &adaptor_name,
        const std::string &property,
        std::vector<std::any> &data
    ) const;

    /** @brief Switches on adaptions for this object */
    bool setAdaptionsActive();
    /** @brief Disables adaptions for this object */
    bool setAdaptionsInactive();
    /** @brief Determines whether adaptions are performed for this object */
    bool adaptionsActive() const;
    /** @brief Determines whether adaptions are inactive for this object */
    bool adaptionsInactive() const;

    /** @brief Triggers random initialization of the parameter(-collection) */
    virtual bool randomInit(const activityMode &, Gem::Hap::GRandomBase &);

    /** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
    bool isIndividualParameter() const;
    /** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
    bool isParameterCollection() const;

    /** @brief Allows to assign a name to this parameter */
    void setParameterName(const std::string &);
    /** @brief Allows to retrieve the name of this parameter */
    std::string getParameterName() const;

    /** @brief Checks whether this object matches a given activity mode */
    bool amMatch(const activityMode &) const;
    /** @brief Returns true on the case of an activity mode mismatch */
    bool amMismatch(const activityMode &) const;

    /** @brief Checks whether this object matches a given activity mode and is modifiable */
    bool modifiableAmMatchOrHandover(const activityMode &) const;

    /***************************************************************************/
    /**
     * Allows to count parameters of a specific type. This function is a trap, needed to
     * catch attempts to use this function with unsupported types. Use the supplied
     * specializations instead.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     * @return The number of parameters of a given Type
     */
    template <typename par_type>
    std::size_t countParameters(
        activityMode /*am*/
    ) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::countParameters()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );

        // Make the compiler happy
        return static_cast<std::size_t>(0);
    }

    /***************************************************************************/
    /**
     * Allows to add all boundaries if parameters of a specific type to the vectors. This
     * function is a trap, needed to catch streamlining attempts with unsupported types.
     * Use the supplied specializations instead.
     *
     * @oaram l_bnd_vec The vector with lower boundaries of parameters
     * @oaram u_bnd_vec The vector with upper boundaries of parameters
     */
    template <typename par_type>
    void boundaries(
        std::vector<par_type> &l_bnd_vec,
        std::vector<par_type> &u_bnd_vec,
        activityMode /*am*/
    ) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::boundaries(std::vector<>&)" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Allows to add all parameters of a specific type to the vector. This function is a
     * trap, needed to catch streamlining attempts with unsupported types. Use the supplied
     * specializations instead.
     *
     * @oaram par_vec The vector to which the items should be added
     */
    template <typename par_type>
    void streamline(
        std::vector<par_type> &par_vec,
        activityMode /*am*/
    ) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::streamline(std::vector<par_type>&)" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Allows to add all parameters of a specific type to the map. This function is a
     * trap, needed to catch streamlining attempts with unsupported types. Use the supplied
     * specializations instead.
     *
     * @oaram par_vec The vector to which the items should be added
     */
    template <typename par_type>
    void streamline(
        std::map<std::string, std::vector<par_type>> &par_vec,
        activityMode /*am*/
    ) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::streamline(std::map<std::string, std::vec<par_type>>)"
            << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Allows to assign the parameters inside of a vector the corresponding parameter objects.
     * This function is a trap, needed to catch attempts to use this function with unsupported
     * types. Use the supplied specializations instead.
     *
     * @param par_vec The vector with the parameters to be assigned to the object
     * @param pos The position from which parameters will be taken (will be updated by the call)
     */
    template <typename par_type>
    void assignValueVector(
        const std::vector<par_type> &par_vec,
        std::size_t & /*pos*/
        ,
        activityMode /*am*/
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::assignValueVector()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Assigns values from a std::map<std::string, std::vector<par_type>> to the parameter
     *
     * @param parMao The map with the parameters to be assigned to the object
     */
    template <typename par_type>
    void assignValueVectors(
        const std::map<std::string, std::vector<par_type>> &par_map,
        activityMode /*am*/
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::assignValueVectors()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    template <typename par_type>
    void multiplyByRandom(
        const par_type & /*min*/
        ,
        const par_type & /*max*/
        ,
        activityMode /*am*/
        ,
        Gem::Hap::GRandomBase &
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::multiplyByRandom()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0, 1[
     */
    template <typename par_type>
    void multiplyByRandom(
        activityMode /*am*/
        ,
        Gem::Hap::GRandomBase &
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::multiplyByRandom()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    template <typename par_type>
    void multiplyBy(
        par_type /*val*/
        ,
        activityMode /*am*/
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::multiplyBy()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Initializes all parameters of a given type with a constant value
     */
    template <typename par_type>
    void fixedValueInit(
        par_type /*val*/
        ,
        activityMode /*am*/
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::fixedValueInit()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Adds the parameters of another GParameterSet object to this one
     */
    template <typename par_type>
    void add(
        const std::shared_ptr<GParameterBase> & /*p*/
        ,
        activityMode /*am*/
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::add()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/
    /**
     * Subtracts the parameters of another GParameterSet object from this one
     */
    template <typename par_type>
    void subtract(
        const std::shared_ptr<GParameterBase> & /*p*/
        ,
        activityMode /*am*/
    ) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterBase::subtract()" << '\n'
            << "Function called for unsupported type!" << '\n'
        );
    }

    /***************************************************************************/

    /** @brief Specifies that no random initialization should occur anymore */
    void blockRandomInitialization();
    /** @brief Makes random initialization possible */
    void allowRandomInitialization();
    /** @brief Checks whether initialization has been blocked */
    bool randomInitializationBlocked() const;

    /** @brief Convenience function so we do not need to always cast derived classes */
    virtual bool hasAdaptor() const;

    /** @brief Converts the local data to a boost::property_tree node */
    virtual void toPropertyTree(pt::ptree &, const std::string &) const = 0;

    /** @brief Lets the audience know whether this is a leaf or a branch object */
    virtual bool isLeaf() const;

    /***************************************************************************/
    /**
     * This function converts a GParameterBase std::shared_ptr to the target type.  Note that this
     * template will only be accessible to the compiler if GParameterBase is a base type of load_type.
     *
     * @param load_ptr A std::shared_ptr<load_type> to the item to be converted
     * @param dummy A dummy argument needed for std::enable_if and type_traits magic
     * @return A std::shared_ptr holding the converted object
     */
    template <typename load_type>
        requires std::derived_from<load_type, Gem::Geneva::Parameters::GParameterBase>
    std::shared_ptr<load_type> parameterbase_cast(std::shared_ptr<GParameterBase> load_ptr) const {
#ifdef DEBUG
        std::shared_ptr<load_type> p = std::dynamic_pointer_cast<load_type>(load_ptr);
        if(p) {
            return p;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In std::shared_ptr<load_type> GParameterBase::parameterbase_cast<load_type>() :"
                << '\n'
                << "Invalid conversion with load_type = " << typeid(load_type).name() << '\n'
            );
        }
#else
        return std::static_pointer_cast<load_type>(load_ptr);
#endif
    }

protected:
    /***************************************************************************/
    /** @brief Count the number of float parameters */
    virtual std::size_t countFloatParameters(const activityMode &am) const;

    /** @brief Count the number of double parameters */
    virtual std::size_t countDoubleParameters(const activityMode &am) const;

    /** @brief Count the number of std::int32_t parameters */
    virtual std::size_t countInt32Parameters(const activityMode &am) const;

    /** @brief Count the number of bool parameters */
    virtual std::size_t countBoolParameters(const activityMode &am) const;

    /** @brief Attach boundaries of type float to the vectors */
    virtual void
    floatBoundaries(std::vector<float> &, std::vector<float> &, const activityMode &) const;

    /** @brief Attach boundaries of type double to the vectors */
    virtual void
    doubleBoundaries(std::vector<double> &, std::vector<double> &, const activityMode &) const;

    /** @brief Attach boundaries of type std::int32_t to the vectors */
    virtual void int32Boundaries(
        std::vector<std::int32_t> &,
        std::vector<std::int32_t> &,
        const activityMode &
    ) const;

    /** @brief Attach boundaries of type bool to the vectors */
    virtual void
    booleanBoundaries(std::vector<bool> &, std::vector<bool> &, const activityMode &) const;

    /** @brief Attach parameters of type float to the vector */
    virtual void floatStreamline(std::vector<float> &, const activityMode &) const;

    /** @brief Attach parameters of type double to the vector */
    virtual void doubleStreamline(std::vector<double> &, const activityMode &) const;

    /** @brief Attach parameters of type std::int32_t to the vector */
    virtual void
    int32Streamline(std::vector<std::int32_t> &, const activityMode &) const;

    /** @brief Attach parameters of type bool to the vector */
    virtual void booleanStreamline(std::vector<bool> &, const activityMode &) const;

    /** @brief Attach parameters of type float to the map */
    virtual void
    floatStreamline(std::map<std::string, std::vector<float>> &, const activityMode &) const;

    /** @brief Attach parameters of type double to the map */
    virtual void
    doubleStreamline(std::map<std::string, std::vector<double>> &, const activityMode &) const;

    /** @brief Attach parameters of type std::int32_t to the map */
    virtual void
    int32Streamline(std::map<std::string, std::vector<std::int32_t>> &, const activityMode &) const;

    /** @brief Attach parameters of type bool to the map */
    virtual void
    booleanStreamline(std::map<std::string, std::vector<bool>> &, const activityMode &) const;

    /** @brief Assigns part of a value vector to the parameter */
    virtual void
    assignFloatValueVector(const std::vector<float> &, std::size_t &, const activityMode &);

    /** @brief Assigns part of a value vector to the parameter */
    virtual void
    assignDoubleValueVector(const std::vector<double> &, std::size_t &, const activityMode &);

    /** @brief Assigns part of a value vector to the parameter */
    virtual void
    assignInt32ValueVector(const std::vector<std::int32_t> &, std::size_t &, const activityMode &);

    /** @brief Assigns part of a value vector to the parameter */
    virtual void
    assignBooleanValueVector(const std::vector<bool> &, std::size_t &, const activityMode &);

    /** @brief Assigns part of a value vector to the parameter */
    virtual void assignFloatValueVectors(
        const std::map<std::string, std::vector<float>> &,
        const activityMode &
    );

    /** @brief Assigns part of a value vector to the parameter */
    virtual void assignDoubleValueVectors(
        const std::map<std::string, std::vector<double>> &,
        const activityMode &
    );

    /** @brief Assigns part of a value vector to the parameter */
    virtual void assignInt32ValueVectors(
        const std::map<std::string, std::vector<std::int32_t>> &,
        const activityMode &
    );

    /** @brief Assigns part of a value vector to the parameter */
    virtual void assignBooleanValueVectors(
        const std::map<std::string, std::vector<bool>> &,
        const activityMode &
    );

    /** @brief Multiplication with a random value in a given range */
    virtual void floatMultiplyByRandom(
        const float &min,
        const float &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &
    );

    /** @brief Multiplication with a random value in a given range */
    virtual void doubleMultiplyByRandom(
        const double &min,
        const double &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &
    );

    /** @brief Multiplication with a random value in a given range */
    virtual void int32MultiplyByRandom(
        const std::int32_t &min,
        const std::int32_t &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &
    );

    /** @brief Multiplication with a random value in a given range */
    virtual void booleanMultiplyByRandom(
        const bool &min,
        const bool &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &
    );

    /** @brief Multiplication with a random value in the range [0,1[ */
    virtual void
    floatMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &);

    /** @brief Multiplication with a random value in the range [0,1[ */
    virtual void
    doubleMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &);

    /** @brief Multiplication with a random value in the range [0,1[ */
    virtual void
    int32MultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &);

    /** @brief Multiplication with a random value in the range [0,1[ */
    virtual void
    booleanMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &);

    /** @brief Multiplication with a constant value */
    virtual void floatMultiplyBy(const float &value, const activityMode &am);

    /** @brief Multiplication with a constant value */
    virtual void doubleMultiplyBy(const double &value, const activityMode &am);

    /** @brief Multiplication with a constant value */
    virtual void int32MultiplyBy(const std::int32_t &value, const activityMode &am);

    /** @brief Multiplication with a const value */
    virtual void booleanMultiplyBy(const bool &value, const activityMode &am);

    /** @brief Initialization with a constant value */
    virtual void floatFixedValueInit(const float &value, const activityMode &am);

    /** @brief Initialization with a constant value */
    virtual void doubleFixedValueInit(const double &value, const activityMode &am);

    /** @brief Initialization with a constant value */
    virtual void
    int32FixedValueInit(const std::int32_t &value, const activityMode &am);

    /** @brief Initialization with a const value */
    virtual void booleanFixedValueInit(const bool &value, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void floatAdd(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void doubleAdd(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void int32Add(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void booleanAdd(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void
    floatSubtract(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void
    doubleSubtract(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void
    int32Subtract(std::shared_ptr<GParameterBase>, const activityMode &am);

    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    virtual void
    booleanSubtract(std::shared_ptr<GParameterBase>, const activityMode &am);

    /***************************************************************************/
    /** @brief Loads the data of another GObject */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterBase>(
        GParameterBase const &,
        GParameterBase const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Triggers random initialization of the parameter(-collection) */
    virtual bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &) = 0;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override = 0;

    /** @brief The actual adaption logic */
    virtual std::size_t adapt_(Gem::Hap::GRandomBase &) = 0;
    /** @brief Triggers updates when the optimization process has stalled */
    virtual bool updateAdaptorsOnStall_(std::size_t) = 0;

    /** @brief Retrieves information from an adaptor on a given property */
    virtual void queryAdaptor_(
        const std::string &adaptor_name,
        const std::string &property,
        std::vector<std::any> &data
    ) const = 0;

    /** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
    virtual bool isIndividualParameter_() const;

    /***************************************************************************/
    bool adaptionsActive_ =
        true; ///< Specifies whether adaptions of this object should be carried out
    bool randomInitializationBlocked_ =
        false; ///< Specifies that this object should not be initialized again
    std::string parameterName_ = Gem::Common::generate_uuid_v4(); ///< A name assigned to this parameter object
};

/******************************************************************************/
/**
 * Specializations of some template functions
 */

/******************************************************************************/
/**
 * Allows to add all parameters of type float to the vector.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<float>(std::vector<float> &par_vec, activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatStreamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type double to the vector.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void
GParameterBase::streamline<double>(std::vector<double> &par_vec, activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleStreamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type std::int32_t to the vector.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<std::int32_t>(
    std::vector<std::int32_t> &par_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32Streamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type bool to the vector.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<bool>(std::vector<bool> &par_vec, activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->booleanStreamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type float to the map.
 *
 * @oaram par_vec The map to which the items should be added
 */
template <>
inline void GParameterBase::streamline<float>(
    std::map<std::string, std::vector<float>> &par_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatStreamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type double to the map.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<double>(
    std::map<std::string, std::vector<double>> &par_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleStreamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type std::int32_t to the map.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<std::int32_t>(
    std::map<std::string, std::vector<std::int32_t>> &par_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32Streamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type bool to the map.
 *
 * @oaram par_vec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<bool>(
    std::map<std::string, std::vector<bool>> &par_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->booleanStreamline(par_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type float
 *
 * @param l_bnd_vec A vector of lower double parameter boundaries
 * @param u_bnd_vec A vector of upper double parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<float>(
    std::vector<float> &l_bnd_vec,
    std::vector<float> &u_bnd_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatBoundaries(l_bnd_vec, u_bnd_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type double
 *
 * @param l_bnd_vec A vector of lower double parameter boundaries
 * @param u_bnd_vec A vector of upper double parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<double>(
    std::vector<double> &l_bnd_vec,
    std::vector<double> &u_bnd_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleBoundaries(l_bnd_vec, u_bnd_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type std::int32_t
 *
 * @param l_bnd_vec A vector of lower std::int32_t parameter boundaries
 * @param u_bnd_vec A vector of upper std::int32_t parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<std::int32_t>(
    std::vector<std::int32_t> &l_bnd_vec,
    std::vector<std::int32_t> &u_bnd_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32Boundaries(l_bnd_vec, u_bnd_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type bool
 *
 * @param l_bnd_vec A vector of lower bool parameter boundaries
 * @param u_bnd_vec A vector of upper bool parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<bool>(
    std::vector<bool> &l_bnd_vec,
    std::vector<bool> &u_bnd_vec,
    activityMode am
) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->booleanBoundaries(l_bnd_vec, u_bnd_vec, am);
    }
}

/******************************************************************************/
/**
 * Allows to count parameters of type float.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type float
 */
template <>
inline std::size_t GParameterBase::countParameters<float>(activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        return this->countFloatParameters(am);
    }
    else {
        return 0;
    }
}

/******************************************************************************/
/**
 * Allows to count parameters of type double.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type double
 */
template <>
inline std::size_t GParameterBase::countParameters<double>(activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        return this->countDoubleParameters(am);
    }
    else {
        return 0;
    }
}

/******************************************************************************/
/**
 * Allows to count parameters of type std::int32_t.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type std::int32_t
 */
template <>
inline std::size_t GParameterBase::countParameters<std::int32_t>(activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        return this->countInt32Parameters(am);
    }
    else {
        return 0;
    }
}

/******************************************************************************/
/**
 * Allows to count parameters of type bool.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type bool
 */
template <>
inline std::size_t GParameterBase::countParameters<bool>(activityMode am) const {
    if(this->modifiableAmMatchOrHandover(am)) {
        return this->countBoolParameters(am);
    }
    else {
        return 0;
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param par_vec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<float>(
    const std::vector<float> &par_vec,
    std::size_t &pos,
    activityMode am
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->assignFloatValueVector(par_vec, pos, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param par_vec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<double>(
    const std::vector<double> &par_vec,
    std::size_t &pos,
    activityMode am
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->assignDoubleValueVector(par_vec, pos, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param par_vec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<std::int32_t>(
    const std::vector<std::int32_t> &par_vec,
    std::size_t &pos,
    activityMode am
) {
    this->assignInt32ValueVector(par_vec, pos, am);
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param par_vec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<bool>(
    const std::vector<bool> &par_vec,
    std::size_t &pos,
    activityMode am
) {
    this->assignBooleanValueVector(par_vec, pos, am);
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param par_map The vector with the parameters to be assigned to the object
 */
template <>
inline void GParameterBase::assignValueVectors<float>(
    const std::map<std::string, std::vector<float>> &par_map,
    activityMode am
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->assignFloatValueVectors(par_map, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param par_map The vector with the parameters to be assigned to the object
 */
template <>
inline void GParameterBase::assignValueVectors<double>(
    const std::map<std::string, std::vector<double>> &par_map,
    activityMode am
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->assignDoubleValueVectors(par_map, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param par_map The vector with the parameters to be assigned to the object
 */
template <>
inline void GParameterBase::assignValueVectors<std::int32_t>(
    const std::map<std::string, std::vector<std::int32_t>> &par_map,
    activityMode am
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->assignInt32ValueVectors(par_map, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param par_map The vector with the parameters to be assigned to the object
 */
template <>
inline void GParameterBase::assignValueVectors<bool>(
    const std::map<std::string, std::vector<bool>> &par_map,
    activityMode am
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->assignBooleanValueVectors(par_map, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
template <>
inline void GParameterBase::multiplyByRandom<float>(
    const float &min,
    const float &max,
    activityMode am,
    Gem::Hap::GRandomBase &gr
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatMultiplyByRandom(min, max, am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
template <>
inline void GParameterBase::multiplyByRandom<double>(
    const double &min,
    const double &max,
    activityMode am,
    Gem::Hap::GRandomBase &gr
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleMultiplyByRandom(min, max, am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
template <>
inline void GParameterBase::multiplyByRandom<std::int32_t>(
    const std::int32_t &min,
    const std::int32_t &max,
    activityMode am,
    Gem::Hap::GRandomBase &gr
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32MultiplyByRandom(min, max, am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range. This specialization for
 * boolean values has been added for completeness and error-detection. It will throw
 * when called.
 */
template <>
inline void GParameterBase::multiplyByRandom<bool>(
    const bool &min,
    const bool &max,
    activityMode am,
    Gem::Hap::GRandomBase &gr
) {
    if(this->modifiableAmMatchOrHandover(am)) {
        // NOTE: This will throw
        this->booleanMultiplyByRandom(min, max, am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
template <>
inline void GParameterBase::multiplyByRandom<float>(activityMode am, Gem::Hap::GRandomBase &gr) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatMultiplyByRandom(am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
template <>
inline void GParameterBase::multiplyByRandom<double>(activityMode am, Gem::Hap::GRandomBase &gr) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleMultiplyByRandom(am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
template <>
inline void
GParameterBase::multiplyByRandom<std::int32_t>(activityMode am, Gem::Hap::GRandomBase &gr) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32MultiplyByRandom(am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[. This specialization for
 * boolean values has been added for completeness and error-detection. It will throw
 * when called.
 */
template <>
inline void GParameterBase::multiplyByRandom<bool>(activityMode am, Gem::Hap::GRandomBase &gr) {
    if(this->modifiableAmMatchOrHandover(am)) {
        // NOTE: This will throw
        this->booleanMultiplyByRandom(am, gr);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
template <>
inline void GParameterBase::multiplyBy<float>(float val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatMultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
template <>
inline void GParameterBase::multiplyBy<double>(double val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleMultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
template <>
inline void GParameterBase::multiplyBy<std::int32_t>(std::int32_t val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32MultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value. This specialization for
 * boolean values has been added for completeness and error-detection.
 * It will throw when called.
 */
template <>
inline void GParameterBase::multiplyBy<bool>(bool val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        // NOTE: This will throw
        this->booleanMultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline void GParameterBase::fixedValueInit<float>(float val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatFixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline void GParameterBase::fixedValueInit<double>(double val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleFixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline void GParameterBase::fixedValueInit<std::int32_t>(std::int32_t val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32FixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline void GParameterBase::fixedValueInit<bool>(bool val, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->booleanFixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
template <>
inline void GParameterBase::add<float>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatAdd(p, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
template <>
inline void GParameterBase::add<double>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleAdd(p, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
template <>
inline void
GParameterBase::add<std::int32_t>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32Add(p, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one.
 * This specialization for boolean values has been added for completeness and error-detection.
 * It will throw when called.
 */
template <>
inline void GParameterBase::add<bool>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        // Note: This call will throw!
        this->booleanAdd(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
template <>
inline void
GParameterBase::subtract<float>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->floatSubtract(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
template <>
inline void
GParameterBase::subtract<double>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->doubleSubtract(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
template <>
inline void
GParameterBase::subtract<std::int32_t>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        this->int32Subtract(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one.
 * This specialization for boolean values has been added for completeness and error-detection.
 * It will throw when called.
 */
template <>
inline void
GParameterBase::subtract<bool>(const std::shared_ptr<GParameterBase> &p, activityMode am) {
    if(this->modifiableAmMatchOrHandover(am)) {
        // NOTE: This call will throw
        this->booleanSubtract(p, am);
    }
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Parameters::GParameterBase) // NOLINT
/******************************************************************************/
