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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/GPluggableOptimizationMonitors.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/Interface/GOptimizerIT.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GStandardMonitor)                     // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFitnessMonitor)                      // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GCollectiveMonitor)                   // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProgressPlotter)                     // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAllSolutionFileLogger)               // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GIterationResultsFileLogger)          // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNAdpationsLogger)                    // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<double>)       // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<bool>)         // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProcessingTimesLogger)               // NOLINT

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GStandardMonitor::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    switch(im) {
    case Gem::Geneva::infoMode::INFOINIT: {
        glogger << "Starting an optimization run with algorithm \"" << goa->getAlgorithmName()
                << "\"" << '\n'
                << GLOGGING;
    } break;

    case Gem::Geneva::infoMode::INFOPROCESSING: {
        glogger << std::setprecision(5) << goa->getIteration() << ": "
                << Gem::Common::g_to_string(goa->getBestCurrentPrimaryFitness())
                << " // best past: " << Gem::Common::g_to_string(goa->getBestKnownPrimaryFitness())
                << '\n'
                << GLOGGING;
    } break;

    case Gem::Geneva::infoMode::INFOEND: {
        glogger << "End of optimization reached in algorithm \"" << goa->getAlgorithmName() << "\""
                << '\n'
                << GLOGGING;
    } break;
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
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GStandardMonitor::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GStandardMonitor reference independent of this object and convert the pointer
    const GStandardMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

    GToken token("GStandardMonitor", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GStandardMonitorT object, camouflaged as a GBasePluggableOM
 */
void GStandardMonitor::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GStandardMonitor reference independent of this object and convert the pointer
    const GStandardMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GStandardMonitor::clone_() const {
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

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GStandardMonitor::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GStandardMonitor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GStandardMonitor::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GStandardMonitor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GStandardMonitor::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The copy constructor
 */
GFitnessMonitor::GFitnessMonitor(const GFitnessMonitor &cp)
  : oa::GBasePluggableOM(cp)
  , x_dim_(cp.x_dim_)
  , y_dim_(cp.y_dim_)
  , n_monitor_inds_(cp.n_monitor_inds_)
  , result_file_(cp.result_file_)
  , info_init_run_(cp.info_init_run_) {
    Gem::Common::copyCloneableSmartPointerContainer(
        cp.global_fitness_graph_vec_,
        global_fitness_graph_vec_
    );
    Gem::Common::copyCloneableSmartPointerContainer(
        cp.iteration_fitness_graph_vec_,
        iteration_fitness_graph_vec_
    );
}

/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param result_file The desired name of the result file
 */
void GFitnessMonitor::setResultFileName(const std::string &result_file) {
    result_file_ = result_file;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GFitnessMonitor::getResultFileName() const {
    return result_file_;
}

/******************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param x_dim The desired dimension of the canvas in x-direction
 * @param y_dim The desired dimension of the canvas in y-direction
 */
void GFitnessMonitor::setDims(const std::uint32_t &x_dim, const std::uint32_t &y_dim) {
    x_dim_ = x_dim;
    y_dim_ = y_dim;
}

/******************************************************************************/
/**
 * Retrieve the dimensions as a tuple
 *
 * @return The dimensions of the canvas as a tuple
 */
std::tuple<std::uint32_t, std::uint32_t> GFitnessMonitor::getDims() const {
    return std::tuple<std::uint32_t, std::uint32_t>{x_dim_, y_dim_};
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
std::uint32_t GFitnessMonitor::getXDim() const {
    return x_dim_;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
std::uint32_t GFitnessMonitor::getYDim() const {
    return y_dim_;
}

/******************************************************************************/
/**
 * Sets the number of individuals in the population that should be monitored.
 * If n_monitor_inds_ == 0, the default will be set to 3, as fitness graphs are plotted in a row,
 * and more than 3 will not give satisfactory graphical results. You may however
 * request more monitored individuals, but will likely have to postprocess the ROOT script.
 * If n_monitor_inds_ is set to a larger number than there are individuals in the population,
 * the value will be reset to that amount of individuals in informationFunction.
 *
 * @param n_monitor_inds The number of individuals in the population that should be monitored
 */
void GFitnessMonitor::setNMonitorIndividuals(const std::size_t &n_monitor_inds) {
    // A request of 0 means "use the built-in default"; any positive value is
    // honoured as-is. (Clamping to the actual population size happens later,
    // in informationFunction().)
    if(n_monitor_inds == 0) {
        n_monitor_inds_ = (DEFNMONITORINDS);
    }
    else {
        n_monitor_inds_ = n_monitor_inds;
    }
}

/******************************************************************************/
/**
 * Retrieves the number of individuals that are being monitored
 *
 * @return The number of individuals in the population being monitored
 */
std::size_t GFitnessMonitor::getNMonitorIndividuals() const {
    return n_monitor_inds_;
}

/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GFitnessMonitor::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    switch(im) {
    case Gem::Geneva::infoMode::INFOINIT: {
        // We set a marker whenever a new INFOINIT call happens. This way we
        // may "chain" algorithms and will get the entire progress information
        // for all algorithms
    } break;

    case Gem::Geneva::infoMode::INFOPROCESSING: {
        // Retrieve the list of globally- and iteration bests individuals
        auto global_bests =
            goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::template getBestGlobalIndividuals<gen::GOptimizableEntity>();
        auto iter_bests =
            goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::template getBestIterationIndividuals<gen::GOptimizableEntity>();

        // Retrieve the current iteration in the population
        std::uint32_t iteration = goa->getIteration();

        // We expect both sizes to be identical
        if(global_bests.size() != iter_bests.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFitnessMonitor::informationFunction_(): Error!" << '\n'
                << "global_bests.size() = " << global_bests.size()
                << " != iter_bests.size() = " << iter_bests.size() << '\n'
            );
        }

        //------------------------------------------------------------------------------
        // Setup of local vectors

        if(not info_init_run_) {
            // Reset the number of monitored individuals to a suitable value, if necessary.
            if(n_monitor_inds_ > global_bests.size()) {
                glogger << "In GFitnessMonitor::informationFunction_(): Warning!" << '\n'
                        << "Requested number of individuals to be monitored in iteration "
                        << iteration << " is larger" << '\n'
                        << "than the number of best individuals " << n_monitor_inds_ << " / "
                        << global_bests.size() << '\n'
                        << GWARNING;

                n_monitor_inds_ = global_bests.size();
            }

            // Set up the plotters
            for(std::size_t ind = 0; ind < n_monitor_inds_; ind++) {
                std::shared_ptr<Gem::Common::GGraph2D> global_graph(new Gem::Common::GGraph2D());
                global_graph->setXAxisLabel("Iteration");
                global_graph->setYAxisLabel("Best Fitness");
                global_graph->setPlotLabel(
                    std::string("Individual ") + Gem::Common::to_string(ind)
                );
                global_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

                global_fitness_graph_vec_.push_back(global_graph);

                std::shared_ptr<Gem::Common::GGraph2D> iteration_graph(new Gem::Common::GGraph2D());
                iteration_graph->setXAxisLabel("Iteration");
                iteration_graph->setYAxisLabel("Best Fitness");
                iteration_graph->setPlotLabel(
                    std::string("Individual ") + Gem::Common::to_string(ind)
                );
                iteration_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

                iteration_fitness_graph_vec_.push_back(iteration_graph);

                // Add the iteration graph as secondary plotter
                global_graph->registerSecondaryPlotter(iteration_graph);
            }

            // Make sure global_fitness_graph_vec_ is only initialized once
            info_init_run_ = true;
        }
        else {
            // We might have a situation where the number of best individuals changes in each
            // iteration, e.g. when dealing with pareto optimization in EA. In this case we reduce the
            // number of n_monitor_inds_ to 1, which is the only safe option. Recorded data of other
            // individuals will then be lost -- the program will warn about this.
            if(n_monitor_inds_ > global_bests.size()) {
                glogger
                    << "In GFitnessMonitor::informationFunction_(): Warning!" << '\n'
                    << "Requested number of individuals to be monitored in iteration " << iteration
                    << " is larger" << '\n'
                    << "than the number of best individuals " << n_monitor_inds_ << " / "
                    << global_bests.size() << '\n'
                    << "This seems to be a result of a varying number of best individuals."
                    << '\n'
                    << "We will now reduce the number of monitored individuals to 1 for the"
                    << '\n'
                    << "rest of the optimization run. Recorded information for other individuals"
                    << '\n'
                    << "will be deleted" << '\n'
                    << GWARNING;

                n_monitor_inds_ = 1;
                global_fitness_graph_vec_.resize(1);
                iteration_fitness_graph_vec_.resize(1);
            }
        }

        //------------------------------------------------------------------------------
        // Fill in the data for the best individuals

        std::vector<std::shared_ptr<Gem::Common::GGraph2D>>::iterator global_it;
        std::vector<std::shared_ptr<Gem::Common::GGraph2D>>::iterator iter_it;
        std::vector<std::shared_ptr<gen::GOptimizableEntity>>::iterator global_ind_it;
        std::vector<std::shared_ptr<gen::GOptimizableEntity>>::iterator iter_ind_it;

        std::size_t m_ind = 0;
        for(global_it = global_fitness_graph_vec_.begin(),
        iter_it = iteration_fitness_graph_vec_.begin(),
        global_ind_it = global_bests.begin(),
        iter_ind_it = iter_bests.begin();
            global_it != global_fitness_graph_vec_.end();
            ++global_it, ++iter_it, ++global_ind_it, ++iter_ind_it) {
            (*global_it)
                ->add(Gem::Common::narrow<double>(iteration), (*global_ind_it)->raw_fitness(0));
            (*iter_it)->add(Gem::Common::narrow<double>(iteration), (*iter_ind_it)->raw_fitness(0));
        }

        //------------------------------------------------------------------------------

    } break;

    case Gem::Geneva::infoMode::INFOEND: { /* nothing */
    } break;
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
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GFitnessMonitor::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GFitnessMonitor reference independent of this object and convert the pointer
    const GFitnessMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

    GToken token("GFitnessMonitor", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GFitnessMonitorT object, camouflaged as a GBasePluggableOM
 */
void GFitnessMonitor::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GFitnessMonitor reference independent of this object and convert the pointer
    const GFitnessMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GFitnessMonitor::clone_() const {
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

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFitnessMonitor::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFitnessMonitor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFitnessMonitor::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFitnessMonitor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFitnessMonitor::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The copy constructor
 */
GCollectiveMonitor::GCollectiveMonitor(const GCollectiveMonitor &cp)
  : oa::GBasePluggableOM(cp) {
    Gem::Common::copyCloneableSmartPointerContainer(cp.pluggable_monitors_, pluggable_monitors_);
}

/******************************************************************************/
/**
 * Aggregates the work of all registered pluggable monitors
 */
void GCollectiveMonitor::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    for(auto const &pm_ptr : pluggable_monitors_) {
        pm_ptr->informationFunction(im, goa);
    }
}

/******************************************************************************/
/**
 * Allows to register a new pluggable monitor
 */
void GCollectiveMonitor::registerPluggableOM(
    std::shared_ptr<oa::GBasePluggableOM> om_ptr
) {
    if(om_ptr) {
        pluggable_monitors_.push_back(om_ptr);
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GCollectiveMonitor::registerPluggableOM(): Error!" << '\n'
            << "Got empty pointer to pluggable optimization monitor." << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Checks if adaptors have been registered in the collective monitor
 */
bool GCollectiveMonitor::hasOptimizationMonitors() const {
    return not pluggable_monitors_.empty();
}

/******************************************************************************/
/**
 * Allows to clear all registered monitors
 */
void GCollectiveMonitor::resetPluggbleOM() {
    pluggable_monitors_.clear();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GCollectiveMonitor::name_() const {
    return std::string("GCollectiveMonitor");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GCollectiveMonitor::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GCollectiveMonitor reference independent of this object and convert the pointer
    const GCollectiveMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

    GToken token("GCollectiveMonitor", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GCollectiveMonitorT object, camouflaged as a GBasePluggableOM
 */
void GCollectiveMonitor::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GCollectiveMonitor reference independent of this object and convert the pointer
    const GCollectiveMonitor *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... and then the local data, derived from the single localMembers() declaration.
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GCollectiveMonitor::clone_() const {
    return new GCollectiveMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GCollectiveMonitor::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GCollectiveMonitor::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GCollectiveMonitor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GCollectiveMonitor::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GCollectiveMonitor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GCollectiveMonitor::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * Initialization with a file name. Note that some variables may be initialized in the class body.
 */
GAllSolutionFileLogger::GAllSolutionFileLogger(const std::string &file_name)
  : file_name_(file_name) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a file name and boundaries.
 * Note that some variables may be initialized in the class body.
 */
GAllSolutionFileLogger::GAllSolutionFileLogger(
    const std::string &file_name,
    const std::vector<double> &boundaries
)
  : file_name_(file_name)
  , boundaries_(boundaries)
  , boundaries_active_(true) { /* nothing */
}

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
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GAllSolutionFileLogger::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GAllSolutionFileLogger reference independent of this object and convert the pointer
    const GAllSolutionFileLogger *p_load =
        Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GAllSolutionFileLogger>(cp, this);

    GToken token("GAllSolutionFileLogger", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name
 */
void GAllSolutionFileLogger::setFileName(const std::string &file_name) {
    file_name_ = file_name;
}

/******************************************************************************/
/**
 * Retrieves the current file name
 */
std::string GAllSolutionFileLogger::getFileName() const {
    return file_name_;
}

/******************************************************************************/
/**
 * Sets the boundaries
 */
void GAllSolutionFileLogger::setBoundaries(const std::vector<double> &boundaries) {
    boundaries_ = boundaries;
    boundaries_active_ = true;
}

/******************************************************************************/
/**
 * Allows to retrieve the boundaries
 */
std::vector<double> GAllSolutionFileLogger::getBoundaries() const {
    return boundaries_;
}

/******************************************************************************/
/**
 * Allows to check whether boundaries are active
 */
bool GAllSolutionFileLogger::boundariesActive() const {
    return boundaries_active_;
}

/******************************************************************************/
/**
 * Allows to inactivate boundaries
 */
void GAllSolutionFileLogger::setBoundariesInactive() {
    boundaries_active_ = false;
}

/******************************************************************************/
/**
 * Allows to specify whether explanations should be printed for parameter-
 * and fitness values.
 */
void GAllSolutionFileLogger::setPrintWithNameAndType(bool with_name_and_type) {
    with_name_and_type_ = with_name_and_type;
}

/******************************************************************************/
/**
 * Allows to check whether explanations should be printed for parameter-
 * and fitness values
 */
bool GAllSolutionFileLogger::getPrintWithNameAndType() const {
    return with_name_and_type_;
}

/******************************************************************************/
/**
 * Allows to specify whether commas should be printed in-between values
 */
void GAllSolutionFileLogger::setPrintWithCommas(bool with_commas) {
    with_commas_ = with_commas;
}

/******************************************************************************/
/**
 * Allows to check whether commas should be printed in-between values
 */
bool GAllSolutionFileLogger::getPrintWithCommas() const {
    return with_commas_;
}

/******************************************************************************/
/**
 * Allows to specify whether the true (instead of the transformed) fitness should be shown
 */
void GAllSolutionFileLogger::setUseTrueFitness(bool use_raw_fitness) {
    use_raw_fitness_ = use_raw_fitness;
}

/******************************************************************************/
/**
 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
 */
bool GAllSolutionFileLogger::getUseTrueFitness() const {
    return use_raw_fitness_;
}

/******************************************************************************/
/**
 * Allows to specify whether the validity of a solution should be shown
 */
void GAllSolutionFileLogger::setShowValidity(bool show_validity) {
    show_validity_ = show_validity;
}

/******************************************************************************/
/**
 * Allows to check whether the validity of a solution will be shown
 */
bool GAllSolutionFileLogger::getShowValidity() const {
    return show_validity_;
}

/******************************************************************************/
/**
 * Allows to specifiy whether the initial population (prior to any
 * optimization work) should be printed.
 */
void GAllSolutionFileLogger::setPrintInitial(bool print_initial) {
    print_initial_ = print_initial;
}

/******************************************************************************/
/**
 * Allows to check whether the initial population (prior to any
 * optimization work) should be printed.
 */
bool GAllSolutionFileLogger::getPrintInitial() const {
    return print_initial_;
}

/******************************************************************************/
/**
* Allows to specifiy whether a comment line should be inserted
* between iterations
*/
void GAllSolutionFileLogger::setShowIterationBoundaries(bool show_iteration_boundaries) {
    show_iteration_boundaries_ = show_iteration_boundaries;
}

/******************************************************************************/
/**
 * Allows to check whether a comment line should be inserted
 * between iterations
 */
bool GAllSolutionFileLogger::getShowIterationBoundaries() const {
    return show_iteration_boundaries_;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GAllSolutionFileLogger::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    switch(im) {
    case Gem::Geneva::infoMode::INFOINIT: {
        // If the file pointed to by file_name_ already exists, make a back-up
        if(std::filesystem::exists(file_name_)) {
            std::string new_file_name =
                file_name_ + ".bak_" +
                Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

            glogger << "In GAllSolutionFileLogger::informationFunction_(): Warning!" << '\n'
                    << "Attempt to output information to file " << file_name_ << '\n'
                    << "which already exists. We will rename the old file to" << '\n'
                    << new_file_name << '\n'
                    << GWARNING;

            std::filesystem::rename(file_name_, new_file_name);
        }

        if(print_initial_) {
            this->printPopulation("Initial population", goa);
        }
    } break;

    case Gem::Geneva::infoMode::INFOPROCESSING: {
        this->printPopulation(
            "At end of iteration " + Gem::Common::to_string(goa->getIteration()),
            goa
        );
    } break;

    case Gem::Geneva::infoMode::INFOEND:
        // nothing
        break;
    };
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GAllSolutionFileLoggerT object, camouflaged as a GBasePluggableOM
 */
void GAllSolutionFileLogger::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GAllSolutionFileLogger reference independent of this object and convert the pointer
    const GAllSolutionFileLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GAllSolutionFileLogger::clone_() const {
    return new GAllSolutionFileLogger(*this);
}

/******************************************************************************/
/**
 * Does the actual printing
 */
void GAllSolutionFileLogger::printPopulation(
    const std::string &iteration_description,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    // Open the external file
    std::ofstream data(file_name_, std::ofstream::app); // NOLINT(cppcoreguidelines-init-variables)

    if(show_iteration_boundaries_) {
        data << "#" << '\n'
             << "# -----------------------------------------------------------------------------"
             << '\n'
             << "# " << iteration_description << ":" << '\n'
             << "#" << '\n';
    }

    // Loop over all individuals of the algorithm.
    for(std::size_t pos = 0; pos < goa->size(); pos++) {
        std::shared_ptr<gen::GOptimizableEntity> ind = goa->template individual_cast<gen::GOptimizableEntity>(pos);

        // Note that isGoodEnough may throw if loop acts on a "dirty" individual
        if(not boundaries_active_ || ind->isGoodEnough(boundaries_)) {
            // Append the data to the external file
            if(0 == pos &&
               goa->inFirstIteration()) { // Only output name and type in the very first line (if at all)
                data
                    << ind->toCSV(with_name_and_type_, with_commas_, use_raw_fitness_, show_validity_);
            }
            else {
                data << ind->toCSV(
                    false /* with_name_and_type */,
                    with_commas_,
                    use_raw_fitness_,
                    show_validity_
                );
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

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GAllSolutionFileLogger::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GAllSolutionFileLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GAllSolutionFileLogger::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GAllSolutionFileLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GAllSolutionFileLogger::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * Initialization with a file name. Note that some variables may be initialized
 * in the class body.
 */
GIterationResultsFileLogger::GIterationResultsFileLogger(const std::string &file_name)
  : file_name_(file_name) { /* nothing */
}

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
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GIterationResultsFileLogger::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GIterationResultsFileLogger
    // reference independent of this object and convert the pointer
    const GIterationResultsFileLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

    GToken token("GIterationResultsFileLogger", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name
 */
void GIterationResultsFileLogger::setFileName(const std::string &file_name) {
    file_name_ = file_name;
}

/******************************************************************************/
/**
 * Retrieves the current file name
 */
std::string GIterationResultsFileLogger::getFileName() const {
    return file_name_;
}

/******************************************************************************/
/**
 * Allows to specify whether commas should be printed in-between values
 */
void GIterationResultsFileLogger::setPrintWithCommas(bool with_commas) {
    with_commas_ = with_commas;
}

/******************************************************************************/
/**
 * Allows to check whether commas should be printed in-between values
 */
bool GIterationResultsFileLogger::getPrintWithCommas() const {
    return with_commas_;
}

/******************************************************************************/
/**
 * Allows to specify whether the true (instead of the transformed) fitness should be shown
 */
void GIterationResultsFileLogger::setUseTrueFitness(bool use_raw_fitness) {
    use_raw_fitness_ = use_raw_fitness;
}

/******************************************************************************/
/**
 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
 */
bool GIterationResultsFileLogger::getUseTrueFitness() const {
    return use_raw_fitness_;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GIterationResultsFileLogger::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    switch(im) {
    case Gem::Geneva::infoMode::INFOINIT: {
        // If the file pointed to by file_name_ already exists, make a back-up
        if(std::filesystem::exists(file_name_)) {
            std::string new_file_name =
                file_name_ + ".bak_" +
                Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

            glogger << "In GIterationResultsFileLogger::informationFunction_(): Warning!" << '\n'
                    << "Attempt to output information to file " << file_name_ << '\n'
                    << "which already exists. We will rename the old file to" << '\n'
                    << new_file_name << '\n'
                    << GWARNING;

            std::filesystem::rename(file_name_, new_file_name);
        }
    } break;

    case Gem::Geneva::infoMode::INFOPROCESSING: {
        // Open the external file
        std::ofstream data(
            file_name_.c_str(),
            std::ofstream::app
        ); // NOLINT(cppcoreguidelines-init-variables)
        std::vector<double> fitness_cnt;

        // Write a flat line of every individual's fitness values, a comma (or space) after each value
        // except the LAST, via an explicit emitted-vs-total counter. The previous PRODUCT test
        // (n_fitness_criteria*n_individuals > (i+1)*(pos+1)) was obscure and fragile: it happened to be
        // correct only because the product reaches the total exactly at the final cell.
        const std::size_t n_individuals = goa->size();
        const std::size_t n_fitness_criteria = goa->at(0)->individual().getNStoredResults();
        const std::size_t total = n_fitness_criteria * n_individuals;
        std::size_t emitted = 0;
        for(std::size_t pos = 0; pos < n_individuals; pos++) {
            fitness_cnt = goa->at(pos)->individual().raw_fitness_vec();
            for(std::size_t i = 0; i < n_fitness_criteria; i++) {
                data << fitness_cnt.at(i) << ((with_commas_ && (++emitted < total)) ? ", " : " ");
            }
        }
        data << '\n';

        // Close the external file
        data.close();
    } break;

    case Gem::Geneva::infoMode::INFOEND:
        // nothing
        break;
    };
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GIterationResultsFileLoggerT object, camouflaged as a GBasePluggableOM
 */
void GIterationResultsFileLogger::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GIterationResultsFileLogger
    // reference independent of this object and convert the pointer
    const GIterationResultsFileLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GIterationResultsFileLogger::clone_() const {
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

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GIterationResultsFileLogger::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GIterationResultsFileLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GIterationResultsFileLogger::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GIterationResultsFileLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GIterationResultsFileLogger::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with a file name. Note that some variables may be
 * initialized in the class body.
 */
GNAdpationsLogger::GNAdpationsLogger(const std::string &file_name)
  : file_name_(file_name)
  , canvas_dimensions_(std::tuple<std::uint32_t, std::uint32_t>(1200, 1600))
  , gpd_("Number of adaptions per iteration", 1, 2) { /* nothing */
}

/******************************************************************************/
/**
 * The copy constructor
 */
GNAdpationsLogger::GNAdpationsLogger(const GNAdpationsLogger &cp)
  : oa::GBasePluggableOM(cp)
  , file_name_(cp.file_name_)
  , canvas_dimensions_(cp.canvas_dimensions_)
  , gpd_(cp.gpd_)
  , monitor_best_only_(cp.monitor_best_only_)
  , add_print_command_(cp.add_print_command_)
  , max_iteration_(cp.max_iteration_)
  , n_iterations_recorded_(cp.n_iterations_recorded_)
  , n_adaptions_store_(cp.n_adaptions_store_) {
    Gem::Common::copyCloneableSmartPointer(cp.n_adaptions_hist2_d_oa_, n_adaptions_hist2_d_oa_);
    Gem::Common::copyCloneableSmartPointer(cp.n_adaptions_graph2_d_oa_, n_adaptions_graph2_d_oa_);
    Gem::Common::copyCloneableSmartPointer(cp.fitness_graph2_d_oa_, fitness_graph2_d_oa_);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GNAdpationsLogger::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GNAdpationsLogger reference independent of this object and convert the pointer
    const GNAdpationsLogger *p_load =
        Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GNAdpationsLogger>(cp, this);

    GToken token("GNAdpationsLogger", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name
 */
void GNAdpationsLogger::setFileName(const std::string &file_name) {
    file_name_ = file_name;
}

/******************************************************************************/
/**
 * Retrieves the current file name
 */
std::string GNAdpationsLogger::getFileName() const {
    return file_name_;
}

/******************************************************************************/
/**
 * Allows to specify whether only the best individuals should be monitored.
 */
void GNAdpationsLogger::setMonitorBestOnly(bool monitor_best_only) {
    monitor_best_only_ = monitor_best_only;
}

/******************************************************************************/
/**
 * Allows to check whether only the best individuals should be monitored.
 */
bool GNAdpationsLogger::getMonitorBestOnly() const {
    return monitor_best_only_;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions
 */
void GNAdpationsLogger::setCanvasDimensions(
    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions
) {
    canvas_dimensions_ = canvas_dimensions;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions using separate x and y values
 */
void GNAdpationsLogger::setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
    canvas_dimensions_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
}

/******************************************************************************/
/**
 * Gives access to the canvas dimensions
 */
std::tuple<std::uint32_t, std::uint32_t> GNAdpationsLogger::getCanvasDimensions() const {
    return canvas_dimensions_;
}

/******************************************************************************/
/**
 * Allows to add a "Print" command to the end of the script so that picture files are created
 */
void GNAdpationsLogger::setAddPrintCommand(bool add_print_command) {
    add_print_command_ = add_print_command;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the add_print_command_ variable
 */
bool GNAdpationsLogger::getAddPrintCommand() const {
    return add_print_command_;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GNAdpationsLogger::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    using namespace Gem::Common;

    switch(im) {
    case Gem::Geneva::infoMode::INFOINIT: {
        // If the file pointed to by file_name_ already exists, make a back-up
        if(std::filesystem::exists(file_name_)) {
            std::string new_file_name =
                file_name_ + ".bak_" +
                Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

            glogger << "In GNAdpationsLogger::informationFunction_(): Error!" << '\n'
                    << "Attempt to output information to file " << file_name_ << '\n'
                    << "which already exists. We will rename the old file to" << '\n'
                    << new_file_name << '\n'
                    << GWARNING;

            std::filesystem::rename(file_name_, new_file_name);
        }

        // Make sure the progress plotter has the desired size
        gpd_.setCanvasDimensions(canvas_dimensions_);

        // Set up a graph to monitor the best fitness found
        fitness_graph2_d_oa_ = std::make_shared<Gem::Common::GGraph2D>();
        fitness_graph2_d_oa_->setXAxisLabel("Iteration");
        fitness_graph2_d_oa_->setYAxisLabel("Fitness");
        fitness_graph2_d_oa_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
    } break;

    case Gem::Geneva::infoMode::INFOPROCESSING: {
        std::uint32_t iteration = goa->getIteration();

        // Record the current fitness
        std::shared_ptr<gen::GOptimizableEntity> p =
            goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::template getBestGlobalIndividual<gen::GOptimizableEntity>();
        (*fitness_graph2_d_oa_) & std::tuple<double, double>(static_cast<double>(iteration), p->raw_fitness(0));

        // Update the largest known iteration and the number of recorded iterations
        max_iteration_ = iteration;
        n_iterations_recorded_++;

        // Do the actual logging
        if(monitor_best_only_) {
            std::shared_ptr<gen::GOptimizableEntity> best =
                goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::template getBestGlobalIndividual<gen::GOptimizableEntity>();
            n_adaptions_store_.emplace_back(static_cast<double>(iteration), static_cast<double>(best->getNAdaptions()));
        }
        else { // Monitor all individuals
            // Loop over all individuals of the algorithm.
            for(std::size_t pos = 0; pos < goa->size(); pos++) {
                std::shared_ptr<gen::GOptimizableEntity> ind =
                    goa->template individual_cast<gen::GOptimizableEntity>(pos);
                n_adaptions_store_.emplace_back(static_cast<double>(iteration), static_cast<double>(ind->getNAdaptions()));
            }
        }
    } break;

    case Gem::Geneva::infoMode::INFOEND: {
        if(monitor_best_only_) {
            // Create the graph object
            n_adaptions_graph2_d_oa_ =
                std::make_shared<Gem::Common::GGraph2D>();
            n_adaptions_graph2_d_oa_->setXAxisLabel("Iteration");
            n_adaptions_graph2_d_oa_->setYAxisLabel("Number of parameter adaptions");
            n_adaptions_graph2_d_oa_->setPlotMode(Gem::Common::graphPlotMode::CURVE);

            // Fill the object with data
            for(const auto &n_adaptions : n_adaptions_store_) {
                (*n_adaptions_graph2_d_oa_) & n_adaptions;
            }

            // Add the histogram to the plot designer
            gpd_.registerPlotter(n_adaptions_graph2_d_oa_);
        }
        else { // All individuals are monitored
            // Within n_adaptions_store_, find the largest number of adaptions performed
            std::size_t max_n_adaptions = 0;
            for(const auto &n_adaptions : n_adaptions_store_) {
                if(std::get<1>(n_adaptions) > max_n_adaptions) {
                    max_n_adaptions = Gem::Common::narrow<std::size_t>(std::get<1>(n_adaptions));
                }
            }

            // Create the histogram object
            n_adaptions_hist2_d_oa_ = std::make_shared<GHistogram2D>(
                n_iterations_recorded_,
                max_n_adaptions + 1,
                0.,
                static_cast<double>(max_iteration_),
                0.,
                static_cast<double>(max_n_adaptions)
            );

            n_adaptions_hist2_d_oa_->setXAxisLabel("Iteration");
            n_adaptions_hist2_d_oa_->setYAxisLabel("Number of parameter adaptions");
            n_adaptions_hist2_d_oa_->setDrawingArguments("BOX");

            // Fill the object with data
            for(const auto &n_adaptions : n_adaptions_store_) {
                (*n_adaptions_hist2_d_oa_) & n_adaptions;
            }

            // Add the histogram to the plot designer
            gpd_.registerPlotter(n_adaptions_hist2_d_oa_);
        }

        // Add the fitness monitor
        gpd_.registerPlotter(fitness_graph2_d_oa_);

        // Inform the plot designer whether it should print png files
        gpd_.setAddPrintCommand(add_print_command_);

        // Write out the result. Note that we add
        gpd_.writeToFile(file_name_);

        // Remove all plotters
        gpd_.resetPlotters();
        n_adaptions_hist2_d_oa_.reset();
        n_adaptions_graph2_d_oa_.reset();
    } break;
    };
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GNAdpationsLoggerT object, camouflaged as a GBasePluggableOM
 */
void GNAdpationsLogger::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GNAdpationsLogger reference independent of this object and convert the pointer
    const GNAdpationsLogger *p_load =
        Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GNAdpationsLogger>(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GNAdpationsLogger::clone_() const {
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

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GNAdpationsLogger::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GNAdpationsLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GNAdpationsLogger::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GNAdpationsLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GNAdpationsLogger::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
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
    const std::string &file_name_pth,
    const std::string &file_name_pth2,
    const std::string &file_name_txt,
    std::size_t n_bins_x,
    std::size_t n_bins_y
)
  : file_name_pth_(file_name_pth)
  , canvas_dimensions_pth_(std::tuple<std::uint32_t, std::uint32_t>(1600, 1200))
  , gpd_pth_("Timings for the processing steps of individuals", 2, 2)
  , file_name_pth2_(file_name_pth2)
  , canvas_dimensions_pth2_(std::tuple<std::uint32_t, std::uint32_t>(1600, 1200))
  , gpd_pth2_("Timings for the processing steps of individuals vs. iteration", 2, 2)
  , file_name_txt_(file_name_txt)
  , n_bins_x_(n_bins_x)
  , n_bins_y_(n_bins_y) { /* nothing */
}

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
 * @param cp A constant reference to another GBasePluggableOM object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GProcessingTimesLogger::compare_(
    const oa::GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GProcessingTimesLogger reference independent of this object and convert the pointer
    const GProcessingTimesLogger *p_load =
        Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GProcessingTimesLogger>(cp, this);

    GToken token("GProcessingTimesLogger", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

    // ... and then all local data, derived from the single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Sets the file name for the processing times histogram
 */
void GProcessingTimesLogger::setFileName_pth(const std::string &file_name) {
    file_name_pth_ = file_name;
}

/******************************************************************************/
/**
 * Retrieves the current file name for the processing times histogram
 */
std::string GProcessingTimesLogger::getFileName_pth() const {
    return file_name_pth_;
}

/******************************************************************************/
/**
 * Sets the file name for the processing times histograms (2D)
 */
void GProcessingTimesLogger::setFileName_pth2(const std::string &file_name) {
    file_name_pth2_ = file_name;
}

/******************************************************************************/
/**
 * Retrieves the current file name for the processing times histograms (2D)
 */
std::string GProcessingTimesLogger::getFileName_pth2() const {
    return file_name_pth2_;
}

/******************************************************************************/
/**
 * Sets the file name for the text output
 */
void GProcessingTimesLogger::setFileName_txt(const std::string &file_name) {
    file_name_txt_ = file_name;
}

/******************************************************************************/
/**
 * Retrieves the current file name for the text output
 */
std::string GProcessingTimesLogger::getFileName_txt() const {
    return file_name_txt_;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions for the processing times histograms
 */
void GProcessingTimesLogger::setCanvasDimensions_pth(
    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions
) {
    canvas_dimensions_pth_ = canvas_dimensions;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions using separate x and y values for the
 * processing times histograms
 */
void GProcessingTimesLogger::setCanvasDimensions_pth(std::uint32_t x, std::uint32_t y) {
    canvas_dimensions_pth_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
}

/******************************************************************************/
/**
 * Gives access to the canvas dimensions of the processing times histograms
 */
std::tuple<std::uint32_t, std::uint32_t> GProcessingTimesLogger::getCanvasDimensions_pth() const {
    return canvas_dimensions_pth_;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions for the processing times histograms (2D)
 */
void GProcessingTimesLogger::setCanvasDimensions_pth2(
    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions
) {
    canvas_dimensions_pth2_ = canvas_dimensions;
}

/******************************************************************************/
/**
 * Allows to set the canvas dimensions using separate x and y values for the
 * processing times histograms (2D)
 */
void GProcessingTimesLogger::setCanvasDimensions_pth2(std::uint32_t x, std::uint32_t y) {
    canvas_dimensions_pth2_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
}

/******************************************************************************/
/**
 * Gives access to the canvas dimensions of the processing times histograms (2D)
 */
std::tuple<std::uint32_t, std::uint32_t> GProcessingTimesLogger::getCanvasDimensions_pth2() const {
    return canvas_dimensions_pth2_;
}

/******************************************************************************/
/**
 * Sets the number of bins for the processing times histograms in y-direction
 */
void GProcessingTimesLogger::setNBinsX(std::size_t n_bins_x) {
    if(n_bins_x > 0) {
        n_bins_x_ = n_bins_x;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GProcessingTimesLogger::setNBinsX(): Error!" << '\n'
            << "n_bins_x is set to 0" << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Retrieves the current number of bins for the processing times
 * histograms in x-direction
 */
std::size_t GProcessingTimesLogger::getNBinsX() const {
    return n_bins_x_;
}

/******************************************************************************/
/**
 * Sets the number of bins for the processing times histograms in y-direction
 */
void GProcessingTimesLogger::setNBinsY(std::size_t n_bins_y) {
    if(n_bins_y > 0) {
        n_bins_y_ = n_bins_y;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GProcessingTimesLogger::setNBinsY(): Error!" << '\n'
            << "n_bins_y is set to 0" << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Retrieves the current number of bins for the processing times
 * histograms in y-direction
 */
std::size_t GProcessingTimesLogger::getNBinsY() const {
    return n_bins_y_;
}

/******************************************************************************/
/**
 * Allows to emit information in different stages of the information cycle
 * (initialization, during each cycle and during finalization)
 */
void GProcessingTimesLogger::informationFunction_(
    infoMode im,
    oa::GOptimizationAlgorithmBase const *const goa
) {
    switch(im) {
    case Gem::Geneva::infoMode::INFOINIT: {
        //---------------------------------------------------------------
        // Histograms

        // If the file pointed to by file_name_pth_ already exists, make a back-up
        if(std::filesystem::exists(file_name_pth_)) {
            std::string new_file_name =
                file_name_pth_ + ".bak_" +
                Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

            glogger << "In GProcessingTimesLogger::informationFunction_(): Warning!" << '\n'
                    << "Attempt to output information to file " << file_name_pth_ << '\n'
                    << "which already exists. We will rename the old file to" << '\n'
                    << new_file_name << '\n'
                    << GWARNING;

            std::filesystem::rename(file_name_pth_, new_file_name);
        }

        // Make sure the processing times plotter has the desired size
        gpd_pth_.setCanvasDimensions(canvas_dimensions_pth_);

        pre_processing_times_hist_ = std::make_shared<Gem::Common::GHistogram1D>(n_bins_x_);
        pre_processing_times_hist_->setXAxisLabel("Pre-processing time [s]");
        pre_processing_times_hist_->setYAxisLabel("Number of Entries");
        pre_processing_times_hist_->setDrawingArguments("hist");

        gpd_pth_.registerPlotter(pre_processing_times_hist_);

        processing_times_hist_ = std::make_shared<Gem::Common::GHistogram1D>(n_bins_x_);
        processing_times_hist_->setXAxisLabel("Main processing time [s]");
        processing_times_hist_->setYAxisLabel("Number of Entries");
        processing_times_hist_->setDrawingArguments("hist");

        gpd_pth_.registerPlotter(processing_times_hist_);

        post_processing_times_hist_ = std::make_shared<Gem::Common::GHistogram1D>(n_bins_x_);
        post_processing_times_hist_->setXAxisLabel("Post-processing time [s]");
        post_processing_times_hist_->setYAxisLabel("Number of Entries");
        post_processing_times_hist_->setDrawingArguments("hist");

        gpd_pth_.registerPlotter(post_processing_times_hist_);

        all_processing_times_hist_ = std::make_shared<Gem::Common::GHistogram1D>(n_bins_x_);
        all_processing_times_hist_->setXAxisLabel("Overall processing time for all steps [s]");
        all_processing_times_hist_->setYAxisLabel("Number of Entries");
        all_processing_times_hist_->setDrawingArguments("hist");

        gpd_pth_.registerPlotter(all_processing_times_hist_);

        //---------------------------------------------------------------
        // 2D Histograms

        // If the file pointed to by file_name_pth2_ already exists, make a back-up
        if(std::filesystem::exists(file_name_pth2_)) {
            std::string new_file_name =
                file_name_pth2_ + ".bak_" +
                Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

            glogger << "In GProcessingTimesLogger::informationFunction_(): Warning!" << '\n'
                    << "Attempt to output information to file " << file_name_pth2_ << '\n'
                    << "which already exists. We will rename the old file to" << '\n'
                    << new_file_name << '\n'
                    << GWARNING;

            std::filesystem::rename(file_name_pth2_, new_file_name);
        }

        // Make sure the processing times has the desired size
        gpd_pth2_.setCanvasDimensions(canvas_dimensions_pth2_);

        pre_processing_times_hist2_d_ =
            std::make_shared<Gem::Common::GHistogram2D>(n_bins_x_, n_bins_y_);
        pre_processing_times_hist2_d_->setXAxisLabel("Iteration");
        pre_processing_times_hist2_d_->setYAxisLabel("Pre-processing time [s]");
        pre_processing_times_hist2_d_->setZAxisLabel("Number of Entries");
        pre_processing_times_hist2_d_->setDrawingArguments("box");

        gpd_pth2_.registerPlotter(pre_processing_times_hist2_d_);

        processing_times_hist2_d_ = std::make_shared<Gem::Common::GHistogram2D>(n_bins_x_, n_bins_y_);
        processing_times_hist2_d_->setXAxisLabel("Iteration");
        processing_times_hist2_d_->setYAxisLabel("Main processing time [s]");
        processing_times_hist2_d_->setZAxisLabel("Number of Entries");
        processing_times_hist2_d_->setDrawingArguments("box");

        gpd_pth2_.registerPlotter(processing_times_hist2_d_);

        post_processing_times_hist2_d_ =
            std::make_shared<Gem::Common::GHistogram2D>(n_bins_x_, n_bins_y_);
        post_processing_times_hist2_d_->setXAxisLabel("Iteration");
        post_processing_times_hist2_d_->setYAxisLabel("Post-processing time [s]");
        post_processing_times_hist2_d_->setZAxisLabel("Number of Entries");
        post_processing_times_hist2_d_->setDrawingArguments("box");

        gpd_pth2_.registerPlotter(post_processing_times_hist2_d_);

        all_processing_times_hist2_d_ =
            std::make_shared<Gem::Common::GHistogram2D>(n_bins_x_, n_bins_y_);
        all_processing_times_hist2_d_->setXAxisLabel("Iteration");
        all_processing_times_hist2_d_->setYAxisLabel("Overall processing time [s]");
        all_processing_times_hist2_d_->setZAxisLabel("Number of Entries");
        all_processing_times_hist2_d_->setDrawingArguments("box");

        gpd_pth2_.registerPlotter(all_processing_times_hist2_d_);

        //---------------------------------------------------------------
        // Make sure the output file is empty (rename, if it exists)

        // If the file pointed to by file_name_txt_ already exists, make a back-up
        if(std::filesystem::exists(file_name_txt_)) {
            std::string new_file_name =
                file_name_txt_ + ".bak_" +
                Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

            glogger << "In GProcessingTimesLogger::informationFunction_(): Warning!" << '\n'
                    << "Attempt to output information to file " << file_name_pth2_ << '\n'
                    << "which already exists. We will rename the old file to" << '\n'
                    << new_file_name << '\n'
                    << GWARNING;

            std::filesystem::rename(file_name_txt_, new_file_name);
        }

        //---------------------------------------------------------------

    } break;

    case Gem::Geneva::infoMode::INFOPROCESSING: {
        // Open the external text-file
        std::ofstream data_txt(
            file_name_txt_,
            std::ofstream::app
        ); // NOLINT(cppcoreguidelines-init-variables)

        // Retrieve the current iteration in the population
        auto iteration = Gem::Common::narrow<double>(goa->getIteration());

        // Loop over all individuals of the algorithm.
        for(std::size_t pos = 0; pos < goa->size(); pos++) {
            // Get access to each individual in sequence
            std::shared_ptr<gen::GOptimizableEntity> ind = goa->template individual_cast<gen::GOptimizableEntity>(pos);

            // Retrieve the processing timings
            std::tuple<double, double, double> processing_times = ind->getProcessingTimes();

            double pre_processing_time = std::get<0>(processing_times);
            double main_processing_time = std::get<1>(processing_times);
            double post_processing_time = std::get<2>(processing_times);
            double all_processing_time =
                pre_processing_time + main_processing_time + post_processing_time;

            // Fill the timings into the histograms
            pre_processing_times_hist_->add(pre_processing_time);   // PREPROCESSING
            processing_times_hist_->add(main_processing_time);      // PROCESSING
            post_processing_times_hist_->add(post_processing_time); // POSTPROCESSING
            all_processing_times_hist_->add(all_processing_time);   // OVERALL PROCESSING TIME

            // Fill the timings into the 2D histograms ...
            pre_processing_times_hist2_d_->add(
                Gem::Common::narrow<double>(iteration),
                Gem::Common::narrow<double>(pre_processing_time)
            ); // PREPROCESSING
            processing_times_hist2_d_->add(
                Gem::Common::narrow<double>(iteration),
                Gem::Common::narrow<double>(main_processing_time)
            ); // PROCESSING
            post_processing_times_hist2_d_->add(
                Gem::Common::narrow<double>(iteration),
                Gem::Common::narrow<double>(post_processing_time)
            ); // POSTPROCESSING
            all_processing_times_hist2_d_->add(
                Gem::Common::narrow<double>(iteration),
                Gem::Common::narrow<double>(all_processing_time)
            ); // OVERALL PROCESSING TIME

            data_txt << Gem::Common::narrow<std::uint32_t>(iteration) << ", " << std::showpoint
                     << pre_processing_time << ", " << main_processing_time << ", "
                     << post_processing_time << '\n';
        }

        // Close the external text-file
        data_txt.close();
    } break;

    case Gem::Geneva::infoMode::INFOEND: {
        // Write out the results
        gpd_pth_.writeToFile(file_name_pth_);
        gpd_pth2_.writeToFile(file_name_pth2_);

        // Remove all plotters
        gpd_pth_.resetPlotters();
        gpd_pth2_.resetPlotters();

        pre_processing_times_hist_.reset();
        processing_times_hist_.reset();
        post_processing_times_hist_.reset();
        all_processing_times_hist_.reset();

        pre_processing_times_hist2_d_.reset();
        processing_times_hist2_d_.reset();
        post_processing_times_hist2_d_.reset();
        all_processing_times_hist2_d_.reset();
    } break;
    };
}

/************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GProcessingTimesLoggerT object, camouflaged as a GBasePluggableOM
 */
void GProcessingTimesLogger::load_(const oa::GBasePluggableOM *cp) {
    // Check that we are dealing with a GProcessingTimesLogger reference independent of this object and convert the pointer
    const GProcessingTimesLogger *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load the parent classes' data ...
    oa::GBasePluggableOM::load_(cp);

    // ... and then all local data, derived from the single localMembers() declaration.
    // (This also fixes a latent bug: the previous hand-written load_() forgot to
    // load n_bins_y_, which was serialized and compared but never copied on load.)
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/************************************************************************/
/**
 * Creates a deep clone of this object
 */
oa::GBasePluggableOM *GProcessingTimesLogger::clone_() const {
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

    bool result = false;

    // Call the parent classes' functions
    if(oa::GBasePluggableOM::modify_GUnitTests_()) {
        result = true;
    }

    // no local data -- nothing to change

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GProcessingTimesLogger::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GProcessingTimesLogger::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GProcessingTimesLogger::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GProcessingTimesLogger::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GProcessingTimesLogger::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
