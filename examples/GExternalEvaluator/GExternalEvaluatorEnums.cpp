/**
 * @file GExternalEvaluatorEnums.cpp
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

#include "GExternalEvaluatorEnums.hpp"

namespace Gem {

namespace Geneva {

/************************************************************************************************/
/**
 * Puts a Gem::Geneva::dataExchangeMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param exchMode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::dataExchangeMode& exchMode){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(exchMode);
	o << tmp;
	return o;
}

/************************************************************************************************/
/**
 * Reads a Gem::Geneva::dataExchangeMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param exchMode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::dataExchangeMode& exchMode){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	exchMode = boost::numeric_cast<Gem::Geneva::dataExchangeMode>(tmp);
#else
	exchMode = static_cast<Gem::Geneva::dataExchangeMode>(tmp);
#endif /* DEBUG */

	return i;
}

/************************************************************************************************/

} /* namespace Geneva */

} /* namespace Gem */
