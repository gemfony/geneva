/**
 * @file
 */

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

// Standard headers go here
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "geneva/GNumBiGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GFPBiGaussAdaptorT is used for the adaption of numeric types, by the addition of random numbers
 * distributed as two adjacent gaussians. Different numeric types may be used, including Boost's
 * integer representations. The type used needs to be specified as a template parameter. In comparison
 * to GNumGaussAdaptorT, an additional parameter "delta" is added, which represents the distance between
 * both gaussians. Just like sigma, delta can be subject to mutations. It is also possible to use
 * two different sigma/sigmaSigma values and adaption rates for both gaussians. Note that this adaptor
 * is experimental. Your mileage may vary.
 */
template<typename fp_type>
class GFPBiGaussAdaptorT :
    public GNumBiGaussAdaptorT<fp_type, fp_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // Save all necessary data
        ar
        & make_nvp(
            "GAdaptorT_num"
            , boost::serialization::base_object<GNumBiGaussAdaptorT<fp_type, fp_type>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if fp_type really is a floating point type
    static_assert(
        std::is_floating_point<fp_type>::value
        , "fp_type should be a floating point type"
    );


public:
    /***************************************************************************/
    /**
     * The standard constructor.
     */
    GFPBiGaussAdaptorT() = default;

    /***************************************************************************/
    /**
     * Initialization of the parent class'es adaption probability.
     *
     * @param probability The likelihood for a adaption actually taking place
     */
    GFPBiGaussAdaptorT(const fp_type &probability)
        :
        GNumBiGaussAdaptorT<fp_type, fp_type>(probability)
    { /* nothing */ }

    /***************************************************************************/
    /**
     * A standard copy constructor
     *
     * @param cp Another GFPBiGaussAdaptorT object
     */
    GFPBiGaussAdaptorT(const GFPBiGaussAdaptorT<fp_type> &) = default;

    /***************************************************************************/
    /**
     * The standard destructor. Empty, as we have no local, dynamically
     * allocated data.
     */
    ~GFPBiGaussAdaptorT() override = default;

protected:
    /***************************************************************************/
    /**
     * This function loads the data of another GFPBiGaussAdaptorT, camouflaged as a GObject.
     * We assume that the values given to us by the other object are correct and do no error checks.
     *
     * @param A copy of another GFPBiGaussAdaptorT, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GFPBiGaussAdaptorT<fp_type> reference independent of this object and convert the pointer
        const GFPBiGaussAdaptorT<fp_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GFPBiGaussAdaptorT<fp_type>>(
            cp
            , this
        );

        // Load the data of our parent class ...
        GNumBiGaussAdaptorT<fp_type, fp_type>::load_(cp);

        // no local data ...
    }

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFPBiGaussAdaptorT<fp_type>>(
        GFPBiGaussAdaptorT<fp_type> const &
        , GFPBiGaussAdaptorT<fp_type> const &
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
        , const fp_type &limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GFPBiGaussAdaptorT<fp_type> reference independent of this object and convert the pointer
        const GFPBiGaussAdaptorT<fp_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GFPBiGaussAdaptorT<fp_type>>(
            cp
            , this
        );

        GToken token(
            "GFPBiGaussAdaptorT<fp_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GNumBiGaussAdaptorT<fp_type, fp_type>>(
            *this
            , *p_load
            , token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * The actual adaption of the supplied value takes place here
     *
     * @param value The value that is going to be adapted in situ
     * @param range A typical range for the parameter with type num_type
     */
    void customAdaptions(
        fp_type &value
        , const fp_type &range
        , Gem::Hap::GRandomBase &gr
    ) override {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        if (GNumBiGaussAdaptorT<fp_type, fp_type>::useSymmetricSigmas_) { // Should we use the same sigma for both gaussians ?
            // adapt the value in situ. Note that this changes
            // the argument of this function
            value
                += (
                range * GNumBiGaussAdaptorT<fp_type, fp_type>::m_bi_normal_distribution(
                    gr
                    , typename Gem::Hap::bi_normal_distribution<fp_type>::param_type(
                        fp_type(0.)
                        , GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_
                        , GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_ // Intended to be m_sigma1 (symmetry-case)
                        , GNumBiGaussAdaptorT<fp_type, fp_type>::delta_
                    )
                )
            );
        } else { // We allow asymmetric sigmas, i.e. different widths of both gaussians
            // adapt the value in situ. Note that this changes
            // the argument of this function
            value
                += (
                range * GNumBiGaussAdaptorT<fp_type, fp_type>::m_bi_normal_distribution(
                    gr
                    , typename Gem::Hap::bi_normal_distribution<fp_type>::param_type(
                        fp_type(0.)
                        , GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_
                        , GNumBiGaussAdaptorT<fp_type, fp_type>::sigma2_
                        , GNumBiGaussAdaptorT<fp_type, fp_type>::delta_
                    )
                )
            );
        }
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        bool result = false;

        // Call the parent classes' functions
        if (GNumBiGaussAdaptorT<fp_type, fp_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPBiGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        // Call the parent classes' functions
        GNumBiGaussAdaptorT<fp_type, fp_type>::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPBiGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        // Call the parent classes' functions
        GNumBiGaussAdaptorT<fp_type, fp_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPBiGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

private:
    /***********************************************************************************/
    /** @brief Retrieves the id of the adaptor */
    Gem::Geneva::adaptorId getAdaptorId_() const override = 0;

    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GFPBiGaussAdaptorT");
    }

    /***************************************************************************/
    /** @brief This function creates a deep copy of this object */
    GObject *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost {
namespace serialization {
template<typename fp_type>
struct is_abstract<Gem::Geneva::GFPBiGaussAdaptorT<fp_type>> :
    public boost::true_type
{
};
template<typename fp_type>
struct is_abstract<const Gem::Geneva::GFPBiGaussAdaptorT<fp_type>> :
    public boost::true_type
{
};
}
}

/******************************************************************************/

