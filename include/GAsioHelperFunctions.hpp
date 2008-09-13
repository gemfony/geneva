/**
 * @file GAsioHelperFunctions.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the Geneva library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cfloat>
#include <climits>

// Boost headers go here

#ifndef GASIOHELPERFUNCTIONS_HPP_
#define GASIOHELPERFUNCTIONS_HPP_

// GenEvA headers go here

#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem
{
namespace GenEvA
{

/**********************************************************************************/
/** @brief Needed so that server and client agree about the size of the headers and commands */
const std::size_t COMMANDLENGTH=64;

/** @brief Assembles a query string from a given command */
std::string assembleQueryString(const std::string&, const std::size_t&);

/** @brief Extracts the size of ASIO's data section from a C string. */
std::size_t extractDataSize(const char*, const std::size_t&);

/**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GASIOHELPERFUNCTIONS_HPP_ */
