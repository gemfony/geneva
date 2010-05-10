/**
 * @file GLongParameter.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GLONGPARAMETER_HPP_
#define GLONGPARAMETER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here

#include "GNumericParameterT.hpp"

namespace Gem {
namespace Util {

/*************************************************************************/
/**
 * A parameter type used for the communication with external programs. See
 * GNumericParameterT.hpp for further details.
 */
typedef GNumericParameterT<boost::int32_t> GLongParameter;

/** @brief Helper function to aid IO  of this parameter type */
std::ostream& operator<<(std::ostream&, const GLongParameter&);
/** @brief Helper function to aid IO  of this parameter type */
std::istream& operator>>(std::istream&, GLongParameter&);

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GLONGPARAMETER_HPP_ */
