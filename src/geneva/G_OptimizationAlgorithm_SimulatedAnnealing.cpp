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

#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing.hpp"

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSimulatedAnnealing) // NOLINT

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
GSimulatedAnnealing::GSimulatedAnnealing()
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
void GSimulatedAnnealing::compare_(
	const GObject& cp // the other object
	, const Gem::Common::expectation& e // the expectation for this object, e.g. equality
	, const double& limit// the limit for allowed deviations of floating point types
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
	const GSimulatedAnnealing *p_load
		= Gem::Common::g_convert_and_compare<GObject, GSimulatedAnnealing>(cp, this);

	GToken token("GSimulatedAnnealing", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<G_OptimizationAlgorithm_ParChild>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(m_t0, p_load->m_t0), token);
	compare_t(IDENTITY(m_t, p_load->m_t), token);
	compare_t(IDENTITY(m_alpha, p_load->m_alpha), token);
	compare_t(IDENTITY(m_n_threads, p_load->m_n_threads), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GSimulatedAnnealing::resetToOptimizationStart_() {
	// Reset the temperature
	m_t = m_t0;

	// There is no more work to be done here, so we simply call the
	// function of the parent class
	G_OptimizationAlgorithm_ParChild::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GSimulatedAnnealing::getAlgorithmPersonalityType_() const {
	return std::string("PERSONALITY_SA");
}

/******************************************************************************/
/**
  * Returns the name of this optimization algorithm
  *
  * @return The name assigned to this optimization algorithm
  */
std::string GSimulatedAnnealing::getAlgorithmName_() const {
	return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
  * Adds local configuration options to a GParserBuilder object
  *
  * @param gpb The GParserBuilder object to which configuration options should be added
  */
void GSimulatedAnnealing::addConfigurationOptions_ (
	Gem::Common::GParserBuilder& gpb
) {
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

	gpb.registerFileParameter<double>(
		"t0" // The name of the variable
		, SA_T0 // The default value
		, [this](double sat0) { this->setT0(sat0); }
	)
		<< "The start temperature used in simulated annealing";

	gpb.registerFileParameter<double>(
		"alpha" // The name of the variable
		, SA_ALPHA // The default value
		, [this](double ds) { this->setTDegradationStrength(ds); }
	)
		<< "The degradation strength used in the cooling" << std::endl
		<< "schedule in simulated annealing;";
}

/******************************************************************************/
/**
 * Sets the number of threads this population uses for adaption. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the number of hardware
 * threading units (e.g. number of cores or hyperthreading units).
  *
  * @param nThreads The number of threads this class uses
  */
void GSimulatedAnnealing::setNThreads(std::uint16_t nThreads) {
	if (nThreads == 0) {
		glogger
			<< "In GSimulatedAnnealing::setNThreads(nThreads):" << std::endl
			<< "nThreads == 0 was requested. m_n_threads was reset to the default "
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
std::uint16_t GSimulatedAnnealing::getNThreads() const {
	return m_n_threads;
}

/******************************************************************************/
/**
  * Determines the strength of the temperature degradation. This function is used for simulated annealing.
  *
  * @param alpha The degradation speed of the temperature
  */
void GSimulatedAnnealing::setTDegradationStrength(double alpha) {
	if (alpha <= 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSimulatedAnnealing::setTDegradationStrength(const double&):" << std::endl
				<< "Got negative alpha: " << alpha << std::endl
		);
	}

	m_alpha = alpha;
}

/******************************************************************************/
/**
  * Retrieves the temperature degradation strength. This function is used for simulated annealing.
  *
  * @return The temperature degradation strength
  */
double GSimulatedAnnealing::getTDegradationStrength() const {
	return m_alpha;
}

/******************************************************************************/
/**
  * Sets the start temperature. This function is used for simulated annealing.
  *
  * @param t0 The start temperature
  */
void GSimulatedAnnealing::setT0(double t0) {
	if (t0 <= 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSimulatedAnnealing::setT0(const double&):" << std::endl
				<< "Got negative start temperature: " << t0 << std::endl
		);
	}

	m_t0 = t0;
}

/******************************************************************************/
/**
  * Retrieves the start temperature. This function is used for simulated annealing.
  *
  * @return The start temperature
  */
double GSimulatedAnnealing::getT0() const {
	return m_t0;
}

/******************************************************************************/
/**
  * Retrieves the current temperature. This function is used for simulated annealing.
  *
  * @return The current temperature
  */
double GSimulatedAnnealing::getT() const {
	return m_t;
}

/******************************************************************************/
/**
  * Emits a name for this class / object
  */
std::string GSimulatedAnnealing::name_() const {
	return std::string("GSimulatedAnnealing");
}

/******************************************************************************/
/**
  * Loads the data of another GSimulatedAnnealingT object, camouflaged as a GObject.
  *
  * @param cp A pointer to another GSimulatedAnnealingT object, camouflaged as a GObject
  */
void GSimulatedAnnealing::load_(const GObject *cp) {
	// Check that we are dealing with a GSimulatedAnnealing reference independent
	// of this object and convert the pointer
	const GSimulatedAnnealing *p_load = Gem::Common::g_convert_and_compare<GObject, GSimulatedAnnealing>(cp, this);

	// First load the parent class'es data ...
	G_OptimizationAlgorithm_ParChild::load_(cp);

	// ... and then our own data
	m_t0 = p_load->m_t0;
	m_t = p_load->m_t;
	m_alpha = p_load->m_alpha;
	m_n_threads = p_load->m_n_threads;
}

/******************************************************************************/
/**
  * Creates a deep copy of this object
  *
  * @return A deep copy of this object
  */
GObject * GSimulatedAnnealing::clone_() const {
	return new GSimulatedAnnealing(*this);
}

/******************************************************************************/
/**
  * Some error checks related to population sizes
  */
void GSimulatedAnnealing::populationSanityChecks_() const {
	// First check that we have been given a suitable value for the number of parents.
	// Note that a number of checks (e.g. population size != 0) has already been done
	// in the parent class.
	if (this->m_n_parents == 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSimulatedAnnealing::populationSanityChecks(): Error!" << std::endl
				<< "Number of parents is set to 0"
		);
	}

	// We need at least as many children as parents
	std::size_t popSize = this->getPopulationSize();
	if (popSize <= this->m_n_parents) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSimulatedAnnealing::populationSanityChecks() :" << std::endl
				<< "Requested size of population is too small :" << popSize << " " << this->m_n_parents << std::endl
		);
	}
}

/******************************************************************************/
/**
  * Adapt all children in parallel. Evaluation is done in a separate function (runFitnessCalculation).
  */
void GSimulatedAnnealing::adaptChildren_() {
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
	m_tp_ptr->wait();

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
void GSimulatedAnnealing::runFitnessCalculation_() {
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
					<< "In GSimulatedAnnealing::runFitnessCalculation(): Error!" << std::endl
					<< "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
					<< "but found \"clean\" individual in position " << i << std::endl
			);
		}
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
		, "GSimulatedAnnealing::runFitnessCalculation()"
	);

	//--------------------------------------------------------------------------------
	// Take care of unprocessed items, if these exist. We simply remove them and continue.
	if(not status.is_complete) {
		std::size_t n_erased = Gem::Common::erase_if(
			this->m_data_cnt
			, [this](std::shared_ptr<GParameterSet> p) -> bool {
				return (p->getProcessingStatus() == Gem::Courtier::processingStatus::DO_PROCESS);
			}
		);

#ifdef DEBUG
		glogger
			<< "In GSimulatedAnnealing::runFitnessCalculation(): " << std::endl
			<< "Removed " << n_erased << " unprocessed work items in iteration " << this->getIteration() << std::endl
			<< GLOGGING;
#endif
	}

	// Remove items for which an error has occurred during processing
	// We simply remove them and continue.
	if(status.has_errors) {
		std::size_t n_erased = Gem::Common::erase_if(
			this->m_data_cnt
			, [this](std::shared_ptr<GParameterSet> p) -> bool {
				return p->has_errors();
			}
		);

#ifdef DEBUG
		glogger
			<< "In GSimulatedAnnealing::runFitnessCalculation(): " << std::endl
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
 */
void GSimulatedAnnealing::fixAfterJobSubmission() {
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
				return x->getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->isParent() && x->getAssignedIteration() != iteration;
			}
		)
		, old_work_items.end()
	);

	// Make it known to remaining old individuals that they are now part of a new iteration
	std::for_each(
		old_work_items.begin(), old_work_items.end(),
		[iteration](std::shared_ptr <GParameterSet> p) { p->setAssignedIteration(iteration); }
	);

	// Make sure that parents are at the beginning of the array.
	sort(
		this->begin()
		, this->end()
		, [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) -> bool {
			return (x->getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->isParent() > y->getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->isParent());
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
				<< "In GSimulatedAnnealing::fixAfterJobSubmission(): Error!" << std::endl
				<< "Population holds no data" << std::endl
		);
	} else {
		// Emit a warning if no children have returned
		if(this->size() <= this->getNParents()) {
			glogger
				<< "In GSimulatedAnnealing::fixAfterJobSubmission(): Warning!" << std::endl
				<< "No child individuals have returned" << std::endl
				<< "We need to fill up the population with clones from parent individuals" << std::endl
				<< GWARNING;
		}
	}

	// Check that the last individual is not unprocessed. This is a severe error.
	if(this->back()->is_due_for_processing()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSimulatedAnnealing::fixAfterJobSubmission():" << std::endl
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
	typename G_OptimizationAlgorithm_Base::iterator it;
	for (it = this->begin(); it != this->begin() + np; ++it) {
		(*it)->GParameterSet::template getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->setIsParent();
	}
	for (it = this->begin() + np; it != this->end(); ++it) {
		(*it)->GParameterSet::template getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->setIsChild();
	}

	// We care for too many returned individuals in the selectBest() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.
}

/******************************************************************************/
/**
  * Choose new parents, based on the SA selection scheme.
  */
void GSimulatedAnnealing::selectBest_() {
	// Sort according to the "Simulated Annealing" scheme
	sortSAMode();

	// Let parents know they are parents
	this->markParents();

#ifdef DEBUG
	// Make sure our population is not smaller than its nominal size -- this
	// should have been taken care of in fixAfterJobSubmission() .
	if(this->size() < this->getDefaultPopulationSize()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSimulatedAnnealing::selectBest(): Error!" << std::endl
				<< "Size of population is smaller than expected: " << this->size() << " / " << this->getDefaultPopulationSize() << std::endl
		);
	}
#endif /* DEBUG */

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next iteration finds a "standard" population. This
	// function will remove the last items.
	this->resize(this->getNParents() + this->getDefaultNChildren());

	// Let children know they are children
	this->markChildren();

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
std::tuple<std::size_t,std::size_t> GSimulatedAnnealing::getEvaluationRange_() const {
	// We evaluate all individuals in the first iteration This happens so pluggable
	// optimization monitors do not need to distinguish between algorithms
	return std::tuple<std::size_t,std::size_t>{this->inFirstIteration() ? 0 : this->getNParents(), this->size()};
}

/******************************************************************************/
/**
 * Does any necessary initialization work
  */
void GSimulatedAnnealing::init() {
	// To be performed before any other action. Place any further work after this call.
	G_OptimizationAlgorithm_ParChild::init();

	// Initialize our thread pool
	m_tp_ptr.reset(new Gem::Common::GThreadPool(m_n_threads));
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GSimulatedAnnealing::finalize() {
	// Terminate our thread pool
	m_tp_ptr.reset();

	// Last action. Place any "local" finalization action before this call.
	G_OptimizationAlgorithm_ParChild::finalize();
}

/******************************************************************************/
/**
  * Retrieve a GPersonalityTraits object belonging to this algorithm
  */
std::shared_ptr<GPersonalityTraits> GSimulatedAnnealing::getPersonalityTraits_() const {
	return std::shared_ptr<GSimulatedAnnealing_PersonalityTraits>(new GSimulatedAnnealing_PersonalityTraits());
}

/******************************************************************************/
/**
 * Performs a simulated annealing style sorting and selection
 */
void GSimulatedAnnealing::sortSAMode() {
	// Position the nParents best children of the population right behind the parents
	std::partial_sort(
		this->begin() + this->m_n_parents, this->begin() + 2 * this->m_n_parents, this->end(),
		[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);

	// Check for each parent whether it should be replaced by the corresponding child
	for (std::size_t np = 0; np < this->m_n_parents; np++) {
		double pPass = saProb(
			minOnly_transformed_fitness(this->at(np))
			, minOnly_transformed_fitness(this->at(this->m_n_parents + np))
		);
		if (pPass >= 1.) {
			this->at(np)->GObject::load(this->at(this->m_n_parents + np));
		} else {
			double challenge =
				this->m_uniform_real_distribution(this->m_gr, std::uniform_real_distribution<double>::param_type(0.,1.));
			if (challenge < pPass) {
				this->at(np)->GObject::load(this->at(this->m_n_parents + np));
			}
		}
	}

	// Sort the new parents -- it is possible that a child with a worse fitness has replaced a parent
	std::sort(
		this->begin(), this->begin() + this->m_n_parents,
		[](std::shared_ptr <GParameterSet> x_ptr, std::shared_ptr <GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);

	// Make sure the temperature gets updated
	updateTemperature();
}

/******************************************************************************/
/**
  * Calculates the simulated annealing probability for a child to replace a parent.
  * Note that this function only sees minimization problems, as maximization problems
  * are transformed to minimization problems inside of GParameterSet.
  *
  * @param fMinOnlyParent The "min only" fitness of the parent
  * @param fMinOnlyChild The "min only" fitness of the child
  * @return A double value in the range [0,1[, representing the likelihood for the child to replace the parent
  */
double GSimulatedAnnealing::saProb(
	const double &fMinOnlyParent
	, const double &fMinOnlyChild
) {
	return exp(-(fMinOnlyChild - fMinOnlyParent) / m_t);
}

/******************************************************************************/
/**
  * Updates the temperature. This function is used for simulated annealing.
  */
void GSimulatedAnnealing::updateTemperature() {
	m_t *= m_alpha;
}

/******************************************************************************/
/**
  * Applies modifications to this object. This is needed for testing purposes
  *
  * @return A boolean which indicates whether modifications were made
  */
bool GSimulatedAnnealing::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if(G_OptimizationAlgorithm_ParChild::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSimulatedAnnealing::modify_GUnitTests", "GEM_TESTING");
		 return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to succeed. This is needed for testing purposes
  */
void GSimulatedAnnealing::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_ParChild::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSimulatedAnnealing::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to fail. This is needed for testing purposes
  */
void GSimulatedAnnealing::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_ParChild::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */
	Gem::Common::condnotset("GSimulatedAnnealing::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
