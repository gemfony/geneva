/**
 * @file GNumCollectionT.cpp
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

#include "GNumCollectionT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
//////////////////////// Specialization for double and integer types ////////////////////////////
/***********************************************************************************************/
/**
 * Specialization for typeof(num_type) == typeof(double).
 *
 * @param nval Number of values to put into the vector
 * @param min The lower boundary for random entries
 * @param max The upper boundary for random entries
 */
template<>
void GNumCollectionT<double>::addRandomData(const std::size_t& nval,
										   const double& min,
										   const double& max)
{
	Gem::Util::GRandom gr;
	gr.setRnrGenerationMode(Gem::Util::RNRLOCAL);

	for(std::size_t i= 0; i<nval; i++) this->push_back(gr.evenRandom(min,max));
}

/***********************************************************************************************/
/**
 * Specialization for typeof(num_type) == typeof(boost::int32_t).
 *
 * @param nval Number of values to put into the vector
 * @param min The lower boundary for random entries
 * @param max The upper boundary for random entries
 */
template<>
void GNumCollectionT<boost::int32_t>::addRandomData(const std::size_t& nval,
		                                            const boost::int32_t& min,
		                                            const boost::int32_t& max)
{
	Gem::Util::GRandom gr;
	gr.setRnrGenerationMode(Gem::Util::RNRLOCAL);

	for(std::size_t i= 0; i<nval; i++) this->push_back(gr.discreteRandom(min,max));
}

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
