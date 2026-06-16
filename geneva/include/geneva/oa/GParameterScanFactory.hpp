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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>

// Boost header files go here

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/oa/GOptimizationAlgorithmFactoryT.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GParameterScan_PersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for simulated annealing.
 * It will only return objects which perform all evaluation through the broker.
 */
class GParameterScanFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmFactoryT<GParameterScan, GParameterScan_PersonalityTraits> {
    using Base = GOptimizationAlgorithmFactoryT<GParameterScan, GParameterScan_PersonalityTraits>;

public:
    /** @brief The default constructor */
    GParameterScanFactory() = default;
    /** @brief Initialization with the name of the config file */
    explicit GParameterScanFactory(std::filesystem::path const &config_file)
      : Base(config_file) { /* nothing */ }
    /** @brief Initialization with the name of the config file and a content creator */
    GParameterScanFactory(
        std::filesystem::path const &config_file,
        std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> content_creator_ptr
    )
      : Base(config_file, content_creator_ptr) { /* nothing */ }
    /** @brief The copy constructor */
    GParameterScanFactory(const GParameterScanFactory &) = default;
    /** @brief The destructor */
    ~GParameterScanFactory() override = default;

    // The constructors' config path, getMnemonic(), getAlgorithmName() and getObject_() are generated
    // by the GOptimizationAlgorithmFactoryT scaffold. Parameter Scan adds a command-line parameter
    // spec (the variables to scan), so it also overrides addCLOptions()/postProcess_() below.

    /** @brief Adds local command line options to boost::program_options::options_description objects */
    void addCLOptions(
        boost::program_options::options_description &,
        boost::program_options::options_description &
    ) override;

    /** @brief Allows to specify the command line parameter manually for variables to be scanned */
    void setCLParameterSpecs(std::string par_str);
    /** @brief Allows to retrieve the command line parameter settings for variables to be scanned */
    std::string getCLParameterSpecs() const;
    /** @brief Allows to reset the command line parameter specs */
    void resetCLParameterSpecs();

protected:
    /** @brief Allows to act on the configuration options received from the configuration file */
    void postProcess_(std::shared_ptr<GOptimizationAlgorithmBase> &) override;

private:
    /** @brief Holds information on the variables to be optimized -- set through the corresponding member function or on the command line */
    std::string parameter_spec_cl_ = "empty";
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

