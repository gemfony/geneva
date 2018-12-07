/**
 * @file
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
	 /** @brief Perform the actual optimization cycle, starting to count iterations at a given offset */
	 virtual G_API_GENEVA const optimizer_type * const optimize(const std::uint32_t& offset) BASE = 0;

	 /** @brief Retrieves the current iteration of this object */
	 virtual G_API_GENEVA std::uint32_t getIteration() const BASE = 0;

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

		 for(const auto& ind_ptr: bestBaseIndividuals) { // std::shared_ptr may be copied
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
	 std::shared_ptr<individual_type> getBestIterationIndividual (
		 typename std::enable_if<std::is_base_of<GParameterSet, individual_type>::value>::type *dummy = nullptr
	 ) {
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
	 ) {
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

		 for(const auto& ind_ptr: bestBaseIndividuals) { // std::shared_ptr may be copied
			 bestIndividuals.push_back(ind_ptr->template clone<individual_type>());
		 }

		 return std::move(bestIndividuals);
	 }

	 /***************************************************************************/
	 /**
	  * Returns one-word information about the type of optimization algorithm.
	  */
	 virtual std::string getAlgorithmPersonalityType() const BASE {
		 return std::string("PERSONALITY_NONE");
	 }

	 /***************************************************************************/

	 /** @brief Returns a descriptive name assigned to this algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName() const BASE = 0;

protected:
	 /***************************************************************************/
	 // Defaulted constructors / destructors

	 /** @brief The default constructor */
	 G_API_GENEVA G_Interface_OptimizerT() = default;
	 /**
 	  * The destructor. Making this function protected and non-virtual follows
 	  * this discussion: http://www.gotw.ca/publications/mill18.htm
 	  */
	 G_API_GENEVA ~G_Interface_OptimizerT() = default;

	 /***************************************************************************/
	 /** @brief Retrieves the best individual found globally */
	 virtual G_API_GENEVA std::shared_ptr<GParameterSet> getBestGlobalIndividual_() const BASE = 0;
	 /** @brief Retrieves a list of the best individuals found globally*/
	 virtual G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestGlobalIndividuals_() const BASE = 0;
	 /** @brief Retrieves the best individual found in the current iteration*/
	 virtual G_API_GENEVA std::shared_ptr<GParameterSet> getBestIterationIndividual_() BASE = 0;
	 /** @brief Retrieves a list of the best individuals found in the current iteration */
	 virtual G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestIterationIndividuals_() BASE = 0;

	 /***************************************************************************/
	 /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	 virtual G_API_GENEVA void runFitnessCalculation() BASE = 0;

	 /***************************************************************************/

private:
	 mutable std::mutex m_get_best_mutex; ///< Protects access to the best individual of an iteration
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GOptimizableI)

/******************************************************************************/

