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

/***********************************************************************************/
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

/***********************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GSwarm object
 */
GSwarm::GSwarm(const GSwarm& cp)
	: GOptimizationAlgorithm(cp)
	, infoFunction_(cp.infoFunction_)
{
	setPopulationSize(cp.nNeighborhoods_, cp.nNeighborhoodMembers_);

	global_best_ = (cp.global_best_)->clone_bptr_cast<GIndividual>();
	std::vector<boost::shared_ptr<GIndividual> >::const_iterator cit;
	for(cit=cp.local_bests_.begin(); cit!=cp.local_bests_.end(); ++cit) {
		local_bests_.push_back(cit->clone_bptr_cast<GIndividual>());
	}
}

/***********************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GSwarm::~GSwarm()
{ /* nothing */ }

/***********************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp Another GSwarm object
 * @return A constant reference to this object
 */
const GSwarm& GSwarm::operator=(const GSwarm& cp) {
	GSwarm::load(&cp);
	return *this;
}

/***********************************************************************************/
/**
 * Loads the data of another GSwarm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GSwarm object, camouflaged as a GObject
 */
void GSwarm::load(const GObject * cp)
{
	const GSwarm *p_load = this->conversion_cast(cp,this);

	// First load the parent class'es data ...
	GOptimizationAlgorithm::load(cp);

	// ... and then our own data
	infoFunction_ = p_load->infoFunction_;

	nNeighborhoods_ = p_load->nNeighborhoods_;
	nNeighborhoodMembers_ = p_load->nNeighborhoodMembers_;

	global_best_->load((p_load->global_best_).get());
	copySmartPointerVector(p_load->local_bests_, local_bests_);
}

/***********************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object, camouflaged as a pointer to a GObject
 */
GObject *GSwarm::clone() const  {
	return new GSwarm(*this);
}

/***********************************************************************************/
/**
 * Checks for equality with another GSwarm object
 *
 * @param  cp A constant reference to another GSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarm::operator==(const GSwarm& cp) const {
	return GSwarm::isEqualTo(cp,  boost::logic::indeterminate);
}

/***********************************************************************************/
/**
 * Checks for inequality with another GSwarm object
 *
 * @param  cp A constant reference to another GSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarm::operator!=(const GSwarm& cp) const {
	return !GSwarm::isEqualTo(cp,  boost::logic::indeterminate);
}

/***********************************************************************************/
/**
 * Checks for equality with another GSwarm object.
 *
 * @param  cp A constant reference to another GSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarm::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
	using namespace Gem::Util;

	// Check that we are indeed dealing with a GIndividual reference
	const GSwarm *p_load = GObject::conversion_cast(&cp,  this);

	// First take care of our parent class
	if(!GOptimizationAlgorithm::isEqualTo( *p_load, expected)) return  false;

	// Then we take care of the local data
	if(checkForInequality("GSwarm", nNeighborhoods_, p_load->nNeighborhoods_,"nNeighborhoods_", "p_load->nNeighborhoods_", expected)) return false;
	if(checkForInequality("GSwarm", nNeighborhoodMembers_, p_load->nNeighborhoodMembers_,"nNeighborhoodMembers_", "p_load->nNeighborhoodMembers_", expected)) return false;
	if(!global_best_->isEqualTo(*(p_load->global_best_), expected)) return false;
	if(checkForInequality("GSwarm", local_bests_, p_load->local_bests_,"local_bests_", "p_load->local_bests_", expected)) return false;

	return true;
}

/***********************************************************************************/
/**
 * Checks for similarity with another GSwarm object.
 *
 * @param  cp A constant reference to another GSwarm object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GSwarm::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
	using namespace Gem::Util;

	// Check that we are indeed dealing with a GIndividual reference
	const GSwarm *p_load = GObject::conversion_cast(&cp,  this);

	// First take care of our parent class
	if(!GOptimizationAlgorithm::isSimilarTo(*p_load, limit, expected)) return  false;

	// Then we take care of the local data
	if(checkForDissimilarity("GSwarm", nNeighborhoods_, p_load->nNeighborhoods_, limit, "nNeighborhoods_", "p_load->nNeighborhoods_", expected)) return false;
	if(checkForDissimilarity("GSwarm", nNeighborhoodMembers_, p_load->nNeighborhoodMembers_, limit, "nNeighborhoodMembers_", "p_load->nNeighborhoodMembers_", expected)) return false;
	if(!global_best_->isSimilarTo(*(p_load->global_best_), limit, expected)) return false;
	if(checkForDissimilarity("GSwarm", local_bests_, p_load->local_bests_, limit, "local_bests_", "p_load->local_bests_", expected)) return false;

	return true;
}

/***********************************************************************************/
/**
 * Sets the individual's personality types to EA
 */
void GSwarm::setIndividualPersonalities() {
	GSwarm::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) (*it)->setPersonality(SWARM);
}

/***********************************************************************************/
/**
 * Saves the state of the class to disc. The function adds the current generation
 * and the fitness to the base name. We do not save the entire population, but only
 * the best individuals, as these contain the "real" information. Note that no real
 * copying of the individual's data takes place here, as we are dealing with
 * boost::shared_ptr objects.
 */
void GSwarm::saveCheckpoint() const {
	/* nothing */
}

/***********************************************************************************/
/**
 * Loads the state of the class from disc. We do not load the entire population,
 * but only the best individuals of a former optimization run, as these contain the
 * "real" information.
 */
void GSwarm::loadCheckpoint(const std::string& cpFile) {
	/* nothing */
}

/***********************************************************************************/
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

/***********************************************************************************/
/**
 * The user can specify what information should be emitted in a call-back function
 * that is registered in the setup phase. This functionality is based on boost::function .
 *
 * @param infoFunction A Boost.function object allowing the emission of information
 */
void GSwarm::registerInfoFunction(boost::function<void (const infoMode&, GSwarm * const)> infoFunction) {
	infoFunction_ = infoFunction;
}

/***********************************************************************************/
/**
 * This function implements the logic that constitutes evolutionary algorithms. The
 * function is called by GOptimizationAlgorithm for each cycle of the optimization,
 *
 * @return The value of the best individual found
 */
double GSwarm::cycleLogic() {
	double bestFitness = 0.;
	return bestFitness;
}


/***********************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithm::optimize(), before the
 * actual optimization cycle starts.
 */
void GSwarm::init() {
	// To be performed before any other action
	GOptimizationAlgorithm::init();

	// Attach the relevant information to the individual's personalities
	updatePersonalities();
}

/***********************************************************************************/
/**
 * Does any necessary finalization work
 */
void GSwarm::finalize() {
	// Last action
	GOptimizationAlgorithm::finalize();
}

/***********************************************************************************/
/**
 * Sets the local multiplier used when calculating velocities to a fixed value
 * in all individuals. This function results in a fixed factor.
 *
 * @param c1 The multiplication factor for the difference between a local particle and and the local best
 */
void GSwarm::setCLocal(const double& c1) {

}

/***********************************************************************************/
/**
 * Sets the local multiplier of each individual randomly within a given range
 */
void GSwarm::setCLocal(const double&, const double&) {

}

/***********************************************************************************/
/**
 * Sets the global multiplier used when calculating velocities to a fixed value in
 * all individuals
 */
void GSwarm::setCGlobal(const double&) {

}

/***********************************************************************************/
/**
 * Sets the global multiplier of each individual randomly within a given range
 */
void GSwarm::setCGlobal(const double&, const double&) {

}

/***********************************************************************************/
/**
 * Sets the velocity multiplier to a fixed value for each individual
 */
void GSwarm::setCVelocity(const double&) {

}

/***********************************************************************************/
/**
 * Sets the velocity multiplier to a random value separately for each individual
 */
void GSwarm::setCVelocity(const double&, const double&) {

}

/***********************************************************************************/
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

/***********************************************************************************/
/**
 * Retrieves the number of neighborhoods
 *
 * @return The number of neighborhoods in the population
 */
std::size_t GSwarm::getNNeighborhoods() const {
	return nNeighborhoodMembers_;
}

/***********************************************************************************/
/**
 * Retrieves the number of individuals in each neighborhood
 *
 * @return The number of individuals in each neighborhood
 */
std::size_t GSwarm::getNNeighborhoodMembers() const {
	return nNeighborhoodMembers_;
}

/***********************************************************************************/
/**
 * Retrieves the local multiplier
 */
double GSwarm::getCLocal() const {

}

/***********************************************************************************/
/**
 * Retrieves the global multiplier
 */
double GSwarm::getCGlobal() const {

}

/***********************************************************************************/
/**
 * Retrieves the velocity multiplier
 */
double GSwarm::getCVelocity() const {

}

/***********************************************************************************/
/**
 * Helper function that updates the personality information
 */
void GSwarm::updatePersonalities() {
	// Check that all individuals have the same geometry (i.e. same number of doubles)

	// Initialize the double vectors in the personality traits accordingly

	// Set the individual's positions. This needs to be done only once, as individuals
	// do not change position in a swarm algorithm.

	// Set the local multipliers according to the user's instructions

	// Loop over all individuals
	GSwarm::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		// Extract all double parameters from the individuals
		std::vector<boost::shared_ptr<GDouble> > gd_ptr_vec;
		it->attachViewTo(gd_vec);

		std::vector<boost::shared_ptr<GDoubleCollection> > gdc_ptr_vec;
		it->attachViewTo(gdc_ptr_vec);

		std::vector<boost::shared_ptr<GBoundedDouble> > gbd_ptr_vec;
		it->attachViewTo(gdc_ptr_vec);

		std::vector<boost::shared_ptr<GBoundedDoubleCollection> > gbdc_ptr_vec;
		it->attachViewTo(gbdc_ptr_vec);
	}
}

/***********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
