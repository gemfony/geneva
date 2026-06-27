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
#include <iostream>
#include <sstream>

// Boost header files go here

// Geneva header files go here

namespace Gem::Geneva::Interface {

/******************************************************************************/
/**
 * This is a interface class for mutable objects, in particular individuals.
 *
 * Historically it declared a pure virtual adapt(). With the struct-based population the
 * per-group adaption state and the adaption logic moved entirely off the individual and onto the
 * optimization-algorithm side (the GIndividualSlot's scratch + the geneva/oa/ adaption free functions),
 * so the individual is now pure data and carries no adapt() of its own. The interface is kept as a
 * minimal marker base for the individual hierarchy.
 */
class GMutableI {
protected:
    /**************************************************************************/
    // Defaulted constructors / destructors / assignment operators / rule of five

    /** @brief The default constructor */
    GMutableI() = default;
    /**
      * @brief The copy constructor
      * @param other Another GMutableI object whose (empty) base state is copied into this one
      */
    GMutableI(GMutableI const &) = default;
    /**
      * @brief The move constructor
      * @param other Another GMutableI object whose (empty) base state is moved into this one
      */
    GMutableI(GMutableI &&) = default;

    /**
      * @brief The destructor. Making this function protected and non-virtual follows
      * this discussion: http://www.gotw.ca/publications/mill18.htm
      */
    ~GMutableI() = default;

    /**
      * @brief The copy assignment operator
      * @param other Another GMutableI object whose (empty) base state is copied into this one
      * @return A reference to this object
      */
    GMutableI &operator=(GMutableI const &) = default;
    /**
      * @brief The move assignment operator
      * @param other Another GMutableI object whose (empty) base state is moved into this one
      * @return A reference to this object
      */
    GMutableI &operator=(GMutableI &&) = default;

    /**************************************************************************/
};

} /* namespace Gem::Geneva::Interface */

/******************************************************************************/
