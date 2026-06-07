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
#include <concepts>

// Boost header files go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GConstrainedValueLimitT.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * This is a template-ized version of the GParameterBase class. Its main
 * addition over that class is the storage of an adaptor, which allows the
 * adaption of parameters. As this functionality has to be type specific,
 * this class was implemented as a template. Storing the adaptors in
 * the GParameterBase class would not have been possible, as it cannot be
 * template-ized - it serves as a base class for the objects stored in the
 * GParameterSet collections.
 */
template <typename T>
class GParameterBaseWithAdaptorsT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterBase {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterBase) & BOOST_SERIALIZATION_NVP(adaptor_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The adaptor base type for a parameter of type T. The adaption precision follows the
     *  parameter type: a floating-point parameter is adapted in its own precision (float -> float),
     *  any other type borrows double (see adaption_fp_type_t). */
    using adaptor_base_t = GAdaptorT<T, adaption_fp_type_t<T>>;

    /***************************************************************************/
    /**
	  * The default constructor.
	  */
    GParameterBaseWithAdaptorsT() = default;

    /***************************************************************************/
    /**
	  * The copy constructor.
	  *
	  * @param cp A copy of another GParameterBaseWithAdaptorsT object
	  */
    GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T> &cp)
      : GParameterBase(cp)
      , adaptor_((cp.adaptor_)->template clone<adaptor_base_t>()) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The destructor. All cleanup work is done by std::shared_ptr.
	  */
    ~GParameterBaseWithAdaptorsT() override = default;

    /***************************************************************************/
    /**
	  * Adds an adaptor to this object. Please note that this class takes ownership of the adaptor
	  * by cloning it.
	  *
	  * @param gat_ptr A std::shared_ptr to an adaptor
	  */
    void addAdaptor(std::shared_ptr<adaptor_base_t> gat_ptr) {
        // Check that we have indeed been given an adaptor
        if(not gat_ptr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << ":" << '\n'
                << "Error: Empty adaptor provided." << '\n'
            );
        }

        if(adaptor_) { // Is an adaptor already present ?
            if(adaptor_->getAdaptorId() == gat_ptr->getAdaptorId()) {
                adaptor_->load(gat_ptr);
            }
            else { // Different type - need to clone and assign to gat_ptr
                adaptor_ = gat_ptr->template clone<adaptor_base_t>();
            }
        }
        else { // None there ? This should not happen
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << '\n'
                << "Found no local adaptor. This should not happen." << '\n'
            );
        }
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * Effects of adding different adaptors to empty/full object tested in GInt32Object::specificTestsNoFailureExpected_GUnitTests()
	  * Failures/throws tested in GDoubleObject::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the adaptor. Throws in DBEUG mode , if we have no adaptor. It is assumed
	  * that only the object holding the "master" adaptor pointer should be allowed to modify it.
	  *
	  * @return A std::shared_ptr to the adaptor
	  */
    std::shared_ptr<adaptor_base_t> getAdaptor() const {
#ifdef DEBUG
        if(not adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT::getAdaptor() :" << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << '\n'
                << "Tried to retrieve adaptor while none is present" << '\n'
            );
        }
#endif /* DEBUG */

        return adaptor_;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * Failures/throws tested in GDoubleObject::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Transforms the adaptor stored in this class to the desired target type. The function
	  * will check in DEBUG mode whether an adaptor was indeed stored in this class. It will
	  * also complain in DEBUG mode if this function was called while no local adaptor was
	  * stored here. Note that this function will only be accessible to the compiler if adaptor_type
	  * is a derivative of adaptor_base_t, thanks to the magic of std::enable_if and type_traits.
	  *
	  * @return The desired adaptor instance, using its "natural" type
	  */
    template <typename adaptor_type>
        requires std::derived_from<adaptor_type, adaptor_base_t>
    std::shared_ptr<adaptor_type> getAdaptor() const {
#ifdef DEBUG
        if(not adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT::getAdaptor<adaptor_type>()" << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << " :" << '\n'
                << "Tried to access empty adaptor pointer." << '\n'
            );

            // Make the compiler happy
            return std::shared_ptr<adaptor_type>();
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<adaptor_base_t, adaptor_type>(adaptor_);
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * Failures/throws tested in GDoubleObject::specificTestsFailuresExpected_GUnitTests() and
	  * GInt32Object::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * This function resets the local adaptor_ pointer.
	  */
    void resetAdaptor() {
        adaptor_ = getDefaultAdaptor<T>();
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Indicates whether an adaptor is present
	  *
	  * @return A boolean indicating whether adaptors are present
	  */
    bool hasAdaptor() const override {
        if(adaptor_) {
            return true;
        }
        return false;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another GParameterBaseWithAdaptorsT object, which
	  * is camouflaged as a GParameterBase.
	  *
	  * @param cp A copy of another GParameterBaseWithAdaptorsT, camouflaged as a GParameterBase
	  */
    void load_(const GParameterBase *cp) override {
        // Check that we are dealing with a  GParameterBaseWithAdaptorsT<T> reference independent of this object and convert the pointer
        const GParameterBaseWithAdaptorsT<T> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GParameterBaseWithAdaptorsT<T>>(cp, this);

        // Load our parent class'es data ...
        GParameterBase::load_(cp);

        // and then our local data
#ifdef DEBUG
        // Check that both we and the "foreign" object have an adaptor
        if(not adaptor_ || not p_load->adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::load_():" << '\n'
                << "Missing adaptor!" << '\n'
            );
        }
#endif
        // Same type: We can just load the data
        if(adaptor_->getAdaptorId() == p_load->adaptor_->getAdaptorId()) {
            adaptor_->load(p_load->adaptor_);
        }
        else { // Different type - need to convert
            adaptor_ = p_load->adaptor_->template clone<adaptor_base_t>();
        }
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterBaseWithAdaptorsT<T>>(
        GParameterBaseWithAdaptorsT<T> const &,
        GParameterBaseWithAdaptorsT<T> const &,
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

        // Check that we are dealing with a  GParameterBaseWithAdaptorsT<T> reference independent of this object and convert the pointer
        const GParameterBaseWithAdaptorsT<T> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GParameterBaseWithAdaptorsT<T>>(cp, this);

        GToken token("GParameterBaseWithAdaptorsT<T>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterBase>(*this, *p_load, token);

        // We access the relevant data of one of the parent classes directly for simplicity reasons
        compare_t(IDENTITY(adaptor_, p_load->adaptor_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    /** @brief Returns a "comparative range"; this is e.g. used to make Gauss-adaption independent of a parameters value range */
    virtual T range() const = 0;

    /***************************************************************************/
    /**
	  * This function applies our adaptor to a value. Note that the argument of
	  * this function will get changed.
	  *
	  * @return The number of adaptions that were carried out
	  */
    std::size_t applyAdaptor(T &value, const T &range, Gem::Hap::GRandomBase &gr) {
#ifdef DEBUG
        if(not adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(value,range):" << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << '\n'
                << "Error: No adaptor was found." << '\n'
            );
        }
#endif /* DEBUG */

        // Apply the adaptor
        return adaptor_->adapt(value, range, gr);
    }

    /***************************************************************************/
    /**
	  * This function applies our adaptor to a collection of values. Note that the argument
	  * of this function will get changed.
	  *
	  * @return The number of adaptions that were carried out
	  */
    std::size_t
    applyAdaptor(std::vector<T> &collection, const T &range, Gem::Hap::GRandomBase &gr) {
#ifdef DEBUG
        if(not adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(collection, range, gr):"
                << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << '\n'
                << "Error: No adaptor was found." << '\n'
            );
        }
#endif /* DEBUG */

        // Apply the adaptor to each data item in turn
        return adaptor_->adapt(collection, range, gr);
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
        if(GParameterBase::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GParameterBaseWithAdaptorsT<>::modify_GUnitTests", "GEM_TESTING");
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
        GParameterBase::specificTestsNoFailureExpected_GUnitTests_();

        // Get a random number generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Test that trying to reset the adaptor will not remove it
            std::shared_ptr<GParameterBaseWithAdaptorsT<T>> p_test =
                this->clone<GParameterBaseWithAdaptorsT<T>>();

            // Make sure no adaptor is present
            CHECK_NOTHROW(p_test->resetAdaptor());
            CHECK(p_test->hasAdaptor() == true);

            T test_val = T(0);
            // We have a local adaptor, so trying to call the applyAdaptor() function should not throw
            CHECK_NOTHROW(p_test->applyAdaptor(test_val, T(1), gr));
        }

        //------------------------------------------------------------------------------

        { // Test that trying to call applyAdaptor(collection) after resetting the adaptor works
            std::shared_ptr<GParameterBaseWithAdaptorsT<T>> p_test =
                this->clone<GParameterBaseWithAdaptorsT<T>>();

            // Make sure no adaptor is present
            CHECK_NOTHROW(p_test->resetAdaptor());
            CHECK(p_test->hasAdaptor() == true);

            std::vector<T> test_vec;
            for(std::size_t i = 0; i < 10; i++) {
                test_vec.push_back(T(0));
            }
            // We have a local adaptor, so trying to call the applyAdaptor(collection) function should not throw
            CHECK_NOTHROW(p_test->applyAdaptor(test_vec, T(1), gr));
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GParameterBaseWithAdaptorsT<>::specificTestsNoFailureExpected_GUnitTests",
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
        GParameterBase::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GParameterBaseWithAdaptorsT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/

private:
    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GParameterBaseWithAdaptorsT");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object. Purely virtual, as we do not want this class to be instantiated directly */
    GParameterBase *clone_() const override = 0;

    /******************************************************************************/
    /**
     * Triggers updates when the optimization process has stalled
     *
     * @param n_stalls The number of consecutive stalls up to this point
     * @return A boolean indicating whether updates were performed
     */
    bool updateAdaptorsOnStall_(std::size_t n_stalls) override {
#ifdef DEBUG
        if(not adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::updateAdaptorsOnStall_(...):" << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << '\n'
                << "Error: No adaptor was found." << '\n'
            );
        }
#endif /* DEBUG */

        return this->adaptor_->updateOnStall(n_stalls, this->range());
    }

    /******************************************************************************/
    /**
     * Retrieves information from an adaptor on a given property
     *
     * @param adaptor_name The name of the adaptor to be queried
     * @param property The property for which information is sought
     * @param data A vector, to which the properties should be added
     */
    void queryAdaptor_(
        const std::string &adaptor_name,
        const std::string &property,
        std::vector<std::any> &data
    ) const override {
#ifdef DEBUG
        if(not adaptor_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterBaseWithAdaptorsT<T>::queryAdaptor:(...):" << '\n'
                << "with typeid(T).name() = " << typeid(T).name() << '\n'
                << "Error: No adaptor was found." << '\n'
            );
        }
#endif /* DEBUG */

        // Note: The following will throw if the adaptor with name "adaptor_name" has
        // no property named "property".
        this->adaptor_->queryPropertyFrom(adaptor_name, property, data);
    }

    /***************************************************************************/
    /**
	  * @brief Holds the adaptor used for adaption of the values stored in derived classes.
	  */
    std::shared_ptr<adaptor_base_t> adaptor_{Gem::Geneva::getDefaultAdaptor<T>()};
};

/******************************************************************************/
/////////////////////////// Specializations for T == bool //////////////////////
/******************************************************************************/
/**
 * This function applies the first adaptor of the adaptor sequence to a collection of values.
 * Note that the parameter of this function will get changed. This is a specialization of a
 * generic template function which is needed due to the peculiarities of a std::vector<bool>
 * (which doesn't return a bool but an object).
 *
 * @return The number of adaptions that were carried out
 */
template <>
inline std::size_t GParameterBaseWithAdaptorsT<bool>::applyAdaptor(
    std::vector<bool> &collection,
    const bool &range,
    Gem::Hap::GRandomBase &gr
) {
#ifdef DEBUG
    if(not adaptor_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(std::vector<bool>& collection):"
            << '\n'
            << "Error: No adaptor was found." << '\n'
        );
    }
#endif /* DEBUG */

    std::size_t n_adapted = 0;

    std::vector<bool>::iterator it;
    for(it = collection.begin(); it != collection.end(); ++it) {
        bool value = *it;
        if(1 == adaptor_->adapt(value, range, gr)) {
            *it = value;
            n_adapted += 1;
        }
    }

    return n_adapted;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename T>
struct is_abstract<Gem::Geneva::Parameters::GParameterBaseWithAdaptorsT<T>> : public boost::true_type {};
template <typename T>
struct is_abstract<const Gem::Geneva::Parameters::GParameterBaseWithAdaptorsT<T>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
