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

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. All work is delegated to another constructor.
 */
GBaseSwarm::GBaseSwarm()
	: GBaseSwarm(DEFAULTNNEIGHBORHOODS, DEFAULTNNEIGHBORHOODMEMBERS)
{ /* nothing */ }

/******************************************************************************/
/**
 * This constructor sets the number of neighborhoods and the number of individuals in them. Note that there
 * is no public default constructor, as it is only needed for de-serialization purposes.
 *
 * @param nNeighborhoods The desired number of neighborhoods (hardwired to >= 1)
 * @oaram nNeighborhoodMembers The default number of individuals in each neighborhood (hardwired to >= 2)
 */
GBaseSwarm::GBaseSwarm(
	const std::size_t &nNeighborhoods
	, const std::size_t &defaultNNeighborhoodMembers
)
	: GOptimizationAlgorithmT<GParameterSet>()
	, nNeighborhoods_((nNeighborhoods>=1) ? nNeighborhoods : 1)
	, defaultNNeighborhoodMembers_((defaultNNeighborhoodMembers >= 2) ? defaultNNeighborhoodMembers: 2)
{
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_ * defaultNNeighborhoodMembers_);
}

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GBaseSwarm object
 */
GBaseSwarm::GBaseSwarm(const GBaseSwarm &cp)
	: GOptimizationAlgorithmT<GParameterSet>(cp)
	, nNeighborhoods_(cp.nNeighborhoods_)
	, defaultNNeighborhoodMembers_(cp.defaultNNeighborhoodMembers_)
	, nNeighborhoodMembers_(cp.nNeighborhoodMembers_)
	, global_best_((cp.afterFirstIteration()) ? (cp.global_best_)->clone<GParameterSet>() : std::shared_ptr<GParameterSet>())
	, neighborhood_bests_(nNeighborhoods_) // We copy the smart pointers over later
	, c_personal_(cp.c_personal_)
	, c_neighborhood_(cp.c_neighborhood_)
	, c_global_(cp.c_global_)
	, c_velocity_(cp.c_velocity_)
	, updateRule_(cp.updateRule_)
	, randomFillUp_(cp.randomFillUp_)
	, repulsionThreshold_(cp.repulsionThreshold_)
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
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_ * defaultNNeighborhoodMembers_);

	// Clone cp's best individuals in each neighborhood
	if (cp.afterFirstIteration()) {
		for (std::size_t i = 0; i < nNeighborhoods_; i++) {
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

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBaseSwarm &GBaseSwarm::operator=(const GBaseSwarm &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBaseSwarm object
 *
 * @param  cp A constant reference to another GBaseSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseSwarm::operator==(const GBaseSwarm &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GBaseSwarm object
 *
 * @param  cp A constant reference to another GBaseSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseSwarm::operator!=(const GBaseSwarm &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

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
 * Loads the data of another GBaseSwarm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GBaseSwarm object, camouflaged as a GObject
 */
void GBaseSwarm::load_(const GObject *cp) {
	// Check that we are dealing with a GBooleanAdaptor reference independent of this object and convert the pointer
	const GBaseSwarm *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSwarm >(cp, this);

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
	repulsionThreshold_ = p_load->repulsionThreshold_;

	dblLowerParameterBoundaries_ = p_load->dblLowerParameterBoundaries_;
	dblUpperParameterBoundaries_ = p_load->dblUpperParameterBoundaries_;
	dblVelVecMax_ = p_load->dblVelVecMax_;

	velocityRangePercentage_ = p_load->velocityRangePercentage_;

	// We start from scratch if the number of neighborhoods or the alleged number of members in them differ
	if (nNeighborhoods_ != p_load->nNeighborhoods_ ||
		 !nNeighborhoodMembersEqual(nNeighborhoodMembers_, p_load->nNeighborhoodMembers_)) {
		nNeighborhoods_ = p_load->nNeighborhoods_;

		nNeighborhoodMembers_.clear();
		neighborhood_bests_.clear();

		nNeighborhoodMembers_.resize(nNeighborhoods_);
		neighborhood_bests_.resize(nNeighborhoods_);

		// Copy the neighborhood bests and number of neighborhood members over
		for (std::size_t i = 0; i < nNeighborhoods_; i++) {
			nNeighborhoodMembers_[i] = p_load->nNeighborhoodMembers_[i];
			// The following only makes sense if this is not the first iteration. Note that
			// getIteration will return the "foreign" GBaseSwarm object's iteration, as it has
			// already been copied.
			if (afterFirstIteration()) {
				neighborhood_bests_[i] = p_load->neighborhood_bests_[i]->clone<GParameterSet>();
			}
			// we do not need to reset the neighborhood_bests_, as that array has just been created
		}
	}
	else { // We now assume that we can just load neighborhood bests in each position.
		// Copying only makes sense if the foreign GBaseSwarm object's iteration is larger
		// than the iteration offset. Note that getIteration() will return the foreign iteration,
		// as that value has already been copied.
		if (afterFirstIteration()) {
			for (std::size_t i = 0; i < nNeighborhoods_; i++) {
				// We might be in a situation where the std::shared_ptr which usually
				// holds the neighborhood bests has not yet been initialized
				if (neighborhood_bests_[i]) {
					neighborhood_bests_[i]->GObject::load(p_load->neighborhood_bests_[i]);
				} else {
					neighborhood_bests_[i] = p_load->neighborhood_bests_[i]->clone<GParameterSet>();
				}
			}
		} else {
			for (std::size_t i = 0; i < nNeighborhoods_; i++) {
				neighborhood_bests_[i].reset();
			}
		}
	}

	// Copy the global best over
	if (p_load->afterFirstIteration()) { // cp has a global best, we don't
		if (global_best_) { // If we already have a global best, just load the other objects global best
			global_best_->GObject::load(p_load->global_best_);
		} else {
			global_best_ = p_load->GObject::clone<GParameterSet>();
		}
	}
	else if (p_load->inFirstIteration()) { // cp does not have a global best
		global_best_.reset(); // empty the smart pointer
	}
	// else {} // We do not need to do anything if both iterations are 0 as there is no global best at all
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBaseSwarm::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBooleanAdaptor reference independent of this object and convert the pointer
	const GBaseSwarm *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSwarm >(cp, this);

	GToken token("GBaseSwarm", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GOptimizationAlgorithmT<GParameterSet>>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(nNeighborhoods_, p_load->nNeighborhoods_), token);
	compare_t(IDENTITY(defaultNNeighborhoodMembers_, p_load->defaultNNeighborhoodMembers_), token);
	compare_t(IDENTITY(global_best_, p_load->global_best_), token);
	compare_t(IDENTITY(c_personal_, p_load->c_personal_), token);
	compare_t(IDENTITY(c_neighborhood_, p_load->c_neighborhood_), token);
	compare_t(IDENTITY(c_global_, p_load->c_global_), token);
	compare_t(IDENTITY(c_velocity_, p_load->c_velocity_), token);
	compare_t(IDENTITY(updateRule_, p_load->updateRule_), token);
	compare_t(IDENTITY(randomFillUp_, p_load->randomFillUp_), token);
	compare_t(IDENTITY(repulsionThreshold_, p_load->repulsionThreshold_), token);
	compare_t(IDENTITY(dblLowerParameterBoundaries_, p_load->dblLowerParameterBoundaries_), token);
	compare_t(IDENTITY(dblUpperParameterBoundaries_, p_load->dblUpperParameterBoundaries_), token);
	compare_t(IDENTITY(dblVelVecMax_, p_load->dblVelVecMax_), token);
	compare_t(IDENTITY(velocityRangePercentage_, p_load->velocityRangePercentage_), token);

	// The next checks only makes sense if the number of neighborhoods are equal
	if (nNeighborhoods_ == p_load->nNeighborhoods_) {
		compare_t(IDENTITY(nNeighborhoodMembers_, p_load->nNeighborhoodMembers_), token);
		// No neighborhood bests have been assigned yet in iteration 0
		if (afterFirstIteration()) {
			compare_t(IDENTITY(neighborhood_bests_, p_load->neighborhood_bests_), token);
		}
	}

	// React on deviations from the expectation
	token.evaluate();
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
	if (nNeighborhoods == 0) {
		glogger
		<< "In GBaseSwarm::setSwarmSizes(): Warning!" << std::endl
		<< "Requested number of neighborhoods is 0. Setting to 1." << std::endl
		<< GWARNING;
	}

	if (defaultNNeighborhoodMembers <= 1) {
		glogger
		<< "In GBaseSwarm::setSwarmSizes(): Warning!" << std::endl
		<< "Requested number of members in each neighborhood is too small. Setting to 2." << std::endl
		<< GWARNING;
	}

	nNeighborhoods_ = (nNeighborhoods >= 1) ? nNeighborhoods : 1;
	defaultNNeighborhoodMembers_ = (defaultNNeighborhoodMembers >= 2) ? defaultNNeighborhoodMembers : 2;

	// Update our parent class'es values
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize(nNeighborhoods_ * defaultNNeighborhoodMembers_);
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

	this->toFile(boost::filesystem::path(outputFile), getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Loads the state of the object from disc.
 *
 * @param cpFile The name of the file the checkpoint should be loaded from
 */
void GBaseSwarm::loadCheckpoint(const boost::filesystem::path &cpFile) {
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
	const std::vector<std::size_t> &one, const std::vector<std::size_t> &two
) const {
	if (one.size() != two.size()) {
		return false;
	}

	for (std::size_t i = 0; i < one.size(); i++) {
		if (one[i] != two[i]) {
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
std::size_t GBaseSwarm::getFirstNIPos(const std::size_t &neighborhood) const {
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
	const std::size_t &neighborhood, const std::vector<std::size_t> &vec
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

	if (neighborhood == 0) return 0;
	else {   // Sum up the number of members in each neighborhood
		std::size_t nPreviousMembers = 0;
		for (std::size_t n = 0; n < neighborhood; n++) {
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
std::size_t GBaseSwarm::getLastNIPos(const std::size_t &neighborhood) const {
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
	std::shared_ptr < GParameterSet > p
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
	std::shared_ptr < GParameterSet > p
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

	if (this->isBetter
		(
			std::get<G_TRANSFORMED_FITNESS>(
				p->getPersonalityTraits<GSwarmPersonalityTraits>()->getPersonalBestQuality()), p->transformedFitness()
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
 */
void GBaseSwarm::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<std::size_t, std::size_t>(
		"nNeighborhoods" // The name of the first variable
		, "nNeighborhoodMembers" // The name of the second variable
		, DEFAULTNNEIGHBORHOODS // The default value for the first variable
		, DEFAULTNNEIGHBORHOODMEMBERS // The default value for the second variable
		, [this](std::size_t nh, std::size_t nhm) { this->setSwarmSizes(nh, nhm); }
		, "swarmSize"
	)
	<< "The desired number of neighborhoods in the population" << Gem::Common::nextComment()
	<< "The desired number of members in each neighborhood";

	gpb.registerFileParameter<double>(
		"cPersonal" // The name of the variable
		, DEFAULTCPERSONAL // The default value
		, [this](double cp) { this->setCPersonal(cp); }
	)
	<< "A constant to be multiplied with the personal direction vector";

	gpb.registerFileParameter<double>(
		"cNeighborhood" // The name of the variable
		, DEFAULTCNEIGHBORHOOD // The default value
		, [this](double cn) { this->setCNeighborhood(cn); }
	)
	<< "A constant to be multiplied with the neighborhood direction vector";

	gpb.registerFileParameter<double>(
		"cGlobal" // The name of the variable
		, DEFAULTCGLOBAL // The default value
		, [this](double cg) { this->setCGlobal(cg); }
	)
	<< "A constant to be multiplied with the global direction vector";

	gpb.registerFileParameter<double>(
		"cVelocity" // The name of the variable
		, DEFAULTCVELOCITY // The default value
		, [this](double cv) { this->setCVelocity(cv); }
	)
	<< "A constant to be multiplied with the old velocity vector";

	gpb.registerFileParameter<double>(
		"velocityRangePercentage" // The name of the variable
		, DEFAULTVELOCITYRANGEPERCENTAGE // The default value
		, [this](double vrp) { this->setVelocityRangePercentage(vrp); }
	)
	<< "Sets the velocity-range percentage";

	gpb.registerFileParameter<updateRule>(
		"updateRule" // The name of the variable
		, DEFAULTUPDATERULE // The default value
		, [this](updateRule ur) { this->setUpdateRule(ur); }
	)
	<< "Specifies whether a linear (0) or classical (1)" << std::endl
	<< "update rule should be used";

	gpb.registerFileParameter<bool>(
		"randomFillUp" // The name of the variable
		, true // The default value
		, [this](bool nhrf) { this->setNeighborhoodsRandomFillUp(nhrf); }
	)
	<< "Specifies whether neighborhoods should be filled up" << std::endl
	<< "randomly (true) or start with equal values (false)";

	gpb.registerFileParameter<std::uint32_t>(
		"repulsionThreshold" // The name of the variable
		, DEFREPULSIONTHRESHOLD // The default value
		, [this](std::uint32_t rt) { this->setRepulsionThreshold(rt); }
	)
	<< "The number of stalls as of which the algorithm switches to repulsive mode" << std::endl
	<< "Set this to 0 in order to disable this feature";
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
	this->at(0)->boundaries(dblLowerParameterBoundaries_, dblUpperParameterBoundaries_, activityMode::ACTIVEONLY);

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
	for (std::size_t i = 0; i < dblLowerParameterBoundaries_.size(); i++) {
		dblVelVecMax_.push_back(l * (dblUpperParameterBoundaries_[i] - dblLowerParameterBoundaries_[i]));
	}

	// Make sure the velocities_ vector is really empty
	velocities_.clear();

	// Create copies of our individuals in the velocities_ vector.
	std::size_t pos = 0;
	for (GBaseSwarm::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
		if(!(*it)) {
			glogger
			<< "In GBaseSwarm::init(): Error!" << std::endl
			<< "Found empty std::shared_ptr in position " << pos << std::endl
			<< GEXCEPTION;
		}
#endif /* DEBUG */

		// Create a copy of the current individual. Note that, if you happen
		// to have assigned anything else than a GParameterSet derivative to
		// the swarm, then the following line will throw in DEBUG mode or return
		// undefined results in RELEASE mode
		std::shared_ptr <GParameterSet> p((*it)->clone<GParameterSet>());

		// Extract the parameter vector
		std::vector<double> velVec;
		p->streamline(velVec, activityMode::ACTIVEONLY);

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
		for (std::size_t i = 0; i < velVec.size(); i++) {
			double range = dblVelVecMax_[i];
			velVec[i] =
                    GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(-range,range));
		}

		// Load the array into the velocity object
		p->assignValueVector<double>(velVec, activityMode::ACTIVEONLY);
		p->setDirtyFlag(); // Catch cases where a value is calculated for the velocity individual

		// Add the initialized velocity to the array.
		velocities_.push_back(p);

		pos++;
	}

	// Make sure neighborhood_bests_ has the correct size
	// It will only hold empty smart pointers. However, new ones
	// will be assigned in findBests()
	neighborhood_bests_.resize(nNeighborhoods_);
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBaseSwarm::finalize() {
	// Remove remaining velocity individuals. The std::shared_ptr<GParameterSet>s
	// will take care of deleting the GParameterSet objects.
	velocities_.clear();

	// Last action
	GOptimizationAlgorithmT<GParameterSet>::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr <GPersonalityTraits> GBaseSwarm::getPersonalityTraits() const {
	return std::shared_ptr<GSwarmPersonalityTraits>(new GSwarmPersonalityTraits());
}

/******************************************************************************/
/**
 * This function implements the logic that constitutes each cycle of a swarm algorithm. The
 * function is called by GOptimizationAlgorithmT<GParameterSet>::optimize() for each iteration of
 * the optimization,
 *
 * @return The value of the best individual found
 */
std::tuple<double, double> GBaseSwarm::cycleLogic() {
	std::tuple<double, double> bestIndividualFitness;

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
	adjustNeighborhoods();

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
	for (std::size_t n = 0; n < nNeighborhoods_; n++) {
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

		for (std::size_t member = 0; member < nNeighborhoodMembers_[n]; member++) {
			GBaseSwarm::iterator current = start + neighborhood_offset;

			// Update the neighborhood ids
			this->at(neighborhood_offset)->getPersonalityTraits<GSwarmPersonalityTraits>()->setNeighborhood(n);

			// Note: global/n bests and velocities haven't been determined yet in the first iteration and are not needed there
			if (afterFirstIteration() &&
				 !(*current)->getPersonalityTraits<GSwarmPersonalityTraits>()->checkNoPositionUpdateAndReset()) {
				// Update the swarm positions:
				updateIndividualPositions(
					n, (*current), neighborhood_bests_[n], global_best_, velocities_[neighborhood_offset], std::make_tuple(
						getCPersonal(), getCNeighborhood(), getCGlobal(), getCVelocity()
					)
				);
			}

			neighborhood_offset++;
		}
	}
}

/******************************************************************************/
/**
 * Update the individual's positions. Note that we use a std::tuple as an argument,
 * so that we do not have to pass too many parameters.
 *
 * @param neighborhood The neighborhood that has been assigned to the individual
 * @param ind The individual whose position should be updated
 * @param neighborhood_best_tmp The best data set of the individual's neighborhood
 * @param global_best_tmp The globally best individual so far
 * @param velocity A velocity vector
 * @param constants A std::tuple holding the various constants needed for the position update
 */
void GBaseSwarm::updateIndividualPositions(
	const std::size_t &neighborhood
	, std::shared_ptr <GParameterSet> ind
	, std::shared_ptr <GParameterSet> neighborhood_best
	, std::shared_ptr <GParameterSet> global_best
	, std::shared_ptr <GParameterSet> velocity
	, std::tuple<double, double, double, double> constants
) {
	// Extract the constants from the tuple
	double cPersonal = std::get<0>(constants);
	double cNeighborhood = std::get<1>(constants);
	double cGlobal = std::get<2>(constants);
	double cVelocity = std::get<3>(constants);

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
	std::shared_ptr <GParameterSet> personal_best = ind->getPersonalityTraits<GSwarmPersonalityTraits>()->getPersonalBest();

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
	std::vector<double> indVec, personalBestVec, nbhBestVec, glbBestVec, velVec;
	ind->streamline(indVec, activityMode::ACTIVEONLY);
	personal_best->streamline(personalBestVec, activityMode::ACTIVEONLY);
	neighborhood_best->streamline(nbhBestVec, activityMode::ACTIVEONLY);
	global_best->streamline(glbBestVec, activityMode::ACTIVEONLY);
	velocity->streamline(velVec, activityMode::ACTIVEONLY);

	// Subtract the individual vector from the personal, neighborhood and global bests
	Gem::Common::subtractVec<double>(personalBestVec, indVec);
	Gem::Common::subtractVec<double>(nbhBestVec, indVec);
	Gem::Common::subtractVec<double>(glbBestVec, indVec);

	switch (updateRule_) {
		case updateRule::SWARM_UPDATERULE_CLASSIC:
			// Multiply each floating point value with a random fp number in the range [0,1[, times a constant
			for (std::size_t i = 0; i < personalBestVec.size(); i++) {
				personalBestVec[i] *= (cPersonal * GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
				nbhBestVec[i] *= (cNeighborhood * GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
				glbBestVec[i] *= (cGlobal * GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			}
			break;

		case updateRule::SWARM_UPDATERULE_LINEAR:
			// Multiply each position with the same random floating point number times a constant
			Gem::Common::multVecConst<double>(personalBestVec, cPersonal * GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			Gem::Common::multVecConst<double>(nbhBestVec, cNeighborhood * GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			Gem::Common::multVecConst<double>(glbBestVec, cGlobal * GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			break;

		default: {
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
	if (getNNeighborhoods() > 1) {
		Gem::Common::addVec<double>(velVec, glbBestVec);
	}

	// Prune the velocity vector so that we can
	// be sure it is inside of the allowed range
	pruneVelocity(velVec);

	// Add or subtract the velocity parameters to the individual's parameters, depending on
	// the number of stalls and the value of the repulsionThreshold_ variable. This allows
	// the algorithm to escape local optima, if repulsionThreshold_ is > 0.
	if (0 < repulsionThreshold_ && this->getStallCounter() >= repulsionThreshold_) {
		Gem::Common::subtractVec<double>(indVec, velVec); // repulsion -- walk away from best known individuals
	} else {
		Gem::Common::addVec<double>(indVec, velVec); // attraction - walk towards best known individuals
	}

	// Update the velocity individual
	velocity->assignValueVector<double>(velVec, activityMode::ACTIVEONLY);

	// Update the candidate solution
	ind->assignValueVector<double>(indVec, activityMode::ACTIVEONLY);
}

/******************************************************************************/
/**
 * Adjusts the velocity vector so that its parameters don't exceed the allowed value range.
 *
 * @param velVec the velocity vector to be adjusted
 */
void GBaseSwarm::pruneVelocity(std::vector<double> &velVec) {
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
	bool overflowFound = false;
	for (std::size_t i = 0; i < velVec.size(); i++) {
#ifdef DEBUG
		if(dblVelVecMax_[i] <= 0.) {
		   glogger
		   << "In GBaseSwarm::pruneVelocity(): Error!" << std::endl
         << "Found invalid max value: " << dblVelVecMax_[i] << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		if (Gem::Common::gfabs(velVec[i]) > dblVelVecMax_[i]) {
			overflowFound = true;
			currentPercentage = Gem::Common::gfabs(velVec[i]) / dblVelVecMax_[i];
			if (currentPercentage > maxPercentage) {
				maxPercentage = currentPercentage;
			}
		}
	}

	if (overflowFound) {
		// Scale all velocity entries by maxPercentage
		for (std::size_t i = 0; i < velVec.size(); i++) {
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
std::tuple<double, double> GBaseSwarm::findBests() {
	std::size_t bestLocalId = 0;
	std::tuple<double, double> bestLocalFitness = std::make_tuple(this->getWorstCase(), this->getWorstCase());
	std::tuple<double, double> bestIterationFitness = std::make_tuple(this->getWorstCase(), this->getWorstCase());

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
	if (inFirstIteration()) {
		GBaseSwarm::iterator it;
		for (it = this->begin(); it != this->end(); ++it) {
			updatePersonalBest(*it);
		}
	} else {
		for (it = this->begin(); it != this->end(); ++it) {
			updatePersonalBestIfBetter(*it);
		}
	}

	// Sort individuals in all neighborhoods according to their fitness
	for (std::size_t n = 0; n < nNeighborhoods_; n++) {
		// identify the first and last id of the individuals in the current neighborhood
		std::size_t firstCounter = getFirstNIPos(n);
		std::size_t lastCounter = getLastNIPos(n);

		// Only partially sort the arrays
		std::sort(
			this->begin() + firstCounter, this->begin() + lastCounter,
			[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				return x->minOnly_fitness() < y->minOnly_fitness();
			}
		);

		// Check whether the best individual of the neighborhood is better than
		// the best individual found so far in this neighborhood
		if (inFirstIteration()) {
			neighborhood_bests_.at(n) = (*(this->begin() + firstCounter))->clone<GParameterSet>();
		}
		else {
			if (this->isBetter(
				(*(this->begin() + firstCounter))->transformedFitness(), neighborhood_bests_.at(n)->transformedFitness()
			)
				) {
				(neighborhood_bests_.at(n))->GObject::load(*(this->begin() + firstCounter));
			}
		}
	}

	// Identify the best individuals among all neighborhood bests
	for (std::size_t n = 0; n < nNeighborhoods_; n++) {
		if (this->isBetter((neighborhood_bests_.at(n))->transformedFitness(),
								 std::get<G_TRANSFORMED_FITNESS>(bestLocalFitness))) {
			bestLocalId = n;
			bestLocalFitness = (neighborhood_bests_.at(n))->getFitnessTuple();
		}
	}

	// Compare the best neighborhood individual with the globally best individual and
	// update it, if necessary. Initialize it in the first generation.
	if (inFirstIteration()) {
		global_best_ = (neighborhood_bests_.at(bestLocalId))->clone<GParameterSet>();
	} else {
		if (this->isBetter(std::get<G_TRANSFORMED_FITNESS>(bestLocalFitness), global_best_->transformedFitness())) {
			global_best_->GObject::load(neighborhood_bests_.at(bestLocalId));
		}
	}

	// Identify the best fitness in the current iteration
	for (std::size_t i = 0; i < this->size(); i++) {
		if (this->isBetter(std::get<G_TRANSFORMED_FITNESS>(this->at(i)->getFitnessTuple()),
								 std::get<G_TRANSFORMED_FITNESS>(bestIterationFitness))) {
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

	if (currentSize == 0) {
		glogger
		<< "In GBaseSwarm::adjustPopulation() :" << std::endl
		<< "No individuals found in the population." << std::endl
		<< "You need to add at least one individual before" << std::endl
		<< "the call to optimize<>()" << std::endl
		<< GEXCEPTION;
	} else if (currentSize == 1) {
		// Fill up with random items to the number of neighborhoods
		for (std::size_t i = 1; i < nNeighborhoods_; i++) {
			this->push_back(this->front()->clone<GParameterSet>());
			this->back()->randomInit(activityMode::ACTIVEONLY);
		}

		// Fill in remaining items in each neighborhood. This will
		// also take care of the above case, where only one individual
		// has been added.
		fillUpNeighborhood1();
	} else if (currentSize == nNeighborhoods) {
		// Fill in remaining items in each neighborhood.
		fillUpNeighborhood1();
	} else if (currentSize == defaultPopSize) {
		// Update the number of individuals in each neighborhood
		for (std::size_t n = 0; n < nNeighborhoods_; n++) {
			nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
		}
	} else {
		if (currentSize < nNeighborhoods_) {
			// First fill up the neighborhoods, if required
			for (std::size_t m = 0; m < (nNeighborhoods_ - currentSize); m++) {
				this->push_back(this->front()->clone<GParameterSet>());
				this->back()->randomInit(activityMode::ACTIVEONLY);
			}

			// Now follow the procedure used for the "nNeighborhoods_" case
			fillUpNeighborhood1();
		} else if (currentSize > nNeighborhoods_ && currentSize < defaultPopSize) {
			// New procedure:
			// - Find out how many individuals exist in each neighborhood (Check: Has the neighborhood-id already been assigned here ?)
			// - For each neighborhood: add missing items to the end of vector
			// - Sort the vector according to neighborhoods
			// - Remove surplus items in each neighborhood
			// - In DEBUG mode: make sure each neighborhood is at the correct size

			// TODO: For now we simply resize the population to the number of neighborhoods,
			// then fill up again. This means that we loose some predefined values, which
			// is ugly and needs to be changed in later versions.
			this->resize(nNeighborhoods_);
			fillUpNeighborhood1();

			// TODO: This is catastrophic if work items didn't return in GBrokerSwarm,
			// as it uses adjustPopulation to fix the population.
			// MUST FIX
		} else { // currentSize > defaultPopsize
			// Update the number of individuals in each neighborhood
			for (std::size_t n = 0; n < nNeighborhoods_ - 1; n++) {
				nNeighborhoodMembers_[n] = defaultNNeighborhoodMembers_;
			}

			// Adjust the nNeighborhoodMembers_ array. The surplus items will
			// be assumed to belong to the last neighborhood, all other neighborhoods
			// have the default size.
			// TODO: This is bad, as adjustPopulation is used to adjust sizes also during an optimization run
			// Must remove worst items for each neighborhood individually. Also: Must make sure that. while some
			// neighborhoods might have too many, others might have too few entries. MUST FIX.
			nNeighborhoodMembers_[nNeighborhoods_ - 1] = defaultNNeighborhoodMembers_ + (currentSize - defaultPopSize);
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

	// TODO: Split into a fix-population function and a adjustPopulation function
	// Move adjustPopulation funcionality into the init() function (?)
	// Make sure neighborhood ids are set for new individuals (both in fixed populations and initially)
}

/******************************************************************************/
/**
 * Small helper function that helps to fill up a neighborhood, if there is just one entry in it.
 */
void GBaseSwarm::fillUpNeighborhood1() {
	// Do some error checking
	if (this->size() != nNeighborhoods_) {
		glogger
		<< "In GBaseSwarm::fillUpNeighborhood1():" << std::endl
		<< "Invalid size: " << this->size() << " Expected " << nNeighborhoods_ << std::endl
		<< GEXCEPTION;
	}

	if (defaultNNeighborhoodMembers_ == 1) return; // nothing to do

	// Starting with the last item, loop over all neighborhoods
	for (std::size_t i = 0; i < nNeighborhoods_; i++) {
		std::size_t n = nNeighborhoods_ - 1 - i; // Calculate the correct neighborhood

		// Insert the required number of clones after the existing individual
		for (std::size_t m = 1; m < defaultNNeighborhoodMembers_; m++) { // m stands for "missing"
			// Add a clone of the first individual in the neighborhood to the next position
			this->insert(this->begin() + n, (*(this->begin() + n))->clone<GParameterSet>());
			// Make sure it has a unique value, if requested
			if (randomFillUp_) {
#ifdef DEBUG
				if(!(*(this->begin()+n+1))) {
				   glogger
				   << "In GBaseSwarm::fillUpNeighborhood1():" << std::endl
               << "Found empty position " << n << std::endl
               << GEXCEPTION;
				}
#endif /* DEBUG */

				(*(this->begin() + n + 1))->randomInit(activityMode::ACTIVEONLY);
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
	if (velocityRangePercentage <= 0. || velocityRangePercentage > 1.) {
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
std::size_t GBaseSwarm::getCurrentNNeighborhoodMembers(const std::size_t &neighborhood) const {
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
 * Allows to specify the number of stalls as of which the algorithm switches to
 * repulsive mode. Set this value to 0 in order to disable repulsive mode.
 *
 * @param repulsionThreshold The threshold as of which the algorithm switches to repulsive mode
 */
void GBaseSwarm::setRepulsionThreshold(std::uint32_t repulsionThreshold) {
	repulsionThreshold_ = repulsionThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of stalls as of which the algorithm switches
 * to repulsive mode.
 *
 * @return The value of the repulsionThreshold_ variable
 */
std::uint32_t GBaseSwarm::getRepulsionThreshold() const {
	return repulsionThreshold_;
}

/******************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have equal value
 */
void GBaseSwarm::setNeighborhoodsEqualFillUp() {
	randomFillUp_ = false;
}

/******************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have a random value
 */
void GBaseSwarm::setNeighborhoodsRandomFillUp(bool randomFillUp) {
	randomFillUp_ = randomFillUp;
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
	if (GOptimizationAlgorithmT<GParameterSet>::modify_GUnitTests()) result = true;

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

} /* namespace Geneva */
} /* namespace Gem */

