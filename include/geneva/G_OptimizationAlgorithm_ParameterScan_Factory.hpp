/**
 * @file
 */

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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/filesystem.hpp>

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_FactoryT.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_ParameterScan.hpp"
#include "geneva/G_OptimizationAlgorithm_InitializerT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for simulated annealing.
 * It will only return objects which perform all evaluation through the broker.
 */
class GParameterScanFactory
	: public G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA GParameterScanFactory();
	 /** @brief Initialization with the name of the config file */
	 explicit G_API_GENEVA GParameterScanFactory(boost::filesystem::path const&);
	 /** @brief Initialization with the name of the config file and a content creator */
	 G_API_GENEVA GParameterScanFactory(
		 boost::filesystem::path const&
		 , std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>>
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA GParameterScanFactory(const GParameterScanFactory&) = default;
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GParameterScanFactory() = default;

	 /** @brief Gives access to the mnemonics / nickname describing an algorithm */
	 G_API_GENEVA std::string getMnemonic() const override;
	 /** @brief Gives access to a clear-text description of the algorithm */
	 G_API_GENEVA std::string getAlgorithmName() const override;

	 /** @brief Adds local command line options to boost::program_options::options_description objects */
	 G_API_GENEVA void addCLOptions(
		 boost::program_options::options_description&
		 , boost::program_options::options_description&
	 ) override;

	 /** @brief Allows to specify the command line parameter manually for variables to be scanned */
	 G_API_GENEVA void setCLParameterSpecs(std::string parStr);
	 /** @brief Allows to retrieve the command line parameter settings for variables to be scanned */
	 G_API_GENEVA std::string getCLParameterSpecs() const;
	 /** @brief Allows to reset the command line parameter specs */
	 G_API_GENEVA void resetCLParameterSpecs();

protected:
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 G_API_GENEVA void postProcess_(std::shared_ptr<G_OptimizationAlgorithm_Base>&) override;

private:
	 /** @brief Creates individuals of this type */
	 G_API_GENEVA std::shared_ptr<G_OptimizationAlgorithm_Base> getObject_(
		Gem::Common::GParserBuilder&
		, const std::size_t&
	 ) override;

	 /** @brief Holds information on the variables to be optimized -- set through the corresponding member function or on the command line */
	 std::string m_parameterSpecCL = "empty";
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

