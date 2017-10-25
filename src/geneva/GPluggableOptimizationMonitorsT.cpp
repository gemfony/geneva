/**
 * @file GPluggableOptimizationMonitors.cpp
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

#include "geneva/GPluggableOptimizationMonitorsT.hpp"

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GStandardMonitor)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFitnessMonitor)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GCollectiveMonitor)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProgressPlotter)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAllSolutionFileLogger)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GIterationResultsFileLogger)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNAdpationsLogger)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<double>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<bool>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProcessingTimesLogger)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * A standard assignment operator
 */
const GStandardMonitor& GStandardMonitor::operator=(const GStandardMonitor& cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GStandardMonitorT object
 *
 * @param  cp A constant reference to another GStandardMonitorT object
 * @return A boolean indicating whether both objects are equal
 */
bool GStandardMonitor::operator==(const GStandardMonitor& cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch(g_expectation_violation&) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GStandardMonitorT object
 *
 * @param  cp A constant reference to another GStandardMonitorT object
 * @return A boolean indicating whether both objects are inequal
 */
bool GStandardMonitor::operator!=(const GStandardMonitor& cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch(g_expectation_violation&) {
		return false;
	}
}

/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GStandardMonitor::informationFunction(
	const infoMode& im
	, G_OptimizationAlgorithm_Base *const goa
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
std::string GStandardMonitor::name() const {
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
void GStandardMonitor::compare(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GStandardMonitor reference independent of this object and convert the pointer
	const GStandardMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GStandardMonitor", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBasePluggableOM>(IDENTITY(*this, *p_load), token);

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
bool GStandardMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GStandardMonitor::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GStandardMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GStandardMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GStandardMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GStandardMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
 * A standard assignment operator
 */
const GFitnessMonitor& GFitnessMonitor::operator=(const GFitnessMonitor& cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GFitnessMonitorT object
 *
 * @param  cp A constant reference to another GFitnessMonitorT object
 * @return A boolean indicating whether both objects are equal
 */
bool GFitnessMonitor::operator==(const GFitnessMonitor& cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch(g_expectation_violation&) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GFitnessMonitorT object
 *
 * @param  cp A constant reference to another GFitnessMonitorT object
 * @return A boolean indicating whether both objects are inequal
 */
bool GFitnessMonitor::operator!=(const GFitnessMonitor& cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch(g_expectation_violation&) {
		return false;
	}
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
	return std::tuple<std::uint32_t, std::uint32_t>(m_xDim, m_yDim);
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
void GFitnessMonitor::informationFunction(
	const infoMode& im
	, G_OptimizationAlgorithm_Base *const goa
) {
	switch(im) {
		case Gem::Geneva::infoMode::INFOINIT: {
			// We set a marker whenever a new INFOINIT call happens. This way we
			// may "chain" algorithms and will get the entire progress information
			// for all algorithms
		} break;

		case Gem::Geneva::infoMode::INFOPROCESSING: {
			// Retrieve the list of globally- and iteration bests individuals
			auto global_bests = goa->G_Interface_Optimizer::template getBestGlobalIndividuals<GParameterSet>();
			auto iter_bests   = goa->G_Interface_Optimizer::template getBestIterationIndividuals<GParameterSet>();

			// Retrieve the current iteration in the population
			std::uint32_t iteration = goa->getIteration();

			// We expect both sizes to be identical
			if(global_bests.size() != iter_bests.size()) {
				glogger
					<< "In GFitnessMonitor::informationFunction(): Error!" << std::endl
					<< "global_bests.size() = " << global_bests.size() << " != iter_bests.size() = " << iter_bests.size() << std::endl
					<< GEXCEPTION;
			}

			//------------------------------------------------------------------------------
			// Setup of local vectors

			if(!m_infoInitRun) {
				// Reset the number of monitored individuals to a suitable value, if necessary.
				if(m_nMonitorInds > global_bests.size()) {
					glogger
						<< "In GFitnessMonitor::informationFunction(): Warning!" << std::endl
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
					global_graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
					global_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

					m_globalFitnessGraphVec.push_back(global_graph);

					std::shared_ptr <Gem::Common::GGraph2D> iteration_graph(new Gem::Common::GGraph2D());
					iteration_graph->setXAxisLabel("Iteration");
					iteration_graph->setYAxisLabel("Best Fitness");
					iteration_graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
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
						<< "In GFitnessMonitor::informationFunction(): Warning!" << std::endl
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
				(*global_it)->add(boost::numeric_cast<double>(iteration), (*global_ind_it)->fitness());
				(*iter_it  )->add(boost::numeric_cast<double>(iteration), (*iter_ind_it  )->fitness());
			}

			//------------------------------------------------------------------------------

		} break;

		case Gem::Geneva::infoMode::INFOEND: {
			/* nothing */
		} break;

		default: {
			glogger
				<< "In GFitnessMonitor::informationFunction(): Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
		} break;
	}
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFitnessMonitor::name() const {
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
void GFitnessMonitor::compare(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GFitnessMonitor reference independent of this object and convert the pointer
	const GFitnessMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GFitnessMonitor", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBasePluggableOM>(IDENTITY(*this, *p_load), token);

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
bool GFitnessMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(GBasePluggableOM::modify_GUnitTests()) {
		result = true;
	}

	// no local data -- nothing to change

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GFitnessMonitor::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFitnessMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GFitnessMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFitnessMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GFitnessMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
