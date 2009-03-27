/**
 * @file GBoolParameter.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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
 */

// Standard headers go here

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#ifndef GBOOLPARAMETER_HPP_
#define GBOOLPARAMETER_HPP_

// GenEvA headers go here

#include "GNumericParameterT.hpp"

namespace Gem {
namespace Util {

/*************************************************************************/
/**
 * A parameter type used for the communication with external programs. See
 * GNumericParameterT.hpp for further details.
 */
typedef GNumericParameterT<bool> GBoolParameter;

/** @brief Helper function to aid IO  of this parameter type */
std::ostream& operator<<(std::ostream&, const GBoolParameter&);
/** @brief Helper function to aid IO  of this parameter type */
std::istream& operator>>(std::istream&, GBoolParameter&);

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GBOOLPARAMETER_HPP_ */
