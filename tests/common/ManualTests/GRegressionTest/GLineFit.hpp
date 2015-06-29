/**
 * @file GLineFit.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <algorithm>

// Boost headers go here

#ifndef GLINEFIT_HPP_
#define GLINEFIT_HPP_

// Geneva headers go here
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva-individuals/GLineFitIndividual.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** @brief This function fits a line to a set of x-y coordinates */
G_API_GENEVA std::tuple<double, double> gLineFit(const std::vector<std::tuple<double, double>>&);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


#endif /* GLINEFIT_HPP_ */
