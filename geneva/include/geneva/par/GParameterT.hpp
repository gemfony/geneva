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
#include <tuple>

// Boost header files go here

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GTypeToStringT.hpp"
#include "geneva/par/GParameterBaseWithAdaptorsT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * A class holding a single, mutable parameter - usually just an atomic value (double, long,
 * boolean, ...).
 */
template <typename T>
class GParameterT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterBaseWithAdaptorsT<T> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GParameterBaseWithAdaptors_T",
            boost::serialization::base_object<GParameterBaseWithAdaptorsT<T>>(*this)
        );
        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Used to identify the type supplied to this object */
    using p_type = T;

    /***************************************************************************/
    /** The default constructor */
    GParameterT() = default;

    /***************************************************************************/
    /**
	  * Initialization by contained value.
	  *
	  * @param val The new value of val_
	  */
    explicit GParameterT(const T &val)
      : GParameterBaseWithAdaptorsT<T>()
      , val_(val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The copy constructor.
	  *
	  * @param cp A copy of another GParameterT<T> object
	  */
    GParameterT(const GParameterT<T> &cp) = default;

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GParameterT() override = default;

    /***************************************************************************/
    /**
	  * An assignment operator that allows us to set val_ . Note that the value is returned as
	  * a copy, not a reference. Hence we assume here that val_ is copy-constructible.
	  *
	  * @param val The new value for val_
	  * @return The new value of val_
	  */
    virtual GParameterT<T> &operator=(const T &val) {
        setValue(val);
        return *this;
    }

    /***************************************************************************/
    /**
	  * Allows to set the internal (and usually externally visible) value. Note
	  * that we assume here that T has an operator=() or is a basic value type, such as double
	  * or int.
	  *
	  * @param val The new T value stored in this class
	  */
    virtual void setValue(const T &val) {
        val_ = val;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Automatic conversion to the target type
	  */
    operator T() const {
        return this->value();
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieval of the value
	  *
	  * @return The value of val_
	  */
    virtual T value() const {
        return val_;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

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
        ptr.put(base_name + ".baseType", Gem::Common::GTypeToStringT<T>::value());
        ptr.put(base_name + ".isLeaf", this->isLeaf());
        ptr.put(base_name + ".n_vals", 1);
        ptr.put(base_name + ".values.value0", this->value());
        ptr.put(base_name + ".initRandom", false); // Unused for the creation of a property tree
        ptr.put(base_name + ".adaptionsActive", this->adaptionsActive());
    }

    /***************************************************************************/
    /**
	  * Lets the audience know whether this is a leaf or a branch object
	  */
    bool isLeaf() const override {
        return true;
    }

protected:
    /***************************************************************************/
    /**
	  * Gives derived classes access to the internal value. A constant function is needed to
	  * allow resetting the value in the GConstrained family of classes from within the value()
	  * function (which by design should be constant). Still, users should be aware that generally
	  * setting of values is not a "const" action, so this function is protected.
	  *
	  * @param val The new T value stored in this class
	  */
    void setValue_(const T &val) const {
        val_ = val;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("val_", val_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("val_", val_)
        );
    }

    /***************************************************************************/
    /**
	  * Loads the data of another GParameterBase
	  *
	  * @param cp A copy of another GParameterT<T> object, camouflaged as a GParameterBase
	  */
    void load_(const GParameterBase *cp) override {
        // Check that we are dealing with a  GParameterT<T> reference independent of this object and convert the pointer
        const GParameterT<T> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GParameterT<T>>(cp, this);

        // Load our parent class'es data ...
        GParameterBaseWithAdaptorsT<T>::load_(cp);

        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterT<T>>(
        GParameterT<T> const &,
        GParameterT<T> const &,
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
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a  GParameterT<T> reference independent of this object and convert the pointer
        const GParameterT<T> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GParameterT<T>>(cp, this);

        GToken token("GParameterT<T>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterBaseWithAdaptorsT<T>>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    /** @brief Triggers random initialization of the parameter(-collection) */
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
        if(GParameterBaseWithAdaptorsT<T>::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GParameterT<>::modify_GUnitTests", "GEM_TESTING");
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
        GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests_();

        // All tests of our local functions are made in derived classes

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GParameterT<>::specificTestsNoFailureExpected_GUnitTests",
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
        GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GParameterT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
	  * The internal representation of our value. Mutability is needed as in some cases value
	  * calculation implies resetting of the internal value. We nevertheless want to be able
	  * to call the value() function from constant functions. Declared protected so some derived
	  * classes can (re-)set the value from a const function without forcing us to declare
	  * setValue() const.
	  *
	  * Thread-safety: this mutable member is written from logically-const paths
	  * (e.g. GConstrainedNumT<T>::value()) without synchronisation. That is safe
	  * because of a single-owner invariant: a given parameter object -- and the
	  * individual that owns it -- is only ever evaluated by one thread at a time.
	  * The broker deep-clones each work item per worker and dispatches every
	  * unique item to exactly one worker, so concurrent value() calls on the same
	  * object never occur. Do NOT share a single parameter/individual instance
	  * across threads without adding external synchronisation first.
	  */
    mutable T val_ = Gem::Common::GDefaultValueT<T>::value();

private:
    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GParameterT");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    GParameterBase *clone_() const override = 0;

    /***************************************************************************/
    /**
     * Allows to adapt the value stored in this class.
     *
     * @return The number of adaptions that were performed
     */
    std::size_t adapt_(Gem::Hap::GRandomBase &gr) override {
        return GParameterBaseWithAdaptorsT<T>::applyAdaptor(val_, this->range(), gr);
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename T>
struct is_abstract<Gem::Geneva::Parameters::GParameterT<T>> : public boost::true_type {};
template <typename T>
struct is_abstract<const Gem::Geneva::Parameters::GParameterT<T>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
