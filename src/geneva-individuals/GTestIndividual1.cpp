/**
 * @file GTestIndividual1.cpp
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

#include "geneva-individuals/GTestIndividual1.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual1)

namespace Gem
{
namespace Tests
{

/********************************************************************************************/
/**
 * The default constructor.
 */
GTestIndividual1::GTestIndividual1()
: GParameterSet()
, fakeUpdateOnStall_(false)
{
	// Fill with some data
	boost::shared_ptr<Gem::Geneva::GDoubleCollection > gdc_ptr(new Gem::Geneva::GDoubleCollection(100, -10., 10.));
	boost::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga1(new Gem::Geneva::GDoubleGaussAdaptor(1.,0.6,0.,2.));
	gdc_ptr->addAdaptor(gdga1);
	gdc_ptr->randomInit();
	this->push_back(gdc_ptr);
}

/********************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual1 object
 */
GTestIndividual1::GTestIndividual1(const GTestIndividual1& cp)
: Gem::Geneva::GParameterSet(cp)
, fakeUpdateOnStall_(cp.fakeUpdateOnStall_)
{	/* nothing */ }

/********************************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual1::~GTestIndividual1()
{ /* nothing */	}

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GTestIndividual1 object
 * @return A constant reference to this object
 */
const GTestIndividual1& GTestIndividual1::operator=(const GTestIndividual1& cp){
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
bool GTestIndividual1::operator==(const GTestIndividual1& cp) const {
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
bool GTestIndividual1::operator!=(const GTestIndividual1& cp) const {
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
boost::optional<std::string> GTestIndividual1::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	const GTestIndividual1 *p_load = conversion_cast<GTestIndividual1>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(Gem::Geneva::GParameterSet::checkRelationshipWith(cp, e, limit, "GTestIndividual1", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GTestIndividual1", fakeUpdateOnStall_, p_load->fakeUpdateOnStall_, "fakeUpdateOnStall_", "p_load->fakeUpdateOnStall_", e , limit));

	return evaluateDiscrepancies("GTestIndividual1", caller, deviations, e);
}

/********************************************************************************************/
/**
 * Sets the fakeUpdateOnStall_ variable. When set, this object's customUpdateOnStall() function
 * will return true.
 *
 * @param fakeUpdateOnStall The desired new value for the fakeUpdateOnStall_ flag
 */
void GTestIndividual1::setFakeCustomUpdateOnStall(const bool& fakeUpdateOnStall) {
	fakeUpdateOnStall_ = fakeUpdateOnStall;
}

/********************************************************************************************/
/**
 * Retrieves the current value of the fakeUpdateOnStall_ flag
 *
 * @return The current value of the fakeUpdateOnStall_ flag
 */
bool GTestIndividual1::getFakeCustomUpdateOnStall() const {
	return fakeUpdateOnStall_;
}

/********************************************************************************************/
/**
 * An overload of GIndividual::customUpdateOnStall() that can fake updates.
 *
 * @return A boolean indicating whether an update was performed and the object has changed
 */
bool GTestIndividual1::customUpdateOnStall() {
	if(fakeUpdateOnStall_) return true;
	else return false;
}

/********************************************************************************************/
/**
 * Loads the data of another GTestIndividual1, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual1, camouflaged as a GObject
 */
void GTestIndividual1::load_(const GObject* cp)
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	const GTestIndividual1 *p_load = conversion_cast<GTestIndividual1>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// Load our local data
	fakeUpdateOnStall_ = p_load->fakeUpdateOnStall_;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GTestIndividual1::clone_() const {
	return new GTestIndividual1(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual1::fitnessCalculation(const std::size_t& id){
	double result = 0.;

	// Extract the first Gem::Geneva::GDoubleCollection object. In a realistic scenario, you might want
	// to add error checks here upon first invocation.
	boost::shared_ptr<Gem::Geneva::GDoubleCollection> vC = at<Gem::Geneva::GDoubleCollection>(0);

	// Calculate the value of the parabola
	for(std::size_t i=0; i<vC->size(); i++) {
		result += vC->at(i) * vC->at(i);
	}

	return result;
}

#ifdef GENEVATESTING
// Note: The following code is designed to mainly test parent classes

/******************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual1::modify_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

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
 * Adds a number of GDoubleObject objects to the individual
 *
 * @param nItems The number of items to be added
 */
void GTestIndividual1::addGDoubleObjects(const std::size_t& nItems) {
	using namespace Gem::Geneva;

	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GDoubleObject items with adaptors to p_test1
	for(std::size_t i=0; i<nItems; i++) {
		// Create a suitable adaptor
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr;

		BOOST_CHECK_NO_THROW(gdga_ptr = boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0)));
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionThreshold(0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionMode(true)); // Always adapt

		// Create a suitable GDoubleObject object
		boost::shared_ptr<GDoubleObject> gdo_ptr;

		BOOST_CHECK_NO_THROW(gdo_ptr = boost::shared_ptr<GDoubleObject>(new GDoubleObject(-100., 100.))); // Initialization in the range -100, 100

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gdo_ptr->addAdaptor(gdga_ptr));

		// Randomly initialize the GDoubleObject object, so it is unique
		BOOST_CHECK_NO_THROW(gdo_ptr->randomInit());

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gdo_ptr));
	}
}

/******************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual1::specificTestsNoFailureExpected_GUnitTests() {
	using namespace Gem::Geneva;

	// A few settings
	const std::size_t nItems = 100;

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

	{ // Check the effects of the process function in EA mode, using the "adapt" call, with multiple allowed processing cycles, in an iteration > 0
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

		// Allow just multiple processing cycles, with an iteration > 0
		BOOST_CHECK_NO_THROW(p_test->setProcessingCycles(5));
		BOOST_CHECK_NO_THROW(p_test->setParentAlgIteration(3));
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

	{ // Check the effects of the process function in EA mode, using the "evaluate" call
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make sure the individual is clean
		BOOST_CHECK_NO_THROW(p_test->fitness());

		// Set the dirty flag
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());

		// Check that the dirty flag has indeed been set
		BOOST_CHECK(p_test->isDirty());

		// Tell the individual about its personality and duty
		BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::EA));
		BOOST_CHECK_NO_THROW(p_test->getPersonalityTraits()->setCommand("evaluate"));

		// Calling the process() function with the "evaluate" call should clear the dirty flag
		BOOST_CHECK_NO_THROW(p_test->process());

		// The dirty flag should have been cleared
		BOOST_CHECK(!p_test->isDirty());
	}

	//------------------------------------------------------------------------------

	{ // Check that processing works even in server mode and that this mode is restored
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make sure the individual is clean
		BOOST_CHECK_NO_THROW(p_test->fitness());

		// Set the dirty flag
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());

		// Check that the dirty flag has indeed been set
		BOOST_CHECK(p_test->isDirty());

		// Tell the individual about its personality
		BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::EA));

		// Set the server mode, so calling the fitness function throws
		BOOST_CHECK_NO_THROW(p_test->setServerMode(true));

		// Make sure the server mode is indeed set
		BOOST_CHECK(p_test->serverMode());

		// Set the command
		BOOST_CHECK_NO_THROW(p_test->getPersonalityTraits()->setCommand("evaluate"));

		// Calling the process() function with the "evaluate" call should clear the dirty flag
		BOOST_CHECK_NO_THROW(p_test->process());

		// The dirty flag should have been cleared
		BOOST_CHECK(!p_test->isDirty());

		// Check that the individual is still in server mode
		BOOST_CHECK(p_test->serverMode());
	}

	//------------------------------------------------------------------------------

	{ // Check the effects of the process function in SWARM mode, using the "evaluate" call
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make sure the individual is clean
		BOOST_CHECK_NO_THROW(p_test->fitness());

		// Set the dirty flag
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());

		// Check that the dirty flag has indeed been set
		BOOST_CHECK(p_test->isDirty());

		// Tell the individual about its personality and duty
		BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::SWARM));
		BOOST_CHECK_NO_THROW(p_test->getPersonalityTraits()->setCommand("evaluate"));

		// Calling the process() function with the "evaluate" call should clear the dirty flag
		BOOST_CHECK_NO_THROW(p_test->process());

		// The dirty flag should have been cleared
		BOOST_CHECK(!p_test->isDirty());
	}

	//------------------------------------------------------------------------------

	{ // Check the effects of the customUpdateOnStall() function
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make the individual fake updates
		p_test->setFakeCustomUpdateOnStall(true);

		// Check that customUpdateOnStall() indeed returns "true"
		BOOST_CHECK(p_test->customUpdateOnStall() == true);

		// Make this a parent individual in EA mode
		BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::EA));
		BOOST_CHECK_NO_THROW(p_test->getEAPersonalityTraits()->setIsParent());

		// Perform the actual update
		bool updatePerformed = false;
		BOOST_CHECK_NO_THROW(updatePerformed = p_test->updateOnStall());

		// Check whether an update was performed
		BOOST_CHECK(updatePerformed == true);

		// Check that the individual's dirty flag is set
		BOOST_CHECK(p_test->isDirty());
	}

	//------------------------------------------------------------------------------

	{ // Test of GMutableSetT<T>::swap(const GMutableSetT<T>&)
		boost::shared_ptr<GTestIndividual1> p_test1 = this->clone<GTestIndividual1>();
		boost::shared_ptr<GTestIndividual1> p_test2 = this->clone<GTestIndividual1>();

		// Check that both individuals are the same
		BOOST_CHECK(*p_test1 == *p_test2);

		// Adapt p_test2, so that both individuals are different
		BOOST_CHECK_NO_THROW(p_test2->adapt());

		// Make sure both individuals are clean and evaluated
		double fitness1_old = 0., fitness2_old = 0;
		BOOST_CHECK_NO_THROW(fitness1_old = p_test1->fitness());
		BOOST_CHECK_NO_THROW(fitness2_old = p_test2->fitness());
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
		BOOST_CHECK_NO_THROW(fitness1_new = p_test1->fitness());
		BOOST_CHECK_NO_THROW(fitness2_new = p_test2->fitness());
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
		boost::shared_ptr<GTestIndividual1> p_test1 = this->clone<GTestIndividual1>();
		boost::shared_ptr<GTestIndividual1> p_test2 = this->clone<GTestIndividual1>();

		// Check that both individuals are the same
		BOOST_CHECK(*p_test1 == *p_test2);

		// Make sure both individuals are clean and evaluated
		double fitness1_old = 0., fitness2_old = 0;
		BOOST_CHECK_NO_THROW(fitness1_old = p_test1->fitness());
		BOOST_CHECK_NO_THROW(fitness2_old = p_test2->fitness());
		BOOST_CHECK(!p_test1->isDirty());
		BOOST_CHECK(!p_test2->isDirty());

		// Extract and clone the first individual's GDoubleCollection object for later comparisons
		boost::shared_ptr<Gem::Geneva::GDoubleCollection> gdc_ptr_old = p_test1->at(0)->clone<Gem::Geneva::GDoubleCollection>();

		// Adapt and evaluate the first individual
		BOOST_CHECK_NO_THROW(p_test1->customAdaptions());
		// We need to manually mark the individual as dirty
		BOOST_CHECK_NO_THROW(p_test1->setDirtyFlag());

		// The fitness of individual1 should have changed. Re-evaluate and check
		double fitness1_new = 0.;
		BOOST_CHECK_NO_THROW(fitness1_new = p_test1->fitness());
		BOOST_CHECK(fitness1_new != fitness1_old);

		// The individuals should now differ
		BOOST_CHECK(*p_test1 != *p_test2);

		// Extract and clone the first individual's GDoubleCollection object for comparison
		boost::shared_ptr<Gem::Geneva::GDoubleCollection> gdc_ptr_new = p_test1->at(0)->clone<Gem::Geneva::GDoubleCollection>();

		// Check that both GDoubleCollection objects differ
		BOOST_CHECK(*gdc_ptr_old != *gdc_ptr_new);
	}

	//------------------------------------------------------------------------------

	{ // Test resize_clone, resize_noclone, finding and counting of items (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Initialize with a fixed value
		BOOST_CHECK_NO_THROW(p_test->fpFixedValueInit(42.));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		// Create a copy of the first parameter item
		boost::shared_ptr<GDoubleObject> search_ptr;
		GTestIndividual1::const_iterator find_cit;
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
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		// Create a copy of the first parameter item
		boost::shared_ptr<GDoubleObject> insert_ptr;
		BOOST_CHECK_NO_THROW(insert_ptr = p_test->at(0)->clone<GDoubleObject>());

		// Assign a fixed value to insert_ptr
		BOOST_CHECK_NO_THROW(*insert_ptr = 1.);
		BOOST_CHECK(insert_ptr->value() == 1.);

		// Insert one item and check the resulting size and value of the first item
		BOOST_CHECK_NO_THROW(p_test->insert_clone(p_test->begin(), insert_ptr));
		BOOST_CHECK(p_test->size() == nItems + 1);
		BOOST_CHECK(p_test->at<GDoubleObject>(0)->value() == 1.);

		// Find the first item which is identical to insert_ptr -- should be at the beginning
		GTestIndividual1::const_iterator find_cit;
		BOOST_CHECK_NO_THROW(find_cit = p_test->find(insert_ptr));
		BOOST_CHECK(find_cit == p_test->begin());

		// Insert another (nItems) - 1 items and count the number of items identical to insert_ptr
		BOOST_CHECK_NO_THROW(p_test->insert_clone(p_test->begin(), nItems - 1, insert_ptr));
		BOOST_CHECK(p_test->size() == 2*nItems);
		BOOST_CHECK((std::size_t)p_test->count(insert_ptr) >= nItems);

		// Check that there is no item with the same physical address as insert_ptr
		for(std::size_t i=0; i<p_test->size(); i++) {
			BOOST_CHECK((p_test->at(i)).get() != insert_ptr.get());
		}

		// Insert one more item at the end, using insert_noclone
		BOOST_CHECK_NO_THROW(p_test->insert_noclone(p_test->end(), insert_ptr));
		BOOST_CHECK(p_test->size() == 2*nItems + 1);

		// There should now be exactly one item with the same address as insert_ptr (i.e. the same object)
		std::size_t nIdentical = 0;
		for(std::size_t i=0; i<p_test->size(); i++) {
			if((p_test->at(i)).get() == insert_ptr.get()) nIdentical++;
		}
		BOOST_CHECK(nIdentical == 1);

		// Remove the item again and check the size
		BOOST_CHECK_NO_THROW(p_test->pop_back());
		BOOST_CHECK(p_test->size() == 2*nItems);

		// Check that there is no item left with the same address
		for(std::size_t i=0; i<p_test->size(); i++) {
			BOOST_CHECK((p_test->at(i)).get() != insert_ptr.get());
		}

		// Insert another nItems items at the beginning, using insert_noclone; cross-check the size
		BOOST_CHECK_NO_THROW(p_test->insert_noclone(p_test->begin(), nItems, insert_ptr));
		BOOST_CHECK(p_test->size() == 3*nItems);

		// There should again be exactly one item with the same address as insert_ptr (i.e. the same object)
		nIdentical = 0;
		for(std::size_t i=0; i<p_test->size(); i++) {
			if((p_test->at(i)).get() == insert_ptr.get()) nIdentical++;
		}
		BOOST_CHECK(nIdentical == 1);

		// The identical item should be at the very beginning of the collection
		BOOST_CHECK((p_test->at<GDoubleObject>(0)).get() == insert_ptr.get());
	}

	//------------------------------------------------------------------------------

	{ // Test push_back_clone and push_back_noclone (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		// Create a copy of the first parameter item
		boost::shared_ptr<GDoubleObject> pushback_ptr;
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
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Check the current size
		BOOST_CHECK(p_test->size() == nItems);

		std::vector<boost::shared_ptr<GParameterBase> > dataCopy;
		BOOST_CHECK_NO_THROW(p_test->getDataCopy(dataCopy));

		// Check the size and content
		BOOST_CHECK(dataCopy.size() == p_test->size() && p_test->size() != 0);
		for(std::size_t i=0; i<p_test->size(); i++) {
			BOOST_CHECK((p_test->at(i)).get() != dataCopy.at(i).get());
		}
	}

	//------------------------------------------------------------------------------
}

/******************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual1::specificTestsFailuresExpected_GUnitTests() {
	using namespace Gem::Geneva;

	// A few settings
	const std::size_t nItems = 100;

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

#ifdef DEBUG
	{ // Trying to run the process call on a dirty individual with the "adapt" command,
		// using multiple processing cycles in an iteration > 0 should throw in DEBUG mode
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::EA));
		BOOST_CHECK_NO_THROW(p_test->getPersonalityTraits()->setCommand("adapt"));

		// Make sure the individual is dirty
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
		// Cross check
		BOOST_CHECK(p_test->isDirty());

		// Allow just multiple processing cycles, with an iteration > 0
		BOOST_CHECK_NO_THROW(p_test->setProcessingCycles(5));
		BOOST_CHECK_NO_THROW(p_test->setParentAlgIteration(3));

		// Calling the process function should throw when the process() function is called
		// on a dirty individual that allows multiple processing cycles
		BOOST_CHECK_THROW(p_test->process(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Check that calling GParameterSet::updateOnStall throws in EA mode, if this is not a parent
		// The exception will only be triggered in DEBUG mode
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make the individual fake updates
		p_test->setFakeCustomUpdateOnStall(true);

		// Check that customUpdateOnStall() indeed returns "true"
		BOOST_CHECK(p_test->customUpdateOnStall() == true);

		// Make this a parent individual in EA mode
		BOOST_CHECK_NO_THROW(p_test->setPersonality(Gem::Geneva::EA));
		BOOST_CHECK_NO_THROW(p_test->getEAPersonalityTraits()->setIsChild());

		// Perform the actual update
		BOOST_CHECK_THROW(p_test->updateOnStall(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

	{ // Test that trying to count an empty smart pointer throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to count the number of occurances of an empty smart pointer. Should throw
		BOOST_CHECK_THROW(p_test->count(boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to find an empty smart pointer throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to find an empty smart pointer. Should throw
		BOOST_CHECK_THROW(p_test->find(boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_noclone(pos, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_noclone(p_test->begin(), boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_noclone(pos, amount, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_noclone(p_test->begin(), 10, boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_clone(pos, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_clone(p_test->begin(), boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to insert an empty smart pointer with insert_clone(pos, amount, item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to insert a number of empty smart pointers. Should throw
		BOOST_CHECK_THROW(p_test->insert_clone(p_test->begin(), 10, boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to add an empty smart pointer with push_back_clone(item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to attach an empty smart pointer Should throw
		BOOST_CHECK_THROW(p_test->push_back_clone(boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to add an empty smart pointer with push_back_noclone(item) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Add a few data items
		BOOST_CHECK_NO_THROW(p_test->addGDoubleObjects(nItems));

		// Try to attach an empty smart pointer Should throw
		BOOST_CHECK_THROW(p_test->push_back_noclone(boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to resize an empty collection with resize(amount) throws (Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make sure p_test is empty
		BOOST_CHECK_NO_THROW(p_test->clear());
		BOOST_CHECK(p_test->empty());

		// Try to resize an empty collection
		BOOST_CHECK_THROW(p_test->resize(10), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to resize an empty collection with resize_noclone(amount, item) throws if item is an empty smart pointer(Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make sure p_test is empty
		BOOST_CHECK_NO_THROW(p_test->clear());
		BOOST_CHECK(p_test->empty());

		// Try to resize an empty collection
		BOOST_CHECK_THROW(p_test->resize_noclone(10, boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that trying to resize an empty collection with resize_clone(amount, item) throws if item is an empty smart pointer(Test of GStdPtrVectorInterfaceT<GParameterBase> functionality)
		boost::shared_ptr<GTestIndividual1> p_test = this->clone<GTestIndividual1>();

		// Make sure p_test is empty
		BOOST_CHECK_NO_THROW(p_test->clear());
		BOOST_CHECK(p_test->empty());

		// Try to resize an empty collection
		BOOST_CHECK_THROW(p_test->resize_clone(10, boost::shared_ptr<GDoubleObject>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------
}

/******************************************************************/

#endif /* GENEVATESTING */

} /* namespace Tests */
} /* namespace Gem */
