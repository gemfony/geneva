/**
 * @file GHapEnums.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "hap/GHapEnums.hpp"

namespace Gem
{
namespace Hap
{

/******************************************************************************/
/**
 * Puts a Gem::Hap::gRandomTSpecialization item into a stream
 *
 * @param o The ostream the item should be added to
 * @param grts the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Hap::gRandomTSpecialization& grts){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(grts);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Hap::gRandomTSpecialization item from a stream
 *
 * @param i The stream the item should be read from
 * @param grts The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Hap::gRandomTSpecialization& grts){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	grts = boost::numeric_cast<Gem::Hap::gRandomTSpecialization>(tmp);
#else
	grts = static_cast<Gem::Hap::gRandomTSpecialization>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
