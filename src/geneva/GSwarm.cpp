/**
 * @file GSwarm.cpp
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
#include "geneva/GSwarm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSwarm)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSwarm::GSwarmOptimizationMonitor)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor. Intentionally protected and thus unaccessible by the general public, as it is
 * only needed for de-serialization purposes. The id of the optimization algorithm will be set when the
 * parent class is de-serialized.
 */
GSwarm::GSwarm()
{ /* nothing */ }

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
	, nNeighborhoods_(nNeighborhoods?nNeighborhoods:1)
	, defaultNNeighborhoodMembers_((nNeighborhoodMembers<=1)?2:nNeighborhoodMembers)
	, nNeighborhoodMembers_(nNeighborhoods_)
	, local_bests_(nNeighborhoods_)
	, c_personal_(DEFAULTCPERSONAL)
	, c_local_(DEFAULTCLOCAL)
	, c_global_(DEFAULTCGLOBAL)
	, c_delta_(DEFAULTCDELTA)
	, ur_(DEFAULTUPDATERULE)
	, randomFillUp_(true)
{
	GOptimizationAlgorithmT<GParameterSet>::setOptimizationAlgorithm(SWARM);
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Initialize with 0. adjustPopulation() will later take care to fill the population with individuals as needed.
	// and set the array to the correct values
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		nNeighborhoodMembers_[i] = 0;
	}

	// Register the default optimization monitor
	this->registerOptimizationMonitor(
			boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
					new GSwarmOptimizationMonitor()
			)
	);
}

/************************************************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GSwarm object
 */
GSwarm::GSwarm(const GSwarm& cp)
	: GOptimizationAlgorithmT<GParameterSet>(cp)
	, nNeighborhoods_(cp.nNeighborhoods_)
	, defaultNNeighborhoodMembers_(cp.defaultNNeighborhoodMembers_)
	, nNeighborhoodMembers_(cp.nNeighborhoodMembers_)
	, global_best_((cp.getIteration()>0)?(cp.global_best_)->clone<GParameterSet>():boost::shared_ptr<GParameterSet>())
	, local_bests_(nNeighborhoods_) // We copy the smart pointers over later
	, c_personal_(cp.c_personal_)
	, c_local_(cp.c_local_)
	, c_global_(cp.c_global_)
	, c_delta_(cp.c_delta_)
	, ur_(cp.ur_)
	, randomFillUp_(cp.randomFillUp_)
{
	// Note that this setting might differ from nCPIndividuals, as it is not guaranteed
	// that cp has, at the time of copying, all individuals present in each neighborhood.
	// Differences might e.g. occur if not all individuals return from their remote
	// evaluation. adjustPopulation will take care to resize the population appropriately
	// inside of the "optimize()" call.
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Clone cp's locally best individuals
	if(cp.getIteration()>0) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			local_bests_[i] = cp.local_bests_[i]->clone<GParameterSet>();
		}
	}

	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/************************************************************************************************************/
/**
 * The standard destructor. Most work is done in the parent class.
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
	c_personal_ = p_load->c_personal_;
	c_local_ = p_load->c_local_;
	c_global_ = p_load->c_global_;
	c_delta_ = p_load->c_delta_;
	ur_ = p_load->ur_;
	randomFillUp_ = p_load->randomFillUp_;

	// We start from scratch if the number of neighborhoods or the alleged number of members in them differ
	if(nNeighborhoods_!=p_load->nNeighborhoods_ || !nNeighborhoodMembersEqual(nNeighborhoodMembers_, p_load->nNeighborhoodMembers_)) {
		nNeighborhoods_ = p_load->nNeighborhoods_;

		nNeighborhoodMembers_.clear();
		local_bests_.clear();

		nNeighborhoodMembers_.resize(nNeighborhoods_);
		local_bests_.resize(nNeighborhoods_);

		// Copy the local bests and number of neighborhood members over
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			nNeighborhoodMembers_[i] = p_load->nNeighborhoodMembers_[i];
			// The following only makes sense if this is not the first iteration. Note that
			// getIteration will return the "foreign" GSwarm object's iteration, as it has
			// already been copied.
			if(getIteration() > 0) {
				local_bests_[i] = p_load->local_bests_[i]->clone<GParameterSet>();
			}
			// we do not need to reset the local_bests_, as that array has just been created
		}
	}
	else { // We now assume that we can just load local bests in each position.
		// Copying only makes sense of the foreign GSwarm object's iteration is > 0.
		// Note that getIteration() will return the foreign iteration, as that value
		// has already been copied.
		if(getIteration() > 0) {
			for(std::size_t i=0; i<nNeighborhoods_; i++) {
				local_bests_[i]->GObject::load(p_load->local_bests_[i]);
			}
		} else {
			for(std::size_t i=0; i<nNeighborhoods_; i++) {
				local_bests_[i].reset();
			}
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
boost::optional<std::string> GSwarm::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
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
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_personal_, p_load->c_personal_, "c_personal_", "p_load->c_personal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_local_, p_load->c_local_, "c_local_", "p_load->c_local_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_global_, p_load->c_global_, "c_global_", "p_load->c_global_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", c_delta_, p_load->c_delta_, "c_delta_", "p_load->c_delta_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", ur_, p_load->ur_, "ur_", "p_load->ur_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm", randomFillUp_, p_load->randomFillUp_, "randomFillUp_", "p_load->randomFillUp_", e , limit));

	// The next checks only makes sense if the number of neighborhoods are equal
	if(nNeighborhoods_ == p_load->nNeighborhoods_) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			std::string local = "nNeighborhoodMembers_[" + boost::lexical_cast<std::string>(i) + "]";
			std::string remote = "(p_load->nNeighborhoodMembers_)[" + boost::lexical_cast<std::string>(i) + "]";
			deviations.push_back(checkExpectation(withMessages, "GSwarm", nNeighborhoodMembers_[i], (p_load->nNeighborhoodMembers_)[i], local, remote, e , limit));

			// No local bests have been assigned yet in iteration 0
			if(getIteration() > 0) {
				deviations.push_back(checkExpectation(withMessages, "GSwarm", local_bests_[i], p_load->local_bests_[i], local, remote, e , limit));
			}
		}
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
		raiseException(
				"In GSwarm::saveCheckpoint():" << std::endl
				<< "global_best_ has not yet been initialized!"
		);
	}
#endif /* DEBUG */

	bool isDirty;
	double newValue = global_best_->getCurrentFitness(isDirty);

#ifdef DEBUG
	if(isDirty) {
		raiseException(
				"In GSwarm::saveCheckpoint():" << std::endl
				<< "global_best_ has dirty flag set!"
		);
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
		p->fpFixedValueInit(0.f);
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
bool GSwarm::nNeighborhoodMembersEqual(const std::vector<std::size_t> &one, const std::vector<std::size_t>& two) const {
	if(one.size() != two.size()) return false;
	for(std::size_t i=0; i<one.size(); i++) {
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
	return getFirstNIPosVec(neighborhood, nNeighborhoodMembers_);
}

/************************************************************************************************************/
/**
 * Helper function that returns the id of the first individual of a neighborhood, using a vector of neighborhood
 * sizes. "NI" stands for NeighborhoodIndividual. "neighborhood" is assumed to be a counter, starting at 0 and assuming
 * a maximum value of (nNeighborhoods_-1).
 *
 * @param neighborhood The id of the neighborhood for which the id of the first individual should be calculated
 * @return The position of the first individual of a neighborhood
 */
std::size_t GSwarm::getFirstNIPosVec(const std::size_t& neighborhood, const std::vector<std::size_t>& vec) const {
#ifdef DEBUG
	if(neighborhood >= nNeighborhoods_) {
		raiseException(
				"In GSwarm::getFirstNIPosVec():" << std::endl
				<< "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
				<< "The number of neighborhoods is " << nNeighborhoods_ << "," << std::endl
				<< "hence the maximum allowed value of the id is " << nNeighborhoods_-1 << "."
		);
	}
#endif

	if(neighborhood == 0) return 0;
	else {	// Sum up the number of members in each neighborhood
		std::size_t nPreviousMembers=0;
		for(std::size_t n=0; n<neighborhood; n++) {
			nPreviousMembers += vec[n];
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
		raiseException(
				"In GSwarm::getLastNIPos():" << std::endl
				<< "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
				<< "The number of neighborhoods is " << nNeighborhoods_ << " ." << std::endl
				<< "hence the maximum allowed value of the id is " << nNeighborhoods_-1 << "."
		);
	}
#endif

	return getFirstNIPos(neighborhood) + nNeighborhoodMembers_[neighborhood];
}

/************************************************************************************************************/
/**
 * Updates the personal best of an individual
 *
 * @param p_outer A pointer to the GParameterSet object to be updated
 * @param p_inner A pointer to the GParameterSet object serving as the personal best
 */
void GSwarm::updatePersonalBest(
		boost::shared_ptr<GParameterSet> p_outer
		, boost::shared_ptr<GParameterSet> p_inner
) {
#ifdef DEBUG
	if(p_outer->isDirty() || p_inner->isDirty()) {
		raiseException(
				"In GSwarm::updatePersonalBest():" << std::endl
				<< "p_inner and/or p_outer has dirty flag set:" << p_outer->isDirty() << " / " << p_inner->isDirty()
		);
	}
#endif /* DEBUG */

	p_outer->getSwarmPersonalityTraits()->registerPersonalBest(p_inner);
}

/************************************************************************************************************/
/**
 * Updates the personal best of an individual, if a better solution was found
 *
 * @param p_outer A pointer to the GParameterSet object to be updated
 * @param p_inner A pointer to the GParameterSet object serving as the personal best
 */
void GSwarm::updatePersonalBestIfBetter(
		boost::shared_ptr<GParameterSet> p_outer
		, boost::shared_ptr<GParameterSet> p_inner
) {
#ifdef DEBUG
	if(p_outer->isDirty() || p_inner->isDirty()) {
		raiseException(
				"In GSwarm::updatePersonalBestIfBetter():" << std::endl
				<< "p_inner and/or p_outer has dirty flag set:" << p_outer->isDirty() << " / " << p_inner->isDirty()
		);
	}
#endif /* DEBUG */

	if(GOptimizationAlgorithmT<GParameterSet>::isBetter(
			p_inner->getSwarmPersonalityTraits()->getPersonalBestQuality()
			, p_outer->fitness(0)
		)
	) {
		p_outer->getSwarmPersonalityTraits()->registerPersonalBest(p_inner);
	}
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
	swarmLogic();

	// Search for the locally and globally best individuals in all neighborhoods and
	// update the list of locally best solutions, if necessary
	double bestLocalFitness = findBests();

	// Makes sure that each neighborhood has the right size before the next cycle starts
	adjustNeighborhoods();

	// Let the audience know
	return bestLocalFitness;
}

/************************************************************************************************************/
/**
 * Modifies the particle positions, then triggers fitness calculation for all individuals. This function can
 * be overloaded by derived classes so the fitness calculation can be performed in parallel.
 */
void GSwarm::swarmLogic() {
	std::size_t offset = 0;
	GSwarm::iterator start = this->begin();
	boost::uint32_t iteration = getIteration();

	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
#ifdef DEBUG
		if(iteration > 0) {
			if(!local_bests_[neighborhood]) {
				raiseException(
						"In GSwarm::swarmLogic():" << std::endl
						<< "local_bests[" << neighborhood << "] is empty."
				);
			}

			if(neighborhood==0 && !global_best_) { // Only check for the first neighborhood
				raiseException(
						"In GSwarm::swarmLogic():" << std::endl
						<< "global_best_ is empty."
				);
			}
		}

		if(nNeighborhoodMembers_[neighborhood] != defaultNNeighborhoodMembers_) {
			raiseException(
					"In GSwarm::swarmLogic():" << std::endl
					<< "nNeighborhoodMembers_[" << neighborhood << "] should be " << defaultNNeighborhoodMembers_ << std::endl
					<< "but has value " << nNeighborhoodMembers_[neighborhood] << " instead"
			);
		}
#endif /* DEBUG */

		for(std::size_t member=0; member<nNeighborhoodMembers_[neighborhood]; member++) {
			GSwarm::iterator current = start + offset;

			// Note: global/local bests and velocities haven't been determined yet in iteration 0 and are not needed there
			updateSwarm (
				iteration
				, neighborhood
				, (*current)
				, iteration>0?(local_bests_[neighborhood]->clone<GParameterSet>()):boost::shared_ptr<GParameterSet>()
				, iteration>0?global_best_->clone<GParameterSet>():boost::shared_ptr<GParameterSet>()
#ifdef DEBUG
				, iteration>0?velocities_.at(offset):boost::shared_ptr<GParameterSet>()
#else
				, iteration>0?velocities_[offset]:boost::shared_ptr<GParameterSet>()
#endif /* DEBUG */
				, boost::make_tuple(
					getCPersonal()
					, getCLocal()
					, getCGlobal()
					, getCDelta()
				  )
			);

			offset++;
		}
	}
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
 * @param constants A boost::tuple holding the various constants needed for the position update
 */
void GSwarm::updateSwarm(
	  const boost::uint32_t& iteration
	  , const std::size_t& neighborhood
	  , boost::shared_ptr<GParameterSet> ind
	  , boost::shared_ptr<GParameterSet> local_best_tmp
	  , boost::shared_ptr<GParameterSet> global_best_tmp
	  , boost::shared_ptr<GParameterSet> velocity
	  , boost::tuple<double, double, double, double> constants
){
	if(iteration > 0 && !ind->getSwarmPersonalityTraits()->checkNoPositionUpdateAndReset()) {
		// Update the swarm positions:
		updatePositions(
		  neighborhood
		  , ind
		  , local_best_tmp
		  , global_best_tmp
		  , velocity
		  , constants
		);
	}

	// Update the fitness
	updateFitness(
		iteration
		, neighborhood
		, ind
	);
}

/************************************************************************************************************/
/**
 * Prepares individuals for the fitness calculation and performs that calculation. Also updates the
 * personal best.
 *
 * @param iteration The current iteration of the optimization
 * @param neighborhood The neighborhood the individual is in
 * @param ind The individual for which the fitness calculation should be performed
 */
void GSwarm::updateFitness(
	  const boost::uint32_t& iteration
	  , const std::size_t& neighborhood
	  , boost::shared_ptr<GParameterSet> ind
){
	// Let the personality know in which neighborhood it is
	ind->getSwarmPersonalityTraits()->setNeighborhood(neighborhood);

	// Trigger the fitness calculation (if necessary). Make sure
	// that fitness calculation is indeed allowed at this point.
	bool originalServerMode = ind->setServerMode(false);
	ind->fitness(0);
	ind->setServerMode(originalServerMode);

	// Update the personal best
	if(iteration == 0) {
		updatePersonalBest(ind, ind);
	} else {
		updatePersonalBestIfBetter(ind, ind);
	}
}

/************************************************************************************************************/
/**
 * Update the individual's positions. Note that we use a boost::tuple as an argument,
 * so that we do not have to pass too many parameters (problematic with boost::bind).
 *
 * @param neighborhood The neighborhood that has been assigned to the individual
 * @param ind The individual whose position should be updated
 * @param local_best_tmp The locally best dataset of the individual's neighborhood
 * @param global_best_tmp The globally best individual so far
 * @param velocity A velocity vector
 * @param constants A boost::tuple holding the various constants needed for the position update
 */
void GSwarm::updatePositions(
	  const std::size_t& neighborhood
	  , boost::shared_ptr<GParameterSet> ind
	  , boost::shared_ptr<GParameterSet> local_best_tmp
	  , boost::shared_ptr<GParameterSet> global_best_tmp
	  , boost::shared_ptr<GParameterSet> velocity
	  , boost::tuple<double, double, double, double> constants
) {
	// Extract the constants from the tuple
	double cPersonal = constants.get<0>();
	double cLocal = constants.get<1>();
	double cGlobal = constants.get<2>();
	double cDelta = constants.get<3>();

#ifdef DEBUG
	// Do some error checking
	if(!ind) {
		raiseException(
				"In GSwarm::updatePositions():" << std::endl
				<< "Found empty individual \"ind\""
		);
	}
#endif /* DEBUG */

	// Extract the personal best
	boost::shared_ptr<GParameterSet> personal_best_tmp =
			ind->getSwarmPersonalityTraits()->getPersonalBest()->clone<GParameterSet>();

	// Further error checks
#ifdef DEBUG
	if(!personal_best_tmp) {
		raiseException(
				"In GSwarm::updatePositions():" << std::endl
				<< "Found empty individual \"personal_best_tmp\""
		);
	}

	if(!local_best_tmp) {
		raiseException(
				"In GSwarm::updatePositions():" << std::endl
				<< "Found empty individual \"local_best_tmp\""
		);
	}

	if(!global_best_tmp) {
		raiseException(
				"In GSwarm::updatePositions():" << std::endl
				<< "Found empty individual \"global_best_tmp\""
		);
	}

	if(!velocity) {
		raiseException(
				"In GSwarm::updatePositions():" << std::endl
				<< "Found empty individual \"velocity\""
		);
	}

#endif /* DEBUG */

	// Subtract the current individual
	personal_best_tmp->fpSubtract(ind);
	local_best_tmp->fpSubtract(ind);
	global_best_tmp->fpSubtract(ind);

	switch(ur_) {
	case CLASSIC:
		// Multiply each floating point value with a random fp number in the range [0,1[
		personal_best_tmp->fpMultiplyByRandom();
		local_best_tmp->fpMultiplyByRandom();
		global_best_tmp->fpMultiplyByRandom();
		break;
	case LINEAR:
		// Multiply local_best_tmp and global_best_tmp with a single [0,1[ random number each
		personal_best_tmp->fpMultiplyBy(gr.uniform_01<double>());
		local_best_tmp->fpMultiplyBy(gr.uniform_01<double>());
		global_best_tmp->fpMultiplyBy(gr.uniform_01<double>());
		break;
	}

	// Multiply each floating point value with a fixed, configurable constant value
	personal_best_tmp->fpMultiplyBy(cPersonal);
	local_best_tmp->fpMultiplyBy(cLocal);
	global_best_tmp->fpMultiplyBy(cGlobal);

	// Multiply the last iterations velocity with a fixed, configurable constant value
	velocity->fpMultiplyBy(cDelta);

	// Add the personal, local and global tmps
	velocity->fpAdd(personal_best_tmp);
	velocity->fpAdd(local_best_tmp);
	velocity->fpAdd(global_best_tmp);

	// Add the velocity to the current individual
	ind->fpAdd(velocity);
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
		if(this->getMaxMode() == true){
			std::sort(this->begin() + firstCounter, this->begin() + lastCounter,
					boost::bind(&GIndividual::fitness, _1, 0) > boost::bind(&GIndividual::fitness, _2, 0));
		}
		else{
			std::sort(this->begin() + firstCounter, this->begin() + lastCounter,
					boost::bind(&GIndividual::fitness, _1, 0) < boost::bind(&GIndividual::fitness, _2, 0));
		}

		// Check whether the best individual of the neighborhood is better than
		// the best individual found so far in this neighborhood
		if(getIteration() == 0) {
			local_bests_[neighborhood] = (*(this->begin() + firstCounter))->clone<GParameterSet>();
		}
		else {
			if(isBetter((*(this->begin() + firstCounter))->fitness(0), local_bests_[neighborhood]->fitness(0))) {
				local_bests_[neighborhood]->load(*(this->begin() + firstCounter));
			}
		}

		// Find out which is the "best local best"
		if(isBetter((*(this->begin() + firstCounter))->fitness(0), bestCurrentLocalFitness)) {
			bestCurrentLocalFitness = (*(this->begin() + firstCounter))->fitness(0);
			bestCurrentLocalId = neighborhood;
		}
	}

	// Compare the best local individual with the global best and update
	// the global best, if necessary. Initialize it in the first generation.
	if(getIteration() == 0) {
		global_best_= local_bests_[bestCurrentLocalId]->clone<GParameterSet>();
	}
	else {
		if(isBetter(bestCurrentLocalFitness, global_best_->fitness(0))) {
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
 * will then remove the worst items only. Newly added items will start randomly initialized, as the optimization
 * procedure is already running and it makes sense to search new areas of the parameter space.
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
				raiseException(
						"In GSwarm::adjustNeighborhoods():" << std::endl
						<< "Found no entries in the neighborhood."
				);
			}
#endif /* DEBUG */

			// Insert items at the position n*defaultNNeighborhoodMembers_ (i.e. at the beginning of the range).
			// We use the first item of the range as a template, then randomly initialize the data item.
			this->insert(this->begin() + n*defaultNNeighborhoodMembers_, (*(this->begin() + n*defaultNNeighborhoodMembers_))->clone<GParameterSet>());
			(*(this->begin() + n*defaultNNeighborhoodMembers_))->randomInit();
			(*(this->begin() + n*defaultNNeighborhoodMembers_))->getSwarmPersonalityTraits()->setNoPositionUpdate();
		}

		// Update the number of entries in this neighborhood
		nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
	}
}

/************************************************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks. This function implements
 * the purely virtual function GOptimizationAlgorithmT<GParameterSet>::adjustPopulation() .
 */
void GSwarm::adjustPopulation() {
	const std::size_t currentSize = this->size();
	const std::size_t defaultPopSize = getDefaultPopulationSize();
	const std::size_t nNeighborhoods = getNNeighborhoods();

	if(currentSize==0) {
		raiseException(
				"In GSwarm::adjustPopulation() :" << std::endl
				<< "No individuals found in the population." << std::endl
				<< "You need to add at least one individual before" << std::endl
				<< "the call to optimize()."
		);
	}
	else if(currentSize==1) {
		// Fill up with random items to the number of neighborhoods
		for(std::size_t i=1; i<nNeighborhoods_; i++) {
			this->push_back(this->front()->clone<GParameterSet>());
			this->back()->randomInit();
		}

		// Fill in remaining items in each neighborhood. This will
		// also take care of the above case, where only one individual
		// has been added.
		fillUpNeighborhood1();
	}
	else if(currentSize==nNeighborhoods) {
		// Fill in remaining items in each neighborhood. This will
		// also take care of the above case, where only one individual
		// has been added.
		fillUpNeighborhood1();
	}
	else if(currentSize == defaultPopSize) {
		// Update the number of individuals in each neighborhood
		for(std::size_t n=0; n<nNeighborhoods_; n++) {
			nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
		}
	} else {
		if(currentSize < nNeighborhoods_) {
			// First fill up the neighborhoods, if required
			for(std::size_t m=0; m < (nNeighborhoods_-currentSize); m++) {
				this->push_back(this->front()->clone<GParameterSet>());
				this->back()->randomInit();
			}

			// Now follow the procedure used for the "nNeighborhoods_" case
			fillUpNeighborhood1();
		}
		else if(currentSize > nNeighborhoods_ && currentSize < defaultPopSize) {
			// TODO: For now we simply resize the population to the number of neighborhoods,
			// then fill up again. This means that we loose some predefined values, which
			// is ugly and needs to be changed in later versions.
			this->resize(nNeighborhoods_);
			fillUpNeighborhood1();
		}
		else { // currentSize > defaultPopsize
			// Update the number of individuals in each neighborhood
			for(std::size_t n=0; n<nNeighborhoods_-1; n++) {
				nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
			}

			// Adjust the nNeighborhoodMembers_ array. The surplus items will
			// be assumed to belong to the last neighborhood, all other neighborhoods
			// have the default size.
			nNeighborhoodMembers_[nNeighborhoods_-1] = defaultNNeighborhoodMembers_ + (currentSize - defaultPopSize);
		}
	}

#ifdef DEBUG
	// As the above switch statement is quite complicated, cross check that we now
	// indeed have at least the required number of individuals
	if(this->size() < defaultPopSize) {
		raiseException(
				"In GSwarm::adjustPopulation() :" << std::endl
				<< "Expected at least a population size of " << defaultPopSize << std::endl
				<< "but found a size of " << this->size() << ", which is too small."
		);
	}
#endif /* DEBUG */

	// We do not initialize the local and global bests here, as this requires the value of
	// all individuals to be calculated.
}

/************************************************************************************************************/
/**
 * Small helper function that helps to fill up a neighborhood, if there is just one entry in it.
 */
void GSwarm::fillUpNeighborhood1() {
	// Do some error checking
	if(this->size() != nNeighborhoods_) {
		raiseException(
				"In GSwarm::fillUpNeighborhood1():" << std::endl
				<< "Invalid size: " << this->size() << " Expected " << nNeighborhoods_
		);
	}

	if(defaultNNeighborhoodMembers_==1) return; // nothing to do

	// Starting with the last item, loop over all neighborhoods
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		std::size_t n = nNeighborhoods_ - 1 - i; // Calculate the correct neighborhood

		// Insert the required number of clones after the existing individual
		for(std::size_t m=1; m<defaultNNeighborhoodMembers_; m++) { // m stands for "missing"
			// Add a clone of the first individual in the neighborhood to the next position
			this->insert(this->begin()+n, (*(this->begin()+n))->clone<GParameterSet>());
			// Make sure it has a unique value, if requested
			if(randomFillUp_) {
#ifdef DEBUG
				if(!(*(this->begin()+n+1))) {
					raiseException(
							"In GSwarm::fillUpNeighborhood1():" << std::endl
							<< "Found empty position " << n
					);
				}
#endif /* DEBUG */

				(*(this->begin()+n+1))->randomInit();
			}
		}

		// Update the number of individuals in each neighborhood
		nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
	}
}

/************************************************************************************************************/
/**
 * Checks whether each neighborhood has at least the default size
 *
 * @return A boolean which indicates whether all neighborhoods have at least the default size
 */
bool GSwarm::neighborhoodsHaveNominalValues() const {
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		if(nNeighborhoodMembers_[n] < defaultNNeighborhoodMembers_) return false;
	}
	return true;
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for personal distances.
 */
void GSwarm::setCPersonal(const double& c_personal) {
	c_personal_ = c_personal;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances
 */
double GSwarm::getCPersonal() const {
	return c_personal_;
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for local distances.
 */
void GSwarm::setCLocal(const double& c_local) {
	c_local_ = c_local;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances
 */
double GSwarm::getCLocal() const {
	return c_local_;
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for global distances
 */
void GSwarm::setCGlobal(const double& c_global) {
	c_global_ = c_global;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances
 */
double GSwarm::getCGlobal() const {
	return c_global_;
}

/************************************************************************************************************/
/**
 * Allows to set a static multiplier for deltas
 */
void GSwarm::setCDelta(const double& c_delta) {
	c_delta_ = c_delta;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the static multiplier for deltas
 */
double GSwarm::getCDelta() const {
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

/************************************************************************************************************/
/**
 * Allows to specify the update rule to be used by the swarm.
 *
 * @param ur The desired update rule
 */
void GSwarm::setUpdateRule(const updateRule& ur) {
	ur_ = ur;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the update rule currently used by the swarm.
 *
 * @return The current update rule
 */
updateRule GSwarm::getUpdateRule() const {
	return ur_;
}

/************************************************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have equal value
 */
void GSwarm::setNeighborhoodsEqualFillUp() {
	randomFillUp_=false;
}

/************************************************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have a random value
 */
void GSwarm::setNeighborhoodsRandomFillUp() {
	randomFillUp_=true;
}

/************************************************************************************************************/
/**
 * Allows to check whether neighborhoods are filled up with random individuals
 *
 * @return A boolean indicating whether neighborhoods are filled up with random values
 */
bool GSwarm::neighborhoodsFilledUpRandomly() const {
	return randomFillUp_;
}

/************************************************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GSwarm::getNProcessableItems() const {
	return this->size(); // All items in the population are updated in each iteration and need to be processed
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

/**********************************************************************************/
/**
 * The default constructor
 */
GSwarm::GSwarmOptimizationMonitor::GSwarmOptimizationMonitor()
	: xDim_(DEFAULTXDIMOM)
	, yDim_(DEFAULTYDIMOM)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSwarmOptimizationMonitor object
 */
GSwarm::GSwarmOptimizationMonitor::GSwarmOptimizationMonitor(const GSwarm::GSwarmOptimizationMonitor& cp)
	: GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT(cp)
	, xDim_(cp.xDim_)
	, yDim_(cp.yDim_)
  { /* nothing */ }

/**********************************************************************************/
/**
 * The destructor
 */
GSwarm::GSwarmOptimizationMonitor::~GSwarmOptimizationMonitor()
{ /* nothing */ }

/**********************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GSwarmOptimizationMonitor object
 * @return A constant reference to this object
 */
const GSwarm::GSwarmOptimizationMonitor& GSwarm::GSwarmOptimizationMonitor::operator=(const GSwarm::GSwarmOptimizationMonitor& cp){
	GSwarm::GSwarmOptimizationMonitor::load_(&cp);
	return *this;
}

/**********************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GSwarmOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarm::GSwarmOptimizationMonitor::operator==(const GSwarm::GSwarmOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarm::GSwarmOptimizationMonitor::operator==","cp", CE_SILENT);
}

/**********************************************************************************/
/**
 * Checks for inequality with another GSwarmOptimizationMonitor object
 *
 * @param  cp A constant reference to another GSwarmOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarm::GSwarmOptimizationMonitor::operator!=(const GSwarm::GSwarmOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarm::GSwarmOptimizationMonitor::operator!=","cp", CE_SILENT);
}

/**********************************************************************************/
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
boost::optional<std::string> GSwarm::GSwarmOptimizationMonitor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSwarm::GSwarmOptimizationMonitor *p_load = GObject::conversion_cast<GSwarm::GSwarmOptimizationMonitor >(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::checkRelationshipWith(cp, e, limit, "GSwarm::GSwarmOptimizationMonitor", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GSwarm::GSwarmOptimizationMonitor", xDim_, p_load->xDim_, "xDim_", "p_load->xDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarm::GSwarmOptimizationMonitor", yDim_, p_load->yDim_, "yDim_", "p_load->yDim_", e , limit));

	return evaluateDiscrepancies("GSwarm::GSwarmOptimizationMonitor", caller, deviations, e);
}

/**********************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param xDim The desired dimension of the canvas in x-direction
 * @param yDim The desired dimension of the canvas in y-direction
 */
void GSwarm::GSwarmOptimizationMonitor::setDims(const boost::uint16_t& xDim, const boost::uint16_t& yDim) {
	xDim_ = xDim;
	yDim_ = yDim;
}

/**********************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
boost::uint16_t GSwarm::GSwarmOptimizationMonitor::getXDim() const {
	return xDim_;
}

/**********************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
boost::uint16_t GSwarm::GSwarmOptimizationMonitor::getYDim() const {
	return yDim_;
}

/**********************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 * @return A string containing information to written to the output file (if any)
 */
std::string GSwarm::GSwarmOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	// This should always be the first statement in a custom optimization monitor
	std::cout << GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::firstInformation(goa);

	// Perform the conversion to the target algorithm
#ifdef DEBUG
	if(goa->getOptimizationAlgorithm() != SWARM) {
		raiseException(
				"In GSwarm::GSwarmOptimizationMonitor::firstInformation():" << std::endl
				<< "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm()
		);
	}
#endif /* DEBUG */
	GSwarm * const swarm = static_cast<GSwarm * const>(goa);

	// Output the header to the summary stream
	return swarmFirstInformation(swarm);
}

/**********************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 * @return A string containing information to written to the output file (if any)
 */
std::string GSwarm::GSwarmOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	// Let the audience know what the parent has to say
	std::cout << GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::cycleInformation(goa);

	// Perform the conversion to the target algorithm
#ifdef DEBUG
	if(goa->getOptimizationAlgorithm() != SWARM) {
		raiseException(
				"In GSwarm::GSwarmOptimizationMonitor::cycleInformation():" << std::endl
				<< "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm()
		);
	}
#endif /* DEBUG */
	GSwarm * const swarm = static_cast<GSwarm * const>(goa);

	return swarmCycleInformation(swarm);
}

/**********************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 * @return A string containing information to written to the output file (if any)
 */
std::string GSwarm::GSwarmOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {

	// Perform the conversion to the target algorithm
#ifdef DEBUG
	if(goa->getOptimizationAlgorithm() != SWARM) {
		raiseException(
				"In GSwarm::GSwarmOptimizationMonitor::lastInformation():" << std::endl
				<< "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm()
		);
	}
#endif /* DEBUG */
	GSwarm * const swarm = static_cast<GSwarm * const>(goa);

	// Do the actual information gathering
	std::ostringstream result;
	result << swarmLastInformation(swarm);

	// This should always be the last statement in a custom optimization monitor
	std::cout << GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::lastInformation(goa);

	return result.str();
}


/**********************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param swarm The object for which information should be collected
 */
std::string GSwarm::GSwarmOptimizationMonitor::swarmFirstInformation(GSwarm * const swarm) {
	std::ostringstream result;

	// Output the header to the summary stream
	result << "{" << std::endl
		   << "  gROOT->Reset();" << std::endl
		   << "  gStyle->SetOptTitle(0);" << std::endl
		   << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0," << xDim_ << "," << yDim_ << ");" << std::endl
		   << std::endl
		   << "  std::vector<long> iteration;" << std::endl
		   << "  std::vector<double> evaluation;" << std::endl
		   << std::endl;

	return result.str();
}

/**********************************************************************************/
/**
 * A function that is called during each optimization cycle
 *
 * @param swarm The object for which information should be collected
 */
std::string GSwarm::GSwarmOptimizationMonitor::swarmCycleInformation(GSwarm * const swarm) {
	std::ostringstream result;
	bool isDirty = false;
	double currentEvaluation = 0.;

	// Retrieve the current iteration
	boost::uint32_t iteration = swarm->getIteration();

	result << "  iteration.push_back(" << iteration << ");" << std::endl;

	// Get access to the best inidividual
	boost::shared_ptr<GParameterSet> gsi_ptr = swarm->getBestIndividual<GParameterSet>();

	// Retrieve the fitness of this individual
	currentEvaluation = gsi_ptr->getCurrentFitness(isDirty);

	// Write information to the output stream
	result << "  evaluation.push_back(" <<  currentEvaluation << ");" << (isDirty?" // dirty flag is set":"") << std::endl
	       << std::endl; // Improves readability when following the output with "tail -f"

	return result.str();
}

/**********************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param swarm The object for which information should be collected
 */
std::string GSwarm::GSwarmOptimizationMonitor::swarmLastInformation(GSwarm * const swarm) {
	std::ostringstream result;

	// Output final print logic to the stream
	result << "  // Transfer the vectors into arrays" << std::endl
		   << "  double iteration_arr[iteration.size()];" << std::endl
		   << "  double evaluation_arr[evaluation.size()];" << std::endl
		   << std::endl
		   << "  for(std::size_t i=0; i<iteration.size(); i++) {" << std::endl
		   << "     iteration_arr[i] = (double)iteration[i];" << std::endl
		   << "     evaluation_arr[i] = evaluation[i];" << std::endl
		   << "  }" << std::endl
		   << std::endl
		   << "  // Create a TGraph object" << std::endl
		   << "  TGraph *evGraph = new TGraph(evaluation.size(), iteration_arr, evaluation_arr);" << std::endl
		   << std::endl
		   << "  // Set the axis titles" << std::endl
		   << "  evGraph->GetXaxis()->SetTitle(\"Iteration\");" << std::endl
		   << "  evGraph->GetYaxis()->SetTitleOffset(1.1);" << std::endl
		   << "  evGraph->GetYaxis()->SetTitle(\"Fitness\");" << std::endl
		   << std::endl
		   << "  // Do the actual drawing" << std::endl
		   << "  evGraph->Draw(\"APL\");" << std::endl
		   << "}" << std::endl;

	return result.str();
}

/**********************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GSwarmOptimizationMonitor object, camouflaged as a GObject
 */
void GSwarm::GSwarmOptimizationMonitor::load_(const GObject* cp) {
	const GSwarm::GSwarmOptimizationMonitor *p_load = conversion_cast<GSwarm::GSwarmOptimizationMonitor>(cp);

	// Load the parent classes' data ...
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::load_(cp);

	// ... and then our local data
	xDim_ = p_load->xDim_;
	yDim_ = p_load->yDim_;
}

/**********************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GSwarm::GSwarmOptimizationMonitor::clone_() const {
	return new GSwarm::GSwarmOptimizationMonitor(*this);
}

#ifdef GENEVATESTING
/**********************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GSwarm::GSwarmOptimizationMonitor::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

	return result;
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarm::GSwarmOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarm::GSwarmOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();
}

/**********************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/**
 * As Gem::Geneva::GSwarm has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GSwarm> TFactory_GUnitTests<Gem::Geneva::GSwarm>() {
	using namespace Gem::Tests;
	const std::size_t NNEIGHBORHOODS=2;
	const std::size_t NNEIGHBORHOODMEMBERS=3;
	boost::shared_ptr<Gem::Geneva::GSwarm> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GSwarm>(new Gem::Geneva::GSwarm(NNEIGHBORHOODS, NNEIGHBORHOODMEMBERS)));
	for(std::size_t i=0; i<NNEIGHBORHOODS*NNEIGHBORHOODMEMBERS; i++) {
		p->push_back(boost::shared_ptr<GTestIndividual1>(new GTestIndividual1()));
	}
	return p;
}

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */
