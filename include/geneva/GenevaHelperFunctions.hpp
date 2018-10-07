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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <sstream>
#include <type_traits>
#include <memory>
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** @brief Sets the processing flag in a given range */
G_API_GENEVA void setProcessingFlag(
	std::vector<std::shared_ptr<GParameterSet>>&
	, const std::tuple<std::size_t, std::size_t>&
);

/******************************************************************************/
/** @brief Transforms the individual fitness so that the optimization algorithm always "sees" a minimization problem */
G_API_GENEVA double minOnly_transformed_fitness(const std::shared_ptr<GParameterSet> &, std::size_t = 0);

/******************************************************************************/
/** @brief Checks whether the first individual is better than the second */
G_API_GENEVA bool isBetter(
	const std::shared_ptr<GParameterSet> x_ptr
	, const std::shared_ptr<GParameterSet> y_ptr
);

/******************************************************************************/
/** @brief Checks whether the first individual is worse than the second */
G_API_GENEVA bool isWorse(
	const std::shared_ptr<GParameterSet> x_ptr
	, const std::shared_ptr<GParameterSet> y_ptr
);

/******************************************************************************/
/** @brief Checks whether the first value is better than the second */
G_API_GENEVA bool isBetter(
	double x
	, double y
	, maxMode m
);

/******************************************************************************/
/** @brief Checks whether the first value is worse than the second*/
G_API_GENEVA bool isWorse(
	double x
	, double y
	, maxMode m
);

/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */

