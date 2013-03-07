/**
 * @file GNumericParameterT.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "common/GNumericParameterT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(void)
    : param_(false)
    , lowerBoundary_(false)
    , upperBoundary_(true)
    , name_("none")
    , randomInit_(false)
{ /* nothing */ }

/******************************************************************************/

template <> void GNumericParameterT<bool>::setParameter(
      const bool& param
      , const bool&
      , const bool&
) {
   param_ = param;
   lowerBoundary_ = false;
   upperBoundary_ = true;
}

/******************************************************************************/
/**
 * Retrieves the parameter type name as a string. This is an overloaded version
 * for typeof(T) == double .
 */
template <> std::string GNumericParameterT<double>::getParameterType() const {
   return std::string("cd");
}

/******************************************************************************/
/**
 * Retrieves the parameter type name as a string. This is an overloaded version
 * for typeof(T) == boost::int32_t .
 */
template <> std::string GNumericParameterT<boost::int32_t>::getParameterType() const {
   return std::string("ci");
}

/******************************************************************************/
/**
 * Retrieves the parameter type name as a string. This is an overloaded version
 * for typeof(T) == bool .
 */
template <> std::string GNumericParameterT<bool>::getParameterType() const {
   return std::string("b");
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
