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

#include "geneva/G_OptimizationAlgorithm_Base.hpp"

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GSerialExecutorT<Gem::Geneva::GParameterSet>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GMTExecutorT<Gem::Geneva::GParameterSet>) // NOLINT

/******************************************************************************/


namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBasePluggableOM::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBasePluggableOM reference independent of this object and convert the pointer
	const GBasePluggableOM *p_load = Gem::Common::g_convert_and_compare<GObject, GBasePluggableOM>(cp, this);

	GToken token("GBasePluggableOM", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GObject>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_useRawEvaluation, p_load->m_useRawEvaluation), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Allows to set the m_useRawEvaluation variable
 */
void GBasePluggableOM::setUseRawEvaluation(bool useRaw) {
	m_useRawEvaluation = useRaw;
}

/******************************************************************************/
/**
 * Allows to retrieve the value of the m_useRawEvaluation variable
 */
bool GBasePluggableOM::getUseRawEvaluation() const {
	return m_useRawEvaluation;
}

/******************************************************************************/
/**
 * Access tp information about the current iteration. This is a wrapper
 * function to avoid public virtual.
 */
void GBasePluggableOM::informationFunction(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	informationFunction_(im, goa);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GBasePluggableOMT object, camouflaged as a GObject
 */
void GBasePluggableOM::load_(const GObject* cp) {
	// Check that we are dealing with a GBasePluggableOM reference independent of this object and convert the pointer
	const GBasePluggableOM *p_load = Gem::Common::g_convert_and_compare<GObject, GBasePluggableOM>(cp, this);

	// Load the parent classes' data ...
	GObject::load_(cp);

	// ... and then our local data
	m_useRawEvaluation = p_load->m_useRawEvaluation;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBasePluggableOM::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests_()) result = true;

	this->setUseRawEvaluation(!this->getUseRawEvaluation());
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBasePluggableOM", "GEM_TESTING");
			return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBasePluggableOM::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The copy constructor. Note that the executor is neither copied nor cloned.
 * You need to register your own executor or let the algorithm use the default executor.
 *
 * @param cp A constant reference to another G_OptimizationAlgorithm_Base object
 */
G_OptimizationAlgorithm_Base::G_OptimizationAlgorithm_Base(const G_OptimizationAlgorithm_Base& cp)
	: GObject(cp)
	  , Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>(cp)
	  , m_iteration(cp.m_iteration)
	  , m_offset(DEFAULTOFFSET)
	  , m_minIteration(cp.m_minIteration)
	  , m_maxIteration(cp.m_maxIteration)
	  , m_maxStallIteration(cp.m_maxStallIteration)
	  , m_reportIteration(cp.m_reportIteration)
	  , m_nRecordbestGlobalIndividuals(cp.m_nRecordbestGlobalIndividuals)
	  , m_bestGlobalIndividuals_pq(cp.m_bestGlobalIndividuals_pq)
	  , m_bestIterationIndividuals_pq(cp.m_bestIterationIndividuals_pq)
	  , m_defaultPopulationSize(cp.m_defaultPopulationSize)
	  , m_bestKnownPrimaryFitness(cp.m_bestKnownPrimaryFitness)
	  , m_bestCurrentPrimaryFitness(cp.m_bestCurrentPrimaryFitness)
	  , m_stallCounter(cp.m_stallCounter)
	  , m_stallCounterThreshold(cp.m_stallCounterThreshold)
	  , m_cp_interval(cp.m_cp_interval)
	  , m_cp_base_name(cp.m_cp_base_name)
	  , m_cp_directory_path(cp.m_cp_directory_path)
	  , m_cp_last(cp.m_cp_last)
	  , m_cp_remove(cp.m_cp_remove)
	  , m_cp_serialization_mode(cp.m_cp_serialization_mode)
	  , m_qualityThreshold(cp.m_qualityThreshold)
	  , m_hasQualityThreshold(cp.m_hasQualityThreshold)
	  , m_maxDuration(cp.m_maxDuration)
	  , m_minDuration(cp.m_minDuration)
	  , m_terminationFile(cp.m_terminationFile)
	  , m_terminateOnFileModification(cp.m_terminateOnFileModification)
	  , m_emitTerminationReason(cp.m_emitTerminationReason)
	  , m_worstKnownValids_cnt(cp.m_worstKnownValids_cnt)
	  , m_default_execMode(cp.m_default_execMode)
	  , m_default_executor_config(cp.m_default_executor_config)
{
	// Copy atomics over
	m_halted.store(cp.m_halted.load());

	// Copy the executor over
	Gem::Common::copyCloneableSmartPointer(cp.m_executor_ptr, m_executor_ptr);

	// Copy the pluggable optimization monitors over (if any)
	Gem::Common::copyCloneableSmartPointerContainer(cp.m_pluggable_monitors_cnt, m_pluggable_monitors_cnt);
}

/******************************************************************************/
/**
 * Performs the necessary administratory work of doing check-pointing. Special
 * work necessary for a given optimization algorithm may be performed in the
 * virtual function saveCheckpoint(), which is called by this function.
 *
 * @param better A boolean which indicates whether a better result was found
 */
void G_OptimizationAlgorithm_Base::checkpoint(bool is_better) const {
	bool do_save = false;

	// Determine a suitable name for the checkpoint file
	std::filesystem::path output_file;
	output_file = getCheckpointDirectoryPath() / std::filesystem::path(
		"checkpoint-" + this->getAlgorithmPersonalityType() + "-" +
		(this->halted() ? "final" : Gem::Common::to_string(getIteration())) + "-" +
		Gem::Common::to_string(std::get<G_TRANSFORMED_FITNESS>(getBestKnownPrimaryFitness())) + "-" +
		getCheckpointBaseName()
	);

	// Save checkpoints if required by the user
	if(m_cp_interval < 0 && is_better) {
		do_save = true;
	} // Only save when a better solution was found
	else if(m_cp_interval > 0 && m_iteration%m_cp_interval == 0) {
		do_save = true;
	} // Save in regular intervals
	else if(this->halted()) {
		do_save = true;
	} // Save the final result


	if(do_save) {
		saveCheckpoint(output_file);

		// Remove the last checkoint file if requested by the user
		if(m_cp_remove && m_cp_last != "empty") {
			if(std::filesystem::exists(std::filesystem::path(m_cp_last))) {
				std::filesystem::remove(std::filesystem::path(m_cp_last));
			}
		}

		// Record the name of the last known checkpoint file
		m_cp_last = output_file.string();
	}
}

/******************************************************************************/
/**
 * Loads the state of the class from disc
 */
void G_OptimizationAlgorithm_Base::loadCheckpoint(std::filesystem::path const & cpFile) {
	// Extract the name of the optimization algorithm used for this file
	std::string opt_desc = this->extractOptAlgFromPath(cpFile);

	// Make sure it fits our own algorithm
	if(opt_desc != this->getAlgorithmPersonalityType()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::loadCheckpoint(): Error!" << std::endl
				<< "Checkpoint file " << cpFile << std::endl
				<< "seems to belong to another algorithm. Expected " << this->getAlgorithmPersonalityType() << std::endl
				<< "but got " << opt_desc << std::endl
		);
	}

	this->fromFile(cpFile, this->getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Checks whether the optimization process has been halted, because the halt() function
 * has returned "true"
 *
 * @return A boolean indicating whether the optimization process has been halted
 */
bool G_OptimizationAlgorithm_Base::halted() const {
	return m_halted;
}

/******************************************************************************/
/**
 * Allows to set the number of generations after which a checkpoint should be written.
 * A negative value will result in automatic checkpointing, whenever a better solution
 * was found.
 *
 * @param cpInterval The number of generations after which a checkpoint should be written
 */
void G_OptimizationAlgorithm_Base::setCheckpointInterval(std::int32_t cpInterval) {
	m_cp_interval = cpInterval;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of generations after which a checkpoint should be written
 *
 * @return The number of generations after which a checkpoint should be written
 */
std::int32_t G_OptimizationAlgorithm_Base::getCheckpointInterval() const {
	return m_cp_interval;
}

/******************************************************************************/
/**
 * Allows to set the base name of the checkpoint file and the directory where it
 * should be stored.
 *
 * @param cpDirectory The directory where checkpoint files should be stored
 * @param cpBaseName The base name used for the checkpoint files
 */
void G_OptimizationAlgorithm_Base::setCheckpointBaseName(
	std::string cpDirectory
	, std::string cpBaseName
) {
	// Do some basic checks
	if(cpBaseName == "empty" || cpBaseName.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
				<< "Error: Invalid cpBaseName: " << cpBaseName << std::endl
		);
	}

	if(cpDirectory == "empty" || cpDirectory.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
				<< "Error: Invalid cpDirectory: " << cpDirectory << std::endl
		);
	}

	m_cp_base_name = cpBaseName;

	// Transform the directory into a path
	m_cp_directory_path = std::filesystem::path(cpDirectory);

	// Check that the provided directory exists
	if(not std::filesystem::exists(m_cp_directory_path)) {
		glogger
			<< "In G_OptimizationAlgorithm_Base::setCheckpointBaseName(): Warning!" << std::endl
			<< "Directory " << m_cp_directory_path.string() << " does not exist and will be created automatically." << std::endl
			<< GWARNING;

		if(not std::filesystem::create_directory(m_cp_directory_path)) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In G_OptimizationAlgorithm_Base::setCheckpointBaseName(): Error!" << std::endl
					<< "Could not create directory " << m_cp_directory_path.string() << std::endl
			);
		}
	} else if(not std::filesystem::is_directory(m_cp_directory_path)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base::setCheckpointBaseName(): Error!" << std::endl
				<< m_cp_directory_path.string() << " exists but is no directory." << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the base name of the checkpoint file.
 *
 * @return The base name used for checkpoint files
 */
std::string G_OptimizationAlgorithm_Base::getCheckpointBaseName() const {
	return m_cp_base_name;
}

/******************************************************************************/
/**
 * Allows to retrieve the directory where checkpoint files should be stored
 *
 * @return The base name used for checkpoint files
 */
std::string G_OptimizationAlgorithm_Base::getCheckpointDirectory() const {
	return m_cp_directory_path.string();
}

/******************************************************************************/
/**
 * Allows to retrieve the directory where checkpoint files should be stored
 *
 * @return The base name used for checkpoint files
 */
std::filesystem::path G_OptimizationAlgorithm_Base::getCheckpointDirectoryPath() const {
	return m_cp_directory_path;
}

/******************************************************************************/
/**
 * Determines whether checkpointing should be done in Text-, XML- or Binary-mode
 *
 * @param cpSerMode The desired new checkpointing serialization mode
 */
void G_OptimizationAlgorithm_Base::setCheckpointSerializationMode(Gem::Common::serializationMode cpSerMode) {
	m_cp_serialization_mode = cpSerMode;
}

/******************************************************************************/
/**
 * Retrieves the current checkpointing serialization mode
 *
 * @return The current checkpointing serialization mode
 */
Gem::Common::serializationMode G_OptimizationAlgorithm_Base::getCheckpointSerializationMode() const {
	return m_cp_serialization_mode;
}

/******************************************************************************/
/**
 * Allows to set the m_cp_overwrite flag (determines whether checkpoint files
 * should be removed or kept
 */
void G_OptimizationAlgorithm_Base::setRemoveCheckpointFiles(bool cp_remove) {
	m_cp_remove = cp_remove;
}

/******************************************************************************/
/**
 * Allows to check whether checkpoint files will be removed
 */
bool G_OptimizationAlgorithm_Base::checkpointFilesAreRemoved() const {
	return m_cp_remove;
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
void G_OptimizationAlgorithm_Base::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

// Check that we are dealing with a G_OptimizationAlgorithm_Base reference independent of this object and convert the pointer
	const G_OptimizationAlgorithm_Base *p_load = Gem::Common::g_convert_and_compare<GObject, G_OptimizationAlgorithm_Base>(cp, this);

	GToken token("G_OptimizationAlgorithm_Base", e);

// Compare our parent data ...
	Gem::Common::compare_base_t<GObject>(*this, *p_load, token);

// ... and then the local data
	compare_t(IDENTITY(this->m_data_cnt,  p_load->m_data_cnt), token); // This allows us to compare the parent class without directly referring to it.
	compare_t(IDENTITY(m_iteration, p_load->m_iteration), token);
	compare_t(IDENTITY(m_offset, p_load->m_offset), token);
	compare_t(IDENTITY(m_maxIteration, p_load->m_maxIteration), token);
	compare_t(IDENTITY(m_minIteration, p_load->m_minIteration), token);
	compare_t(IDENTITY(m_maxStallIteration, p_load->m_maxStallIteration), token);
	compare_t(IDENTITY(m_reportIteration, p_load->m_reportIteration), token);
	compare_t(IDENTITY(m_nRecordbestGlobalIndividuals, p_load->m_nRecordbestGlobalIndividuals), token);
	compare_t(IDENTITY(m_bestGlobalIndividuals_pq, p_load->m_bestGlobalIndividuals_pq), token);
	compare_t(IDENTITY(m_bestIterationIndividuals_pq, p_load->m_bestIterationIndividuals_pq), token);
	compare_t(IDENTITY(m_defaultPopulationSize, p_load->m_defaultPopulationSize), token);
	compare_t(IDENTITY(m_bestKnownPrimaryFitness, p_load->m_bestKnownPrimaryFitness), token);
	compare_t(IDENTITY(m_bestCurrentPrimaryFitness, p_load->m_bestCurrentPrimaryFitness), token);
	compare_t(IDENTITY(m_stallCounter, p_load->m_stallCounter), token);
	compare_t(IDENTITY(m_stallCounterThreshold, p_load->m_stallCounterThreshold), token);
	compare_t(IDENTITY(m_cp_interval, p_load->m_cp_interval), token);
	compare_t(IDENTITY(m_cp_base_name, p_load->m_cp_base_name), token);
	compare_t(IDENTITY(m_cp_directory_path.string(), p_load->m_cp_directory_path.string()), token);
	compare_t(IDENTITY(m_cp_last, p_load->m_cp_last), token);
	compare_t(IDENTITY(m_cp_remove, p_load->m_cp_remove), token);
	compare_t(IDENTITY(m_cp_serialization_mode, p_load->m_cp_serialization_mode), token);
	compare_t(IDENTITY(m_qualityThreshold, p_load->m_qualityThreshold), token);
	compare_t(IDENTITY(m_hasQualityThreshold, p_load->m_hasQualityThreshold), token);
	compare_t(IDENTITY(m_maxDuration.count(), p_load->m_maxDuration.count()), token); // Cannot directly compare std::chrono::duration<double>
	compare_t(IDENTITY(m_minDuration.count(), p_load->m_minDuration.count()), token); // Cannot directly compare std::chrono::duration<double>
	compare_t(IDENTITY(m_terminationFile, p_load->m_terminationFile), token);
	compare_t(IDENTITY(m_terminateOnFileModification, p_load->m_terminateOnFileModification), token);
	compare_t(IDENTITY(m_emitTerminationReason, p_load->m_emitTerminationReason), token);
	compare_t(IDENTITY(m_halted, p_load->m_halted), token);
	compare_t(IDENTITY(m_worstKnownValids_cnt, p_load->m_worstKnownValids_cnt), token);
	compare_t(IDENTITY(m_pluggable_monitors_cnt, p_load->m_pluggable_monitors_cnt), token);
	compare_t(IDENTITY(m_executor_ptr, p_load->m_executor_ptr), token);
	compare_t(IDENTITY(m_default_execMode, p_load->m_default_execMode), token);
	compare_t(IDENTITY(m_default_executor_config, p_load->m_default_executor_config), token);

// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Resets the class to the state before the optimize call.
 */
void G_OptimizationAlgorithm_Base::resetToOptimizationStart() {
	resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Resets the class to the state before the optimize call. This will in
 * particular erase all individuals stored in this class and clear the list
 * of best individuals. Please note that a subsequent call to optimize will
 * result in an error, unless you add new individuals. The purpose of this
 * function is allow repeated optimization with the same settings, but different
 * starting points. Actual implementations of optimization algorithms derived
 * from this class may have to perform additional work by overloading (and
 * calling) this function. Since this function will also reset the executor,
 * unless you register a new executor, calling this function will result in
 * the default executor being used.
 */
void G_OptimizationAlgorithm_Base::resetToOptimizationStart_() {
	this->clear(); // Remove all individuals found in this population

	m_iteration = 0; // The current iteration
	m_bestGlobalIndividuals_pq.clear(); // A priority queue with the best individuals found so far
	m_bestIterationIndividuals_pq.clear(); // A priority queue with the best individuals of a given iteration

	m_bestKnownPrimaryFitness = std::tuple<double,double>(0.,0.); // Records the best primary fitness found so far
	m_bestCurrentPrimaryFitness = std::tuple<double,double>(0.,0.); // Records the best fitness found in the current iteration

	m_stallCounter = 0; // Counts the number of iterations without improvement

	m_halted = true; // Also means: No optimization is currently running

	m_worstKnownValids_cnt.clear(); // Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)

	m_executor_ptr.reset(); // Removes the local executor
}

/******************************************************************************/
/**
 * Adds a new executor to the class, replacing the default executor. The
 * executor is responsible for evaluating the individuals.
 *
 * @param executor_ptr A pointer to an executor
 * @param executorConfigFile The name of a file used to configure the executor
 */
void G_OptimizationAlgorithm_Base::registerExecutor(
	std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> executor_ptr
	, std::filesystem::path const& executorConfigFile
) {
	if(not executor_ptr) {
		glogger
			<< "In G_OptimizationAlgorithm_Base::registerExecutor(): Warning!" << std::endl
			<< "Tried to register empty executor-pointer. We will leave the existing" << std::endl
			<< "executor in place" << std::endl
			<< GWARNING;

		return;
	}

	if(not m_halted) {
		glogger
			<< "In G_OptimizationAlgorithm_Base::registerExecutor(): Warning!" << std::endl
			<< "Tried to register an executor while the optimization is already running" << std::endl
			<< "The new executor will be ignored." << std::endl
			<< GWARNING;

		return;
	}

	// Register the new executor
	m_executor_ptr = executor_ptr;

	// Give the executor a chance to configure itself from
	// user-defined configuration options
	Gem::Common::GParserBuilder gpb;
	m_executor_ptr->addConfigurationOptions(gpb);
	if (not gpb.parseConfigFile(executorConfigFile)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base::registerExecutor(): Error!" << std::endl
				<< "Could not parse configuration file " << executorConfigFile.string() << std::endl
		);
	}

	// TODO: Check that the new executor has the desired configuration
}

/******************************************************************************/
/**
 * Adds a new executor to the class, using the chosen execution mode
 *
 * @param e The execution mode
 * @param executorConfigFile The name of a file used to configure the executor
 */
void G_OptimizationAlgorithm_Base::registerExecutor(
	execMode e
	, std::filesystem::path const& executorConfigFile
) {
	auto executor_ptr = this->createExecutor(e);
	this->registerExecutor(executor_ptr, executorConfigFile);
}

/******************************************************************************/
/**
 * This function encapsulates some common functionality of iteration-based
 * optimization algorithms. E.g., they all need a loop that stops if some
 * predefined criterion is reached. This function is also the main entry
 * point for all optimization algorithms.
 *
 * @param offset Specifies the iteration number to start with (e.g. useful when starting from a checkpoint file)
 * @return A constant pointer to this object
 */
G_OptimizationAlgorithm_Base const * const G_OptimizationAlgorithm_Base::optimize_(
	std::uint32_t offset
) {
	// Reset the generation counter
	m_iteration = offset;

	// Set the iteration offset
	m_offset = offset;

	// Store any *clean* individuals that have been added to this algorithm
	// in the priority queue. This happens so that best individuals from a
	// previous "chained" optimization run aren't lost.
	addCleanStoredBests(m_bestGlobalIndividuals_pq);

	// Resize the population to the desired size and do some error checks.
	// This function will also check that individuals have indeed been registered
	adjustPopulation_();

	// Set the individual's personalities (some algorithm-specific information needs to be stored
	// in individuals. Optimization algorithms need to re-implement this function to add
	// the required functionality.)
	setIndividualPersonalities();

	// Emit the info header, unless we do not want any info (parameter 0).
	// Note that this call needs to come after the initialization, so we have the
	// complete set of individuals available.
	if(m_reportIteration) informationUpdate(infoMode::INFOINIT);

	// We want to know if no better values were found for a longer period of time
	double worstCase = this->at(0)->getWorstCase();
	m_bestKnownPrimaryFitness   = std::make_tuple(worstCase, worstCase);
	m_bestCurrentPrimaryFitness = std::make_tuple(worstCase, worstCase);

	m_stallCounter = 0;

	// Give derived classes the opportunity to perform any other necessary preparatory work.
	init();

	// Let the algorithm know that the optimization process hasn't been halted yet.
	m_halted = false; // general halt criterion

	// Initialize the start time with the current time.
	m_startTime = std::chrono::system_clock::now();

	// Initialize a file start time, as it may not be comparable with system_clock
	m_file_startTime = Gem::Common::touch_time("geneva_file_startTime","marker",true);

	do {
		// Let all individuals know the current iteration
		markIteration();

		// Update fitness values and the stall counter
		updateStallCounter((m_bestCurrentPrimaryFitness= cycleLogic_()));

		// Add the best individuals to the m_bestGlobalIndividuals_pq
		// and m_bestIterationIndividuals_pq vectors
		updateGlobalBestsPQ_(m_bestGlobalIndividuals_pq);
		updateIterationBestsPQ_(m_bestIterationIndividuals_pq);

		// Check whether a better value was found, and do the check-pointing, if necessary and requested.
		checkpoint(progress());

		// Let all individuals know about the best fitness known so far
		markBestFitness();

		// Let individuals know about the stalls encountered so far
		markNStalls();

		// Give derived classes an opportunity to act on stalls. NOTE that no action
		// may be taken that affects the "dirty" state of individuals
		if(m_stallCounterThreshold && stallCounterThresholdExceeded()) {
			actOnStalls_();
		}

		// We want to provide feedback to the user in regular intervals.
		// Set the reportGeneration_ variable to 0 in order not to emit
		// any information at all.
		if(m_reportIteration && (m_iteration%m_reportIteration == 0)) {
			informationUpdate(infoMode::INFOPROCESSING);
		}

		// update the m_iteration counter
		m_iteration++;
	}
	while(not (m_halted = halt()));

	// Give derived classes the opportunity to perform any remaining clean-up work
	finalize();

	// Finalize the info output
	if(m_reportIteration) informationUpdate(infoMode::INFOEND);

	// Remove information particular to the optimization algorithms from the individuals
	resetIndividualPersonalities();

	return this;
}

/******************************************************************************/
/**
 * Emits information specific to this class (basic information in each iteration
 * plus some user-defined information via pluggable optimization monitors)
 *
 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
 */
void G_OptimizationAlgorithm_Base::informationUpdate(infoMode const& im) {
	// Act on the information mode provided
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT:
			std::cout << "Starting an optimization run with algorithm \"" << this->getAlgorithmName() << "\"" << std::endl;
			break;

		case Gem::Geneva::infoMode::INFOPROCESSING:
		{
			// We output raw values here, as this is likely what the user is interested in
			std::cout
				<< std::setprecision(5)
				<< this->getIteration() << ": "
				<< Gem::Common::g_to_string(this->getBestCurrentPrimaryFitness())
				<< " // best past: " << Gem::Common::g_to_string(this->getBestKnownPrimaryFitness())
				<< std::endl;
		}
			break;

		case Gem::Geneva::infoMode::INFOEND:
			std::cout << "End of optimization reached in algorithm \""<< this->getAlgorithmName() << "\"" << std::endl;
			break;
	};

	// Perform any action defined by the user through pluggable monitor objects
	for(auto const& pm_ptr: m_pluggable_monitors_cnt) {
		pm_ptr->informationFunction(im, this);
	}
}

/******************************************************************************/
/**
 * Checks whether a better solution was found. If so, the stallCounter_
 * variable will have been set to 0
 *
 * @return A boolean indicating whether a better solution was found
 */
bool G_OptimizationAlgorithm_Base::progress() const {
	return (0==m_stallCounter);
}

/******************************************************************************/
/**
 * Allows to register a pluggable optimization monitor. Note that this
 * function does NOT take ownership of the optimization monitor.
 */
void G_OptimizationAlgorithm_Base::registerPluggableOM(
	std::shared_ptr<GBasePluggableOM> pluggableOM
) {
	if(pluggableOM) {
		m_pluggable_monitors_cnt.push_back(pluggableOM);
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GoptimizationMonitorT<>::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
		);
	}
}

/************************************************************************/
/**
 * Allows to reset the local pluggable optimization monitors
 */
void G_OptimizationAlgorithm_Base::resetPluggableOM() {
	m_pluggable_monitors_cnt.clear();
}

/******************************************************************************/
/**
 * Allows to check whether pluggable optimization monitors were registered
  */
bool G_OptimizationAlgorithm_Base::hasPluggableOptimizationMonitors() const {
	return not m_pluggable_monitors_cnt.empty();
}

/******************************************************************************/
/**
 * Retrieves the default population size
 *
 * @return The default population size
 */
std::size_t G_OptimizationAlgorithm_Base::getDefaultPopulationSize() const {
	return m_defaultPopulationSize;
}

/******************************************************************************/
/**
 * Retrieve the current population size
 *
 * @return The current population size
 */
std::size_t G_OptimizationAlgorithm_Base::getPopulationSize() const {
	return this->size();
}

/******************************************************************************/
/**
 * Set the number of iterations after which the optimization should
 * be stopped
 *
 * @param maxIteration The number of iterations after which the optimization should terminate
 */
void G_OptimizationAlgorithm_Base::setMaxIteration(std::uint32_t maxIteration) {
	// Check that the maximum number of iterations is > the minimum number
	// The check is only valid if a maximum number of iterations has been set (i.e. is != 0)
	if(m_maxIteration > 0 && m_maxIteration <= m_minIteration) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::setMaxIteration(): Error!" << std::endl
				<< "Maximum number of iterations " << 	m_maxIteration << " is <= the minimum number " << m_minIteration << std::endl
		);
	}

	m_maxIteration = maxIteration;
}

/******************************************************************************/
/**
 * Retrieve the number of iterations after which optimization should
 * be stopped
 *
 * @return The number of iterations after which the optimization should terminate
 */
std::uint32_t G_OptimizationAlgorithm_Base::getMaxIteration() const {
	return m_maxIteration;
}

/******************************************************************************/
/**
 * This function checks whether a minimal number of iterations was reached.
  * No halt will be performed if this is not the case (with the exception of halts
  * that are triggered by user-actions, such as Ctrl-C (Sighup-Halt) and touched halt
  * (Geneva checks whether a file was modified after Geneva has started). Set the number
  * of iterations to 0 in order to disable a check for the minimal number of iterations.
*/
void G_OptimizationAlgorithm_Base::setMinIteration(std::uint32_t minIteration) {
	// Check that the maximum number of iterations is > the minimum number
	// The check is only valid if a maximum number of iterations has been set (i.e. is != 0)
	if(m_maxIteration > 0 && m_maxIteration <= m_minIteration) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::setMinIteration(): Error!" << std::endl
				<< "Maximum number of iterations " << 	m_maxIteration << " is <= the minimum number " << m_minIteration << std::endl
		);
	}

	m_minIteration = minIteration;
}

/******************************************************************************/
/**
 * This function retrieves the value of the minIteration_ variable
 */
std::uint32_t G_OptimizationAlgorithm_Base::getMinIteration() const {
	return m_minIteration;
}

/******************************************************************************/
/**
 * Sets the maximum number of generations allowed without improvement of the best
 * individual. Set to 0 in order for this stop criterion to be disabled.
 *
 * @param The maximum number of allowed generations
 */
void G_OptimizationAlgorithm_Base::setMaxStallIteration(std::uint32_t maxStallIteration) {
	m_maxStallIteration = maxStallIteration;
}

/******************************************************************************/
/**
 * Retrieves the maximum number of generations allowed in an optimization run without
 * improvement of the best individual.
 *
 * @return The maximum number of generations
 */
std::uint32_t G_OptimizationAlgorithm_Base::getMaxStallIteration() const {
	return m_maxStallIteration;
}

/******************************************************************************/
/**
 * Sets the maximum allowed processing time
 *
 * @param maxDuration The maximum allowed processing time
 */
void G_OptimizationAlgorithm_Base::setMaxTime(std::chrono::duration<double> maxDuration) {
	if(not Gem::Common::isClose<double>(maxDuration.count(), 0.) && maxDuration < m_minDuration) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::setMaxTime(): Error!" << std::endl
				<< "Desired maxDuration (" << maxDuration.count() << " is smaller than m_minDuration(" << m_minDuration.count() << ")" << std::endl
		);
	}

	m_maxDuration = maxDuration;
}

/******************************************************************************/
/**
 * Retrieves the value of the maxDuration_ parameter.
 *
 * @return The maximum allowed processing time
 */
std::chrono::duration<double> G_OptimizationAlgorithm_Base::getMaxTime() const {
	return m_maxDuration;
}

/******************************************************************************/
/**
* Sets the minimum required processing time. NOTE: Always set the maximum duration
* before the minumum duration.
*
* @param minDuration The minimum allowed processing time
*/
void G_OptimizationAlgorithm_Base::setMinTime(std::chrono::duration<double> minDuration) {
	if(not Gem::Common::isClose<double>(m_maxDuration.count(),0.) && m_maxDuration < minDuration) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::setMinTime(): Error!" << std::endl
				<< "Desired maxDuration (" << m_maxDuration.count() << " is smaller than m_minDuration(" << minDuration.count() << ")" << std::endl
		);
	}

	m_minDuration = minDuration;
}

/******************************************************************************/
/**
* Retrieves the value of the minDuration_ parameter.
*
* @return The minimum required processing time
*/
std::chrono::duration<double> G_OptimizationAlgorithm_Base::getMinTime() const {
	return m_minDuration;
}

/******************************************************************************/
/**
 *  Sets a quality threshold beyond which optimization is expected to stop
 *
 *  @param qualityThreshold A threshold beyond which optimization should stop
 *  @param hasQualityThreshold Allows to (de-)activate the quality threshold
 */
void G_OptimizationAlgorithm_Base::setQualityThreshold(double qualityThreshold, bool hasQualityThreshold) {
	m_qualityThreshold = qualityThreshold;
	m_hasQualityThreshold=hasQualityThreshold;
}

/******************************************************************************/
/**
 * Retrieves the current value of the quality threshold and also indicates whether
 * the threshold is active
 *
 * @param hasQualityThreshold A boolean indicating whether a quality threshold has been set
 * @return The current value of the quality threshold
 */
double G_OptimizationAlgorithm_Base::getQualityThreshold(bool& hasQualityThreshold) const {
	hasQualityThreshold = m_hasQualityThreshold;
	return m_qualityThreshold;
}

/******************************************************************************/
/**
 *  Sets the name of a "termination file" (optimization is supposed to
 *  stop when the modification time of this file is more recent than the
 *  start of the optimizatoon rn.
 *
 *  @param terminationFile The name of a file used to initiate termination
 *  @param hasQualityThreshold Allows to (de-)activate "touched termination"
 */
void G_OptimizationAlgorithm_Base::setTerminationFile(std::string terminationFile, bool terminateOnFileModification) {
	m_terminationFile = std::move(terminationFile);
	m_terminateOnFileModification = terminateOnFileModification;
}

/******************************************************************************/
/**
 * Retrieves the current name of the termination file and also indicates whether
 * the "touched halt" is active
 *
 * @param terminateOnFileModification A boolean indicating whether "touched termination" is active
 * @return The current value of the terminationFile_ variable
 */
std::string G_OptimizationAlgorithm_Base::getTerminationFile(bool& terminateOnFileModification) const {
	terminateOnFileModification = m_terminateOnFileModification;
	return m_terminationFile;
}

/******************************************************************************/
/**
 * Removes the quality threshold
 */
void G_OptimizationAlgorithm_Base::resetQualityThreshold() {
	m_hasQualityThreshold = false;
}

/******************************************************************************/
/**
 * Checks whether a quality threshold has been set
 *
 * @return A boolean indicating whether a quality threshold has been set
 */
bool G_OptimizationAlgorithm_Base::hasQualityThreshold() const {
	return m_hasQualityThreshold;
}

/******************************************************************************/
/**
 * Retrieve the current iteration of the optimization run
 *
 * @return The current iteration of the optimization run
 */
std::uint32_t G_OptimizationAlgorithm_Base::getIteration_() const {
	return m_iteration;
}

/******************************************************************************/
/**
 * Returns the current offset used to calculate the current iteration. This
 * is identical to the iteration the optimization starts with.
 *
 * @return The current iteration offset
 */
std::uint32_t G_OptimizationAlgorithm_Base::getStartIteration() const {
	return m_offset;
}

/******************************************************************************/
/**
 * Sets the number of iterations after which the algorithm should
 * report about its inner state.
 *
 * @param iter The number of iterations after which information should be emitted
 */
void G_OptimizationAlgorithm_Base::setReportIteration(std::uint32_t iter) {
	m_reportIteration = iter;
}

/******************************************************************************/
/**
 * Returns the number of iterations after which the algorithm should
 * report about its inner state.
 *
 * @return The number of iterations after which information is emitted
 */
std::uint32_t G_OptimizationAlgorithm_Base::getReportIteration() const {
	return m_reportIteration;
}

/******************************************************************************/
/**
 * Retrieves the current number of failed optimization attempts
 *
 * @return The current number of failed optimization attempts
 */
std::uint32_t G_OptimizationAlgorithm_Base::getStallCounter() const {
	return m_stallCounter;
}

/******************************************************************************/
/**
 * Allows to set the number of iterations without improvement, after which
 * individuals are asked to update their internal data structures
 */
void G_OptimizationAlgorithm_Base::setStallCounterThreshold(std::uint32_t stallCounterThreshold) {
	m_stallCounterThreshold = stallCounterThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of iterations without improvement, after which
 * individuals are asked to update their internal data structures
 */
std::uint32_t G_OptimizationAlgorithm_Base::getStallCounterThreshold() const {
	return m_stallCounterThreshold;
}

/******************************************************************************/
/**
 * Retrieve the best value found in the entire optimization run so far
 *
 * @return The best raw and transformed fitness found so far
 */
std::tuple<double, double> G_OptimizationAlgorithm_Base::getBestKnownPrimaryFitness() const {
	return (m_bestGlobalIndividuals_pq.best())->getFitnessTuple();

	// return m_bestKnownPrimaryFitness;
}

/******************************************************************************/
/**
 * Retrieves the best value found in the current iteration
 *
 * @return The best raw and transformed fitness found in the current iteration
 */
std::tuple<double, double> G_OptimizationAlgorithm_Base::getBestCurrentPrimaryFitness() const {
	return m_bestCurrentPrimaryFitness;
}

/******************************************************************************/
/**
 * Specifies whether information about termination reasons should be emitted
 *
 * @param etr A boolean which specifies whether reasons for the termination of the optimization run should be emitted
 */
void G_OptimizationAlgorithm_Base::setEmitTerminationReason(bool emitTerminationReason) {
	m_emitTerminationReason = emitTerminationReason;
}

/******************************************************************************/
/**
 * Retrieves information on whether information about termination reasons should be emitted
 *
 * @return A boolean which specifies whether reasons for the termination of the optimization run will be emitted
 */
bool G_OptimizationAlgorithm_Base::getEmitTerminationReason() const {
	return m_emitTerminationReason;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 */
std::size_t G_OptimizationAlgorithm_Base::getNProcessableItems() const {
    return getNProcessableItems_();
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration. This function should
 * be overloaded for derived classes. It is used to determine a suitable wait factor for
 * networked execution.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t G_OptimizationAlgorithm_Base::getNProcessableItems_() const {
	return this->size();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void G_OptimizationAlgorithm_Base::addConfigurationOptions_ (
	Gem::Common::GParserBuilder& gpb
) {
	// Call our parent class'es function
	GObject::addConfigurationOptions_(gpb);

	// Add local data
	gpb.registerFileParameter<std::uint32_t>(
		"maxIteration" // The name of the variable
		, DEFAULTMAXIT // The default value
		, [this](std::uint32_t maxIt){ this->setMaxIteration(maxIt); }
	)
		<< "The maximum allowed number of iterations";

	gpb.registerFileParameter<std::uint32_t>(
		"minIteration" // The name of the variable
		, DEFAULTMINIT // The default value
		, [this](std::uint32_t minIt){ this->setMinIteration(minIt); }
	)
		<< "The minimum allowed number of iterations";

	gpb.registerFileParameter<std::uint32_t>(
		"maxStallIteration" // The name of the variable
		, DEFAULTMAXSTALLIT // The default value
		, [this](std::uint32_t maxStallIt){ this->setMaxStallIteration(maxStallIt); }
	)
		<< "The maximum allowed number of iterations without improvement" << std::endl
		<< "0 means: no constraint.";

	gpb.registerFileParameter<std::string, bool>(
		"terminationFile" // The name of the variable
		, "touchedTerminationActive"
		, DEFAULTTERMINATIONFILE // The default value
		, false
		, [this](std::string tf, bool tfa){ this->setTerminationFile(tf, tfa); }
		, "touchedTermination"
	)
		<< "The name of a file which, when modified after the start of an" << std::endl
		<< "optimization run, instructs Geneva to terminate optimitation." << std::endl
		<< "This can be used to \"touch a file\" after the start of an optimization" << std::endl
		<< "run, which will lead to the termination of the run after the current iteration." << Gem::Common::nextComment()
		<< "Activates (1) or de-activates (0) the \"touched termination\"";

	gpb.registerFileParameter<std::uint32_t>(
		"indivdualUpdateStallCounterThreshold" // The name of the variable
		, DEFAULTSTALLCOUNTERTHRESHOLD // The default value
		, [this](std::uint32_t stallCounterThreshold){ this->setStallCounterThreshold(stallCounterThreshold); }
	)
		<< "The number of iterations without improvement after which" << std::endl
		<< "individuals are asked to update their internal data structures" << std::endl
		<< "through the actOnStalls() function. A value of 0 disables this check";

	gpb.registerFileParameter<std::uint32_t>(
		"reportIteration" // The name of the variable
		, DEFAULTREPORTITER // The default value
		, [this](std::uint32_t rI){ this->setReportIteration(rI); }
	)
		<< "The number of iterations after which a report should be issued";

	gpb.registerFileParameter<std::size_t>(
		"nRecordBestIndividuals" // The name of the variable
		, DEFNRECORDBESTINDIVIDUALS // The default value
		, [this](std::size_t nRecBI){ this->setNRecordBestIndividuals(nRecBI); }
	)
		<< "Indicates how many \"best\" individuals should be recorded in each iteration";

	gpb.registerFileParameter<std::int32_t>(
		"cpInterval" // The name of the variable
		, DEFAULTCHECKPOINTIT // The default value
		, [this](std::int32_t cpI){ this->setCheckpointInterval(cpI); }
	)
		<< "The number of iterations after which a checkpoint should be written." << std::endl
		<< "-1 means: Write a checkpoint file whenever an improvement was encountered" << std::endl
		<< " 0 means: Never emit checkpoint files.";

	gpb.registerFileParameter<std::string, std::string>(
		"cpDirectory"  // The name of the first variable
		, "cpBaseName" // The name of the second variable
		, DEFAULTCPDIR // Default value for the first variable
		, DEFAULTCPBASENAME // Default value for the second variable
		, [this](std::string cpDir, std::string cpBN){ this->setCheckpointBaseName(cpDir, cpBN); }
		, "checkpointLocation"
	)
		<< "The directory where checkpoint files should be stored." << Gem::Common::nextComment() // comments for the second option follow
		<< "The significant part of the checkpoint file name.";

	gpb.registerFileParameter<bool>(
		"cpOverwrite" // The name of the variable
		, true // The default value -- we always remove old checkpoints
		, [this](bool cp_overwrite){ this->setRemoveCheckpointFiles(cp_overwrite); }
	)
		<< "When set to \"true\", old checkpoint files will not be kept";

	gpb.registerFileParameter<Gem::Common::serializationMode>(
		"cpSerMode" // The name of the variable
		, DEFAULTCPSERMODE // The default value
		, [this](Gem::Common::serializationMode sM){ this->setCheckpointSerializationMode(sM); }
	)
		<< "Determines whether check-pointing should be done in" << std::endl
		<< "text- (0), XML- (1), or binary-mode (2)";

	gpb.registerFileParameter<double, bool>(
		"threshold" // The name of the variable
		, "thresholdActive"
		, DEFAULTQUALITYTHRESHOLD // The default value
		, false
		, [this](double qt, bool ta){ this->setQualityThreshold(qt, ta); }
		, "qualityTermination"
	)
		<< "A threshold beyond which optimization is expected to stop" << std::endl
		<< "Note that in order to activate this threshold, you also need to" << std::endl
		<< "set \"hasQualityThreshold\" to 1." << Gem::Common::nextComment()
		<< "Activates (1) or de-activates (0) the quality threshold";

	gpb.registerFileParameter<std::string>(
		"maxDuration" // The name of the variable
		, DEFAULTDURATION // The default value
		, [this](std::string mt_str){ this->setMaxTime(Gem::Common::duration_from_string(mt_str)); }
	)
		<< "The maximum allowed time-frame for the optimization" << std::endl
		<< "in the format hours:minutes:seconds";

	gpb.registerFileParameter<std::string>(
		"minDuration" // The name of the variable
		, DEFAULTMINDURATION // The default value
		, [this](std::string mt_str){ this->setMinTime(Gem::Common::duration_from_string(mt_str)); }
	)
		<< "The minimum required time-frame for the optimization" << std::endl
		<< "in the format hours:minutes:seconds";

	gpb.registerFileParameter<bool>(
		"emitTerminationReason" // The name of the variable
		, DEFAULTEMITTERMINATIONREASON // The default value
		, [this](bool etr){ this->setEmitTerminationReason(etr); }
	)
		<< "Triggers emission (1) or omission (0) of information about reasons for termination";

	gpb.registerFileParameter<execMode, std::string>(
		"defaultExecMode" // The name of the variable
		, "defaultExecConfig"
		, this->m_default_execMode // The default value
		, this->m_default_executor_config
		, [this](execMode e, std::string config){ this->m_default_execMode = e; this->m_default_executor_config = config; }
		, "defaultExecutor"
	)
		<< "The default executor type to be used for this algorithm." << std::endl
		<< "0: serial" << std::endl
		<< "1: multi-threaded" << std::endl
		<< "2: brokered" << std::endl << Gem::Common::nextComment()
		<< "The configuration file for the default executor. Note that it needs to fit the executor type.";
}

/******************************************************************************/
/**
 * Adds the individuals of this iteration to a priority queue. The
  * queue will be sorted by the first evaluation criterion of the individuals
  * and may either have a limited or unlimited size, depending on user-
  * settings
 */
void G_OptimizationAlgorithm_Base::updateGlobalBestsPQ_(GParameterSetFixedSizePriorityQueue & bestIndividuals) {
	const bool CLONE = true;
	const bool DONOTREPLACE = false;

#ifdef DEBUG
	if(this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base::updateGlobalBestsPQ() :" << std::endl
				<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
		);
	}
#endif /* DEBUG */

	// We simply add all individuals to the queue -- only the best ones will actually be added (and cloned)
	// Unless we have asked for the queue to have an unlimited size, the queue will be resized as required
	// by its maximum allowed size.
	bestIndividuals.add(this->m_data_cnt, CLONE, DONOTREPLACE);
}

/******************************************************************************/
/**
 * Adds the individuals of this iteration to a priority queue. The
 * queue will be sorted by the first evaluation criterion of the individuals
 * and may either have a limited or unlimited size, depending on user-
 * settings
 */
void G_OptimizationAlgorithm_Base::updateIterationBestsPQ_(GParameterSetFixedSizePriorityQueue & bestIndividuals) {
	const bool CLONE = true;
	const bool REPLACE = true;

#ifdef DEBUG
	if(this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base::updateIterationBestsPQ() :" << std::endl
				<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
		);
	}
#endif /* DEBUG */

	// We simply add all individuals to the queue. They will automatically be sorted.
	bestIndividuals.add(this->m_data_cnt, CLONE, REPLACE);
}

/******************************************************************************/
/**
 * If individuals have been stored in this population, they are added to the
 * priority queue. This happens before the optimization cycle starts, so that
 * best individuals from a previous "chained" optimization run aren't lost.
 * Only those individuals are stored in the priority queue that do not have the
 * "dirty flag" set.
 */
void G_OptimizationAlgorithm_Base::addCleanStoredBests(GParameterSetFixedSizePriorityQueue& bestIndividuals) {
	const bool CLONE = true;

	// We simply add all *clean* individuals to the queue -- only the best ones will actually be added
	// (and cloned) Unless we have asked for the queue to have an unlimited size, the queue will be
	// resized as required by its maximum allowed size.
	for(auto const & ind_ptr: *this) {
		if(ind_ptr->is_processed()) {
			bestIndividuals.add(ind_ptr, CLONE);
		}
	}
}

/******************************************************************************/
/**
 * A little helper function that determines whether we are currently inside of the first
 * iteration
 *
 * @return A boolean indicating whether we are inside of the first iteration
 */
bool G_OptimizationAlgorithm_Base::inFirstIteration() const {
	return m_iteration == m_offset;
}

/******************************************************************************/
/**
 * A little helper function that determines whether we are after the first iteration
 *
 * @return A boolean indicating whether we are after the first iteration
 */
bool G_OptimizationAlgorithm_Base::afterFirstIteration() const {
	return m_iteration > m_offset;
}

/******************************************************************************/
/**
 * Checks whether a checkpoint-file has the same "personality" as our
 * own algorithm
 */
bool G_OptimizationAlgorithm_Base::cp_personality_fits(const std::filesystem::path& p) const {
	// Extract the name of the optimization algorithm used for this file
	std::string opt_desc = this->extractOptAlgFromPath(p);

	// Make sure it fits our own algorithm
	return opt_desc == this->getAlgorithmPersonalityType();
}

/******************************************************************************/
/**
 * Loads the data of another GOptimizationAlgorithm object
 *
 * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
 */
void G_OptimizationAlgorithm_Base::load_(const GObject* cp) {
	// Check that we are dealing with a G_OptimizationAlgorithm_Base reference independent of this object and convert the pointer
	const G_OptimizationAlgorithm_Base *p_load = Gem::Common::g_convert_and_compare<GObject, G_OptimizationAlgorithm_Base>(cp, this);

	// Load the parent class'es data
	GObject::load_(cp);
	Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>::operator=(*p_load);

	// and then our local data
	m_iteration = p_load->m_iteration;
	m_offset = p_load->m_offset;
	m_maxIteration = p_load->m_maxIteration;
	m_minIteration = p_load->m_minIteration;
	m_maxStallIteration = p_load->m_maxStallIteration;
	m_reportIteration = p_load->m_reportIteration;
	m_nRecordbestGlobalIndividuals = p_load->m_nRecordbestGlobalIndividuals;
	m_bestGlobalIndividuals_pq = p_load->m_bestGlobalIndividuals_pq;
	m_bestIterationIndividuals_pq = p_load->m_bestIterationIndividuals_pq;
	m_defaultPopulationSize = p_load->m_defaultPopulationSize;
	m_bestKnownPrimaryFitness = p_load->m_bestKnownPrimaryFitness;
	m_bestCurrentPrimaryFitness = p_load->m_bestCurrentPrimaryFitness;
	m_stallCounter = p_load->m_stallCounter;
	m_stallCounterThreshold = p_load->m_stallCounterThreshold;
	m_cp_interval = p_load->m_cp_interval;
	m_cp_base_name = p_load->m_cp_base_name;
	m_cp_directory_path = p_load->m_cp_directory_path;
	m_cp_last = p_load->m_cp_last;
	m_cp_remove = p_load->m_cp_remove;
	m_cp_serialization_mode = p_load->m_cp_serialization_mode;
	m_qualityThreshold = p_load->m_qualityThreshold;
	m_hasQualityThreshold = p_load->m_hasQualityThreshold;
	m_terminationFile = p_load->m_terminationFile;
	m_terminateOnFileModification = p_load->m_terminateOnFileModification;
	m_maxDuration = p_load->m_maxDuration;
	m_minDuration = p_load->m_minDuration;
	m_emitTerminationReason = p_load->m_emitTerminationReason;
	m_halted.store(p_load->m_halted.load());
	m_worstKnownValids_cnt = p_load->m_worstKnownValids_cnt;
	Gem::Common::copyCloneableSmartPointerContainer(p_load->m_pluggable_monitors_cnt, m_pluggable_monitors_cnt);
	Gem::Common::copyCloneableSmartPointer(p_load->m_executor_ptr, m_executor_ptr);
	m_default_execMode = p_load->m_default_execMode;
	m_default_executor_config = p_load->m_default_executor_config;
}

/******************************************************************************/
/**
 * Delegation of work to be performed to the private executor object. Note that
 * the return values "is_complete" and "has_errors" may both be true, i.e. all items
 * may have returned, but there were errors in some or all of them. The function
 * will also make the executor use this objects iteration counter.
 *
 * @param workItems The set of work items to be processed
 * @param resubmitUnprocessed Indicates whether unprocessed work items should be resubmitted after a timeout
 * @param caller The name of the caller (used for error messages and logs)
 * @return A struct which indicates whether all items have returned ("is_complete") and whether there were errors ("has_errors")
 */
Gem::Courtier::executor_status_t G_OptimizationAlgorithm_Base::workOn(
	std::vector<std::shared_ptr<GParameterSet>>& workItems
	, bool resubmitUnprocessed
	, const std::string &caller
) {
	auto iterationCounter = std::make_tuple<Gem::Courtier::ITERATION_COUNTER_TYPE, bool>(
		boost::numeric_cast<Gem::Courtier::ITERATION_COUNTER_TYPE>(this->getIteration())
		, true
	);

	return m_executor_ptr->workOn(
		workItems
		, resubmitUnprocessed
		, iterationCounter
		, caller
	);
}

/******************************************************************************/
/**
 * Retrieves a vector of old work items after job submission
 */
std::vector<std::shared_ptr<GParameterSet>> G_OptimizationAlgorithm_Base::getOldWorkItems() {
	return m_executor_ptr->getOldWorkItems();
}

/******************************************************************************/
/**
 * Saves the state of the class to disc
 */
void G_OptimizationAlgorithm_Base::saveCheckpoint(std::filesystem::path const& outputFile) const {
	this->toFile(outputFile, this->getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Extracts the short name of the optimization algorithm (example:
 * "PERSONALITY_EA") from a path which complies to the following
 * scheme: /some/path/word1-PERSONALITY_EA-some-other-information .
 * This is mainly used for checkpointing and associated cross-checks.
 */
std::string G_OptimizationAlgorithm_Base::extractOptAlgFromPath(const std::filesystem::path& p) const {
	// Extract the filename
	std::string filename = p.filename().string();

	// Divide the name into tokens
	std::vector<std::string> tokens = Gem::Common::splitString(filename, "-");

	// Check that the size is at least 2 (i.e. the PERSONALITY_X-part may exist)
	if(tokens.size() < 2) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::extractOptAlgFromPath(): Error!" << std::endl
				<< "Found file name " << filename << " that does not comply to rules." << std::endl
				<< "Expected \"/some/path/word1-PERSONALITY_EA-some-other-information \"" << std::endl
		);
	}

	// Let the audience know
	return tokens[1];
}

/******************************************************************************/
/**
 * Retrieves the best individual found up to now (which is usually the best individual
 * in the priority queue).
 */
std::shared_ptr<GParameterSet> G_OptimizationAlgorithm_Base::getBestGlobalIndividual_() const {
#ifdef DEBUG
	std::shared_ptr<GParameterSet> p = m_bestGlobalIndividuals_pq.best();
	if(p) return p;
	else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<T>::getBestGlobalIndividual_(): Error!" << std::endl
				<< "Best individual seems to be empty" << std::endl
		);
	}
#else
	return m_bestGlobalIndividuals_pq.best()->clone<GParameterSet>();
#endif
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found (equal to the content of
 * the priority queue)
 */
std::vector<std::shared_ptr<GParameterSet>> G_OptimizationAlgorithm_Base::getBestGlobalIndividuals_() const {
	std::vector<std::shared_ptr<GParameterSet>> bestIndividualsVec;

	for(const auto& ind_ptr: m_bestGlobalIndividuals_pq.toVector()) {
		bestIndividualsVec.push_back(ind_ptr->clone<GParameterSet>());
	}

	return bestIndividualsVec;
}

/******************************************************************************/
/**
 * Retrieves the best individual found in the iteration (which is the best individual
 * in the priority queue).
 */
std::shared_ptr<GParameterSet> G_OptimizationAlgorithm_Base::getBestIterationIndividual_() const {
#ifdef DEBUG
	std::shared_ptr<GParameterSet> p = m_bestIterationIndividuals_pq.best();
	if(p) return p;
	else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<T>::getBestIterationIndividual_(): Error!" << std::endl
				<< "Best individual seems to be empty" << std::endl
		);
	}
#else
	return m_bestIterationIndividuals_pq.best();
#endif
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found in the iteration (equal to the content of
 * the priority queue)
 */
std::vector<std::shared_ptr<GParameterSet>> G_OptimizationAlgorithm_Base::getBestIterationIndividuals_() const {
	return m_bestIterationIndividuals_pq.toVector();
}

/******************************************************************************/
/**
 * Allows to set the personality type of the individuals
 */
void G_OptimizationAlgorithm_Base::setIndividualPersonalities() {
	for(auto const & ind_ptr: *this) { ind_ptr->setPersonality(this->getPersonalityTraits_()); }
}

/******************************************************************************/
/**
 * Resets the individual's personality types
 */
void G_OptimizationAlgorithm_Base::resetIndividualPersonalities() {
	for(auto const & ind_ptr: *this) { ind_ptr->resetPersonality(); }
}

/******************************************************************************/
/**
 * Sets the default size of the population
 *
 * @param popSize The desired size of the population
 */
void G_OptimizationAlgorithm_Base::setDefaultPopulationSize(const std::size_t& defPopSize) {
	m_defaultPopulationSize = defPopSize;
}

/******************************************************************************/
/**
 * Set the number of "best" individuals to be recorded in each iteration
 *
 * @param nRecordBestIndividuals The number of "best" individuals to be recorded in each iteration
 */
void G_OptimizationAlgorithm_Base::setNRecordBestIndividuals(std::size_t nRecordBestIndividuals) {
	if(0 == nRecordBestIndividuals) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In G_OptimizationAlgorithm_Base<>::setNRecordBestIndividuals(): Error!" << std::endl
				<< "Invalid number of individuals to be recorded: " << nRecordBestIndividuals << std::endl
		);
	}

	m_nRecordbestGlobalIndividuals = nRecordBestIndividuals;
	m_bestGlobalIndividuals_pq.setMaxSize(m_nRecordbestGlobalIndividuals);
}

/******************************************************************************/
/**
 * Retrieve the number of best individuals to be recorded in each iteration
 *
 * @return The number of best individuals to be recorded in each iteration
 */
std::size_t G_OptimizationAlgorithm_Base::getNRecordBestIndividuals() const {
	return m_nRecordbestGlobalIndividuals;
}

/******************************************************************************/
/**
 * Allows derived classes to reset the stall counter.
 */
void G_OptimizationAlgorithm_Base::resetStallCounter() {
	m_stallCounter = 0;
}

/******************************************************************************/
/**
 * Allows to perform initialization work before the optimization cycle starts. This
 * function will usually be overloaded by derived functions, which should however,
 * as their first action, call this function.
 */
void G_OptimizationAlgorithm_Base::init() {
	// Add an executor, if none has been registered
	if(not m_executor_ptr) {
		auto executor_ptr = this->createExecutor(m_default_execMode);

#ifdef DEBUG
		if(not executor_ptr) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In G_OptimizationAlgorithm_Base<>::init(): Error!" << std::endl
					<< "Did not receive a valid executor" << std::endl
			);
		}
#endif

		glogger
			<< "In G_OptimizationAlgorithm_Base<>::init(): No explicit executor was registered. Using default" << std::endl
			<< "\"" << executor_ptr->name() << "\" with config \"" << this->m_default_executor_config << "\" instead" << std::endl
			<< GLOGGING;

		this->registerExecutor(
			executor_ptr
			, this->m_default_executor_config
		);
	}

	// Initialize the executor
	m_executor_ptr->init();
}

/******************************************************************************/
/**
 * Allows to perform any remaining work after the optimization cycle has finished.
 * This function will usually be overloaded by derived functions, which should however
 * call this function as their last action.
 */
void G_OptimizationAlgorithm_Base::finalize() {
	// Finalize the broker connector
	m_executor_ptr->finalize();
}

/******************************************************************************/
/**
 * Lets individuals know about the current iteration of the optimization
 * cycle.
 */
void G_OptimizationAlgorithm_Base::markIteration() {
	for(auto const & ind_ptr: *this) { ind_ptr->setAssignedIteration(m_iteration); }
}

/******************************************************************************/
/**
 * Let individuals know the number of stalls encountered so far
 */
void G_OptimizationAlgorithm_Base::markNStalls() {
	for(auto const & ind_ptr: *this) { ind_ptr->setNStalls(m_stallCounter); }
}

/******************************************************************************/
/**
 * Update the stall counter. We use the transformed fitness for comparison
 * here, so we can usually deal with finite values (due to the transformation
 * in the case of a constraint violation).
 */
void G_OptimizationAlgorithm_Base::updateStallCounter(const std::tuple<double, double>& bestEval) {
	auto m = this->at(0)->getMaxMode(); // We assume the same maxMode for all individuals
	if(isBetter(
		std::get<G_TRANSFORMED_FITNESS>(bestEval)
		, std::get<G_TRANSFORMED_FITNESS>(m_bestKnownPrimaryFitness)
		, m
	)) {
		m_bestKnownPrimaryFitness = bestEval;
		m_stallCounter = 0;
	} else {
		m_stallCounter++;
	}
}

/******************************************************************************/
/**
 * This function returns true once a given time (set with
 * GOptimizationAlgorithm<GParameterSet>::setMaxTime()) has passed.
 * It is used in the G_OptimizationAlgorithm_Base::halt() function.
 *
 * @return A boolean indicating whether a given amount of time has passed
 */
bool G_OptimizationAlgorithm_Base::timedHalt(const std::chrono::system_clock::time_point& currentTime) const {
	if((currentTime - m_startTime) >= m_maxDuration) {
		if(m_emitTerminationReason) {
			glogger
				<< "Terminating optimization run because maximum time frame has been exceeded." << std::endl
				<< GLOGGING;
		}

		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * This function checks whether a minimum amount of time has passed
  */
bool G_OptimizationAlgorithm_Base::minTimePassed(const std::chrono::system_clock::time_point& currentTime) const {
	return (currentTime - m_startTime) > m_minDuration;
}

/******************************************************************************/
/**
 * This function returns true once the quality is below or above a given threshold
 * (depending on whether we maximize or minimize). This function uses user-visible
 * (i.e. untransformed) fitness values, as a quality threshold will usually be
 * set using a true "physical" value.
 *
 * @return A boolean indicating whether the quality is above or below a given threshold
 */
bool G_OptimizationAlgorithm_Base::qualityHalt() const {
	auto m = this->at(0)->getMaxMode(); // We assume the same maxMode for all individuals
	if(isBetter(
		std::get<G_RAW_FITNESS>(m_bestKnownPrimaryFitness) // note: we use the raw fitness so users do not have to specify "transformed" thresholds
		, m_qualityThreshold
		, m
	)) {
		if(m_emitTerminationReason) {
			glogger
				<< "Terminating optimization run because" << std::endl
				<< "quality threshold " << m_qualityThreshold << " has been exceeded." << std::endl
				<< "Best untransformed quality found was " << std::get<G_RAW_FITNESS>(m_bestKnownPrimaryFitness) << std::endl
				<< "with termination in iteration " << m_iteration << std::endl
				<< GLOGGING;
		}

		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * This function returns true once a given number of stalls has been exceeded in a row
 *
 * @return A boolean indicating whether the optimization has stalled too often in a row
 */
bool G_OptimizationAlgorithm_Base::stallHalt() const {
	if(m_stallCounter > m_maxStallIteration) {
		if(m_emitTerminationReason) {
			glogger
				<< "Terminating optimization run because" << std::endl
				<< "maximum number of stalls " << m_maxStallIteration << " has been exceeded." << std::endl
				<< "This is considered to be a criterion for convergence." << std::endl
	         << GLOGGING;
		}

		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * This function returns true once a maximum number of iterations has been exceeded
 *
 * @return A boolean indicating whether the maximum number of iterations has been exceeded
 */
bool G_OptimizationAlgorithm_Base::iterationHalt() const {
	if(m_iteration >= m_maxIteration) {
		if(m_emitTerminationReason) {
			glogger
				<< "Terminating optimization run because" << std::endl
				<< "iteration threshold " << m_maxIteration << " has been exceeded." << std::endl
				<< GLOGGING;
		}

		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * This function returns true when the minimum number of iterations has
 * been passed.
 */
bool G_OptimizationAlgorithm_Base::minIterationPassed() const {
	return m_iteration > m_minIteration;
}

/******************************************************************************/
/**
 * This function returns true if a SIGHUP / CTRL_CLOSE_EVENT signal was sent (provided the user
 * has registered the GObject::sigHupHandler signal handler
 *
 * @return A boolean indicating whether the program was interrupted with a SIGHUP or CTRL_CLOSE_EVENT signal
 */
bool G_OptimizationAlgorithm_Base::sigHupHalt() const {
	if(GObject::G_SIGHUP_SENT()) {
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
		std::cout
			<< "Terminating optimization run because a CTRL_CLOSE_EVENT signal has been received" << std::endl;
#else
		std::cout
			<< "Terminating optimization run because a SIGHUP signal has been received" << std::endl;
#endif
		return true;
	}
	else return false;
}

/******************************************************************************/
/**
 * Triggers termination of the optimization run, when a file with a user-defined
 * name is modified (e.g. "touch'ed") after the optimization run was started. Note
 * that the function will silently return false if the file does not exist, as it
 * is assumed that users may "touch" the file for termination only, so that the
 * possibility exists that the file isn't there until that time.
 */
bool G_OptimizationAlgorithm_Base::touchHalt() const {
	// Create a suitable path object
	std::filesystem::path p(m_terminationFile);

	// Return if the file doesn't exist
	if(not std::filesystem::exists(p)) {
		return false;
	}

	// Determine the modification time of the file
    const auto modTime = std::filesystem::last_write_time(p);

	// Check if the file was modified after the start of the optimization run
	if(modTime > m_file_startTime) {
		if(m_emitTerminationReason) {
			glogger
				<< "Terminating optimization run because" << std::endl
				<< p << " was modified after the start of the optimization" << std::endl
				<< GLOGGING;
		}

		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * A wrapper for the customHalt() function that allows us to emit the termination reason
 *
 * @return A boolean indicating whether a custom halt criterion has been reached
 */
bool G_OptimizationAlgorithm_Base::customHalt() const {
	if(customHalt_()) {
		if(m_emitTerminationReason) {
			glogger
				<< "Terminating optimization run because custom halt criterion has triggered." << std::endl
				<< GLOGGING;
		}

		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * Custom halt condition
 *
 * @return A boolean indicating whether a custom halt criterion has been reached
 */
bool G_OptimizationAlgorithm_Base::customHalt_() const {
	return false;
}

/******************************************************************************/
/**
 * This function checks whether a halt criterion has been reached. The most
 * common criterion is the maximum number of iterations. Set the maxIteration_
 * counter to 0 if you want to disable this criterion.
 *
 * @return A boolean indicating whether a halt criterion has been reached
 */
bool G_OptimizationAlgorithm_Base::halt() const {
	// Retrieve the current time, so all time-based functions act on the same basis
	std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();

	//------------------------------------------------------------------------
	// The following halt criteria are triggered by the user. They override
	// all other (automatic) criteria

	// Have we received a SIGHUP signal ?
	if(sigHupHalt()) return true;

	// Are we supposed to stop when a file was modified after the start of the optimization run ?
	if(m_terminateOnFileModification && touchHalt()) return true;

	//------------------------------------------------------------------------
	// With the exception of the above criteria, no other halt criterion will
	// have an effect unless some minimum criteria have been met. E.g., if the
	// minimum number of iterations, as defined by the user, hasn't been passwd,
	// the optimization will continue (no matter whether e.g. the optimization
	// has stalled for a given number of times).

	// Has the minimum number of iterations, as defined by the user, been passed?
	if(not minIterationPassed()) return false;

	// Has the minimum required optimization time been passed?
	if(not minTimePassed(currentTime)) return false;

	//------------------------------------------------------------------------
	// The following halt criteria are evaluated by Geneva at run-time,
	// without any user-interaction.

	// Have we exceeded the maximum number of iterations and
	// do we indeed intend to stop in this case ?
	if(maxIterationHaltset() && iterationHalt()) return true;

	// Has the optimization stalled too often ?
	if(stallHaltSet() && stallHalt()) return true;

	// Do we have a scheduled halt time ? The comparatively expensive
	// timedHalt() calculation is only called if m_maxDuration
	// is at least one microsecond.
	if(maxDurationHaltSet() && timedHalt(currentTime)) return true;

	// Are we supposed to stop when the quality has exceeded a threshold ?
	if(qualityThresholdHaltSet() && qualityHalt()) return true;

	// Has the user specified an additional stop criterion ?
	return customHalt();
}

/******************************************************************************/
/**
 * Check whether the max-iteration halt is set
 *
 * @return A boolean indicating whether the "max-iteration halt" has been set
 */
bool G_OptimizationAlgorithm_Base::maxIterationHaltset() const {
	return 0 != m_maxIteration;
}

/******************************************************************************/
/**
 * Check whether a halt criterion based on the number of stalls has been set
 *
 * @return A boolean indicating whether a halt criterion based on the number of stalls has been set
 */
bool G_OptimizationAlgorithm_Base::stallHaltSet() const {
	return 0 != m_maxStallIteration;
}

/******************************************************************************/
/**
 * Check whether the maxDuration-halt criterion has been set
 *
 * @return A boolean indication whether the max-duration halt criterion has been set
 */
bool G_OptimizationAlgorithm_Base::maxDurationHaltSet() const {
	return 0. != m_maxDuration.count();
}

/******************************************************************************/
/**
 * Check whether the quality-threshold halt-criterion has been set
 *
 * @return A boolean indicating whether the quality-threshold halt-criterion has been set
 */
bool G_OptimizationAlgorithm_Base::qualityThresholdHaltSet() const {
	return m_hasQualityThreshold;
}

/******************************************************************************/
/**
 * Marks the globally best known fitness in all individuals
 */
void G_OptimizationAlgorithm_Base::markBestFitness() {
	for(auto const & ind_ptr: *this) { ind_ptr->setBestKnownPrimaryFitness(this->getBestKnownPrimaryFitness()); }
}

/******************************************************************************/
/**
 * Indicates whether the stallCounterThreshold_ has been exceeded
 */
bool G_OptimizationAlgorithm_Base::stallCounterThresholdExceeded() const {
	return (m_stallCounter > m_stallCounterThreshold);
}

/******************************************************************************/
/**
 * Retrieves an executor for the given execution mode
 */
std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> G_OptimizationAlgorithm_Base::createExecutor(const execMode& e) {
	std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> executor_ptr;

	switch(e) {
		case execMode::SERIAL:
			glogger << "Creating GSerialExecutorT" << std::endl << GLOGGING;
			executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(
				new Gem::Courtier::GSerialExecutorT<GParameterSet>()
			);
			break;

		case execMode::MULTITHREADED:
			glogger << "Creating GMTExecutorT" << std::endl << GLOGGING;
			executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(
				new Gem::Courtier::GMTExecutorT<GParameterSet>(Gem::Courtier::DEFAULTNSTDTHREADS)
			);
			break;

		case execMode::BROKER:
			glogger << "Creating GBrokerExecutorT" << std::endl << GLOGGING;
			executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(
				new Gem::Courtier::GBrokerExecutorT<GParameterSet>()
			);
			break;
	}

	return executor_ptr;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool G_OptimizationAlgorithm_Base::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests_()) result = true;
	if(Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>::modify_GUnitTests_()) result = true;

	// Try to change the objects contained in the collection
	for(auto const & o_ptr: *this) {
		if(o_ptr->modify_GUnitTests()) result = true;
	}

	this->setMaxIteration(this->getMaxIteration() + 1);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("G_OptimizationAlgorithm_Base<>::modify_GUnitTests", "GEM_TESTING");
		 return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GObject::specificTestsNoFailureExpected_GUnitTests_();
	Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("G_OptimizationAlgorithm_Base<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GObject::specificTestsFailuresExpected_GUnitTests_();
	Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("G_OptimizationAlgorithm_Base<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
