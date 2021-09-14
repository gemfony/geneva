/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm.hpp"

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithm) // NOLINT

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
  * Searches for compliance with expectations with respect to another object
  * of the same type
  *
  * @param cp A constant reference to another GObject object
  * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GEvolutionaryAlgorithm::compare_(
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
	Gem::Common::compare_base_t<G_OptimizationAlgorithm_ParChild>(*this, *p_load, token);

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
void GEvolutionaryAlgorithm::resetToOptimizationStart_() {
	// There is no more work to be done here, so we simply call the
	// function of the parent class
	G_OptimizationAlgorithm_ParChild::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm.
 *
 * @return The type of optimization algorithm
 */
std::string GEvolutionaryAlgorithm::getAlgorithmPersonalityType_() const {
	return std::string("PERSONALITY_EA");
}

/******************************************************************************/
/**
  * Returns the name of this optimization algorithm
  *
  * @return The name assigned to this optimization algorithm
  */
std::string GEvolutionaryAlgorithm::getAlgorithmName_() const {
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

	for(const auto& ind_ptr: *this) {
		if(ind_ptr->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) {
			paretoInds.push_back(ind_ptr);
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
void GEvolutionaryAlgorithm::updateGlobalBestsPQ_(
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
			G_OptimizationAlgorithm_Base::updateGlobalBestsPQ_(bestIndividuals);
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
void GEvolutionaryAlgorithm::updateIterationBestsPQ_(
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
			G_OptimizationAlgorithm_Base::updateIterationBestsPQ_(bestIndividuals);
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
void GEvolutionaryAlgorithm::addConfigurationOptions_ (
	Gem::Common::GParserBuilder& gpb
)
{
	// Call our parent class'es function
	G_OptimizationAlgorithm_ParChild::addConfigurationOptions_(gpb);

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
std::string GEvolutionaryAlgorithm::name_() const {
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
		glogger
			<< "In GEvolutionaryAlgorithm::setNThreads(nThreads):" << std::endl
			<< "nThreads == 0 was requested. nThreads was reset to the default "
			<< DEFAULTNSTDTHREADS << std::endl
			<< GWARNING;

		m_n_threads = DEFAULTNSTDTHREADS;
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
void GEvolutionaryAlgorithm::populationSanityChecks_() const {
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
void GEvolutionaryAlgorithm::adaptChildren_() {
	// Retrieve the range of individuals to be adapted
	std::tuple<std::size_t, std::size_t> range = this->getAdaptionRange();

	// Will hold the future objects generated by the async_schedule call
	std::vector<std::future<std::size_t>> futures_cnt;

	// Loop over all requested individuals and perform the adaption
	for (auto it = (this->begin() + std::get<0>(range)); it != (this->begin() + std::get<1>(range)); ++it) {
		futures_cnt.push_back(
			m_tp_ptr->async_schedule(
				// Note: may not pass it as a reference, as it is a local variable in the loop and might
				// vanish or have been altered once the thread has started and adaption is requested.
				[it]() { return (*it)->adapt(); } // Returns the number of adaptions
			)
		);
	}

	// Wait for all threads in the pool to complete their work
    m_tp_ptr->drain_queue();

#ifdef DEBUG
	// Check for errors
	for(auto& f: futures_cnt) {
		try {
			f.get();
		} catch(std::exception& e) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSimulatedAnnealing::adaptChildren() :" << std::endl
					<< "Got error during thread execution with message:" << std::endl
					<< e.what() << std::endl
			);
		} catch(...) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GSimulatedAnnealing::adaptChildren() :" << std::endl
					<< "Got unknown exception during thread execution" << std::endl
			);
		}
	}
#endif
}

/******************************************************************************/
/**
  * We submit individuals to the broker connector and wait for processed items.
 */
void GEvolutionaryAlgorithm::runFitnessCalculation_() {
	//--------------------------------------------------------------------------------
	// Start by marking the work to be done in the individuals.
	// "range" will hold the start- and end-points of the range
	// to be worked on
	std::tuple<std::size_t, std::size_t> range = getEvaluationRange_();

#ifdef DEBUG
	// There should be no situation in which a "clean" child is submitted
	// through this function. There MAY be situations, where in the first iteration
	// parents are clean, e.g. when they were extracted from another optimization.
	for(std::size_t i=this->getNParents(); i<this->size(); i++) {
		if(not this->at(i)->is_due_for_processing()) {
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
	// Set the "DO_PROCESS" flag in all required work items, the "DO_IGNORE" flag in all others.

	setProcessingFlag(this->m_data_cnt, range);

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results.
	auto status = this->workOn(
		this->m_data_cnt
		, false // do not resubmit unprocessed items
		, "GEvolutionaryAlgorithm::runFitnessCalculation()"
	);

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
			<< "In GEvolutionaryAlgorithm::runFitnessCalculation(): " << std::endl
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
			<< "In GEvolutionaryAlgorithm::runFitnessCalculation(): " << std::endl
			<< "Removed " << n_erased << " erroneous work items in iteration " << this->getIteration() << std::endl
			<< GLOGGING;
#endif
	}

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
	for(const auto& item_ptr: old_work_items) {
		this->push_back(item_ptr);
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
				<< "We have a size of " << this->size() << " with " << this->getNParents() << " parents" << std::endl
				<< "We need to fill up the population with clones from parent individuals" << std::endl
				<< GWARNING;
		}
	}

	// Check that the last individual is not unprocessed. This is a severe error.
	if(this->back()->is_due_for_processing()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GEvolutionaryAlgorithm::fixAfterJobSubmission(): Error!" << std::endl
				<< "The last individual in the population is is unprocessed" << std::endl
				<< "so we cannot use it for cloning" << std::endl
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
void GEvolutionaryAlgorithm::selectBest_() {
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
			if (1==m_n_parents || this->inFirstIteration()) {
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
std::tuple<std::size_t,std::size_t> GEvolutionaryAlgorithm::getEvaluationRange_() const {
	// We evaluate all individuals in the first iteration This happens so pluggable
	// optimization monitors do not need to distinguish between algorithms, and
	// MUCOMMANU selection may fall back to MUPLUSNU in the first iteration.
	return std::make_tuple<std::size_t, std::size_t>(this->inFirstIteration() ? std::size_t(0) : this->getNParents(), this->size());
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
	// Terminate our thread pool
	m_tp_ptr.reset();

	// Last action. Place any "local" finalization action before this call.
	G_OptimizationAlgorithm_ParChild::finalize();
}

/******************************************************************************/
/**
  * Retrieve a GPersonalityTraits object belonging to this algorithm
  */
std::shared_ptr<GPersonalityTraits> GEvolutionaryAlgorithm::getPersonalityTraits_() const {
	return std::shared_ptr<GEvolutionaryAlgorithm_PersonalityTraits>(new GEvolutionaryAlgorithm_PersonalityTraits());
}

/******************************************************************************/
/**
 * Selection, MUPLUSNU_SINGLEEVAL style. Note that not all individuals of the population (including parents)
 * are sorted -- only the nParents best individuals are identified. The quality of the population can only
 * increase, but the optimization will stall more easily in MUPLUSNU_SINGLEEVAL mode.
 */
void GEvolutionaryAlgorithm::sortMuPlusNuMode() {
#ifdef DEBUG
	// Check that we do not accidently trigger value calculation
	std::size_t pos = 0;
	for(auto const & ind_ptr: *this) {
		if(ind_ptr->is_due_for_processing()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GEvolutionaryAlgorithm::sortMuplusnuMode(): Error!" << std::endl
					<< "In iteration " << G_OptimizationAlgorithm_Base::getIteration() << ": Found individual in position " << pos << std::endl
					<< " that is unprocessed." << std::endl
			);
		}
		pos++;
	}
#endif /* DEBUG */

	// Only partially sort the arrays
	std::partial_sort(
		G_OptimizationAlgorithm_Base::m_data_cnt.begin()
		, G_OptimizationAlgorithm_Base::m_data_cnt.begin() + m_n_parents
		, G_OptimizationAlgorithm_Base::m_data_cnt.end()
		, [](std::shared_ptr<GParameterSet> x_ptr, std::shared_ptr<GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);
}

/******************************************************************************/
/**
 * Selection, MUCOMMANU_SINGLEEVAL style. New parents are selected from children only. The quality
 * of the population may decrease occasionally from generation to generation, but the
 * optimization is less likely to stall.
 */
void GEvolutionaryAlgorithm::sortMuCommaNuMode() {
#ifdef DEBUG
	if (G_OptimizationAlgorithm_Base::inFirstIteration()) {
		// Check that we do not accidentally trigger value calculation -- check the whole range
		typename GEvolutionaryAlgorithm::iterator it;
		for (it = this->begin(); it != this->end(); ++it) {
			if ((*it)->is_due_for_processing()) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GEvolutionaryAlgorithm::sortMucommanuMode(): Error!" << std::endl
						<< "In iteration " << G_OptimizationAlgorithm_Base::getIteration() << ": Found individual in position " << std::distance(
						this->begin()
						, it
					) << std::endl
						<< " whose dirty flag is set." << std::endl
				);
			}
		}
	} else {
		// Check that we do not accidentally trigger value calculation -- check children only
		typename GEvolutionaryAlgorithm::iterator it;
		for (it = this->begin() + m_n_parents; it != this->end(); ++it) {
			if ((*it)->is_due_for_processing()) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GEvolutionaryAlgorithm::sortMucommanuMode(): Error!" << std::endl
						<< "In iteration " << G_OptimizationAlgorithm_Base::getIteration() << ": Found individual in position " << std::distance(
						this->begin()
						, it
					) << std::endl
						<< " which is unprocessed." << std::endl
				);
			}
		}
	}
#endif /* DEBUG */

	// Only sort the children
	std::partial_sort(
		G_OptimizationAlgorithm_Base::m_data_cnt.begin() + m_n_parents
		, G_OptimizationAlgorithm_Base::m_data_cnt.begin() + 2 * m_n_parents
		, G_OptimizationAlgorithm_Base::m_data_cnt.end()
		, [](std::shared_ptr<GParameterSet> x_ptr, std::shared_ptr<GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);

	std::swap_ranges(
		G_OptimizationAlgorithm_Base::m_data_cnt.begin()
		, G_OptimizationAlgorithm_Base::m_data_cnt.begin() + m_n_parents
		, G_OptimizationAlgorithm_Base::m_data_cnt.begin() + m_n_parents
	);
}

/******************************************************************************/
/**
 * Selection, MUNU1PRETAIN_SINGLEEVAL style. This is a hybrid between MUPLUSNU_SINGLEEVAL and MUCOMMANU_SINGLEEVAL
 * mode. If a better child was found than the best parent of the last generation,
 * all former parents are replaced. If no better child was found than the best
 * parent of the last generation, then this parent stays in place. All other parents
 * are replaced by the (nParents_-1) best children. The scheme falls back to MUPLUSNU_SINGLEEVAL
 * mode, if only one parent is available, or if this is the first generation (so we
 * do not accidentally trigger value calculation).
 */
void GEvolutionaryAlgorithm::sortMunu1pretainMode() {
#ifdef DEBUG
	// Check that we do not accidentally trigger value calculation
	typename GEvolutionaryAlgorithm::iterator it;
	for(it=this->begin()+m_n_parents; it!=this->end(); ++it) {
		if((*it)->is_due_for_processing()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GEvolutionaryAlgorithm::sortMunu1pretainMode(): Error!" << std::endl
					<< "In iteration " << G_OptimizationAlgorithm_Base::getIteration() << ": Found individual in position " << std::distance(this->begin(),it) << std::endl
					<< " whose dirty flag is set." << std::endl
			);
		}
	}
#endif /* DEBUG */

	// Sort the children
	std::partial_sort(
		G_OptimizationAlgorithm_Base::m_data_cnt.begin() + m_n_parents
		, G_OptimizationAlgorithm_Base::m_data_cnt.begin() + 2*m_n_parents
		, G_OptimizationAlgorithm_Base::m_data_cnt.end()
		, [](std::shared_ptr<GParameterSet> x_ptr, std::shared_ptr<GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);

	// Retrieve the best child's and the last generation's best parent's fitness
	double bestTranformedChildFitness_MinOnly  = minOnly_transformed_fitness(
		*(G_OptimizationAlgorithm_Base::m_data_cnt.begin() + m_n_parents));
	double bestTranformedParentFitness_MinOnly = minOnly_transformed_fitness(*(G_OptimizationAlgorithm_Base::m_data_cnt.begin()));

	// Leave the best parent in place, if no better child was found
	if(bestTranformedChildFitness_MinOnly < bestTranformedParentFitness_MinOnly) { // A better child was found. Overwrite all parents
		std::swap_ranges(
			G_OptimizationAlgorithm_Base::m_data_cnt.begin()
			,G_OptimizationAlgorithm_Base::m_data_cnt.begin()+m_n_parents
			,G_OptimizationAlgorithm_Base::m_data_cnt.begin()+m_n_parents
		);
	} else {
		std::swap_ranges(
			G_OptimizationAlgorithm_Base::m_data_cnt.begin()+1
			,G_OptimizationAlgorithm_Base::m_data_cnt.begin()+m_n_parents
			,G_OptimizationAlgorithm_Base::m_data_cnt.begin()+m_n_parents
		);
	}
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
	if (not (*it)->hasMultipleFitnessCriteria()) {
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
			if (not (*it_cmp)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) continue;

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
		std::shuffle(this->begin(), this->begin() + nIndividualsOnParetoFront, this->m_gr);
	} else if (nIndividualsOnParetoFront < this->getNParents()) {
		// Sort the non-pareto-front individuals according to their master fitness
		std::partial_sort(
			this->begin() + nIndividualsOnParetoFront, this->begin() + this->m_n_parents, this->end(),
			[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
				return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
			}
		);
	}

	// Finally, we sort the parents only according to their master fitness. This is meant
	// to give some sense to the value recombination scheme. It won't change much in case of the
	// random recombination scheme.
	std::sort(
		this->begin(), this->begin() + this->m_n_parents,
		[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
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
	if (not (*it)->hasMultipleFitnessCriteria()) {
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
			if (not (*it_cmp)->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()->isOnParetoFront()) continue;

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
		std::shuffle(this->begin(), this->begin() + nIndividualsOnParetoFront, this->m_gr);
	} else if (nIndividualsOnParetoFront < this->getNParents()) {
		// Sort the non-pareto-front individuals according to their master fitness
		std::partial_sort(
			this->begin() + nIndividualsOnParetoFront, this->begin() + this->m_n_parents, this->end(),
			[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
				return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
			}
		);
	}

	// Finally, we sort the parents only according to their master fitness. This is meant
	// to give some sense to the value recombination scheme. It won't change much in case of the
	// random recombination scheme.
	std::sort(
		this->begin(), this->begin() + this->m_n_parents,
		[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);
}

/******************************************************************************/
/**
  * Determines whether the first individual dominates the second.
  *
  * @param x_ptr The individual that is assumed to dominate
  * @param y_ptr The individual that is assumed to be dominated
  * @return A boolean indicating whether the first individual dominates the second
  */
bool GEvolutionaryAlgorithm::aDominatesB(
	std::shared_ptr<GParameterSet> x_ptr
	, std::shared_ptr<GParameterSet> y_ptr
) const {
	std::size_t nCriteriaX = x_ptr->getNStoredResults();

#ifdef DEBUG
	std::size_t nCriteriaY = y_ptr->getNStoredResults();
	if(nCriteriaX != nCriteriaY) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OA_EvolutionaryAlgorithm::aDominatesB(): Error!" << std::endl
				<< "Number of fitness criteria differ: " << nCriteriaX << " / " << nCriteriaY << std::endl
		);
	}
#endif

	// x dominates y, if none of its fitness criteria is worse than the
	// corresponding criterion from y
	for (std::size_t i = 0; i < nCriteriaX; i++) {
		if(isWorse(x_ptr, y_ptr)) return false;
	}

	return true;
}

/******************************************************************************/
/**
  * Applies modifications to this object. This is needed for testing purposes
  *
  * @return A boolean which indicates whether modifications were made
  */
bool GEvolutionaryAlgorithm::modify_GUnitTests_() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (G_OptimizationAlgorithm_ParChild::modify_GUnitTests_()) result = true;

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
	for(const auto& ind_ptr: *this) {
		ind_ptr->randomInit(activityMode::ALLPARAMETERS);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("G_OA_EvolutionaryAlgorithm::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to succeed. This is needed for testing purposes
  */
void GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_ParChild::specificTestsNoFailureExpected_GUnitTests_();

	//------------------------------------------------------------------------------

	{ // Call the parent class'es function
		std::shared_ptr<GEvolutionaryAlgorithm> p_test = this->template clone<GEvolutionaryAlgorithm>();

		// Fill p_test with individuals
		p_test->fillWithObjects(100);

		// Run the parent class'es tests
		p_test->G_OptimizationAlgorithm_ParChild::specificTestsNoFailureExpected_GUnitTests_();
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
void GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_ParChild::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */
	Gem::Common::condnotset("GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

