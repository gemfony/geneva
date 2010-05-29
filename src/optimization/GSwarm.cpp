/**
 * @file GSwarm.cpp
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

#include "GSwarm.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GSwarm)

namespace Gem {
namespace GenEvA {

/************************************************************************************************************/
/**
 * The default constructor sets the number of neighborhoods and the number of individuals in them
 *
 * @param nNeighborhoods The desired number of neighborhoods (hardwired to >= 1)
 * @oaram nNeighborhoodMembers The default number of individuals in each neighborhood (hardwired to >= 1)
 */
GSwarm::GSwarm(const std::size_t& nNeighborhoods, const std::size_t& nNeighborhoodMembers)
	: GOptimizationAlgorithm()
	, infoFunction_(&GSwarm::simpleInfoFunction)
	, nNeighborhoods_((nNeighborhoods==0)?1:nNeighborhoods)
	, defaultNNeighborhoodMembers_((nNeighborhoodMembers==0)?1:nNeighborhoodMembers)
	, nNeighborhoodMembers_(new std::size_t[nNeighborhoods_])
	, local_bests_(new boost::shared_ptr<GIndividual>[nNeighborhoods_])
{
	GOptimizationAlgorithm::setPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Initialize with the default number
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		nNeighborhoodMembers_[i] = defaultNNeighborhoodMembers_;
	}
}

/************************************************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GSwarm object
 */
GSwarm::GSwarm(const GSwarm& cp)
	: GOptimizationAlgorithm(cp)
	, infoFunction_(&GSwarm::simpleInfoFunction) // Note that we do not copy the info function
	, nNeighborhoods_(cp.nNeighborhoods_)
	, defaultNNeighborhoodMembers_(cp.defaultNNeighborhoodMembers_)
	, nNeighborhoodMembers_(new std::size_t[nNeighborhoods_])
	, global_best_((cp.getIteration()>0)?(cp.global_best_)->clone<GIndividual>():boost::shared_ptr<GIndividual>())
	, local_bests_(new boost::shared_ptr<GIndividual>[nNeighborhoods_])
{
	// Copy the current number of individuals in each neighborhood over
#ifdef DEBUG
	std::size_t nCPIndividuals = 0;
#endif /* DEBUG */

	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		nNeighborhoodMembers_[i] = cp.nNeighborhoodMembers_[i];

#ifdef DEBUG
		// Calculate the total number of individuals that should be present
		nCPIndividuals += nNeighborhoodMembers_[i];
#endif /* DEBUG */
	}

#ifdef DEBUG
	if(nCPIndividuals != cp.size()) {
		std::ostringstream error;
		error << "In GSwarm::GSwarm(const GSwarm& cp): Error!" << std::endl
			  << "Number of individuals in cp " << cp.size() << "differs from expected number " << nCPIndividuals << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */

	// Note that this setting might differ from nCPIndividuals, as it is not guaranteed
	// that cp has, at the time of copying, all individuals present in each neighborhood.
	// Differences might e.g. occur if not all individuals return from their remote
	// evaluation. adjustPopulation will take care to resize the population appropriately
	// inside of the "optimize()" call.
	GOptimizationAlgorithm::setPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Clone cp's locally best individuals, if this is not the first iteration
	if(cp.getIteration()>0) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			local_bests_[i] = cp.local_bests_[i]->clone<GIndividual>();
		}
	}
}

/************************************************************************************************************/
/**
 * The standard destructor. Most work is done in the parent class.
 */
GSwarm::~GSwarm() {
	if(nNeighborhoodMembers_) delete [] nNeighborhoodMembers_;
	if(local_bests_) delete [] local_bests_; // This will also get rid of the objects pointed to
}

/************************************************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp Another GSwarm object
 * @return A constant reference to this object
 */
const GSwarm& GSwarm::operator=(const GSwarm& cp) {
	GSwarm::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data of another GSwarm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GSwarm object, camouflaged as a GObject
 */
void GSwarm::load_(const GObject *cp)
{
	// Make a note of the current iteration (needed for a check below).
	// The information would otherwise be lost after the load call below
	boost::uint32_t currentIteration = this->getIteration();

	const GSwarm *p_load = this->conversion_cast<GSwarm>(cp);

	// First load the parent class'es data ...
	GOptimizationAlgorithm::load_(cp);

	// ... and then our own data

	// Make sure we are dealing with the same number of neighborhoods. If this
	// is not the case we need to set up everything from scratch.
	if(nNeighborhoods_ != p_load->nNeighborhoods_) {
		nNeighborhoods_ = p_load->nNeighborhoods_;

		delete [] nNeighborhoodMembers_;
		delete [] local_bests_;

		nNeighborhoodMembers_ = new std::size_t[nNeighborhoods_];
		local_bests_ = new boost::shared_ptr<GIndividual>[nNeighborhoods_];

		if(p_load->getIteration() > 0) {
			for(std::size_t i=0; i<nNeighborhoods_; i++) {
				if(currentIteration == 0) {
					local_bests_[i] = p_load->local_bests_[i]->clone<GIndividual>();
				}
				else {
#ifdef DEBUG
					if(!local_bests_[i]) {
						std::ostringstream error;
						error << "GSwarm::load_(): Error!" << std::endl
							  << "Found empty local best at position " << i << std::endl
							  << "in iteration " << this->getIteration() << std::endl;
						throw Gem::Common::gemfony_error_condition(error.str());
					}
#endif /* DEBUG */

					local_bests_[i] = (p_load->local_bests_)[i]->clone<GIndividual>();
				}
			}
		}
	}
	else {
		if(p_load->getIteration() > 0) {
			for(std::size_t i=0; i<nNeighborhoods_; i++) {
#ifdef DEBUG
				if(!local_bests_[i]) {
					std::ostringstream error;
					error << "GSwarm::load_(): Error/2 !" << std::endl
						  << "Found empty local best at position " << i << std::endl
						  << "in iteration " << this->getIteration() << std::endl;
					throw Gem::Common::gemfony_error_condition(error.str());
				}
#endif /* DEBUG */

				local_bests_[i]->load((p_load->local_bests_)[i]);
			}
		}
	}

	defaultNNeighborhoodMembers_ = p_load->defaultNNeighborhoodMembers_;

	// Copy the number of individuals in each neighborhood as well as the local bests over
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		nNeighborhoodMembers_[i] = (p_load->nNeighborhoodMembers_)[i];
	}

	if(p_load->getIteration() > 0) {
		global_best_->GObject::load(p_load->global_best_);
	}

	// Note that we do not copy the info function
}

/************************************************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object, camouflaged as a pointer to a GObject
 */
GObject *GSwarm::clone_() const  {
	return new GSwarm(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GSwarm object
 *
 * @param  cp A constant reference to another GSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarm::operator==(const GSwarm& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarm::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GSwarm object
 *
 * @param  cp A constant reference to another GSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarm::operator!=(const GSwarm& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarm::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GSwarm::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSwarm *p_load = GObject::conversion_cast<GSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithm::checkRelationshipWith(cp, e, limit, "GEvolutionaryAlgorithm", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GSwarm", nNeighborhoods_, p_load->nNeighborhoods_, "nNeighborhoods_", "p_load->nNeighborhoods_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", defaultNNeighborhoodMembers_, p_load->defaultNNeighborhoodMembers_, "defaultNNeighborhoodMembers_", "p_load->defaultNNeighborhoodMembers_", e , limit));
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		std::string local = "nNeighborhoodMembers_[" + boost::lexical_cast<std::string>(i) + "]";
		std::string remote = "(p_load->nNeighborhoodMembers_)[" + boost::lexical_cast<std::string>(i) + "]";
		deviations.push_back(checkExpectation(withMessages, "GSwarm", nNeighborhoodMembers_[i], (p_load->nNeighborhoodMembers_)[i], local, remote, e , limit));
	}

	deviations.push_back(checkExpectation(withMessages, "GSwarm", global_best_, p_load->global_best_, "global_best_", "p_load->global_best_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", local_bests_, p_load->local_bests_, "local_bests_", "p_load->local_bests_", e , limit));

	return evaluateDiscrepancies("GSwarm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Sets the individual's personality types to EA
 */
void GSwarm::setIndividualPersonalities() {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it)
		(*it)->setPersonality(SWARM);
}

/************************************************************************************************************/
/**
 * Saves the state of the class to disc. The function adds the current generation
 * and the fitness to the base name. The entire object is saved.
 */
void GSwarm::saveCheckpoint() const {
#ifdef DEBUG
	// Check that the global best has been initialized
	if(!global_best_) {
		std::ostringstream error;
		error << "In GSwarm::saveCheckpoint(): Error!" << std::endl
			  << "global_best_ has not yet been initialized!" << std::endl;
		throw geneva_error_condition(error.str());
	}
#endif /* DEBUG */

	bool isDirty;
	double newValue = global_best_->getCurrentFitness(isDirty);

#ifdef DEBUG
	if(isDirty) {
		std::ostringstream error;
		error << "In GSwarm::saveCheckpoint(): Error!" << std::endl
			  << "global_best_ is dirty!" << std::endl;
		throw geneva_error_condition(error.str());
	}
#endif

	// Determine a suitable name for the output file
	std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(getIteration()) + "_"
		+ boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

	toFile(outputFile, getCheckpointSerializationMode());
}

/************************************************************************************************************/
/**
 * Loads the state of the object from disc.
 *
 * @param cpFile The name of the file the checkpoint should be loaded from
 */
void GSwarm::loadCheckpoint(const std::string& cpFile) {
	fromFile(cpFile, getCheckpointSerializationMode());
}

/************************************************************************************************************/
/**
 * Emits information specific to this population. The function can be overloaded
 * in derived classes. By default we allow the user to register a call-back function
 * using GSwarm::registerInfoFunction() . Please note that it is not
 * possible to serialize this function, so it will only be active on the host were
 * it was registered, but not on remote systems.
 *
 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
 */
void GSwarm::doInfo(const infoMode& im) {
	if(!infoFunction_.empty()) infoFunction_(im, this);
}

/************************************************************************************************************/
/**
 * The user can specify what information should be emitted in a call-back function
 * that is registered in the setup phase. This functionality is based on boost::function .
 *
 * @param infoFunction A Boost.function object allowing the emission of information
 */
void GSwarm::registerInfoFunction(boost::function<void (const infoMode&, GSwarm * const)> infoFunction) {
	infoFunction_ = infoFunction;
}

/************************************************************************************************************/
/**
 * This function does some preparatory work and tagging required by swarm algorithms. It is called
 * from within GOptimizationAlgorithm::optimize(), immediately before the actual optimization cycle starts.
 */
void GSwarm::init() {
	// To be performed before any other action
	GOptimizationAlgorithm::init();

	// Attach the relevant information to the individual's personalities
	initPersonalities();
}

/************************************************************************************************************/
/**
 * Helper function that initializes the personality information.
 */
void GSwarm::initPersonalities() {
	// Set the individual's positions and attach swarm adaptors to the individuals.
	// Setting the position needs to be done only once before the start of the optimization
	// cycle, as individuals do not change position in a swarm algorithm.
	std::size_t pos=0;
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it, ++pos) {
		// Make the position known to the individual
		(*it)->getSwarmPersonalityTraits()->setPopulationPosition(pos);
	}
}

/************************************************************************************************************/
/**
 * This function implements the logic that constitutes each cycle of a swarm algorithm. The
 * function is called by GOptimizationAlgorithm for each iteration of the optimization,
 *
 * @return The value of the best individual found
 */
double GSwarm::cycleLogic() {
	// Modifies the individual's positions, then triggers the fitness calculation of all individuals.
	// This function can be overloaded in derived classes so that part of the modification, as well as
	// fitness calculation are performed in parallel.
	updatePositionsAndFitness();

	// Search for the locally and globally best individuals in all neighborhoods and
	// update the list of locally best solutions, if necessary
	return findBests();
}

/************************************************************************************************************/
/**
 * Modifies the particle positions, then triggers fitness calculation for all individuals. This function can
 * be overloaded by derived classes so the fitness calculation can be performed in parallel.
 */
void GSwarm::updatePositionsAndFitness() {
	std::size_t offset = 0;
	GSwarm::iterator start = this->begin();
	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
		for(std::size_t member=0; member<nNeighborhoodMembers_[neighborhood]; member++) {
			// Update the swarm positions. Note that this only makes sense
			// once the first series of evaluations has been done and local as well
			// as global bests are known
			if(getIteration() > 0) {
				// Add the local and global best to each individual
				(*(start + offset))->getSwarmPersonalityTraits()->registerLocalBest(local_bests_[neighborhood]);
				(*(start + offset))->getSwarmPersonalityTraits()->registerGlobalBest(global_best_);

				// Make sure the individual's parameters are updated
				(*(start + offset))->getSwarmPersonalityTraits()->updateParameters();
			}

			// Trigger the actual fitness calculation
			(*(start + offset))->fitness();

			offset++;
		}
	}
}

/************************************************************************************************************/
/**
 * Updates the best individuals found
 *
 * @return The best evaluation found in this iteration
 */
double GSwarm::findBests() {
	double bestFitnessInThisIteration = getWorstCase();
	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
		boost::shared_ptr<GIndividual> neighborhoodBest = *(this->begin() + (neighborhood*nNeighborhoodMembers_));

		// Search in the local neighborhood for the best solution
		for(std::size_t member=0; member<nNeighborhoodMembers_; member++) {
			boost::shared_ptr<GIndividual> currentIndividual = *(this->begin() + (neighborhood*nNeighborhoodMembers_ + member));
			double currentFitness = currentIndividual->fitness();
			if(isBetter(currentFitness, neighborhoodBest->fitness())) {
				currentIndividual = neighborhoodBest;
			}

			// Make sure we know when a better solution was found
			if(isBetter(currentFitness, bestFitnessInThisIteration)) {
				bestFitnessInThisIteration = currentFitness;
			}
		}

		// Update the bests
		if(getIteration() > 0) {
			if(isBetter(neighborhoodBest->fitness(), local_bests_[neighborhood]->fitness())) {
				local_bests_[neighborhood]->load(neighborhoodBest);
			}

			// Note that the following can result in some unnecessary loading, if more than one
			// better solution was found
			if(isBetter(neighborhoodBest->fitness(), global_best_->fitness())) {
				global_best_->load(neighborhoodBest);
			}
		}
		// This is the first generation, we need to attach a cloned individual to the list instead of loading it
		else {
			local_bests_.push_back(neighborhoodBest->clone());

			// We need to initialize the global best
			if(neighborhood == 0) {
				global_best_ = neighborhoodBest->clone();
			}
			// We need to compare with the existing global best
			else {
				if(isBetter(neighborhoodBest->fitness(), global_best_->fitness())) {
					global_best_->load(neighborhoodBest);
				}
			}
		}
	}

	return bestFitnessInThisIteration;
}

/************************************************************************************************************/
/**
 * Does any necessary finalization work
 */
void GSwarm::finalize() {
	// Last action
	GOptimizationAlgorithm::finalize();
}

/************************************************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks. This is an overloaded version
 * from GOptimizationAlgorithm::adjustPopulation() which is needed to take into account varying number of
 * individuals in each neighborhood. E.g., it will remove the worst individuals in each neighborhood instead
 * of just removing individuals at the end, in case of surplus items.
 */
void GSwarm::adjustPopulation() {

}

/************************************************************************************************************/
/**
 * Sets the local multiplier used when calculating velocities to a fixed value
 * in all individuals. This function results in a fixed factor.
 *
 * @param cl The multiplication factor for the difference between a local particle and and the local best
 */
void GSwarm::setCLocal(const double& cl) {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->getSwarmPersonalityTraits()->setCLocal(cl);
	}
}

/************************************************************************************************************/
/**
 * Sets the local multiplier of each individual randomly within a given range in each iteration
 */
void GSwarm::setCLocal(const double& cl_lower, const double& cl_upper) {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->getSwarmPersonalityTraits()->setCLocal(cl_lower, cl_upper);
	}
}

/************************************************************************************************************/
/**
 * Sets the global multiplier used when calculating velocities to a fixed value in
 * all individuals
 */
void GSwarm::setCGlobal(const double& cg) {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->getSwarmPersonalityTraits()->setCGlobal(cg);
	}
}

/************************************************************************************************************/
/**
 * Sets the global multiplier of each individual randomly within a given range in each iteration
 */
void GSwarm::setCGlobal(const double& cg_lower, const double& cg_upper) {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->getSwarmPersonalityTraits()->setCGlobal(cg_lower, cg_upper);
	}
}

/************************************************************************************************************/
/**
 * Sets the velocity multiplier to a fixed value for each individual
 */
void GSwarm::setCDelta(const double& cd) {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->getSwarmPersonalityTraits()->setCDelta(cd);
	}
}

/************************************************************************************************************/
/**
 * Sets the velocity multiplier to a random value separately for each individual in each iteration
 */
void GSwarm::setCDelta(const double& cd_lower, const double& cd_upper) {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->getSwarmPersonalityTraits()->setCDelta(cd_lower, cd_upper);
	}
}

/************************************************************************************************************/
/**
 * Sets the population size based on the number of neighborhoods and the number of
 * individuals in them
 *
 * @param nNeighborhoods The number of neighborhoods in the population
 * @param nNeighborhoodMembers The number of individuals in each neighborhood
 */
void GSwarm::setPopulationSize(const std::size_t& nNeighborhoods, const std::size_t& nNeighborhoodMembers) {
	nNeighborhoods_ = nNeighborhoods;
	nNeighborhoodMembers_ = nNeighborhoodMembers;


}

/************************************************************************************************************/
/**
 * Retrieves the number of neighborhoods
 *
 * @return The number of neighborhoods in the population
 */
std::size_t GSwarm::getNNeighborhoods() const {
	return nNeighborhoodMembers_;
}

/************************************************************************************************************/
/**
 * Retrieves the default number of individuals in each neighborhood
 *
 * @return The default number of individuals in each neighborhood
 */
std::size_t GSwarm::getDefaultNNeighborhoodMembers() const {
	return defaultNNeighborhoodMembers_;
}

/************************************************************************************************************/
/**
 * Retrieves the current number of individuals in a given neighborhood
 *
 * @return The current number of individuals in a given neighborhood
 */
std::size_t getCurrentNNeighborhoodMembers(const std::size_t& neighborhood) const {

}

/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
