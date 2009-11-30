/**
 * @file GFunctionIndividualDefines.hpp
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

#ifndef GFUNCTIONINDIVIDUALDEFINES_HPP_
#define GFUNCTIONINDIVIDUALDEFINES_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum demoFunction {
					PARABOLA=0,
					NOISYPARABOLA=1,
					ROSENBROCK=2
};

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GFUNCTIONINDIVIDUALDEFINES_HPP_ */
