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
#include "geneva/GParameterSet.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "geneva/oa/GNelderMead.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for the Nelder-Mead
 * downhill simplex. It will only return objects which perform all evaluation
 * through the broker.
 */
class GNelderMeadFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOAFactoryT<GBase> {
public:
    /** @brief The default constructor */
    GNelderMeadFactory();
    /** @brief Initialization with the name of the config file */
    explicit GNelderMeadFactory(std::filesystem::path const &);
    /** @brief Initialization with the name of the config file and a content creator */
    GNelderMeadFactory(
        const std::string &,
        std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>>
    );
    /** @brief The copy constructor */
    GNelderMeadFactory(const GNelderMeadFactory &) = default;
    /** @brief The destructor */
    ~GNelderMeadFactory() override = default;

    /** @brief Gives access to the mnemonics / nickname describing an algorithm */
    std::string getMnemonic() const override;
    /** @brief Gives access to a clear-text description of the algorithm */
    std::string getAlgorithmName() const override;

protected:
    /** @brief Allows to act on the configuration options received from the configuration file */
    void postProcess_(std::shared_ptr<GBase> &) override;

private:
    /** @brief Creates individuals of this type */
    std::shared_ptr<GBase>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
