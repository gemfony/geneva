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
#include <vector>
#include <utility>
#include <thread>
#include <mutex>

// Boost header files go here

// Geneva headers go here
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class specifies the interface that needs to be implemented by optimization
 * algorithms.
 */
template <typename optimizer_type>
class G_Interface_OptimizerT {
public:
	 /***************************************************************************/
	 // Deleted move construction and move assignment operators. For now moving
	 // optimization algorithms seems too complex and of very limited use, so
	 // we prevent it until the need arises.

	 G_API_GENEVA G_Interface_OptimizerT(G_Interface_OptimizerT<optimizer_type> &&) noexcept = delete;
	 G_API_GENEVA G_Interface_OptimizerT<optimizer_type>& operator=(G_Interface_OptimizerT<optimizer_type> &&) noexcept = delete;

	 /***************************************************************************/
	 /**
	  * Triggers the optimization cycle, starting to count iterations at a given offset
	  */
	 optimizer_type const * const optimize(std::uint32_t offset = 0) {
	 	return this->optimize_(offset);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found so far and converts it to a given target type. Note that
	  * this function will not allow you to modify the best individual itself as it will
	  * return a copy to you.
	  *
	  * @return A copy of the best individual found in the optimization run
	  */
	 template <typename individual_type>
	 std::shared_ptr<individual_type> getBestGlobalIndividual(
		 typename std::enable_if<std::is_base_of<GParameterSet, individual_type>::value>::type *dummy = nullptr
	 ) const {
		 std::unique_lock<std::mutex> iteration_best_lock(m_get_best_mutex);
		 return std::dynamic_pointer_cast<individual_type>(this->getBestGlobalIndividual_());
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found so far and converts them to a given target type.
	  * Note that this function will not allow you to modify the best individuals themselves
	  * as it will return copies to you.
	  *
	  * @return A list of copies of the best individuals found in the optimization run
	  */
	 template <typename individual_type>
	 std::vector<std::shared_ptr<individual_type>> getBestGlobalIndividuals(
		 typename std::enable_if<std::is_base_of<GParameterSet, individual_type>::value>::type * dummy = nullptr
	 ) const {
		 std::unique_lock<std::mutex> iteration_best_lock(m_get_best_mutex);

		 std::vector<std::shared_ptr<individual_type>> bestIndividuals;
		 std::vector<std::shared_ptr<GParameterSet>> bestBaseIndividuals = this->getBestGlobalIndividuals_();

		 // Cross check that we indeed got a valid set of individuals
		 if(bestBaseIndividuals.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_Interface_OptimizerT<optimizer_type>::getBestGlobalIndividuals(): Error!" << std::endl
					 << "Received empty collection of best individuals." << std::endl
			 );
		 }

		 for(auto const& ind_ptr: bestBaseIndividuals) {
			 bestIndividuals.push_back(std::dynamic_pointer_cast<individual_type>(ind_ptr));
		 }

		 return std::move(bestIndividuals);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found in the iteration and converts it to a given target type.
	  * Note that this function will not allow you to modify the best individual itself as it will
	  * return a copy to you. Retrieval of this copy is protected by a lock, so that potentially
	  * costly operations on results may be performed in parallel (i.e. a copy of the best individual
	  * is retrieved under protection, any action on this individual may then be carried out in parallel).
	  *
	  * @return A copy of the best individual found in the iteration
	  */
	 template <typename individual_type>
	 std::shared_ptr<individual_type> getBestIterationIndividual(
		 typename std::enable_if<std::is_base_of<GParameterSet, individual_type>::value>::type *dummy = nullptr
	 ) const {
		 std::unique_lock<std::mutex> iteration_best_lock(m_get_best_mutex);
		 return getBestIterationIndividual_()->template clone<individual_type>();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found in the iteration and converts them to a
	  * given target type. Note that this function will not allow you to modify the best individuals
	  * themselves as it will return copies to you.
	  *
	  * @return A list of copies of the best individuals found in the iteration
	  */
	 template <typename individual_type>
	 std::vector<std::shared_ptr<individual_type>> getBestIterationIndividuals(
		 typename std::enable_if<std::is_base_of<GParameterSet, individual_type>::value>::type *dummy = nullptr
	 ) const {
		 std::unique_lock<std::mutex> iteration_best_lock(m_get_best_mutex);

		 std::vector<std::shared_ptr<individual_type>> bestIndividuals;
		 std::vector<std::shared_ptr<GParameterSet>> bestBaseIndividuals = this->getBestIterationIndividuals_();

		 // Cross check that we indeed got a valid set of individuals
		 if(bestBaseIndividuals.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_Interface_OptimizerT<optimizer_type>::getBestIterationIndividuals(): Error!" << std::endl
					 << "Received empty collection of best individuals." << std::endl
			 );
		 }

		 for(auto const & ind_ptr: bestBaseIndividuals) {
			 bestIndividuals.push_back(ind_ptr->template clone<individual_type>());
		 }

		 return std::move(bestIndividuals);
	 }

	 /***************************************************************************/
	 /**
	  * Returns one-word information about the type of optimization algorithm.
	  */
	 std::string getAlgorithmPersonalityType() const {
		 return this->getAlgorithmPersonalityType_();
	 }

	 /***************************************************************************/
	 /**
	  * Returns a descriptive name assigned to this algorithm
	  */
	 std::string getAlgorithmName() const {
	 	return this->getAlgorithmName_();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current iteration of this object
	  */
	 std::uint32_t getIteration() const {
	 	return this->getIteration_();
	 }

protected:
	 /***************************************************************************/
	 // Defaulted or constructors / destructors / assignment operators

	 G_API_GENEVA G_Interface_OptimizerT() = default;
	 G_API_GENEVA G_Interface_OptimizerT(G_Interface_OptimizerT<optimizer_type> const &) = default;

	 /**
 	  * The destructor. Making this function protected and non-virtual follows
 	  * this discussion: http://www.gotw.ca/publications/mill18.htm
 	  */
	 G_API_GENEVA ~G_Interface_OptimizerT() = default;

	 G_API_GENEVA G_Interface_OptimizerT<optimizer_type>& operator=(G_Interface_OptimizerT<optimizer_type> const &) = default;

	 /***************************************************************************/

private:
	 /** @brief Perform the actual optimization cycle, starting to count iterations at a given offset */
	 virtual G_API_GENEVA optimizer_type const * const optimize_(std::uint32_t offset) BASE = 0;

	 /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	 virtual G_API_GENEVA void runFitnessCalculation_() BASE = 0;

	 /** @brief Retrieves the best individual found globally */
	 virtual G_API_GENEVA std::shared_ptr<GParameterSet> getBestGlobalIndividual_() const BASE = 0;
	 /** @brief Retrieves a list of the best individuals found globally*/
	 virtual G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestGlobalIndividuals_() const BASE = 0;
	 /** @brief Retrieves the best individual found in the current iteration*/
	 virtual G_API_GENEVA std::shared_ptr<GParameterSet> getBestIterationIndividual_() const BASE = 0;
	 /** @brief Retrieves a list of the best individuals found in the current iteration */
	 virtual G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestIterationIndividuals_() const BASE = 0;

	 /** @brief Returns one-word information about the type of optimization algorithm. */
	 virtual std::string getAlgorithmPersonalityType_() const BASE = 0;
	 /** @brief Returns a descriptive name assigned to this algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName_() const BASE = 0;
	 /** @brief Retrieves the current iteration of this object */
	 virtual G_API_GENEVA std::uint32_t getIteration_() const BASE = 0;

	 /***************************************************************************/
	 // Data

	 mutable std::mutex m_get_best_mutex; ///< Protects access to the best individual of an iteration
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/

