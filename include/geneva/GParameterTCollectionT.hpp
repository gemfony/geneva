/**
 * @file GParameterTCollectionT.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// Standard header files go here
#include <sstream>
#include <vector>
#include <algorithm>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GPARAMETERTCOLLECTIONT_HPP_
#define GPARAMETERTCOLLECTIONT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GHelperFunctionsT.hpp"
#include "GParameterBase.hpp"
#include "GParameterT.hpp"
#include "GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace Geneva {

/***********************************************************************************************/
/**
 * This class shares many similarities with the GParameterCollectionT class. Instead
 * of individual values that can be modified with adaptors, however, it assumes that
 * the objects stored in it have their own adapt() function. This class has been designed
 * as a collection of GParameterT objects, hence the name.  As an example, one can create a
 * collection of GConstrainedDoubleObject objects with this class rather than a simple GDoubleCollection.
 * In order to facilitate memory management, the GParameterT objects are stored
 * in boost::shared_ptr objects.
 */
template<typename T>
class GParameterTCollectionT
	:public GParameterBase,
	 public GStdPtrVectorInterfaceT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save the data
		ar & make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this))
		   & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<GStdPtrVectorInterfaceT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * Allows to find out which type is stored in this class
	 */
	typedef T collection_type;

	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterTCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 */
	GParameterTCollectionT(const GParameterTCollectionT<T>& cp)
		: GParameterBase(cp)
		, GStdPtrVectorInterfaceT<T>(cp)
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterTCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 * @return A constant reference to this object
	 */
	const GParameterTCollectionT<T>& operator=(const GParameterTCollectionT<T>& cp)
	{
		GParameterTCollectionT<T>::load_(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterTCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterTCollectionT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterTCollectionT<T>::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterTCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterTCollectionT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterTCollectionT<T>::operator==","cp", CE_SILENT);
	}


	/*******************************************************************************************/
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
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GParameterTCollectionT<T>  *p_load = GObject::conversion_cast<GParameterTCollectionT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent classes' data ...
		deviations.push_back(GParameterBase::checkRelationshipWith(cp, e, limit, "GParameterTCollectionT<T>", y_name, withMessages));
		deviations.push_back(GStdPtrVectorInterfaceT<T>::checkRelationshipWith(*p_load, e, limit, "GParameterTCollectionT<T>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GParameterTCollectionT<T>", caller, deviations, e);
	}

	/*******************************************************************************************/
	/**
	 * Allows to adapt the values stored in this class. We assume here that
	 * each item has its own adapt function. Hence we do not need to use or
	 * store own adaptors.
	 */
	virtual void adaptImpl() {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->adapt();
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GDoubleObjectCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Initializes floating-point-based parameters with a given value. Allows e.g. to set all
	 * floating point parameters to 0.
	 *
	 * @param val The value to be assigned to the parameters
	 */
	virtual void fpFixedValueInit(const float& val) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpFixedValueInit(val);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in most-derived classes
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Multiplies floating-point-based parameters with a given value.
	 *
	 * @param val The value to be multiplied with the parameter
	 */
	virtual void fpMultiplyBy(const float& val) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpMultiplyBy(val);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in most-derived classes
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Multiplies with a random floating point number in a given range.
	 *
	 * @param min The lower boundary for random number generation
	 * @param max The upper boundary for random number generation
	 */
	virtual void fpMultiplyByRandom(const float& min, const float& max)	{
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpMultiplyByRandom(min, max);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in most-derived classes
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Multiplies with a random floating point number in the range [0, 1[.
	 */
	virtual void fpMultiplyByRandom() {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpMultiplyByRandom();
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in most-derived classes
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Adds the floating point parameters of another GParameterTCollectionT object to this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpAdd(boost::shared_ptr<GParameterBase> p_base)	{
		// We first need to convert p_base to our local type
		typename boost::shared_ptr<GParameterTCollectionT<T> > p
			= GParameterBase::parameterbase_cast<GParameterTCollectionT<T> >(p_base);

		// Check that both collections have the same size
		if(this->size() != p->size()) {
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::fpAdd(): Error!" << std::endl
				  << "Collections have different sizes: " << this->size() << " " << p->size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Call fpAdd on all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it, it_p;
		for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
			(*it)->fpAdd(*it_p);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in most-derived classes
	 * Throw due to invalid size tested in GDoubleObjectCollection::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Subtracts the floating point parameters of another GParameterTCollectionT object from this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpSubtract(boost::shared_ptr<GParameterBase> p_base)	{
		// We first need to convert p_base to our local type
		typename boost::shared_ptr<GParameterTCollectionT<T> > p
			= GParameterBase::parameterbase_cast<GParameterTCollectionT<T> >(p_base);

		// Check that both collections have the same size
		if(this->size() != p->size()) {
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::fpSubtract(): Error!" << std::endl
				  << "Collections have different sizes: " << this->size() << " " << p->size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Call fpAdd on all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it, it_p;
		for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
			(*it)->fpSubtract(*it_p);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in most-derived classes
	 * Throw due to invalid size tested in GDoubleObjectCollection::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Attach parameters of type double to the vector. This function distributes this task to
	 * objects contained in the container.
	 */
	virtual void doubleStreamline(std::vector<double>& parVec) const {
		typename GParameterTCollectionT<T>::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->doubleStreamline(parVec);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Attach parameters of type boost::int32_t to the vector. This function distributes this task
	 * to objects contained in the container.
	 */
	virtual void int32Streamline(std::vector<boost::int32_t>& parVec) const {
		typename GParameterTCollectionT<T>::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->int32Streamline(parVec);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Attach parameters of type bool to the vector.  This function distributes this task
	 * to objects contained in the container.
	 */
	virtual void booleanStreamline(std::vector<bool>& parVec) const {
		typename GParameterTCollectionT<T>::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->booleanStreamline(parVec);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Count the number of double parameters. This function returns the responses from all
	 * objects contained in this collection.
	 *
	 * @return The number of double parameters in this collection
	 */
	virtual std::size_t countDoubleParameters() const {
		std::size_t result = 0;

		typename GParameterTCollectionT<T>::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			result += (*cit)->countDoubleParameters();
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Count the number of boost::int32_t parameters. This function returns the responses from all
	 * objects contained in this collection.
	 *
	 * @return The number of boost::int32_t parameters in this collection
	 */
	virtual std::size_t countInt32Parameters() const {
		std::size_t result = 0;

		typename GParameterTCollectionT<T>::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			result += (*cit)->countInt32Parameters();
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Count the number of bool parameters. This function returns the responses from all
	 * objects contained in this collection.
	 *
	 * @return The number of bool parameters in this collection
	 */
	virtual std::size_t countBoolParameters() const {
		std::size_t result = 0;

		typename GParameterTCollectionT<T>::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			result += (*cit)->countBoolParameters();
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Assigns part of a value vector to the parameter
	 */
	void assignDoubleValueVector(const std::vector<double>& parVec, std::size_t& pos) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->assignDoubleValueVector(parVec, pos);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Assigns part of a value vector to the parameter
	 */
	void assignInt32ValueVector(const std::vector<boost::int32_t>& parVec, std::size_t& pos) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->assignInt32ValueVector(parVec, pos);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Assigns part of a value vector to the parameter
	 */
	void assignBooleanValueVector(const std::vector<bool>& parVec, std::size_t& pos) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->assignBooleanValueVector(parVec, pos);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested
	 * ----------------------------------------------------------------------------------
	 */

	/*******************************************************************************************/
	/**
	 * Assigns a random number generator from another object to all objects stored in this
	 * collection and to the object itself.
	 *
	 * @param gr_cp A reference to another object's GRandomBaseT object derivative
	 */
	virtual void assignGRandomPointer(Gem::Hap::GRandomBaseT<double, boost::int32_t> *gr_cp) {
		// Do some error checking
		if(!gr_cp) {
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::assignGRandomPointer(): Error!" << std::endl
				  << "Tried to assign a NULL pointer" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Assign the foreign pointer to all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->assignGRandomPointer(gr_cp);
		}

		// Assign the foreign pointer to this object as well
		GParameterBase::assignGRandomPointer(gr_cp);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterTCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * Throw tested in GParameterTCollectionT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Re-connects the local random number generator to gr and distributes the call
	 * to all objects contained in this collection class.
	 */
	virtual void resetGRandomPointer() {
		// Reset all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->resetGRandomPointer();
		}

		// Reset our parent class
		GParameterBase::resetGRandomPointer();
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterTCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Checks whether solely the local random number generator is used. The function returns
	 * false if at least one component of this class does not use a local random number
	 * generator
	 *
	 * @bool A boolean indicating whether solely the local random number generator is used
	 */
	virtual bool usesLocalRNG() const {
		bool result = true;

		// Check all components of this class
		typename GParameterTCollectionT<T>::const_iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			if(!(*it)->usesLocalRNG()) result = false;
		}

		// Check our parent class
		if(!GParameterBase::usesLocalRNG()) result = false;

		return result;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterTCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Checks whether all relevant objects use the assigned random number generator.
	 *
	 * @return A boolean indicating whether an assigned random number generator is used
	 */
	virtual bool assignedRNGUsed() const {
		bool result = true;

		// Check all components of this class
		typename GParameterTCollectionT<T>::const_iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			if(!(*it)->assignedRNGUsed()) result = false;
		}

		// Check our parent class
		if(!GParameterBase::assignedRNGUsed()) result = false;

		return result;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterTCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

protected:
	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) {
		// Convert cp into local format
		const GParameterTCollectionT<T> *p_load = GObject::conversion_cast<GParameterTCollectionT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBase::load_(cp);
		GStdPtrVectorInterfaceT<T>::operator=(*p_load);
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Declared purely virtual, as this class is not
	 * intended to be used directly.
	 */
	virtual GObject* clone_() const = 0;

	/*******************************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Making the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() { /* nothing */ }

	/*******************************************************************************************/
	/**
	 * Triggers random initialization of all parameter objects
	 */
	virtual void randomInit_() {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			// Note that we do not call the randomInit_() function. First of all, we
			// do not have access to it. Secondly it might be that re-initialization of
			// a specific object is not desired.
			(*it)->GParameterBase::randomInit();
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GParameterObjectCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

#ifdef GENEVATESTING
public:
	/*******************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent classes' functions
		if(GParameterBase::modify_GUnitTests()) result = true;
		if(GStdPtrVectorInterfaceT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBase::specificTestsNoFailureExpected_GUnitTests();
		GStdPtrVectorInterfaceT<T>::specificTestsNoFailureExpected_GUnitTests();

		//---------------------------------------------------------------------

		{ // Check adding and resetting of random number generators
			// Create two local clones
			boost::shared_ptr<GParameterTCollectionT<T> > p_test1 = this->clone<GParameterTCollectionT<T> >();

			// Assign a factory generator
			Gem::Hap::GRandomBaseT<double, boost::int32_t> *gr_test = new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY, double, boost::int32_t>();
			BOOST_CHECK_NO_THROW(p_test1->assignGRandomPointer(gr_test));

			// Has the generator been assigned ?
			BOOST_CHECK(p_test1->usesLocalRNG() == false);
			BOOST_CHECK(p_test1->assignedRNGUsed() == true);

			// Make sure we use the local generator again
			BOOST_CHECK_NO_THROW(p_test1->resetGRandomPointer());

			// Get rid of the test generator
			delete gr_test;

			// We should now be using a local random number generator again
			BOOST_CHECK(p_test1->usesLocalRNG() == true);
			BOOST_CHECK(p_test1->assignedRNGUsed() == false);
		}

		//------------------------------------------------------------------------------
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBase::specificTestsFailuresExpected_GUnitTests();
		GStdPtrVectorInterfaceT<T>::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Check that assigning a NULL pointer for the random number generator throws
			boost::shared_ptr<GParameterTCollectionT<T> > p_test = this->clone<GParameterTCollectionT<T> >();

			// Assigning a NULL pointer should throw
			BOOST_CHECK_THROW(
					p_test->assignGRandomPointer(NULL);
					, Gem::Common::gemfony_error_condition
			);
		}

		//------------------------------------------------------------------------------
	}
#endif /* GENEVATESTING */
};


/***********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GPARAMETERTCOLLECTIONT_HPP_ */
