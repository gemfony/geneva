/**
 * @file G_OptimizationAlgorithm_EvolutionaryAlgorithm.cpp
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

#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm.hpp"

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithm)

/******************************************************************************/

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The default constructor. All initialization work of member variable
 * is done in the class body.
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm()
	: G_OptimizationAlgorithm_ParChild()
{
	// Make sure we start with a valid population size if the user does not supply these values
	this->setPopulationSizes(100, 1);
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp Another GEvolutionaryAlgorithm object
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm& cp)
	: G_OptimizationAlgorithm_ParChild(cp)
	  , m_sorting_mode(cp.m_sorting_mode)
	  , m_n_threads(cp.m_n_threads)
{
	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of optimization monitors.
}

/******************************************************************************/
/**
* The standard assignment operator
*/
const GEvolutionaryAlgorithm& GEvolutionaryAlgorithm::operator=(const GEvolutionaryAlgorithm& cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GEvolutionaryAlgorithm object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithm::operator==(const GEvolutionaryAlgorithm& cp) const {
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
 * Checks for inequality with another GEvolutionaryAlgorithm object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEvolutionaryAlgorithm::operator!=(const GEvolutionaryAlgorithm& cp) const {
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
  * Searches for compliance with expectations with respect to another object
  * of the same type
  *
  * @param cp A constant reference to another GObject object
  * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GEvolutionaryAlgorithm::compare(
	const GObject& cp // the other object
	, const Gem::Common::expectation& e // the expectation for this object, e.g. equality
	, const double& limit// the limit for allowed deviations of floating point types
) const {
	using namespace Gem::Common;

// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithm *p_load
		= Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithm>(cp, this);

	GToken token("GEvolutionaryAlgorithm", e);

// Compare our parent data ...
	Gem::Common::compare_base<G_OptimizationAlgorithm_ParChild>(IDENTITY(*this, *p_load), token);

// ... and then the local data
	compare_t(IDENTITY(m_sorting_mode, p_load->m_sorting_mode), token);
	compare_t(IDENTITY(m_n_threads, p_load->m_n_threads), token);

// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GEvolutionaryAlgorithm::resetToOptimizationStart() {
	// There is no more work to be done here, so we simply call the
	// function of the parent class
	G_OptimizationAlgorithm_ParChild::resetToOptimizationStart();
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm.
 *
 * @return The type of optimization algorithm
 */
std::string GEvolutionaryAlgorithm::getAlgorithmPersonalityType() const {
	return std::string("PERSONALITY_EA");
}

/******************************************************************************/
/**
  * Returns the name of this optimization algorithm
  *
  * @return The name assigned to this optimization algorithm
  */
std::string GEvolutionaryAlgorithm::getAlgorithmName() const {
	return std::string("Evolutionary Algorithm");
}

/******************************************************************************/
/**
  * Sets the sorting scheme. In MUPLUSNU_SINGLEEVAL, new parents will be selected from the entire
  * population, including the old parents. In MUCOMMANU_SINGLEEVAL new parents will be selected
  * from children only. MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation
  * will also become a new parent (unless a better child was found). All other parents are
  * selected from children only.
  *
  * @param smode The desired sorting scheme
  */
void GEvolutionaryAlgorithm::setSortingScheme(sortingMode smode) {
	m_sorting_mode = smode;
}

/******************************************************************************/
/**
  * Retrieves information about the current sorting scheme (see
  * G_OA_EvolutionaryAlgorithm::setSortingScheme() for further information).
  *
  * @return The current sorting scheme
  */
sortingMode GEvolutionaryAlgorithm::getSortingScheme() const {
	return m_sorting_mode;
}

/******************************************************************************/
/**
  * Extracts all individuals on the pareto front
  */
void GEvolutionaryAlgorithm::extractCurrentParetoIndividuals(
	std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>>& paretoInds
) {
	// Make sure the vector is empty
	paretoInds.clear();

	for(auto it=this->begin(); it!= this->end(); ++it) {
		if((*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) {
			paretoInds.push_back(*it);
		}
	}
}

/******************************************************************************/
/**
  * Adds the individuals of this iteration to a priority queue. The
  * queue will be sorted by the first evaluation criterion of the individuals
  * and may either have a limited or unlimited size, depending on user-
  * settings. The procedure is different for pareto optimization, as we only
  * want the individuals on the current pareto front to be added.
  */
void GEvolutionaryAlgorithm::updateGlobalBestsPQ(
	GParameterSetFixedSizePriorityQueue & bestIndividuals
) {
	const bool REPLACE = true;
	const bool CLONE = true;

#ifdef DEBUG
	if(this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OA_EvolutionaryAlgorithm::updateGlobalBestsPQ() :" << std::endl
				<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
		);
	}
#endif /* DEBUG */

	switch (m_sorting_mode) {
		//----------------------------------------------------------------------------
		case sortingMode::MUPLUSNU_SINGLEEVAL:
		case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
		case sortingMode::MUCOMMANU_SINGLEEVAL:
			G_OptimizationAlgorithm_Base::updateGlobalBestsPQ(bestIndividuals);
			break;

			//----------------------------------------------------------------------------
		case sortingMode::MUPLUSNU_PARETO:
		case sortingMode::MUCOMMANU_PARETO: {
			// Retrieve all individuals on the pareto front
			std::vector<std::shared_ptr < Gem::Geneva::GParameterSet>> paretoInds;
			this->extractCurrentParetoIndividuals(paretoInds);

			// We simply add all parent individuals to the queue. As we only want
			// the individuals on the current pareto front, we replace all members
			// of the current priority queue
			bestIndividuals.add(paretoInds, CLONE, REPLACE);
		}
			break;

			//----------------------------------------------------------------------------
	}
}

/******************************************************************************/
/**
 * Adds the individuals of this iteration to a priority queue. The
 * queue will be sorted by the first evaluation criterion of the individuals
 * and will be cleared prior to adding the new individuals. This results in
 * the best individuals of the current iteration.
 */
void GEvolutionaryAlgorithm::updateIterationBestsPQ(
	GParameterSetFixedSizePriorityQueue& bestIndividuals
) {
	const bool CLONE = true;
	const bool REPLACE = true;

#ifdef DEBUG
	if(this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "G_OA_EvolutionaryAlgorithm::updateIterationBestsPQ() :" << std::endl
				<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
		);
	}
#endif /* DEBUG */

	switch (m_sorting_mode) {
		//----------------------------------------------------------------------------
		case sortingMode::MUPLUSNU_SINGLEEVAL:
		case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
		case sortingMode::MUCOMMANU_SINGLEEVAL: {
			G_OptimizationAlgorithm_Base::updateIterationBestsPQ(bestIndividuals);
		} break;

			//----------------------------------------------------------------------------
		case sortingMode::MUPLUSNU_PARETO:
		case sortingMode::MUCOMMANU_PARETO: {
			// Retrieve all individuals on the pareto front
			std::vector<std::shared_ptr < Gem::Geneva::GParameterSet>> paretoInds;
			this->extractCurrentParetoIndividuals(paretoInds);

			// We simply add all parent individuals to the queue. As we only want
			// the individuals on the current pareto front, we replace all members
			// of the current priority queue
			bestIndividuals.add(paretoInds, CLONE, REPLACE);
		} break;

			//----------------------------------------------------------------------------
	}
}

/******************************************************************************/
/**
  * Adds local configuration options to a GParserBuilder object
  *
  * @param gpb The GParserBuilder object to which configuration options should be added
  */
void GEvolutionaryAlgorithm::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
)
{
	// Call our parent class'es function
	G_OptimizationAlgorithm_ParChild::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<std::uint16_t>(
		"nAdaptionThreads" // The name of the variable
		, DEFAULTNSTDTHREADS // The default value
		, [this](std::uint16_t nt) { this->setNThreads(nt); }
	)
		<< "The number of threads used to simultaneously adapt individuals" << std::endl
		<< "0 means \"automatic\"";

	gpb.registerFileParameter<sortingMode>(
		"sortingMethod" // The name of the variable
		, DEFAULTEASORTINGMODE // The default value
		, [this](sortingMode sm) { this->setSortingScheme(sm); }
	)
		<< "The sorting scheme. Options" << std::endl
		<< "0: MUPLUSNU mode with a single evaluation criterion" << std::endl
		<< "1: MUCOMMANU mode with a single evaluation criterion" << std::endl
		<< "2: MUCOMMANU mode with single evaluation criterion," << std::endl
		<< "   the best parent of the last iteration is retained" << std::endl
		<< "   unless a better individual has been found" << std::endl
		<< "3: MUPLUSNU mode for multiple evaluation criteria, pareto selection" << std::endl
		<< "4: MUCOMMANU mode for multiple evaluation criteria, pareto selection";
}

/******************************************************************************/
/**
  * Emits a name for this class / object
  */
std::string GEvolutionaryAlgorithm::name() const {
	return std::string("GEvolutionaryAlgorithm");
}

/******************************************************************************/
/**
  * Sets the number of threads this population uses for adaption. If nThreads is set
  * to 0, an attempt will be made to set the number of threads to the number of hardware
  * threading units (e.g. number of cores or hyper-threading units).
  *
  * @param nThreads The number of threads this class uses
  */
void GEvolutionaryAlgorithm::setNThreads(std::uint16_t nThreads) {
	if (nThreads == 0) {
		m_n_threads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNSTDTHREADS));
	}
	else {
		m_n_threads = nThreads;
	}
}

/******************************************************************************/
/**
 * Retrieves the number of threads this population uses for adaption
 *
 * @return The maximum number of allowed threads
 */
std::uint16_t GEvolutionaryAlgorithm::getNThreads() const {
	return m_n_threads;
}

/******************************************************************************/
/**
  * Loads the data of another GEvolutionaryAlgorithm object, camouflaged as a GObject.
 *
  * @param cp A pointer to another GEvolutionaryAlgorithm object, camouflaged as a GObject
  */
void GEvolutionaryAlgorithm::load_(const GObject *cp) {
	// Check that we are dealing with a GEvolutionaryAlgorithm reference independent
	// of this object and convert the pointer
	const GEvolutionaryAlgorithm *p_load = Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithm>(cp, this);

	// First load the parent class'es data ...
	G_OptimizationAlgorithm_ParChild::load_(cp);

	// ... and then our own data
	m_sorting_mode = p_load->m_sorting_mode;
	m_n_threads = p_load->m_n_threads;
}

/******************************************************************************/
/**
  * Creates a deep copy of this object
  *
  * @return A deep copy of this object
  */
GObject * GEvolutionaryAlgorithm::clone_() const {
	return new GEvolutionaryAlgorithm(*this);
}

/******************************************************************************/
/**
 * Some error checks related to population sizes
 */
void GEvolutionaryAlgorithm::populationSanityChecks() const {
	// First check that we have been given a suitable value for the number of parents.
	// Note that a number of checks (e.g. population size != 0) has already been done
	// in the parent class.
	if (this->m_n_parents == 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OA_EvolutionaryAlgorithm::populationSanityChecks(): Error!" << std::endl
				<< "Number of parents is set to 0"
		);
	}

	// In MUCOMMANU_SINGLEEVAL mode we want to have at least as many children as parents,
	// whereas MUPLUSNU_SINGLEEVAL only requires the population size to be larger than the
	// number of parents. MUNU1PRETAIN has the same requirements as MUCOMMANU_SINGLEEVAL,
	// as it is theoretically possible that all children are better than the former
	// parents, so that the first parent individual will be replaced.
	std::size_t popSize = this->getPopulationSize();
	if ( // TODO: Why are PARETO modes missing here ?
		((m_sorting_mode == sortingMode::MUCOMMANU_SINGLEEVAL || m_sorting_mode == sortingMode::MUNU1PRETAIN_SINGLEEVAL) && (popSize < 2 * this->m_n_parents)) ||
		(m_sorting_mode == sortingMode::MUPLUSNU_SINGLEEVAL && popSize <= this->m_n_parents)
		) {
		std::ostringstream error;
		error
			<< "In G_OA_EvolutionaryAlgorithm::populationSanityChecks() :" << std::endl
			<< "Requested size of population is too small :" << popSize << " " << this->m_n_parents << std::endl
			<< "Sorting scheme is ";

		switch (m_sorting_mode) {
			case sortingMode::MUPLUSNU_SINGLEEVAL:
				error << "MUPLUSNU_SINGLEEVAL" << std::endl;
				break;
			case sortingMode::MUCOMMANU_SINGLEEVAL:
				error << "MUCOMMANU_SINGLEEVAL" << std::endl;
				break;
			case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
				error << "MUNU1PRETAIN" << std::endl;
				break;
			case sortingMode::MUPLUSNU_PARETO:
				error << "MUPLUSNU_PARETO" << std::endl;
				break;
			case sortingMode::MUCOMMANU_PARETO:
				error << "MUCOMMANU_PARETO" << std::endl;
				break;
		};

		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< error.str()
		);
	}
}

/******************************************************************************/
/**
  * Adapt all children in parallel. Evaluation is done in a separate function (runFitnessCalculation).
  */
void GEvolutionaryAlgorithm::adaptChildren() {
	// Retrieve the range of individuals to be adapted
	std::tuple<std::size_t, std::size_t> range = this->getAdaptionRange();

	// Loop over all requested individuals and perform the adaption
	for (auto it = (this->begin() + std::get<0>(range)); it != (this->begin() + std::get<1>(range)); ++it) {
		m_tp_ptr->async_schedule(
			// Note: may not pass it as a reference, as it is a local variable in the loop and might
			// vanish of have been altered once the thread has started and adaption is requested.
			[it]() {
				(*it)->adapt();
			}
		);
	}

	// Wait for all threads in the pool to complete their work
	m_tp_ptr->wait();
}

/******************************************************************************/
/**
  * We submit individuals to the broker connector and wait for processed items.
 */
void GEvolutionaryAlgorithm::runFitnessCalculation() {
	//--------------------------------------------------------------------------------
	// Start by marking the work to be done in the individuals.
	// "range" will hold the start- and end-points of the range
	// to be worked on
	std::tuple<std::size_t, std::size_t> range = getEvaluationRange();

#ifdef DEBUG
	// There should be no situation in which a "clean" child is submitted
	// through this function. There MAY be situations, where in the first iteration
	// parents are clean, e.g. when they were extracted from another optimization.
	for(std::size_t i=this->getNParents(); i<this->size(); i++) {
		if(!this->at(i)->isDirty()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GEvolutionaryAlgorithm::runFitnessCalculation(): Error!" << std::endl
					<< "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
					<< "but found \"clean\" individual in position " << i << std::endl
			);
		}
	}


	if(this->size() != this->getDefaultPopulationSize()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GEvolutionaryAlgorithm::runFitnessCalculation(): Error!" << std::endl
				<< "Size of data vector (" << this->size() << ") should be " << this->getDefaultPopulationSize() << std::endl
		);
	}
#endif

	//--------------------------------------------------------------------------------
	// Retrieve a vector describing the items to be modified
	std::vector<bool> workItemPos = Gem::Courtier::getBooleanMask(
		this->size()
		, std::get<0>(range)
		, std::get<1>(range)
	);

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results.
	this->workOn(
		this->data
		, workItemPos
		, false // do not resubmit unprocessed items
		, "GEvolutionaryAlgorithm::runFitnessCalculation()"
	);

	//--------------------------------------------------------------------------------
	// Take care of unprocessed items
	// TODO: Check how often this happens
	Gem::Common::erase_according_to_flags(
		this->data
		, workItemPos
		, Gem::Courtier::GBC_UNPROCESSED
		, 0
		, this->size()
	);

	// Remove items for which an error has occurred during processing
	Gem::Common::erase_if(
		this->data
		, [this](std::shared_ptr<GParameterSet> p) -> bool {
			return p->processing_was_unsuccessful();
		}
	);

	//--------------------------------------------------------------------------------
	// Now fix the population -- it may be smaller than its nominal size
	fixAfterJobSubmission();
}

/******************************************************************************/
/**
 * Fixes the population after a job submission
 *
 * TODO: Make this a plugin for the executor?
 */
void GEvolutionaryAlgorithm::fixAfterJobSubmission() {
	std::size_t np = this->getNParents();
	std::uint32_t iteration = this->getIteration();

	// Retrieve a vector of old work items
	auto old_work_items = this->getOldWorkItems();

	// Remove parents from older iterations from old work items -- we do not want them.
	// Note that "remove_if" simply moves items not satisfying the predicate to the end of the list.
	// We thus need to explicitly erase these items. remove_if returns the iterator position right after
	// the last item not satisfying the predicate.
	old_work_items.erase(
		std::remove_if(
			old_work_items.begin()
			, old_work_items.end()
			, [iteration](std::shared_ptr<GParameterSet> x) -> bool {
				return x->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isParent() && x->getAssignedIteration() != iteration;
			}
		)
		, old_work_items.end()
	);

	// Make it known to remaining old individuals that they are now part of a new iteration
	std::for_each(
		old_work_items.begin()
		, old_work_items.end(),
		[iteration](std::shared_ptr <GParameterSet> p) { p->setAssignedIteration(iteration); }
	);

	// Make sure that parents are at the beginning of the array.
	sort(
		this->begin()
		, this->end()
		, [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) -> bool {
			return (x->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isParent() > y->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isParent());
		}
	);

	// Attach all old work items to the end of the current population and clear the array of old items
	for(auto item: old_work_items) {
		this->push_back(item);
	}
	old_work_items.clear();

	// Check that individuals do exist in the population. We cannot continue, if this is not the case
	if(this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GEvolutionaryAlgorithm::fixAfterJobSubmission(): Error!" << std::endl
				<< "Population holds no data" << std::endl
		);
	} else {
		// Emit a warning if no children have returned
		if(this->size() <= this->getNParents()) {
			glogger
				<< "In GEvolutionaryAlgorithm::fixAfterJobSubmission(): Warning!" << std::endl
				<< "No child individuals have returned" << std::endl
				<< "We need to fill up the population with clones from parent individuals" << std::endl
				<< GWARNING;
		}
	}

	// Check that the dirty flag of the last individual isn't set. This is a severe error.
	if(this->back()->isDirty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GEvolutionaryAlgorithm::fixAfterJobSubmission(): Error!" << std::endl
				<< "The last individual in the population has the dirty" << std::endl
				<< "flag set, so we cannot use it for cloning" << std::endl
		);
	}


	// Add missing individuals, as clones of the last item
	if (this->size() < this->getDefaultPopulationSize()) {
		std::size_t fixSize = this->getDefaultPopulationSize() - this->size();
		for (std::size_t i = 0; i < fixSize; i++) {
			// This function will create a clone of its argument
			this->push_back_clone(this->back());
		}
	}

	// Mark the first this->m_n_parents individuals as parents and the rest of the individuals as children.
	// We want to have a sane population.
	for (auto it = this->begin(); it != this->begin() + np; ++it) {
		(*it)->GParameterSet::template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsParent();
	}
	for (auto it = this->begin() + np; it != this->end(); ++it) {
		(*it)->GParameterSet::template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsChild();
	}

	// We care for too many returned individuals in the selectBest() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.
}

/******************************************************************************/
/**
	* Choose new parents, based on the selection scheme set by the user.
	*/
void GEvolutionaryAlgorithm::selectBest() {
#ifdef DEBUG
	// We require at this stage that at least the default number of
	// children is present. If individuals can get lost in your setting,
	// you must add mechanisms to "repair" the population before this
	// function is called
	if((this->size()-this->m_n_parents) < this->m_default_n_children){
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OA_EvolutionaryAlgorithm::select():" << std::endl
				<< "Too few children. Got " << (this->size() - this->getNParents()) << "," << std::endl
				<< "but was expecting at least " << this->getDefaultNChildren() << std::endl
		);
	}
#endif /* DEBUG */

	switch (m_sorting_mode) {
		//----------------------------------------------------------------------------
		case sortingMode::MUPLUSNU_SINGLEEVAL: {
			this->sortMuPlusNuMode();
		}
			break;

			//----------------------------------------------------------------------------
		case sortingMode::MUNU1PRETAIN_SINGLEEVAL: {
			if (this->inFirstIteration()) {
				this->sortMuPlusNuMode();
			} else {
				this->sortMunu1pretainMode();
			}
		}
			break;

			//----------------------------------------------------------------------------
		case sortingMode::MUCOMMANU_SINGLEEVAL: {
			if (this->inFirstIteration()) {
				this->sortMuPlusNuMode();
			} else {
				this->sortMuCommaNuMode();
			}
		}
			break;

			//----------------------------------------------------------------------------
		case sortingMode::MUPLUSNU_PARETO:
			this->sortMuPlusNuParetoMode();
			break;

			//----------------------------------------------------------------------------
		case sortingMode::MUCOMMANU_PARETO: {
			if (this->inFirstIteration()) {
				this->sortMuPlusNuParetoMode();
			} else {
				this->sortMuCommaNuParetoMode();
			}
		}
			break;

			//----------------------------------------------------------------------------
	}

	// Let parents know they are parents
	this->markParents();

#ifdef DEBUG
	// Make sure our population is not smaller than its nominal size -- this
	// should have been taken care of in fixAfterJobSubmission() .
	if(this->size() < this->getDefaultPopulationSize()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OA_EvolutionaryAlgorithm::selectBest(): Error!" << std::endl
				<< "Size of population is smaller than expected: " << this->size() << " / " << this->getDefaultPopulationSize() << std::endl
		);
	}
#endif /* DEBUG */

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next iteration finds a "standard" population. This
	// function will remove the last items.
	this->resize(this->getNParents() + this->getDefaultNChildren());

	// Everything should be back to normal ...
}


/******************************************************************************/
/**
  * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
  * iteration and sorting scheme, the start point will be different. The end-point is not meant
  * to be inclusive.
  *
  * @return The range inside which evaluation should take place
  */
std::tuple<std::size_t,std::size_t> GEvolutionaryAlgorithm::getEvaluationRange() const {
	// We evaluate all individuals in the first iteration This happens so pluggable
	// optimization monitors do not need to distinguish between algorithms, and
	// MUCOMMANU selection may fall back to MUPLUSNU in the first iteration.
	return std::tuple<std::size_t, std::size_t>(this->inFirstIteration() ? 0 : this->getNParents(), this->size());
}

/******************************************************************************/
/**
 * Does any necessary initialization work
  */
void GEvolutionaryAlgorithm::init() {
	// To be performed before any other action. Place any further work after this call.
	G_OptimizationAlgorithm_ParChild::init();

	// Initialize our thread pool
	m_tp_ptr.reset(new Gem::Common::GThreadPool(m_n_threads));
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GEvolutionaryAlgorithm::finalize() {
	// Check whether there were any errors during thread execution
	if (m_tp_ptr->hasErrors()) {
		std::vector<std::string> errors;
		errors = m_tp_ptr->getErrors();

		std::ostringstream error_list;
		for(auto e: errors) { error_list << e << std::endl; }

		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "========================================================================" << std::endl
				<< "In G_OA_EvolutionaryAlgorithm::finalize():" << std::endl
				<< "There were errors during thread execution:" << std::endl
				<< error_list.str()
				<< "========================================================================" << std::endl
		);
	}

	// Terminate our thread pool
	m_tp_ptr.reset();

	// Last action. Place any "local" finalization action before this call.
	G_OptimizationAlgorithm_ParChild::finalize();
}

/******************************************************************************/
/**
  * Retrieve a GPersonalityTraits object belonging to this algorithm
  */
std::shared_ptr<GPersonalityTraits> GEvolutionaryAlgorithm::getPersonalityTraits() const {
	return std::shared_ptr<GEvolutionaryAlgorithm_PersonalityTraits>(new GEvolutionaryAlgorithm_PersonalityTraits());
}

/******************************************************************************/
/**
  * Selection according to the pareto tag, also taking into account the parents of a population (i.e. in MUPLUSNU
  * mode). This is used in conjunction with multi-criterion optimization. See e.g.
  * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
  */
void GEvolutionaryAlgorithm::sortMuPlusNuParetoMode() {
	typename GEvolutionaryAlgorithm::iterator it, it_cmp;

	// We fall back to the single-eval MUPLUSNU mode if there is just one evaluation criterion
	it = this->begin();
	if (!(*it)->hasMultipleFitnessCriteria()) {
		this->sortMuPlusNuMode();
		return;
	}

	// Mark all individuals as being on the pareto front initially
	for (it = this->begin(); it != this->end(); ++it) {
		(*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->resetParetoTag();
	}

	// Compare all parameters
	for (it = this->begin(); it != this->end(); ++it) {
		for (it_cmp = it + 1; it_cmp != this->end(); ++it_cmp) {
			// If we already know that this individual is *not*
			// on the front we do not have to do any tests
			if (!(*it_cmp)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) continue;

			// Check if it dominates it_cmp. If so, mark it accordingly
			if (aDominatesB(*it, *it_cmp)) {
				(*it_cmp)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsNotOnParetoFront();
			}

			// If a it dominated by it_cmp, we mark it accordingly and break the loop
			if (aDominatesB(*it_cmp, *it)) {
				(*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsNotOnParetoFront();
				break;
			}
		}
	}

	// At this point we have tagged all individuals according to whether or not they are
	// on the pareto front. Lets sort them accordingly, bringing individuals with the
	// pareto tag to the front of the collection.
	sort(
		this->begin()
		, this->end()
		, [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) {
			return x->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront() > y->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront();
		}
	);

	// Count the number of individuals on the pareto front
	std::size_t nIndividualsOnParetoFront = 0;
	for (it = this->begin(); it != this->end(); ++it) {
		if ((*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) nIndividualsOnParetoFront++;
	}

	// If the number of individuals on the pareto front exceeds the number of parents, we
	// do not want to introduce a bias by selecting only the first nParent individuals. Hence
	// we randomly shuffle them. Note that not all individuals on the pareto front might survive,
	// as subsequent iterations will only take into account parents for the reproduction step.
	// If fewer individuals are on the pareto front than there are parents, then we want the
	// remaining parent positions to be filled up with the non-pareto-front individuals with
	// the best minOnly_fitness(0), i.e. with the best "master" fitness (transformed to take into
	// account minimization and maximization).
	if (nIndividualsOnParetoFront > this->getNParents()) {
		// randomly shuffle pareto-front individuals to avoid a bias
		std::random_shuffle(this->begin(), this->begin() + nIndividualsOnParetoFront);
	} else if (nIndividualsOnParetoFront < this->getNParents()) {
		// Sort the non-pareto-front individuals according to their master fitness
		std::partial_sort(
			this->begin() + nIndividualsOnParetoFront, this->begin() + this->m_n_parents, this->end(),
			[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				return x->minOnly_fitness() < y->minOnly_fitness();
			}
		);
	}

	// Finally, we sort the parents only according to their master fitness. This is meant
	// to give some sense to the value recombination scheme. It won't change much in case of the
	// random recombination scheme.
	std::sort(
		this->begin(), this->begin() + this->m_n_parents,
		[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);
}

/******************************************************************************/
/**
 * Selection according to the pareto tag, not taking into account the parents of a population (i.e. in MUCOMMANU
  * mode). This is used in conjunction with multi-criterion optimization. See e.g.
  * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
  */
void GEvolutionaryAlgorithm::sortMuCommaNuParetoMode() {
	typename GEvolutionaryAlgorithm::iterator it, it_cmp;

	// We fall back to the single-eval MUCOMMANU mode if there is just one evaluation criterion
	it = this->begin();
	if (!(*it)->hasMultipleFitnessCriteria()) {
		this->sortMuCommaNuMode();
		return;
	}

	// Mark the last iterations parents as not being on the pareto front
	for (it = this->begin(); it != this->begin() + this->m_n_parents; ++it) {
		(*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsNotOnParetoFront();
	}

	// Mark all children as being on the pareto front initially
	for (it = this->begin() + this->m_n_parents; it != this->end(); ++it) {
		(*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->resetParetoTag();
	}

	// Compare all parameters of all children
	for (it = this->begin() + this->m_n_parents; it != this->end(); ++it) {
		for (it_cmp = it + 1; it_cmp != this->end(); ++it_cmp) {
			// If we already know that this individual is *not*
			// on the front we do not have to do any tests
			if (!(*it_cmp)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) continue;

			// Check if it dominates it_cmp. If so, mark it accordingly
			if (aDominatesB(*it, *it_cmp)) {
				(*it_cmp)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsNotOnParetoFront();
			}

			// If a it dominated by it_cmp, we mark it accordingly and break the loop
			if (aDominatesB(*it_cmp, *it)) {
				(*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->setIsNotOnParetoFront();
				break;
			}
		}
	}

	// At this point we have tagged all children according to whether or not they are
	// on the pareto front. Lets sort them accordingly, bringing individuals with the
	// pareto tag to the front of the population. Note that parents have been manually
	// tagged as not being on the pareto front in the beginning of this function, so
	// sorting the individuals according to the pareto tag will move former parents out
	// of the parents section.
	sort(
		this->begin()
		, this->end()
		, [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) {
			return x->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront() > y->getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront();
		}
	);

	// Count the number of individuals on the pareto front
	std::size_t nIndividualsOnParetoFront = 0;
	for (it = this->begin(); it != this->end(); ++it) {
		if ((*it)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) nIndividualsOnParetoFront++;
	}

	// If the number of individuals on the pareto front exceeds the number of parents, we
	// do not want to introduce a bias by selecting only the first nParent individuals. Hence
	// we randomly shuffle them. Note that not all individuals on the pareto front might survive,
	// as subsequent iterations will only take into account parents for the reproduction step.
	// If fewer individuals are on the pareto front than there are parents, then we want the
	// remaining parent positions to be filled up with the non-pareto-front individuals with
	// the best minOnly_fitness(0), i.e. with the best "master" fitness, transformed to take into account
	// minimization and maximization. Note that, unlike MUCOMMANU_SINGLEEVAL
	// this implies the possibility that former parents are "elected" as new parents again. This
	// might be changed in subsequent versions of Geneva (TODO).
	if (nIndividualsOnParetoFront > this->getNParents()) {
		// randomly shuffle pareto-front individuals to avoid a bias
		std::random_shuffle(this->begin(), this->begin() + nIndividualsOnParetoFront);
	} else if (nIndividualsOnParetoFront < this->getNParents()) {
		// Sort the non-pareto-front individuals according to their master fitness
		std::partial_sort(
			this->begin() + nIndividualsOnParetoFront, this->begin() + this->m_n_parents, this->end(),
			[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				return x->minOnly_fitness() < y->minOnly_fitness();
			}
		);
	}

	// Finally, we sort the parents only according to their master fitness. This is meant
	// to give some sense to the value recombination scheme. It won't change much in case of the
	// random recombination scheme.
	std::sort(
		this->begin(), this->begin() + this->m_n_parents,
		[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);
}

/******************************************************************************/
/**
  * Determines whether the first individual dominates the second.
  *
  * @param a The individual that is assumed to dominate
  * @param b The individual that is assumed to be dominated
  * @return A boolean indicating whether the first individual dominates the second
  */
bool GEvolutionaryAlgorithm::aDominatesB(
	std::shared_ptr < GParameterSet > a
	, std::shared_ptr < GParameterSet > b
) const {
	std::size_t nCriteriaA = a->getNumberOfFitnessCriteria();

#ifdef DEBUG
	std::size_t nCriteriaB = b->getNumberOfFitnessCriteria();
	if(nCriteriaA != nCriteriaB) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OA_EvolutionaryAlgorithm::aDominatesB(): Error!" << std::endl
				<< "Number of fitness criteria differ: " << nCriteriaA << " / " << nCriteriaB << std::endl
		);
	}
#endif

	for (std::size_t i = 0; i < nCriteriaA; i++) {
		if (this->at(0)->isWorse(a->transformedFitness(i), b->transformedFitness(i))) return false;
	}

	return true;
}

/******************************************************************************/
/**
  * Applies modifications to this object. This is needed for testing purposes
  *
  * @return A boolean which indicates whether modifications were made
  */
bool GEvolutionaryAlgorithm::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (G_OptimizationAlgorithm_ParChild::modify_GUnitTests()) result = true;

	if(sortingMode::MUPLUSNU_SINGLEEVAL == this->getSortingScheme()) {
		this->setSortingScheme(sortingMode::MUCOMMANU_SINGLEEVAL);
	} else {
		this->setSortingScheme(sortingMode::MUPLUSNU_SINGLEEVAL);
	}
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("G_OA_EvolutionaryAlgorithm::modify_GUnitTests", "GEM_TESTING");
	return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Fills the collection with individuals.
  *
  * @param nIndividuals The number of individuals that should be added to the collection
  */
void GEvolutionaryAlgorithm::fillWithObjects(const std::size_t &nIndividuals) {
#ifdef GEM_TESTING
	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add some some
	for (std::size_t i = 0; i < nIndividuals; i++) {
		this->push_back(std::shared_ptr<Gem::Tests::GTestIndividual1>(new Gem::Tests::GTestIndividual1()));
	}

	// Make sure we have unique data items
	for(auto ind: *this) {
		ind->randomInit(activityMode::ALLPARAMETERS);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("G_OA_EvolutionaryAlgorithm::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to succeed. This is needed for testing purposes
  */
void GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_ParChild::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Call the parent class'es function
		std::shared_ptr<GEvolutionaryAlgorithm> p_test = this->template clone<GEvolutionaryAlgorithm>();

		// Fill p_test with individuals
		p_test->fillWithObjects(100);

		// Run the parent class'es tests
		p_test->G_OptimizationAlgorithm_ParChild::specificTestsNoFailureExpected_GUnitTests();
	}

	//------------------------------------------------------------------------------

	{ // Check setting and retrieval of the population size and number of parents/children
		std::shared_ptr <GEvolutionaryAlgorithm> p_test = this->template clone<GEvolutionaryAlgorithm>();

		// Set the default population size and number of children to different numbers
		for (std::size_t nChildren = 5; nChildren < 10; nChildren++) {
			for (std::size_t nParents = 1; nParents < nChildren; nParents++) {
				// Clear the collection
				BOOST_CHECK_NO_THROW(p_test->clear());

				// Add the required number of individuals
				p_test->fillWithObjects(nParents + nChildren);

				BOOST_CHECK_NO_THROW(p_test->setPopulationSizes(nParents + nChildren, nParents));

				// Check that the number of parents is as expected
				BOOST_CHECK_MESSAGE(p_test->getNParents() == nParents,
					"p_test->getNParents() == " << p_test->getNParents()
														 << ", nParents = " << nParents
														 << ", size = " << p_test->size());

				// Check that the actual number of children has the same value
				BOOST_CHECK_MESSAGE(p_test->getNChildren() == nChildren,
					"p_test->getNChildren() = " << p_test->getNChildren()
														 << ", nChildren = " << nChildren);
			}
		}
	}

	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to fail. This is needed for testing purposes
  */
void GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_ParChild::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
	Gem::Common::condnotset("GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

