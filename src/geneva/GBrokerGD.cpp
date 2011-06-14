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

/************************************************************************************************************/
/**
 * The default constructor
 */
GBrokerGD::GBrokerGD()
	: GGradientDescent()
	, GBrokerConnector()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Initialization with the number of starting points and the size of the finite step
 */
GBrokerGD::GBrokerGD (
		const std::size_t& nStartingPoints
		, const float& finiteStep
		, const float& stepSize
)
	: GGradientDescent(nStartingPoints, finiteStep, stepSize)
	, GBrokerConnector()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor
 */
GBrokerGD::GBrokerGD(const GBrokerGD& cp)
	: GGradientDescent(cp)
	, GBrokerConnector(cp)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor.
 */
GBrokerGD::~GBrokerGD()
{ /* nothing */ }

/************************************************************************************************************/
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

/************************************************************************************************************/
/**
 * Checks for equality with another GBrokerGD object
 *
 * @param  cp A constant reference to another GBrokerGD object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerGD::operator==(const GBrokerGD& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerGD::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GBrokerGD object
 *
 * @param  cp A constant reference to another GBrokerGD object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerGD::operator!=(const GBrokerGD& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerGD::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBrokerGD::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerGD *p_load = GObject::conversion_cast<GBrokerGD>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GGradientDescent::checkRelationshipWith(cp, e, limit, "GBrokerGD", y_name, withMessages));
	deviations.push_back(GBrokerConnector::checkRelationshipWith(*p_load, e, limit, "GBrokerGD", y_name, withMessages));

	// no local data ...
	return evaluateDiscrepancies("GBrokerGD", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * @return The best achieved fitness
 */
double GBrokerGD::cycleLogic() {
	// Let the connector know how many processable items are available
	GBrokerConnector::setBCNProcessableItems(GGradientDescent::getNProcessableItems());

	// Let the GBrokerConnector know that we are starting a new iteration
	GBrokerConnector::markNewIteration();

	// Start the actual optimization cycle
	return GGradientDescent::cycleLogic();
}

/************************************************************************************************************/
/**
 * Loads the data from another GBrokerGD object.
 *
 * @param vp Pointer to another GBrokerGD object, camouflaged as a GObject
 */
void GBrokerGD::load_(const GObject *cp) {
	const GBrokerGD *p_load = conversion_cast<GBrokerGD>(cp);

	// Load the parent classes' data ...
	GGradientDescent::load_(cp);
	GBrokerConnector::load(p_load);
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GBrokerGD::clone_() const  {
	return new GBrokerGD(*this);
}

/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GBrokerGD::init() {
	// GGradientDesccent sees exactly the environment it would when called from its own class
	GGradientDescent::init();

	// Connect to the broker
	GBrokerConnector::init();

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
 * Necessary clean-up work after the optimization has finished
 */
void GBrokerGD::finalize() {
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
	GBrokerConnector::finalize();

	// GGradientDescent sees exactly the environment it would when called from its own class
	GGradientDescent::finalize();
}

/************************************************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GGradientDescent, albeit by delegating work to the broker.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
double GBrokerGD::doFitnessCalculation(const std::size_t& finalPos) {
	double bestFitness = getWorstCase(); // Holds the best fitness found so far
	std::size_t nStartingPoints = this->getNStartingPoints();
	boost::uint32_t iteration = getIteration();
	bool complete = false;

#ifdef DEBUG
	if(finalPos > this->size()) {
		raiseException(
				"In GBrokerGD::doFitnessCalculation(const std::size_t&):" << std::endl
				<< "Got invalid final position: " << finalPos << "/" << this->size()
		);
	}

	if(finalPos < nStartingPoints) {
		raiseException(
				"In GBrokerGD::doFitnessCalculation(const std::size_t&):" << std::endl
				<< "We require finalPos to be at least " << nStartingPoints << ", but got " << finalPos
		);
	}
#endif

	// Trigger value calculation for all individuals (including parents)
	for(std::size_t i=0; i<finalPos; i++) {
#ifdef DEBUG
		// Make sure the evaluated individuals have the dirty flag set
		if(!this->at(i)->isDirty()) {
			raiseException(
					"In GBrokerGD::doFitnessCalculation(const std::size_t&):" << std::endl
					<< "Found individual in position " << i << " whose dirty flag isn't set"
			);
		}
#endif /* DEBUG*/

		// Let the individual know that it should perform the "evaluate" command
		// after having passed the broker (i.e. usually on a remote machine)
		this->at(i)->getPersonalityTraits()->setCommand("evaluate");

		GBrokerConnector::submit(this->at(i));
	}

	//--------------------------------------------------------------------------------
	// We can now wait for the individuals to return from their journey.
	std::size_t nReceivedCurrent = 0;
	std::size_t nReceivedOlder   = 0;
	boost::shared_ptr<GParameterSet> p; // Will hold returned items

	// As we might be forced to resubmit individuals, we cannot clear our own vector
	// but need to store returning items in its own vector.
	std::vector<boost::shared_ptr<GParameterSet> > gps_vec;

	// First wait for the first individual of the current iteration to arrive.
	while(true) {
		// Retrieve the item from the server. Note that this call
		// may throw if a timeout for the first item has been set.
		p = GBrokerConnector::retrieveFirstItem<GParameterSet>();

		if(p->getParentAlgIteration() == iteration) {
			// Store the individual locally
			gps_vec.push_back(p);

			// Give the GBrokerConnector the opportunity to perform logging
			GBrokerConnector::log();

			// Update the counter.
			nReceivedCurrent++;

			// Mark as complete, if all submitted individuals have returned
			if(nReceivedCurrent == finalPos) complete = true;

			// Leave the loop
			break;
		} else {
			// We just update the counter so we can do some statistics.
			// Individuals from older iterations will simply be discarded,
			// as they have no significance in a gradient descent
			nReceivedOlder++;
  		}
	}

	// Wait for all submitted individuals to return. Unlike many other optimization algorithms,
	// gradient descents cannot cope easily with missing responses. The only option is to resubmit
	// items that didn't return before a given deadline.
	while(!complete) {
		if(p=GBrokerConnector::retrieveItem<GParameterSet>()) { // Did we receive a valid item ?
			if(p->getParentAlgIteration() == iteration) {
				// Check that the item hasn't already been received
				std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
				bool itemIsUnique = true;
				for(it=gps_vec.begin(); it!=gps_vec.end(); ++it) {
					if((*it)->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition() == p->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition()) {
						itemIsUnique = false;
						break; // Leave the for-loop
					}
				}


				// We can retrieve the next item (and discard this one) if the item is already present
				if(itemIsUnique) {
					// Store the individual locally
					gps_vec.push_back(p);

					// Give the GBrokerConnector the opportunity to perform logging
					GBrokerConnector::log();

					// Update the counter.
					nReceivedCurrent++;

					// Mark as complete, if all submitted individuals have returned
					if(nReceivedCurrent == finalPos) {
						complete = true;
						break; // Leave the while loop
					}
				}
			} else {
				// We just update the counter so we can do some statistics.
				// Individuals from older iterations will simply be discarded,
				// as they have no significance in a gradient descent
				nReceivedOlder++;
			}
		} else { // We have encountered a time out. Check which items are missing and resubmit
			// Sort the vector according to the expected position in the population
			std::sort(gps_vec.begin(), gps_vec.end(), indPositionComp());

			// Create a list of missing items
			std::vector<std::size_t> missingItems;
			std::size_t gps_start_pos = 0;
			for(std::size_t pos=0; pos < finalPos; pos++) {
				bool found = false;
				for(std::size_t gps_pos = gps_start_pos; gps_pos<gps_vec.size(); gps_pos++) {
					if(gps_vec[gps_pos]->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition() == pos) {
						found = true;
						gps_start_pos = gps_pos+1; // We can start searching in the next position in the next iteration
						break;
					}
				}

				if(!found) missingItems.push_back(pos);
			}

			// Make sure we do not immediately run into a timeout after re-submission
			GBrokerConnector::resetIterationStartTime();

			// Resubmit the corresponding individuals
			for(std::size_t m=0; m<missingItems.size(); m++) {
				GBrokerConnector::submit(this->at(missingItems[m]));
			}
		}
	}

#ifdef DEBUG
	// Check that we are indeed complete
	if(gps_vec.size() != this->size()) {
		raiseException(
				"In GBrokerGD::doFitnessCalculation():" << std::endl
				<< "Found invalid size of retrieved item vector: " << gps_vec.size() << std::endl
				<< "Should be " << this->size()
		);
	}
#endif /* DEBUG */

	// Sort the vector according to the expected position in the population
	std::sort(gps_vec.begin(), gps_vec.end(), indPositionComp());

	// Transfer the individuals into our own collection
	GBrokerGD::iterator it;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator pit;
	for(it=this->begin(), pit=gps_vec.begin(); it!=this->end(); ++it, ++pit) {
		(*it) = (*pit); // This will get rid of the old individuals
	}
	gps_vec.clear();

	// Retrieve information about the best fitness found
	double fitnessFound = 0.;
	for(std::size_t i=0; i<nStartingPoints; i++) {
		fitnessFound = this->at(i)->fitness();

		if(isBetter(fitnessFound, bestFitness)) {
			bestFitness = fitnessFound;
		}
	}

	return bestFitness;
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerGD::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GGradientDescent::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerGD::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GGradientDescent::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerGD::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GGradientDescent::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
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

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */
