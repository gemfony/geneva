/**
 * @file GBrokerGD.cpp
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

#include "geneva/GBrokerGD.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerGD)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBrokerGD::GBrokerGD()
	: GBaseGD(), Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>(
	Gem::Courtier::RESUBMISSIONAFTERTIMEOUT) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of starting points and the size of the finite step
 */
GBrokerGD::GBrokerGD(
	const std::size_t &nStartingPoints, const double &finiteStep, const double &stepSize
)
	: GBaseGD(nStartingPoints, finiteStep, stepSize), Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>(
	Gem::Courtier::RESUBMISSIONAFTERTIMEOUT) { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 */
GBrokerGD::GBrokerGD(const GBrokerGD &cp)
	: GBaseGD(cp), Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor.
 */
GBrokerGD::~GBrokerGD() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBrokerGD &GBrokerGD::operator=(
	const GBrokerGD &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBrokerGD object
 *
 * @param  cp A constant reference to another GBrokerGD object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerGD::operator==(const GBrokerGD &cp) const {
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
 * Checks for inequality with another GBrokerGD object
 *
 * @param  cp A constant reference to another GBrokerGD object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerGD::operator!=(const GBrokerGD &cp) const {
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
void GBrokerGD::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GBaseEA reference
	const GBrokerGD *p_load = GObject::gobject_conversion<GBrokerGD>(&cp);

	GToken token("GBrokerGD", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseGD>(IDENTITY(*this, *p_load), token);

	// We do not compare the broker data

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBrokerGD::name() const {
	return std::string("GBrokerGD");
}

/******************************************************************************/
/**
 * Checks whether this algorithm communicates via the broker. This is an overload from the corresponding
 * GOptimizableI function
 *
 * @return A boolean indicating whether this algorithm communicates via the broker
 */
bool GBrokerGD::usesBroker() const {
	return true;
}

/******************************************************************************/
/**
 * Loads the data from another GBrokerGD object.
 *
 * @param vp Pointer to another GBrokerGD object, camouflaged as a GObject
 */
void GBrokerGD::load_(const GObject *cp) {
	const GBrokerGD *p_load = gobject_conversion<GBrokerGD>(cp);

	// Load the parent classes' data ...
	GBaseGD::load_(cp);
	Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>::load(p_load);

	// ... no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GBrokerGD::clone_() const {
	return new GBrokerGD(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GBrokerGD::init() {
	// GGradientDesccent sees exactly the environment it would when called from its own class
	GBaseGD::init();

	// Initialize the broker connector
	Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>::init();
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GBrokerGD::finalize() {
	// Finalize the broker connector
	Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>::finalize();

	// GBaseGD sees exactly the environment it would when called from its own class
	GBaseGD::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GBrokerGD::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	std::string comment;

	// Call our parent class'es function
	GBaseGD::addConfigurationOptions(gpb);
	Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>::addConfigurationOptions(gpb);

	// no local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GBrokerGD::getIndividualCharacteristic() const {
	return std::string("GENEVA_BROKEROPTALG");
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBaseGD, albeit by delegating work to the broker. Items are evaluated up to the maximum position
 * in the vector. Note that we always start the evaluation with the first item in the vector.
 */
void GBrokerGD::runFitnessCalculation() {
	using namespace Gem::Courtier;

	bool complete = false;

#ifdef DEBUG
	GBrokerGD::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
      // Make sure the evaluated individuals have the dirty flag set
      if(afterFirstIteration() && !(*it)->isDirty()) {
         glogger
         << "In GBrokerGD::runFitnessCalculation():" << std::endl
         << "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag isn't set" << std::endl
         << GEXCEPTION;
      }
	}
#endif /* DEBUG */

	//--------------------------------------------------------------------------------
	// Submit all work items and wait for their return
	boost::tuple<std::size_t, std::size_t> range(0, this->size());
	complete = GBrokerConnector2T<Gem::Geneva::GParameterSet>::workOn(
		data, range, oldWorkItems_, false // Do not remove unprocessed item
	);

	if (!complete) {
		glogger
		<< "In GBrokerGD::runFitnessCalculation(): Error!" << std::endl
		<< "No complete set of items received" << std::endl
		<< GEXCEPTION;
	}

	//--------------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerGD::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (GBaseGD::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerGD::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerGD::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseGD::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerGD::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerGD::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseGD::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerGD::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

