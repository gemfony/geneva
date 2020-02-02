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

#include "geneva/GPluggableOptimizationMonitors.hpp"

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GStandardMonitor) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFitnessMonitor) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GCollectiveMonitor) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProgressPlotter) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAllSolutionFileLogger) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GIterationResultsFileLogger) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNAdpationsLogger) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<double>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<bool>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProcessingTimesLogger) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GStandardMonitor::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT: {
			glogger
				<< "Starting an optimization run with algorithm \"" << goa->getAlgorithmName() << "\"" << std::endl
				<< GLOGGING;
		}
			break;

		case Gem::Geneva::infoMode::INFOPROCESSING: {
			glogger
				<< std::setprecision(5)
				<< goa->getIteration() << ": "
				<< Gem::Common::g_to_string(goa->getBestCurrentPrimaryFitness())
				<< " // best past: " << Gem::Common::g_to_string(goa->getBestKnownPrimaryFitness())
				<< std::endl
				<< GLOGGING;
		}
			break;

		case Gem::Geneva::infoMode::INFOEND: {
			glogger
				<< "End of optimization reached in algorithm \"" << goa->getAlgorithmName()
				<< "\"" << std::endl
				<< GLOGGING;
		}
			break;
	}
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GStandardMonitor::name_() const {
	return std::string("GStandardMonitor");
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
void GStandardMonitor::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GStandardMonitor reference independent of this object and convert the pointer
	const GStandardMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GStandardMonitor", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GStandardMonitorT object, camouflaged as a GObject
 */
void GStandardMonitor::load_(const GObject* cp) {
	// Check that we are dealing with a GStandardMonitor reference independent of this object and convert the pointer
	const GStandardMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GStandardMonitor::clone_() const {
	return new GStandardMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GStandardMonitor::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GStandardMonitor::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GStandardMonitor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GStandardMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GStandardMonitor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GStandardMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The copy constructor
 */
GFitnessMonitor::GFitnessMonitor(const GFitnessMonitor& cp)
	: GBasePluggableOM(cp)
	  , m_xDim(cp.m_xDim)
	  , m_yDim(cp.m_yDim)
	  , m_nMonitorInds(cp.m_nMonitorInds)
	  , m_resultFile(cp.m_resultFile)
	  , m_infoInitRun(cp.m_infoInitRun)
{
	Gem::Common::copyCloneableSmartPointerContainer(cp.m_globalFitnessGraphVec, m_globalFitnessGraphVec);
	Gem::Common::copyCloneableSmartPointerContainer(cp.m_iterationFitnessGraphVec, m_iterationFitnessGraphVec);
}

/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param resultFile The desired name of the result file
 */
void GFitnessMonitor::setResultFileName(
	const std::string &resultFile
) {
	m_resultFile = resultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GFitnessMonitor::getResultFileName() const {
	return m_resultFile;
}

/******************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param xDim The desired dimension of the canvas in x-direction
 * @param yDim The desired dimension of the canvas in y-direction
 */
void GFitnessMonitor::setDims(const std::uint32_t &xDim, const std::uint32_t &yDim) {
	m_xDim = xDim;
	m_yDim = yDim;
}

/******************************************************************************/
/**
 * Retrieve the dimensions as a tuple
 *
 * @return The dimensions of the canvas as a tuple
 */
std::tuple<std::uint32_t, std::uint32_t> GFitnessMonitor::getDims() const {
	return std::tuple<std::uint32_t, std::uint32_t>{m_xDim, m_yDim};
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
std::uint32_t GFitnessMonitor::getXDim() const {
	return m_xDim;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
std::uint32_t GFitnessMonitor::getYDim() const {
	return m_yDim;
}

/******************************************************************************/
/**
 * Sets the number of individuals in the population that should be monitored.
 * If m_nMonitorInds == 0, the default will be set to 3, as fitness graphs are plotted in a row,
 * and more than 3 will not give satisfactory graphical results. You may however
 * request more monitored individuals, but will likely have to postprocess the ROOT script.
 * If m_nMonitorInds is set to a larger number than there are individuals in the population,
 * the value will be reset to that amount of individuals in informationFunction.
 *
 * @oaram nMonitorInds The number of individuals in the population that should be monitored
 */
void GFitnessMonitor::setNMonitorIndividuals(const std::size_t &nMonitorInds) {
	// Determine a suitable number of monitored individuals, if it hasn't already
	// been set externally.
	if(m_nMonitorInds == 0) {
		m_nMonitorInds = std::size_t(DEFNMONITORINDS);
	}

	m_nMonitorInds = nMonitorInds;
}

/******************************************************************************/
/**
 * Retrieves the number of individuals that are being monitored
 *
 * @return The number of individuals in the population being monitored
 */
std::size_t GFitnessMonitor::getNMonitorIndividuals() const {
	return m_nMonitorInds;
}

/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GFitnessMonitor::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT: {
			// We set a marker whenever a new INFOINIT call happens. This way we
			// may "chain" algorithms and will get the entire progress information
			// for all algorithms
		} break;

		case Gem::Geneva::infoMode::INFOPROCESSING: {
			// Retrieve the list of globally- and iteration bests individuals
			auto global_bests = goa->G_Interface_OptimizerT::template getBestGlobalIndividuals<GParameterSet>();
			auto iter_bests   = goa->G_Interface_OptimizerT::template getBestIterationIndividuals<GParameterSet>();

			// Retrieve the current iteration in the population
			std::uint32_t iteration = goa->getIteration();

			// We expect both sizes to be identical
			if(global_bests.size() != iter_bests.size()) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GFitnessMonitor::informationFunction_(): Error!" << std::endl
						<< "global_bests.size() = " << global_bests.size() << " != iter_bests.size() = " << iter_bests.size() << std::endl
				);
			}

			//------------------------------------------------------------------------------
			// Setup of local vectors

			if(not m_infoInitRun) {
				// Reset the number of monitored individuals to a suitable value, if necessary.
				if(m_nMonitorInds > global_bests.size()) {
					glogger
						<< "In GFitnessMonitor::informationFunction_(): Warning!" << std::endl
						<< "Requested number of individuals to be monitored in iteration " << iteration << " is larger" << std::endl
						<< "than the number of best individuals " << m_nMonitorInds << " / " << global_bests.size() << std::endl
						<< GWARNING;

					m_nMonitorInds = global_bests.size();
				}

				// Set up the plotters
				for(std::size_t ind = 0; ind<m_nMonitorInds; ind++) {
					std::shared_ptr <Gem::Common::GGraph2D> global_graph(new Gem::Common::GGraph2D());
					global_graph->setXAxisLabel("Iteration");
					global_graph->setYAxisLabel("Best Fitness");
					global_graph->setPlotLabel(std::string("Individual ") + Gem::Common::to_string(ind));
					global_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

					m_globalFitnessGraphVec.push_back(global_graph);

					std::shared_ptr <Gem::Common::GGraph2D> iteration_graph(new Gem::Common::GGraph2D());
					iteration_graph->setXAxisLabel("Iteration");
					iteration_graph->setYAxisLabel("Best Fitness");
					iteration_graph->setPlotLabel(std::string("Individual ") + Gem::Common::to_string(ind));
					iteration_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

					m_iterationFitnessGraphVec.push_back(iteration_graph);

					// Add the iteration graph as secondary plotter
					global_graph->registerSecondaryPlotter(iteration_graph);
				}

				// Make sure m_globalFitnessGraphVec is only initialized once
				m_infoInitRun = true;
			} else {
				// We might have a situation where the number of best individuals changes in each
				// iteration, e.g. when dealing with pareto optimization in EA. In this case we reduce the
				// number of m_nMonitorInds to 1, which is the only safe option. Recorded data of other
				// individuals will then be lost -- the program will warn about this.
				if(m_nMonitorInds > global_bests.size()) {
					glogger
						<< "In GFitnessMonitor::informationFunction_(): Warning!" << std::endl
						<< "Requested number of individuals to be monitored in iteration " << iteration << " is larger" << std::endl
						<< "than the number of best individuals " << m_nMonitorInds << " / " << global_bests.size() << std::endl
						<< "This seems to be a result of a varying number of best individuals." << std::endl
						<< "We will now reduce the number of monitored individuals to 1 for the" << std::endl
						<< "rest of the optimization run. Recorded information for other individuals" << std::endl
						<< "will be deleted" << std::endl
						<< GWARNING;

					m_nMonitorInds = 1;
					m_globalFitnessGraphVec.resize(1);
					m_iterationFitnessGraphVec.resize(1);
				}
			}

			//------------------------------------------------------------------------------
			// Fill in the data for the best individuals

			std::vector<std::shared_ptr<Gem::Common::GGraph2D>>::iterator global_it;
			std::vector<std::shared_ptr<Gem::Common::GGraph2D>>::iterator iter_it;
			std::vector<std::shared_ptr<GParameterSet>>::iterator  global_ind_it;
			std::vector<std::shared_ptr<GParameterSet>>::iterator  iter_ind_it;

			std::size_t mInd = 0;
			for(
				global_it=m_globalFitnessGraphVec.begin(), iter_it=m_iterationFitnessGraphVec.begin(), global_ind_it = global_bests.begin(), iter_ind_it = iter_bests.begin()
				; global_it != m_globalFitnessGraphVec.end()
				; ++global_it, ++iter_it, ++global_ind_it, ++iter_ind_it
			) {
				(*global_it)->add(boost::numeric_cast<double>(iteration), (*global_ind_it)->raw_fitness(0));
				(*iter_it  )->add(boost::numeric_cast<double>(iteration), (*iter_ind_it  )->raw_fitness(0));
			}

			//------------------------------------------------------------------------------

		}
			break;

		case Gem::Geneva::infoMode::INFOEND: { /* nothing */ }
			break;
	}
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFitnessMonitor::name_() const {
	return std::string("GFitnessMonitor");
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
void GFitnessMonitor::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GFitnessMonitor reference independent of this object and convert the pointer
	const GFitnessMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GFitnessMonitor", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_xDim, p_load->m_xDim), token);
	compare_t(IDENTITY(m_yDim, p_load->m_yDim), token);
	compare_t(IDENTITY(m_nMonitorInds, p_load->m_nMonitorInds), token);
	compare_t(IDENTITY(m_resultFile, p_load->m_resultFile), token);
	compare_t(IDENTITY(m_infoInitRun, p_load->m_infoInitRun), token);
	compare_t(IDENTITY(m_globalFitnessGraphVec, p_load->m_globalFitnessGraphVec), token);
	compare_t(IDENTITY(m_iterationFitnessGraphVec, p_load->m_iterationFitnessGraphVec), token);

	// React on deviations from the expectation
	token.evaluate();
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GFitnessMonitorT object, camouflaged as a GObject
 */
void GFitnessMonitor::load_(const GObject* cp) {
	// Check that we are dealing with a GFitnessMonitor reference independent of this object and convert the pointer
	const GFitnessMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... and then our local data
	m_xDim = p_load->m_xDim;
	m_yDim = p_load->m_yDim;
	m_nMonitorInds = p_load->m_nMonitorInds;
	m_resultFile = p_load->m_resultFile;
	m_infoInitRun = p_load->m_infoInitRun;

	Gem::Common::copyCloneableSmartPointerContainer(p_load->m_globalFitnessGraphVec, m_globalFitnessGraphVec);
	Gem::Common::copyCloneableSmartPointerContainer(p_load->m_iterationFitnessGraphVec, m_iterationFitnessGraphVec);
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GFitnessMonitor::clone_() const {
	return new GFitnessMonitor(*this);
}
/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFitnessMonitor::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GFitnessMonitor::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFitnessMonitor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GFitnessMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFitnessMonitor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GFitnessMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The copy constructor
 */
GCollectiveMonitor::GCollectiveMonitor(const GCollectiveMonitor& cp) : GBasePluggableOM(cp)
{
	Gem::Common::copyCloneableSmartPointerContainer(cp.m_pluggable_monitors, m_pluggable_monitors);
}

/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GCollectiveMonitor::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
)  {
	for(auto const & pm_ptr : m_pluggable_monitors) {
		pm_ptr->informationFunction(im,goa);
	}
}

/******************************************************************************/
/**
 * Allows to register a new pluggable monitor
 */
void GCollectiveMonitor::registerPluggableOM(
	std::shared_ptr<Gem::Geneva::GBasePluggableOM> om_ptr
) {
	if(om_ptr) {
		m_pluggable_monitors.push_back(om_ptr);
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GCollectiveMonitor::registerPluggableOM(): Error!" << std::endl
				<< "Got empty pointer to pluggable optimization monitor." << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Checks if adaptors have been registered in the collective monitor
 */
bool GCollectiveMonitor::hasOptimizationMonitors() const {
	return not m_pluggable_monitors.empty();
}

/******************************************************************************/
/**
 * Allows to clear all registered monitors
 */
void GCollectiveMonitor::resetPluggbleOM() {
	m_pluggable_monitors.clear();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GCollectiveMonitor::name_() const  {
	return std::string("GCollectiveMonitor");
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
void GCollectiveMonitor::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const  {
	using namespace Gem::Common;

	// Check that we are dealing with a GCollectiveMonitor reference independent of this object and convert the pointer
	const GCollectiveMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GCollectiveMonitor", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_pluggable_monitors, p_load->m_pluggable_monitors), token);

	// React on deviations from the expectation
	token.evaluate();
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GCollectiveMonitorT object, camouflaged as a GObject
 */
void GCollectiveMonitor::load_(const GObject* cp)  {
	// Check that we are dealing with a GCollectiveMonitor reference independent of this object and convert the pointer
	const GCollectiveMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... and then our local data
	Gem::Common::copyCloneableSmartPointerContainer(p_load->m_pluggable_monitors, m_pluggable_monitors);
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GCollectiveMonitor::clone_() const  {
	return new GCollectiveMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GCollectiveMonitor::modify_GUnitTests_()  {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GCollectiveMonitor::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GCollectiveMonitor::specificTestsNoFailureExpected_GUnitTests_()  {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GCollectiveMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GCollectiveMonitor::specificTestsFailuresExpected_GUnitTests_()  {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GCollectiveMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * Initialization with a file name. Note that some variables may be initialized in the class body.
 */
GAllSolutionFileLogger::GAllSolutionFileLogger(const std::string& fileName)
	: m_fileName(fileName)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a file name and boundaries.
 * Note that some variables may be initialized in the class body.
 */
GAllSolutionFileLogger::GAllSolutionFileLogger(
	const std::string& fileName
	, const std::vector<double>& boundaries
)
	: m_fileName(fileName)
	  , m_boundaries(boundaries)
	  , m_boundariesActive(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GAllSolutionFileLogger::name_() const {
	return std::string("GAllSolutionFileLogger");
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
void GAllSolutionFileLogger::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GAllSolutionFileLogger reference independent of this object and convert the pointer
	const GAllSolutionFileLogger *p_load = Gem::Common::g_convert_and_compare<GObject, GAllSolutionFileLogger>(cp, this);

	GToken token("GAllSolutionFileLogger", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
	compare_t(IDENTITY(m_boundaries, p_load->m_boundaries), token);
	compare_t(IDENTITY(m_boundariesActive, p_load->m_boundariesActive), token);
	compare_t(IDENTITY(m_withNameAndType, p_load->m_withNameAndType), token);
	compare_t(IDENTITY(m_withCommas, p_load->m_withCommas), token);
	compare_t(IDENTITY(m_useRawFitness, p_load->m_useRawFitness), token);
	compare_t(IDENTITY(m_showValidity, p_load->m_showValidity), token);
	compare_t(IDENTITY(m_printInitial, p_load->m_printInitial), token);
	compare_t(IDENTITY(m_showIterationBoundaries, p_load->m_showIterationBoundaries), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name
 */
void GAllSolutionFileLogger::setFileName(const std::string& fileName) {
	m_fileName = fileName;
}

/******************************************************************************/
/**
 * Retrieves the current file name
 */
std::string GAllSolutionFileLogger::getFileName() const {
	return m_fileName;
}

/******************************************************************************/
/**
 * Sets the boundaries
 */
void GAllSolutionFileLogger::setBoundaries(const std::vector<double>& boundaries) {
	m_boundaries = boundaries;
	m_boundariesActive = true;
}

/******************************************************************************/
/**
 * Allows to retrieve the boundaries
 */
std::vector<double> GAllSolutionFileLogger::getBoundaries() const {
	return m_boundaries;
}

/******************************************************************************/
/**
 * Allows to check whether boundaries are active
 */
bool GAllSolutionFileLogger::boundariesActive() const {
	return m_boundariesActive;
}

/******************************************************************************/
/**
 * Allows to inactivate boundaries
 */
void GAllSolutionFileLogger::setBoundariesInactive() {
	m_boundariesActive = false;
}

/******************************************************************************/
/**
 * Allows to specify whether explanations should be printed for parameter-
 * and fitness values.
 */
void GAllSolutionFileLogger::setPrintWithNameAndType(bool withNameAndType) {
	m_withNameAndType = withNameAndType;
}

/******************************************************************************/
/**
 * Allows to check whether explanations should be printed for parameter-
 * and fitness values
 */
bool GAllSolutionFileLogger::getPrintWithNameAndType() const {
	return m_withNameAndType;
}

/******************************************************************************/
/**
 * Allows to specify whether commas should be printed in-between values
 */
void GAllSolutionFileLogger::setPrintWithCommas(bool withCommas) {
	m_withCommas = withCommas;
}

/******************************************************************************/
/**
 * Allows to check whether commas should be printed in-between values
 */
bool GAllSolutionFileLogger::getPrintWithCommas() const {
	return m_withCommas;
}

/******************************************************************************/
/**
 * Allows to specify whether the true (instead of the transformed) fitness should be shown
 */
void GAllSolutionFileLogger::setUseTrueFitness(bool useRawFitness) {
	m_useRawFitness = useRawFitness;
}

/******************************************************************************/
/**
 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
 */
bool GAllSolutionFileLogger::getUseTrueFitness() const {
	return m_useRawFitness;
}

/******************************************************************************/
/**
 * Allows to specify whether the validity of a solution should be shown
 */
void GAllSolutionFileLogger::setShowValidity(bool showValidity) {
	m_showValidity = showValidity;
}

/******************************************************************************/
/**
 * Allows to check whether the validity of a solution will be shown
 */
bool GAllSolutionFileLogger::getShowValidity() const {
	return m_showValidity;
}

/******************************************************************************/
/**
 * Allows to specifiy whether the initial population (prior to any
 * optimization work) should be printed.
 */
void GAllSolutionFileLogger::setPrintInitial(bool printInitial) {
	m_printInitial = printInitial;
}

/******************************************************************************/
/**
 * Allows to check whether the initial population (prior to any
 * optimization work) should be printed.
 */
bool GAllSolutionFileLogger::getPrintInitial() const {
	return m_printInitial;
}

/******************************************************************************/
/**
* Allows to specifiy whether a comment line should be inserted
* between iterations
*/
void GAllSolutionFileLogger::setShowIterationBoundaries(bool showIterationBoundaries) {
	m_showIterationBoundaries = showIterationBoundaries;
}

/******************************************************************************/
/**
 * Allows to check whether a comment line should be inserted
 * between iterations
 */
bool GAllSolutionFileLogger::getShowIterationBoundaries() const {
	return m_showIterationBoundaries;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GAllSolutionFileLogger::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT:
		{
			// If the file pointed to by m_fileName already exists, make a back-up
			if(bf::exists(m_fileName)) {
				std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

				glogger
					<< "In GAllSolutionFileLogger::informationFunction_(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

				bf::rename(m_fileName, newFileName);
			}

			if(m_printInitial) {
				this->printPopulation("Initial population", goa);
			}
		}
			break;

		case Gem::Geneva::infoMode::INFOPROCESSING:
		{
			this->printPopulation("At end of iteration " + Gem::Common::to_string(goa->getIteration()), goa);
		}
			break;

		case Gem::Geneva::infoMode::INFOEND:
			// nothing
			break;
	};
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GAllSolutionFileLoggerT object, camouflaged as a GObject
 */
void GAllSolutionFileLogger::load_(const GObject* cp) {
	// Check that we are dealing with a GAllSolutionFileLogger reference independent of this object and convert the pointer
	const GAllSolutionFileLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... and then our local data
	m_fileName = p_load->m_fileName;
	m_boundaries = p_load->m_boundaries;
	m_boundariesActive = p_load->m_boundariesActive;
	m_withNameAndType = p_load->m_withNameAndType;
	m_withCommas = p_load->m_withCommas;
	m_useRawFitness = p_load->m_useRawFitness;
	m_showValidity = p_load->m_showValidity;
	m_printInitial = p_load->m_printInitial;
	m_showIterationBoundaries = p_load->m_showIterationBoundaries;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GAllSolutionFileLogger::clone_() const {
	return new GAllSolutionFileLogger(*this);
}

/******************************************************************************/
/**
 * Does the actual printing
 */
void GAllSolutionFileLogger::printPopulation(
	const std::string& iterationDescription
	, G_OptimizationAlgorithm_Base const * const goa
) {
	// Open the external file
	boost::filesystem::ofstream data(m_fileName, std::ofstream::app);

	if(m_showIterationBoundaries) {
		data
			<< "#" << std::endl
			<< "# -----------------------------------------------------------------------------" << std::endl
			<< "# " << iterationDescription << ":" << std::endl
			<< "#" << std::endl;
	}

	// Loop over all individuals of the algorithm.
	for(std::size_t pos=0; pos<goa->size(); pos++) {
		std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

		// Note that isGoodEnough may throw if loop acts on a "dirty" individual
		if(not m_boundariesActive || ind->isGoodEnough(m_boundaries)) {
			// Append the data to the external file
			if(0 == pos && goa->inFirstIteration()) { // Only output name and type in the very first line (if at all)
				data << ind->toCSV(m_withNameAndType, m_withCommas, m_useRawFitness, m_showValidity);
			} else {
				data << ind->toCSV(false /* withNameAndType */, m_withCommas, m_useRawFitness, m_showValidity);
			}
		}
	}

	// Close the external file
	data.close();
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GAllSolutionFileLogger::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GAllSolutionFileLogger::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GAllSolutionFileLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GAllSolutionFileLogger::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GAllSolutionFileLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GAllSolutionFileLogger::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * Initialization with a file name. Note that some variables may be initialized
 * in the class body.
 */
GIterationResultsFileLogger::GIterationResultsFileLogger(const std::string& fileName)
	: m_fileName(fileName)
{ /* nothing */ }

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GIterationResultsFileLogger::name_() const {
	return std::string("GIterationResultsFileLogger");
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
void GIterationResultsFileLogger::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GIterationResultsFileLogger
	// reference independent of this object and convert the pointer
	const GIterationResultsFileLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GIterationResultsFileLogger", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
	compare_t(IDENTITY(m_withCommas, p_load->m_withCommas), token);
	compare_t(IDENTITY(m_useRawFitness, p_load->m_useRawFitness), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name
 */
void GIterationResultsFileLogger::setFileName(const std::string& fileName) {
	m_fileName = fileName;
}

/******************************************************************************/
/**
 * Retrieves the current file name
 */
std::string GIterationResultsFileLogger::getFileName() const {
	return m_fileName;
}

/******************************************************************************/
/**
 * Allows to specify whether commas should be printed in-between values
 */
void GIterationResultsFileLogger::setPrintWithCommas(bool withCommas) {
	m_withCommas = withCommas;
}

/******************************************************************************/
/**
 * Allows to check whether commas should be printed in-between values
 */
bool GIterationResultsFileLogger::getPrintWithCommas() const {
	return m_withCommas;
}

/******************************************************************************/
/**
 * Allows to specify whether the true (instead of the transformed) fitness should be shown
 */
void GIterationResultsFileLogger::setUseTrueFitness(bool useRawFitness) {
	m_useRawFitness = useRawFitness;
}

/******************************************************************************/
/**
 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
 */
bool GIterationResultsFileLogger::getUseTrueFitness() const {
	return m_useRawFitness;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GIterationResultsFileLogger::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT:
		{
			// If the file pointed to by m_fileName already exists, make a back-up
			if(bf::exists(m_fileName)) {
				std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

				glogger
					<< "In GIterationResultsFileLogger::informationFunction_(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

				bf::rename(m_fileName, newFileName);
			}
		}
			break;

		case Gem::Geneva::infoMode::INFOPROCESSING:
		{
			// Open the external file
			boost::filesystem::ofstream data(m_fileName.c_str(), std::ofstream::app);
			std::vector<double> fitness_cnt;

			// Loop over all individuals of the algorithm.
			std::size_t nIndividuals = goa->size();
			for(std::size_t pos=0; pos<nIndividuals; pos++) {
				std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
				fitness_cnt = goa->at(pos)->raw_fitness_vec();

				std::size_t nFitnessCriteria = goa->at(0)->getNStoredResults();
				for(std::size_t i=0; i<nFitnessCriteria; i++) {
					data << fitness_cnt.at(i) << ((m_withCommas && (nFitnessCriteria*nIndividuals > (i+1)*(pos+1)))?", ":" ");
				}
			}
			data << std::endl;

			// Close the external file
			data.close();
		}
			break;

		case Gem::Geneva::infoMode::INFOEND:
			// nothing
			break;
	};
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GIterationResultsFileLoggerT object, camouflaged as a GObject
 */
void GIterationResultsFileLogger::load_(const GObject* cp) {
	// Check that we are dealing with a GIterationResultsFileLogger
	// reference independent of this object and convert the pointer
	const GIterationResultsFileLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... and then our local data
	m_fileName = p_load->m_fileName;
	m_withCommas = p_load->m_withCommas;
	m_useRawFitness = p_load->m_useRawFitness;
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GIterationResultsFileLogger::clone_() const {
	return new GIterationResultsFileLogger(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GIterationResultsFileLogger::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GIterationResultsFileLogger::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GIterationResultsFileLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GIterationResultsFileLogger::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GIterationResultsFileLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GIterationResultsFileLogger::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with a file name. Note that some variables may be
 * initialized in the class body.
 */
GNAdpationsLogger::GNAdpationsLogger(const std::string& fileName)
	: m_fileName(fileName)
	  , m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
	  , m_gpd("Number of adaptions per iteration", 1, 2)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GNAdpationsLogger::GNAdpationsLogger(const GNAdpationsLogger& cp)
	: m_fileName(cp.m_fileName)
	  , m_canvasDimensions(cp.m_canvasDimensions)
	  , m_gpd(cp.m_gpd)
	  , m_monitorBestOnly(cp.m_monitorBestOnly)
	  , m_addPrintCommand(cp.m_addPrintCommand)
	  , m_maxIteration(cp.m_maxIteration)
	  , m_nIterationsRecorded(cp.m_nIterationsRecorded)
	  , m_nAdaptionsStore(cp.m_nAdaptionsStore)
{
	Gem::Common::copyCloneableSmartPointer(cp.m_nAdaptionsHist2D_oa, m_nAdaptionsHist2D_oa);
	Gem::Common::copyCloneableSmartPointer(cp.m_nAdaptionsGraph2D_oa, m_nAdaptionsGraph2D_oa);
	Gem::Common::copyCloneableSmartPointer(cp.m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
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
void GNAdpationsLogger::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GNAdpationsLogger reference independent of this object and convert the pointer
	const GNAdpationsLogger *p_load = Gem::Common::g_convert_and_compare<GObject, GNAdpationsLogger>(cp, this);

	GToken token("GNAdpationsLogger", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
	compare_t(IDENTITY(m_canvasDimensions, p_load->m_canvasDimensions), token);
	compare_t(IDENTITY(m_gpd, p_load->m_gpd), token);
	compare_t(IDENTITY(m_nAdaptionsHist2D_oa, p_load->m_nAdaptionsHist2D_oa), token);
	compare_t(IDENTITY(m_nAdaptionsGraph2D_oa, p_load->m_nAdaptionsGraph2D_oa), token);
	compare_t(IDENTITY(m_fitnessGraph2D_oa, p_load->m_fitnessGraph2D_oa), token);
	compare_t(IDENTITY(m_monitorBestOnly, p_load->m_monitorBestOnly), token);
	compare_t(IDENTITY(m_addPrintCommand, p_load->m_addPrintCommand), token);
	compare_t(IDENTITY(m_maxIteration, p_load->m_maxIteration), token);
	compare_t(IDENTITY(m_nIterationsRecorded, p_load->m_nIterationsRecorded), token);
	compare_t(IDENTITY(m_nAdaptionsStore, p_load->m_nAdaptionsStore), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name
 */
void GNAdpationsLogger::setFileName(const std::string& fileName) {
	m_fileName = fileName;
}

/******************************************************************************/
/**
 * Retrieves the current file name
 */
std::string GNAdpationsLogger::getFileName() const {
	return m_fileName;
}

/******************************************************************************/
/**
 * Allows to specify whether only the best individuals should be monitored.
 */
void GNAdpationsLogger::setMonitorBestOnly(bool monitorBestOnly) {
	m_monitorBestOnly = monitorBestOnly;
}

/******************************************************************************/
/**
 * Allows to check whether only the best individuals should be monitored.
 */
bool GNAdpationsLogger::getMonitorBestOnly() const {
	return m_monitorBestOnly;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions
 */
void GNAdpationsLogger::setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
	m_canvasDimensions = canvasDimensions;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions using separate x and y values
 */
void GNAdpationsLogger::setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
	m_canvasDimensions = std::tuple<std::uint32_t,std::uint32_t>(x,y);
}

/******************************************************************************/
/**
 * Gives access to the canvas dimensions
 */
std::tuple<std::uint32_t,std::uint32_t> GNAdpationsLogger::getCanvasDimensions() const {
	return m_canvasDimensions;
}

/******************************************************************************/
/**
 * Allows to add a "Print" command to the end of the script so that picture files are created
 */
void GNAdpationsLogger::setAddPrintCommand(bool addPrintCommand) {
	m_addPrintCommand = addPrintCommand;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the m_addPrintCommand variable
 */
bool GNAdpationsLogger::getAddPrintCommand() const {
	return m_addPrintCommand;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GNAdpationsLogger::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	using namespace Gem::Common;

	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT:
		{
			// If the file pointed to by m_fileName already exists, make a back-up
			if(bf::exists(m_fileName)) {
				std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

				glogger
					<< "In GNAdpationsLogger::informationFunction_(): Error!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

				bf::rename(m_fileName, newFileName);
			}

			// Make sure the progress plotter has the desired size
			m_gpd.setCanvasDimensions(m_canvasDimensions);

			// Set up a graph to monitor the best fitness found
			m_fitnessGraph2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
			m_fitnessGraph2D_oa->setXAxisLabel("Iteration");
			m_fitnessGraph2D_oa->setYAxisLabel("Fitness");
			m_fitnessGraph2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);
		}
			break;

		case Gem::Geneva::infoMode::INFOPROCESSING:
		{
			std::uint32_t iteration = goa->getIteration();

			// Record the current fitness
			std::shared_ptr<GParameterSet> p = goa->G_Interface_OptimizerT::template getBestGlobalIndividual<GParameterSet>();
			(*m_fitnessGraph2D_oa) & std::tuple<double,double>(double(iteration), p->raw_fitness(0));

			// Update the largest known iteration and the number of recorded iterations
			m_maxIteration = iteration;
			m_nIterationsRecorded++;

			// Do the actual logging
			if(m_monitorBestOnly) {
				std::shared_ptr<GParameterSet> best = goa->G_Interface_OptimizerT::template getBestGlobalIndividual<GParameterSet>();
				m_nAdaptionsStore.emplace_back(double(iteration), double(best->getNAdaptions()));
			} else { // Monitor all individuals
				// Loop over all individuals of the algorithm.
				for(std::size_t pos=0; pos<goa->size(); pos++) {
					std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
					m_nAdaptionsStore.emplace_back(double(iteration), double(ind->getNAdaptions()));
				}
			}
		}
			break;

		case Gem::Geneva::infoMode::INFOEND:
		{
			std::vector<std::tuple<double, double>>::iterator it;

			if(m_monitorBestOnly) {
				// Create the graph object
				m_nAdaptionsGraph2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
				m_nAdaptionsGraph2D_oa->setXAxisLabel("Iteration");
				m_nAdaptionsGraph2D_oa->setYAxisLabel("Number of parameter adaptions");
				m_nAdaptionsGraph2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);

				// Fill the object with data
				for(it=m_nAdaptionsStore.begin(); it!=m_nAdaptionsStore.end(); ++it) {
					(*m_nAdaptionsGraph2D_oa) & *it;
				}

				// Add the histogram to the plot designer
				m_gpd.registerPlotter(m_nAdaptionsGraph2D_oa);

			} else { // All individuals are monitored
				// Within m_nAdaptionsStore, find the largest number of adaptions performed
				std::size_t maxNAdaptions = 0;
				for(it=m_nAdaptionsStore.begin(); it!=m_nAdaptionsStore.end(); ++it) {
					if(std::get<1>(*it) > maxNAdaptions) {
						maxNAdaptions = boost::numeric_cast<std::size_t>(std::get<1>(*it));
					}
				}

				// Create the histogram object
				m_nAdaptionsHist2D_oa = std::shared_ptr<GHistogram2D>(
					new GHistogram2D(
						m_nIterationsRecorded
						, maxNAdaptions+1
						, 0., double(m_maxIteration)
						, 0., double(maxNAdaptions)
					)
				);

				m_nAdaptionsHist2D_oa->setXAxisLabel("Iteration");
				m_nAdaptionsHist2D_oa->setYAxisLabel("Number of parameter adaptions");
				m_nAdaptionsHist2D_oa->setDrawingArguments("BOX");

				// Fill the object with data
				for(it=m_nAdaptionsStore.begin(); it!=m_nAdaptionsStore.end(); ++it) {
					(*m_nAdaptionsHist2D_oa) & *it;
				}

				// Add the histogram to the plot designer
				m_gpd.registerPlotter(m_nAdaptionsHist2D_oa);
			}

			// Add the fitness monitor
			m_gpd.registerPlotter(m_fitnessGraph2D_oa);

			// Inform the plot designer whether it should print png files
			m_gpd.setAddPrintCommand(m_addPrintCommand);

			// Write out the result. Note that we add
			m_gpd.writeToFile(m_fileName);

			// Remove all plotters
			m_gpd.resetPlotters();
			m_nAdaptionsHist2D_oa.reset();
			m_nAdaptionsGraph2D_oa.reset();
		}
			break;
	};
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GNAdpationsLoggerT object, camouflaged as a GObject
 */
void GNAdpationsLogger::load_(const GObject* cp) {
	// Check that we are dealing with a GNAdpationsLogger reference independent of this object and convert the pointer
	const GNAdpationsLogger *p_load = Gem::Common::g_convert_and_compare<GObject, GNAdpationsLogger>(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... and then our local data
	m_fileName = p_load->m_fileName;
	m_canvasDimensions = p_load->m_canvasDimensions;
	m_gpd = p_load->m_gpd;
	Gem::Common::copyCloneableSmartPointer(p_load->m_nAdaptionsHist2D_oa, m_nAdaptionsHist2D_oa);
	Gem::Common::copyCloneableSmartPointer(p_load->m_nAdaptionsGraph2D_oa, m_nAdaptionsGraph2D_oa);
	Gem::Common::copyCloneableSmartPointer(p_load->m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
	m_monitorBestOnly = p_load->m_monitorBestOnly;
	m_addPrintCommand = p_load->m_addPrintCommand;
	m_maxIteration = p_load->m_maxIteration;
	m_nIterationsRecorded = p_load->m_nIterationsRecorded;
	m_nAdaptionsStore = p_load->m_nAdaptionsStore;
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GNAdpationsLogger::clone_() const {
	return new GNAdpationsLogger(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GNAdpationsLogger::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GNAdpationsLogger::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GNAdpationsLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GNAdpationsLogger::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GNAdpationsLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GNAdpationsLogger::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The default constructor. Note that some variables may be initialized in the class body.
 */
GProcessingTimesLogger::GProcessingTimesLogger() = default;

/******************************************************************************/
/**
 * Initialization with a file name. Note that some variables may be initialized in the class body.
 */
GProcessingTimesLogger::GProcessingTimesLogger(
	const std::string& fileName_pth
	, const std::string& fileName_pth2
	, const std::string& fileName_txt
	, std::size_t nBinsX
	, std::size_t nBinsY
)
	: m_fileName_pth(fileName_pth)
	  , m_canvasDimensions_pth(std::tuple<std::uint32_t,std::uint32_t>(1600,1200))
	  , m_gpd_pth("Timings for the processing steps of individuals", 2, 2)
	  , m_fileName_pth2(fileName_pth2)
	  , m_canvasDimensions_pth2(std::tuple<std::uint32_t,std::uint32_t>(1600,1200))
	  , m_gpd_pth2("Timings for the processing steps of individuals vs. iteration", 2, 2)
	  , m_fileName_txt(fileName_txt)
	  , m_nBinsX(nBinsX)
	  , m_nBinsY(nBinsY)
{ /* nothing */ }

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GProcessingTimesLogger::name_() const {
	return std::string("GProcessingTimesLogger");
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
void GProcessingTimesLogger::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GProcessingTimesLogger reference independent of this object and convert the pointer
	const GProcessingTimesLogger *p_load = Gem::Common::g_convert_and_compare<GObject, GProcessingTimesLogger>(cp, this);

	GToken token("GProcessingTimesLogger", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBasePluggableOM>(*this, *p_load, token);

	// ... and then our local data
	compare_t(IDENTITY(m_fileName_pth, p_load->m_fileName_pth), token);
	compare_t(IDENTITY(m_canvasDimensions_pth, p_load->m_canvasDimensions_pth), token);
	compare_t(IDENTITY(m_gpd_pth, p_load->m_gpd_pth), token);
	compare_t(IDENTITY(m_fileName_pth2, p_load->m_fileName_pth2), token);
	compare_t(IDENTITY(m_canvasDimensions_pth2, p_load->m_canvasDimensions_pth2), token);
	compare_t(IDENTITY(m_gpd_pth2, p_load->m_gpd_pth2), token);
	compare_t(IDENTITY(m_fileName_txt, p_load->m_fileName_txt), token);
	compare_t(IDENTITY(m_pre_processing_times_hist, p_load->m_pre_processing_times_hist), token);
	compare_t(IDENTITY(m_processing_times_hist, p_load->m_processing_times_hist), token);
	compare_t(IDENTITY(m_post_processing_times_hist, p_load->m_post_processing_times_hist), token);
	compare_t(IDENTITY(m_all_processing_times_hist, p_load->m_all_processing_times_hist), token);
	compare_t(IDENTITY(m_pre_processing_times_hist2D, p_load->m_pre_processing_times_hist2D), token);
	compare_t(IDENTITY(m_processing_times_hist2D, p_load->m_processing_times_hist2D), token);
	compare_t(IDENTITY(m_post_processing_times_hist2D, p_load->m_post_processing_times_hist2D), token);
	compare_t(IDENTITY(m_all_processing_times_hist2D, p_load->m_all_processing_times_hist2D), token);
	compare_t(IDENTITY(m_nBinsX, p_load->m_nBinsX), token);
	compare_t(IDENTITY(m_nBinsY, p_load->m_nBinsY), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name for the processing times histogram
 */
void GProcessingTimesLogger::setFileName_pth(const std::string& fileName) {
	m_fileName_pth = fileName;
}

/******************************************************************************/
/**
 * Retrieves the current file name for the processing times histogram
 */
std::string GProcessingTimesLogger::getFileName_pth() const {
	return m_fileName_pth;
}

/******************************************************************************/
/**
 * Sets the file name for the processing times histograms (2D)
 */
void GProcessingTimesLogger::setFileName_pth2(const std::string& fileName) {
	m_fileName_pth2 = fileName;
}

/******************************************************************************/
/**
 * Retrieves the current file name for the processing times histograms (2D)
 */
std::string GProcessingTimesLogger::getFileName_pth2() const {
	return m_fileName_pth2;
}

/******************************************************************************/
/**
 * Sets the file name for the text output
 */
void GProcessingTimesLogger::setFileName_txt(const std::string& fileName) {
	m_fileName_txt = fileName;
}

/******************************************************************************/
/**
 * Retrieves the current file name for the text output
 */
std::string GProcessingTimesLogger::getFileName_txt() const {
	return m_fileName_txt;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions for the processing times histograms
 */
void GProcessingTimesLogger::setCanvasDimensions_pth(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
	m_canvasDimensions_pth = canvasDimensions;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions using separate x and y values for the
 * processing times histograms
 */
void GProcessingTimesLogger::setCanvasDimensions_pth(std::uint32_t x, std::uint32_t y) {
	m_canvasDimensions_pth = std::tuple<std::uint32_t,std::uint32_t>(x,y);
}

/******************************************************************************/
/**
 * Gives access to the canvas dimensions of the processing times histograms
 */
std::tuple<std::uint32_t,std::uint32_t> GProcessingTimesLogger::getCanvasDimensions_pth() const {
	return m_canvasDimensions_pth;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions for the processing times histograms (2D)
 */
void GProcessingTimesLogger::setCanvasDimensions_pth2(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
	m_canvasDimensions_pth2 = canvasDimensions;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions using separate x and y values for the
 * processing times histograms (2D)
 */
void GProcessingTimesLogger::setCanvasDimensions_pth2(std::uint32_t x, std::uint32_t y) {
	m_canvasDimensions_pth2 = std::tuple<std::uint32_t,std::uint32_t>(x,y);
}

/******************************************************************************/
/**
 * Gives access to the canvas dimensions of the processing times histograms (2D)
 */
std::tuple<std::uint32_t,std::uint32_t> GProcessingTimesLogger::getCanvasDimensions_pth2() const {
	return m_canvasDimensions_pth2;
}

/******************************************************************************/
/**
 * Sets the number of bins for the processing times histograms in y-direction
 */
void GProcessingTimesLogger::setNBinsX(std::size_t nBinsX) {
	if(nBinsX > 0) {
		m_nBinsX = nBinsX;
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GProcessingTimesLogger::setNBinsX(): Error!" << std::endl
				<< "nBinsX is set to 0" << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Retrieves the current number of bins for the processing times
 * histograms in x-direction
 */
std::size_t GProcessingTimesLogger::getNBinsX() const {
	return m_nBinsX;
}

/******************************************************************************/
/**
 * Sets the number of bins for the processing times histograms in y-direction
 */
void GProcessingTimesLogger::setNBinsY(std::size_t nBinsY) {
	if(nBinsY > 0) {
		m_nBinsY = nBinsY;
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GProcessingTimesLogger::setNBinsY(): Error!" << std::endl
				<< "nBinsY is set to 0" << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Retrieves the current number of bins for the processing times
 * histograms in y-direction
 */
std::size_t GProcessingTimesLogger::getNBinsY() const {
	return m_nBinsY;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GProcessingTimesLogger::informationFunction_(
	infoMode im
	, G_OptimizationAlgorithm_Base const * const goa
) {
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT: {
			//---------------------------------------------------------------
			// Histograms

			// If the file pointed to by m_fileName_pth already exists, make a back-up
			if(bf::exists(m_fileName_pth)) {
				std::string newFileName = m_fileName_pth + ".bak_" + Gem::Common::getMSSince1970();

				glogger
					<< "In GProcessingTimesLogger::informationFunction_(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName_pth << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

				bf::rename(m_fileName_pth, newFileName);
			}

			// Make sure the processing times plotter has the desired size
			m_gpd_pth.setCanvasDimensions(m_canvasDimensions_pth);

			m_pre_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
			m_pre_processing_times_hist->setXAxisLabel("Pre-processing time [s]");
			m_pre_processing_times_hist->setYAxisLabel("Number of Entries");
			m_pre_processing_times_hist->setDrawingArguments("hist");

			m_gpd_pth.registerPlotter(m_pre_processing_times_hist);

			m_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
			m_processing_times_hist->setXAxisLabel("Main processing time [s]");
			m_processing_times_hist->setYAxisLabel("Number of Entries");
			m_processing_times_hist->setDrawingArguments("hist");

			m_gpd_pth.registerPlotter(m_processing_times_hist);

			m_post_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
			m_post_processing_times_hist->setXAxisLabel("Post-processing time [s]");
			m_post_processing_times_hist->setYAxisLabel("Number of Entries");
			m_post_processing_times_hist->setDrawingArguments("hist");

			m_gpd_pth.registerPlotter(m_post_processing_times_hist);

			m_all_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
			m_all_processing_times_hist->setXAxisLabel("Overall processing time for all steps [s]");
			m_all_processing_times_hist->setYAxisLabel("Number of Entries");
			m_all_processing_times_hist->setDrawingArguments("hist");

			m_gpd_pth.registerPlotter(m_all_processing_times_hist);

			//---------------------------------------------------------------
			// 2D Histograms

			// If the file pointed to by m_fileName_pth2 already exists, make a back-up
			if(bf::exists(m_fileName_pth2)) {
				std::string newFileName = m_fileName_pth2 + ".bak_" + Gem::Common::getMSSince1970();

				glogger
					<< "In GProcessingTimesLogger::informationFunction_(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName_pth2 << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

				bf::rename(m_fileName_pth2, newFileName);
			}

			// Make sure the processing times has the desired size
			m_gpd_pth2.setCanvasDimensions(m_canvasDimensions_pth2);

			m_pre_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
			m_pre_processing_times_hist2D->setXAxisLabel("Iteration");
			m_pre_processing_times_hist2D->setYAxisLabel("Pre-processing time [s]");
			m_pre_processing_times_hist2D->setZAxisLabel("Number of Entries");
			m_pre_processing_times_hist2D->setDrawingArguments("box");

			m_gpd_pth2.registerPlotter(m_pre_processing_times_hist2D);


			m_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
			m_processing_times_hist2D->setXAxisLabel("Iteration");
			m_processing_times_hist2D->setYAxisLabel("Main processing time [s]");
			m_processing_times_hist2D->setZAxisLabel("Number of Entries");
			m_processing_times_hist2D->setDrawingArguments("box");

			m_gpd_pth2.registerPlotter(m_processing_times_hist2D);

			m_post_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
			m_post_processing_times_hist2D->setXAxisLabel("Iteration");
			m_post_processing_times_hist2D->setYAxisLabel("Post-processing time [s]");
			m_post_processing_times_hist2D->setZAxisLabel("Number of Entries");
			m_post_processing_times_hist2D->setDrawingArguments("box");

			m_gpd_pth2.registerPlotter(m_post_processing_times_hist2D);

			m_all_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
			m_all_processing_times_hist2D->setXAxisLabel("Iteration");
			m_all_processing_times_hist2D->setYAxisLabel("Overall processing time [s]");
			m_all_processing_times_hist2D->setZAxisLabel("Number of Entries");
			m_all_processing_times_hist2D->setDrawingArguments("box");

			m_gpd_pth2.registerPlotter(m_all_processing_times_hist2D);

			//---------------------------------------------------------------
			// Make sure the output file is empty (rename, if it exists)

			// If the file pointed to by m_fileName_txt already exists, make a back-up
			if(bf::exists(m_fileName_txt)) {
				std::string newFileName = m_fileName_txt + ".bak_" + Gem::Common::getMSSince1970();

				glogger
					<< "In GProcessingTimesLogger::informationFunction_(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName_pth2 << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

				bf::rename(m_fileName_txt, newFileName);
			}

			//---------------------------------------------------------------

		} break;

		case Gem::Geneva::infoMode::INFOPROCESSING: {
			// Open the external text-file
			boost::filesystem::ofstream data_txt(m_fileName_txt, std::ofstream::app);

			// Retrieve the current iteration in the population
			auto iteration = boost::numeric_cast<double>(goa->getIteration());

			// Loop over all individuals of the algorithm.
			for(std::size_t pos=0; pos<goa->size(); pos++) {
				// Get access to each individual in sequence
				std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

				// Retrieve the processing timings
				std::tuple<double,double,double> processingTimes = ind->getProcessingTimes();

				double preProcessingTime = std::get<0>(processingTimes);
				double mainProcessingTime = std::get<1>(processingTimes);
				double postProcessingTime = std::get<2>(processingTimes);
				double allProcessingTime = preProcessingTime + mainProcessingTime + postProcessingTime;

				// Fill the timings into the histograms
				m_pre_processing_times_hist->add(preProcessingTime); // PREPROCESSING
				m_processing_times_hist->add(mainProcessingTime); // PROCESSING
				m_post_processing_times_hist->add(postProcessingTime); // POSTPROCESSING
				m_all_processing_times_hist->add(allProcessingTime); // OVERALL PROCESSING TIME

				// Fill the timings into the 2D histograms ...
				m_pre_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(preProcessingTime)); // PREPROCESSING
				m_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(mainProcessingTime)); // PROCESSING
				m_post_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(postProcessingTime)); // POSTPROCESSING
				m_all_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(allProcessingTime)); // OVERALL PROCESSING TIME

				data_txt << boost::numeric_cast<std::uint32_t>(iteration) << ", " << std::showpoint << preProcessingTime << ", " << mainProcessingTime << ", " << postProcessingTime << std::endl;
			}

			// Close the external text-file
			data_txt.close();
		}
			break;

		case Gem::Geneva::infoMode::INFOEND: {
			// Write out the results
			m_gpd_pth.writeToFile(m_fileName_pth);
			m_gpd_pth2.writeToFile(m_fileName_pth2);

			// Remove all plotters
			m_gpd_pth.resetPlotters();
			m_gpd_pth2.resetPlotters();

			m_pre_processing_times_hist.reset();
			m_processing_times_hist.reset();
			m_post_processing_times_hist.reset();
			m_all_processing_times_hist.reset();

			m_pre_processing_times_hist2D.reset();
			m_processing_times_hist2D.reset();
			m_post_processing_times_hist2D.reset();
			m_all_processing_times_hist2D.reset();
		}
			break;
	};
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GProcessingTimesLoggerT object, camouflaged as a GObject
 */
void GProcessingTimesLogger::load_(const GObject* cp) {
	// Check that we are dealing with a GProcessingTimesLogger reference independent of this object and convert the pointer
	const GProcessingTimesLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load the parent classes' data ...
	GBasePluggableOM::load_(cp);

	// ... and then our local data
	m_fileName_pth = p_load->m_fileName_pth;
	m_canvasDimensions_pth = p_load->m_canvasDimensions_pth;
	m_gpd_pth = p_load->m_gpd_pth;

	m_fileName_pth2 = p_load->m_fileName_pth2;
	m_canvasDimensions_pth2 = p_load->m_canvasDimensions_pth2;
	m_gpd_pth2 = p_load->m_gpd_pth2;

	m_fileName_txt = p_load->m_fileName_txt;

	Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processing_times_hist, m_pre_processing_times_hist);
	Gem::Common::copyCloneableSmartPointer(p_load->m_processing_times_hist, m_processing_times_hist);
	Gem::Common::copyCloneableSmartPointer(p_load->m_post_processing_times_hist, m_post_processing_times_hist);
	Gem::Common::copyCloneableSmartPointer(p_load->m_all_processing_times_hist, m_all_processing_times_hist);

	Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processing_times_hist2D, m_pre_processing_times_hist2D);
	Gem::Common::copyCloneableSmartPointer(p_load->m_processing_times_hist2D, m_processing_times_hist2D);
	Gem::Common::copyCloneableSmartPointer(p_load->m_post_processing_times_hist2D, m_post_processing_times_hist2D);
	Gem::Common::copyCloneableSmartPointer(p_load->m_all_processing_times_hist2D, m_all_processing_times_hist2D);

	m_nBinsX = p_load->m_nBinsX;
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GProcessingTimesLogger::clone_() const {
	return new GProcessingTimesLogger(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GProcessingTimesLogger::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests_()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GProcessingTimesLogger::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GProcessingTimesLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GProcessingTimesLogger::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GProcessingTimesLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GProcessingTimesLogger::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
