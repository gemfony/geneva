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

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GNumFlipAdaptorT.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * GIntFlipAdaptorT represents an adaptor used for the adaption of integer
 * types, by flipping an integer number to the next larger or smaller number.
 * The integer type used needs to be specified as a template parameter. Note
 * that a specialization of this class, as defined in GIntFlipAdaptorT.cpp,
 * allows to deal with booleans instead of "standard" integer types.
 */
template <typename int_type>
class GIntFlipAdaptorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GNumFlipAdaptorT<int_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &make_nvp(
            "GNumFlipAdaptorT",
            boost::serialization::base_object<GNumFlipAdaptorT<int_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The standard constructor.
     */
    GIntFlipAdaptorT() = default;

    /***************************************************************************/
    /**
     * This constructor takes an argument, that specifies the (initial) probability
     * for the adaption of an integer or bit value
     *
     * @param ad_prob The probability for a flip
     */
    explicit GIntFlipAdaptorT(const double &ad_prob)
      : GNumFlipAdaptorT<int_type>(ad_prob) { /* nothing */
    }

    /***************************************************************************/
    /**
     * A standard copy constructor.
     *
     * @param cp Another GIntFlipAdaptorT object
     */
    GIntFlipAdaptorT(const GIntFlipAdaptorT<int_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor. Empty, as we have no local, dynamically
     * allocated data.
     */
    ~GIntFlipAdaptorT() override = default;

protected:
    /***************************************************************************/
    /**
     * This function loads the data of another GIntFlipAdaptorT, camouflaged as a GAdaptorT.
     *
     * @param cp A copy of another GIntFlipAdaptorT, camouflaged as a GAdaptorT
     */
    void load_(const GAdaptorT<int_type> *cp) override {
        // Convert the pointer to our target type and check for self-assignment
        const GIntFlipAdaptorT<int_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<int_type>, GIntFlipAdaptorT<int_type>>(cp, this);

        // Load the data of our parent class ...
        GNumFlipAdaptorT<int_type>::load_(cp);

        // no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GIntFlipAdaptorT<int_type>>(
        GIntFlipAdaptorT<int_type> const &,
        GIntFlipAdaptorT<int_type> const &,
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
        const GAdaptorT<int_type> &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GIntFlipAdaptorT<int_type> reference independent of this object and convert the pointer
        const GIntFlipAdaptorT<int_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<int_type>, GIntFlipAdaptorT<int_type>>(cp, this);

        GToken token("GIntFlipAdaptorT<int_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GNumFlipAdaptorT<int_type>>(*this, *p_load, token);

        //... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

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
        if(GNumFlipAdaptorT<int_type>::modify_GUnitTests_()) {
            result = true;
        }

        // no local data -- nothing to change

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GIntFlipAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
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
        GNumFlipAdaptorT<int_type>::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GIntFlipAdaptorT<>::specificTestsNoFailureExpected_GUnitTests",
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
        GNumFlipAdaptorT<int_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GIntFlipAdaptorT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }
    /***************************************************************************/

private:
    /***************************************************************************/
    /**
     * Retrieves the id of the adaptor. Purely virtual, as we do not want this class to be
     * instantiated.
     *
     * @return The id of the adaptor
     */
    Gem::Geneva::adaptorId getAdaptorId_() const override = 0;

    /* ----------------------------------------------------------------------------------
     * Tested in GInt32FlipAdaptor::specificTestsNoFailuresExpected_GUnitTests()
     * Tested in GBooleanAdaptor::specificTestsNoFailuresExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GIntFlipAdaptorT");
    }

    /***************************************************************************/
    /**
     * This function creates a deep copy of this object. The function is purely virtual, as we
     * do not want this class to be instantiated. Use the derived classes instead.
     *
     * @return A deep copy of this object
     */
    GAdaptorT<int_type> *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename int_type>
struct is_abstract<Gem::Geneva::Parameters::GIntFlipAdaptorT<int_type>> : public boost::true_type {};
template <typename int_type>
struct is_abstract<const Gem::Geneva::Parameters::GIntFlipAdaptorT<int_type>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
