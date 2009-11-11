/**
 * @file GFunctionIndividualDefines.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
