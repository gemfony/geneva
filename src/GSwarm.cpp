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
 * The default constructor, As we do not have any individuals yet, we set the population
 * size, and number of parents to 0. It is the philosophy of this class not
 * to provide constructors for each and every use case. Instead, you should set
 * vital parameters, such as the population size or the parent individuals by hand.
 */
GSwarm::GSwarm()
	: GOptimizationAlgorithm()
	, infoFunction_(&GSwarm::simpleInfoFunction)
{
	setPopulationSize(DEFAULTNNEIGHBORHOODS, DEFAULTNNEIGHBORHOODMEMBERS);
}

/************************************************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GSwarm object
 */
GSwarm::GSwarm(const GSwarm& cp)
	: GOptimizationAlgorithm(cp)
	, infoFunction_(&GSwarm::simpleInfoFunction) // Note that we do not copy the info function
{
	setPopulationSize(cp.nNeighborhoods_, cp.nNeighborhoodMembers_);

	global_best_ = (cp.global_best_)->clone<GIndividual>();
	std::vector<boost::shared_ptr<GIndividual> >::const_iterator cit;
	for(cit=cp.local_bests_.begin(); cit!=cp.local_bests_.end(); ++cit) {
		local_bests_.push_back((*cit)->clone<GIndividual>());
	}
}

/************************************************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GSwarm::~GSwarm()
{ /* nothing */ }

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
void GSwarm::load_(const GObject * cp)
{
	const GSwarm *p_load = this->conversion_cast<GSwarm>(cp);

	// First load the parent class'es data ...
	GOptimizationAlgorithm::load_(cp);

	// ... and then our own data
	nNeighborhoods_ = p_load->nNeighborhoods_;
	nNeighborhoodMembers_ = p_load->nNeighborhoodMembers_;

#ifdef DEBUG
	if(this->size() != nNeighborhoods_*nNeighborhoodMembers_) {
		std::ostringstream error;
		error << "In GSwarm::load_(): Error!" << std::endl
			  << "nNeighborhoods_*nNeighborhoodMembers_ = " << nNeighborhoods_ << "*" << nNeighborhoodMembers_ << " = " << nNeighborhoods_*nNeighborhoodMembers_ << std::endl
			  << "is not the same as the population size " << this->size() << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}
#endif /* DEBUG */

	global_best_->GObject::load(p_load->global_best_);
	copySmartPointerVector(p_load->local_bests_, local_bests_);

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
	deviations.push_back(checkExpectation(withMessages, "GSwarm", nNeighborhoodMembers_, p_load->nNeighborhoodMembers_, "nNeighborhoodMembers_", "p_load->nNeighborhoodMembers_", e , limit));
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
	double bestFitnessInThisIterations = getWorstCase();
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
			if(isBetter(currentFitness, bestFitnessInThisIterations)) {
				bestFitnessInThisIterations = currentFitness;
			}
		}

		// Update the bests
		if(getIteration() > 0) {
			if(isBetter(neighborhoodBest->fitness(), local_bests_[neighborhoos]->fitness())) {
				local_bests_[neighborhoos]->load(neighborhoodBest);
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

	return bestFitnessInThisIterations;
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
		for(std::size_t member=0; member<nNeighborhoodMembers_; member++) {
			offset = neighborhood*nNeighborhoods_ + member;

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
		}
	}
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

	GOptimizationAlgorithm::setPopulationSize(nNeighborhoods_*nNeighborhoodMembers_);
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
 * Retrieves the number of individuals in each neighborhood
 *
 * @return The number of individuals in each neighborhood
 */
std::size_t GSwarm::getNNeighborhoodMembers() const {
	return nNeighborhoodMembers_;
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
