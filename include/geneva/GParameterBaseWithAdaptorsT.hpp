/**
 * @file GParameterBaseWithAdaptorsT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */



// Standard header files go here

// Boost header files go here

#ifndef GPARAMETERBASEWITHADAPTORST_HPP_
#define GPARAMETERBASEWITHADAPTORST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GAdaptorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a template-ized version of the GParameterBase class. Its main
 * addition over that class is the storage of an adaptor, which allows the
 * adaption of parameters. As this functionality has to be type specific,
 * this class has also been implemented as a template. Storing the adaptors in
 * the GParameterBase class would not have been possible, as it cannot be
 * template-ized - it serves as a base class for the objects stored in the
 * GParameterSet collections.
 */
template <typename T>
class GParameterBaseWithAdaptorsT:	public GParameterBase
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterBase)
		   & BOOST_SERIALIZATION_NVP(adaptor_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor. adaptor_ will be initialized with the default adaptor for this
	 * type
	 */
	GParameterBaseWithAdaptorsT()
		: GParameterBase()
		, adaptor_(getDefaultAdaptor<T>())
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * The copy constructor.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT object
	 */
	GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T>& cp)
		: GParameterBase(cp)
		, adaptor_((cp.adaptor_)->GObject::template clone<GAdaptorT<T> >())
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor. All cleanup work is done by boost::shared_ptr.
	 */
	virtual ~GParameterBaseWithAdaptorsT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Checks for equality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterBaseWithAdaptorsT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterBaseWithAdaptorsT<T>::operator==","cp", CE_SILENT);
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterBaseWithAdaptorsT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterBaseWithAdaptorsT<T>::operator!=","cp", CE_SILENT);
	}

	/***************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const OVERRIDE {
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GParameterBaseWithAdaptorsT<T>  *p_load = GObject::gobject_conversion<GParameterBaseWithAdaptorsT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterBase::checkRelationshipWith(cp, e, limit, "GParameterBaseWithAdaptorsT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GParameterBaseWithAdaptorsT<T>", adaptor_, p_load->adaptor_, "adaptor_", "p_load->adaptor_", e , limit));

		return evaluateDiscrepancies("GParameterBaseWithAdaptorsT<T>", caller, deviations, e);
	}

	/***************************************************************************/
	/**
	 * Adds an adaptor to this object. Please note that this class takes ownership of the adaptor
	 * by cloning it.
	 *
	 * @param gat_ptr A boost::shared_ptr to an adaptor
	 */
	void addAdaptor(boost::shared_ptr<GAdaptorT<T> > gat_ptr) {
		// Check that we have indeed been given an adaptor
		if(!gat_ptr){
		   glogger
		   << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << std::endl
         << "with typeid(T).name() = " << typeid(T).name() << ":" << std::endl
         << "Error: Empty adaptor provided." << std::endl
         << GEXCEPTION;
		}

		if(adaptor_) { // Is an adaptor already present ?
			if (adaptor_->getAdaptorId() == gat_ptr->getAdaptorId()) {
				adaptor_->GObject::load(gat_ptr);
			}
			// Different type - need to clone and assign to gat_ptr
			else {
				adaptor_ = gat_ptr->GObject::template clone<GAdaptorT<T> >();
			}
		}
		// None there ? This should not happen
		else {
		   glogger
		   << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << std::endl
         << "Found no local adaptor. This should not happen!" << std::endl
         << GEXCEPTION;
		}

		// Make our local random number generator known to the adaptor
		adaptor_->assignGRandomPointer(GParameterBase::gr);
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
	 * @return A boost::shared_ptr to the adaptor
	 */
	boost::shared_ptr<GAdaptorT<T> > getAdaptor() const {
#ifdef DEBUG
		if(!adaptor_) {
		   glogger
		   << "In GParameterBaseWithAdaptorsT::getAdaptor() :" << std::endl
         << "with typeid(T).name() = " << typeid(T).name() << std::endl
         << "Tried to retrieve adaptor while none is present" << std::endl
         << GEXCEPTION;
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
	 * is a derivative of GAdaptorT<T>, thanks to the magic of Boost's enable_if and Type Traits
	 * libraries.
	 *
	 * @return The desired adaptor instance, using its "natural" type
	 */
	template <typename adaptor_type>
	boost::shared_ptr<adaptor_type> getAdaptor(
			typename boost::enable_if<boost::is_base_of<GAdaptorT<T>, adaptor_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		if(!adaptor_) {
		   glogger
		   << "In GParameterBaseWithAdaptorsT::getAdaptor<adaptor_type>()" << std::endl
         << "with typeid(T).name() = " << typeid(T).name() << " :" << std::endl
         << "Tried to access empty adaptor pointer." << std::endl
         << GEXCEPTION;

		   // Make the compiler happy
		   return boost::shared_ptr<adaptor_type>();
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
	bool hasAdaptor() const {
		if(adaptor_) return true;
		return false;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Assigns a random number generator from another object to this object and any adaptor
	 * contained herein.
	 *
	 * @param gr_cp A reference to another object's GRandomBaseT object derivative
	 */
	virtual void assignGRandomPointer(Gem::Hap::GRandomBase *gr_cp) {
		if(adaptor_) adaptor_->assignGRandomPointer(gr_cp);
		GParameterBase::assignGRandomPointer(gr_cp);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Re-connects the local random number generator to gr and tells the adaptor to do the same.
	 */
	virtual void resetGRandomPointer() {
		if(adaptor_) adaptor_->resetGRandomPointer();
		GParameterBase::resetGRandomPointer();
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Checks whether the local random number generator is used in this class and in a
	 * possible adaptor contained in this object. The result will be true only if both
	 * this object and the adaptor (if available) contain a local random number generator.
	 *
	 * @bool A boolean indicating whether solely the local random number generator is used
	 */
	virtual bool usesLocalRNG() const {
		bool result=true;

		if(adaptor_ && !adaptor_->usesLocalRNG()) result=false;
		if(!GParameterBase::usesLocalRNG()) result=false;

		return result;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Checks whether a "foreign" random number generator is used in this class and in a
	 * possible adaptor contained in this object. The result will be true only if both
	 * this object and the adaptor (if available) use the foreign generator
	 *
	 * @bool A boolean indicating whether solely the foreign random number generator is used
	 */
	virtual bool assignedRNGUsed() const {
		bool result=true;

		if(adaptor_ && !adaptor_->assignedRNGUsed()) result=false;
		if(!GParameterBase::assignedRNGUsed()) result=false;

		return result;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const OVERRIDE {
	   return std::string("GParameterBaseWithAdaptorsT");
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GParameterBaseWithAdaptorsT object, which
	 * is camouflaged as a GObject.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) OVERRIDE {
		// Convert cp into local format
		const GParameterBaseWithAdaptorsT<T> *p_load = this->gobject_conversion<GParameterBaseWithAdaptorsT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBase::load_(cp);

		// and then our local data
#ifdef DEBUG
		// Check that both we and the "foreign" object have an adaptor
		if(!adaptor_ || !p_load->adaptor_) {
		   glogger
		   << "In GParameterBaseWithAdaptorsT<T>::load_():" << std::endl
         << "Missing adaptor!" << std::endl
         << GEXCEPTION;
		}
#endif
		// Same type: We can just load the data
		if (adaptor_->getAdaptorId() == p_load->adaptor_->getAdaptorId()) {
			adaptor_->GObject::load(p_load->adaptor_);
		}
		// Different type - need to convert
		else {
			adaptor_ = p_load->adaptor_->GObject::template clone<GAdaptorT<T> >();
		}
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object. Purely virtual, as we do not want this
	 * class to be instantiated directly */
	virtual GObject* clone_() const = 0;

	/***************************************************************************/
	/**
	 * This function applies our adaptor to a value. Note that the argument of
	 * this function will get changed.
	 *
	 * @param value The parameter to be adapted
	 */
	void applyAdaptor(T &value) {
#ifdef DEBUG
		if (!adaptor_) {
		   glogger
		   << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(T& value):" << std::endl
         << "with typeid(T).name() = " << typeid(T).name() << std::endl
         << "Error: No adaptor was found." << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		adaptor_->adapt(value);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested / used indirectly through GParameterT<T>::adaptImpl()
	 * Failures/throws tested in GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * This function applies our adaptor to a collection of values. Note that the argument
	 * of this function will get changed.
	 *
	 * @param collection A vector of values that shall be adapted
	 */
	void applyAdaptor(std::vector<T> &collection) {
#ifdef DEBUG
		if(!adaptor_) {
		   glogger
		   << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(std::vector<T>& collection):" << std::endl
         << "with typeid(T).name() = " << typeid(T).name() << std::endl
         << "Error: No adaptor was found." << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		// Apply the adaptor to each data item in turn
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it)	{
			adaptor_->adapt(*it);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested / used indirectly through GParameterCollectionT<T>::adaptImpl()
	 * Failures/throws tested in GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

private:
	/***************************************************************************/
	/**
	 * @brief Holds the adaptor used for adaption of the values stored in derived classes.
	 */
	boost::shared_ptr<GAdaptorT<T> > adaptor_;

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent classes' functions
		if(GParameterBase::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GParameterBaseWithAdaptorsT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterBase::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Test assigning and resetting of random number generators in this object and any stored adaptor
			boost::shared_ptr<GParameterBaseWithAdaptorsT<T> > p_test = this->clone<GParameterBaseWithAdaptorsT<T> >();

			// Force this object and any stored adaptor to use the local generator
			BOOST_CHECK_NO_THROW(p_test->resetGRandomPointer());

			// Cross-check that the local generator is used
			BOOST_CHECK(p_test->usesLocalRNG() == true);
			// Also check the parent class and the adaptor individually
			BOOST_CHECK(p_test->GParameterBase::usesLocalRNG() == true);
			BOOST_CHECK(p_test->adaptor_ && p_test->adaptor_->usesLocalRNG() == true);

			// Create and assign a "foreign" generator
			Gem::Hap::GRandomBase *gr_test = new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>();
			BOOST_CHECK_NO_THROW(p_test->assignGRandomPointer(gr_test));

			// Cross-check that the foreign generator is used
			BOOST_CHECK(p_test->assignedRNGUsed() == true);
			// Also check the parent class and the adaptor individually
			BOOST_CHECK(p_test->GParameterBase::usesLocalRNG() == false);
			BOOST_CHECK(p_test->adaptor_ && p_test->adaptor_->usesLocalRNG() == false);

			// Clean up p_test before we remove the random number generator
			p_test.reset();

			// Get rid of the dynamically allocated memory
			delete gr_test;
		}

		//------------------------------------------------------------------------------

		{ // Test that trying to reset the adaptor will not remove it
			boost::shared_ptr<GParameterBaseWithAdaptorsT<T> > p_test = this->clone<GParameterBaseWithAdaptorsT<T> >();

			// Make sure no adaptor is present
			BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
			BOOST_CHECK(p_test->hasAdaptor() == true);

			T testVal = T(0);
			// We have a local adaptor, so trying to call the applyAdaptor() function should not throw
			BOOST_CHECK_NO_THROW(p_test->applyAdaptor(testVal));
		}

		//------------------------------------------------------------------------------

		{ // Test that trying to call applyAdaptor(collection) after resetting the adaptor works
			boost::shared_ptr<GParameterBaseWithAdaptorsT<T> > p_test = this->clone<GParameterBaseWithAdaptorsT<T> >();

			// Make sure no adaptor is present
			BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
			BOOST_CHECK(p_test->hasAdaptor() == true);

			std::vector<T> testVec;
			for(std::size_t i=0; i<10; i++) testVec.push_back(T(0));
			// We have a local adaptor, so trying to call the applyAdaptor(collection) function should not throw
			BOOST_CHECK_NO_THROW(p_test->applyAdaptor(testVec));
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GParameterBaseWithAdaptorsT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterBase::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GParameterBaseWithAdaptorsT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
};

// Declaration of specializations for std::vector<bool>
template<> void GParameterBaseWithAdaptorsT<bool>::applyAdaptor(std::vector<bool>&);

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GParameterBaseWithAdaptorsT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GParameterBaseWithAdaptorsT<T> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GPARAMETERBASEWITHADAPTORST_HPP_ */
