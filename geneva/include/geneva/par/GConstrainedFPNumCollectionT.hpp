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
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "geneva/par/GConstrainedNumCollectionT.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * This class represents a collection of floating point values with common
 * boundaries, all modified using the same algorithm. The most likely types to
 * be stored in this class are double values. Note: If you want
 * to access or set the transformed value, use the value() and setValue()
 * functions. Using the subscript operator or at() function, or the
 * native iterator, will give you the "raw" data only.
 */
template <typename fp_type>
    requires std::floating_point<fp_type>
class GConstrainedFPNumCollectionT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GConstrainedNumCollectionT<fp_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &make_nvp(
            "GConstrainedNumCollectionT",
            boost::serialization::base_object<GConstrainedNumCollectionT<fp_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////


public:
    /** @brief Specifies the type of parameters stored in this collection */
    using collection_type = fp_type;

    /***************************************************************************/
    /**
     * Initialize the lower and upper boundaries for data members of this class.
     * Then set all positions to random values.
     *
     * @param size The desired size of the collection
     * @param lower_boundary The lower boundary for data members
     * @param upper_boundary The upper boundary for data members
     */
    GConstrainedFPNumCollectionT(
        const std::size_t &size,
        const fp_type &lower_boundary,
        const fp_type &upper_boundary
    )
      : GConstrainedNumCollectionT<fp_type>(
            size,
            lower_boundary,
            std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
        ) // Note that we define the upper boundary as "open"
    {
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
        typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
            lower_boundary,
            upper_boundary
        );

        // Assign random values to each position
        typename GConstrainedFPNumCollectionT<fp_type>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            *it = uniform_real_distribution(gr);
        }
    }

    /***************************************************************************/
    /**
     * Initialize the lower and upper boundaries for data members of this class.
     * Set all positions to the same value. Note that we take the liberty to adapt val,
     * if it is equal to the unmodified upper boundary. Otherwise you will get an
     * error, where what you likely really meant was to start with the
     * upper boundary.
     *
     * @param size The desired size of the collection
     * @param val The value to be assigned to all positions
     * @param lower_boundary The lower boundary for data members
     * @param upper_boundary The upper boundary for data members
     */
    GConstrainedFPNumCollectionT(
        const std::size_t &size,
        const fp_type &val,
        const fp_type &lower_boundary,
        const fp_type &upper_boundary
    )
      : GConstrainedNumCollectionT<fp_type>(
            size,
            (val == upper_boundary ? std::nextafter(val, -std::numeric_limits<fp_type>::infinity())
                                   : val),
            lower_boundary,
            std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
        ) // Note that we define the upper boundary as "open"
    {     /* nothing */
    }

    /***************************************************************************/
    /**
     * The standard copy constructor
     */
    GConstrainedFPNumCollectionT(const GConstrainedFPNumCollectionT<fp_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GConstrainedFPNumCollectionT() override = default;

    /****************************************************************************/
    /**
     * The transfer function needed to calculate the externally visible value.
     *
     * @param val The value to which the transformation should be applied
     * @return The transformed value
     */
    fp_type transfer(const fp_type &val) const override {
        fp_type lower_boundary = GConstrainedNumCollectionT<fp_type>::getLowerBoundary();
        fp_type upper_boundary = GConstrainedNumCollectionT<fp_type>::getUpperBoundary();

        if(val >= lower_boundary && val < upper_boundary) {
            return val;
        }
        else {
            // Find out which region the value is in (compare figure transferFunction.pdf
            // that should have been delivered with this software). Note that Gem::Common::narrow<>
            // may throw - exceptions must be caught in surrounding functions.
            std::int32_t region = 0;

#ifdef DEBUG
            region = Gem::Common::narrow<std::int32_t>(std::floor(
                (fp_type(val) - fp_type(lower_boundary)) /
                (fp_type(upper_boundary) - fp_type(lower_boundary))
            ));
#else
            region = static_cast<std::int32_t>(std::floor(
                (fp_type(val) - fp_type(lower_boundary)) /
                (fp_type(upper_boundary) - fp_type(lower_boundary))
            ));
#endif

            // Check whether we are in an odd or an even range and calculate the
            // external value accordingly
            fp_type mapping = fp_type(0.);
            if(region % 2 ==
               0) { // can it be divided by 2 ? Region 0,2,... or a negative even range
                mapping = val - fp_type(region) * (upper_boundary - lower_boundary);
            }
            else { // Range 1,3,... or a negative odd range
                mapping = -val + (fp_type(region - 1) * (upper_boundary - lower_boundary) +
                                  2 * upper_boundary);
            }

            return mapping;
        }

        // Make the compiler happy
        return fp_type(0.);
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GConstrainedFPNumCollectionT<fp_type> object,
     * camouflaged as a GParameterBase. We have no local data, so
     * all we need to do is to the standard identity check,
     * preventing that an object is assigned to itself.
     *
     * @param cp A copy of another GConstrainedFPNumCollectionT<fp_type> object, camouflaged as a GParameterBase
     */
    void load_(const GParameterBase *cp) override {
        // Check that we are dealing with a GConstrainedFPNumCollectionT<fp_type>  reference independent of this object and convert the pointer
        const GConstrainedFPNumCollectionT<fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedFPNumCollectionT<fp_type>>(
                cp,
                this
            );

        // Load our parent class'es data ...
        GConstrainedNumCollectionT<fp_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedFPNumCollectionT<fp_type>>(
        GConstrainedFPNumCollectionT<fp_type> const &,
        GConstrainedFPNumCollectionT<fp_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GParameterBase object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GParameterBase &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GConstrainedFPNumCollectionT<fp_type>  reference independent of this object and convert the pointer
        const GConstrainedFPNumCollectionT<fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedFPNumCollectionT<fp_type>>(
                cp,
                this
            );

        GToken token("GConstrainedNumCollectionT<fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GConstrainedNumCollectionT<fp_type>>(*this, *p_load, token);

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Triggers random initialization of the parameter collection
     */
    bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &gr) override {
        typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
            GConstrainedNumCollectionT<fp_type>::getLowerBoundary(),
            GConstrainedNumCollectionT<fp_type>::getUpperBoundary()
        );
        for(std::size_t pos = 0; pos < this->size(); pos++) {
            this->setValue(pos, uniform_real_distribution(gr));
        }

        return true;
    }

    /***************************************************************************/
    /**
     * The default constructor. Intentionally protected, as it is only
     * needed for de-serialization and as the basis for derived class'es
     * default constructors.
     */
    GConstrainedFPNumCollectionT() = default;

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
        if(GConstrainedNumCollectionT<fp_type>::modify_GUnitTests_()) {
            result = true;
        }

        return result;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GConstrainedFPNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GConstrainedNumCollectionT<fp_type>::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GConstrainedFPNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GConstrainedNumCollectionT<fp_type>::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GConstrainedFPNumCollectionT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GConstrainedFPNumCollectionT");
    }
    /***************************************************************************/
    /** @brief Creates a deep copy of this object */
    GParameterBase *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename fp_type>
struct is_abstract<Gem::Geneva::Parameters::GConstrainedFPNumCollectionT<fp_type>> : public boost::true_type {};
template <typename fp_type>
struct is_abstract<const Gem::Geneva::Parameters::GConstrainedFPNumCollectionT<fp_type>>
  : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
