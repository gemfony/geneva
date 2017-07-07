/**
 * @file GOptimizableI.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>

// Boost header files go here

#ifndef GOPTIMIZATIONALGORITHMI_HPP_
#define GOPTIMIZATIONALGORITHMI_HPP_

// Geneva headers go here
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class specifies the interface that needs to be implemented by optimization
 * algorithms.
 */
class GOptimizableI {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &, const unsigned int){
		 using boost::serialization::make_nvp;
		 /* nothing */
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GOptimizableI();
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GOptimizableI();

	 /** @brief Perform the actual optimization cycle, starting to count iterations at a given offset */
	 virtual G_API_GENEVA void optimize(const std::uint32_t& offset) BASE = 0;

	 /** @brief A simple wrapper function that forces the class to start with offset 0 */
	 virtual G_API_GENEVA void optimize() BASE;

	 /** @brief Retrieves the current iteration of this object */
	 virtual G_API_GENEVA std::uint32_t getIteration() const BASE = 0;

	 /***************************************************************************/
	 /**
	  * Starts the optimization cycle and returns the best individual found, converted to
	  * the desired target type.
	  *
	  * @return The best individual found during the optimization process, converted to the desired type
	  */
	 template <typename individual_type>
	 std::shared_ptr<individual_type> optimize() {
		 this->optimize(0);
		 return this->getBestGlobalIndividual<individual_type>();
	 }

	 /***************************************************************************/
	 /**
	  * Starts the optimization cycle and returns the best individual found, converted to
	  * the desired target type. This function uses a configurable offset for the iteration
	  * counter
	  *
	  * @param offset An offset for the iteration counter
	  * @return The best individual found during the optimization process, converted to the desired type
	  */
	 template <typename individual_type>
	 std::shared_ptr<individual_type> optimize(
		 const std::uint32_t& offset
	 ) {
		 this->optimize(offset);
		 return this->getBestGlobalIndividual<individual_type>();
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
	 ) {
		 std::lock_guard<std::mutex> iteration_best_lock(m_get_best_mutex);
		 return customGetBestGlobalIndividual()->clone<individual_type>();
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
	 ) {
		 std::lock_guard<std::mutex> iteration_best_lock(m_get_best_mutex);

		 std::vector<std::shared_ptr<individual_type>> bestIndividuals;
		 std::vector<std::shared_ptr<GParameterSet>> bestBaseIndividuals = this->customGetBestGlobalIndividuals();

		 // Cross check that we indeed got a valid set of individuals
		 if(bestBaseIndividuals.empty()) {
			 glogger
				 << "In GOptimizableI::getBestGlobalIndividuals(): Error!" << std::endl
				 << "Received empty collection of best individuals." << std::endl
				 << GEXCEPTION;
		 }

		 for(auto ind_ptr: bestBaseIndividuals) { // std::shared_ptr may be copied
			 bestIndividuals.push_back(ind_ptr->clone<individual_type>());
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
		 std::lock_guard<std::mutex> iteration_best_lock(m_get_best_mutex);
		 return customGetBestIterationIndividual()->clone<individual_type>();
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
		 std::lock_guard<std::mutex> iteration_best_lock(m_get_best_mutex);

		 std::vector<std::shared_ptr<individual_type>> bestIndividuals;
		 std::vector<std::shared_ptr<GParameterSet>> bestBaseIndividuals = this->customGetBestIterationIndividuals();

		 // Cross check that we indeed got a valid set of individuals
		 if(bestBaseIndividuals.empty()) {
			 glogger
				 << "In GOptimizableI::getBestIterationIndividuals(): Error!" << std::endl
				 << "Received empty collection of best individuals." << std::endl
				 << GEXCEPTION;
		 }

		 for(auto ind_ptr: bestBaseIndividuals) { // std::shared_ptr may be copied
			 bestIndividuals.push_back(ind_ptr->clone<individual_type>());
		 }

		 return std::move(bestIndividuals);
	 }

	 /***************************************************************************/

	 /** @brief Returns one-word information about the type of optimization algorithm. */
	 virtual G_API_GENEVA std::string getOptimizationAlgorithm() const BASE;

	 /** @brief Returns a descriptive name assigned to this algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName() const = 0;
	 /** @brief Checks whether a given algorithm type likes to communicate via the broker */
	 virtual G_API_GENEVA bool usesBroker() const BASE;

protected:
	 /***************************************************************************/
	 /** @brief Retrieves the best individual found globally */
	 virtual G_API_GENEVA std::shared_ptr<GParameterSet> customGetBestGlobalIndividual() BASE = 0;
	 /** @brief Retrieves a list of the best individuals found globally*/
	 virtual G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> customGetBestGlobalIndividuals() BASE = 0;
	 /** @brief Retrieves the best individual found in the current iteration*/
	 virtual G_API_GENEVA std::shared_ptr<GParameterSet> customGetBestIterationIndividual() BASE = 0;
	 /** @brief Retrieves a list of the best individuals found in the current iteration */
	 virtual G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> customGetBestIterationIndividuals() BASE = 0;
	 /***************************************************************************/
	 /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	 virtual G_API_GENEVA void runFitnessCalculation() BASE = 0;

	 /***************************************************************************/

private:
	std::mutex m_get_best_mutex; ///< Protects access to the best individual of an iteration
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

#endif /* GOPTIMIZATIONALGORITHMI_HPP_ */
