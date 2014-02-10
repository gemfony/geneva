/**
 * @file GBaseSwarm.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */
#include "geneva/GBaseSwarm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBaseSwarm::GSwarmOptimizationMonitor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GBaseSwarm::nickname = "swarm";

/******************************************************************************/
/**
 * The default constructor.
 */
GBaseSwarm::GBaseSwarm()
	: GOptimizationAlgorithmT<GParameterSet>()
	, nNeighborhoods_(DEFAULTNNEIGHBORHOODS?DEFAULTNNEIGHBORHOODS:1)
	, defaultNNeighborhoodMembers_((DEFAULTNNEIGHBORHOODMEMBERS<=1)?2:DEFAULTNNEIGHBORHOODMEMBERS)
	, nNeighborhoodMembers_(nNeighborhoods_)
	, neighborhood_bests_(nNeighborhoods_)
	, c_personal_(DEFAULTCPERSONAL)
	, c_neighborhood_(DEFAULTCNEIGHBORHOOD)
	, c_global_(DEFAULTCGLOBAL)
	, c_velocity_(DEFAULTCVELOCITY)
	, updateRule_(DEFAULTUPDATERULE)
	, randomFillUp_(true)
	, dblLowerParameterBoundaries_()
	, dblUpperParameterBoundaries_()
	, dblVelVecMax_()
	, velocityRangePercentage_(DEFAULTVELOCITYRANGEPERCENTAGE)
{
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Initialize the number of neighborhood members with 0. adjustPopulation() will later take care to fill the
	// population with individuals as needed and set the array to the correct values.
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		nNeighborhoodMembers_[i] = 0;
	}

	// Register the default optimization monitor. This can be overridden by the user.
	this->registerOptimizationMonitor(
			boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
					new GSwarmOptimizationMonitor()
			)
	);
}

/******************************************************************************/
/**
 * This constructor sets the number of neighborhoods and the number of individuals in them. Note that there
 * is no public default constructor, as it is only needed for de-serialization purposes.
 *
 * @param nNeighborhoods The desired number of neighborhoods (hardwired to >= 1)
 * @oaram nNeighborhoodMembers The default number of individuals in each neighborhood (hardwired to >= 2)
 */
GBaseSwarm::GBaseSwarm(const std::size_t& nNeighborhoods, const std::size_t& nNeighborhoodMembers)
	: GOptimizationAlgorithmT<GParameterSet>()
	, nNeighborhoods_(nNeighborhoods?nNeighborhoods:1)
	, defaultNNeighborhoodMembers_((nNeighborhoodMembers<=1)?2:nNeighborhoodMembers)
	, nNeighborhoodMembers_(nNeighborhoods_)
	, neighborhood_bests_(nNeighborhoods_)
	, c_personal_(DEFAULTCPERSONAL)
	, c_neighborhood_(DEFAULTCNEIGHBORHOOD)
	, c_global_(DEFAULTCGLOBAL)
	, c_velocity_(DEFAULTCVELOCITY)
	, updateRule_(DEFAULTUPDATERULE)
	, randomFillUp_(true)
	, dblLowerParameterBoundaries_()
	, dblUpperParameterBoundaries_()
	, dblVelVecMax_()
	, velocityRangePercentage_(DEFAULTVELOCITYRANGEPERCENTAGE)
{
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Initialize the number of neighborhood members with 0. adjustPopulation() will later take care to fill the
	// population with individuals as needed and set the array to the correct values.
	for(std::size_t i=0; i<nNeighborhoods_; i++) {
		nNeighborhoodMembers_[i] = 0;
	}

	// Register the default optimization monitor. This can be overridden by the user.
	this->registerOptimizationMonitor(
			boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
					new GSwarmOptimizationMonitor()
			)
	);
}

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GBaseSwarm object
 */
GBaseSwarm::GBaseSwarm(const GBaseSwarm& cp)
	: GOptimizationAlgorithmT<GParameterSet>(cp)
	, nNeighborhoods_(cp.nNeighborhoods_)
	, defaultNNeighborhoodMembers_(cp.defaultNNeighborhoodMembers_)
	, nNeighborhoodMembers_(cp.nNeighborhoodMembers_)
	, global_best_((cp.afterFirstIteration())?(cp.global_best_)->clone<GParameterSet>():boost::shared_ptr<GParameterSet>())
	, neighborhood_bests_(nNeighborhoods_) // We copy the smart pointers over later
	, c_personal_(cp.c_personal_)
	, c_neighborhood_(cp.c_neighborhood_)
	, c_global_(cp.c_global_)
	, c_velocity_(cp.c_velocity_)
	, updateRule_(cp.updateRule_)
	, randomFillUp_(cp.randomFillUp_)
	, dblLowerParameterBoundaries_(cp.dblLowerParameterBoundaries_)
	, dblUpperParameterBoundaries_(cp.dblUpperParameterBoundaries_)
	, dblVelVecMax_(cp.dblVelVecMax_)
	, velocityRangePercentage_(cp.velocityRangePercentage_)
{
	// Note that this setting might differ from nCPIndividuals, as it is not guaranteed
	// that cp has, at the time of copying, all individuals present in each neighborhood.
	// Differences might e.g. occur if not all individuals return from their remote
	// evaluation. adjustPopulation will take care to resize the population appropriately
	// inside of the "optimize()" call.
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);

	// Clone cp's best individuals in each neighborhood
	if(cp.afterFirstIteration()) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			neighborhood_bests_[i] = cp.neighborhood_bests_[i]->clone<GParameterSet>();
		}
	}

	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * The standard destructor. Most work is done in the parent class.
 */
GBaseSwarm::~GBaseSwarm()
{ /* nothing */ }

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GBaseSwarm::getOptimizationAlgorithm() const {
	return "PERSONALITY_SWARM";
}


/******************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp Another GBaseSwarm object
 * @return A constant reference to this object
 */
const GBaseSwarm& GBaseSwarm::operator=(const GBaseSwarm& cp) {
	GBaseSwarm::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Loads the data of another GBaseSwarm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GBaseSwarm object, camouflaged as a GObject
 */
void GBaseSwarm::load_(const GObject *cp)
{
	const GBaseSwarm *p_load = this->gobject_conversion<GBaseSwarm>(cp);

	// First load the parent class'es data.
	// This will also take care of copying all individuals.
	GOptimizationAlgorithmT<GParameterSet>::load_(cp);

	// ... and then our own data
	defaultNNeighborhoodMembers_ = p_load->defaultNNeighborhoodMembers_;
	c_personal_ = p_load->c_personal_;
	c_neighborhood_ = p_load->c_neighborhood_;
	c_global_ = p_load->c_global_;
	c_velocity_ = p_load->c_velocity_;
	updateRule_ = p_load->updateRule_;
	randomFillUp_ = p_load->randomFillUp_;

	dblLowerParameterBoundaries_ = p_load->dblLowerParameterBoundaries_;
	dblUpperParameterBoundaries_ = p_load->dblUpperParameterBoundaries_;
	dblVelVecMax_ = p_load->dblVelVecMax_;

	velocityRangePercentage_ = p_load->velocityRangePercentage_;

	// We start from scratch if the number of neighborhoods or the alleged number of members in them differ
	if(nNeighborhoods_!=p_load->nNeighborhoods_ || !nNeighborhoodMembersEqual(nNeighborhoodMembers_, p_load->nNeighborhoodMembers_)) {
		nNeighborhoods_ = p_load->nNeighborhoods_;

		nNeighborhoodMembers_.clear();
		neighborhood_bests_.clear();

		nNeighborhoodMembers_.resize(nNeighborhoods_);
		neighborhood_bests_.resize(nNeighborhoods_);

		// Copy the neighborhood bests and number of neighborhood members over
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			nNeighborhoodMembers_[i] = p_load->nNeighborhoodMembers_[i];
			// The following only makes sense if this is not the first iteration. Note that
			// getIteration will return the "foreign" GBaseSwarm object's iteration, as it has
			// already been copied.
			if(afterFirstIteration()) {
				neighborhood_bests_[i] = p_load->neighborhood_bests_[i]->clone<GParameterSet>();
			}
			// we do not need to reset the neighborhood_bests_, as that array has just been created
		}
	}
	else { // We now assume that we can just load neighborhood bests in each position.
		// Copying only makes sense if the foreign GBaseSwarm object's iteration is larger
		// than the iteration offset. Note that getIteration() will return the foreign iteration,
		// has that value has already been copied.
		if(afterFirstIteration()) {
			for(std::size_t i=0; i<nNeighborhoods_; i++) {
				// We might be in a situation where the boost::shared_ptr which usually
				// holds the neighborhood bests has not yet been initialized
				if(neighborhood_bests_[i]) {
					neighborhood_bests_[i]->GObject::load(p_load->neighborhood_bests_[i]);
				} else {
					neighborhood_bests_[i] = p_load->neighborhood_bests_[i]->clone<GParameterSet>();
				}
			}
		} else {
			for(std::size_t i=0; i<nNeighborhoods_; i++) {
				neighborhood_bests_[i].reset();
			}
		}
	}

	// Copy the global best over
	if(p_load->afterFirstIteration()) { // cp has a global best, we don't
		if(global_best_) { // If we already have a global best, just load the other objects global best
			global_best_->GObject::load(p_load->global_best_);
		} else {
			global_best_ = p_load->GObject::clone<GParameterSet>();
		}
	}
	else if(p_load->inFirstIteration()) { // cp does not have a global best
		global_best_.reset(); // empty the smart pointer
	}
	// else {} // We do not need to do anything if both iterations are 0 as there is no global best at all
}

/******************************************************************************/
/**
 * Checks for equality with another GBaseSwarm object
 *
 * @param  cp A constant reference to another GBaseSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseSwarm::operator==(const GBaseSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseSwarm::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBaseSwarm object
 *
 * @param  cp A constant reference to another GBaseSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseSwarm::operator!=(const GBaseSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseSwarm::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBaseSwarm::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBaseSwarm *p_load = GObject::gobject_conversion<GBaseSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GOptimizationAlgorithmT<GParameterSet>", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", nNeighborhoods_, p_load->nNeighborhoods_, "nNeighborhoods_", "p_load->nNeighborhoods_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", defaultNNeighborhoodMembers_, p_load->defaultNNeighborhoodMembers_, "defaultNNeighborhoodMembers_", "p_load->defaultNNeighborhoodMembers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", global_best_, p_load->global_best_, "global_best_", "p_load->global_best_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", c_personal_, p_load->c_personal_, "c_personal_", "p_load->c_personal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", c_neighborhood_, p_load->c_neighborhood_, "c_neighborhood_", "p_load->c_neighborhood_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", c_global_, p_load->c_global_, "c_global_", "p_load->c_global_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", c_velocity_, p_load->c_velocity_, "c_velocity_", "p_load->c_velocity_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", updateRule_, p_load->updateRule_, "updateRule_", "p_load->updateRule_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", randomFillUp_, p_load->randomFillUp_, "randomFillUp_", "p_load->randomFillUp_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", dblLowerParameterBoundaries_, p_load->dblLowerParameterBoundaries_, "dblLowerParameterBoundaries_", "p_load->dblLowerParameterBoundaries_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", dblUpperParameterBoundaries_, p_load->dblUpperParameterBoundaries_, "dblUpperParameterBoundaries_", "p_load->dblUpperParameterBoundaries_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", dblVelVecMax_, p_load->dblVelVecMax_, "dblVelVecMax_", "p_load->dblVelVecMax_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", velocityRangePercentage_, p_load->velocityRangePercentage_, "velocityRangePercentage_", "p_load->velocityRangePercentage_", e , limit));

	// The next checks only makes sense if the number of neighborhoods are equal
	if(nNeighborhoods_ == p_load->nNeighborhoods_) {
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			std::string nbh = "nNeighborhoodMembers_[" + boost::lexical_cast<std::string>(i) + "]";
			std::string remote = "(p_load->nNeighborhoodMembers_)[" + boost::lexical_cast<std::string>(i) + "]";
			deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", nNeighborhoodMembers_[i], (p_load->nNeighborhoodMembers_)[i], nbh, remote, e , limit));

			// No neighborhood bests have been assigned yet in iteration 0
			if(afterFirstIteration()) {
				deviations.push_back(checkExpectation(withMessages, "GBaseSwarm", neighborhood_bests_[i], p_load->neighborhood_bests_[i], nbh, remote, e , limit));
			}
		}
	}

	return evaluateDiscrepancies("GBaseSwarm", caller, deviations, e);
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBaseSwarm::name() const {
   return std::string("GBaseSwarm");
}

/******************************************************************************/
/**
 * Sets the number of neighborhoods and the default number of members in them. All work is done inside of
 * the adjustPopulation function, inside of the GOptimizationAlgorithmT<>::optimize() function.
 *
 * @param nNeighborhoods The number of neighborhoods
 * @param defaultNNeighborhoodMembers The default number of individuals in each neighborhood
 */
void GBaseSwarm::setSwarmSizes(std::size_t nNeighborhoods, std::size_t defaultNNeighborhoodMembers) {
	// Enforce useful settings
	if(nNeighborhoods == 0) {
		std::cerr << "In GBaseSwarm::setSwarmSizes(): Warning!" << std::endl
				  << "Requested number of neighborhoods is 0. Setting to 1." << std::endl;
	}

	if(defaultNNeighborhoodMembers <= 1) {
		std::cerr << "In GBaseSwarm::setSwarmSizes(): Warning!" << std::endl
				  << "Requested number of members in each neighborhood is too small. Setting to 2." << std::endl;
	}

	nNeighborhoods_ = nNeighborhoods?nNeighborhoods:1;
	defaultNNeighborhoodMembers_ = defaultNNeighborhoodMembers>1?defaultNNeighborhoodMembers:2;

	// Update our parent class'es values
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_*defaultNNeighborhoodMembers_);
}

/******************************************************************************/
/**
 * Saves the state of the class to disc. The function adds the current generation
 * and the fitness to the base name. The entire object is saved. The function will
 * throw if no global best has been established yet.
 */
void GBaseSwarm::saveCheckpoint() const {
#ifdef DEBUG
	// Check that the global best has been initialized
	if(!global_best_) {
	   glogger
	   << "In GBaseSwarm::saveCheckpoint():" << std::endl
      << "global_best_ has not yet been initialized!" << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	double newValue = global_best_->transformedFitness();

	// Determine a suitable name for the output file
	std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(getIteration()) + "_"
		+ boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

	this->toFile(outputFile, getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Loads the state of the object from disc.
 *
 * @param cpFile The name of the file the checkpoint should be loaded from
 */
void GBaseSwarm::loadCheckpoint(const std::string& cpFile) {
	this->fromFile(cpFile, getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Helper function that checks the content of two nNeighborhoodMembers_ arrays.
 *
 * @param one The first array used for the check
 * @param two The second array used for the check
 * @return A boolean indicating whether both arrays are equal
 */
bool GBaseSwarm::nNeighborhoodMembersEqual(
		const std::vector<std::size_t> &one
		, const std::vector<std::size_t>& two
) const {
	if(one.size() != two.size()) {
		return false;
	}

	for(std::size_t i=0; i<one.size(); i++) {
		if(one[i] != two[i]) {
			return false;
		}
	}

	return true; // Make the compiler happy
}

/******************************************************************************/
/**
 * Helper function that returns the id of the first individual of a neighborhood. "NI" stands
 * for NeighborhoodIndividual. "neighborhood" is assumed to be a counter, starting at 0, with
 * a maximum value of (nNeighborhoods_-1).
 *
 * @param neighborhood The id of the neighborhood for which the id of the first individual should be calculated
 * @return The position of the first individual of a neighborhood
 */
std::size_t GBaseSwarm::getFirstNIPos(const std::size_t& neighborhood) const {
	return getFirstNIPosVec(neighborhood, nNeighborhoodMembers_);
}

/******************************************************************************/
/**
 * Helper function that returns the id of the first individual of a neighborhood, using a vector of neighborhood
 * sizes. "NI" stands for NeighborhoodIndividual. "neighborhood" is assumed to be a counter, starting at 0 and assuming
 * a maximum value of (nNeighborhoods_-1).
 *
 * @param neighborhood The id of the neighborhood for which the id of the first individual should be calculated
 * @return The position of the first individual of a neighborhood
 */
std::size_t GBaseSwarm::getFirstNIPosVec(
		const std::size_t& neighborhood
		, const std::vector<std::size_t>& vec
) const {
#ifdef DEBUG
	if(neighborhood >= nNeighborhoods_) {
	   glogger
	   << "In GBaseSwarm::getFirstNIPosVec():" << std::endl
      << "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
      << "The number of neighborhoods is " << nNeighborhoods_ << "," << std::endl
      << "hence the maximum allowed value of the id is " << nNeighborhoods_-1 << "." << std::endl
      << GEXCEPTION;
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

/******************************************************************************/
/**
 * Helper function that helps to determine the end of a neighborhood. "NI" stands
 * for NeighborhoodIndividual. "neighborhood" is assumed to be a counter, starting at 0, with a maximum
 * value of (nNeighborhoods_-1). The position returned is that right after the last individual, as is common
 * in C++ .
 *
 * @param neighborhood The id of the neighborhood for which the id of the last individual should be calculated
 * @return The position of the individual right after the last of a neighborhood
 */
std::size_t GBaseSwarm::getLastNIPos(const std::size_t& neighborhood) const {
#ifdef DEBUG
	if(neighborhood >= nNeighborhoods_) {
	   glogger
	   << "In GBaseSwarm::getLastNIPos():" << std::endl
      << "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
      << "The number of neighborhoods is " << nNeighborhoods_ << " ." << std::endl
      << "hence the maximum allowed value of the id is " << nNeighborhoods_-1 << "." << std::endl
      << GEXCEPTION;
	}
#endif

	return getFirstNIPos(neighborhood) + nNeighborhoodMembers_[neighborhood];
}

/******************************************************************************/
/**
 * Updates the personal best of an individual
 *
 * @param p A pointer to the GParameterSet object to be updated
 */
void GBaseSwarm::updatePersonalBest(
		boost::shared_ptr<GParameterSet> p
) {
#ifdef DEBUG
   if(!p) {
      glogger
      << "In GBaseSwarm::updatePersonalBest():" << std::endl
      << "Got empty p" << std::endl
      << GEXCEPTION;
   }

	if(p->isDirty()) {
	   glogger
	   << "In GBaseSwarm::updatePersonalBest():" << std::endl
      << "p has its dirty flag set: " << p->isDirty() << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	p->getPersonalityTraits<GSwarmPersonalityTraits>()->registerPersonalBest(p);
}

/******************************************************************************/
/**
 * Updates the personal best of an individual, if a better solution was found
 *
 * @param p A pointer to the GParameterSet object to be updated
 */
void GBaseSwarm::updatePersonalBestIfBetter(
   boost::shared_ptr<GParameterSet> p
) {
#ifdef DEBUG
   if(!p) {
      glogger
      << "In GBaseSwarm::updatePersonalBestIfBetter():" << std::endl
      << "Got empty p" << std::endl
      << GEXCEPTION;
   }

	if(!p->isClean()) {
	   glogger
	   << "In GBaseSwarm::updatePersonalBestIfBetter(): Error!" << std::endl
      << "dirty flag of individual is set." << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	if(this->isBetter
	      (
            boost::get<G_TRANSFORMED_FITNESS>(p->getPersonalityTraits<GSwarmPersonalityTraits>()->getPersonalBestQuality())
            , p->transformedFitness()
	      )
	) {
		p->getPersonalityTraits<GSwarmPersonalityTraits>()->registerPersonalBest(p);
	}
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GBaseSwarm::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;
	std::string comment1;
	std::string comment2;

	// Call our parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	comment1 = "";
	comment2 = "";
	comment1 = "The desired number of neighborhoods in the population;";
	if(showOrigin) comment1 += "[GBaseSwarm]";
	comment2 = "The desired number of members in each neighborhood;";
	if(showOrigin) comment2 += "[GBaseSwarm]";
	gpb.registerFileParameter<std::size_t, std::size_t>(
		"nNeighborhoods" // The name of the first variable
		, "nNeighborhoodMembers" // The name of the second variable
		, DEFAULTNNEIGHBORHOODS // The default value for the first variable
		, DEFAULTNNEIGHBORHOODMEMBERS // The default value for the second variable
		, boost::bind(
			&GBaseSwarm::setSwarmSizes
			, this
			, _1
			, _2
		  )
	    , "swarmSize"
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment1
		, comment2
	);

	comment = ""; // Reset the comment string
	comment += "A constant to be multiplied with the personal direction vector;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<double>(
		"cPersonal" // The name of the variable
		, DEFAULTCPERSONAL // The default value
		, boost::bind(
			&GBaseSwarm::setCPersonal
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "A constant to be multiplied with the neighborhood direction vector;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<double>(
		"cNeighborhood" // The name of the variable
		, DEFAULTCNEIGHBORHOOD // The default value
		, boost::bind(
			&GBaseSwarm::setCNeighborhood
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "A constant to be multiplied with the global direction vector;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<double>(
		"cGlobal" // The name of the variable
		, DEFAULTCGLOBAL // The default value
		, boost::bind(
			&GBaseSwarm::setCGlobal
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "A constant to be multiplied with the old velocity vector;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<double>(
		"cVelocity" // The name of the variable
		, DEFAULTCVELOCITY // The default value
		, boost::bind(
			&GBaseSwarm::setCVelocity
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "Sets the velocity-range percentage;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<double>(
		"velocityRangePercentage" // The name of the variable
		, DEFAULTVELOCITYRANGEPERCENTAGE // The default value
		, boost::bind(
			&GBaseSwarm::setVelocityRangePercentage
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "Specifies whether a linear (0) or classical (1);";
	comment += "update rule should be used;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<updateRule>(
		"updateRule" // The name of the variable
		, DEFAULTUPDATERULE // The default value
		, boost::bind(
			&GBaseSwarm::setUpdateRule
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "Specifies whether neighborhoods should be filled up;";
	comment += "randomly or start with equal values;";
	if(showOrigin) comment += "[GBaseSwarm]";
	gpb.registerFileParameter<bool>(
		"randomFillUp" // The name of the variable
		, true // The default value
		, boost::bind(
			&GBaseSwarm::setNeighborhoodsRandomFillUp
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GBaseSwarm::getAlgorithmName() const {
	return std::string("Swarm Algorithm");
}

/******************************************************************************/
/**
 * This function does some preparatory work and tagging required by swarm algorithms. It is called
 * from within GOptimizationAlgorithmT<GParameterSet>::optimize(), immediately before the actual optimization cycle starts.
 */
void GBaseSwarm::init() {
	// To be performed before any other action
	GOptimizationAlgorithmT<GParameterSet>::init();

   // Extract the boundaries of all parameters
   this->at(0)->boundaries(dblLowerParameterBoundaries_, dblUpperParameterBoundaries_);

#ifdef DEBUG
   // Size matters!
   if(dblLowerParameterBoundaries_.size() != dblUpperParameterBoundaries_.size()) {
      glogger
      << "In GBaseSwarm::init(): Error!" << std::endl
      << "Found invalid sizes: "
      << dblLowerParameterBoundaries_.size() << " / " << dblUpperParameterBoundaries_.size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // Calculate the allowed maximum values of the velocities
   double l = getVelocityRangePercentage();
   dblVelVecMax_.clear();
   for(std::size_t i=0; i<dblLowerParameterBoundaries_.size(); i++) {
      dblVelVecMax_.push_back(l * (dblUpperParameterBoundaries_[i] - dblLowerParameterBoundaries_[i]));
   }

   // Make sure the velocities_ vector is really empty
   velocities_.clear();

   // Create copies of our individuals in the velocities_ vector.
   for(GBaseSwarm::iterator it=this->begin(); it!=this->end(); ++it) {
      // Create a copy of the current individual. Note that, if you happen
      // to have assigned anything else than a GParameterSet derivative to
      // the swarm, then the following line will throw in DEBUG mode or return
      // undefined results in RELEASE mode
      boost::shared_ptr<GParameterSet> p((*it)->clone<GParameterSet>());

      // Extract the parameter vector
      std::vector<double> velVec;
      p->streamline(velVec);

#ifdef DEBUG
      // Check that the number of parameters equals those in the velocity boundaries
      if(velVec.size() != dblLowerParameterBoundaries_.size() || velVec.size() != dblVelVecMax_.size()) {
         glogger
         << "In GBaseSwarm::init(): Error! (2)" << std::endl
         << "Found invalid sizes: " << velVec.size()
         << " / " << dblLowerParameterBoundaries_.size() << std::endl
         << " / " << dblVelVecMax_.size() << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

      // Randomly initialize the velocities
      for(std::size_t i=0; i<velVec.size(); i++) {
         double range = dblVelVecMax_[i];
         velVec[i] = gr.uniform_real<double>(-range, range);
      }

      // Load the array into the velocity object
      p->assignValueVector<double>(velVec);

      // Add the initialized velocity to the array.
      velocities_.push_back(p);

      // TODO: do we need to mark any individuals as dirty here ?
   }
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBaseSwarm::finalize() {
   // Remove remaining velocity individuals. The boost::shared_ptr<GParameterSet>s
   // will take care of deleting the GParameterSet objects.
   velocities_.clear();

	// Last action
	GOptimizationAlgorithmT<GParameterSet>::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
boost::shared_ptr<GPersonalityTraits> GBaseSwarm::getPersonalityTraits() const {
   return boost::shared_ptr<GSwarmPersonalityTraits>(new GSwarmPersonalityTraits());
}

/******************************************************************************/
/**
 * This function implements the logic that constitutes each cycle of a swarm algorithm. The
 * function is called by GOptimizationAlgorithmT<GParameterSet>::optimize() for each iteration of
 * the optimization,
 *
 * @return The value of the best individual found
 */
boost::tuple<double, double> GBaseSwarm::cycleLogic() {
   boost::tuple<double, double> bestIndividualFitness;

	// First update the positions and neighborhood ids
	updatePositions();

	// Now update each individual's fitness
	runFitnessCalculation();

	// Perform post-evaluation updates (mostly of individuals)
	postEvaluationWork();

	// Search for the personal, neighborhood and globally best individuals and
	// update the lists of best solutions, if necessary.
	bestIndividualFitness = findBests();

	// The population might be in a bad state. Check and fix.
	adjustPopulation();

	// Return the result to the audience
	return bestIndividualFitness;
}

/******************************************************************************/
/**
 * Fixes the population after a job submission. We do nothing by default. This
 * function was introduced to avoid having to add a separate cycleLogic to
 * GBrokerSwarm.
 */
void GBaseSwarm::adjustNeighborhoods()
{ /* nothing */ }

/******************************************************************************/
/**
 * Triggers an update of all individual's positions. Also makes sure each
 * individual has the correct neighborhood id
 */
void GBaseSwarm::updatePositions() {
	std::size_t neighborhood_offset = 0;
	GBaseSwarm::iterator start = this->begin();

#ifdef DEBUG
   // Cross-check that we have the nominal amount of individuals
   if(this->size() != nNeighborhoods_*defaultNNeighborhoodMembers_) {
      glogger
      << "In GBaseSwarm::updatePositions(): Error!" << std::endl
      << "Invalid number of individuals found." << std::endl
      << "Expected " << nNeighborhoods_*defaultNNeighborhoodMembers_ << " but got " << this->size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	// First update all positions
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
#ifdef DEBUG
		if(afterFirstIteration()) {
			if(!neighborhood_bests_[n]) {
			   glogger
			   << "In GBaseSwarm::updatePositions():" << std::endl
            << "neighborhood_bests_[" << n << "] is empty." << std::endl
            << GEXCEPTION;
			}

			if(n==0 && !global_best_) { // Only check for the first n
			   glogger
			   << "In GBaseSwarm::updatePositions():" << std::endl
            << "global_best_ is empty." << std::endl
            << GEXCEPTION;
			}
		}

	   // Check that the number if individuals in each neighborhoods has the expected value
		if(nNeighborhoodMembers_[n] != defaultNNeighborhoodMembers_) {
		  glogger
		  << "In GBaseSwarm::updatePositions(): Error!" << std::endl
        << "Invalid number of members in neighborhood " << n << ": " << nNeighborhoodMembers_[n] << std::endl
        << GEXCEPTION;
		}
#endif /* DEBUG */

		for(std::size_t member=0; member<nNeighborhoodMembers_[n]; member++) {
			GBaseSwarm::iterator current = start + neighborhood_offset;

			// Update the neighborhood ids
			this->at(neighborhood_offset)->getPersonalityTraits<GSwarmPersonalityTraits>()->setNeighborhood(n);

			// Note: global/n bests and velocities haven't been determined yet in the first iteration and are not needed there
			if(afterFirstIteration() && !(*current)->getPersonalityTraits<GSwarmPersonalityTraits>()->checkNoPositionUpdateAndReset()) {
				// Update the swarm positions:
				updateIndividualPositions(
					n
					, (*current)
					, neighborhood_bests_[n]
					, global_best_
					, velocities_[neighborhood_offset]
					, boost::make_tuple(
						getCPersonal()
						, getCNeighborhood()
						, getCGlobal()
						, getCVelocity()
					  )
				);
			}

			neighborhood_offset++;
		}
	}
}

/******************************************************************************/
/**
 * Update the individual's positions. Note that we use a boost::tuple as an argument,
 * so that we do not have to pass too many parameters (problematic with boost::bind).
 *
 * @param neighborhood The neighborhood that has been assigned to the individual
 * @param ind The individual whose position should be updated
 * @param neighborhood_best_tmp The best data set of the individual's neighborhood
 * @param global_best_tmp The globally best individual so far
 * @param velocity A velocity vector
 * @param constants A boost::tuple holding the various constants needed for the position update
 */
void GBaseSwarm::updateIndividualPositions(
	  const std::size_t& neighborhood
	  , boost::shared_ptr<GParameterSet> ind
	  , boost::shared_ptr<GParameterSet> neighborhood_best
	  , boost::shared_ptr<GParameterSet> global_best
	  , boost::shared_ptr<GParameterSet> velocity
	  , boost::tuple<double, double, double, double> constants
) {
	// Extract the constants from the tuple
	double cPersonal     = boost::get<0>(constants);
	double cNeighborhood = boost::get<1>(constants);
	double cGlobal       = boost::get<2>(constants);
	double cVelocity     = boost::get<3>(constants);

#ifdef DEBUG
	// Do some error checking
	if(!ind) {
	   glogger
	   << "In GBaseSwarm::updateIndividualPositions():" << std::endl
      << "Found empty individual \"ind\"" << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	// Extract the personal best
	boost::shared_ptr<GParameterSet> personal_best = ind->getPersonalityTraits<GSwarmPersonalityTraits>()->getPersonalBest();

	// Further error checks
#ifdef DEBUG
	if(!personal_best) {
	   glogger
	   << "In GBaseSwarm::updateIndividualPositions():" << std::endl
      << "Found empty individual \"personal_best\"" << std::endl
      << GEXCEPTION;
	}

	if(!neighborhood_best) {
	   glogger
	   << "In GBaseSwarm::updateIndividualPositions():" << std::endl
      << "Found empty individual \"neighborhood_best\"" << std::endl
      << GEXCEPTION;
	}

	if(!global_best) {
	   glogger
	   << "In GBaseSwarm::updateIndividualPositions():" << std::endl
      << "Found empty individual \"global_best\"" << std::endl
      << GEXCEPTION;
	}

	if(!velocity) {
	   glogger
	   << "In GBaseSwarm::updateIndividualPositions():" << std::endl
      << "Found empty individual \"velocity\"" << std::endl
      << GEXCEPTION;
	}

#endif /* DEBUG */

	// Extract the vectors for the individual, the personal, neighborhood and global bests,
	// as well as the velocity
	std::vector <double> indVec, personalBestVec, nbhBestVec, glbBestVec, velVec;
	ind->streamline(indVec);
	personal_best->streamline(personalBestVec);
	neighborhood_best->streamline(nbhBestVec);
	global_best->streamline(glbBestVec);
	velocity->streamline(velVec);

	// Subtract the individual vector from the personal, neighborhood and global bests
	Gem::Common::subtractVec<double>(personalBestVec, indVec);
	Gem::Common::subtractVec<double>(nbhBestVec, indVec);
	Gem::Common::subtractVec<double>(glbBestVec, indVec);

	switch(updateRule_) {
	case SWARM_UPDATERULE_CLASSIC:
		// Multiply each floating point value with a random fp number in the range [0,1[ times a constant
		for(std::size_t i=0; i<personalBestVec.size(); i++) {
			personalBestVec[i] *= (cPersonal * gr.uniform_01<double>());
			nbhBestVec[i] *= (cNeighborhood * gr.uniform_01<double>());
			glbBestVec[i] *= (cGlobal * gr.uniform_01<double>());
		}
		break;

	case SWARM_UPDATERULE_LINEAR:
		// Multiply each position with the same random floating point number times a constant
		Gem::Common::multVecConst<double>(personalBestVec, cPersonal * gr.uniform_01<double>());
		Gem::Common::multVecConst<double>(nbhBestVec, cNeighborhood * gr.uniform_01<double>());
		Gem::Common::multVecConst<double>(glbBestVec, cGlobal * gr.uniform_01<double>());
		break;

	default:
		{
		   glogger
		   << "GBaseSwarm::updateIndividualPositions(): Error!" << std::endl
         << "Invalid update rule requested: " << updateRule_ << std::endl
         << GEXCEPTION;
		}
		break;
	}

	// Scale the velocity
	Gem::Common::multVecConst<double>(velVec, cVelocity);

	// Add the personal and neighborhood parameters to the velocity
	Gem::Common::addVec<double>(velVec, personalBestVec);
	Gem::Common::addVec<double>(velVec, nbhBestVec);

	// Adding a velocity component towards the global best only
	// makes sense if there is more than one neighborhood
	if(getNNeighborhoods() > 1) {
		Gem::Common::addVec<double>(velVec, glbBestVec);
	}

	// Prune the velocity vector so that we can
	// be sure it is inside of the allowed range
	pruneVelocity(velVec);

	// Add the velocity parameters to the individual's parameters
	Gem::Common::addVec<double>(indVec, velVec);

	// Update the velocity individual
	velocity->assignValueVector<double>(velVec);

	// Update the candidate solution
	ind->assignValueVector<double>(indVec);
}

/******************************************************************************/
/**
 * Adjusts the velocity vector so that its parameters don't exceed the allowed value range.
 *
 * @param velVec the velocity vector to be adjusted
 */
void GBaseSwarm::pruneVelocity(std::vector<double>& velVec) {
#ifdef DEBUG
	if(velVec.size() != dblVelVecMax_.size()) {
	   glogger
	   << "In GBaseSwarm::pruneVelocity(): Error!" << std::endl
      << "Found invalid vector sizes: " << velVec.size() << " / " << dblVelVecMax_.size() << std::endl
      << GEXCEPTION;
	}

#endif

	// Find the parameter that exceeds the allowed range by the largest percentage
	double currentPercentage = 0.;
	double maxPercentage = 0.;
	bool overflowFound=false;
	for(std::size_t i=0; i<velVec.size(); i++) {
#ifdef DEBUG
		if(dblVelVecMax_[i] <= 0.) {
		   glogger
		   << "In GBaseSwarm::pruneVelocity(): Error!" << std::endl
         << "Found invalid max value: " << dblVelVecMax_[i] << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		if(fabs(velVec[i]) > dblVelVecMax_[i]) {
			overflowFound=true;
			currentPercentage = fabs(velVec[i])/dblVelVecMax_[i];
			if(currentPercentage > maxPercentage) {
				maxPercentage = currentPercentage;
			}
		}
	}

	if(overflowFound) {
		// Scale all velocity entries by maxPercentage
		for(std::size_t i=0; i<velVec.size(); i++) {
#ifdef DEBUG
			if(maxPercentage <= 0.) {
			   glogger
			   << "In GBaseSwarm::pruneVelocity(): Error!" << std::endl
            << "Invalid maxPercentage: " << maxPercentage << std::endl
            << GEXCEPTION;
			}
#endif
			velVec[i] /= maxPercentage;
		}
	}
}

/******************************************************************************/
/**
 * Updates the best individuals found. This function assumes that the population already contains individuals
 * and that the neighborhood and global bests have been initialized (possibly with dummy values). This should have
 * happened in the adjustPopulation() function. It also assumes that all individuals have already been evaluated.
 *
 * @return The best evaluation found in this iteration
 */
boost::tuple<double, double> GBaseSwarm::findBests() {
	std::size_t bestLocalId = 0;
	boost::tuple<double, double> bestLocalFitness = boost::make_tuple(this->getWorstCase(), this->getWorstCase());
	boost::tuple<double, double> bestIterationFitness = boost::make_tuple(this->getWorstCase(), this->getWorstCase());

   GBaseSwarm::iterator it;

#ifdef DEBUG
	for(it=this->begin(); it!=this->end(); ++it) {
		if((*it)->isDirty()) {
		   glogger
		   << "In GBaseSwarm::findBests(): Error!" << std::endl
         << "Found individual in position " << std::distance(this->begin(), it) << " in iteration " << this->getIteration() << std::endl
         << "whose dirty flag is set." << std::endl
         << GEXCEPTION;
		}
	}
#endif /* DEBUG */

	// Update the personal bests of all individuals
   if(inFirstIteration()) {
      GBaseSwarm::iterator it;
      for(it=this->begin(); it!=this->end(); ++it) {
         updatePersonalBest(*it);
      }
   } else {
      for(it=this->begin(); it!=this->end(); ++it) {
         updatePersonalBestIfBetter(*it);
      }
   }

	// Sort individuals in all neighborhoods according to their fitness
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		// identify the first and last id of the individuals in the current neighborhood
		std::size_t firstCounter = getFirstNIPos(n);
		std::size_t lastCounter = getLastNIPos(n);

		// Only partially sort the arrays
      std::sort(
         this->begin() + firstCounter
         , this->begin() + lastCounter
         , boost::bind(&GParameterSet::transformedFitness, _1) < boost::bind(&GParameterSet::transformedFitness, _2)
      );

		// Check whether the best individual of the neighborhood is better than
		// the best individual found so far in this neighborhood
		if(inFirstIteration()) {
			neighborhood_bests_.at(n) = (*(this->begin() + firstCounter))->clone<GParameterSet>();
		}
		else {
			if(this->isBetter(
			      (*(this->begin() + firstCounter))->transformedFitness()
			      , neighborhood_bests_.at(n)->transformedFitness()
            )
			) {
				(neighborhood_bests_.at(n))->load(*(this->begin() + firstCounter));
			}
		}
	}

	// Identify the best individuals among all neighborhood bests
	for(std::size_t n=0; n<nNeighborhoods_; n++) {
		if(this->isBetter((neighborhood_bests_.at(n))->transformedFitness(), boost::get<G_TRANSFORMED_FITNESS>(bestLocalFitness))) {
			bestLocalId = n;
			bestLocalFitness = (neighborhood_bests_.at(n))->getFitnessTuple();
		}
	}

	// Compare the best neighborhood individual with the globally best individual and
	// update it, if necessary. Initialize it in the first generation.
	if(inFirstIteration()) {
		global_best_= (neighborhood_bests_.at(bestLocalId))->clone<GParameterSet>();
	} else {
		if(this->isBetter(boost::get<G_TRANSFORMED_FITNESS>(bestLocalFitness), global_best_->transformedFitness())) {
			global_best_->load(neighborhood_bests_.at(bestLocalId));
		}
	}

	// Identify the best fitness in the current iteration
	for(std::size_t i=0; i<this->size(); i++) {
	   if(this->isBetter(boost::get<G_TRANSFORMED_FITNESS>(this->at(i)->getFitnessTuple()), boost::get<G_TRANSFORMED_FITNESS>(bestIterationFitness))) {
	      bestIterationFitness = this->at(i)->getFitnessTuple();
	   }
	}

	return bestIterationFitness;
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks. This function implements
 * the purely virtual function GOptimizationAlgorithmT<GParameterSet>::adjustPopulation() .
 */
void GBaseSwarm::adjustPopulation() {
	const std::size_t currentSize = this->size();
	const std::size_t defaultPopSize = getDefaultPopulationSize();
	const std::size_t nNeighborhoods = getNNeighborhoods();

	if(currentSize==0) {
	   glogger
	   << "In GBaseSwarm::adjustPopulation() :" << std::endl
      << "No individuals found in the population." << std::endl
      << "You need to add at least one individual before" << std::endl
      << "the call to this function." << std::endl
      << GEXCEPTION;
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
		// Fill in remaining items in each neighborhood.
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
	   glogger
	   << "In GBaseSwarm::adjustPopulation() :" << std::endl
      << "Expected at least a population size of " << defaultPopSize << std::endl
      << "but found a size of " << this->size() << ", which is too small." << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	// We do not initialize the neighborhood and global bests here, as this requires the value of
	// all individuals to be calculated.
}

/******************************************************************************/
/**
 * Small helper function that helps to fill up a neighborhood, if there is just one entry in it.
 */
void GBaseSwarm::fillUpNeighborhood1() {
	// Do some error checking
	if(this->size() != nNeighborhoods_) {
	   glogger
	   << "In GBaseSwarm::fillUpNeighborhood1():" << std::endl
      << "Invalid size: " << this->size() << " Expected " << nNeighborhoods_ << std::endl
      << GEXCEPTION;
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
				   glogger
				   << "In GBaseSwarm::fillUpNeighborhood1():" << std::endl
               << "Found empty position " << n << std::endl
               << GEXCEPTION;
				}
#endif /* DEBUG */

				(*(this->begin()+n+1))->randomInit();
			}
		}

		// Update the number of individuals in each neighborhood
		nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
	}
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for personal distances.
 *
 * @param c_personal A static multiplier for personal distances
 */
void GBaseSwarm::setCPersonal(double c_personal) {
	c_personal_ = c_personal;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for personal distances
 *
 * @return The static multiplier for personal distances
 */
double GBaseSwarm::getCPersonal() const {
	return c_personal_;
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for neighborhood distances.
 *
 * @param c_neighborhood A static multiplier for neighborhood distances
 */
void GBaseSwarm::setCNeighborhood(double c_neighborhood) {
	c_neighborhood_ = c_neighborhood;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for neighborhood distances
 *
 * @return A static multiplier for neighborhood distances
 */
double GBaseSwarm::getCNeighborhood() const {
	return c_neighborhood_;
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for global distances
 *
 * @param c_global A static multiplier for global distances
 */
void GBaseSwarm::setCGlobal(double c_global) {
	c_global_ = c_global;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for global distances
 *
 * @return The static multiplier for global distances
 */
double GBaseSwarm::getCGlobal() const {
	return c_global_;
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for velocities
 *
 * @param c_velocity A static multiplier for velocities
 */
void GBaseSwarm::setCVelocity(double c_velocity) {
	c_velocity_ = c_velocity;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for velocities
 *
 * @return The static multiplier for velocities
 */
double GBaseSwarm::getCVelocity() const {
	return c_velocity_;
}

/******************************************************************************/
/**
 * Allows to set the velocity range percentage
 *
 * @param velocityRangePercentage The velocity range percentage
 */
void GBaseSwarm::setVelocityRangePercentage(double velocityRangePercentage) {
	// Do some error checking
	if(velocityRangePercentage <= 0. || velocityRangePercentage > 1.) {
	   glogger
	   << "In GBaseSwarm::setVelocityRangePercentage()" << std::endl
      << "Invalid velocityRangePercentage: " << velocityRangePercentage << std::endl
      << GEXCEPTION;
	}

	velocityRangePercentage_ = velocityRangePercentage;
}

/******************************************************************************/
/**
 * Allows to retrieve the velocity range percentage
 *
 * @return The velocity range percentage
 */
double GBaseSwarm::getVelocityRangePercentage() const {
	return velocityRangePercentage_;
}

/******************************************************************************/
/**
 * Retrieves the number of neighborhoods
 *
 * @return The number of neighborhoods in the population
 */
std::size_t GBaseSwarm::getNNeighborhoods() const {
	return nNeighborhoods_;
}

/******************************************************************************/
/**
 * Retrieves the default number of individuals in each neighborhood
 *
 * @return The default number of individuals in each neighborhood
 */
std::size_t GBaseSwarm::getDefaultNNeighborhoodMembers() const {
	return defaultNNeighborhoodMembers_;
}

/******************************************************************************/
/**
 * Retrieves the current number of individuals in a given neighborhood
 *
 * @return The current number of individuals in a given neighborhood
 */
std::size_t GBaseSwarm::getCurrentNNeighborhoodMembers(const std::size_t& neighborhood) const {
	return nNeighborhoodMembers_[neighborhood];
}

/******************************************************************************/
/**
 * Allows to specify the update rule to be used by the swarm.
 *
 * @param ur The desired update rule
 */
void GBaseSwarm::setUpdateRule(updateRule ur) {
	updateRule_ = ur;
}

/******************************************************************************/
/**
 * Allows to retrieve the update rule currently used by the swarm.
 *
 * @return The current update rule
 */
updateRule GBaseSwarm::getUpdateRule() const {
	return updateRule_;
}

/******************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have equal value
 */
void GBaseSwarm::setNeighborhoodsEqualFillUp() {
	randomFillUp_=false;
}

/******************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have a random value
 */
void GBaseSwarm::setNeighborhoodsRandomFillUp(bool randomFillUp) {
	randomFillUp_=randomFillUp;
}

/******************************************************************************/
/**
 * Allows to check whether neighborhoods are filled up with random individuals
 *
 * @return A boolean indicating whether neighborhoods are filled up with random values
 */
bool GBaseSwarm::neighborhoodsFilledUpRandomly() const {
	return randomFillUp_;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GBaseSwarm::getNProcessableItems() const {
	return this->size(); // All items in the population are updated in each iteration and need to be processed
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBaseSwarm::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GParameterSet>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseSwarm::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseSwarm::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GBaseSwarm::GSwarmOptimizationMonitor::GSwarmOptimizationMonitor()
	: xDim_(DEFAULTXDIMOM)
	, yDim_(DEFAULTYDIMOM)
   , resultFile_(DEFAULTROOTRESULTFILEOM)
   , fitnessGraph_(new Gem::Common::GGraph2D())
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSwarmOptimizationMonitor object
 */
GBaseSwarm::GSwarmOptimizationMonitor::GSwarmOptimizationMonitor(const GBaseSwarm::GSwarmOptimizationMonitor& cp)
	: GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT(cp)
	, xDim_(cp.xDim_)
	, yDim_(cp.yDim_)
   , resultFile_(cp.resultFile_)
   , fitnessGraph_(new Gem::Common::GGraph2D())
  { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GBaseSwarm::GSwarmOptimizationMonitor::~GSwarmOptimizationMonitor()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GSwarmOptimizationMonitor object
 * @return A constant reference to this object
 */
const GBaseSwarm::GSwarmOptimizationMonitor& GBaseSwarm::GSwarmOptimizationMonitor::operator=(const GBaseSwarm::GSwarmOptimizationMonitor& cp){
	GBaseSwarm::GSwarmOptimizationMonitor::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GSwarmOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseSwarm::GSwarmOptimizationMonitor::operator==(const GBaseSwarm::GSwarmOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseSwarm::GSwarmOptimizationMonitor::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GSwarmOptimizationMonitor object
 *
 * @param  cp A constant reference to another GSwarmOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseSwarm::GSwarmOptimizationMonitor::operator!=(const GBaseSwarm::GSwarmOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseSwarm::GSwarmOptimizationMonitor::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBaseSwarm::GSwarmOptimizationMonitor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBaseSwarm::GSwarmOptimizationMonitor *p_load = GObject::gobject_conversion<GBaseSwarm::GSwarmOptimizationMonitor >(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::checkRelationshipWith(cp, e, limit, "GBaseSwarm::GSwarmOptimizationMonitor", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm::GSwarmOptimizationMonitor", xDim_, p_load->xDim_, "xDim_", "p_load->xDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm::GSwarmOptimizationMonitor", yDim_, p_load->yDim_, "yDim_", "p_load->yDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseSwarm::GSwarmOptimizationMonitor", resultFile_, p_load->resultFile_, "resultFile_", "p_load->resultFile_", e , limit));

	return evaluateDiscrepancies("GBaseSwarm::GSwarmOptimizationMonitor", caller, deviations, e);
}

/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param resultFile The desired name of the result file
 */
void GBaseSwarm::GSwarmOptimizationMonitor::setResultFileName(
      const std::string& resultFile
) {
  resultFile_ = resultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GBaseSwarm::GSwarmOptimizationMonitor::getResultFileName() const {
  return resultFile_;
}

/******************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param xDim The desired dimension of the canvas in x-direction
 * @param yDim The desired dimension of the canvas in y-direction
 */
void GBaseSwarm::GSwarmOptimizationMonitor::setDims(const boost::uint16_t& xDim, const boost::uint16_t& yDim) {
	xDim_ = xDim;
	yDim_ = yDim;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
boost::uint16_t GBaseSwarm::GSwarmOptimizationMonitor::getXDim() const {
	return xDim_;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
boost::uint16_t GBaseSwarm::GSwarmOptimizationMonitor::getYDim() const {
	return yDim_;
}

/******************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseSwarm::GSwarmOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
#ifdef DEBUG
   if(goa->getOptimizationAlgorithm() != "PERSONALITY_SWARM") {
      glogger
      << "In GBaseSwarm::GSwarmOptimizationMonitor::cycleInformation():" << std::endl
      << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // Configure the plotter
   fitnessGraph_->setXAxisLabel("Iteration");
   fitnessGraph_->setYAxisLabel("Fitness");
   fitnessGraph_->setPlotMode(Gem::Common::CURVE);
}

/******************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseSwarm::GSwarmOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   // Perform the conversion to the target algorithm
   GBaseSwarm * const swarm = static_cast<GBaseSwarm * const>(goa);

   fitnessGraph_->add(boost::tuple<double,double>(swarm->getIteration(), boost::get<G_TRANSFORMED_FITNESS>(swarm->getBestKnownPrimaryFitness())));
}

/******************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseSwarm::GSwarmOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   Gem::Common::GPlotDesigner gpd(
         "Fitness of best individual"
         , 1, 1 // A single plot / grid of dimension 1/1
   );

   gpd.setCanvasDimensions(xDim_, yDim_);
   gpd.registerPlotter(fitnessGraph_);

   gpd.writeToFile(this->getResultFileName());

   // Store a new graph so repeated calls to optimize create new graphs
   fitnessGraph_.reset(new Gem::Common::GGraph2D());
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GSwarmOptimizationMonitor object, camouflaged as a GObject
 */
void GBaseSwarm::GSwarmOptimizationMonitor::load_(const GObject* cp) {
	const GBaseSwarm::GSwarmOptimizationMonitor *p_load = gobject_conversion<GBaseSwarm::GSwarmOptimizationMonitor>(cp);

	// Load the parent classes' data ...
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::load_(cp);

	// ... and then our local data
	xDim_ = p_load->xDim_;
	yDim_ = p_load->yDim_;
	resultFile_ = p_load->resultFile_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GBaseSwarm::GSwarmOptimizationMonitor::clone_() const {
	return new GBaseSwarm::GSwarmOptimizationMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBaseSwarm::GSwarmOptimizationMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::GSwarmOptimizationMonitor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseSwarm::GSwarmOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::GSwarmOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseSwarm::GSwarmOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::GSwarmOptimizationMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

