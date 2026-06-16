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

#include "geneva/GenevaHelperFunctions.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include <cstddef>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

namespace Gem::Geneva {

/******************************************************************************/
/**
 * Transforms the individual fitness so that the optimization algorithm always
 * "sees" a minimization problem. Optimization algorithms should only use this
 * function to retrieve the fitness of individuals.
 *
 * @param item_ptr The work item for which the fitness should be retrieved
 * @param id The id of the fitness criterion (individuals may have more than one)
 */
double minOnly_transformed_fitness(
    const gen::GOptimizableEntity &item,
    const std::size_t id // NOLINT(misc-unused-parameters)
) {
    const double f = item.transformed_fitness(id); // NOLINT(cppcoreguidelines-init-variables)
    const maxMode m = item.getMaxMode();           // NOLINT(cppcoreguidelines-init-variables)

    if(maxMode::MINIMIZE == m) {
        return f;
    }
            // MAXIMIZE
        // Negation will transform maximization problems into minimization problems
        if(std::numeric_limits<double>::max() == f) {
            return std::numeric_limits<double>::lowest();
        }
        else if(std::numeric_limits<double>::lowest() == f) {
            return std::numeric_limits<double>::max();
        }
        else {
            return -f;
        }
   
}

/******************************************************************************/
/**
 * Checks whether the first individual is better than the second. The comparison
 * is done with the first (main) fitness criterion.
 */
bool isBetter(
    const std::shared_ptr<gen::GOptimizableEntity> &x_ptr,
    const std::shared_ptr<gen::GOptimizableEntity> &y_ptr
) {
#ifdef DEBUG
    const auto x_mode = x_ptr->getMaxMode();
    const auto y_mode = y_ptr->getMaxMode();
    // Cross-check that both work items have the same maxMode
    if(x_mode != y_mode) {
        // Throw an exception
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In isBetterThan(x_ptr, y_ptr):" << '\n'
            << "Got different maxMode-settings: " << x_mode << " / " << y_mode << '\n'
        );
    }
#endif

    // We assume that both items have the same maxMode and simply compare the "minOnly-Fitness"
    if(minOnly_transformed_fitness(*x_ptr) < minOnly_transformed_fitness(*y_ptr)) {
        return true;
    }
            return false;
   
}

/******************************************************************************/
/**
 * Checks whether the first individual is worse than the second. The comparison
 * is done with the first (main) fitness criterion.
 */
bool isWorse(
    const std::shared_ptr<gen::GOptimizableEntity> &x_ptr,
    const std::shared_ptr<gen::GOptimizableEntity> &y_ptr
) {
    return not isBetter(x_ptr, y_ptr);
}

/******************************************************************************/
/**
 * Checks whether the first value is better than the second
 */
bool isBetter(const double x, const double y, const maxMode m) {
    if(maxMode::MAXIMIZE == m) {
        if(x > y) {
            return true;
        }
                    return false;
       
    }
    else {
        // maxMode::MINIMIZE
        if(x < y) {
            return true;
        }
                    return false;
       
    }
}

/******************************************************************************/
/**
 * Checks whether the first value is worse than the second
 */
bool isWorse(const double x, const double y, const maxMode m) {
    return not isBetter(x, y, m);
}

/******************************************************************************/

} /* namespace Gem::Geneva */
