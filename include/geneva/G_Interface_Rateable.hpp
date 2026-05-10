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
#include <sstream>
#include <vector>

// Boost header files go here

// Geneva header files go here

namespace Gem::Geneva {
/******************************************************************************/
/**
     * A simple interface class for objects that can be evaluated.
     */
class G_Interface_Rateable {
public:
    /** @brief Retrieves the stored raw fitness with a given id */
    double raw_fitness(std::size_t = 0) const;
    /** @brief Retrieves the stored transformed fitness with a given id */
    double transformed_fitness(std::size_t = 0) const;

    /** @brief Returns all raw fitness results in a std::vector */
    std::vector<double> raw_fitness_vec() const;
    /** @brief Returns all transformed fitness results in a std::vector */
    std::vector<double> transformed_fitness_vec() const;

protected:
    /** @brief The fitness calculation for the main quality criterion takes place here */
    virtual double fitnessCalculation() = 0;

    /**************************************************************************/
    // Defaulted constructors / destructors / assignment operators

    G_Interface_Rateable() = default;
    G_Interface_Rateable(G_Interface_Rateable const &) = default;
    G_Interface_Rateable(G_Interface_Rateable &&) = default;
    ~G_Interface_Rateable() = default;

    G_Interface_Rateable &operator=(G_Interface_Rateable const &) = default;
    G_Interface_Rateable &operator=(G_Interface_Rateable &&) = default;

private:
    /** @brief Retrieves the stored raw fitness with a given id */
    virtual double raw_fitness_(std::size_t) const = 0;
    /** @brief Retrieves the stored transformed fitness with a given id */
    virtual double transformed_fitness_(std::size_t) const = 0;

    /** @brief Returns all raw fitness results in a std::vector */
    virtual std::vector<double> raw_fitness_vec_() const = 0;
    /** @brief Returns all transformed fitness results in a std::vector */
    virtual std::vector<double> transformed_fitness_vec_() const = 0;
};

/******************************************************************************/
} /* namespace Gem::Geneva */
