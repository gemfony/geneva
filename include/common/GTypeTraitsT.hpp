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

// Standard headers go here
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

// Boost headers go here

// Geneva headers go here

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// This simple class is used to simplify detection of classes that have the common
// interface of Gemfony libraries.
class gemfony_common_interface_indicator {};

/**
 * A type trait helping to check whether a class has the Gemfony Scientific library
 * interface. The simple convention is that the base class of a hierarchy must
 * (we recommend) privately inherit from common_gemfony_iterface .
 */
template <typename T>
struct has_gemfony_common_interface {
    enum {
        value = std::is_base_of < gemfony_common_interface_indicator,
        T > ::value
    };
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a compare function.
 */
template <typename T>
class has_compare_member {
    using yes = char;
    using no = long;

    template <typename C>
    static yes test(decltype(&C::compare));

    template <typename C>
    static no test(...);

public:
    enum {
        value = sizeof(test<T>(0)) == sizeof(char)
    };
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a clone function.
 */
template <typename T>
class has_clone_member {
    using yes = char;
    using no = long;

    template <typename C>
    static yes test(decltype(&C::clone));

    template <typename C>
    static no test(...);

public:
    enum {
        value = sizeof(test<T>(0)) == sizeof(char)
    };
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a load function.
 */
template <typename T>
class has_load_member {
    using yes = char;
    using no = long;

    template <typename C>
    static yes test(decltype(&C::load));

    template <typename C>
    static no test(...);

public:
    enum {
        value = sizeof(test<T>(0)) == sizeof(char)
    };
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */
