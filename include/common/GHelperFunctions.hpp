/**
 * @file GHelperFunctions.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General
 * Public License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS DUAL-LICENSING OPTION DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * version 3 and of version 2 of the GNU General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tokenizer.hpp>

namespace bf=boost::filesystem;

#ifndef GHELPERFUNCTIONS_HPP_
#define GHELPERFUNCTIONS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/** @brief This function tries to determine a suitable number of threads for the current architecture */
unsigned int getNHardwareThreads(const unsigned int& defaultNThreads = Gem::Common::DEFAULTNHARDWARETHREADS);

/******************************************************************************/
/** @brief This function loads textual (ASCII) data from an external file */
std::string loadTextDataFromFile(const std::string&);

/******************************************************************************/
/** @brief This function executes an external command on the operating system */
void runExternalCommand(const std::string&);

/******************************************************************************/
/** @brief Returns a string for a given serialization mode */
std::string serializationModeToString(const serializationMode&);

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GHELPERFUNCTIONS_HPP_ */
