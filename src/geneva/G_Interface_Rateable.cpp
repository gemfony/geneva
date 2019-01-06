/**
 * @file
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

#include "geneva/G_Interface_Rateable.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Retrieves the stored raw fitness with a given id
 */
double G_Interface_Rateable::raw_fitness(std::size_t pos) const {
    return raw_fitness_(pos);
}

/******************************************************************************/
/**
 * Retrieves the stored transformed fitness with a given id
 */
double G_Interface_Rateable::transformed_fitness(std::size_t pos) const {
    return transformed_fitness_(pos);
}

/******************************************************************************/
/**
 * Returns all raw fitness results in a std::vector
 */
std::vector<double> G_Interface_Rateable::raw_fitness_vec() const {
    return raw_fitness_vec_();
}

/******************************************************************************/
/**
 * Returns all transformed fitness results in a std::vector
 */
std::vector<double> G_Interface_Rateable::transformed_fitness_vec() const {
    return transformed_fitness_vec_();
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
