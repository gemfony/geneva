/**
 * @file GDataExchangeEnums.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <string>
#include <iostream>

// Includes check for correct Boost version(s)
#include <common/GGlobalDefines.hpp>

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

#ifndef GDATAEXCHANGEENUMS_HPP_
#define GDATAEXCHANGEENUMS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here


namespace Gem {
namespace Dataexchange {

/************************************************************************************************/
/**
 * The allowed modes during data exchange with external programs
 */
enum dataExchangeMode {
          BINARYEXCHANGE = 0
        , TEXTEXCHANGE = 1
};

/************************************************************************************************/
/** @brief Puts a Gem::Geneva::dataExchangeMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Dataexchange::dataExchangeMode&);
/** @brief Reads a Gem::Geneva::dataExchangeMode from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Dataexchange::dataExchangeMode&);
/************************************************************************************************/

} /* namespace Dataexchange */
} /* namespace Gem */

#endif /* GDATAEXCHANGEENUMS_HPP_ */
