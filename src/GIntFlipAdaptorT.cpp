/**
 * @file GIntFlipAdaptorT.cpp
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

#include "GIntFlipAdaptorT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
//////////////////////// Specialization for some types///////////////////////
/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(bool).
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<bool>::customMutations(bool& value) {
	value==true?value=false:value=true;
}


/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(char).
 *
 * @return The id of a GBooleanAdaptor
 */
template<> Gem::GenEvA::adaptorId GIntFlipAdaptorT<bool>::getAdaptorId() const {
	return Gem::GenEvA::GBOOLEANADAPTOR;
}

/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(boost::int32_t).
 *
 * @return The id of a GInt32FlipAdaptor
 */
template<> Gem::GenEvA::adaptorId GIntFlipAdaptorT<boost::int32_t>::getAdaptorId() const {
	return Gem::GenEvA::GINT32FLIPADAPTOR;
}

/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(boost::int32_t).
 *
 * @return The id of a GCharFlipAdaptor
 */
template<> Gem::GenEvA::adaptorId GIntFlipAdaptorT<char>::getAdaptorId() const {
	return Gem::GenEvA::GCHARFLIPADAPTOR;
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
