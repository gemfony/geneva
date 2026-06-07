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
#include <cmath>
#include <limits>
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "geneva/par/GNumGaussAdaptorT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The GFPGaussAdaptorT represents an adaptor used for the adaption of
 * floating point values through the addition of gaussian-distributed random numbers.
 * See the documentation of GNumGaussAdaptorT<T> for further information on adaptors
 * in the Geneva context. This class is at the core of evolutionary strategies,
 * as implemented by this library. It is now implemented through a generic
 * base class that can also be used to adapt other numeric types.
 */
template <typename adaption_fp_type>
class GFPGaussAdaptorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GNumGaussAdaptorT_fp_type",
            boost::serialization::base_object<GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if adaption_fp_type really is a floating point type
    static_assert(
        std::is_floating_point_v<adaption_fp_type>,
        "adaption_fp_type should be a floating point type"
    );

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GFPGaussAdaptorT() = default;

    /***************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GFPGaussAdaptorT object
     */
    GFPGaussAdaptorT(const GFPGaussAdaptorT<adaption_fp_type> &cp) = default;

    /***************************************************************************/
    /**
     * Initialization with a adaption probability
     *
     * @param ad_prob The adaption probability
     */
    explicit GFPGaussAdaptorT(const double &ad_prob)
      : GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>(ad_prob) { /* nothing */
    }

    /***************************************************************************/
    /**
     * Initialization with a number of values belonging to
     * the width of the gaussian.
     *
     * @param sigma The initial value for the sigma_ parameter
     * @param sigma_sigma The initial value for the sigmaSigma_ parameter
     * @param min_sigma The minimal value allowed for sigma_
     * @param max_sigma The maximal value allowed for sigma_
     */
    GFPGaussAdaptorT(
        const adaption_fp_type &sigma,
        const adaption_fp_type &sigma_sigma,
        const adaption_fp_type &min_sigma,
        const adaption_fp_type &max_sigma
    )
      : GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>(
            sigma,
            sigma_sigma,
            min_sigma,
            max_sigma
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
     * This constructor lets a user set all sigma parameters, as well as the adaption
     * probability in one go.
     *
     * @param sigma The initial value for the sigma_ parameter
     * @param sigma_sigma The initial value for the sigmaSigma_ parameter
     * @param min_sigma The minimal value allowed for sigma_
     * @param max_sigma The maximal value allowed for sigma_
     * @param ad_prob The adaption probability
     */
    GFPGaussAdaptorT(
        const adaption_fp_type &sigma,
        const adaption_fp_type &sigma_sigma,
        const adaption_fp_type &min_sigma,
        const adaption_fp_type &max_sigma,
        const double &ad_prob
    )
      : GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>(
            sigma,
            sigma_sigma,
            min_sigma,
            max_sigma,
            ad_prob
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GFPGaussAdaptorT() override = default;

protected:
    /***************************************************************************/
    /**
     * Loads the data of another object of this type
     *
     * @param cp A copy of another GFPGaussAdaptorT<adaption_fp_type> object, camouflaged as a GAdaptorT
     */
    void load_(const GAdaptorT<adaption_fp_type, adaption_fp_type> *cp) override {
        // Convert the pointer to our target type and check for self-assignment
        const GFPGaussAdaptorT<adaption_fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<adaption_fp_type, adaption_fp_type>, GFPGaussAdaptorT<adaption_fp_type>>(cp, this);

        // Load our parent class'es data ...
        GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFPGaussAdaptorT<adaption_fp_type>>(
        GFPGaussAdaptorT<adaption_fp_type> const &,
        GFPGaussAdaptorT<adaption_fp_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GAdaptorT object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GAdaptorT<adaption_fp_type, adaption_fp_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GFPGaussAdaptorT<adaption_fp_type> reference independent of this object and convert the pointer
        const GFPGaussAdaptorT<adaption_fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<adaption_fp_type, adaption_fp_type>, GFPGaussAdaptorT<adaption_fp_type>>(cp, this);

        GToken token("GFPGaussAdaptorT<adaption_fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>>(*this, *p_load, token);

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * The actual adaption of the supplied value takes place here.
     */
    void customAdaptions(adaption_fp_type &value, const adaption_fp_type &range, Gem::Hap::GRandomBase &gr) override {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        // adapt the value in situ. Note that this changes the argument of this function
        const adaption_fp_type before = value;
        const adaption_fp_type delta =
            range * GAdaptorT<adaption_fp_type, adaption_fp_type>::normal_distribution_(
                        gr,
                        typename std::normal_distribution<adaption_fp_type>::param_type(
                            0.,
                            GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>::sigma_
                        )
                    );
        value = before + delta;

        // Guarantee an observable change: at low precision (e.g. float) and large |value|, a small
        // gaussian step can fall below half a ULP and round away, leaving the value unchanged. Mirror
        // the integer adaptor's minimal-change guarantee by nudging one ULP in the step's direction,
        // so an adaption that fires always actually adapts. For double in normal ranges this never
        // triggers (the step is always representable).
        if(value == before) {
            const adaption_fp_type dir = (delta < adaption_fp_type(0))
                ? std::numeric_limits<adaption_fp_type>::lowest()
                : std::numeric_limits<adaption_fp_type>::max();
            value = std::nextafter(before, dir);
        }
    }

    /* ----------------------------------------------------------------------------------
     * - Tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        bool result = false;

        // Call the parent class'es function
        if(GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for
     * testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent class'es function
        GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GFPGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests",
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

        // Call the parent class'es function
        GNumGaussAdaptorT<adaption_fp_type, adaption_fp_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GFPGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /** @brief Retrieves the id of this adaptor */
    Gem::Geneva::adaptorId getAdaptorId_() const override = 0;

    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GFPGaussAdaptorT");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object. */
    GAdaptorT<adaption_fp_type, adaption_fp_type> *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename adaption_fp_type>
struct is_abstract<Gem::Geneva::Parameters::GFPGaussAdaptorT<adaption_fp_type>> : public boost::true_type {};
template <typename adaption_fp_type>
struct is_abstract<const Gem::Geneva::Parameters::GFPGaussAdaptorT<adaption_fp_type>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
