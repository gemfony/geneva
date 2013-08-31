/**
 * @file GExceptions.cpp
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

#include "common/GExceptions.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * The standard constructor
 */
gemfony_error_condition::gemfony_error_condition(const std::string& description) throw()
      : description_(description)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
gemfony_error_condition::~gemfony_error_condition()  throw()
{ /* nothing */ }

/******************************************************************************/
/**
 * Emits information when thrown
 */
const char* gemfony_error_condition::what() const throw() {
   return description_.c_str();
}

/******************************************************************************/
/**
 * This function allows to output a gemfony_error_condition to a stream
 */
std::ostream& operator<<(std::ostream& o, const Gem::Common::gemfony_error_condition& g) {
   o << g.what();
   return o;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
/**
 * Raise an exception if a given define wasn't set. "F" stands for "function",
 * "D" for "define".
 */
void condnotset(const std::string& F, const std::string& D) {
   std::ostringstream error;
   error
        << std::endl
        << "================================================" << std::endl
        << "In function " << F << " Error!" << std::endl
        << "Function was called even though " << D << " hasn't been set." << std::endl
        << "================================================" << std::endl;                               \
        throw(Gem::Common::gemfony_error_condition(error.str()));
}

/******************************************************************************/
