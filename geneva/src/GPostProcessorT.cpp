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

#include "geneva/GPostProcessorT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/concurrency/GThreadPool.hpp"

#include <atomic>
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include <memory>
#include <string>

// Export of GEvolutionaryAlgorithmPostOptimizer
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithmPostOptimizer) // NOLINT

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Initialization with the execution mode and configuration files
 *
 * @param execution_mode The desired execution mode; only SERIAL and MULTITHREADED are accepted (BROKER throws)
 * @param oa_config_file The path to the JSON configuration file for the inner evolutionary algorithm
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(
    execMode execution_mode,
    const std::string &oa_config_file
)
  : oa_config_file_(oa_config_file)
  , execution_mode_(
        (execution_mode == execMode::SERIAL || execution_mode == execMode::MULTITHREADED)
            ? execution_mode
            : execMode::SERIAL
    ) {
    switch(execution_mode) {
    case execMode::SERIAL:
    case execMode::MULTITHREADED:
        /* nothing */
        break;

    case execMode::BROKER: {
        // Consistent with setExecMode(), which also throws for BROKER. The
        // constructor previously only warned and silently fell back to SERIAL.
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(execMode): Error!" << '\n'
            << "Got invalid execution mode " << execution_mode << '\n'
        );
    } break;
    }
}

/******************************************************************************/
/**
 * @brief Returns the name of this class
 *
 * @return The string "GEvolutionaryAlgorithmPostOptimizer", identifying this class
 */
std::string GEvolutionaryAlgorithmPostOptimizer::name_() const {
    return std::string("GEvolutionaryAlgorithmPostOptimizer");
}

/******************************************************************************/
/**
 * @brief Checks for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object (a GEvolutionaryAlgorithmPostOptimizer, passed via its base type) to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GEvolutionaryAlgorithmPostOptimizer::compare_(
    const Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;
using namespace Gem::Common::Concurrency;

    // Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
    const GEvolutionaryAlgorithmPostOptimizer *p_load = Gem::Common::g_convert_and_compare<
        Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity>,
        GEvolutionaryAlgorithmPostOptimizer>(cp, this);

    GToken token("GEvolutionaryAlgorithmPostOptimizer", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPostProcessorBaseT<gen::GOptimizableEntity>>(*this, *p_load, token);

    // ... and then our local data
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Allows to set the execution mode for this post-processor (serial vs. multi-threaded)
 *
 * @param execution_mode The desired execution mode; only SERIAL and MULTITHREADED are accepted (BROKER throws)
 */
void GEvolutionaryAlgorithmPostOptimizer::setExecMode(execMode execution_mode) {
    switch(execution_mode) {
    case execMode::SERIAL:
    case execMode::MULTITHREADED: {
        execution_mode_ = execution_mode;
    } break;

    case execMode::BROKER: {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GEvolutionaryAlgorithmPostOptimizer::setExecMode(): Error!" << '\n'
            << "Got invalid execution mode " << execution_mode << '\n'
        );
    } break;
    }
}

/******************************************************************************/
/**
 * @brief Allows to retrieve the current execution mode
 *
 * @return The currently configured execution mode (SERIAL or MULTITHREADED)
 */
execMode GEvolutionaryAlgorithmPostOptimizer::getExecMode() const {
    return execution_mode_;
}

/******************************************************************************/
/**
 * @brief Allows to specify the name of a configuration file for the inner optimization algorithm
 *
 * @param oa_config_file The path to the JSON configuration file for the inner evolutionary algorithm
 */
void GEvolutionaryAlgorithmPostOptimizer::setOAConfigFile(const std::string &oa_config_file) {
    oa_config_file_ = oa_config_file;
}

/******************************************************************************/
/**
 * @brief Allows to retrieve the configuration file for the inner optimization algorithm
 *
 * @return The path to the JSON configuration file for the inner evolutionary algorithm
 */
std::string GEvolutionaryAlgorithmPostOptimizer::getOAConfigFile() const {
    return oa_config_file_;
}

/******************************************************************************/
/**
 * @brief Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
 *
 * @param cp A pointer to another object (a GEvolutionaryAlgorithmPostOptimizer, passed via its base type) whose data is loaded into this one
 */
void GEvolutionaryAlgorithmPostOptimizer::load_(
    const Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity> *cp
) {
    // Check that we are dealing with a GEvolutionaryAlgorithmPostOptimizer reference independent of this object and convert the pointer
    const GEvolutionaryAlgorithmPostOptimizer *p_load = Gem::Common::g_convert_and_compare<
        Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity>,
        GEvolutionaryAlgorithmPostOptimizer>(cp, this);

    // Load our parent class'es data ...
    GPostProcessorBaseT<gen::GOptimizableEntity>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A pointer to a freshly allocated deep copy of this object (caller takes ownership)
 */
Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity> *
GEvolutionaryAlgorithmPostOptimizer::clone_() const {
    return new GEvolutionaryAlgorithmPostOptimizer(*this);
}

/******************************************************************************/
/**
 * @brief The actual post-processing takes place here (no further checks)
 *
 * Runs an inner evolutionary algorithm to locally refine the given individual and writes the
 * optimized parameter data back into it.
 *
 * @param p The individual to be post-processed; on return it carries the refined parameter data
 * @return Always true, indicating that post-processing was performed
 */
bool GEvolutionaryAlgorithmPostOptimizer::raw_processing_(gen::GOptimizableEntity &p) {
    // Make sure p is processed
    if(not p.is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GEvolutionaryAlgorithmPostOptimizer::raw_processing_: Error!" << '\n'
            << "Provided base_type has dirty flag set." << '\n'
        );
    }

    if(execution_mode_ == execMode::BROKER) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GEvolutionaryAlgorithmPostOptimizer::raw_processing_: Error!" << '\n'
            << "Got invalid execution mode " << execution_mode_ << '\n'
        );
    }

    // Clone the individual for post-processing
    std::shared_ptr<gen::GOptimizableEntity> p_unopt_ptr = p.template clone<gen::GOptimizableEntity>();

    // Make sure the post-optimization does not trigger post-optimization recursively: the sub-EA's
    // population must carry NO post-processor (the optimization algorithm decides post-processing
    // eligibility at setup from the post-processor + its own mnemonic; with no post-processor there is
    // nothing to recurse into). The veto flag alone is not enough -- the sub-EA's setIndividualPersonalities
    // recomputes eligibility and would clear it.
    p_unopt_ptr->clearPostProcessor();
    p_unopt_ptr->vetoPostProcessing(true);

    // Retrieve an evolutionary algorithm
    oa::GEvolutionaryAlgorithmFactory ea_factory(oa_config_file_);
    auto ea_ptr = ea_factory.get<oa::GEvolutionaryAlgorithm>();

    // Post-processing refines each individual locally, inside the individual's own process() -- which
    // itself runs on the one work consumer (a thread-pool worker, or a remote client) when the outer
    // optimization is evaluated. So the inner refinement EA must NOT submit to that work consumer: that
    // would re-enter the pool it is running on (a nested-pool thread explosion / deadlock) or, on a
    // remote client, have no consumer at all. It therefore evaluates its population INLINE, in the
    // calling thread. The outer optimization already parallelises across the individuals being
    // post-processed, so inline inner evaluation costs no useful parallelism.
    ea_ptr->setInlineEvaluation(true);

    // Add our individual to the algorithm (the population owns its individuals by unique_ptr; this
    // shared_ptr is bridged across the boundary with a clone -- the optimized result is read back below).
    ea_ptr->push_back(p_unopt_ptr->clone_unique());

    // The genome carries only structure -- the adaptors live on an OA-owned config. The post-optimizer is
    // a GENERIC local refiner with no knowledge of the problem's specific adaptor configuration, so it
    // authors a default adaption config from the genome's own structure: a Gauss adaptor on every FP
    // group, an integer-Gauss adaptor on every int32 group and a flip adaptor on every bool group. Each
    // Gauss step is scaled by the group's comparative range, so a single relative sigma suits any
    // parameter bounds. Without this the sub-EA has no adaption config and would hard-error at init().
    if(const auto *flat = dynamic_cast<const gen::GFlatGenome *>(p_unopt_ptr.get())) {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*flat);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); ++i) {
            cfg->groupDouble(i).gauss(0.5, 0.8, 1e-3, 2., 1.);
        }
        for(std::size_t i = 0; i < cfg->floatGroups().size(); ++i) {
            cfg->groupFloat(i).gauss(0.5f, 0.8f, 1e-3f, 2.f, 1.f);
        }
        for(std::size_t i = 0; i < cfg->int32Groups().size(); ++i) {
            cfg->groupInt32(i).intGauss(0.5, 0.8, 1e-3, 2., 1.);
        }
        for(std::size_t i = 0; i < cfg->boolGroups().size(); ++i) {
            cfg->groupBool(i).flip(1.);
        }
        ea_ptr->setAdaptionConfig(cfg);
    }

    // Perform the actual (sub-)optimization
    ea_ptr->optimize();

    // Retrieve the best individual
    std::shared_ptr<gen::GOptimizableEntity> p_opt_ptr = ea_ptr->getBestGlobalIndividual<gen::GOptimizableEntity>();

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
 * @brief The standard constructor. Intentionally private, as it is only needed
 * for de-serialization purposes.
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer() { /* nothing */
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEvolutionaryAlgorithmPostOptimizer::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GPostProcessorBaseT<gen::GOptimizableEntity>::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithmPostOptimizer::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvolutionaryAlgorithmPostOptimizer::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GPostProcessorBaseT<gen::GOptimizableEntity>::specificTestsNoFailureExpected_GUnitTests_();

    //---------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithmPostOptimizer::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvolutionaryAlgorithmPostOptimizer::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GPostProcessorBaseT<gen::GOptimizableEntity>::specificTestsFailuresExpected_GUnitTests_();

    //---------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithmPostOptimizer::specificTestsFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
