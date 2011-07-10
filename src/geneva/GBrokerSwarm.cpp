/**
 * @file GBrokerSwarm.cpp
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

#include "geneva/GBrokerSwarm.hpp"

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerSwarm)

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
	, Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerSwarm object
 */
GBrokerSwarm::GBrokerSwarm(const GBrokerSwarm& cp)
	: GSwarm(cp)
	, Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>()
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
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::load(p_load);

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
boost::optional<std::string> GBrokerSwarm::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerSwarm *p_load = GObject::conversion_cast<GBrokerSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GSwarm::checkRelationshipWith(cp, e, limit, "GBrokerSwarm", y_name, withMessages));
	deviations.push_back(Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::checkRelationshipWith(*p_load, e, limit, "GBrokerSwarm", y_name, withMessages));

	// no local data ...
	return evaluateDiscrepancies("GBrokerSwarm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Update the iteration counters
 */
void GBrokerSwarm::markIteration() {
	// Execute the parent classes markIteration() call
	GSwarm::markIteration();

	// Let the GBrokerConnectorT know that we are starting a new iteration
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::markNewIteration(this->getIteration());
}

/************************************************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerSwarm::init() {
	// GSwarm sees exactly the environment it would when called from its own class
	GSwarm::init();

	// Connect to the broker
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::init();

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	sm_value_.clear(); // Make sure we do not have "left-overs"
	// Set the server mode and store the original flag
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		sm_value_.push_back((*it)->setServerMode(true));
	}
}

/************************************************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerSwarm::finalize() {
#ifdef DEBUG
	if(data.size() != sm_value_.size()) {
		raiseException(
				"In GBrokerSwarm::finalize():" << std::endl
				<< "Invalid number of serverMode flags: " << data.size() << "/" << sm_value_.size()
		);
	}
#endif /* DEBUG */

	// Restore the original values
	std::vector<bool>::iterator b_it;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(), b_it=sm_value_.begin(); it!=data.end(); ++it, ++b_it) {
		(*it)->setServerMode(*b_it);
	}
	sm_value_.clear(); // Make sure we have no "left-overs"

	// Invalidate our local broker connection
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::finalize();

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
void GBrokerSwarm::updateIndividualFitness(
		const boost::uint32_t& iteration
		, const std::size_t& neighborhood
		, boost::shared_ptr<GParameterSet> ind
) {
	// Let the individual know in which neighborhood it is
	ind->getPersonalityTraits<GSwarmPersonalityTraits>()->setNeighborhood(neighborhood);

	// Let the individual know that it should perform the "evaluate" command
	// after having passed the broker (i.e. usually on a remote machine)
	ind->getPersonalityTraits()->setCommand("evaluate");

	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::submit(ind);
}

/************************************************************************************************************/
/**
 * Makes individuals aware of the current iteration and integrates them into the population.
 * The return value can be used to break the loop, if the individual is the first one to
 * return in an iteration.
 *
 * @param p The individual that should be integrated into the population
 * @param nReceivedCurrent This variable will be incremented if the individual is part of the current iteration
 * @param nReceivedOlder This variable will be incremented if the individual is part of an older iteration
 * @return A boolean which indicates whether the individual is part of the current iteration
 */
bool GBrokerSwarm::updateIndividualsAndIntegrate(
		boost::shared_ptr<GParameterSet> p
	  , std::size_t& nReceivedCurrent
	  , std::size_t& nReceivedOlder
	  , const boost::uint32_t& iteration
) {
	// Update the personal best
	if(iteration == 0) {
		updatePersonalBest(p, p);
	} else {
		updatePersonalBestIfBetter(p, p);
	}

	if(p->getAssignedIteration() == iteration) {
		// Add the individual to our list.
		this->push_back(p);

		// Update the counter.
		nReceivedCurrent++;

		return true;
	} else {
		// Make it known to the individual that it is now part of a new iteration
		p->setAssignedIteration(iteration);

		// Add to the population. This will effectively increase the last neighborhood
		this->push_back(p);

		// Update the counter
		nReceivedOlder++;

		return false;
	}
}

/************************************************************************************************************/
/**
 * Modifies the particle positions, then triggers fitness calculation for all individuals. This function
 * submits individuals to Geneva's broker infrastructure and then integrates evaluated items, when they
 * come back.
 */
void GBrokerSwarm::swarmLogic() {
	boost::uint32_t iteration = getIteration();
	GBrokerSwarm::iterator it;
	std::vector<std::size_t> nNeighborhoodMembersCp;

	//--------------------------------------------------------------------------------
	// Create a copy of the last iteration's individuals and the number of individuals
	// in each neighborhood, if iteration > 0 . We use this to fill in missing returns.
	// This doesn't make sense for iteration 0 though, as individuals have not generally
	// been evaluated then, and we do not want to fill up with "dirty" individuals.
	std::vector<boost::shared_ptr<GParameterSet> > oldIndividuals;
	if(iteration > 0) {
		// Copy the individuals over
		for(it=this->begin(); it!=this->end(); ++it) {
			oldIndividuals.push_back((*it)->clone<GParameterSet>());
		}

		// Store the current amount of individuals in each neighborhood
		nNeighborhoodMembersCp = nNeighborhoodMembers_;
	}

	//--------------------------------------------------------------------------------
	// This function will call the overloaded GBrokerSwarm::updateIndividualFitness() function,
	// so that all individuals are submitted to the broker. Position updates will be
	// applied locally in the server.
	GSwarm::swarmLogic();

	//--------------------------------------------------------------------------------
	// We can now clear the local data vector. Individuals will be added to it as
	// they return from their journey.
	this->clear();

	//--------------------------------------------------------------------------------
	// We can now wait for the individuals to return from their journey.
	std::size_t nReceivedCurrent = 0;
	std::size_t nReceivedOlder   = 0;

	// Will hold returned items
	boost::shared_ptr<GParameterSet> p;

	// First wait for the first individual of the current iteration to arrive.
	// Individuals from older iterations will also be accepted in this loop.
	while(true) {
		// Note: the following call will throw if a timeout has been reached.
		p = Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::retrieveFirstItem<GParameterSet>();

		// If it is from the current iteration, break the loop, otherwise
		// continue until the first item of the current iteration has been
		// received.
		if(updateIndividualsAndIntegrate(p, nReceivedCurrent, nReceivedOlder, iteration)) break;
	}

	//--------------------------------------------------------------------------------
	// Wait for further arrivals until the population is complete or a timeout has been reached.
	bool complete=false;

	// Retrieve items as long as the population is not complete and
	// GBrokerConnectorT returns valid items
	while(!complete && (p=Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>::retrieveItem<GParameterSet>())) {
		// Integrate the individual into the population and update variables
		updateIndividualsAndIntegrate(p, nReceivedCurrent, nReceivedOlder, iteration);

		// Mark as complete, if a full set of individuals of the current iteration has returned.
		// Older individuals may return in the next iterations and will be taken into account.
		// The loop will terminate if the population has been found to be complete.
		if(nReceivedCurrent == getDefaultPopulationSize()) {
			complete = true;
		}
	}

	//--------------------------------------------------------------------------------
	// We now need to sort the individuals according to their neighborhoods, so that
	// individuals of the same neighborhood are in adjacent areas.
	sort(data.begin(), data.end(), indNeighborhoodComp());

	// Now update the number of items in each neighborhood
	for(std::size_t i=0; i<getNNeighborhoods(); i++) { // First reset the number of members of each neighborhood
		nNeighborhoodMembers_[i] = 0;
	}
	for(it=this->begin(); it!=this->end(); ++it) { // Update the number of individuals in each neighborhood
		nNeighborhoodMembers_[(*it)->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood()] += 1;
	}

	//--------------------------------------------------------------------------------
	// We are done, if a full set of individuals of the current iteration has returned
	// ("complete", which implies that each neighborhood has at least the correct number
	// of entries), or at the very least each neighborhood is at least at nominal values.
	// The population size might be larger than the default values (due to older individuals
	// having returned). Each neighborhood will be adjusted in a later function
	// (GSwarm::adjustNeighborhoods(), called from GSwarm::cycleLogic()), if necessary.
	if(complete || neighborhoodsHaveNominalValues()) {
		oldIndividuals.clear(); // Get rid of the copies
		return;
	}

	//--------------------------------------------------------------------------------
	// O.k., so some individuals are missing in this iteration. Do some fixing
	if(iteration > 0) { // The most likely case
		GBrokerSwarm::iterator insert_it = this->begin();
		std::vector<boost::shared_ptr<GParameterSet> >::iterator remove_it = oldIndividuals.begin();

		// Loop over all neighborhoods
		for(std::size_t n=0; n<nNeighborhoods_; n++) {
			// Find out, how many items are missing (if at all), add as required
			if(nNeighborhoodMembers_[n] < defaultNNeighborhoodMembers_) {
				std::size_t nMissing = defaultNNeighborhoodMembers_ - nNeighborhoodMembers_[n];

				// Copy the nMissing best individuals from the past iteration over. We
				// assume that the best individuals can be found at the front of each neighborhood
				std::size_t firstOldPos;
				std::size_t firstNIPos;

				for(std::size_t nM = 0; nM < nMissing; nM++) {
					// Calculate the position of oldIndividual's first individual in this neighborhood
					firstOldPos = getFirstNIPosVec(n, nNeighborhoodMembersCp);

					// Calculate the position of our own first individual in this neighborhood
					firstNIPos = getFirstNIPos(n);

					// Insert into main data vector
					this->insert(insert_it+firstNIPos, *(remove_it + firstOldPos));

#ifdef DEBUG
					if((*(insert_it+firstNIPos))->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood() != n) {
						raiseException(
								"In GBrokerSwarm::swarmLogic():" << std::endl
								<< "Found invalid neighborhood in copy: " << (*(insert_it+firstNIPos))->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood() << "/" << n
						);
					}
#endif /* DEBUG */

					// Remove entry from the copy
					oldIndividuals.erase(remove_it + firstOldPos);

					// Update the counters
					nNeighborhoodMembers_[n] += 1;
#ifdef DEBUG
					if(nNeighborhoodMembersCp[n] <= 0) {
						raiseException(
								"In GBrokerSwarm::swarmLogic():" << std::endl
								<< "Found copy of neighborhood without entries."
						);
					}
#endif /* DEBUG */
					nNeighborhoodMembersCp[n] -= 1;
				}
			}
		}

		// Get rid of the information about old individuals
		nNeighborhoodMembersCp.clear();
		oldIndividuals.clear();
	} else { // iteration == 0: Fill up with random items
		// At least one individual must have returned. Otherwise getFirstItem() would
		// have thrown an exception. We can thus safely create copies of the first
		// individual in the collection and re-initialize them randomly.

		// The start of the collection
		GBrokerSwarm::iterator insert_it = this->begin();

		// Loop over all neighborhoods. Find out, how many items are missing, add as required
		for(std::size_t n=0; n<nNeighborhoods_; n++) {
			std::size_t nMissing = defaultNNeighborhoodMembers_ - nNeighborhoodMembers_[n];

			std::size_t firstNIPos;
			for(std::size_t nM = 0; nM < nMissing; nM++) {
				// Calculate the position of our own first individual in this neighborhood
				firstNIPos = getFirstNIPos(n);

				// Insert a clone of the first individual of the collection
				this->insert(insert_it + firstNIPos, (this->front())->clone<GParameterSet>());

				// Randomly initialize the item and prevent position updates
				(*(insert_it + firstNIPos))->randomInit();
				(*(insert_it + firstNIPos))->getPersonalityTraits<GSwarmPersonalityTraits>()->setNoPositionUpdate();

				// Set the neighborhood as required
				(*(insert_it + firstNIPos))->getPersonalityTraits<GSwarmPersonalityTraits>()->setNeighborhood(n);

				// Update the counter
				nNeighborhoodMembers_[n] += 1;
			}
		}
	}
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
