/**
 * @file GParameterBaseWithAdaptorsT.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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

#include "GParameterBaseWithAdaptorsT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/////////////////// Specialization for std::vector<bool> ////////////////////////////////////////
/***********************************************************************************************/
/**
 * This function applies the first adaptor of the adaptor sequence to a collection of values.
 * Note that the parameter of this function will get changed. The check for adaptors happens
 * inside applyFirstAdaptor. This is a specialization of a generic template function.
 *
 * @param collection A vector of values that shall be mutated
 */
template<>
void GParameterBaseWithAdaptorsT<bool>::applyAdaptor(std::vector<bool>& collection) {
	std::vector<bool>::iterator it;
	for (it = collection.begin(); it != collection.end(); ++it)	{
		bool value = *it;
		applyAdaptor(value);
		*it = value;
	}
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
