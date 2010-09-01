/**
 * @file GTestIndividual1.hpp
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
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort
#include <utility> // For std::pair

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

#ifndef GTESTINDIVIDUAL1_HPP_
#define GTESTINDIVIDUAL1_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDouble.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "common/GCommonEnums.hpp"

namespace Gem
{
namespace Tests
{
/************************************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual1 :public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The default constructor.
	 */
	GTestIndividual1()
		: GParameterSet()
	{
		// Fill with some data
		boost::shared_ptr<Gem::Geneva::GDoubleCollection > gdc_ptr(new Gem::Geneva::GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga1(new Gem::Geneva::GDoubleGaussAdaptor(1.,0.6,0.,2.));
		gdc_ptr->addAdaptor(gdga1);
		this->push_back(gdc_ptr);
	}

	/********************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A constant reference to another GTestIndividual1 object
	 */
	GTestIndividual1(const GTestIndividual1& cp)
		: GParameterSet(cp)
	{	/* nothing */ }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GTestIndividual1()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GTestIndividual1 object
	 * @return A constant reference to this object
	 */
	const GTestIndividual1& operator=(const GTestIndividual1& cp){
		GTestIndividual1::load_(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GTestIndividual1 object
	 *
	 * @param  cp A constant reference to another GTestIndividual1 object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GTestIndividual1& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GTestIndividual1::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GTestIndividual1 object
	 *
	 * @param  cp A constant reference to another GTestIndividual1 object
	 * @return A boolean indicating whether both objects are in-equal
	 */
	bool operator!=(const GTestIndividual1& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GTestIndividual1::operator!=","cp", CE_SILENT);
	}

	/********************************************************************************************/
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

		// Check that we are not accidently assigning this object to itself
		GObject::selfAssignmentCheck<GTestIndividual1>(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GTestIndividual1", y_name, withMessages));

		// ... no local data

		return evaluateDiscrepancies("GTestIndividual1", caller, deviations, e);
	}

protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GTestIndividual1, camouflaged as a GObject.
	 *
	 * @param cp A copy of another GTestIndividual1, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp)
	{
		// Check that we are not accidently assigning this object to itself
		GObject::selfAssignmentCheck<GTestIndividual1>(cp);

		// Load our parent's data
		GParameterSet::load_(cp);

		// Load local data here like this:
		// myVar = p_load->myVar;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GTestIndividual1(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		double result = 0.;

		// Extract the first Gem::Geneva::GDoubleCollection object. In a realistic scenario, you might want
		// to add error checks here upon first invocation.
		boost::shared_ptr<Gem::Geneva::GDoubleCollection> vC = pc_at<Gem::Geneva::GDoubleCollection>(0);

		// Calculate the value of the parabola
		for(std::size_t i=0; i<vC->size(); i++) {
			result += vC->at(i) * vC->at(i);
		}

		return result;
	}

#ifdef GENEVATESTING
public:
	// Note: The following code is designed to mainly test parent classes

	/******************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent classes' functions
		if(Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

		// Change the parameter settings
		this->adapt();
		result = true;

		return result;
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Tests whether calls to adapt() result in changes of the object
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();
			boost::shared_ptr<GTestIndividual1> p_test_old = this->clone<GTestIndividual1>();

			std::size_t nTests = 1000;

			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(p_test->adapt());
				BOOST_CHECK(*p_test != *p_test_old);
				BOOST_CHECK_NO_THROW(p_test_old->load(p_test));
			}
		}

		//------------------------------------------------------------------------------

		{ // Tests customAdaptions, dirtyFlag and the effects of the fitness function
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

			// Make sure this individual is not dirty
			if(p_test->isDirty()) BOOST_CHECK_NO_THROW(p_test->fitness());
			BOOST_CHECK(!p_test->isDirty());

			std::size_t nTests = 1000;

			double currentFitness = p_test->fitness();
			double oldFitness = currentFitness;
			bool dirtyFlag = false;

			for(std::size_t i=0; i<nTests; i++) {
				// Change the parameters without instantly triggering fitness calculation
				BOOST_CHECK_NO_THROW(p_test->customAdaptions());
				// The dirty flag should not have been set yet (done in adapt() )
				BOOST_CHECK(!p_test->isDirty());
				// Set the flag manually
				BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
				// Check that the dirty flag has indeed been set
				BOOST_CHECK(p_test->isDirty());
				if(i>0) {
					dirtyFlag = false; // The next call should change this value
					// Once oldFitness has been set (in iterations > 0), currentFitness() should return that value here
					BOOST_CHECK_MESSAGE (
							oldFitness == p_test->getCurrentFitness(dirtyFlag)
							,  "\n"
							<< "oldFitness = " << oldFitness << "\n"
							<< "p_test->getCurrentFitness(dirtyFlag) = " << p_test->getCurrentFitness(dirtyFlag) << "\n"
							<< "dirtyFlag = " << dirtyFlag << "\n"
							<< "iteration = " << i << "\n"
					);
					// Check that the dirty flag has been set
					BOOST_CHECK(dirtyFlag == true);
				}
				// Trigger value calculation
				BOOST_CHECK_NO_THROW(currentFitness = p_test->fitness());
				// Check that getCurrentFitness() returns the same value as fitness()
				dirtyFlag = true; // The next call should change this value
				BOOST_CHECK_MESSAGE (
						currentFitness == p_test->getCurrentFitness(dirtyFlag)
						,  "\n"
						<< "currentFitness = " << currentFitness << "\n"
						<< "p_test->getCurrentFitness(dirtyFlag) = " << p_test->getCurrentFitness(dirtyFlag) << "\n"
						<< "dirtyFlag = " << dirtyFlag << "\n"
						<< "iteration = " << i << "\n"
				);
				// Check that the dirtyFlag has the value "false"
				BOOST_CHECK(dirtyFlag == false);
				// Check that the individual is now clean
				BOOST_CHECK(!p_test->isDirty());
				BOOST_CHECK_MESSAGE(
				// Check that the fitness has changed
						currentFitness != oldFitness
						,  "\n"
						<< "currentFitness = " << currentFitness << "\n"
						<< "oldFitness = " << oldFitness << "\n"
						<< "iteration = " << i << "\n"
				);
				oldFitness = currentFitness;
			}
		}

		//------------------------------------------------------------------------------

		{ // Check updating and restoring of RNGs
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

			// Distribute our local generator to all objects
			BOOST_CHECK_NO_THROW(p_test->updateRNGs());
			BOOST_CHECK(p_test->localRNGsUsed() == false);

			// Restore the local generators
			BOOST_CHECK_NO_THROW(p_test->restoreRNGs());
			BOOST_CHECK(p_test->localRNGsUsed() == true);
		}

		//------------------------------------------------------------------------------

		{ // Check the effects of the process function in EA mode, using the "adapt" call, with one allowed processing cycle
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

			// Make sure our individuals are clean and evaluated
			BOOST_CHECK_NO_THROW(p_test->fitness());
			boost::shared_ptr<GTestIndividual1> p_test_orig = p_test->clone<GTestIndividual1>();

			BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::EA));
			BOOST_CHECK_NO_THROW(p_test->getPersonalityTraits()->setCommand("adapt"));
			BOOST_CHECK_NO_THROW(p_test_orig->setPersonality(Gem::Geneva::EA));
			BOOST_CHECK_NO_THROW(p_test_orig->getPersonalityTraits()->setCommand("adapt"));

			// Cross check that both individuals are indeed currently equal
			BOOST_CHECK(*p_test == *p_test_orig);

			// Allow just one processing cycle
			BOOST_CHECK_NO_THROW(p_test->setProcessingCycles(1));
			BOOST_CHECK_NO_THROW(p_test->process());

			// Check that p_test and p_test_orig differ
			BOOST_CHECK(*p_test != *p_test_orig);

			// Check that the dirty flag isn't set for any of them
			BOOST_CHECK(!p_test->isDirty());
			BOOST_CHECK(!p_test_orig->isDirty());

			// Check that the fitness of both individuals differs
			BOOST_CHECK_MESSAGE (
					p_test->fitness() != p_test_orig->fitness()
					,  "\n"
					<< "p_test->fitness() = " << p_test->fitness() << "\n"
					<< "p_test_orig->fitness() = " << p_test_orig->fitness() << "\n"
			);
		}

		//------------------------------------------------------------------------------
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Tests that evaluating a dirty individual in server mode throws
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

			BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
			BOOST_CHECK_NO_THROW(p_test->setServerMode(true));
			BOOST_CHECK_THROW(p_test->fitness(), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that the process function throws for GD personalities
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

			// Set the GD personality
			BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::GD));

			// Calling the process function should throw for this personality type
			BOOST_CHECK_THROW(p_test->process(), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that the process function throws if no personality has been assigned
			boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

			// Reset the personality (sets it to NONE)
			BOOST_CHECK_NO_THROW(p_test->resetPersonality());

			// Calling the process function should throw when no personality has been assigned
			BOOST_CHECK_THROW(p_test->process(), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------
	}

#endif /* GENEVATESTING */
};

} /* namespace Tests */
} /* namespace Gem */

// Needed for serialization purposes
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Tests::GTestIndividual1)

#endif /* GTESTINDIVIDUAL1_HPP_ */
