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

#include "geneva/GSwarm.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GSwarm)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * This constructor sets the number of neighborhoods and the number of individuals in them. Note that there
 * is no public default constructor, as it is only needed for de-serialization purposes.
 *
 * @param nNeighborhoods The desired number of neighborhoods (hardwired to >= 1)
 * @oaram nNeighborhoodMembers The default number of individuals in each neighborhood (hardwired to >= 2)
 */
GSwarm::GSwarm(const std::size_t& nNeighborhoods, const std::size_t& nNeighborhoodMembers)
	: GOptimizationAlgorithmT<GParameterSet>()
	, infoFunction_(&GSwarm::simpleInfoFunction)
	, nNeighborhoods_(nNeighborhoods?nNeighborhoods:1)
	, defaultNNeighborhoodMembers_((nNeighborhoodMembers<=1)?2:nNeighborhoodMembers)
	, nNeighborhoodMembers_(new std::size_t[nNeighborhoods_])
	, local_bests_(new boost::shared_ptr<GParameterSet>[nNeighborhoods_])
	, c_local_(DEFAULTCLOCAL)
	, c_global_(DEFAULTCGLOBAL)
	, c_delta_(DEFAULTCDELTA)
{
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Initialize with the default number of members in each neighborhood. adjustPopulation() will
	// later take care to fill the population with individuals as needed.
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
	: GOptimizationAlgorithmT<GParameterSet>(cp)
	, infoFunction_(&GSwarm::simpleInfoFunction) // Note that we do not copy the info function
	, nNeighborhoods_(cp.nNeighborhoods_)
	, defaultNNeighborhoodMembers_(cp.defaultNNeighborhoodMembers_)
	, nNeighborhoodMembers_(new std::size_t[nNeighborhoods_])
	, global_best_((cp.getIteration()>0)?(cp.global_best_)->clone<GParameterSet>():boost::shared_ptr<GParameterSet>())
	, local_bests_(new boost::shared_ptr<GParameterSet>[nNeighborhoods_])
	, c_local_(cp.c_local_)
	, c_global_(cp.c_global_)
	, c_delta_(cp.c_delta_)
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
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Clone cp's locally best individuals, if this is not the first iteration
	if(cp.getIteration()>0) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			local_bests_[i] = cp.local_bests_[i]->clone<GParameterSet>();
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
 *
 * TODO: Add checks in DEBUG mode whether the smart pointers being cloned or copied actually point somewhere
 */
void GSwarm::load_(const GObject *cp)
{
	// Make a note of the current iteration (needed for a check below).
	// The information would otherwise be lost after the load call below
	boost::uint32_t currentIteration = this->getIteration();

	const GSwarm *p_load = this->conversion_cast<GSwarm>(cp);

	// First load the parent class'es data.
	// This will also take care of copying all individuals.
	GOptimizationAlgorithmT<GParameterSet>::load_(cp);

	// ... and then our own data
	defaultNNeighborhoodMembers_ = p_load->defaultNNeighborhoodMembers_;
	c_local_ = p_load->c_local_;
	c_global_ = p_load->c_global_;
	c_delta_ = p_load->c_delta_;

	// We start from scratch if the number of neighborhoods or the alleged number of members in them differ
	if(nNeighborhoods_!=p_load->nNeighborhoods_ || !nNeighborhoodMembersEqual(nNeighborhoodMembers_, p_load->nNeighborhoodMembers_)) {
		nNeighborhoods_ = p_load->nNeighborhoods_;

		delete [] nNeighborhoodMembers_;
		delete [] local_bests_;

		nNeighborhoodMembers_ = new std::size_t[nNeighborhoods_];
		local_bests_ = new boost::shared_ptr<GParameterSet>[nNeighborhoods_];

		// Copy the local bests and number of neighborhood members over
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			nNeighborhoodMembers_[i] = p_load->nNeighborhoodMembers_[i];
			local_bests_[i] = p_load->local_bests_[i]->clone<GParameterSet>();
		}
	}
	else { // We now assume that we can just load local bests in each position
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			local_bests_[i]->GObject::load(p_load->local_bests_[i]);
		}
	}

	// Copy the global best over
	if(currentIteration == 0 && p_load->getIteration() > 0) { // cp has a global best, we don't
		global_best_ = p_load->GObject::clone<GParameterSet>();
	}
	else if(currentIteration > 0 && p_load->getIteration() == 0) { // cp does not have a global best
		global_best_.reset(); // empty the smart pointer
	}
	// else {} // We do not need to do anything if both iterations are 0 as there is no global best at all

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
	using namespace Gem::Common;
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
	using namespace Gem::Common;
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
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSwarm *p_load = GObject::conversion_cast<GSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GOptimizationAlgorithmT<GParameterSet>", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GSwarm", nNeighborhoods_, p_load->nNeighborhoods_, "nNeighborhoods_", "p_load->nNeighborhoods_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", defaultNNeighborhoodMembers_, p_load->defaultNNeighborhoodMembers_, "defaultNNeighborhoodMembers_", "p_load->defaultNNeighborhoodMembers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", global_best_, p_load->global_best_, "global_best_", "p_load->global_best_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_local_, p_load->c_local_, "c_local_", "p_load->c_local_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_global_, p_load->c_global_, "c_global_", "p_load->c_global_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_delta_, p_load->c_delta_, "c_delta_", "p_load->c_delta_", e , limit));

	// The next checks only makes sense if the number of neighborhoods are equal
	if(nNeighborhoods_ == p_load->nNeighborhoods_) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			std::string local = "nNeighborhoodMembers_[" + boost::lexical_cast<std::string>(i) + "]";
			std::string remote = "(p_load->nNeighborhoodMembers_)[" + boost::lexical_cast<std::string>(i) + "]";
			deviations.push_back(checkExpectation(withMessages, "GSwarm", nNeighborhoodMembers_[i], (p_load->nNeighborhoodMembers_)[i], local, remote, e , limit));
		}

		deviations.push_back(checkExpectation(withMessages, "GSwarm", local_bests_, p_load->local_bests_, "local_bests_", "p_load->local_bests_", e , limit));
	}

	return evaluateDiscrepancies("GSwarm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Sets the individual's personality types to Swarm
 */
void GSwarm::setIndividualPersonalities() {
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->setPersonality(SWARM);
	}
}

/************************************************************************************************************/
/**
 * Saves the state of the class to disc. The function adds the current generation
 * and the fitness to the base name. The entire object is saved. The function will
 * throw if no global best has been established yet.
 */
void GSwarm::saveCheckpoint() const {
#ifdef DEBUG
	// Check that the global best has been initialized
	if(!global_best_) {
		std::ostringstream error;
		error << "In GSwarm::saveCheckpoint(): Error!" << std::endl
			  << "global_best_ has not yet been initialized!" << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}
#endif /* DEBUG */

	bool isDirty;
	double newValue = global_best_->getCurrentFitness(isDirty);

#ifdef DEBUG
	if(isDirty) {
		std::ostringstream error;
		error << "In GSwarm::saveCheckpoint(): Error!" << std::endl
			  << "global_best_ has dirty flag set!" << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}
#endif

	// Determine a suitable name for the output file
	std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(getIteration()) + "_"
		+ boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

	this->toFile(outputFile, getCheckpointSerializationMode());
}

/************************************************************************************************************/
/**
 * Loads the state of the object from disc.
 *
 * @param cpFile The name of the file the checkpoint should be loaded from
 */
void GSwarm::loadCheckpoint(const std::string& cpFile) {
	this->fromFile(cpFile, getCheckpointSerializationMode());
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
 * from within GOptimizationAlgorithmT<GParameterSet>::optimize(), immediately before the actual optimization cycle starts.
 */
void GSwarm::init() {
	// To be performed before any other action
	GOptimizationAlgorithmT<GParameterSet>::init();

	// Create copies of our individuals in the velocities_ vector.
	for(GSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
		// Create a copy of the current individual. Note that, if you happen
		// to have assigned anything else than a GParameterSet derivative to
		// the swarm, then the following line will throw in DEBUG mode or return
		// undefined results in RELEASE mode
		boost::shared_ptr<GParameterSet> p((*it)->clone<GParameterSet>());
		// Assign the value 0. to all floating point parameters
		p->fpFixedValueInit(float(0.));
		// Add the zero-initialized individual to the velocity array.
		// The necessary downcast will be handled by
		velocities_.push_back(p);
	}
}

/************************************************************************************************************/
/**
 * Does any necessary finalization work
 */
void GSwarm::finalize() {
	// Remove remaining velocity individuals. The boost::shared_ptr<GParameterSet>s
	// will take care of deleting the GParameterSet objects.
	velocities_.clear();

	// Last action
	GOptimizationAlgorithmT<GParameterSet>::finalize();
}

/************************************************************************************************************/
/**
 * Helper function that checks the content of two nNeighborhoodMembers_ arrays. Note that this private function
 * assumes that both arrays contain nNeighborhoods_ entries.
 *
 * @param one The first array used for the check
 * @param two The second array used for the check
 * @return A boolean indicating whether both arrays are equal
 */
bool GSwarm::nNeighborhoodMembersEqual(const std::size_t *one, const std::size_t* two) const {
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		if(one[i] != two[i]) return false;
	}

	return true;
}

/************************************************************************************************************/
/**
 * Helper function that returns the id of the first individual of a neighborhood. "NI" stands
 * for NeighborhoodIndividual. "neighborhood" is assumed to be a counter, starting at 0 and assuming
 * a maximum value of (nNeighborhoods_-1).
 *
 * @param neighborhood The id of the neighborhood for which the id of the first individual should be calculated
 * @return The position of the first individual of a neighborhood
 */
std::size_t GSwarm::getFirstNIPos(const std::size_t& neighborhood) const {
#ifdef DEBUG
	if(neighborhood >= nNeighborhoods_) {
		std::ostringstream error;
		error << "In GSwarm::getFirstNIPos(): Error!" << std::endl
			  << "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
			  << "The number of neighborhoods is " << nNeighborhoods_ << "," << std::endl
			  << "hence the maximum allowed value of the id is " << nNeighborhoods_-1 << "." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	if(neighborhood == 0) return 0;
	else {	// Sum up the number of members in each neighborhood
		std::size_t nPreviousMembers=0;
		for(std::size_t n=0; n<neighborhood; n++) {
			nPreviousMembers += nNeighborhoodMembers_[n];
		}

		return nPreviousMembers;
	}
}

/************************************************************************************************************/
/**
 * Helper function that helps to determine the end of a neighborhood. "NI" stands
 * for NeighborhoodIndividual. "neighborhood" is assumed to be a counter, starting at 0 and assuming
 * a maximum value of (nNeighborhoods_-1). The position returned is that right after the last individual,
 * as is common in C++ .
 *
 * @param neighborhood The id of the neighborhood for which the id of the last individual should be calculated
 * @return The position of the individual right after the last of a neighborhood
 *
 * TODO: ATTENTION: Does this work for the last neighborhood ? The id returned does not exist ...
 */
std::size_t GSwarm::getLastNIPos(const std::size_t& neighborhood) const {
#ifdef DEBUG
	if(neighborhood >= nNeighborhoods_) {
		std::ostringstream error;
		error << "In GSwarm::getLastNIPos(): Error!" << std::endl
			  << "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
			  << "The number of neighborhoods is " << nNeighborhoods_ << " ." << std::endl
			  << "hence the maximum allowed value of the id is " << nNeighborhoods_-1 << "." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	return getFirstNIPos(neighborhood) + nNeighborhoodMembers_[neighborhood];
}

/************************************************************************************************************/
/**
 * This function implements the logic that constitutes each cycle of a swarm algorithm. The
 * function is called by GOptimizationAlgorithmT<GParameterSet>::optimize() for each iteration of
 * the optimization,
 *
 * @return The value of the best individual found
 */
double GSwarm::cycleLogic() {
	// Modifies the individual's parameters, then triggers the fitness calculation of all individuals
	// and identifies the local and global bests. This function can be overloaded in derived classes
	// so that part of the modification and/or fitness calculation are performed in parallel.
	updatePositionsAndFitness();

	// Search for the locally and globally best individuals in all neighborhoods and
	// update the list of locally best solutions, if necessary
	double bestLocalFitness = findBests();

	// Makes sure that each neighborhood has the right size before the next cycle starts
	adjustNeighborhoods();

	return bestLocalFitness;
}

/************************************************************************************************************/
/**
 * Modifies the particle positions, then triggers fitness calculation for all individuals. This function can
 * be overloaded by derived classes so the fitness calculation can be performed in parallel.
 */
void GSwarm::updatePositionsAndFitness() {
	std::size_t offset = 0;
	GSwarm::iterator start = this->begin();
	boost::uint32_t iteration = getIteration();

	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
#ifdef DEBUG
		if(getIteration() > 0) {
			if(!local_bests_[neighborhood]) {
				std::ostringstream error;
				error << "In GSwarm::updatePositionsAndFitness(): Error!" << std::endl
					  << "local_bests[" << neighborhood << "] is empty." << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			if(neighborhood==0 && !global_best_) { // Only check for the first neighborhood
				std::ostringstream error;
				error << "In GSwarm::updatePositionsAndFitness(): Error!" << std::endl
					  << "global_best_ is empty." << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
		}
#endif /* DEBUG */

		for(std::size_t member=0; member<nNeighborhoodMembers_[neighborhood]; member++) {
			GSwarm::iterator current = start + offset;

			if(iteration > 0 && !(*current)->getSwarmPersonalityTraits()->checkNoPositionUpdateAndReset()) {
				swarmLogic (
					  neighborhood
					, *current
					, local_bests_[neighborhood]->clone<GParameterSet>()
					, global_best_->clone<GParameterSet>()
					, velocities_[offset]
					, getCLocal()
					, getCGlobal()
					, getCDelta()
				);

			} else { // the first iteration
				swarmLogicNoUpdate (
					  neighborhood
				    , *current
				);
			}

			offset++;
		}
	}
}

/************************************************************************************************************/
/**
 * Triggers the fitness calculation in the first iteration as well as for randomly initialized items.
 *
 * @param neighborhood The neighborhood the individual is in
 * @param ind The individual for which the fitness calculation should be performed
 */
void GSwarm::swarmLogicNoUpdate(
	    std::size_t neighborhood
	  , boost::shared_ptr<GParameterSet> ind
){
	// Let the personality know in which neighborhood it is
	ind->getSwarmPersonalityTraits()->setNeighborhood(neighborhood);

	// Trigger the fitness calculation (if necessary). Make sure lazy evaluation
	// is allowed so we do not trigger an exception.
	bool originalAllowLazyEvaluation = ind->setAllowLazyEvaluation(true);
	ind->fitness();
	ind->setAllowLazyEvaluation(originalAllowLazyEvaluation);
}

/************************************************************************************************************/
/**
 * Updates the individual's position and performs the fitness calculation
 *
 * @param neighborhood The neighborhood that has been assigned to the individual
 * @param ind The individual whose position should be updated
 * @param local_best_tmp The locally best dataset of the individual's neighborhood
 * @param global_best_tmp The globally best individual so far
 * @param velocity A velocity vector
 * @param cLocal A constant used for multiplication with the local direction
 * @param cGlobal A constant used for multiplication with the global direction
 * @param cDelta A constant used for multiplication with the velocity vector
 */
void GSwarm::swarmLogic(
	    std::size_t neighborhood
	  , boost::shared_ptr<GParameterSet> ind
	  , boost::shared_ptr<GParameterSet> local_best_tmp
	  , boost::shared_ptr<GParameterSet> global_best_tmp
	  , boost::shared_ptr<GParameterSet> velocity
	  , double cLocal
	  , double cGlobal
	  , double cDelta
){
	// Let the personality know in which neighborhood it is
	ind->getSwarmPersonalityTraits()->setNeighborhood(neighborhood);

	// Update the swarm positions:

	// Subtract the current individual
	local_best_tmp->fpSubtract(ind);
	global_best_tmp->fpSubtract(ind);

	// Multiply each floating point value with a random fp number in the range [0,1[
	local_best_tmp->fpMultiplyByRandom();
	global_best_tmp->fpMultiplyByRandom();

	// Multiply each floating point value with a fixed, configurable constant value
	local_best_tmp->fpMultiplyBy(cLocal);
	global_best_tmp->fpMultiplyBy(cGlobal);

	// Multiply the last iterations velocity with a fixed, configurable constant value
	velocity->fpMultiplyBy(cDelta);

	// Add the local and global tmps
	velocity->fpAdd(local_best_tmp);
	velocity->fpAdd(global_best_tmp);

	// Add the velocity to the current individual
	ind->fpAdd(velocity);

	// Trigger the fitness calculation (if necessary). Make sure lazy evaluation
	// is allowed so we do not trigger an exception.
	bool originalAllowLazyEvaluation = ind->setAllowLazyEvaluation(true);
	ind->fitness();
	ind->setAllowLazyEvaluation(originalAllowLazyEvaluation);
}

/************************************************************************************************************/
/**
 * Updates the best individuals found. This function assumes that the population already contains individuals
 * and that the local and global bests have been initialized (possibly with dummy values). This should have
 * happened in the adjustPopulation() function. It also assumes that all individuals have already been evaluated.
 *
 * TODO: Add a Debug-check whether there are any individuals that have not yet been evaluated
 *
 * @return The best evaluation found in this iteration
 */
double GSwarm::findBests() {
	std::size_t bestCurrentLocalId = 0;
	double bestCurrentLocalFitness = getWorstCase();

	// Sort all neighborhoods according to their fitness
	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
		// identify the first and last id of the individuals in the current neighborhood
		std::size_t firstCounter = getFirstNIPos(neighborhood);
		std::size_t lastCounter = getLastNIPos(neighborhood);

		// Only partially sort the arrays
		if(getMaximize()){
			std::sort(this->begin() + firstCounter, this->begin() + lastCounter,
					boost::bind(&GIndividual::fitness, _1) > boost::bind(&GIndividual::fitness, _2));
		}
		else{
			std::sort(this->begin() + firstCounter, this->begin() + lastCounter,
					boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
		}

		// Check whether the best individual of the neighborhood is better than
		// the best individual found so far in this neighborhood
		if(getIteration() == 0) {
			local_bests_[neighborhood] = (*(this->begin() + firstCounter))->clone<GParameterSet>();
		}
		else {
			if(isBetter((*(this->begin() + firstCounter))->fitness(), local_bests_[neighborhood]->fitness())) {
				local_bests_[neighborhood]->load(*(this->begin() + firstCounter));
			}
		}

		// Find out which is the "best local best"
		if(isBetter((*(this->begin() + firstCounter))->fitness(), bestCurrentLocalFitness)) {
			bestCurrentLocalFitness = (*(this->begin() + firstCounter))->fitness();
			bestCurrentLocalId = neighborhood;
		}
	}

	// Compare the best local individual with the global best and update
	// the global best, if necessary. Initialize it in the first generation.
	if(getIteration() == 0) {
		global_best_= local_bests_[bestCurrentLocalId]->clone<GParameterSet>();
	}
	else {
		if(isBetter(bestCurrentLocalFitness, global_best_->fitness())) {
			global_best_->load(local_bests_[bestCurrentLocalId]);
		}
	}

	return bestCurrentLocalFitness;
}

/************************************************************************************************************/
/**
 * This function repairs the population by adding or removing missing or surplus items. It assumes that
 * the entries in each neighborhood are sorted by fitness. This function relies on the information found
 * in the nNeighborhoodMembers_ array, i.e. the current number of individuals in each neighborhood, as
 * well as the default number of individuals in each neighborhood. The function also assumes that the
 * neighborhoods have been sorted, so that the worst individuals can be found at the end of the range. It
 * will then remove the worst items only. Newly added items will start randomly initialized.
 */
void GSwarm::adjustNeighborhoods() {
	// Loop over all neighborhoods
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		// We need to remove surplus items
		if(nNeighborhoodMembers_[n] > defaultNNeighborhoodMembers_) {
			// Find out, how many surplus items there are
			std::size_t nSurplus = nNeighborhoodMembers_[n] - defaultNNeighborhoodMembers_;

			// Remove a corresponding amount of items from the position (n+1)*defaultNNeighborhoodMembers_
			for(std::size_t i=0; i<nSurplus; i++) {
				this->erase(this->begin() + (n+1)*defaultNNeighborhoodMembers_);
			}
		}
		// Some items need to be added. Note that this implies cloning
		// one of the existing individuals, and random initialization.
		else if (nNeighborhoodMembers_[n] < defaultNNeighborhoodMembers_) {
			// Find out, how many missing items there are
			std::size_t nMissing = nNeighborhoodMembers_[n] - defaultNNeighborhoodMembers_;

#ifdef DEBUG
			// This number shouldn't equal the default number of entries
			if(nMissing == defaultNNeighborhoodMembers_) {
				std::ostringstream error;
				error << "In GSwarm::adjustNeighborhoods(): Error!" << std::endl
					  << "Found no entries in the neighborhood." << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
#endif /* DEBUG */

			// Insert items at the position n*defaultNNeighborhoodMembers_ (i.e. at the beginning of the range).
			// We use the first item of the range as a template, then randomly initialize the data item.
			this->insert(this->begin() + n*defaultNNeighborhoodMembers_, (*(this->begin() + n*defaultNNeighborhoodMembers_))->clone<GParameterSet>());
			(*(this->begin() + n*defaultNNeighborhoodMembers_))->randomInit();
			(*(this->begin() + n*defaultNNeighborhoodMembers_))->getSwarmPersonalityTraits()->setNoPositionUpdate();
		}
	}
}

/************************************************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks. This function implements
 * the purely virtual function GOptimizationAlgorithmT<GParameterSet>::adjustPopulation() .
 */
void GSwarm::adjustPopulation() {
	std::size_t currentSize = this->size();
	std::size_t defaultPopSize = getDefaultPopulationSize();

	// Check the actual population
	if(currentSize == 0) {
		std::ostringstream error;
		error << "In GSwarm::adjustPopulation() : Error!" << std::endl
			  << "No individuals found in the population." << std::endl
			  << "You need to add at least one individual before" << std::endl
			  << "the call to optimize()." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
	else if(currentSize == 1) {
		for(std::size_t i=1; i<defaultPopSize; i++) {
			this->push_back(this->front()->clone<GParameterSet>());
			this->back()->randomInit();
			this->back()->getSwarmPersonalityTraits()->setNoPositionUpdate();
		}
	}
	else if(currentSize == nNeighborhoods_) {
		fillUpNeighborhood1();
	}
	else if(currentSize == defaultPopSize) { // The default size
		// nothing to do
	}
	else {
		if(currentSize < nNeighborhoods_) {
			// First fill up the neighborhoods, if required
			for(std::size_t m=0; m < (nNeighborhoods_-currentSize); m++) {
				this->push_back(this->front()->clone<GParameterSet>());
				this->back()->randomInit();
				this->back()->getSwarmPersonalityTraits()->setNoPositionUpdate();
			}

			// Now follow the procedure used for the "nNeighborhoods_" case
			fillUpNeighborhood1();
		}
		else if(currentSize > nNeighborhoods_ && currentSize < defaultPopSize) {
			// TODO: For now we simply fill up the population with random entries.
			// This means that there may be neighborhoods in which there is no predefined
			// individual. In principle, though, we have enough data to "man" the
			// first position of each neighborhood and this should be done later.
			// Possible algorithm:
			// -- Transfer all complete pointers to another vector b
			// -- Initialize all local pointers with random objects
			// -- Copy the pointers from b to the neighborhoods in a round-robin fashion
			// or
			// -- Transfer all complete pointers to another vector b
			// -- while nPointers - nNeighborhoods > 0: copy as many pointers as
			// possible into the current neighborhood
			// -- Continue with next neighborhood
			// -- Fill up and initialize missing entries

			std::size_t nMissing = defaultPopSize - currentSize;
			for(std::size_t m=0; m<nMissing; m++) {
				this->push_back(this->front()->clone<GParameterSet>());
				this->back()->randomInit();
				this->back()->getSwarmPersonalityTraits()->setNoPositionUpdate();
			}
		}
		else { // currentSize > defaultPopsize
			// Adjust the nNeighborhoodMembers_ array. The surplus items will
			// be assumed to belong to the last neighborhood, all other neighborhoods
			// have the default size.
			nNeighborhoodMembers_[nNeighborhoods_-1] = defaultNNeighborhoodMembers_ + (currentSize - defaultPopSize);
		}
	}

	// Cross check that we now indeed have at least the required number of individuals
	if(this->size() < defaultPopSize) {
		std::ostringstream error;
		error << "In GSwarm::adjustPopulation() : Error!" << std::endl
			  << "Expected at least a population size of " << defaultPopSize << std::endl
			  << "but found a size of " << this->size() << ", which is too small." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// We do not initialize the local and global bests here, as this requires the value of
	// all individuals to be calculated.
}

/************************************************************************************************************/
/**
 * Small helper function that helps to fill up a neighborhood, if there is just one entry in it.
 */
void GSwarm::fillUpNeighborhood1() {
	if(defaultNNeighborhoodMembers_==1) return; // nothing to do

	// Starting with the last item, loop over all neighborhoods
	for(std::size_t n=(nNeighborhoods_-1); n>=0; n--) {
		// Insert the required number of clones after the existing individual
		for(std::size_t m=1; m<defaultNNeighborhoodMembers_; m++) { // m stands for "missing"
			// Add a clone of the first individual in the neighborhood to the next position
			this->insert(this->begin()+n+1, (*(this->begin()+n))->clone<GParameterSet>());
			// Make sure it has a unique value
			(*(this->begin()+n+1))->randomInit();
			(*(this->begin()+n+1))->getSwarmPersonalityTraits()->setNoPositionUpdate();
		}
	}
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for local distances.
 */
void GSwarm::setCLocal(const float& c_local) {
	c_local_ = c_local;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances or the lower boundary of a random range
 */
float GSwarm::getCLocal() const {
	return c_local_;
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for global distances
 */
void GSwarm::setCGlobal(const float& c_global) {
	c_global_ = c_global;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances or the lower boundary of a random range
 */
float GSwarm::getCGlobal() const {
	return c_global_;
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for deltas
 */
void GSwarm::setCDelta(const float& c_delta) {
	c_delta_ = c_delta;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for deltas or the lower boundary of a random range
 */
float GSwarm::getCDelta() const {
	return c_delta_;
}

/************************************************************************************************************/
/**
 * Retrieves the number of neighborhoods
 *
 * @return The number of neighborhoods in the population
 */
std::size_t GSwarm::getNNeighborhoods() const {
	return nNeighborhoods_;
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
std::size_t GSwarm::getCurrentNNeighborhoodMembers(const std::size_t& neighborhood) const {
	return nNeighborhoodMembers_[neighborhood];
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GParameterSet>::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
