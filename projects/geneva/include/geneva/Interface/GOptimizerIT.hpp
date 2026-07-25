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
#include <concepts>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// Boost header files go here

// Geneva headers go here
#include "geneva/genome/GGenome.hpp"

namespace Gem::Geneva::Interface {

/******************************************************************************/
/**
 * @brief Common interface that every optimization algorithm must implement.
 *
 * Provides type-safe access (via dynamic_pointer_cast / clone) to the best individuals
 * found so far -- both globally and within the current iteration -- as well as basic
 * algorithm metadata and the optimization trigger. The concrete behaviour is supplied by
 * derived classes through the private virtual hooks (the underscore-suffixed members).
 *
 * @tparam optimizer_type The concrete optimization algorithm type returned by optimize().
 */
template <typename optimizer_type>
class GOptimizerIT {
public:
    /***************************************************************************/
    // Deleted move construction and move assignment operators. For now moving
    // optimization algorithms seems too complex and of very limited use, so
    // we prevent it until the need arises.

    /** @brief Deleted move constructor (moving optimization algorithms is not supported). */
    GOptimizerIT(GOptimizerIT<optimizer_type> &&) noexcept = delete;
    /** @brief Deleted move assignment (moving optimization algorithms is not supported). */
    GOptimizerIT<optimizer_type> &
    operator=(GOptimizerIT<optimizer_type> &&) noexcept = delete;

    /***************************************************************************/
    /**
	  * @brief Triggers the optimization cycle, starting to count iterations at a given offset.
	  *
	  * @param offset The iteration number at which to start counting (default 0)
	  * @return A pointer to this optimization algorithm once the cycle has completed
	  */
    optimizer_type const *optimize(std::uint32_t offset = 0) {
        return this->optimize_(offset);
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the best individual found so far and converts it to a given target type.
	  *
	  * Note that this function will not allow you to modify the best individual itself as it will
	  * return a copy to you. Throws if the dynamic_pointer_cast to individual_type fails.
	  *
	  * @tparam individual_type The concrete individual type to cast the best individual to (must derive from gen::GGenome)
	  * @return A copy of the best individual found in the optimization run
	  */
    template <typename individual_type>
        requires std::derived_from<individual_type, gen::GGenome>
    std::shared_ptr<individual_type> getBestGlobalIndividual() const {
        std::scoped_lock<std::mutex> const iteration_best_lock(get_best_mutex_);
        auto result = std::dynamic_pointer_cast<individual_type>(this->getBestGlobalIndividual_());
        if(!result) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizerIT<>::getBestGlobalIndividual(): Error!" << '\n'
                << "dynamic_pointer_cast to requested individual_type failed." << '\n'
            );
        }
        return result;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves a list of the best individuals found so far and converts them to a given target type.
	  *
	  * Note that this function will not allow you to modify the best individuals themselves
	  * as it will return copies to you. Throws if the collection is empty or if any cast fails.
	  *
	  * @tparam individual_type The concrete individual type to cast each best individual to (must derive from gen::GGenome)
	  * @return A list of copies of the best individuals found in the optimization run
	  */
    template <typename individual_type>
        requires std::derived_from<individual_type, gen::GGenome>
    std::vector<std::shared_ptr<individual_type>> getBestGlobalIndividuals() const {
        std::scoped_lock<std::mutex> const iteration_best_lock(get_best_mutex_);

        std::vector<std::shared_ptr<individual_type>> best_individuals;
        std::vector<std::shared_ptr<gen::GGenome>> const best_base_individuals =
            this->getBestGlobalIndividuals_();

        // Cross check that we indeed got a valid set of individuals
        if(best_base_individuals.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizerIT<optimizer_type>::getBestGlobalIndividuals(): Error!"
                << '\n'
                << "Received empty collection of best individuals." << '\n'
            );
        }

        for(auto const &ind_ptr : best_base_individuals) {
            auto cast_ptr = std::dynamic_pointer_cast<individual_type>(ind_ptr);
            if(!cast_ptr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GOptimizerIT<>::getBestGlobalIndividuals(): Error!" << '\n'
                    << "dynamic_pointer_cast to requested individual_type failed." << '\n'
                );
            }
            // clone() yields sole ownership; hand it to the shared_ptr collection this accessor returns
            best_individuals.push_back(std::move(cast_ptr));
        }

        return best_individuals;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the best individual found in the iteration and converts it to a given target type.
	  *
	  * Note that this function will not allow you to modify the best individual itself as it will
	  * return a copy to you. Retrieval of this copy is protected by a lock, so that potentially
	  * costly operations on results may be performed in parallel (i.e. a copy of the best individual
	  * is retrieved under protection, any action on this individual may then be carried out in parallel).
	  * Throws if the dynamic_pointer_cast to individual_type fails.
	  *
	  * @tparam individual_type The concrete individual type to cast the best individual to (must derive from gen::GGenome)
	  * @return A copy of the best individual found in the iteration
	  */
    template <typename individual_type>
        requires std::derived_from<individual_type, gen::GGenome>
    std::shared_ptr<individual_type> getBestIterationIndividual() const {
        std::scoped_lock<std::mutex> const iteration_best_lock(get_best_mutex_);
        auto result = std::dynamic_pointer_cast<individual_type>(getBestIterationIndividual_());
        if(!result) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizerIT<>::getBestIterationIndividual(): Error!" << '\n'
                << "dynamic_pointer_cast to requested individual_type failed." << '\n'
            );
        }
        return result;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves a list of the best individuals found in the iteration and converts them to a given target type.
	  *
	  * Note that this function will not allow you to modify the best individuals
	  * themselves as it will return copies to you. Throws if the collection is empty or if any clone fails.
	  *
	  * @tparam individual_type The concrete individual type to clone each best individual into (must derive from gen::GGenome)
	  * @return A list of copies of the best individuals found in the iteration
	  */
    template <typename individual_type>
        requires std::derived_from<individual_type, gen::GGenome>
    std::vector<std::shared_ptr<individual_type>> getBestIterationIndividuals() const {
        std::scoped_lock<std::mutex> const iteration_best_lock(get_best_mutex_);

        std::vector<std::shared_ptr<individual_type>> best_individuals;
        std::vector<std::shared_ptr<gen::GGenome>> const best_base_individuals =
            this->getBestIterationIndividuals_();

        // Cross check that we indeed got a valid set of individuals
        if(best_base_individuals.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizerIT<optimizer_type>::getBestIterationIndividuals(): "
                   "Error!"
                << '\n'
                << "Received empty collection of best individuals." << '\n'
            );
        }

        for(auto const &ind_ptr : best_base_individuals) {
            auto cast_ptr = ind_ptr->template clone<individual_type>();
            if(!cast_ptr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GOptimizerIT<>::getBestIterationIndividuals(): Error!" << '\n'
                    << "clone<individual_type>() returned null." << '\n'
                );
            }
            // clone() yields sole ownership; hand it to the shared_ptr collection this accessor returns
            best_individuals.push_back(std::move(cast_ptr));
        }

        return best_individuals;
    }

    /***************************************************************************/
    /**
	  * @brief Returns one-word information about the type of optimization algorithm.
	  *
	  * @return A short string identifying the algorithm's personality type
	  */
    std::string getAlgorithmPersonalityType() const {
        return this->getAlgorithmPersonalityType_();
    }

    /***************************************************************************/
    /**
	  * @brief Returns a descriptive name assigned to this algorithm.
	  *
	  * @return The human-readable name of this optimization algorithm
	  */
    std::string getAlgorithmName() const {
        return this->getAlgorithmName_();
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the current iteration of this object.
	  *
	  * @return The current iteration counter
	  */
    std::uint32_t getIteration() const {
        return this->getIteration_();
    }

protected:
    /***************************************************************************/
    // Defaulted or constructors / destructors / assignment operators

    /** @brief The default constructor. */
    GOptimizerIT() = default;
    /** @brief The copy constructor. */
    GOptimizerIT(GOptimizerIT<optimizer_type> const &) = default;

    /**
 	  * @brief The destructor.
 	  *
 	  * Making this function protected and non-virtual follows
 	  * this discussion: http://www.gotw.ca/publications/mill18.htm
 	  */
    ~GOptimizerIT() = default;

    /** @brief The copy assignment operator. */
    GOptimizerIT<optimizer_type> &
    operator=(GOptimizerIT<optimizer_type> const &) = default;

    /***************************************************************************/

private:
    /**
     * @brief Performs the actual optimization cycle, starting to count iterations at a given offset.
     *
     * @param offset The iteration number at which to start counting
     * @return A pointer to this optimization algorithm once the cycle has completed
     */
    virtual optimizer_type const *optimize_(std::uint32_t offset) = 0;

    /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
    virtual void evaluatePopulation_() = 0;

    /** @brief Retrieves the best individual found globally */
    virtual std::shared_ptr<gen::GGenome> getBestGlobalIndividual_() const = 0;
    /** @brief Retrieves a list of the best individuals found globally*/
    virtual std::vector<std::shared_ptr<gen::GGenome>>
    getBestGlobalIndividuals_() const = 0;
    /** @brief Retrieves the best individual found in the current iteration*/
    virtual std::shared_ptr<gen::GGenome> getBestIterationIndividual_() const = 0;
    /** @brief Retrieves a list of the best individuals found in the current iteration */
    virtual std::vector<std::shared_ptr<gen::GGenome>>
    getBestIterationIndividuals_() const = 0;

    /** @brief Returns one-word information about the type of optimization algorithm. */
    virtual std::string getAlgorithmPersonalityType_() const = 0;
    /** @brief Returns a descriptive name assigned to this algorithm */
    virtual std::string getAlgorithmName_() const = 0;
    /** @brief Retrieves the current iteration of this object */
    virtual std::uint32_t getIteration_() const = 0;

    /***************************************************************************/
    // Data

    mutable std::mutex get_best_mutex_; ///< Protects access to the best individuals (global and per-iteration)
};

/******************************************************************************/

} /* namespace Gem::Geneva::Interface */

/******************************************************************************/
