/**
 * @file
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
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSwarmAlgorithm) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. All work is delegated to another constructor.
 */
GSwarmAlgorithm::GSwarmAlgorithm()
	: GSwarmAlgorithm(DEFAULTNNEIGHBORHOODS, DEFAULTNNEIGHBORHOODMEMBERS)
{ /* nothing */ }

/******************************************************************************/
/**
 * This constructor sets the number of neighborhoods and the number of individuals in them. Note that there
 * is no public default constructor, as it is only needed for de-serialization purposes.
 *
 * @param nNeighborhoods The desired number of neighborhoods (hardwired to >= 1)
 * @oaram nNeighborhoodMembers The default number of individuals in each neighborhood (hardwired to >= 2)
 */
GSwarmAlgorithm::GSwarmAlgorithm(
	const std::size_t &nNeighborhoods
	, const std::size_t &defaultNNeighborhoodMembers
)
	: G_OptimizationAlgorithm_Base()
	  , m_n_neighborhoods((nNeighborhoods>=1) ? nNeighborhoods : 1)
	  , m_default_n_neighborhood_members((defaultNNeighborhoodMembers >= 2) ? defaultNNeighborhoodMembers: 2)
{
	G_OptimizationAlgorithm_Base::setDefaultPopulationSize(m_n_neighborhoods * m_default_n_neighborhood_members);
}

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GSwarmAlgorithm object
 */
GSwarmAlgorithm::GSwarmAlgorithm(const GSwarmAlgorithm &cp)
	: G_OptimizationAlgorithm_Base(cp)
	  , m_n_neighborhoods(cp.m_n_neighborhoods)
	  , m_default_n_neighborhood_members(cp.m_default_n_neighborhood_members)
	  , m_n_neighborhood_members_cnt(cp.m_n_neighborhood_members_cnt)
	  , m_global_best_ptr((cp.afterFirstIteration()) ? (cp.m_global_best_ptr)->clone<GParameterSet>() : std::shared_ptr<GParameterSet>())
	  , m_neighborhood_bests_cnt(m_n_neighborhoods) // We copy the smart pointers over later
	  , m_c_personal(cp.m_c_personal)
	  , m_c_neighborhood(cp.m_c_neighborhood)
	  , m_c_global(cp.m_c_global)
	  , m_c_velocity(cp.m_c_velocity)
	  , m_update_rule(cp.m_update_rule)
	  , m_random_fill_up(cp.m_random_fill_up)
	  , m_repulsion_threshold(cp.m_repulsion_threshold)
	  , m_dbl_lower_parameter_boundaries_cnt(cp.m_dbl_lower_parameter_boundaries_cnt)
	  , m_dbl_upper_parameter_boundaries_cnt(cp.m_dbl_upper_parameter_boundaries_cnt)
	  , m_dbl_vel_max_cnt(cp.m_dbl_vel_max_cnt)
	  , m_velocity_range_percentage(cp.m_velocity_range_percentage)
{
	// Note that this setting might differ from nCPIndividuals, as it is not guaranteed
	// that cp has, at the time of copying, all individuals present in each neighborhood.
	// Differences might e.g. occur if not all individuals return from their remote
	// evaluation. adjustPopulation will take care to resize the population appropriately
	// inside of the "optimize()" call.
	G_OptimizationAlgorithm_Base::setDefaultPopulationSize(m_n_neighborhoods * m_default_n_neighborhood_members);

	// Clone cp's best individuals in each neighborhood
	if (cp.afterFirstIteration()) {
		for (std::size_t i = 0; i < m_n_neighborhoods; i++) {
			m_neighborhood_bests_cnt[i] = cp.m_neighborhood_bests_cnt[i]->clone<GParameterSet>();
		}
	}

	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm.
 *
 * @return The type of optimization algorithm
 */
std::string GSwarmAlgorithm::getAlgorithmPersonalityType_() const {
	return "PERSONALITY_SWARM";
}

/******************************************************************************/
/**
 * Loads the data of another GSwarmAlgorithm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GSwarmAlgorithm object, camouflaged as a GObject
 */
void GSwarmAlgorithm::load_(const GObject *cp) {
	// Check that we are dealing with a GSwarmAlgorithm reference independent of this object and convert the pointer
	const GSwarmAlgorithm *p_load = Gem::Common::g_convert_and_compare<GObject, GSwarmAlgorithm >(cp, this);

	// First load the parent class'es data.
	// This will also take care of copying all individuals.
	G_OptimizationAlgorithm_Base::load_(cp);

	// ... and then our own data
	m_default_n_neighborhood_members = p_load->m_default_n_neighborhood_members;
	m_c_personal = p_load->m_c_personal;
	m_c_neighborhood = p_load->m_c_neighborhood;
	m_c_global = p_load->m_c_global;
	m_c_velocity = p_load->m_c_velocity;
	m_update_rule = p_load->m_update_rule;
	m_random_fill_up = p_load->m_random_fill_up;
	m_repulsion_threshold = p_load->m_repulsion_threshold;

	m_dbl_lower_parameter_boundaries_cnt = p_load->m_dbl_lower_parameter_boundaries_cnt;
	m_dbl_upper_parameter_boundaries_cnt = p_load->m_dbl_upper_parameter_boundaries_cnt;
	m_dbl_vel_max_cnt = p_load->m_dbl_vel_max_cnt;

	m_velocity_range_percentage = p_load->m_velocity_range_percentage;

	// We start from scratch if the number of neighborhoods or the alleged number of members in them differ
	if (m_n_neighborhoods != p_load->m_n_neighborhoods ||
		 not nNeighborhoodMembersEqual(m_n_neighborhood_members_cnt, p_load->m_n_neighborhood_members_cnt)) {
		m_n_neighborhoods = p_load->m_n_neighborhoods;

		m_n_neighborhood_members_cnt.clear();
		m_neighborhood_bests_cnt.clear();

		m_n_neighborhood_members_cnt.resize(m_n_neighborhoods);
		m_neighborhood_bests_cnt.resize(m_n_neighborhoods);

		// Copy the neighborhood bests and number of neighborhood members over
		for (std::size_t i = 0; i < m_n_neighborhoods; i++) {
			m_n_neighborhood_members_cnt[i] = p_load->m_n_neighborhood_members_cnt[i];
			// The following only makes sense if this is not the first iteration. Note that
			// getIteration will return the "foreign" GSwarmAlgorithm object's iteration, as it has
			// already been copied.
			if (afterFirstIteration()) {
				m_neighborhood_bests_cnt[i] = p_load->m_neighborhood_bests_cnt[i]->clone<GParameterSet>();
			}
			// we do not need to reset the m_neighborhood_bests_cnt, as that array has just been created
		}
	}
	else { // We now assume that we can just load neighborhood bests in each position.
		// Copying only makes sense if the foreign GSwarmAlgorithm object's iteration is larger
		// than the iteration offset. Note that getIteration() will return the foreign iteration,
		// as that value has already been copied.
		if (afterFirstIteration()) {
			for (std::size_t i = 0; i < m_n_neighborhoods; i++) {
				// We might be in a situation where the std::shared_ptr which usually
				// holds the neighborhood bests has not yet been initialized
				if (m_neighborhood_bests_cnt[i]) {
					m_neighborhood_bests_cnt[i]->GObject::load(p_load->m_neighborhood_bests_cnt[i]);
				} else {
					m_neighborhood_bests_cnt[i] = p_load->m_neighborhood_bests_cnt[i]->clone<GParameterSet>();
				}
			}
		} else {
			for (std::size_t i = 0; i < m_n_neighborhoods; i++) {
				m_neighborhood_bests_cnt[i].reset();
			}
		}
	}

	// Copy the global best over
	if (p_load->afterFirstIteration()) { // cp has a global best, we don't
		if (m_global_best_ptr) { // If we already have a global best, just load the other objects global best
			m_global_best_ptr->GObject::load(p_load->m_global_best_ptr);
		} else {
			m_global_best_ptr = p_load->GObject::clone<GParameterSet>();
		}
	}
	else if (p_load->inFirstIteration()) { // cp does not have a global best
		m_global_best_ptr.reset(); // empty the smart pointer
	}
	// else {} // We do not need to do anything if both iterations are 0 as there is no global best at all
}

/******************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GSwarmAlgorithm::clone_() const {
	return new GSwarmAlgorithm(*this);
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
void GSwarmAlgorithm::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBooleanAdaptor reference independent of this object and convert the pointer
	const GSwarmAlgorithm *p_load = Gem::Common::g_convert_and_compare<GObject, GSwarmAlgorithm >(cp, this);

	GToken token("GSwarmAlgorithm", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(m_n_neighborhoods, p_load->m_n_neighborhoods), token);
	compare_t(IDENTITY(m_default_n_neighborhood_members, p_load->m_default_n_neighborhood_members), token);
	compare_t(IDENTITY(m_global_best_ptr, p_load->m_global_best_ptr), token);
	compare_t(IDENTITY(m_c_personal, p_load->m_c_personal), token);
	compare_t(IDENTITY(m_c_neighborhood, p_load->m_c_neighborhood), token);
	compare_t(IDENTITY(m_c_global, p_load->m_c_global), token);
	compare_t(IDENTITY(m_c_velocity, p_load->m_c_velocity), token);
	compare_t(IDENTITY(m_update_rule, p_load->m_update_rule), token);
	compare_t(IDENTITY(m_random_fill_up, p_load->m_random_fill_up), token);
	compare_t(IDENTITY(m_repulsion_threshold, p_load->m_repulsion_threshold), token);
	compare_t(IDENTITY(m_dbl_lower_parameter_boundaries_cnt, p_load->m_dbl_lower_parameter_boundaries_cnt), token);
	compare_t(IDENTITY(m_dbl_upper_parameter_boundaries_cnt, p_load->m_dbl_upper_parameter_boundaries_cnt), token);
	compare_t(IDENTITY(m_dbl_vel_max_cnt, p_load->m_dbl_vel_max_cnt), token);
	compare_t(IDENTITY(m_velocity_range_percentage, p_load->m_velocity_range_percentage), token);

	// The next checks only makes sense if the number of neighborhoods are equal
	if (m_n_neighborhoods == p_load->m_n_neighborhoods) {
		compare_t(IDENTITY(m_n_neighborhood_members_cnt, p_load->m_n_neighborhood_members_cnt), token);
		// No neighborhood bests have been assigned yet in iteration 0
		if (afterFirstIteration()) {
			compare_t(IDENTITY(m_neighborhood_bests_cnt, p_load->m_neighborhood_bests_cnt), token);
		}
	}

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GSwarmAlgorithm::resetToOptimizationStart_() {
	m_n_neighborhood_members_cnt = std::vector<std::size_t>(m_n_neighborhoods, 0); // The current number of individuals belonging to each neighborhood

	m_global_best_ptr.reset(); // The globally best individual

	m_neighborhood_bests_cnt = std::vector<std::shared_ptr<GParameterSet>>(m_n_neighborhoods); // The collection of best individuals from each neighborhood
	m_velocities_cnt = std::vector<std::shared_ptr<GParameterSet>>(); // Holds velocities, as calculated in the previous iteration

	m_dbl_lower_parameter_boundaries_cnt.clear(); // Holds lower boundaries of double parameters
	m_dbl_upper_parameter_boundaries_cnt.clear(); // Holds upper boundaries of double parameters
	m_dbl_vel_max_cnt.clear(); // Holds the maximum allowed values of double-type velocities

	m_last_iteration_individuals_cnt.clear(); // A temporary copy of the last iteration's individuals

	// There is no more work to be done here, so we simply call the
	// function of the parent class
	G_OptimizationAlgorithm_Base::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSwarmAlgorithm::name_() const {
	return std::string("GSwarmAlgorithm");
}

/******************************************************************************/
/**
 * Sets the number of neighborhoods and the default number of members in them. All work is done inside of
 * the adjustPopulation function, inside of the G_OptimizationAlgorithm_Base::optimize() function.
 *
 * @param nNeighborhoods The number of neighborhoods
 * @param defaultNNeighborhoodMembers The default number of individuals in each neighborhood
 */
void GSwarmAlgorithm::setSwarmSizes(
	std::size_t nNeighborhoods
	, std::size_t defaultNNeighborhoodMembers
) {
	// Enforce useful settings
	if (nNeighborhoods == 0) {
		glogger
			<< "In GSwarmAlgorithm::setSwarmSizes(): Warning!" << std::endl
			<< "Requested number of neighborhoods is 0. Setting to 1." << std::endl
			<< GWARNING;
	}

	if (defaultNNeighborhoodMembers <= 1) {
		glogger
			<< "In GSwarmAlgorithm::setSwarmSizes(): Warning!" << std::endl
			<< "Requested number of members in each neighborhood is too small. Setting to 2." << std::endl
			<< GWARNING;
	}

	m_n_neighborhoods = (nNeighborhoods >= 1) ? nNeighborhoods : 1;
	m_default_n_neighborhood_members = (defaultNNeighborhoodMembers >= 2) ? defaultNNeighborhoodMembers : 2;

	// Update our parent class'es values
	G_OptimizationAlgorithm_Base::setDefaultPopulationSize(m_n_neighborhoods * m_default_n_neighborhood_members);
}

/******************************************************************************/
/**
 * Helper function that checks the content of two nNeighborhoodMembers_ arrays.
 *
 * @param one The first array used for the check
 * @param two The second array used for the check
 * @return A boolean indicating whether both arrays are equal
 */
bool GSwarmAlgorithm::nNeighborhoodMembersEqual(
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
std::size_t GSwarmAlgorithm::getFirstNIPos(const std::size_t &neighborhood) const {
	return getFirstNIPosVec(neighborhood, m_n_neighborhood_members_cnt);
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
std::size_t GSwarmAlgorithm::getFirstNIPosVec(
	const std::size_t &neighborhood
	, const std::vector<std::size_t> &vec
) const {
#ifdef DEBUG
	if(neighborhood >= m_n_neighborhoods) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::getFirstNIPosVec():" << std::endl
				<< "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
				<< "The number of neighborhoods is " << m_n_neighborhoods << "," << std::endl
				<< "hence the maximum allowed value of the id is " << m_n_neighborhoods-1 << "." << std::endl
		);
	}

	// TODO: Add check for array sizes
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
std::size_t GSwarmAlgorithm::getLastNIPos(const std::size_t &neighborhood) const {
#ifdef DEBUG
	if(neighborhood >= m_n_neighborhoods) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::getLastNIPos():" << std::endl
				<< "Received id " << neighborhood << " of a neighborhood which does not exist." << std::endl
				<< "The number of neighborhoods is " << m_n_neighborhoods << " ." << std::endl
				<< "hence the maximum allowed value of the id is " << m_n_neighborhoods-1 << "." << std::endl
		);
	}
#endif

	return getFirstNIPos(neighborhood) + m_n_neighborhood_members_cnt[neighborhood];
}

/******************************************************************************/
/**
 * Updates the personal best of an individual
 *
 * @param p A pointer to the GParameterSet object to be updated
 */
void GSwarmAlgorithm::updatePersonalBest(
	std::shared_ptr<GParameterSet> ind_ptr
) {
#ifdef DEBUG
	if(not ind_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updatePersonalBest():" << std::endl
				<< "Got empty ind_ptr" << std::endl
		);
	}

	if(ind_ptr->is_due_for_processing() || ind_ptr->has_errors()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updatePersonalBest():" << std::endl
				<< "ind_ptr is unprocessed or has errors: " << std::endl
				<< "is_due_for_processing() == " << ind_ptr->is_due_for_processing() << ", has_errors() == " << ind_ptr->has_errors() << std::endl
		);
	}
#endif /* DEBUG */

	// TODO: Is this correct ? Updates the personal best of itself ?!??
	ind_ptr->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->registerPersonalBest(ind_ptr);
}

/******************************************************************************/
/**
 * Updates the personal best of an individual, if a better solution was found
 *
 * @param p A pointer to the GParameterSet object to be updated
 */
void GSwarmAlgorithm::updatePersonalBestIfBetter(
	std::shared_ptr<GParameterSet> ind_ptr
) {
#ifdef DEBUG
	if(not ind_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updatePersonalBestIfBetter():" << std::endl
				<< "Got empty ind_ptr" << std::endl
		);
	}

	if(ind_ptr->is_due_for_processing() || ind_ptr->has_errors()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updatePersonalBestIfBetter(): Error!" << std::endl
				<< "dirty flag of individual is set." << std::endl
		);
	}
#endif /* DEBUG */

	auto m = this->at(0)->getMaxMode(); // We assume that the maxMode is the same for all individuals
	if(isBetter(
		std::get<G_TRANSFORMED_FITNESS>(ind_ptr->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->getPersonalBestQuality())
		, ind_ptr->transformed_fitness(0)
		, m
	)) {
		ind_ptr->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->registerPersonalBest(ind_ptr);
	}
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GSwarmAlgorithm::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);

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
std::string GSwarmAlgorithm::getAlgorithmName_() const {
	return std::string("Swarm Algorithm");
}

/******************************************************************************/
/**
 * This function does some preparatory work and tagging required by swarm algorithms. It is called
 * from within G_OptimizationAlgorithm_Base::optimize(), immediately before the actual optimization cycle starts.
 */
void GSwarmAlgorithm::init() {
	// To be performed before any other action
	G_OptimizationAlgorithm_Base::init();

	// Extract the boundaries of all parameters
	this->at(0)->boundaries(m_dbl_lower_parameter_boundaries_cnt, m_dbl_upper_parameter_boundaries_cnt, activityMode::ACTIVEONLY);

#ifdef DEBUG
	// Size matters!
	if(m_dbl_lower_parameter_boundaries_cnt.size() != m_dbl_upper_parameter_boundaries_cnt.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::init(): Error!" << std::endl
				<< "Found invalid sizes: "
				<< m_dbl_lower_parameter_boundaries_cnt.size() << " / " << m_dbl_upper_parameter_boundaries_cnt.size() << std::endl
		);
	}
#endif /* DEBUG */

	// Calculate the allowed maximum values of the velocities
	double l = getVelocityRangePercentage();
	m_dbl_vel_max_cnt.clear();
	for (std::size_t i = 0; i < m_dbl_lower_parameter_boundaries_cnt.size(); i++) {
		m_dbl_vel_max_cnt.push_back(l * (m_dbl_upper_parameter_boundaries_cnt[i] - m_dbl_lower_parameter_boundaries_cnt[i]));
	}

	// Make sure the m_velocities_cnt vector is really empty
	m_velocities_cnt.clear();

	// Create copies of our individuals in the m_velocities_cnt vector.
	std::size_t pos = 0;
	for(const auto& ind_ptr: *this) {
#ifdef DEBUG
		if(not ind_ptr) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::init(): Error!" << std::endl
					<< "Found empty std::shared_ptr in position " << pos << std::endl
			);
		}
#endif /* DEBUG */

		// Create a copy of the current individual. Note that, if you happen
		// to have assigned anything else than a GParameterSet derivative to
		// the swarm, then the following line will throw in DEBUG mode or return
		// undefined results in RELEASE mode
		std::shared_ptr<GParameterSet> p(ind_ptr->clone<GParameterSet>());

		// Extract the parameter vector
		std::vector<double> velVec;
		p->streamline(velVec, activityMode::ACTIVEONLY);

#ifdef DEBUG
		// Check that the number of parameters equals those in the velocity boundaries
		if(velVec.size() != m_dbl_lower_parameter_boundaries_cnt.size() || velVec.size() != m_dbl_vel_max_cnt.size()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::init(): Error! (2)" << std::endl
					<< "Found invalid sizes: " << velVec.size()
					<< " / " << m_dbl_lower_parameter_boundaries_cnt.size() << std::endl
					<< " / " << m_dbl_vel_max_cnt.size() << std::endl
			);
		}
#endif /* DEBUG */

		// Randomly initialize the velocities
		for (std::size_t i = 0; i < velVec.size(); i++) {
			double range = m_dbl_vel_max_cnt[i];
			velVec[i] =
				G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(-range,range));
		}

		// Load the array into the velocity object
		p->assignValueVector<double>(velVec, activityMode::ACTIVEONLY);
		p->mark_as_due_for_processing(); // Catch cases where a value is calculated for the velocity individual

		// Add the initialized velocity to the array.
		m_velocities_cnt.push_back(p);

		pos++;
	}

	// Make sure m_neighborhood_bests_cnt has the correct size
	// It will only hold empty smart pointers. However, new ones
	// will be assigned in findBests()
	m_neighborhood_bests_cnt.resize(m_n_neighborhoods);

	// Make sure the m_n_neighborhood_members_cnt vector has the correct size
	m_n_neighborhood_members_cnt.resize(m_n_neighborhoods, m_default_n_neighborhood_members);
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GSwarmAlgorithm::finalize() {
	// Remove remaining velocity individuals. The std::shared_ptr<GParameterSet>s
	// will take care of deleting the GParameterSet objects.
	m_velocities_cnt.clear();

	// Last action
	G_OptimizationAlgorithm_Base::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr <GPersonalityTraits> GSwarmAlgorithm::getPersonalityTraits_() const {
	return std::shared_ptr<GSwarmAlgorithm_PersonalityTraits>(new GSwarmAlgorithm_PersonalityTraits());
}

/******************************************************************************/
/**
 * Gives individuals an opportunity to update their internal structures. Currently
 * nothing -- might search in the vicinity of the best known solution or run a small
 * EA.
 */
void GSwarmAlgorithm::actOnStalls_() {
	/* nothing */
}

/******************************************************************************/
/**
 * This function implements the logic that constitutes each cycle of a swarm algorithm. The
 * function is called by G_OptimizationAlgorithm_Base::optimize() for each iteration of
 * the optimization,
 *
 * @return The value of the best individual found
 */
std::tuple<double, double> GSwarmAlgorithm::cycleLogic_() {
	std::tuple<double, double> bestIndividualFitness;

	// First update the positions and neighborhood ids
	updatePositions();

	// Now update each individual's fitness
	runFitnessCalculation_();

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
 * GSwarmAlgorithm.
 *
 * TODO: Change name to fixAfterJobSubmission ?
 */
void GSwarmAlgorithm::adjustNeighborhoods() {
	std::size_t firstNIPos; // Will hold the expected first position of a neighborhood

#ifdef DEBUG
	// Check that m_last_iteration_individuals_cnt has the desired size in iterations other than the first
	if(afterFirstIteration() && m_last_iteration_individuals_cnt.size() != m_default_n_neighborhood_members*m_n_neighborhoods) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::adjustNeighborhoods(): Error!" << std::endl
				<< "m_last_iteration_individuals_cnt has incorrect size! Expected" << std::endl
				<< "m_default_n_neighborhood_members*m_n_neighborhoods = " << m_default_n_neighborhood_members*m_n_neighborhoods << std::endl
				<< "but found " << m_last_iteration_individuals_cnt.size() << std::endl
		);
	}
#endif /* DEBUG */

	// Add missing items to neighborhoods that are too small. We use stored copies from the
	// last iteration to fill in the missing items, or add random items in the first iteration.
	// Neighborhoods with too many items are pruned. findBests() has sorted each neighborhood
	// according to its fitness, so we know that the best items are in the front position of each
	// neighborhood. We thus simply remove items at the end of neighborhoods that are too large.
	for (std::size_t n = 0; n < m_n_neighborhoods; n++) { // Loop over all neighborhoods
		// Calculate the desired position of our own first individual in this neighborhood
		// As we start with the first neighborhood and add or remove surplus or missing items,
		// getFirstNIPos() will return a valid position.
		firstNIPos = getFirstNIPos(n);

		if (m_n_neighborhood_members_cnt[n] == m_default_n_neighborhood_members) {
			continue;
		} else if (m_n_neighborhood_members_cnt[n] > m_default_n_neighborhood_members) { // Remove surplus items from the end of the neighborhood
			// Find out, how many surplus items there are
			std::size_t nSurplus = m_n_neighborhood_members_cnt[n] - m_default_n_neighborhood_members;

			// Remove nSurplus items from the position (n+1)*m_default_n_neighborhood_members
			m_data_cnt.erase(
				m_data_cnt.begin() + (n + 1) * m_default_n_neighborhood_members
				, m_data_cnt.begin() + ((n + 1) * m_default_n_neighborhood_members + nSurplus)
			);
		} else { // m_n_neighborhood_members_cnt[n] < m_default_n_neighborhood_members
			// TODO: Deal with cases where no items of a given neighborhood have returned
			// The number of missing items
			std::size_t nMissing = m_default_n_neighborhood_members - m_n_neighborhood_members_cnt[n];

			if (afterFirstIteration()) { // The most likely case
				// Copy the best items of this neighborhood over from the m_last_iteration_individuals_cnt vector.
				// Each neighborhood there should have been sorted according to the individuals
				// fitness, with the best individuals in the front of each neighborhood.
				for (std::size_t i = 0; i < nMissing; i++) {
					m_data_cnt.insert(m_data_cnt.begin() + firstNIPos, *(m_last_iteration_individuals_cnt.begin() + firstNIPos + i));
				}
			} else { // first iteration
#ifdef DEBUG
				// At least one individual must have returned.
				if(this->empty()) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In GSwarmAlgorithm::adjustNeighborhoods(): Error!" << std::endl
							<< "No items found in the population. Cannot fix." << std::endl
					);
				}
#endif

				// Fill up with random items.
				for (std::size_t nM = 0; nM < nMissing; nM++) {
					// Insert a clone of the first individual of the collection
					m_data_cnt.insert(m_data_cnt.begin() + firstNIPos, (this->front())->clone<GParameterSet>());

					// Randomly initialize the item and prevent position updates
					(*(m_data_cnt.begin() + firstNIPos))->randomInit(activityMode::ACTIVEONLY);
					(*(m_data_cnt.begin() + firstNIPos))->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->setNoPositionUpdate();

					// Set the neighborhood as required
					(*(m_data_cnt.begin() + firstNIPos))->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->setNeighborhood(n);
				}
			}
		}

		// Finally adjust the number of entries in this neighborhood
		m_n_neighborhood_members_cnt[n] = m_default_n_neighborhood_members;
	}

#ifdef DEBUG
	// Check that the population has the expected size
	if(this->size() != m_n_neighborhoods*m_default_n_neighborhood_members) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::adjustNeighborhoods(): Error!" << std::endl
				<< "The population has an incorrect size of " << this->size() << ", expected " << m_n_neighborhoods*m_default_n_neighborhood_members << std::endl
		);
	}
#endif

	m_last_iteration_individuals_cnt.clear(); // Get rid of the copies
}


/******************************************************************************/
/**
 * Checks whether each neighborhood has the default size
 *
 * @return A boolean which indicates whether all neighborhoods have the default size
 */
bool GSwarmAlgorithm::neighborhoodsHaveNominalValues() const {
	for (std::size_t n = 0; n < m_n_neighborhoods; n++) {
		if (m_n_neighborhood_members_cnt[n] == m_default_n_neighborhood_members) return false;
	}
	return true;
}

/******************************************************************************/
/**
 * Triggers an update of all individual's positions. Also makes sure each
 * individual has the correct neighborhood id. Creates a copy of the last iteration's
 * individuals, if this is not the first iteration, then performs the standard position
 * update using GSwam::updatePositions(). We use the old individuals to fill in missing
 * returns in adjustNeighborhoods. This doesn't make sense for the first iteration though,
 * as individuals have not generally been evaluated then, and we do not want to fill
 * up with "dirty" individuals.
 */
void GSwarmAlgorithm::updatePositions() {
	std::size_t neighborhood_offset = 0;
	auto start = this->begin();

#ifdef DEBUG
	// Check that all neighborhoods have the default size
	for(std::size_t n=0; n<m_n_neighborhoods; n++) {
		if(m_n_neighborhood_members_cnt[n] != m_default_n_neighborhood_members) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::updatePositions(): Error!" << std::endl
					<< "m_n_neighborhood_members_cnt[" << n << "] has invalid size " << m_n_neighborhood_members_cnt[n] << std::endl
					<< "but expected size " << m_default_n_neighborhood_members << std::endl
			);
		}

		if(this->size() != m_n_neighborhoods*m_default_n_neighborhood_members) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::updatePositions(): Error!" << std::endl
					<< "The population has an incorrect size of " << this->size() << ", expected " << m_n_neighborhoods*m_default_n_neighborhood_members << std::endl
			);
		}
	}
#endif

	m_last_iteration_individuals_cnt.clear();
	if (afterFirstIteration()) {
		// Clone the individuals and copy them over
		for(const auto& ind_ptr: *this) {
			m_last_iteration_individuals_cnt.push_back(ind_ptr->clone<GParameterSet>());
		}
	}

#ifdef DEBUG
	// Cross-check that we have the nominal amount of individuals
	if(this->size() != m_n_neighborhoods*m_default_n_neighborhood_members) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updatePositions(): Error!" << std::endl
				<< "Invalid number of individuals found." << std::endl
				<< "Expected " << m_n_neighborhoods*m_default_n_neighborhood_members << " but got " << this->size() << std::endl
		);
	}
#endif /* DEBUG */

	// First update all positions
	for (std::size_t n = 0; n < m_n_neighborhoods; n++) {
#ifdef DEBUG
		if(afterFirstIteration()) {
			if(not m_neighborhood_bests_cnt[n]) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GSwarmAlgorithm::updatePositions():" << std::endl
						<< "m_neighborhood_bests_cnt[" << n << "] is empty." << std::endl
				);
			}

			if(n==0 && not m_global_best_ptr) { // Only check for the first n
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GSwarmAlgorithm::updatePositions():" << std::endl
						<< "m_global_best_ptr is empty." << std::endl
				);
			}
		}

		// Check that the number if individuals in each neighborhoods has the expected value
		if(m_n_neighborhood_members_cnt[n] != m_default_n_neighborhood_members) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::updatePositions(): Error!" << std::endl
					<< "Invalid number of members in neighborhood " << n << ": " << m_n_neighborhood_members_cnt[n] << std::endl
			);
		}
#endif /* DEBUG */

		for (std::size_t member = 0; member < m_n_neighborhood_members_cnt[n]; member++) {
			auto current = start + neighborhood_offset;

			// Update the neighborhood ids
			this->at(neighborhood_offset)->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->setNeighborhood(n);

			// Note: global/n bests and velocities haven't been determined yet in the first iteration and are not needed there
			if (afterFirstIteration() &&
				 not (*current)->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->checkNoPositionUpdateAndReset()) {
				// Update the swarm positions:
				updateIndividualPositions(
					n, (*current), m_neighborhood_bests_cnt[n], m_global_best_ptr, m_velocities_cnt[neighborhood_offset], std::make_tuple(
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
void GSwarmAlgorithm::updateIndividualPositions(
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
	if(not ind) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updateIndividualPositions():" << std::endl
				<< "Found empty individual \"ind\"" << std::endl
		);
	}
#endif /* DEBUG */

	// Extract the personal best
	std::shared_ptr <GParameterSet> personal_best = ind->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->getPersonalBest();

	// Further error checks
#ifdef DEBUG
	if(not personal_best) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updateIndividualPositions():" << std::endl
				<< "Found empty individual \"personal_best\"" << std::endl
		);
	}

	if(not neighborhood_best) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updateIndividualPositions():" << std::endl
				<< "Found empty individual \"neighborhood_best\"" << std::endl
		);
	}

	if(not global_best) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updateIndividualPositions():" << std::endl
				<< "Found empty individual \"global_best\"" << std::endl
		);
	}

	if(not velocity) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::updateIndividualPositions():" << std::endl
				<< "Found empty individual \"velocity\"" << std::endl
		);
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

	switch (m_update_rule) {
		case updateRule::SWARM_UPDATERULE_CLASSIC:
			// Multiply each floating point value with a random fp number in the range [0,1[, times a constant
			for (std::size_t i = 0; i < personalBestVec.size(); i++) {
				personalBestVec[i] *= (cPersonal * G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
				nbhBestVec[i] *= (cNeighborhood * G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
				glbBestVec[i] *= (cGlobal * G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			}
			break;

		case updateRule::SWARM_UPDATERULE_LINEAR:
			// Multiply each position with the same random floating point number times a constant
			Gem::Common::multVecConst<double>(personalBestVec, cPersonal * G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			Gem::Common::multVecConst<double>(nbhBestVec, cNeighborhood * G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
			Gem::Common::multVecConst<double>(glbBestVec, cGlobal * G_OptimizationAlgorithm_Base::m_uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)));
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
	// the number of stalls and the value of the m_repulsion_threshold variable. This allows
	// the algorithm to escape local optima, if m_repulsion_threshold is > 0.
	if (0 < m_repulsion_threshold && this->getStallCounter() >= m_repulsion_threshold) {
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
void GSwarmAlgorithm::pruneVelocity(std::vector<double> &velVec) {
#ifdef DEBUG
	if(velVec.size() != m_dbl_vel_max_cnt.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::pruneVelocity(): Error!" << std::endl
				<< "Found invalid vector sizes: " << velVec.size() << " / " << m_dbl_vel_max_cnt.size() << std::endl
		);
	}

#endif

	// Find the parameter that exceeds the allowed range by the largest percentage
	double currentPercentage = 0.;
	double maxPercentage = 0.;
	bool overflowFound = false;
	for (std::size_t i = 0; i < velVec.size(); i++) {
#ifdef DEBUG
		if(m_dbl_vel_max_cnt[i] <= 0.) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::pruneVelocity(): Error!" << std::endl
					<< "Found invalid max value: " << m_dbl_vel_max_cnt[i] << std::endl
			);
		}
#endif /* DEBUG */

		if (Gem::Common::gfabs(velVec[i]) > m_dbl_vel_max_cnt[i]) {
			overflowFound = true;
			currentPercentage = Gem::Common::gfabs(velVec[i]) / m_dbl_vel_max_cnt[i];
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
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GSwarmAlgorithm::pruneVelocity(): Error!" << std::endl
						<< "Invalid maxPercentage: " << maxPercentage << std::endl
				);
			}
#endif
			velVec[i] /= maxPercentage;
		}
	}
}

/******************************************************************************/
/**
 * Triggers the fitness calculation of all individuals
 */
void GSwarmAlgorithm::runFitnessCalculation_() {
	using namespace Gem::Courtier;

	//--------------------------------------------------------------------------------
	// Submit work items and wait for results.
	setProcessingFlag(this->m_data_cnt, std::make_tuple(std::size_t(0), this->m_data_cnt.size()));
	auto status = this->workOn(
		m_data_cnt
		, false // do not resubmit unprocessed items
		, "GSwarmAlgorithm::runFitnessCalculation()"
	);

	// Retrieve a vector of old work items
	auto old_work_items = this->getOldWorkItems();

	// Update the iteration of older individuals (they will keep their old neighborhood id)
	// and attach them to the data vector
	for(const auto& item_ptr: old_work_items) {
		item_ptr->setAssignedIteration(this->getIteration());
		this->push_back(item_ptr);
	}
	old_work_items.clear();

	//--------------------------------------------------------------------------------
	// Take care of unprocessed items, if these exist
	if(not status.is_complete) {
		std::size_t n_erased = Gem::Common::erase_if(
			this->m_data_cnt
			, [this](std::shared_ptr<GParameterSet> p) -> bool {
				return (p->getProcessingStatus() == Gem::Courtier::processingStatus::DO_PROCESS);
			}
		);

#ifdef DEBUG
		glogger
			<< "In GSwarmAlgorithm::runFitnessCalculation(): " << std::endl
			<< "Removed " << n_erased << " unprocessed work items in iteration " << this->getIteration() << std::endl
			<< GLOGGING;
#endif
	}

	// Remove items for which an error has occurred during processing
	if(status.has_errors) {
		std::size_t n_erased = Gem::Common::erase_if(
			this->m_data_cnt
			, [this](std::shared_ptr<GParameterSet> p) -> bool {
				return p->has_errors();
			}
		);

#ifdef DEBUG
		glogger
			<< "In GSwarmAlgorithm::runFitnessCalculation(): " << std::endl
			<< "Removed " << n_erased << " erroneous work items in iteration " << this->getIteration() << std::endl
			<< GLOGGING;
#endif
	}

	//--------------------------------------------------------------------------------
	// Sort according to the individuals' neighborhoods
	sort(
		m_data_cnt.begin()
		, m_data_cnt.end()
		, [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) -> bool {
			return x->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->getNeighborhood() < y->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->getNeighborhood();
		}
	);

	// Now update the number of items in each neighborhood: First reset the number of members of each neighborhood
	Gem::Common::assignVecConst(m_n_neighborhood_members_cnt, (std::size_t)0);
	// Then update the number of individuals in each neighborhood
	for(const auto& item_ptr: *this) {
		m_n_neighborhood_members_cnt[item_ptr->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->getNeighborhood()] += 1;
	}

	// The population will be fixed in the GSwarmAlgorithm::adjustNeighborhoods() function
	// TODO: Change to fixAfterJobSubmission
}

/******************************************************************************/
/**
 * Updates the best individuals found. This function assumes that the population already contains individuals
 * and that the neighborhood and global bests have been initialized (possibly with dummy values). This should have
 * happened in the adjustPopulation() function. It also assumes that all individuals have already been evaluated.
 *
 * @return The best evaluation found in this iteration
 */
std::tuple<double, double> GSwarmAlgorithm::findBests() {
	auto m = this->at(0)->getMaxMode(); // We assume that the maxMode is the same for all individuals

	std::size_t bestLocalId = 0;
	std::tuple<double, double> bestLocalFitness = std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());
	std::tuple<double, double> bestIterationFitness = std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());

#ifdef DEBUG
	std::size_t pos = 0;
	for(const auto& ind_ptr: *this) {
		if(ind_ptr->is_due_for_processing() || ind_ptr->has_errors()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSwarmAlgorithm::findBests(): Error!" << std::endl
					<< "Found individual in position " << pos << " in iteration " << this->getIteration() << std::endl
					<< "which is unprocessed or has errors" << std::endl
					<< "is_due_for_processing() == " << ind_ptr->is_due_for_processing() << ", has_errors() == " << ind_ptr->has_errors() << std::endl
			);
		}

		pos++;
	}
#endif /* DEBUG */

	// Update the personal bests of all individuals
	if (inFirstIteration()) {
		for (const auto& ind_ptr: *this) {
			updatePersonalBest(ind_ptr);
		}
	} else {
		for (const auto& ind_ptr: *this) {
			updatePersonalBestIfBetter(ind_ptr);
		}
	}

	// Sort individuals in all neighborhoods according to their fitness
	for (std::size_t n = 0; n < m_n_neighborhoods; n++) {
		// identify the first and last id of the individuals in the current neighborhood
		std::size_t firstCounter = getFirstNIPos(n);
		std::size_t lastCounter = getLastNIPos(n);

		// Only partially sort the arrays
		std::sort(
			this->begin() + firstCounter, this->begin() + lastCounter,
			[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
				return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
			}
		);

		// Check whether the best individual of the neighborhood is better than
		// the best individual found so far in this neighborhood
		if (inFirstIteration()) {
			m_neighborhood_bests_cnt.at(n) = (*(this->begin() + firstCounter))->clone<GParameterSet>();
		}
		else {
			if(isBetter(
				(*(this->begin() + firstCounter))->transformed_fitness(0)
				, m_neighborhood_bests_cnt.at(n)->transformed_fitness(0)
				, m
			)) {
				(m_neighborhood_bests_cnt.at(n))->GObject::load(*(this->begin() + firstCounter));
			}
		}
	}

	// Identify the best individuals among all neighborhood bests
	for (std::size_t n = 0; n < m_n_neighborhoods; n++) {
		if(isBetter(
			(m_neighborhood_bests_cnt.at(n))->transformed_fitness(0)
			, std::get<G_TRANSFORMED_FITNESS>(bestLocalFitness)
			, m
		)) {
			bestLocalId = n;
			bestLocalFitness = (m_neighborhood_bests_cnt.at(n))->getFitnessTuple();
		}
	}

	// Compare the best neighborhood individual with the globally best individual and
	// update it, if necessary. Initialize it in the first generation.
	if (inFirstIteration()) {
		m_global_best_ptr = (m_neighborhood_bests_cnt.at(bestLocalId))->clone<GParameterSet>();
	} else {
		if(isBetter(
			std::get<G_TRANSFORMED_FITNESS>(bestLocalFitness)
			, m_global_best_ptr->transformed_fitness(0)
			, m
		)) {
			m_global_best_ptr->GObject::load(m_neighborhood_bests_cnt.at(bestLocalId));
		}
	}

	// Identify the best fitness in the current iteration
	for (std::size_t i = 0; i < this->size(); i++) {
		if(isBetter(
			std::get<G_TRANSFORMED_FITNESS>(this->at(i)->getFitnessTuple())
			, std::get<G_TRANSFORMED_FITNESS>(bestIterationFitness)
			, m
		)) {
			bestIterationFitness = this->at(i)->getFitnessTuple();
		}
	}

	return bestIterationFitness;
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks. This function implements
 * the purely virtual function G_OptimizationAlgorithm_Base::adjustPopulation() .
 */
void GSwarmAlgorithm::adjustPopulation_() {
	const std::size_t currentSize = this->size();
	const std::size_t defaultPopSize = getDefaultPopulationSize();
	const std::size_t nNeighborhoods = getNNeighborhoods();

	if (currentSize == 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::adjustPopulation() :" << std::endl
				<< "No individuals found in the population." << std::endl
				<< "You need to add at least one individual before" << std::endl
				<< "the call to optimize<>()" << std::endl
		);
	} else if (currentSize == 1) {
		// Fill up with random items to the number of neighborhoods
		for (std::size_t i = 1; i < m_n_neighborhoods; i++) {
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
		for (std::size_t n = 0; n < m_n_neighborhoods; n++) {
			m_n_neighborhood_members_cnt[n] = m_default_n_neighborhood_members;
		}
	} else {
		if (currentSize < m_n_neighborhoods) {
			// First fill up the neighborhoods, if required
			for (std::size_t m = 0; m < (m_n_neighborhoods - currentSize); m++) {
				this->push_back(this->front()->clone<GParameterSet>());
				this->back()->randomInit(activityMode::ACTIVEONLY);
			}

			// Now follow the procedure used for the "m_n_neighborhoods" case
			fillUpNeighborhood1();
		} else if (currentSize > m_n_neighborhoods && currentSize < defaultPopSize) {
			// New procedure:
			// - Find out how many individuals exist in each neighborhood (Check: Has the neighborhood-id already been assigned here ?)
			// - For each neighborhood: add missing items to the end of vector
			// - Sort the vector according to neighborhoods
			// - Remove surplus items in each neighborhood
			// - In DEBUG mode: make sure each neighborhood is at the correct size

			// TODO: For now we simply resize the population to the number of neighborhoods,
			// then fill up again. This means that we loose some predefined values, which
			// is ugly and needs to be changed in later versions.
			this->resize(m_n_neighborhoods);
			fillUpNeighborhood1();

			// TODO: This is catastrophic if work items didn't return in GSwarmAlgorithm,
			// as it uses adjustPopulation to fix the population.
			// MUST FIX
		} else { // currentSize > defaultPopsize
			// Update the number of individuals in each neighborhood
			for (std::size_t n = 0; n < m_n_neighborhoods - 1; n++) {
				m_n_neighborhood_members_cnt[n] = m_default_n_neighborhood_members;
			}

			// Adjust the m_n_neighborhood_members_cnt array. The surplus items will
			// be assumed to belong to the last neighborhood, all other neighborhoods
			// have the default size.
			// TODO: This is bad, as adjustPopulation is used to adjust sizes also during an optimization run
			// Must remove worst items for each neighborhood individually. Also: Must make sure that. while some
			// neighborhoods might have too many, others might have too few entries. MUST FIX.
			m_n_neighborhood_members_cnt[m_n_neighborhoods - 1] = m_default_n_neighborhood_members + (currentSize - defaultPopSize);
		}
	}

#ifdef DEBUG
	// As the above switch statement is quite complicated, cross check that we now
	// indeed have at least the required number of individuals
	if(this->size() < defaultPopSize) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::adjustPopulation() :" << std::endl
				<< "Expected at least a population size of " << defaultPopSize << std::endl
				<< "but found a size of " << this->size() << ", which is too small." << std::endl
		);
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
void GSwarmAlgorithm::fillUpNeighborhood1() {
	// Do some error checking
	if (this->size() != m_n_neighborhoods) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::fillUpNeighborhood1():" << std::endl
				<< "Invalid size: " << this->size() << " Expected " << m_n_neighborhoods << std::endl
		);
	}

	if (m_default_n_neighborhood_members == 1) return; // nothing to do

	// Starting with the last item, loop over all neighborhoods
	for (std::size_t i = 0; i < m_n_neighborhoods; i++) {
		std::size_t n = m_n_neighborhoods - 1 - i; // Calculate the correct neighborhood

		// Insert the required number of clones after the existing individual
		for (std::size_t m = 1; m < m_default_n_neighborhood_members; m++) { // m stands for "missing"
			// Add a clone of the first individual in the neighborhood to the next position
			this->insert(this->begin() + n, (*(this->begin() + n))->clone<GParameterSet>());
			// Make sure it has a unique value, if requested
			if (m_random_fill_up) {
#ifdef DEBUG
				if(not (*(this->begin()+n+1))) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In GSwarmAlgorithm::fillUpNeighborhood1():" << std::endl
							<< "Found empty position " << n << std::endl
					);
				}
#endif /* DEBUG */

				(*(this->begin() + n + 1))->randomInit(activityMode::ACTIVEONLY);
			}
		}

		// Update the number of individuals in each neighborhood
		m_n_neighborhood_members_cnt[n] = m_default_n_neighborhood_members;
	}
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for personal distances.
 *
 * @param c_personal A static multiplier for personal distances
 */
void GSwarmAlgorithm::setCPersonal(double c_personal) {
	m_c_personal = c_personal;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for personal distances
 *
 * @return The static multiplier for personal distances
 */
double GSwarmAlgorithm::getCPersonal() const {
	return m_c_personal;
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for neighborhood distances.
 *
 * @param c_neighborhood A static multiplier for neighborhood distances
 */
void GSwarmAlgorithm::setCNeighborhood(double c_neighborhood) {
	m_c_neighborhood = c_neighborhood;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for neighborhood distances
 *
 * @return A static multiplier for neighborhood distances
 */
double GSwarmAlgorithm::getCNeighborhood() const {
	return m_c_neighborhood;
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for global distances
 *
 * @param c_global A static multiplier for global distances
 */
void GSwarmAlgorithm::setCGlobal(double c_global) {
	m_c_global = c_global;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for global distances
 *
 * @return The static multiplier for global distances
 */
double GSwarmAlgorithm::getCGlobal() const {
	return m_c_global;
}

/******************************************************************************/
/**
 * Allows to set a static multiplier for velocities
 *
 * @param c_velocity A static multiplier for velocities
 */
void GSwarmAlgorithm::setCVelocity(double c_velocity) {
	m_c_velocity = c_velocity;
}

/******************************************************************************/
/**
 * Allows to retrieve the static multiplier for velocities
 *
 * @return The static multiplier for velocities
 */
double GSwarmAlgorithm::getCVelocity() const {
	return m_c_velocity;
}

/******************************************************************************/
/**
 * Allows to set the velocity range percentage
 *
 * @param velocityRangePercentage The velocity range percentage
 */
void GSwarmAlgorithm::setVelocityRangePercentage(double velocityRangePercentage) {
	// Do some error checking
	if (velocityRangePercentage <= 0. || velocityRangePercentage > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm::setVelocityRangePercentage()" << std::endl
				<< "Invalid velocityRangePercentage: " << velocityRangePercentage << std::endl
		);
	}

	m_velocity_range_percentage = velocityRangePercentage;
}

/******************************************************************************/
/**
 * Allows to retrieve the velocity range percentage
 *
 * @return The velocity range percentage
 */
double GSwarmAlgorithm::getVelocityRangePercentage() const {
	return m_velocity_range_percentage;
}

/******************************************************************************/
/**
 * Retrieves the number of neighborhoods
 *
 * @return The number of neighborhoods in the population
 */
std::size_t GSwarmAlgorithm::getNNeighborhoods() const {
	return m_n_neighborhoods;
}

/******************************************************************************/
/**
 * Retrieves the default number of individuals in each neighborhood
 *
 * @return The default number of individuals in each neighborhood
 */
std::size_t GSwarmAlgorithm::getDefaultNNeighborhoodMembers() const {
	return m_default_n_neighborhood_members;
}

/******************************************************************************/
/**
 * Retrieves the current number of individuals in a given neighborhood
 *
 * @return The current number of individuals in a given neighborhood
 */
std::size_t GSwarmAlgorithm::getCurrentNNeighborhoodMembers(const std::size_t &neighborhood) const {
	return m_n_neighborhood_members_cnt[neighborhood];
}

/******************************************************************************/
/**
 * Allows to specify the update rule to be used by the swarm.
 *
 * @param ur The desired update rule
 */
void GSwarmAlgorithm::setUpdateRule(updateRule ur) {
	m_update_rule = ur;
}

/******************************************************************************/
/**
 * Allows to retrieve the update rule currently used by the swarm.
 *
 * @return The current update rule
 */
updateRule GSwarmAlgorithm::getUpdateRule() const {
	return m_update_rule;
}

/******************************************************************************/
/**
 * Allows to specify the number of stalls as of which the algorithm switches to
 * repulsive mode. Set this value to 0 in order to disable repulsive mode.
 *
 * @param repulsionThreshold The threshold as of which the algorithm switches to repulsive mode
 */
void GSwarmAlgorithm::setRepulsionThreshold(std::uint32_t repulsionThreshold) {
	m_repulsion_threshold = repulsionThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of stalls as of which the algorithm switches
 * to repulsive mode.
 *
 * @return The value of the repulsionThreshold_ variable
 */
std::uint32_t GSwarmAlgorithm::getRepulsionThreshold() const {
	return m_repulsion_threshold;
}

/******************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have equal value
 */
void GSwarmAlgorithm::setNeighborhoodsEqualFillUp() {
	m_random_fill_up = false;
}

/******************************************************************************/
/**
 * All individuals automatically added to a neighborhood will have a random value
 */
void GSwarmAlgorithm::setNeighborhoodsRandomFillUp(bool randomFillUp) {
	m_random_fill_up = randomFillUp;
}

/******************************************************************************/
/**
 * Allows to check whether neighborhoods are filled up with random individuals
 *
 * @return A boolean indicating whether neighborhoods are filled up with random values
 */
bool GSwarmAlgorithm::neighborhoodsFilledUpRandomly() const {
	return m_random_fill_up;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GSwarmAlgorithm::getNProcessableItems_() const {
	return this->size(); // All items in the population are updated in each iteration and need to be processed
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarmAlgorithm::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (G_OptimizationAlgorithm_Base::modify_GUnitTests_()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSwarmAlgorithm::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmAlgorithm::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSwarmAlgorithm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmAlgorithm::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSwarmAlgorithm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

