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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GTypeToStringT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterCollectionT.hpp"
#include "geneva/GConstrainedValueLimitT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents a collection of numeric values with common
 * boundaries, all modified using the same algorithm. The most likely types to
 * be stored in this class are double and std::int32_t . Note: If you want
 * to access or set the transformed value, use the value() and setValue()
 * functions. Using the subscript operator or at() function, or the
 * native iterator, will give you the "raw" data only.
 */
template<typename num_type>
class GConstrainedNumCollectionT
    : public GParameterCollectionT<num_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & make_nvp(
            "GParameterCollectionT"
            , boost::serialization::base_object<GParameterCollectionT<num_type>>(*this))
        & BOOST_SERIALIZATION_NVP(m_lowerBoundary)
        & BOOST_SERIALIZATION_NVP(m_upperBoundary);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Specifies the type of parameters stored in this collection */
    using collection_type = num_type;

    /***************************************************************************/
    /**
     * Initialize the lower and upper boundaries for data members of this class
     *
     * @param size The desired size of the vector
     * @param lowerBoundary The lower boundary of the value range
     * @param upperBoundary The upper boundary of the value range
     */
    GConstrainedNumCollectionT(
        const std::size_t size
        , const num_type &lowerBoundary
        , const num_type &upperBoundary
    )
        :
        GParameterCollectionT<num_type>(
            size
            , lowerBoundary
        )
        , m_lowerBoundary(lowerBoundary)
        , m_upperBoundary(upperBoundary) {
        // Naturally the upper boundary should be >= the lower boundary
        if (m_lowerBoundary > m_upperBoundary) {
            glogger
                << "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, lower,upper):"
                << std::endl
                << "lowerBoundary_ = " << m_lowerBoundary << "is larger than" << std::endl
                << "upperBoundary_ = " << m_upperBoundary << std::endl
                << GTERMINATION;
        }

        // We might have constraints regarding the allowed boundaries. Cross-check
        if (lowerBoundary < GConstrainedValueLimitT<num_type>::lowest() ||
            upperBoundary > GConstrainedValueLimitT<num_type>::highest()) {
            glogger
                << "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, lower,upper):"
                << std::endl
                << "lower and/or upper limit outside of allowed value range:" << std::endl
                << "lowerBoundary = " << lowerBoundary << std::endl
                << "upperBoundary = " << upperBoundary << std::endl
                << "GConstrainedValueLimit<num_type>::lowest() = " << GConstrainedValueLimitT<num_type>::lowest()
                << std::endl
                << "GConstrainedValueLimit<num_type>::highest() = " << GConstrainedValueLimitT<num_type>::highest()
                << GTERMINATION;
        }
    }

    /***************************************************************************/
    /**
     * Initialize the lower and upper boundaries for data members of this class
     * and assign a fixed value to each position
     *
     * @param size The desired size of the vector
     * @param val The value to be assigned to each position
     * @param lowerBoundary The lower boundary of the value range
     * @param upperBoundary The upper boundary of the value range
     */
    GConstrainedNumCollectionT(
        const std::size_t size
        , const num_type &val
        , const num_type &lowerBoundary
        , const num_type &upperBoundary
    )
        :
        GParameterCollectionT<num_type>(
            size
            , val
        )
        , m_lowerBoundary(lowerBoundary)
        , m_upperBoundary(upperBoundary) {
        // Naturally the upper boundary should be > the lower boundary
        if (m_lowerBoundary > m_upperBoundary) {
            glogger
                << "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, val, lower,upper):"
                << std::endl
                << "lowerBoundary_ = " << m_lowerBoundary << "is larger than" << std::endl
                << "upperBoundary_ = " << m_upperBoundary << std::endl
                << GTERMINATION;
        }

        // We might have constraints regarding the allowed boundaries. Cross-check
        if (lowerBoundary < GConstrainedValueLimitT<num_type>::lowest() ||
            upperBoundary > GConstrainedValueLimitT<num_type>::highest()) {
            glogger
                << "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, val, lower,upper):"
                << std::endl
                << "lower and/or upper limit outside of allowed value range:" << std::endl
                << "lowerBoundary = " << lowerBoundary << std::endl
                << "upperBoundary = " << upperBoundary << std::endl
                << "GConstrainedValueLimit<num_type>::lowest() = " << GConstrainedValueLimitT<num_type>::lowest()
                << std::endl
                << "GConstrainedValueLimit<num_type>::highest() = " << GConstrainedValueLimitT<num_type>::highest()
                << std::endl
                << GTERMINATION;
        }

        // Check that assigned value is in the allowed range
        if (val < lowerBoundary || val > upperBoundary) {
            glogger
                << "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, val, lower,upper):"
                << std::endl
                << "Assigned value is outside of allowed value range:" << std::endl
                << "val = " << val << std::endl
                << "lowerBoundary = " << lowerBoundary << std::endl
                << "upperBoundary = " << upperBoundary << std::endl
                << GTERMINATION;
        }
    }

    /***************************************************************************/
    /**
     * The standard copy constructor. We assume that the boundaries have
     * "legal" values. Thus we do not make any error checks here.
     */
    GConstrainedNumCollectionT(const GConstrainedNumCollectionT<num_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GConstrainedNumCollectionT() override = default;

    /***************************************************************************/
    /**
       * Retrieves the lower boundary
       *
       * @return The value of the lower boundary
       */
    num_type getLowerBoundary() const {
        return m_lowerBoundary;
    }

    /***************************************************************************/
    /**
       * Retrieves the upper boundary
       *
       * @return The value of the upper boundary
       */
    num_type getUpperBoundary() const {
        return m_upperBoundary;
    }

    /***************************************************************************/
    /**
     * Resets the boundaries to the maximum allowed value.
     */
    void resetBoundaries() {
        this->setBoundaries(
            GConstrainedValueLimitT<num_type>::lowest()
            , GConstrainedValueLimitT<num_type>::highest());
    }

    /***************************************************************************/
    /**
     * Sets the boundaries of this object and does corresponding
     * error checks. If the current value is below or above the new
     * boundaries, this function will throw. Set the external value to
     * a new value between the new boundaries before calling this
     * function, or use the corresponding "setValue()" overload, which
     * also allows setting of boundaries.
     *
     * @param lower The new lower boundary for this object
     * @param upper The new upper boundary for this object
     */
    virtual void setBoundaries(const num_type &lower, const num_type &upper) BASE {
        std::vector<num_type> currentValues;
        for (std::size_t pos = 0; pos < this->size(); pos++) {
            currentValues.push_back(GParameterCollectionT<num_type>::value(pos));

            // Check that the value is inside the allowed range
            if (currentValues[pos] < lower || currentValues[pos] > upper) {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GConstrainedNumT<num_type>::setBoundaries(const T&, const T&) :" << std::endl
                        << "with typeid(num_type).name() = " << typeid(num_type).name() << std::endl
                        << "Attempt to set new boundaries [" << lower << ":" << upper << "]" << std::endl
                        << "with existing value  " << currentValues[pos] << " at position " << pos
                        << " outside of this range." << std::endl
                );
            }
        }

        // Check that the boundaries make sense
        if (lower > upper) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GConstrainedNumT<num_type>::setBoundaries(const num_type&, const num_type&)" << std::endl
                    << "with typeid(num_type).name() = " << typeid(num_type).name() << " :" << std::endl
                    << "Lower and/or upper boundary has invalid value : " << lower << " " << upper << std::endl
            );
        }

        m_lowerBoundary = lower;
        m_upperBoundary = upper;

        // Re-set the internal representation of the values -- we might be in a different
        // region of the transformation internally, and the mapping will likely depend on
        // the boundaries.
        for (std::size_t pos = 0; pos < this->size(); pos++) {
            GParameterCollectionT<num_type>::setValue(
                pos
                , currentValues.at(pos));
        }
    }

    /***************************************************************************/
    /**
     * Allows to set the value in a given position. This function will throw if val is
     * not in the currently assigned value range. Use the corresponding overload if
     * you want to set the value together with its boundaries instead.
     *
     * @param pos The position of the parameter to be set
     * @param val The new num_type value stored in this class
     */
    void setValue(const std::size_t &pos, const num_type &val) override {
        // Do some error checking
        if (val < m_lowerBoundary || val > m_upperBoundary) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GConstrainedNumCollectionT<num_type>::setValue(pos, val):" << std::endl
                    << "In position " << pos << ":" << std::endl
                    << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
                    << "lowerBoundary_ = " << m_lowerBoundary << std::endl
                    << "upperBoundary_ = " << m_upperBoundary << std::endl
            );
        }

        // O.k., assign value
        GParameterCollectionT<num_type>::setValue(
            pos
            , val
        );
    }

    /***************************************************************************/
    /**
     * Retrieval of the value at a given position. This is an overloaded version
     * of the original GParameterCollectionT<num_type>::value(pos) function which
     * applies a transformation, to be defined in derived classes.
     *
     * @param pos The position for which the transformed value needs to be returned
     * @return The transformed value of val_
     */
    num_type value(const std::size_t &pos) override {
        num_type mapping = transfer(GParameterCollectionT<num_type>::value(pos));

        // Reset internal value
        GParameterCollectionT<num_type>::setValue(
            pos
            , mapping
        );

        return mapping;
    }

    /***************************************************************************/
    /** @brief The transfer function needed to calculate the externally visible
     * value. Declared public so we can do tests of the value transformation. */
    virtual num_type transfer(const num_type &) const BASE = 0;

    /***************************************************************************/
    /**
     * Converts the local data to a boost::property_tree node
     *
     * @param ptr The boost::property_tree object the data should be saved to
     * @param id The id assigned to this object
     */
    void toPropertyTree(
        pt::ptree &ptr
        , const std::string &baseName
    ) const override {
#ifdef DEBUG
        // Check that the object isn't empty
        if (this->empty()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GConstrainedNumCollectionT<num_type>::toPropertyTree(): Error!" << std::endl
                    << "Object is empty!" << std::endl
            );
        }
#endif /* DEBUG */

        ptr.put(
            baseName + ".name"
            , this->getParameterName());
        ptr.put(
            baseName + ".type"
            , this->name());
        ptr.put(
            baseName + ".baseType"
            , Gem::Common::GTypeToStringT<num_type>::value());
        ptr.put(
            baseName + ".isLeaf"
            , this->isLeaf());
        ptr.put(
            baseName + ".nVals"
            , this->size());

        typename GConstrainedNumCollectionT<num_type>::const_iterator cit;
        std::size_t pos;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            pos = cit - this->begin();
            ptr.put(
                baseName + "values.value" + Gem::Common::to_string(pos)
                , *cit
            );
        }
        ptr.put(
            baseName + ".lowerBoundary"
            , this->getLowerBoundary());
        ptr.put(
            baseName + ".upperBoundary"
            , this->getUpperBoundary());
        ptr.put(
            baseName + ".initRandom"
            , false
        ); // Unused for the creation of a property tree
        ptr.put(
            baseName + ".adaptionsActive"
            , this->adaptionsActive());
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GConstrainedNumCollectionT<num_type> object,
     * camouflaged as a GObject. We have no local data, so
     * all we need to do is to the standard identity check,
     * preventing that an object is assigned to itself.
     *
     * @param cp A copy of another GConstrainedNumCollectionT<num_type> object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GConstrainedNumCollectionT<num_type> reference independent of this object and convert the pointer
        const GConstrainedNumCollectionT<num_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedNumCollectionT<num_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GParameterCollectionT<num_type>::load_(cp);

        // ... and then our local data
        m_lowerBoundary = p_load->m_lowerBoundary;
        m_upperBoundary = p_load->m_upperBoundary;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedNumCollectionT<num_type>>(
        GConstrainedNumCollectionT<num_type> const &
        , GConstrainedNumCollectionT<num_type> const &
        , Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const GObject &cp
        , const Gem::Common::expectation &e
        , const double &limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GConstrainedNumCollectionT<num_type> reference independent of this object and convert the pointer
        const GConstrainedNumCollectionT<num_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedNumCollectionT<num_type>>(
            cp
            , this
        );

        GToken token(
            "GConstrainedNumCollectionT<num_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterCollectionT<num_type>>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(m_lowerBoundary
                     , p_load->m_lowerBoundary)
            , token
        );
        compare_t(
            IDENTITY(m_upperBoundary
                     , p_load->m_upperBoundary)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Returns a "comparative range". This is e.g. used to make Gauss-adaption
     * independent of a parameters value range
     */
    num_type range() const override {
        return m_upperBoundary - m_lowerBoundary;
    }

    /***************************************************************************/
    /** @brief Triggers random initialization of the parameter collection */
    bool randomInit_(
        const activityMode &
        , Gem::Hap::GRandomBase &gr
    ) override = 0;

    /***************************************************************************/
    /**
     * The default constructor. Intentionally protected -- only needed
     * for de-serialization and as the basis for derived class'es default
     * constructors
     */
    GConstrainedNumCollectionT() = default;

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        bool result = false;

        // Call the parent classes' functions
        if (GParameterCollectionT<num_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GConstrainedNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GConstrainedNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterCollectionT<num_type>::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GConstrainedNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GConstrainedNumCollectionT");
    }

    /***************************************************************************/
    /**
     * Creates a deep copy of this object. Purely virtual as this class
     * should not be instantiable.
     *
     * @return A pointer to a deep clone of this object
     */
    GObject *clone_() const override = 0;

    /***************************************************************************/
    num_type m_lowerBoundary = num_type(0); ///< The lower allowed boundary for our value
    num_type m_upperBoundary = num_type(1); ///< The upper allowed boundary for our value
};

/******************************************************************************/
/**
 * Returns a "comparative range". Specialization for T==bool;
 */
template<>
inline bool GConstrainedNumCollectionT<bool>::range() const {
    return true;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename num_type>
struct is_abstract<Gem::Geneva::GConstrainedNumCollectionT<num_type>> :
    public boost::true_type
{
};
template<typename num_type>
struct is_abstract<const Gem::Geneva::GConstrainedNumCollectionT<num_type>> :
    public boost::true_type
{
};
}
}
/******************************************************************************/

