/**
 * @file GParameterT.cpp
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

#include "GParameterT.hpp"

namespace Gem
{
namespace GenEvA
{

/***********************************************************************************************/
/**
  * A default constructor for typeof(T)==typeof(char), needed as it seems useful to
  * initialize the value with a printable character.
  */
template <> GParameterT<char>::GParameterT()
   :GParameterBaseWithAdaptorsT<char>(),
     val_('a')
{ /* nothing */ }

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
