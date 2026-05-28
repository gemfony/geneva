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

#include "geneva/oa/GParameterScanFactory.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GParameterScan_PersonalityTraits.hpp"
#include "geneva/par/GParameterSet.hpp"
#include <boost/program_options.hpp>
#include <cstddef>
#include <memory>
#include <string>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The default constructor
 */
GParameterScanFactory::GParameterScanFactory()
  : GOAFactoryT<GBase>(
        "./config/GParameterScan.json"
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GParameterScanFactory::GParameterScanFactory(std::filesystem::path const &config_file)
  : GOAFactoryT<GBase>(config_file) { /* nothing */
}

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GParameterScanFactory::GParameterScanFactory(
    std::filesystem::path const &config_file,
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GParameterSet>> content_creator_ptr
)
  : GOAFactoryT<GBase>(
        config_file,
        content_creator_ptr
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GParameterScanFactory::getMnemonic() const {
    return GParameterScan_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GParameterScanFactory::getAlgorithmName() const {
    return std::string("Parameter Scan");
}

/***************************************************************************/
/**
 * Adds local command line options to a boost::program_options::options_description object.
 *
 * @param visible Command line options that should always be visible
 * @param hidden Command line options that should only be visible upon request
 */
void GParameterScanFactory::addCLOptions(
    boost::program_options::options_description &visible // NOLINT(misc-unused-parameters)
    ,
    boost::program_options::options_description &hidden
) {
    namespace po = boost::program_options;

    hidden.add_options()(
        "parameterSpec",
        po::value<std::string>(&parameter_spec_cl_)->default_value(std::string("empty")),
        "\t[GParameterScanFactory] Specification of parameters to be scanned. Syntax: \"d(0, -10., "
        "10., 100)\". Use a comma-separated list for more than one variable. A single entry "
        "\"s(1000)\" will lead to a random scan over all parameters of up to 1000 individuals"
    );

    // Add the parent class'es options
    GOAFactoryT<GBase>::addCLOptions(visible, hidden);
}

/******************************************************************************/
/**
 * Allows to specify the command line parameter manually for variables to be scanned
 */
void GParameterScanFactory::setCLParameterSpecs(std::string par_str) {
    parameter_spec_cl_ = par_str;
}

/******************************************************************************/
/**
 * Allows to retrieve the command line parameter settings for variables to be scanned
 */
std::string GParameterScanFactory::getCLParameterSpecs() const {
    return parameter_spec_cl_;
}

/******************************************************************************/
/**
 * Allows to reset the command line parameter specs
 */
void GParameterScanFactory::resetCLParameterSpecs() {
    parameter_spec_cl_ = "empty";
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GBase> GParameterScanFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
) {
    std::shared_ptr<GParameterScan> target(new GParameterScan());

    // Make the local configuration options known (up to the level of GParameterScan)
    target->GParameterScan::addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p_base A smart-pointer to be acted on during post-processing
 */
void GParameterScanFactory::postProcess_(std::shared_ptr<GBase> &p_base) {
    if(parameter_spec_cl_ != "empty") {
        std::shared_ptr<GParameterScan> p =
            Gem::Common::convertSmartPointer<GBase, GParameterScan>(p_base);

        p->setParameterSpecs(parameter_spec_cl_);
    }

    // Call our parent class'es function
    GOAFactoryT<GBase>::postProcess_(p_base);
}

/******************************************************************************/

/******************************************************************************/
/**
 * Self-registration of this optimization-algorithm factory with the global
 * factory store at library-load time, so that Go2 needs no explicit
 * registration call. (Geneva is always built as a shared library, so these
 * load-time initializers are never stripped.)
 */
namespace {
GInitializerT<GParameterScanFactory> g_oaf_registrant;
} // anonymous namespace

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
