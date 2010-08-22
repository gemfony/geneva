/**
 * @file GBrokerSwarm.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "geneva/GBrokerSwarm.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GBrokerSwarm)

namespace Gem
{
namespace Geneva
{

/************************************************************************************************************/
/**
 * The default constructor
 */
GBrokerSwarm::GBrokerSwarm(const std::size_t& nNeighborhoods, const std::size_t& nNeighborhoodMembers)
	: GSwarm(nNeighborhoods, nNeighborhoodMembers)
	, GBrokerConnector()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerSwarm object
 */
GBrokerSwarm::GBrokerSwarm(const GBrokerSwarm& cp)
	: GSwarm(cp)
	, GBrokerConnector(cp)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerSwarm::~GBrokerSwarm()
{ /* nothing */}

/************************************************************************************************************/
/**
 * A standard assignment operator for GBrokerSwarm objects,
 *
 * @param cp A copy of another GBrokerSwarm object
 * @return A constant reference to this object
 */
const GBrokerSwarm& GBrokerSwarm::operator=(const GBrokerSwarm& cp) {
	GBrokerSwarm::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data of another GBrokerSwarm object, camouflaged as a
 * pointer to a GObject
 *
 * @param cp A pointer to another GBrokerSwarm object, camouflaged as a GObject
 */
void GBrokerSwarm::load_(const GObject * cp) {
	const GBrokerSwarm *p_load = conversion_cast<GBrokerSwarm>(cp);

	// Load the parent classes' data ...
	GSwarm::load_(cp);
	GBrokerConnector::load(p_load);

	// no local data
}

/************************************************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GBrokerSwarm::clone_() const {
	return new GBrokerSwarm(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GBrokerSwarm object
 *
 * @param  cp A constant reference to another GBrokerSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerSwarm::operator==(const GBrokerSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerSwarm::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GBrokerSwarm object
 *
 * @param  cp A constant reference to another GBrokerSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerSwarm::operator!=(const GBrokerSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerSwarm::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
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
boost::optional<std::string> GBrokerSwarm::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerSwarm *p_load = GObject::conversion_cast<GBrokerSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GSwarm::checkRelationshipWith(cp, e, limit, "GBrokerSwarm", y_name, withMessages));
	deviations.push_back(GBrokerConnector::checkRelationshipWith(*p_load, e, limit, "GBrokerSwarm", y_name, withMessages));

	// no local data ...
	return evaluateDiscrepancies("GBrokerSwarm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * @return The best achieved fitness
 */
double GBrokerSwarm::cycleLogic() {
	// Let the GBrokerConnector know that we are starting a new iteration
	GBrokerConnector::markNewIteration();

	// Start the actual optimization cycle
	return GSwarm::cycleLogic();
}

/************************************************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerSwarm::init() {
	// GSwarm sees exactly the environment it would when called from its own class
	GSwarm::init();

	// Connect to the broker
	GBrokerConnector::init();
}

/************************************************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerSwarm::finalize() {
	// Invalidate our local broker connection
	GBrokerConnector::finalize();

	// GSwarm sees exactly the environment it would when called from its own class
	GSwarm::finalize();
}

/************************************************************************************************************/
/**
 * Triggers the fitness calculation of a GParameterSet-derivative, using Geneva's broker infrastructure.
 *
 * @param neighborhood The neighborhood the individual is in
 * @param ind The individual for which the fitness calculation should be performed
 */
void GBrokerSwarm::updateFitness(std::size_t neighborhood, boost::shared_ptr<GParameterSet> ind) {
	// Let the individual know in which neighborhood it is
	ind->getSwarmPersonalityTraits()->setNeighborhood(neighborhood);

	// Let the individual know that it should perform the "evaluate" command
	// after having passed the broker (i.e. usually on a remote machine)
	ind->getPersonalityTraits()->setCommand("evaluate");

	GBrokerConnector::submit(ind);
}
/************************************************************************************************************/
/**
 * Modifies the particle positions, then triggers fitness calculation for all individuals. This function
 * submits individuals to Geneva's broker infrastructure.
 */
void GBrokerSwarm::swarmLogic() {
	//--------------------------------------------------------------------------------
	// This function will call the overloaded GBrokerSwarm::updateFitness() function,
	// so that all individuals are submitted to the broker.
	GSwarm::swarmLogic();

	//--------------------------------------------------------------------------------
	// We can now wait for the individuals to return from their journey.
	std::size_t nReceivedCurrent = 0;
	std::size_t nReceivedOlder   = 0;
/*
	// First wait for the first individual of the current iteration to arrive.
	// Individuals from older iterations will also be accepted in this loop.
	while(true) {
		// Note: the following call will throw if a timeout has been reached.
		boost::shared_ptr<GParameterSet> p = GBrokerConnector::retrieveFirstItem<GParameterSet>();

		// If it is from the current iteration, break the loop, otherwise
		// continue until the first item of the current iteration has been
		// received.
		if(p->getParentAlgIteration() == iteration) {
			// Add the individual to our list.
			this->push_back(p);

			// Give the GBrokerConnector the opportunity to perform logging
			GBrokerConnector::log();

			// Update the counter.
			nReceivedCurrent++;

			break;
		} else {
			// Add to the population

			// Update the individual's iteration

			// Update the counter
			nReceivedOlder++;

		}
	}
	*/
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerSwarm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GSwarm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GSwarm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GSwarm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
