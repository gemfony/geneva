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
#include <iomanip>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cfloat>
#include <climits>

// Boost headers go here
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GASIOHELPERFUNCTIONS_HPP_
#define GASIOHELPERFUNCTIONS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GEnums.hpp"
#include "GenevaExceptions.hpp"
#include "GAsioTCPClient.hpp"

namespace Gem
{
namespace GenEvA
{

/**********************************************************************************/
/** @brief Assembles a query string from a given command */
std::string assembleQueryString(const std::string&, const std::size_t&);

/** @brief Extracts the size of ASIO's data section from a C string. */
std::size_t extractDataSize(const char*, const std::size_t&);

/** @brief Starts a number of threads with clients */
void startNClients(unsigned short, std::string, unsigned short);

/**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GASIOHELPERFUNCTIONS_HPP_ */
