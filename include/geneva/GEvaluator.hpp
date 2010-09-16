/**
 * @file GEvaluator.hpp
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

// Standard header files go here

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

#ifndef GEVALUATOR_HPP_
#define GEVALUATOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "GObject.hpp"
#include "GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************************/
/**
 * This class can be assigned to a GEvaluatorIndividual object. The purpose of this
 * class is to centralize the definition of parameters and their evaluation in a single
 * place. The user derives a class from GEvaluator, in which he defines the parameter
 * types and possible boundary conditions. The GEvaluatorIndividual then uses that
 * information to set up the appropriate data structures. Upon evaluation, the user is
 * presented with vectors of the value types he has chosen, in the sequence he has defined
 * them.
 */
class GEvaluator :public GObject
{
public:
private:
};

/******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GEVALUATOR_HPP_ */
