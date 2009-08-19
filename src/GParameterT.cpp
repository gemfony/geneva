/**
 * @file GParameterT.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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

#include "GParameterT.hpp"

#include "GDouble.hpp"

namespace Gem
{
namespace GenEvA
{

/***********************************************************************************************/
/**
  * A default constructor for typeof(T)==typeof(bool), needed as it seems useful to
  * initialize the value with a printable character.
  */
template <> GParameterT<char>::GParameterT()
   :GParameterBaseWithAdaptorsT<char>(),
     val_('a')
{ /* nothing */ }

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
