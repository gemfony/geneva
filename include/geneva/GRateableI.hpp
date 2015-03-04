/**
 * @file GRateableI.hpp
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
#include <sstream>
#include <vector>

// Boost header files go here

#ifndef GRATEABLEI_HPP_
#define GRATEABLEI_HPP_

// Geneva header files go here

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A simple interface class for objects that can be evaluated.
 */
class GRateableI {
public:
	/** @brief The destructor */
	virtual G_API ~GRateableI();

   /** @brief Retrieve a value for this class, using a fixed fitness function */
   virtual G_API double fitness() const BASE = 0;
   /** @brief Retrieve a value for this class, using a fitness function with a given id */
   virtual G_API double fitness(const std::size_t&) const BASE = 0;

   /** @brief Returns the transformed result of the fitness function with id 0 */
   virtual G_API double transformedFitness() const BASE = 0;
   /** @brief Returns the transformed result of the fitness function with id 0 */
   virtual G_API double transformedFitness(const std::size_t&) const BASE = 0;

   /** @brief Returns a fitness targetted at optimization algorithms, taking into account maximization and minimization */
   virtual G_API double minOnly_fitness() const BASE = 0;
   /** @brief Returns a fitness targetted at optimization algorithms, taking into account maximization and minimization */
   virtual G_API double minOnly_fitness(const std::size_t&) const BASE = 0;

   /** @brief Calculate or returns the result of a fitness function with a given id */
   virtual G_API double fitness(const std::size_t&, bool, bool) BASE = 0;
   /** @brief Calculate or returns the result of a fitness function with a given id */
   virtual G_API double fitness(const std::size_t&, bool, bool) const BASE = 0;

   /** @brief Returns all raw fitness results in a std::vector */
   virtual G_API std::vector<double> fitnessVec() const BASE = 0;
   /** @brief Returns all raw or transformed results in a std::vector */
   virtual G_API std::vector<double> fitnessVec(bool) const BASE = 0;
   /** @brief Returns all transformed fitness results in a std::vector */
   virtual G_API std::vector<double> transformedFitnessVec() const BASE = 0;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GRATEABLEI_HPP_ */
