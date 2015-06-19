/**
 * @file GSerialEA.cpp
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
#include "geneva/GSerialEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialEA)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GSerialEA::GSerialEA()
	: GBaseEA() { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp Reference to another GSerialEA object
 */
GSerialEA::GSerialEA(const GSerialEA &cp)
	: GBaseEA(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GSerialEA::~GSerialEA() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GSerialEA &GSerialEA::operator=(const GSerialEA &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Loads the data from another GSerialEA object.
 *
 * @param vp Pointer to another GSerialEA object, camouflaged as a GObject
 */
void GSerialEA::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GSerialEA * p_load = Gem::Common::g_convert_and_compare<GObject, GSerialEA>(cp, this);

	// First load our parent class'es data ...
	GBaseEA::load_(cp);

	// no local data ...
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialEA::clone_() const {
	return new GSerialEA(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GSerialEA object
 *
 * @param  cp A constant reference to another GSerialEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialEA::operator==(const GSerialEA &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GSerialEA object
 *
 * @param  cp A constant reference to another GSerialEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialEA::operator!=(const GSerialEA &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GSerialEA::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GSerialEA reference independent of this object and convert the pointer
	const GSerialEA *p_load = Gem::Common::g_convert_and_compare<GObject, GSerialEA>(cp, this);

	GToken token("GSerialEA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseEA>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSerialEA::name() const {
	return std::string("GSerialEA");
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialEA::init() {
	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::init();

	// Put own initialization code here
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialEA::finalize() {
	// Put own finalization code here

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GSerialEA::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GBaseEA::addConfigurationOptions(gpb);

	// No local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GSerialEA::getIndividualCharacteristic() const {
	return std::string("GENEVA_SERIALOPTALG");
}

/******************************************************************************/
/**
 * Adapt all children in sequence. Evaluation is done in a seperate function (evaluateChildren).
 */
void GSerialEA::adaptChildren() {
	boost::tuple<std::size_t, std::size_t> range = getAdaptionRange();
	std::vector<std::shared_ptr < GParameterSet>> ::iterator it;

	for (it = data.begin() + boost::get<0>(range); it != data.begin() + boost::get<1>(range); ++it) {
		(*it)->adapt();
	}
}

/******************************************************************************/
/**
 * Evaluate all children (and possibly parents, depending on the iteration)
 */
void GSerialEA::runFitnessCalculation() {
	boost::tuple<std::size_t, std::size_t> range = getEvaluationRange();
	std::vector<std::shared_ptr < GParameterSet>> ::iterator it;

#ifdef DEBUG
   // There should be no situation in which a "clean" child is submitted
   // through this function. There MAY be situations, where in the first iteration
   // parents are clean, e.g. when they were extracted from another optimization.
   for(std::size_t i=this->getNParents(); i<this->size(); i++) {
      if(!this->at(i)->isDirty()) {
         glogger
         << "In GSerialThreadedEA::runFitnessCalculation(): Error!" << std::endl
         << "Tried to evaluate children in range " << boost::get<0>(range) << " - " << boost::get<1>(range) << std::endl
         << "but found \"clean\" individual in position " << i << std::endl
         << GEXCEPTION;
      }
   }
#endif

	for (it = data.begin() + boost::get<0>(range); it != data.begin() + boost::get<1>(range); ++it) {
		// Perform the actual evaluation
		(*it)->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS);
	}
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSerialEA::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseEA::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialEA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialEA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	GBaseEA::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------------

	{ // Call the parent class'es function
		std::shared_ptr <GSerialEA> p_test = this->clone<GSerialEA>();

		// Fill p_test with individuals
		p_test->fillWithObjects();

		// Run the parent class'es tests
		p_test->GBaseEA::specificTestsNoFailureExpected_GUnitTests();
	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialEA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialEA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseEA::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
