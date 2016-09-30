/**
 * @file GSerialSwarm.cpp
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
#include "geneva/GSerialSwarm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialSwarm)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Intentionally empty, as it is only needed for de-serialization purposes
 */
GSerialSwarm::GSerialSwarm()
	: GBaseSwarm() { /* nothing */ }

/******************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data, hence this function is empty.
 */
GSerialSwarm::GSerialSwarm(
	const std::size_t &nNeighborhoods, const std::size_t &nNeighborhoodMembers
)
	: GBaseSwarm(nNeighborhoods, nNeighborhoodMembers) { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GSerialSwarm::GSerialSwarm(const GSerialSwarm &cp)
	: GBaseSwarm(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor.
 */
GSerialSwarm::~GSerialSwarm() { /* nothing */ }

/******************************************************************************/
/**
 * Loads the data from another GSerialSwarm object.
 *
 * @param vp Pointer to another GSerialSwarm object, camouflaged as a GObject
 */
void GSerialSwarm::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GSerialSwarm * p_load = Gem::Common::g_convert_and_compare<GObject, GSerialSwarm>(cp, this);

	// First load our parent class'es data ...
	GBaseSwarm::load_(cp);

	// no local data
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GSerialSwarm &GSerialSwarm::operator=(const GSerialSwarm &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GSerialSwarm object
 *
 * @param  cp A constant reference to another GSerialSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialSwarm::operator==(const GSerialSwarm &cp) const {
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
 * Checks for inequality with another GSerialSwarm object
 *
 * @param  cp A constant reference to another GSerialSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialSwarm::operator!=(const GSerialSwarm &cp) const {
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
void GSerialSwarm::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GSerialSwarm reference independent of this object and convert the pointer
	const GSerialSwarm *p_load = Gem::Common::g_convert_and_compare<GObject, GSerialSwarm>(cp, this);

	GToken token("GSerialSwarm", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseSwarm>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSerialSwarm::name() const {
	return std::string("GSerialSwarm");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialSwarm::clone_() const {
	return new GSerialSwarm(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialSwarm::init() {
	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::init();

	// Add local initialization code here
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialSwarm::finalize() {
	// Add local finalization code here

	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GSerialSwarm::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GBaseSwarm::addConfigurationOptions(gpb);

	// no local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GSerialSwarm::getIndividualCharacteristic() const {
	return std::string("GENEVA_SERIALOPTALG");
}

/******************************************************************************/
/**
 * Updates the fitness of all individuals
 */
void GSerialSwarm::runFitnessCalculation() {
	bool originalServerMode = false;
	GSerialSwarm::iterator it;
	for (it = this->begin(); it != this->end(); ++it) {
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
bool GSerialSwarm::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseSwarm::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSwarm::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialSwarm::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSwarm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialSwarm::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSwarm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
