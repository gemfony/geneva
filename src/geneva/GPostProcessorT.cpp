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

#include "geneva/GPostProcessorT.hpp"

// Export of GEvolutionaryAlgorithmPostOptimizer
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithmPostOptimizer) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the execution mode and configuration file
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(
	execMode executionMode
	, const std::string& oa_configFile
	, const std::string& executor_configFile
)
	: GPostProcessorBaseT<GParameterSet>()
	, m_oa_configFile(oa_configFile)
	, m_executor_configFile(executor_configFile)
	, m_executionMode((executionMode == execMode::SERIAL || executionMode == execMode::MULTITHREADED) ? executionMode : execMode::SERIAL)
{
	switch (executionMode) {
		case execMode::SERIAL:
		case execMode::MULTITHREADED:
			/* nothing */
			break;

		case execMode::BROKER:
		{
			glogger
				<< "In GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(execMode): Error!"
				<< std::endl
				<< "Got invalid execution mode " << executionMode << std::endl
				<< "The mode was reset to execMode::SERIAL" << std::endl
				<< GWARNING;
		}
			break;
	}
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GEvolutionaryAlgorithmPostOptimizer::name_() const {
	return std::string("GEvolutionaryAlgorithmPostOptimizer");
}

/******************************************************************************/
/**
 * Checks for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GEvolutionaryAlgorithmPostOptimizer::compare_(
	const Gem::Common::GSerializableFunctionObjectT<GParameterSet> &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const  {
	using namespace Gem::Common;

	// Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithmPostOptimizer *p_load
		= Gem::Common::g_convert_and_compare<Gem::Common::GSerializableFunctionObjectT<GParameterSet>, GEvolutionaryAlgorithmPostOptimizer>(cp, this);

	GToken token("GEvolutionaryAlgorithmPostOptimizer", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GPostProcessorBaseT<GParameterSet>>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_oa_configFile, p_load->m_oa_configFile), token);
	compare_t(IDENTITY(m_executor_configFile, p_load->m_executor_configFile), token);
	compare_t(IDENTITY(m_executionMode, p_load->m_executionMode), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Allows to set the execution mode for this post-processor (serial vs. multi-threaded)
 */
void GEvolutionaryAlgorithmPostOptimizer::setExecMode(execMode executionMode)
{
	switch (executionMode) {
		case execMode::SERIAL:
		case execMode::MULTITHREADED: {
			m_executionMode = executionMode;
		}
			break;

		case execMode::BROKER:
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GEvolutionaryAlgorithmPostOptimizer::setExecMode(): Error!" << std::endl
					<< "Got invalid execution mode " << executionMode << std::endl
			);
		}
			break;
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the current execution mode
 */
execMode GEvolutionaryAlgorithmPostOptimizer::getExecMode() const {
	return m_executionMode;
}

/******************************************************************************/
/**
 * Allows to specify the name of a configuration file
 */
void GEvolutionaryAlgorithmPostOptimizer::setOAConfigFile(const std::string& oa_configFile) {
	m_oa_configFile = oa_configFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the configuration file
 */
std::string GEvolutionaryAlgorithmPostOptimizer::getOAConfigFile() const {
	return m_oa_configFile;
}

/******************************************************************************/
/**
 * Allows to specify the name of a configuration file for the executor
 */
void GEvolutionaryAlgorithmPostOptimizer::setExecutorConfigFile(const std::string& executorConfigFile) {
	m_executor_configFile = executorConfigFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the configuration file for the executor
 */
std::string GEvolutionaryAlgorithmPostOptimizer::getExecutorConfigFile() const {
	return m_executor_configFile;
}

/******************************************************************************/
/**
 * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
 */
void GEvolutionaryAlgorithmPostOptimizer::load_(const Gem::Common::GSerializableFunctionObjectT<GParameterSet> *cp)
{
	// Check that we are dealing with a GEvolutionaryAlgorithmPostOptimizer reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithmPostOptimizer *p_load
		= Gem::Common::g_convert_and_compare<Gem::Common::GSerializableFunctionObjectT<GParameterSet>, GEvolutionaryAlgorithmPostOptimizer>(cp, this);

	// Load our parent class'es data ...
	GPostProcessorBaseT<GParameterSet>::load_(cp);

	// ... and then our local data
	m_oa_configFile = p_load->m_oa_configFile;
	m_executor_configFile = p_load->m_executor_configFile;
	m_executionMode = p_load->m_executionMode;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
Gem::Common::GSerializableFunctionObjectT<GParameterSet> *GEvolutionaryAlgorithmPostOptimizer::clone_() const {
	return new GEvolutionaryAlgorithmPostOptimizer(*this);
}

/******************************************************************************/
/**
 * The actual post-processing takes place here (no further checks)
 */
bool GEvolutionaryAlgorithmPostOptimizer::raw_processing_(GParameterSet &p) {
	// Make sure p is processed
	if (not p.is_processed()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GEvolutionaryAlgorithmPostOptimizer::raw_processing_: Error!" << std::endl
				<< "Provided base_type has dirty flag set." << std::endl
		);
	}

	if(m_executionMode == execMode::BROKER) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GEvolutionaryAlgorithmPostOptimizer::raw_processing_: Error!" << std::endl
				<< "Got invalid execution mode " << m_executionMode << std::endl
		);
	}

	// Clone the individual for post-processing
	std::shared_ptr<GParameterSet> p_unopt_ptr = p.template clone<GParameterSet>();

	// Make sure the post-optimization does not trigger post-optimization recursively ...
	p_unopt_ptr->vetoPostProcessing(true);

	// Retrieve an evolutionary algorithm
	GEvolutionaryAlgorithmFactory eaFactory(m_oa_configFile);
	auto ea_ptr = eaFactory.get<GEvolutionaryAlgorithm>();

	// Add an executor to the algorithm
	ea_ptr->registerExecutor(m_executionMode, m_executor_configFile);

	// Add our individual to the algorithm
	ea_ptr->push_back(p_unopt_ptr);

	// Perform the actual (sub-)optimization
	ea_ptr->optimize();

	// Retrieve the best individual
	std::shared_ptr<GParameterSet> p_opt_ptr = ea_ptr->getBestGlobalIndividual<GParameterSet>();

	// Make sure subsequent optimization cycles may generally perform post-optimization again.
	// This needs to be done on the optimized individual, as it will be loaded into the
	// original individual.
	p_opt_ptr->vetoPostProcessing(false);

	// Load the parameter data into the argument base_type (will also clear the dirty flag)
	p.cannibalize(*p_opt_ptr);

	return true;
}

/******************************************************************************/
/**
 * The standard constructor. Intentionally private, as it is only needed
 * for de-serialization purposes.
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer()
{ /* nothing */ }


/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GEvolutionaryAlgorithmPostOptimizer::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GPostProcessorBaseT<GParameterSet>::modify_GUnitTests_()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithmPostOptimizer::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvolutionaryAlgorithmPostOptimizer::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GPostProcessorBaseT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests_();

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithmPostOptimizer::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvolutionaryAlgorithmPostOptimizer::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GPostProcessorBaseT<GParameterSet>::specificTestsFailuresExpected_GUnitTests_();

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithmPostOptimizer::specificTestsFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
