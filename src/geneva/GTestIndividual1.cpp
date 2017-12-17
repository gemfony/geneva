/**
 * @file GTestIndividual1.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#include "geneva/GTestIndividual1.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual1)

namespace Gem {
namespace Tests {

/******************************************************************************/
/**
 * The default constructor.
 */
GTestIndividual1::GTestIndividual1()
	: GParameterSet() {
	// Fill with some data
	std::shared_ptr <Gem::Geneva::GDoubleCollection> gdc_ptr(new Gem::Geneva::GDoubleCollection(100, -10., 10.));
	std::shared_ptr <Gem::Geneva::GDoubleGaussAdaptor> gdga1(new Gem::Geneva::GDoubleGaussAdaptor(0.025, 0.1, 0., 1.));

	// Prevent changes to m_adProb
	gdga1->setAdaptAdProb(0.);

	gdc_ptr->addAdaptor(gdga1);
	gdc_ptr->randomInit(Gem::Geneva::activityMode::ACTIVEONLY, m_gr);
	this->push_back(gdc_ptr);
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual1 object
 */
GTestIndividual1::GTestIndividual1(const GTestIndividual1 &cp)
	: Gem::Geneva::GParameterSet(cp) {   /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual1::~GTestIndividual1() { /* nothing */   }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GTestIndividual1::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are dealing with a GTestIndividual1 reference independent of this object and convert the pointer
	const GTestIndividual1 *p_load = Gem::Common::g_convert_and_compare<GObject, GTestIndividual1>(cp, this);

	GToken token("GTestIndividual1", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual1, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual1, camouflaged as a GObject
 */
void GTestIndividual1::load_(const GObject *cp) {
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are dealing with a GTestIndividual1 reference independent of this object and convert the pointer
	const GTestIndividual1 *p_load = Gem::Common::g_convert_and_compare<GObject, GTestIndividual1>(cp, this);

	// Load our parent's data
	GParameterSet::load_(cp);

	// No local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject *GTestIndividual1::clone_() const {
	return new GTestIndividual1(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual1::fitnessCalculation() {
	double result = 0.;

	// Extract the first Gem::Geneva::GDoubleCollection object. In a realistic scenario, you might want
	// to add error checks here upon first invocation.
	std::shared_ptr <Gem::Geneva::GDoubleCollection> vC = at<Gem::Geneva::GDoubleCollection>(0);

	// Calculate the value of the parabola
	for (std::size_t i = 0; i < vC->size(); i++) {
		result += vC->at(i) * vC->at(i);
	}

	return result;
}

// Note: The following code is designed to mainly test parent classes

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 * only.
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual1::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if (Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

	// Change the parameter settings
	this->adapt();
	result = true;

	return result;
#else
	return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Adds a number of GDoubleObject objects to the individual
 *
 * @param nItems The number of items to be added
 */
void GTestIndividual1::addGDoubleObjects(const std::size_t &nItems) {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GDoubleObject items with adaptors to p_test1
	for (std::size_t i = 0; i < nItems; i++) {
		// Create a suitable adaptor
		std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr;

		BOOST_CHECK_NO_THROW(
			gdga_ptr = std::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0)));
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionThreshold(
			0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionMode(true)); // Always adapt

		// Create a suitable GDoubleObject object
		std::shared_ptr <GDoubleObject> gdo_ptr;

		BOOST_CHECK_NO_THROW(gdo_ptr = std::shared_ptr<GDoubleObject>(
			new GDoubleObject(-100., 100.))); // Initialization in the range -100, 100

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gdo_ptr->addAdaptor(gdga_ptr));

		// Randomly initialize the GDoubleObject object, so it is unique
		BOOST_CHECK_NO_THROW(gdo_ptr->randomInit(Gem::Geneva::activityMode::ACTIVEONLY, m_gr));

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gdo_ptr));
	}
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual1::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	// A few settings
	const std::size_t nItems = 100;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Tests whether calls to adapt() result in changes of the object
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test_old = this->clone<Gem::Tests::GTestIndividual1>();

		std::size_t nTests = 1000;

		for (std::size_t i = 0; i < nTests; i++) {
			BOOST_CHECK_NO_THROW(p_test->adapt());
			BOOST_CHECK(*p_test != *p_test_old);
			BOOST_CHECK_NO_THROW(p_test_old->GObject::load(p_test));
		}
	}

	//------------------------------------------------------------------------------

	{ // Tests customAdaptions, dirtyFlag and the effects of the fitness function. Also test setting of server-mode flag
		double evaluation = 0.;
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure this individual is not dirty
		if (p_test->isDirty()) {
			BOOST_CHECK_NO_THROW(
				evaluation = p_test->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
			BOOST_CHECK(!p_test->isDirty());
		}

		std::size_t nTests = 1000;

		double currentFitness = p_test->transformedFitness();
		double oldFitness = currentFitness;
		bool dirtyFlag = false;

		for (std::size_t i = 0; i < nTests; i++) {
			// Change the parameters without instantly triggering fitness calculation
			BOOST_CHECK_NO_THROW(p_test->customAdaptions());
			// The dirty flag should not have been set yet (done in adapt() )
			BOOST_CHECK(!p_test->isDirty());
			// Set the flag manually
			BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
			// Check that the dirty flag has indeed been set
			BOOST_CHECK(p_test->isDirty());
			if (i > 0) {
				dirtyFlag = false; // The next call should change this value
				// Once oldFitness has been set (in iterations > 0), currentFitness() should return that value here
				BOOST_CHECK_MESSAGE (
					oldFitness == p_test->getCachedFitness(0, Gem::Geneva::USETRANSFORMEDFITNESS), "\n"
																															 << "oldFitness = " <<
																															 oldFitness << "\n"
																															 <<
																															 "p_test->getCachedFitness(0, Gem::Geneva::USETRANSFORMEDFITNESS) = " <<
																															 p_test->getCachedFitness(
																																 0,
																																 Gem::Geneva::USETRANSFORMEDFITNESS) <<
																															 "\n"
																															 << "iteration = " << i <<
																															 "\n"
				);
			}
			// Trigger value calculation
			BOOST_CHECK_NO_THROW(
				currentFitness = p_test->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));

			// Check that getCachedFitness() returns the same value as fitness()
			BOOST_CHECK_MESSAGE (
				currentFitness == p_test->getCachedFitness(0, Gem::Geneva::USETRANSFORMEDFITNESS), "\n"
																															  << "currentFitness = " <<
																															  currentFitness << "\n"
																															  <<
																															  "p_test->getCachedFitness(0, Gem::Geneva::USETRANSFORMEDFITNESS) = " <<
																															  p_test->getCachedFitness(
																																  0,
																																  Gem::Geneva::USETRANSFORMEDFITNESS) <<
																															  "\n"
																															  << "iteration = " << i <<
																															  "\n"
			);
			// Check that the individual is now clean
			BOOST_CHECK(!p_test->isDirty());
			BOOST_CHECK_MESSAGE(
			// Check that the fitness has changed
				currentFitness != oldFitness, "\n"
														<< "currentFitness = " << currentFitness << "\n"
														<< "oldFitness = " << oldFitness << "\n"
														<< "iteration = " << i << "\n"
			);
			oldFitness = currentFitness;
		}
	}

	//------------------------------------------------------------------------------

	{ // Tests whether modification of one clone influences another clone
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test1 = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure the individual is clean
		if (p_test1->isDirty()) {
			BOOST_CHECK_NO_THROW(p_test1->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
			BOOST_CHECK(!p_test1->isDirty());
		}

		// Create a clone of p_test1
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test2 = p_test1->clone<Gem::Tests::GTestIndividual1>();
		// Check that the clone is identical to p_test1;
		BOOST_CHECK_NO_THROW(*p_test2 == *p_test1);

		// Modify p_test2
		BOOST_CHECK_NO_THROW(p_test2->adapt());
		// Check that it is dirty
		BOOST_CHECK(p_test2->isDirty());
		// Check that p_test1 is not dirty
		BOOST_CHECK(!p_test1->isDirty());
		// Check that the two individuals differ
		BOOST_CHECK(*p_test1 != *p_test2);
	}

	//------------------------------------------------------------------------------

	{ // Check the effects of the process function in EA mode, using the "evaluate" call
		double currentFitness = 0.;
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure the individual is clean
		BOOST_CHECK_NO_THROW(
			currentFitness = p_test->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));

		// Set the dirty flag
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());

		// Setting the dirty flag should result in DO_PROCESS being set
		BOOST_CHECK(Gem::Courtier::processingStatus::DO_PROCESS == p_test->getProcessingStatus());

		// Check that the dirty flag has indeed been set
		BOOST_CHECK(p_test->isDirty());

		// Tell the individual about its personality and duty
		BOOST_CHECK_NO_THROW(p_test->setPersonality(std::shared_ptr<GEvolutionaryAlgorithm_PersonalityTraits>(new GEvolutionaryAlgorithm_PersonalityTraits())));

		// Calling the process() function with the "evaluate" call should clear the dirty flag
		BOOST_CHECK_NO_THROW(p_test->process());

		// The dirty flag should have been cleared
		BOOST_CHECK(!p_test->isDirty());
	}

	//------------------------------------------------------------------------------

	{ // Check the process() function
		double currentFitness = 0.;
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure the individual is clean
		BOOST_CHECK_NO_THROW(
			currentFitness = p_test->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));

		// Set the dirty flag
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());

		// Check that the dirty flag has indeed been set
		BOOST_CHECK(p_test->isDirty());

		// Tell the individual about its personality
		BOOST_CHECK_NO_THROW(p_test->setPersonality(std::shared_ptr<GEvolutionaryAlgorithm_PersonalityTraits>(new GEvolutionaryAlgorithm_PersonalityTraits())));

		// Calling the process() function with the "evaluate" call should clear the dirty flag
		BOOST_CHECK_NO_THROW(p_test->process());

		// The dirty flag should have been cleared
		BOOST_CHECK(!p_test->isDirty());
	}

	//------------------------------------------------------------------------------

	{ // Check the effects of the process function in SWARM mode, using the "evaluate" call
		double currentFitness = 0.;
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure the individual is clean
		BOOST_CHECK_NO_THROW(
			currentFitness = p_test->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));

		// Set the dirty flag
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());

		// Check that the dirty flag has indeed been set
		BOOST_CHECK(p_test->isDirty());

		// Tell the individual about its personality and duty
		BOOST_CHECK_NO_THROW(
			p_test->setPersonality(std::shared_ptr<GSwarmAlgorithm_PersonalityTraits>(new GSwarmAlgorithm_PersonalityTraits())));

		// Calling the process() function with the "evaluate" call should clear the dirty flag
		BOOST_CHECK_NO_THROW(p_test->process());

		// The dirty flag should have been cleared
		BOOST_CHECK(!p_test->isDirty());
	}

	//------------------------------------------------------------------------------

	{ // Test of Gem::Common::GStdPtrVectorInterfaceT<T,GObject>::swap(...)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test1 = this->clone<Gem::Tests::GTestIndividual1>();
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test2 = this->clone<Gem::Tests::GTestIndividual1>();

		// Check that both individuals are the same
		BOOST_CHECK(*p_test1 == *p_test2);

		// Adapt p_test2, so that both individuals are different
		BOOST_CHECK_NO_THROW(p_test2->adapt());

		// Make sure both individuals are clean and evaluated
		double fitness1_old = 0., fitness2_old = 0;
		BOOST_CHECK_NO_THROW(
			fitness1_old = p_test1->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK_NO_THROW(
			fitness2_old = p_test2->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK(!p_test1->isDirty());
		BOOST_CHECK(!p_test2->isDirty());

		// Make sure the individuals are different
		BOOST_CHECK(*p_test1 != *p_test2);

		// Make sure their fitness differs
		BOOST_CHECK(p_test1->fitness() != p_test2->fitness());

		// Swap their data vectors
		BOOST_CHECK_NO_THROW(p_test1->swap(*p_test2));

		// They should now both have the dirty flag set
		BOOST_CHECK(p_test1->isDirty());
		BOOST_CHECK(p_test2->isDirty());

		// Make sure both individuals are clean and evaluated
		double fitness1_new = 0., fitness2_new = 0;
		BOOST_CHECK_NO_THROW(
			fitness1_new = p_test1->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK_NO_THROW(
			fitness2_new = p_test2->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK(!p_test1->isDirty());
		BOOST_CHECK(!p_test2->isDirty());

		// The fitness values of both individuals should effectively have been exchanged
		// Note that rounding errors might prevent fitness1_new to be == fitness2_old
		// and vice versa
		BOOST_CHECK(fabs(fitness1_new - fitness2_old) < pow(10, -8));
		BOOST_CHECK(fabs(fitness2_new - fitness1_old) < pow(10, -8));
	}

	//------------------------------------------------------------------------------

	{ // Check of the GParameterSet::customAdaptions() function
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test1 = this->clone<Gem::Tests::GTestIndividual1>();
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test2 = this->clone<Gem::Tests::GTestIndividual1>();

		// Check that both individuals are the same
		BOOST_CHECK(*p_test1 == *p_test2);

		// Make sure both individuals are clean and evaluated
		double fitness1_old = 0., fitness2_old = 0;
		BOOST_CHECK_NO_THROW(
			fitness1_old = p_test1->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK_NO_THROW(
			fitness2_old = p_test2->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK(!p_test1->isDirty());
		BOOST_CHECK(!p_test2->isDirty());

		// Extract and clone the first individual's GDoubleCollection object for later comparisons
		std::shared_ptr <Gem::Geneva::GDoubleCollection> gdc_ptr_old = p_test1->at(
			std::size_t(0))->clone<Gem::Geneva::GDoubleCollection>();

		// Adapt and evaluate the first individual
		BOOST_CHECK_NO_THROW(p_test1->customAdaptions());
		// We need to manually mark the individual as dirty
		BOOST_CHECK_NO_THROW(p_test1->setDirtyFlag());

		// The fitness of individual1 should have changed. Re-evaluate and check
		double fitness1_new = 0.;
		BOOST_CHECK_NO_THROW(
			fitness1_new = p_test1->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS));
		BOOST_CHECK(fitness1_new != fitness1_old);

		// The individuals should now differ
		BOOST_CHECK(*p_test1 != *p_test2);

		// Extract and clone the first individual's GDoubleCollection object for comparison
		std::shared_ptr <Gem::Geneva::GDoubleCollection> gdc_ptr_new = p_test1->at(
			0)->clone<Gem::Geneva::GDoubleCollection>();

		// Check that both GDoubleCollection objects differ
		BOOST_CHECK(*gdc_ptr_old != *gdc_ptr_new);
	}

	//------------------------------------------------------------------------------

	{ // Test resize_clone, resize_noclone, finding and counting of items (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Initialize with a fixed value
		BOOST_CHECK_NO_THROW(p_test->fixedValueInit<double>(42., activityMode::ALLPARAMETERS));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		// Create a copy of the first parameter item
		std::shared_ptr <GDoubleObject> search_ptr;
		Gem::Tests::GTestIndividual1::const_iterator find_cit;
		BOOST_CHECK_NO_THROW(search_ptr = p_test->at(0)->clone<GDoubleObject>());

		// Find the first item that complies to a GDoubleObject, initialized with the number 42
		BOOST_CHECK_NO_THROW(find_cit = p_test->find(search_ptr));
		BOOST_CHECK(find_cit == p_test->begin());

		// Resize, so that only one item remains, cross-check
		BOOST_CHECK_NO_THROW(p_test->resize_clone(1, search_ptr));
		BOOST_CHECK(p_test->size() == 1);

		// Use resize_clone to resize to the original size
		BOOST_CHECK_NO_THROW(p_test->resize_clone(nItems, search_ptr));

		// Count the number of items identical to search_ptr (should be nItems)
		BOOST_CHECK(p_test->count(search_ptr) == nItems);

		// Resize again to 1, using resize_noclone
		BOOST_CHECK_NO_THROW(p_test->resize_noclone(1, search_ptr));
		BOOST_CHECK(p_test->size() == 1);

		// Resize back to the original size
		BOOST_CHECK_NO_THROW(p_test->resize_noclone(nItems, search_ptr));
		BOOST_CHECK(p_test->size() == nItems);

		// Check that the pointer of the last item is identical to the one used in search_ptr
		BOOST_CHECK((p_test->back()).get() == search_ptr.get());
	}

	//------------------------------------------------------------------------------

	{ // Test insert_clone, insert_noclone (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		// Create a copy of the first parameter item
		std::shared_ptr <GDoubleObject> insert_ptr;
		BOOST_CHECK_NO_THROW(insert_ptr = p_test->at(0)->clone<GDoubleObject>());

		// Assign a fixed value to insert_ptr
		BOOST_CHECK_NO_THROW(*insert_ptr = 1.);
		BOOST_CHECK(insert_ptr->value() == 1.);

		// Insert one item and check the resulting size and value of the first item
		BOOST_CHECK_NO_THROW(p_test->insert_clone(p_test->begin(), insert_ptr));
		BOOST_CHECK(p_test->size() == nItems + 1);
		BOOST_CHECK(p_test->at<GDoubleObject>(0)->value() == 1.);

		// Find the first item which is identical to insert_ptr -- should be at the beginning
		Gem::Tests::GTestIndividual1::const_iterator find_cit;
		BOOST_CHECK_NO_THROW(find_cit = p_test->find(insert_ptr));
		BOOST_CHECK(find_cit == p_test->begin());

		// Insert another (nItems) - 1 items and count the number of items identical to insert_ptr
		BOOST_CHECK_NO_THROW(p_test->insert_clone(p_test->begin(), nItems - 1, insert_ptr));
		BOOST_CHECK(p_test->size() == 2 * nItems);
		BOOST_CHECK((std::size_t) p_test->count(insert_ptr) >= nItems);

		// Check that there is no item with the same physical address as insert_ptr
		for (std::size_t i = 0; i < p_test->size(); i++) {
			BOOST_CHECK((p_test->at(i)).get() != insert_ptr.get());
		}

		// Insert one more item at the end, using insert_noclone
		BOOST_CHECK_NO_THROW(p_test->insert_noclone(p_test->end(), insert_ptr));
		BOOST_CHECK(p_test->size() == 2 * nItems + 1);

		// There should now be exactly one item with the same address as insert_ptr (i.e. the same object)
		std::size_t nIdentical = 0;
		for (std::size_t i = 0; i < p_test->size(); i++) {
			if ((p_test->at(i)).get() == insert_ptr.get()) nIdentical++;
		}
		BOOST_CHECK(nIdentical == 1);

		// Remove the item again and check the size
		BOOST_CHECK_NO_THROW(p_test->pop_back());
		BOOST_CHECK(p_test->size() == 2 * nItems);

		// Check that there is no item left with the same address
		for (std::size_t i = 0; i < p_test->size(); i++) {
			BOOST_CHECK((p_test->at(i)).get() != insert_ptr.get());
		}

		// Insert another nItems items at the beginning, using insert_noclone; cross-check the size
		BOOST_CHECK_NO_THROW(p_test->insert_noclone(p_test->begin(), nItems, insert_ptr));
		BOOST_CHECK(p_test->size() == 3 * nItems);

		// There should again be exactly one item with the same address as insert_ptr (i.e. the same object)
		nIdentical = 0;
		for (std::size_t i = 0; i < p_test->size(); i++) {
			if ((p_test->at(i)).get() == insert_ptr.get()) nIdentical++;
		}
		BOOST_CHECK(nIdentical == 1);

		// The identical item should be at the very beginning of the collection
		BOOST_CHECK((p_test->at<GDoubleObject>(0)).get() == insert_ptr.get());
	}

	//------------------------------------------------------------------------------

	{ // Test push_back_clone and push_back_noclone (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		// Create a copy of the first parameter item
		std::shared_ptr <GDoubleObject> pushback_ptr;
		BOOST_CHECK_NO_THROW(pushback_ptr = p_test->at(0)->clone<GDoubleObject>());

		// Assign a fixed value to pushback_ptr
		BOOST_CHECK_NO_THROW(*pushback_ptr = 1.);
		BOOST_CHECK(pushback_ptr->value() == 1.);

		// Push back the cloned item to the collection; cross-check the size and the pointers
		BOOST_CHECK_NO_THROW(p_test->push_back_clone(pushback_ptr));
		BOOST_CHECK(p_test->size() == nItems + 1);
		BOOST_CHECK((p_test->back()).get() != pushback_ptr.get());

		// Push back the un-cloned item to the collection; cross-check the size and the pointers
		BOOST_CHECK_NO_THROW(p_test->push_back_noclone(pushback_ptr));
		BOOST_CHECK(p_test->size() == nItems + 2);
		BOOST_CHECK((p_test->back()).get() == pushback_ptr.get());
	}

	//------------------------------------------------------------------------------

	{ // Test retrieval of a data copy (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		std::vector<std::shared_ptr < GParameterBase>> dataCopy;
		BOOST_CHECK_NO_THROW(p_test->getDataCopy(dataCopy));

		// Check the size and content
		BOOST_CHECK(dataCopy.size() == p_test->size() && p_test->size() != 0);
		for (std::size_t i = 0; i < p_test->size(); i++) {
			BOOST_CHECK((p_test->at(i)).get() != dataCopy.at(i).get());
		}
	}

	//------------------------------------------------------------------------------


	{ // Check setting and retrieval of the current personality status and whether the personalities themselves can be accessed
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();
		std::shared_ptr <GPersonalityTraits> p_pt;

		// Reset the personality type
		BOOST_CHECK_NO_THROW(p_test->resetPersonality());
		BOOST_CHECK_MESSAGE(
			p_test->getPersonality() == "PERSONALITY_NONE", "\n"
																			<< "p_test->getPersonality() = " << p_test->getPersonality() <<
																			"\n"
																			<< "expected PERSONALITY_NONE\n"
		);

		// Set the personality type to EA
		BOOST_CHECK_NO_THROW(p_test->setPersonality(std::shared_ptr<GEvolutionaryAlgorithm_PersonalityTraits>(new GEvolutionaryAlgorithm_PersonalityTraits())));
		BOOST_CHECK_MESSAGE(
			p_test->getPersonality() == "GEvolutionaryAlgorithm_PersonalityTraits", "\n"
																				 << "p_test->getPersonality() = " <<
																				 p_test->getPersonality() << "\n"
																				 << "expected EA\n"
		);

		// Try to retrieve a GEvolutionaryAlgorithm_PersonalityTraits object and check that the smart pointer actually points somewhere
		std::shared_ptr <GEvolutionaryAlgorithm_PersonalityTraits> p_pt_ea;
		BOOST_CHECK_NO_THROW(p_pt_ea = p_test->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>());
		BOOST_CHECK(p_pt_ea);
		p_pt_ea.reset();

		// Retrieve a base pointer to the EA object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to GD
		BOOST_CHECK_NO_THROW(p_test->setPersonality(std::shared_ptr<GGradientDescent_PersonalityTraits>(new GGradientDescent_PersonalityTraits())));
		BOOST_CHECK_MESSAGE(
			p_test->getPersonality() == "GGradientDescent_PersonalityTraits", "\n"
																				 << "p_test->getPersonality() = " <<
																				 p_test->getPersonality() << "\n"
																				 << "expected GGradientDescent_PersonalityTraits\n"
		);

		// Try to retrieve a GGradientDescent_PersonalityTraits object and check that the smart pointer actually points somewhere
		std::shared_ptr <GGradientDescent_PersonalityTraits> p_pt_gd;
		BOOST_CHECK_NO_THROW(p_pt_gd = p_test->getPersonalityTraits<GGradientDescent_PersonalityTraits>());
		BOOST_CHECK(p_pt_gd);
		p_pt_gd.reset();

		// Retrieve a base pointer to the GD object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to SWARM
		BOOST_CHECK_NO_THROW(
			p_test->setPersonality(std::shared_ptr<GSwarmAlgorithm_PersonalityTraits>(new GSwarmAlgorithm_PersonalityTraits())));
		BOOST_CHECK_MESSAGE(
			p_test->getPersonality() == "GSwarmAlgorithm_PersonalityTraits", "\n"
																					 << "p_test->getPersonality() = " <<
																					 p_test->getPersonality() << "\n"
																					 << "expected GSwarmAlgorithm_PersonalityTraits\n"
		);

		// Try to retrieve a GSwarmAlgorithm_PersonalityTraits object and check that the smart pointer actually points somewhere
		std::shared_ptr <GSwarmAlgorithm_PersonalityTraits> p_pt_swarm;
		BOOST_CHECK_NO_THROW(p_pt_swarm = p_test->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>());
		BOOST_CHECK(p_pt_swarm);
		p_pt_swarm.reset();

		// Retrieve a base pointer to the SWARM object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to PERSONALITY_NONE
		BOOST_CHECK_NO_THROW(p_test->resetPersonality());
		BOOST_CHECK_MESSAGE(
			p_test->getPersonality() == "PERSONALITY_NONE", "\n"
																			<< "p_test->getPersonality() = " << p_test->getPersonality() <<
																			"\n"
																			<< "expected PERSONALITY_NONE\n"
		);
	}

	// --------------------------------------------------------------------------
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("Gem::Tests::GTestIndividual1::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual1::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	// A few settings
	const std::size_t nItems = 100;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Tests that evaluating a dirty individual in "server mode" throws
		std::shared_ptr<Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
		BOOST_CHECK_THROW(
         p_test->transformedFitness()
         , gemfony_exception
      );
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

	{ // Test that trying to count an empty smart pointer throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to count the number of occurrences of an empty smart pointer. Should throw
		BOOST_CHECK_THROW(p_test->count(std::shared_ptr<GDoubleObject>()), gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to find an empty smart pointer throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to find an empty smart pointer. Should throw
		BOOST_CHECK_THROW(p_test->find(std::shared_ptr<GDoubleObject>()), gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_noclone(pos, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert an empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_noclone(p_test->begin(), std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_noclone(pos, amount, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_noclone(p_test->begin(), 10, std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_clone(pos, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_clone(p_test->begin(), std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_clone(pos, amount, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_clone(p_test->begin(), 10, std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to add an empty smart pointer with push_back_clone(item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to attach an empty smart pointer Should throw
		BOOST_CHECK_THROW(p_test->push_back_clone(std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to add an empty smart pointer with push_back_noclone(item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to attach an empty smart pointer Should throw
		BOOST_CHECK_THROW(p_test->push_back_noclone(std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to resize an empty collection with resize(amount) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure p_test is empty
		BOOST_CHECK_NO_THROW(p_test->clear());
		BOOST_CHECK(p_test->empty());

		// Try to resize an empty collection
		BOOST_CHECK_THROW(p_test->resize(10), gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to resize an empty collection with resize_noclone(amount, item) throws if item is an empty smart pointer(Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure p_test is empty
		BOOST_CHECK_NO_THROW(p_test->clear());
		BOOST_CHECK(p_test->empty());

		// Try to resize an empty collection
		BOOST_CHECK_THROW(p_test->resize_noclone(10, std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to resize an empty collection with resize_clone(amount, item) throws if item is an empty smart pointer(Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		std::shared_ptr <Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

		// Make sure p_test is empty
		BOOST_CHECK_NO_THROW(p_test->clear());
		BOOST_CHECK(p_test->empty());

		// Try to resize an empty collection
		BOOST_CHECK_THROW(p_test->resize_clone(10, std::shared_ptr<GDoubleObject>()),
								gemfony_exception);
	}

	//------------------------------------------------------------------------------

#ifdef DEBUG
   { // Test that retrieval of an EA personality traits object from an uninitialized pointer throws in DEBUG mode
      std::shared_ptr<Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

      // Make sure the personality type is set to PERSONALITY_NONE
      BOOST_CHECK_NO_THROW(p_test->resetPersonality());

      // Trying to retrieve an EA personality object should throw
      std::shared_ptr<GEvolutionaryAlgorithm_PersonalityTraits> p_pt_ea;
      BOOST_CHECK_THROW(p_pt_ea = p_test->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>(), gemfony_exception);
   }
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
   { // Test that retrieval of an EA personality traits object from an individual with SWARM personality throws
      std::shared_ptr<Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

      // Make sure the personality type is set to SWARM
      BOOST_CHECK_NO_THROW(p_test->setPersonality(std::shared_ptr<GSwarmAlgorithm_PersonalityTraits>(new GSwarmAlgorithm_PersonalityTraits())));

      // Trying to retrieve an EA personality object should throw
      BOOST_CHECK_THROW(p_test->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>(), gemfony_exception);
   }
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
   { // Test that retrieval of a personality traits base object from an individual without personality throws
      std::shared_ptr<Gem::Tests::GTestIndividual1> p_test = this->clone<Gem::Tests::GTestIndividual1>();

      // Make sure the personality type is set to PERSONALITY_NONE
      BOOST_CHECK_NO_THROW(p_test->resetPersonality());

      // Trying to retrieve an EA personality object should throw
      std::shared_ptr<GPersonalityTraits> p_pt;
      BOOST_CHECK_THROW(p_pt = p_test->getPersonalityTraits(), gemfony_exception);
   }
#endif /* DEBUG */

	//------------------------------------------------------------------------------
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GTestIndividual1::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */
