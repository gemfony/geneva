/**
 * @file GIntFlipAdaptorT.cpp
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

#include "geneva/GIntFlipAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/***********************************************************************************************/
//////////////////////// Specialization for some types///////////////////////
/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(bool).
 *
 * @param value The value to be adapted
 */
template<>
void GIntFlipAdaptorT<bool>::customAdaptions(bool& value) {
	value==true?value=false:value=true;
}


/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(char).
 *
 * @return The id of a GBooleanAdaptor
 */
template<> Gem::Geneva::adaptorId GIntFlipAdaptorT<bool>::getAdaptorId() const {
	return Gem::Geneva::GBOOLEANADAPTOR;
}

/***********************************************************************************************/
/**
 * Specialization for typeof(T) == typeof(boost::int32_t).
 *
 * @return The id of a GInt32FlipAdaptor
 */
template<> Gem::Geneva::adaptorId GIntFlipAdaptorT<boost::int32_t>::getAdaptorId() const {
	return Gem::Geneva::GINT32FLIPADAPTOR;
}

/***********************************************************************************************/

} /* namespace Geneva  */
} /* namespace Gem */
