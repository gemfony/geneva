/**
 * @file GSerialSA.cpp
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
#include "geneva/GSerialSA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialSA)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GSerialSA::GSerialSA()
	: GBaseSA() { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp Reference to another GSerialSA object
 */
GSerialSA::GSerialSA(const GSerialSA &cp)
	: GBaseSA(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GSerialSA::~GSerialSA() { /* nothing */ }

/******************************************************************************/
/**
 * Loads the data from another GSerialSA object.
 *
 * @param vp Pointer to another GSerialSA object, camouflaged as a GObject
 */
void GSerialSA::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GSerialSA * p_load = Gem::Common::g_convert_and_compare<GObject, GSerialSA>(cp, this);

	// First load our parent class'es data ...
	GBaseSA::load_(cp);

	// no local data ...
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialSA::clone_() const {
	return new GSerialSA(*this);
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GSerialSA &GSerialSA::operator=(const GSerialSA &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GSerialSA object
 *
 * @param  cp A constant reference to another GSerialSA object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialSA::operator==(const GSerialSA &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GSerialSA object
 *
 * @param  cp A constant reference to another GSerialSA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialSA::operator!=(const GSerialSA &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
void GSerialSA::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GSerialSA reference independent of this object and convert the pointer
	const GSerialSA *p_load = Gem::Common::g_convert_and_compare<GObject, GSerialSA>(cp, this);

	GToken token("GSerialSA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseSA>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSerialSA::name() const {
	return std::string("GSerialSA");
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialSA::init() {
	// GBaseSA sees exactly the environment it would when called from its own class
	GBaseSA::init();

	// Put own initialization code here
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialSA::finalize() {
	// Put own finalization code here

	// GBaseSA sees exactly the environment it would when called from its own class
	GBaseSA::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GSerialSA::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GBaseSA::addConfigurationOptions(gpb);

	// No local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GSerialSA::getIndividualCharacteristic() const {
	return std::string("GENEVA_SERIALOPTALG");
}

/******************************************************************************/
/**
 * Adapt all children in sequence. Evaluation is done in a seperate function (runFitnessCalculation).
 */
void GSerialSA::adaptChildren() {
	std::tuple<std::size_t, std::size_t> range = getAdaptionRange();
	std::vector<std::shared_ptr < GParameterSet>> ::iterator
	it;

	for (it = data.begin() + std::get<0>(range); it != data.begin() + std::get<1>(range); ++it) {
		(*it)->adapt();
	}
}

/******************************************************************************/
/**
 * Evaluate all children (and possibly parents, depending on the iteration)
 */
void GSerialSA::runFitnessCalculation() {
	std::tuple<std::size_t, std::size_t> range = getEvaluationRange();
	std::vector<std::shared_ptr < GParameterSet>> ::iterator
	it;

#ifdef DEBUG
   // There should be no situation in which a "clean" child is submitted
   // through this function. There MAY be situations, where in the first iteration
   // parents are clean, e.g. when they were extracted from another optimization.
   for(std::size_t i=this->getNParents(); i<this->size(); i++) {
      if(!this->at(i)->isDirty()) {
         glogger
         << "In GSerialSA::runFitnessCalculation(): Error!" << std::endl
         << "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
         << "but found \"clean\" individual in position " << i << std::endl
         << GEXCEPTION;
      }
   }
#endif

	for (it = data.begin() + std::get<0>(range); it != data.begin() + std::get<1>(range); ++it) {
		// Perform the actual evaluation
		(*it)->process();
	}
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSerialSA::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseSA::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialSA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	// Call the parent class'es function
	GBaseSA::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialSA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSA::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
