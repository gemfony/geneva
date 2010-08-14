/**
 * @file GExternalEvaluatorEnums.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
#include <istream>
#include <ostream>

// Includes check for correct Boost version(s)
#include <common/GGlobalDefines.hpp>

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

#ifndef GEXTERNALEVALUATORENUMS_HPP_
#define GEXTERNALEVALUATORENUMS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here


namespace Gem {

namespace Geneva {

/************************************************************************************************/
/**
 * The allowed modes during data exchange with external programs
 */
enum dataExchangeMode {
          BINARYEXCHANGE
        , TEXTEXCHANGE
};

/************************************************************************************************/
/** @brief Puts a Gem::Geneva::dataExchangeMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::dataExchangeMode&);
/** @brief Reads a Gem::Geneva::dataExchangeMode from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::dataExchangeMode&);
/************************************************************************************************/

} /* namespace Geneva */

} /* namespace Gem */

#endif /* GEXTERNALEVALUATORENUMS_HPP_ */
