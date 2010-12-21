/**
 * @file GExceptions.hpp
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

// Standard header files go here

#include <string>
#include <sstream>
#include <exception>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#ifndef GEXCEPTIONS_HPP_
#define GEXCEPTIONS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

namespace Gem {
namespace Common {

/**************************************************************************************************/
// Exceptions and related definitions

/** @brief General error class to be thrown in the case of severe errors */
class gemfony_error_condition : public std::exception {
public:
	gemfony_error_condition(const std::string& description) throw() { description_ = description; }
	virtual ~gemfony_error_condition()  throw() {;}

	virtual const char* what() const throw() {
		return description_.c_str();
	}

private:
	std::string description_;
};

/**************************************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * This define allows easy access to throwing exceptions.
 */
#define raiseException(E)                                                                                \
  {						                                                                                 \
    std::ostringstream error;			                                                                 \
    error                                                                                                \
       << std::endl                                                                                      \
       << "================================================" << std::endl                                \
       << "ERROR" << std::endl                                                                           \
       << "in file " << __FILE__ << std::endl                                                            \
       << "near line " << __LINE__ << " with description:" << std::endl                                  \
       << std::endl                                                                                      \
       << E << std::endl							                                                     \
       << std::endl                                                                                      \
       << "If you suspect that this error is due to Geneva," << std::endl                                \
       << "then please consider filing a bug via" << std::endl                                           \
       << "http://www.gemfony.com (link \"Bug Reports\") or" << std::endl                                \
       << "through http://www.launchpad.net/geneva" << std::endl                                         \
       << std::endl                                                                                      \
       << "We appreciate your help!" << std::endl                                                        \
       << "The Geneva team" << std::endl                                                                 \
       << "================================================" << std::endl;                               \
    throw(Gem::Common::gemfony_error_condition(error.str()));                                            \
  }                                                                                                      \

/**************************************************************************************************/

#endif /* GEXCEPTIONS_HPP_ */
