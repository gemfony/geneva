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
	: GSerialSwarm(nNeighborhoods, nNeighborhoodMembers)
	, Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerSwarm object
 */
GBrokerSwarm::GBrokerSwarm(const GBrokerSwarm& cp)
	: GSerialSwarm(cp)
    , Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>(cp)
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
	GSerialSwarm::load_(cp);
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
    using namespace Gem::Courtier;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerSwarm *p_load = GObject::conversion_cast<GBrokerSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GSerialSwarm::checkRelationshipWith(cp, e, limit, "GBrokerSwarm", y_name, withMessages));
	deviations.push_back(GBrokerConnectorT<GIndividual>::checkRelationshipWith(*p_load, e, limit, "GBrokerSwarm", y_name, withMessages));

	// no local data

	return evaluateDiscrepancies("GBrokerSwarm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerSwarm::init() {
	// GSerialSwarm sees exactly the environment it would when called from its own class
	GSerialSwarm::init();

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

	// GSerialSwarm sees exactly the environment it would when called from its own class
	GSerialSwarm::finalize();
}

/************************************************************************************************************/
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
	std::string comment;

	// no local data

	// Call our parent class'es function
	GSerialSwarm::addConfigurationOptions(gpb, showOrigin);
	Gem::Courtier::GBrokerConnectorT<GIndividual>::addConfigurationOptions(gpb, showOrigin);
}

/************************************************************************************************************/
/**
 * Creates a copy of the last iteration's individuals, if iteration > 0, then performs the standard position
 * update using GSwam::updatePositions(). We use the old individuals to fill in missing returns in
 * adjustNeighborhoods. This doesn't make sense for iteration 0 though, as individuals have not generally
 * been evaluated then, and we do not want to fill up with "dirty" individuals.
 */
void GBrokerSwarm::updatePositions() {
#ifdef DEBUG
	// Check that all neighborhoods have the default size
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		if(nNeighborhoodMembers_[n] != defaultNNeighborhoodMembers_) {
			raiseException(
					"In GBrokerSwarm::updatePositions(): Error!" << std::endl
					<< "nNeighborhoodMembers_[" << n << "] has invalid size " << nNeighborhoodMembers_[n] << std::endl
					<< "but expected size " << defaultNNeighborhoodMembers_ << std::endl
			);
		}

		if(this->size() != nNeighborhoods_*defaultNNeighborhoodMembers_) {
			raiseException(
					"In GBrokerSwarm::updatePositions(): Error!" << std::endl
					<< "The population has an incorrect size of " << this->size() << ", expected " << nNeighborhoods_*defaultNNeighborhoodMembers_ << std::endl
			);
		}
	}
#endif

	oldIndividuals_.clear();
	if(getIteration() > 0) {
		GBrokerSwarm::iterator it;

		// Clone the individuals and copy them over
		for(it=this->begin(); it!=this->end(); ++it) {
			oldIndividuals_.push_back((*it)->clone<GParameterSet>());
		}
	}

	GSerialSwarm::updatePositions();
}

/************************************************************************************************************/
/**
 * Triggers the fitness calculation of all individuals
 */
void GBrokerSwarm::updateFitness() {
	using namespace Gem::Courtier;

	boost::uint32_t iteration = getIteration();
	GBrokerSwarm::iterator it;

	//--------------------------------------------------------------------------------
	// Let all individual know that they should perform the "evaluate" command
	// after having passed the broker (i.e. usually on a remote machine)
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->getPersonalityTraits()->setCommand("evaluate");
	}

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results
	Gem::Courtier::GBrokerConnectorT<GIndividual>::workOn(
			data
			, 0
			, data.size()
			, ACCEPTOLDERITEMS
	);

	// Update the personal best of all individuals. Also update the iteration
	// of older individuals (they will keep their old neighborhood id)
	for(it=this->begin(); it!=this->end(); ++it) {
		// Update the personal best
		if(iteration == 0) {
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

/************************************************************************************************************/
/**
 * Fixes the population after a job submission, possibly using stored copies of the previous iteration.
 */
void GBrokerSwarm::adjustNeighborhoods() {
	std::size_t firstNIPos; // Will hold the expected first position of a neighborhood
	boost::uint32_t iteration = getIteration();

#ifdef DEBUG
	// Check that oldIndividuals_ has the desired size in iterations other than the first
	if(iteration > 0 && oldIndividuals_.size() != defaultNNeighborhoodMembers_*nNeighborhoods_) {
		raiseException(
				"In GBrokerSwarm::adjustNeighborhoods(): Error!" << std::endl
				<< "oldIndividuals_ has incorrect size! Expected" << std::endl
				<< "defaultNNeighborhoodMembers_*nNeighborhoods_ = " << defaultNNeighborhoodMembers_*nNeighborhoods_ << std::endl
				<< "but found " << oldIndividuals_.size()
		);
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

			if(iteration > 0) { // The most likely case
				// Copy the best items of this neighborhood over from the oldIndividuals_ vector.
				// Each neighborhood there should have been sorted according to the individuals
				// fitness, with the best individuals in the front of each neighborhood.
				for(std::size_t i=0; i<nMissing; i++) {
					data.insert(data.begin() + firstNIPos, *(oldIndividuals_.begin() + firstNIPos + i));
				}
			} else { // iteration == 0
#ifdef DEBUG
				// At least one individual must have returned.
				if(this->empty()) {
					raiseException(
							"In GBrokerSwarm::adjustNeighborhoods(): Error!" << std::endl
							<< "No items found in the population. Cannot fix." << std::endl
					);
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
		raiseException(
				"In GBrokerSwarm::adjustNeighborhoods(): Error!" << std::endl
				<< "The population has an incorrect size of " << this->size() << ", expected " << nNeighborhoods_*defaultNNeighborhoodMembers_ << std::endl
		);
	}
#endif

	oldIndividuals_.clear(); // Get rid of the copies
}

/************************************************************************************************************/
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
	if(GSerialSwarm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GSerialSwarm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GSerialSwarm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
