/**
 * @file MetaOptimizerIndividualT.cpp
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

#include "geneva-individuals/GMetaOptimizerIndividualT.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMetaOptimizerIndividualT<Gem::Geneva::GFunctionIndividual>)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Puts a Gem::Geneva::metaOptimizationTarget item into a stream
 *
 * @param o The ostream the item should be added to
 * @param mot the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::metaOptimizationTarget &mot) {
	std::uint16_t tmp = static_cast<std::uint16_t>(mot);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::metaOptimizationTarget item from a stream
 *
 * @param i The stream the item should be read from
 * @param mot The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::metaOptimizationTarget &mot) {
	std::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
   mot = boost::numeric_cast<Gem::Geneva::metaOptimizationTarget>(tmp);
#else
	mot = static_cast<Gem::Geneva::metaOptimizationTarget>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
