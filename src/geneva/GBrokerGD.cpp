/**
 * @file GBrokerGD.cpp
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

#include "geneva/GBrokerGD.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerGD)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBrokerGD::GBrokerGD() :
	GBaseGD()
	, Gem::Courtier::GBrokerConnectorT<GIndividual>()
	, storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of starting points and the size of the finite step
 */
GBrokerGD::GBrokerGD(
		const std::size_t& nStartingPoints
		, const float& finiteStep, const float& stepSize
)
	: GBaseGD(nStartingPoints, finiteStep, stepSize)
	, Gem::Courtier::GBrokerConnectorT<GIndividual>()
	, storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 */
GBrokerGD::GBrokerGD(const GBrokerGD& cp)
	: GBaseGD(cp)
	, Gem::Courtier::GBrokerConnectorT<GIndividual>(cp)
	, storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor.
 */
GBrokerGD::~GBrokerGD()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator for GBrokerGD objects.
 *
 * @param cp Reference to another GBrokerGD object
 * @return A constant reference to this object
 */
const GBrokerGD& GBrokerGD::operator=(const GBrokerGD& cp) {
	GBrokerGD::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBrokerGD object
 *
 * @param  cp A constant reference to another GBrokerGD object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerGD::operator==(const GBrokerGD& cp) const
{
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0., "GBrokerGD::operator==",
			"cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBrokerGD object
 *
 * @param  cp A constant reference to another GBrokerGD object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerGD::operator!=(const GBrokerGD& cp) const
{
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,
			"GBrokerGD::operator!=", "cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GBrokerGD::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
	using namespace Gem::Common;
	using namespace Gem::Courtier;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerGD *p_load = GObject::gobject_conversion<GBrokerGD>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector < boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GBaseGD::checkRelationshipWith(cp, e, limit, "GBrokerGD",	y_name, withMessages));
	deviations.push_back(GBrokerConnectorT<GIndividual>::checkRelationshipWith(*p_load, e, limit, "GBrokerGD", y_name, withMessages));

	// no local data

	return evaluateDiscrepancies("GBrokerGD", caller, deviations, e);
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
	const GBrokerGD *p_load = gobject_conversion<GBrokerGD> (cp);

	// Load the parent classes' data ...
	GBaseGD::load_(cp);
	Gem::Courtier::GBrokerConnectorT<GIndividual>::load(p_load);

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

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	// The function will throw if not all individuals have the same server mode flag.

	// Set the server mode and store the original flag
	bool first = true;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		if(first){
			storedServerMode_ = (*it)->getServerMode();
			first = false;
		}

		if(storedServerMode_ != (*it)->setServerMode(true)) {
		   glogger
		   << "In GBrokerGD::init():" << std::endl
         << "Not all server mode flags have the same value!" << std::endl
         << GEXCEPTION;
		}
	}
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GBrokerGD::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(storedServerMode_);
	}

	// GBaseGD sees exactly the environment it would when called from its own class
	GBaseGD::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GBrokerGD::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GBaseGD::addConfigurationOptions(gpb, showOrigin);
	Gem::Courtier::GBrokerConnectorT<GIndividual>::addConfigurationOptions(gpb, showOrigin);

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
 * in GBaseGD, albeit by delegating work to the broker. Items are evaluated up to a maximum position
 * in the vector. Note that we always start the evaluation with the first item in the vector.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
double GBrokerGD::doFitnessCalculation(const std::size_t& finalPos) {
	using namespace Gem::Courtier;

	std::size_t nStartingPoints = this->getNStartingPoints();
	bool complete = false;

#ifdef DEBUG
	if(finalPos > this->size()) {
	   glogger
	   << "In GBrokerGD::doFitnessCalculation(const std::size_t&):" << std::endl
      << "Got invalid final position: " << finalPos << "/" << this->size() << std::endl
      << GEXCEPTION;
	}

	if(finalPos < nStartingPoints) {
	   glogger
	   << "In GBrokerGD::doFitnessCalculation(const std::size_t&):" << std::endl
      << "We require finalPos to be at least " << nStartingPoints << ", but got " << finalPos << std::endl
      << GEXCEPTION;
	}

	for (std::size_t i = 0; i < finalPos; i++) {
		// Make sure the evaluated individuals have the dirty flag set
		if(afterFirstIteration() && !this->at(i)->isDirty()) {
		   glogger
		   << "In GBrokerGD::doFitnessCalculation(const std::size_t&):" << std::endl
         << "Found individual in position " << i << " whose dirty flag isn't set" << std::endl
         << GEXCEPTION;
		}
	}
#endif /* DEBUG */

	//--------------------------------------------------------------------------------
	// Submit all work items and wait for their return
	boost::tuple<std::size_t,std::size_t> range(0, finalPos);
	complete = GBrokerConnectorT<GIndividual>::workOn(
			data
			, range
			, EXPECTFULLRETURN
	);

	if(!complete) {
	   glogger
	   << "In GBrokerGD::doFitnessCalculation(): Error!" << std::endl
      << "No complete set of items received" << std::endl
      << GEXCEPTION;
	}

	//--------------------------------------------------------------------------------
	// Retrieve information about the best fitness found
	double bestFitness = getWorstCase(); // Holds the best fitness found so far
	double fitnessFound = 0.;
	for (std::size_t i = 0; i < nStartingPoints; i++) {
		fitnessFound = this->at(i)->fitness();

		if (isBetter(fitnessFound, bestFitness)) {
			bestFitness = fitnessFound;
		}
	}

	return bestFitness;
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
	if(GBaseGD::modify_GUnitTests()) result = true;

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

#ifdef GEM_TESTING

// Tests of this class (and parent classes)
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GBrokerGD has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GBrokerGD> TFactory_GUnitTests<Gem::Geneva::GBrokerGD>() {
	boost::shared_ptr<Gem::Geneva::GBrokerGD> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GBrokerGD>(new Gem::Geneva::GBrokerGD()));
	return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */
