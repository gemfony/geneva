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
#include <tuple>
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "common/GTypeToStringT.hpp"
#include "geneva/par/GParameterT.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {

constexpr double DEFAULTLOWERINITBOUNDARYSINGLE = 0.;
constexpr double DEFAULTUPPERINITBOUNDARYSINGLE = 1.;

/******************************************************************************/
/**
 * This class represents numeric values. The most likely types to be stored
 * in this class are double and std::int32_t . By using the framework provided
 * by GParameterT, this class becomes rather simple.
 */
template <typename num_type>
    requires Gem::Common::arithmetic<num_type>
class GNumT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterT<num_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &make_nvp(
            "GParameterT",
            boost::serialization::base_object<GParameterT<num_type>>(*this)
        );
        // ... and then our local data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////


public:
    /** @brief Specifies the type of parameters stored in this collection */
    using collection_type = num_type;

    /***************************************************************************/
    /**
	  * The default constructor.
	  */
    GNumT() = default;

    /*****************************************************************/
    /*
	  * Initialize with a single value
	  *
	  * @param val The value used for the initialization
	  */
    explicit GNumT(const num_type &val)
      : GParameterT<num_type>(val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initialize the boundaries. The internal value will be
	  * initialized with the lower boundary.
	  *
	  * @param min The lower boundary for random entries
	  * @param max The upper boundary for random entries
	  */
    GNumT(const num_type &min, const num_type &max)
      : GParameterT<num_type>(min)
      , lower_init_boundary_(min)
      , upper_init_boundary_(max) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The standard copy constructor
	  */
    GNumT(const GNumT<num_type> &cp) = default;

    /***************************************************************************/
    /**
	  * The standard destructor
	  */
    ~GNumT() override = default;

    /***************************************************************************/
    /**
	  * An assignment operator for the contained value type
	  */
    GNumT<num_type> &operator=(const num_type &val) override {
        GParameterT<num_type>::operator=(val);
        return *this;
    }

    /***************************************************************************/
    /**
	  * Sets the initialization boundaries
	  *
	  * @param lower_init_boundary The lower boundary for random initialization
	  * @param upper_init_boundary The upper boundary for random initialization
	  */
    void
    setInitBoundaries(const num_type &lower_init_boundary, const num_type &upper_init_boundary) {
        // Do some error checking
        if(lower_init_boundary >= upper_init_boundary) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumT<T>::setInitBoundaries():" << '\n'
                << "Invalid boundaries provided: " << '\n'
                << "lower_init_boundary = " << lower_init_boundary << '\n'
                << "upper_init_boundary = " << upper_init_boundary << '\n'
            );
        }

        lower_init_boundary_ = lower_init_boundary;
        upper_init_boundary_ = upper_init_boundary;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested GNumT<T>::specificTestsNoFailureExpected_GUnitTests()
	  * Setting of invalid boundaries is tested in GNumT<T>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the value of the lower initialization boundary
	  *
	  * @return The value of the lower initialization boundary
	  */
    num_type getLowerInitBoundary() const {
        return lower_init_boundary_;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested GNumT<T>::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the value of the upper initialization boundary
	  *
	  * @return The value of the upper initialization boundary
	  */
    num_type getUpperInitBoundary() const {
        return upper_init_boundary_;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested GNumT<T>::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Lets the audience know whether this is a leaf or a branch object
	  */
    bool isLeaf() const override {
        return true;
    }

    /***************************************************************************/
    /**
	  * Converts the local data to a boost::property_tree node
	  *
	  * @param ptr The boost::property_tree object the data should be saved to
	  * @param base_name The name assigned to the object
	  */
    void toPropertyTree(pt::ptree &ptr, const std::string &base_name) const override {
        ptr.put(base_name + ".name", this->getParameterName());
        ptr.put(base_name + ".type", this->name());
        ptr.put(base_name + ".baseType", Gem::Common::GTypeToStringT<num_type>::value());
        ptr.put(base_name + ".isLeaf", this->isLeaf());
        ptr.put(base_name + ".n_vals", 1);
        ptr.put(base_name + ".values.value0", this->value());
        ptr.put(base_name + ".lowerBoundary", this->getLowerInitBoundary());
        ptr.put(base_name + ".upperBoundary", this->getUpperInitBoundary());
        ptr.put(base_name + ".initRandom", false); // Unused for the creation of a property tree
        ptr.put(base_name + ".adaptionsActive", this->adaptionsActive());
    }

protected:
    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("lower_init_boundary_", lower_init_boundary_),
            Gem::Common::make_member("upper_init_boundary_", upper_init_boundary_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("lower_init_boundary_", lower_init_boundary_),
            Gem::Common::make_member("upper_init_boundary_", upper_init_boundary_)
        );
    }

    /***************************************************************************/
    /**
	  * Loads the data of another GNumT<T> object,
	  * camouflaged as a GParameterBase. We have no local data, so
	  * all we need to do is to the standard identity check,
	  * preventing that an object is assigned to itself.
	  *
	  * @param cp A copy of another GNumT<T> object, camouflaged as a GParameterBase
	  */
    void load_(const GParameterBase *cp) override {
        // Check that we are dealing with a GNumT<T> reference independent of this object and convert the pointer
        const GNumT<num_type> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GNumT<num_type>>(cp, this);

        // Load our parent class'es data ...
        GParameterT<num_type>::load_(cp);

        // ... and then our local data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNumT<num_type>>(
        GNumT<num_type> const &,
        GNumT<num_type> const &,
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

        // Check that we are dealing with a GNumT<T> reference independent of this object and convert the pointer
        const GNumT<num_type> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GNumT<num_type>>(cp, this);

        GToken token("GNumT<T>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterT<num_type>>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * Returns a "comparative range". This is e.g. used to make Gauss-adaption
	  * independent of a parameters value range
	  */
    num_type range() const override {
        return upper_init_boundary_ - lower_init_boundary_;
    }

    /***************************************************************************/
    /** @brief Triggers random initialization of the parameter */
    bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &) override = 0;

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
        if(GParameterT<num_type>::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumT<>::modify_GUnitTests", "GEM_TESTING");
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
        GParameterT<num_type>::specificTestsNoFailureExpected_GUnitTests_();

        // A few settings
        const num_type lowertestinitval =
            num_type(1); // Do not choose a negative value as T might be an unsigned type
        const num_type uppertestinitval = num_type(3);

        //------------------------------------------------------------------------------

        { // Test setting and retrieval of initialization boundaries
            std::shared_ptr<GNumT<num_type>> p_test = this->template clone<GNumT<num_type>>();

            // Set the boundaries
            CHECK_NOTHROW(p_test->setInitBoundaries(lowertestinitval, uppertestinitval));

            // Check that these values have indeed been assigned
            CHECK(p_test->getLowerInitBoundary() == lowertestinitval);
            CHECK(p_test->getUpperInitBoundary() == uppertestinitval);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GNumT<>::specificTestsNoFailureExpected_GUnitTests",
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
        GParameterT<num_type>::specificTestsFailuresExpected_GUnitTests_();

        // A few settings
        const num_type lowertestinitval =
            num_type(1); // Do not choose a negative value as T might be an unsigned type
        const num_type uppertestinitval = num_type(3);

        //------------------------------------------------------------------------------

        { // Check that assignement of initialization boundaries throws for invalid boundaries
            std::shared_ptr<GNumT<num_type>> p_test = this->template clone<GNumT<num_type>>();

            CHECK_THROWS_AS(
                (p_test->setInitBoundaries(uppertestinitval, lowertestinitval)),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif                  /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GNumT<>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep copy of this object. Purely virtual as this class
	  * should not be instantiable.
	  *
	  * @return A pointer to a deep clone of this object
	  */
    GParameterBase *clone_() const override = 0;

    /***************************************************************************/
    num_type lower_init_boundary_ =
        num_type(DEFAULTLOWERINITBOUNDARYSINGLE); ///< The lower boundary for random initialization
    num_type upper_init_boundary_ =
        num_type(DEFAULTUPPERINITBOUNDARYSINGLE); ///< The upper boundary for random initialization
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename num_type>
struct is_abstract<Gem::Geneva::Parameters::GNumT<num_type>> : public boost::true_type {};
template <typename num_type>
struct is_abstract<const Gem::Geneva::Parameters::GNumT<num_type>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
