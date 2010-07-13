/**
 * @file GEnums.cpp
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

#include "GEnums.hpp"

namespace Gem
{
namespace Common
{

/*********************************************************************/
/**
 * Puts a Gem::Util::triboolStates item into a stream
 *
 * @param o The ostream the item should be added to
 * @param tbs the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Common::triboolStates& tbs){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(tbs);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::Util::triboolStates item from a stream
 *
 * @param i The stream the item should be read from
 * @param tbs The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Common::triboolStates& tbs){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	tbs = boost::numeric_cast<Gem::Common::triboolStates>(tmp);
#else
	tbs = static_cast<Gem::Common::triboolStates>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::Common::serializationMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param serMode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Common::serializationMode& serMode){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(serMode);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::Common::serializationMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param serMode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Common::serializationMode& serMode){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	serMode = boost::numeric_cast<Gem::Common::serializationMode>(tmp);
#else
	serMode = static_cast<Gem::Common::serializationMode>(tmp);
#endif  /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::Common::expectation item into a stream
 *
 * @param o The ostream the item should be added to
 * @param expect the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Common::expectation& expect){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(expect);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::Common::expectation item from a stream
 *
 * @param i The stream the item should be read from
 * @param expect The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Common::expectation& expect){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	expect = boost::numeric_cast<Gem::Common::expectation>(tmp);
#else
	expect = static_cast<Gem::Common::expectation>(tmp);
#endif  /* DEBUG */

	return i;
}

/*********************************************************************/

} /* namespace Common */
} /* namespace Gem */
