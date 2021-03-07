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

// Boost header files go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GAdaptorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GConstrainedValueLimitT.hpp"

namespace Gem::Geneva {

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
class GParameterBaseWithAdaptorsT
	 : public GParameterBase
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterBase)
		 & BOOST_SERIALIZATION_NVP(adaptor_);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
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
	 GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T>& cp)
		 : GParameterBase(cp)
		 , adaptor_((cp.adaptor_)->GObject::template clone<GAdaptorT<T>>())
	 { /* nothing */ }

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
	 void addAdaptor(std::shared_ptr<GAdaptorT<T>> gat_ptr) {
		 // Check that we have indeed been given an adaptor
		 if(not gat_ptr){
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << std::endl
					 << "with typeid(T).name() = " << typeid(T).name() << ":" << std::endl
					 << "Error: Empty adaptor provided." << std::endl
			 );
		 }

		 if(adaptor_) { // Is an adaptor already present ?
			 if (adaptor_->getAdaptorId() == gat_ptr->getAdaptorId()) {
				 adaptor_->GObject::load(gat_ptr);
			 } else { // Different type - need to clone and assign to gat_ptr
				 adaptor_ = gat_ptr->GObject::template clone<GAdaptorT<T>>();
			 }
		 } else { // None there ? This should not happen
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << std::endl
					 << "Found no local adaptor. This should not happennot " << std::endl
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
	 [[nodiscard]] std::shared_ptr<GAdaptorT<T>> getAdaptor() const {
#ifdef DEBUG
		 if(not adaptor_) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT::getAdaptor() :" << std::endl
					 << "with typeid(T).name() = " << typeid(T).name() << std::endl
					 << "Tried to retrieve adaptor while none is present" << std::endl
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
	  * is a derivative of GAdaptorT<T>, thanks to the magic of std::enable_if and type_traits.
	  *
	  * @return The desired adaptor instance, using its "natural" type
	  */
	 template <typename adaptor_type>
	 std::shared_ptr<adaptor_type> getAdaptor(
		 typename std::enable_if<std::is_base_of<GAdaptorT<T>, adaptor_type>::value>::type * = nullptr // NOLINT
	 ) const {
#ifdef DEBUG
		 if(not adaptor_) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT::getAdaptor<adaptor_type>()" << std::endl
					 << "with typeid(T).name() = " << typeid(T).name() << " :" << std::endl
					 << "Tried to access empty adaptor pointer." << std::endl
			 );
		 }
#endif /* DEBUG */

		 // Does error checks on the conversion internally
		 return Gem::Common::convertSmartPointer<GAdaptorT<T>,adaptor_type>(adaptor_);
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
	 [[nodiscard]] bool hasAdaptor() const override {
		 if(adaptor_) return true;
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
	  * is camouflaged as a GObject.
	  *
	  * @param cp A copy of another GParameterBaseWithAdaptorsT, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Check that we are dealing with a  GParameterBaseWithAdaptorsT<T> reference independent of this object and convert the pointer
		 const GParameterBaseWithAdaptorsT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterBaseWithAdaptorsT<T>>(cp, this);

		 // Load our parent class'es data ...
		 GParameterBase::load_(cp);

		 // and then our local data
#ifdef DEBUG
		 // Check that both we and the "foreign" object have an adaptor
		 if(not adaptor_ || not p_load->adaptor_) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT<T>::load_():" << std::endl
					 << "Missing adaptor!" << std::endl
			 );
		 }
#endif
		 // Same type: We can just load the data
		 if (adaptor_->getAdaptorId() == p_load->adaptor_->getAdaptorId()) {
			 adaptor_->GObject::load(p_load->adaptor_);
		 } else { // Different type - need to convert
			 adaptor_ = p_load->adaptor_->GObject::template clone<GAdaptorT<T>>();
		 }
	 }

	/***************************************************************************/
	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GParameterBaseWithAdaptorsT<T>>(
		GParameterBaseWithAdaptorsT<T> const &
		, GParameterBaseWithAdaptorsT<T> const &
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
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a  GParameterBaseWithAdaptorsT<T> reference independent of this object and convert the pointer
		const GParameterBaseWithAdaptorsT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterBaseWithAdaptorsT<T>>(cp, this);

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
	 [[nodiscard]] virtual T range() const BASE  = 0;

	 /***************************************************************************/
	 /**
	  * This function applies our adaptor to a value. Note that the argument of
	  * this function will get changed.
	  *
	  * @param value The parameter to be adapted
	  * @param range A typical value range of underlying parameter types
	  * @return The number of adaptions that were carried out
	  */
	 std::size_t applyAdaptor(
		 T &value
		 , const T& range
		 , Gem::Hap::GRandomBase& gr
	 ) {
#ifdef DEBUG
		 if (not adaptor_) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(value,range):" << std::endl
					 << "with typeid(T).name() = " << typeid(T).name() << std::endl
					 << "Error: No adaptor was found." << std::endl
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
	  * @param collection A vector of values that shall be adapted
	  * @param range A typical value range of underlying parameter types
	  * @return The number of adaptions that were carried out
	  */
	 std::size_t applyAdaptor(
		 std::vector<T> &collection
		 , const T& range
		 , Gem::Hap::GRandomBase& gr
	 ) {
#ifdef DEBUG
		 if(not adaptor_) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(collection, range, gr):" << std::endl
					 << "with typeid(T).name() = " << typeid(T).name() << std::endl
					 << "Error: No adaptor was found." << std::endl
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
		if(GParameterBase::modify_GUnitTests_()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GParameterBaseWithAdaptorsT<>::modify_GUnitTests", "GEM_TESTING");
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
		GParameterBase::specificTestsNoFailureExpected_GUnitTests_();

		// Get a random number generator
		Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

		//------------------------------------------------------------------------------

		{ // Test that trying to reset the adaptor will not remove it
			std::shared_ptr<GParameterBaseWithAdaptorsT<T>> p_test = this->clone<GParameterBaseWithAdaptorsT<T>>();

			// Make sure no adaptor is present
			BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
			BOOST_CHECK(p_test->hasAdaptor() == true);

			T testVal = T(0);
			// We have a local adaptor, so trying to call the applyAdaptor() function should not throw
			BOOST_CHECK_NO_THROW(p_test->applyAdaptor(testVal, T(1), gr));
		}

		//------------------------------------------------------------------------------

		{ // Test that trying to call applyAdaptor(collection) after resetting the adaptor works
			std::shared_ptr<GParameterBaseWithAdaptorsT<T>> p_test = this->clone<GParameterBaseWithAdaptorsT<T>>();

			// Make sure no adaptor is present
			BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
			BOOST_CHECK(p_test->hasAdaptor() == true);

			std::vector<T> testVec;
			for(std::size_t i=0; i<10; i++) testVec.push_back(T(0));
			// We have a local adaptor, so trying to call the applyAdaptor(collection) function should not throw
			BOOST_CHECK_NO_THROW(p_test->applyAdaptor(testVec, T(1), gr));
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GParameterBaseWithAdaptorsT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
	void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterBase::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GParameterBaseWithAdaptorsT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name_() const override { // NOLINT
		 return std::string("GParameterBaseWithAdaptorsT");
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object. Purely virtual, as we do not want this class to be instantiated directly */
	 [[nodiscard]] GObject* clone_() const override = 0;

    /******************************************************************************/
    /**
     * Triggers updates when the optimization process has stalled
     *
     * @param nStalls The number of consecutive stalls up to this point
     * @return A boolean indicating whether updates were performed
     */
    bool updateAdaptorsOnStall_(std::size_t nStalls) override {
#ifdef DEBUG
        if (not adaptor_) {
            throw gemfony_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                            << "In GParameterBaseWithAdaptorsT<T>::updateAdaptorsOnStall_(...):" << std::endl
                            << "with typeid(T).name() = " << typeid(T).name() << std::endl
                            << "Error: No adaptor was found." << std::endl
            );
        }
#endif /* DEBUG */

        return this->adaptor_->updateOnStall(nStalls, this->range());
    }

	/******************************************************************************/
	/**
     * Retrieves information from an adaptor on a given property
     *
     * @param adaoptorName The name of the adaptor to be queried
     * @param property The property for which information is sought
     * @param data A vector, to which the properties should be added
     */
	void queryAdaptor_(
			const std::string& adaptorName
			, const std::string& property
			, std::vector<boost::any>& data
	) const override {
#ifdef DEBUG
		if (not adaptor_) {
			throw gemfony_exception(
					g_error_streamer(DO_LOG, time_and_place)
							<< "In GParameterBaseWithAdaptorsT<T>::queryAdaptor:(...):" << std::endl
							<< "with typeid(T).name() = " << typeid(T).name() << std::endl
							<< "Error: No adaptor was found." << std::endl
			);
		}
#endif /* DEBUG */

		// Note: The following will throw if the adaptor with name "adaptorName" has
		// no property named "property".
		this->adaptor_->queryPropertyFrom(adaptorName, property, data);
	}

	 /***************************************************************************/
	 /**
	  * @brief Holds the adaptor used for adaption of the values stored in derived classes.
	  */
	 std::shared_ptr<GAdaptorT<T>> adaptor_{Gem::Geneva::getDefaultAdaptor<T>()};
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
 * @param collection A vector of values that shall be adapted
 * @return The number of adaptions that were carried out
 */
template <>
inline std::size_t GParameterBaseWithAdaptorsT<bool>::applyAdaptor(
	std::vector<bool>& collection
	, const bool& range
	, Gem::Hap::GRandomBase& gr
) {
#ifdef DEBUG
	if(not adaptor_) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(std::vector<bool>& collection):" << std::endl
				<< "Error: No adaptor was found." << std::endl
		);
	}
#endif /* DEBUG */

	std::size_t nAdapted = 0;

	std::vector<bool>::iterator it;
	for (it = collection.begin(); it != collection.end(); ++it) {
		bool value = *it;
		if(1 == adaptor_->adapt(value, range, gr)) {
			*it = value;
			nAdapted += 1;
		}
	}

	return nAdapted;
}

/******************************************************************************/

} // namespace Gem::Geneva

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost::serialization {
template<typename T>
struct is_abstract<Gem::Geneva::GParameterBaseWithAdaptorsT<T>> : public boost::true_type {};
template<typename T>
struct is_abstract< const Gem::Geneva::GParameterBaseWithAdaptorsT<T>> : public boost::true_type {};
} // namespace boost::serialization

/******************************************************************************/
