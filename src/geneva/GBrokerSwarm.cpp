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

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBrokerSwarm::GBrokerSwarm()
	: GBaseSwarm()
	, Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>()
	, storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor that initializes the swarm with the number of neighborhoods and the expected number of
 * members inside of them.
 *
 * @param nNeighborhoods The number of neighborhoods
 * @param nNeighborhoodMembers The expected number of members inside of them
 */
GBrokerSwarm::GBrokerSwarm(
      const std::size_t& nNeighborhoods
      , const std::size_t& nNeighborhoodMembers
)
	: GBaseSwarm(nNeighborhoods, nNeighborhoodMembers)
	, Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>()
   , storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerSwarm object
 */
GBrokerSwarm::GBrokerSwarm(const GBrokerSwarm& cp)
	: GBaseSwarm(cp)
    , Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>(cp)
    , storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerSwarm::~GBrokerSwarm()
{ /* nothing */}

/******************************************************************************/
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

/******************************************************************************/
/**
 * Loads the data of another GBrokerSwarm object, camouflaged as a
 * pointer to a GObject
 *
 * @param cp A pointer to another GBrokerSwarm object, camouflaged as a GObject
 */
void GBrokerSwarm::load_(const GObject * cp) {
	const GBrokerSwarm *p_load = gobject_conversion<GBrokerSwarm>(cp);

	// Load the parent classes' data ...
	GBaseSwarm::load_(cp);
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>::load(p_load);

	// no local data
}

/******************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GBrokerSwarm::clone_() const {
	return new GBrokerSwarm(*this);
}

/******************************************************************************/
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

/******************************************************************************/
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
boost::optional<std::string> GBrokerSwarm::checkRelationshipWith(
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
	const GBrokerSwarm *p_load = GObject::gobject_conversion<GBrokerSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GBaseSwarm::checkRelationshipWith(cp, e, limit, "GBrokerSwarm", y_name, withMessages));
	deviations.push_back(GBrokerConnectorT<GParameterSet>::checkRelationshipWith(*p_load, e, limit, "GBrokerSwarm", y_name, withMessages));

	// no local data

	return evaluateDiscrepancies("GBrokerSwarm", caller, deviations, e);
}

/******************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerSwarm::init() {
	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::init();

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
		   << "In GBrokerSwarm::init():" << std::endl
         << "Not all server mode flags have the same value!" << std::endl
         << GEXCEPTION;
		}
	}
}

/******************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerSwarm::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(storedServerMode_);
	}

	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::finalize();
}

/******************************************************************************/
/**
 * Checks whether this algorithm communicates via the broker. This is an overload from the corresponding
 * GOptimizableI function
 *
 * @return A boolean indicating whether this algorithm communicates via the broker
 */
bool GBrokerSwarm::usesBroker() const {
	return true;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GBrokerSwarm::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	// Call our parent class'es function
	GBaseSwarm::addConfigurationOptions(gpb, showOrigin);
	Gem::Courtier::GBrokerConnectorT<GParameterSet>::addConfigurationOptions(gpb, showOrigin);

	// no local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GBrokerSwarm::getIndividualCharacteristic() const {
	return std::string("GENEVA_BROKEROPTALG");
}

/******************************************************************************/
/**
 * Creates a copy of the last iteration's individuals, if this is not the first iteration, then performs the standard position
 * update using GSwam::updatePositions(). We use the old individuals to fill in missing returns in
 * adjustNeighborhoods. This doesn't make sense for the first iteration though, as individuals have not generally
 * been evaluated then, and we do not want to fill up with "dirty" individuals.
 */
void GBrokerSwarm::updatePositions() {
#ifdef DEBUG
	// Check that all neighborhoods have the default size
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		if(nNeighborhoodMembers_[n] != defaultNNeighborhoodMembers_) {
		   glogger
		   << "In GBrokerSwarm::updatePositions(): Error!" << std::endl
         << "nNeighborhoodMembers_[" << n << "] has invalid size " << nNeighborhoodMembers_[n] << std::endl
         << "but expected size " << defaultNNeighborhoodMembers_ << std::endl
         << GEXCEPTION;
		}

		if(this->size() != nNeighborhoods_*defaultNNeighborhoodMembers_) {
		   glogger
		   << "In GBrokerSwarm::updatePositions(): Error!" << std::endl
         << "The population has an incorrect size of " << this->size() << ", expected " << nNeighborhoods_*defaultNNeighborhoodMembers_ << std::endl
         << GEXCEPTION;
		}
	}
#endif

	oldIndividuals_.clear();
	if(afterFirstIteration()) {
		GBrokerSwarm::iterator it;

		// Clone the individuals and copy them over
		for(it=this->begin(); it!=this->end(); ++it) {
			oldIndividuals_.push_back((*it)->clone<GParameterSet>());
		}
	}

	GBaseSwarm::updatePositions();
}


/******************************************************************************/
/**
 * This function implements the logic that constitutes each cycle of a swarm algorithm. It
 * adds code to repair the population in case of incomplete returns to our parent class'es
 * function.
 *
 * @return The value of the best individual found
 */
double GBrokerSwarm::cycleLogic() {
   // Perform the actual business logic
   double bestResult = GBaseSwarm::cycleLogic();

   // Makes sure that each neighborhood has the right size before the next cycle starts
   adjustNeighborhoods();

   // Let the audience know
   return bestResult;
}

/******************************************************************************/
/**
 * Triggers the fitness calculation of all individuals
 */
void GBrokerSwarm::updateFitness() {
	using namespace Gem::Courtier;

	boost::uint32_t iteration = getIteration();
	GBrokerSwarm::iterator it;

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results
	Gem::Courtier::GBrokerConnectorT<GParameterSet>::workOn(
			data
			// , EXPECTFULLRETURN
			, ACCEPTOLDERITEMS
	);

	// Update the personal best of all individuals. Also update the iteration
	// of older individuals (they will keep their old neighborhood id)
	for(it=this->begin(); it!=this->end(); ++it) {
		// Update the personal best
		if(inFirstIteration()) {
			updatePersonalBest(*it);
		} else {
			updatePersonalBestIfBetter(*it);
		}

		if((*it)->getAssignedIteration() != iteration) {
			(*it)->setAssignedIteration(iteration);
		}
	}

	// Sort according to the individuals' neighborhoods
	sort(data.begin(), data.end(), indNeighborhoodComp());

	// Now update the number of items in each neighborhood: First reset the number of members of each neighborhood
	Gem::Common::assignVecConst(nNeighborhoodMembers_, (std::size_t)0);
	// Then update the number of individuals in each neighborhood
	for(it=this->begin(); it!=this->end(); ++it) {
		nNeighborhoodMembers_[(*it)->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood()] += 1;
	}

	// The population will be fixed in the overloaded GBrokerSwarm::adjustNeighborhoods() function
}

/******************************************************************************/
/**
 * Fixes the population after a job submission, possibly using stored copies of the previous iteration.
 */
void GBrokerSwarm::adjustNeighborhoods() {
	std::size_t firstNIPos; // Will hold the expected first position of a neighborhood

#ifdef DEBUG
	// Check that oldIndividuals_ has the desired size in iterations other than the first
	if(afterFirstIteration() && oldIndividuals_.size() != defaultNNeighborhoodMembers_*nNeighborhoods_) {
	   glogger
	   << "In GBrokerSwarm::adjustNeighborhoods(): Error!" << std::endl
      << "oldIndividuals_ has incorrect size! Expected" << std::endl
      << "defaultNNeighborhoodMembers_*nNeighborhoods_ = " << defaultNNeighborhoodMembers_*nNeighborhoods_ << std::endl
      << "but found " << oldIndividuals_.size() << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	// Add missing items to neighborhoods that are too small. We use stored copies from the
	// last iteration to fill in the missing items, or add random items in the first iteration.
	// Neighborhoods with too many items are pruned. findBests() has sorted each neighborhood
	// according to its fitness, so we know that the best items are in the front position of each
	// neighborhood. We thus simply remove items at the end of neighborhoods that are too large.
	for(std::size_t n=0; n<nNeighborhoods_; n++) { // Loop over all neighborhoods
		// Calculate the desired position of our own first individual in this neighborhood
		// As we start with the first neighborhood and add or remove surplus or missing items,
		// getFirstNIPos() will return a valid position.
		firstNIPos = getFirstNIPos(n);

		if(nNeighborhoodMembers_[n] == defaultNNeighborhoodMembers_) {
			continue;
		} else if(nNeighborhoodMembers_[n] > defaultNNeighborhoodMembers_) { // Remove surplus items from the end of the neighborhood
			// Find out, how many surplus items there are
			std::size_t nSurplus = nNeighborhoodMembers_[n] - defaultNNeighborhoodMembers_;

			// Remove nSurplus items from the position (n+1)*defaultNNeighborhoodMembers_
			data.erase(
					data.begin()   +  (n+1)*defaultNNeighborhoodMembers_
					, data.begin() + ((n+1)*defaultNNeighborhoodMembers_ + nSurplus)
			);
		} else { // nNeighborhoodMembers_[n] < defaultNNeighborhoodMembers_
			// The number of missing items
			std::size_t nMissing = defaultNNeighborhoodMembers_ - nNeighborhoodMembers_[n];

			if(afterFirstIteration()) { // The most likely case
				// Copy the best items of this neighborhood over from the oldIndividuals_ vector.
				// Each neighborhood there should have been sorted according to the individuals
				// fitness, with the best individuals in the front of each neighborhood.
				for(std::size_t i=0; i<nMissing; i++) {
					data.insert(data.begin() + firstNIPos, *(oldIndividuals_.begin() + firstNIPos + i));
				}
			} else { // first iteration
#ifdef DEBUG
				// At least one individual must have returned.
				if(this->empty()) {
				   glogger
				   << "In GBrokerSwarm::adjustNeighborhoods(): Error!" << std::endl
               << "No items found in the population. Cannot fix." << std::endl
               << GEXCEPTION;
				}
#endif

				// Fill up with random items.
				for(std::size_t nM = 0; nM < nMissing; nM++) {
					// Insert a clone of the first individual of the collection
					data.insert(data.begin() + firstNIPos, (this->front())->clone<GParameterSet>());

					// Randomly initialize the item and prevent position updates
					(*(data.begin() + firstNIPos))->randomInit();
					(*(data.begin() + firstNIPos))->getPersonalityTraits<GSwarmPersonalityTraits>()->setNoPositionUpdate();

					// Set the neighborhood as required
					(*(data.begin() + firstNIPos))->getPersonalityTraits<GSwarmPersonalityTraits>()->setNeighborhood(n);
				}
			}
		}

		// Finally adjust the number of entries in this neighborhood
		nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
	}

#ifdef DEBUG
	// Check that the population has the expected size
	if(this->size() != nNeighborhoods_*defaultNNeighborhoodMembers_) {
	   glogger
	   << "In GBrokerSwarm::adjustNeighborhoods(): Error!" << std::endl
      << "The population has an incorrect size of " << this->size() << ", expected " << nNeighborhoods_*defaultNNeighborhoodMembers_ << std::endl
      << GEXCEPTION;
	}
#endif

	oldIndividuals_.clear(); // Get rid of the copies
}

/******************************************************************************/
/**
 * Checks whether each neighborhood has the default size
 *
 * @return A boolean which indicates whether all neighborhoods have the default size
 */
bool GBrokerSwarm::neighborhoodsHaveNominalValues() const {
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		if(nNeighborhoodMembers_[n] == defaultNNeighborhoodMembers_) return false;
	}
	return true;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerSwarm::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GBaseSwarm::modify_GUnitTests()) result = true;

	return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerSwarm::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerSwarm::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
	GBaseSwarm::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerSwarm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerSwarm::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerSwarm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
