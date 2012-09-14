/**
 * @file GMathHelperFunctions.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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


// Standard headers go here
#include <cmath>
#include <cstdlib>

// Boost headers go here

#ifndef GMATHHELPERFUNCTIONS_HPP_
#define GMATHHELPERFUNCTIONS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"

namespace Gem {
namespace Common {

/************************************************************************************/

/** @brief Calculates the maximum value of two float values */
float GMax(const float&, const float&);
/** @brief Calculates the maximum value of two double values */
double GMax(const double&, const double&);
/** @brief Calculates the maximum value of two long double values */
long double GMax(const long double&, const long double&);

/** @brief Calculates the minimum value of two float values */
float GMin(const float&, const float&);
/** @brief Calculates the minimum value of two double values */
double GMin(const double&, const double&);
/** @brief Calculates the minimum value of two long double values */
long double GMin(const long double&, const long double&);

/** @brief Calculates the floor value of a float value */
float GFloor(const float&);
/** @brief Calculates the floor value of a double value */
double GFloor(const double&);
/** @brief Calculates the floor value of a long double value */
long double GFloor(const long double&);

/** @brief Calculates the fabs value of a float value */
float GFabs(const float&);
/** @brief Calculates the fabs value of a double value */
double GFabs(const double&);
/** @brief Calculates the fabs value of a long double value */
long double GFabs(const long double&);

/** @brief Calculates the abs value of an int value */
int GIabs(const int&);
/** @brief Calculates the abs value of a long int value */
long GIabs(const long&);

/** @brief Calculates the sqrt value of a float value */
float GSqrt(const float&);
/** @brief Calculates the sqrt value of a double value */
double GSqrt(const double&);
/** @brief Calculates the sqrt value of a long double value */
long double GSqrt(const long double&);

/** @brief Calculates the sin value of a float value */
float GSin(const float&);
/** @brief Calculates the sin value of a double value */
double GSin(const double&);
/** @brief Calculates the sin value of a long double value */
long double GSin(const long double&);

/** @brief Calculates the cos value of a float value */
float GCos(const float&);
/** @brief Calculates the cos value of a double value */
double GCos(const double&);
/** @brief Calculates the cos value of a long double value */
long double GCos(const long double&);

/** @brief Calculates the log value of a float value */
float GLog(const float&);
/** @brief Calculates the log value of a double value */
double GLog(const double&);

/** @brief Calculates the pow value of a float value */
float GPow(const float&, const float&);
/** @brief Calculates the pow value of a float value */
double GPow(const double&, const double&);
/** @brief Calculates the pow value of a float value */
long double GPow(const long double&, const long double&);

/** @brief Performs alpha blending for floats */
float GMix(const float&, const float&, const float&);
/** @brief Performs alpha blending for doubles */
double GMix(const double&, const double&, const double&);
/** @brief Performs alpha blending for long doubles */
long double GMix(const long double&, const long double&, const long double&);

#ifdef _GLIBCXX_HAVE_LOGL
/** @brief Calculates the log value of a long double value */
long double GLog(const long double&);
#endif /* _GLIBCXX_HAVE_LOGL */

/** @brief Calculates the exp value of a float value */
float GExp(const float&);
/** @brief Calculates the exp value of a double value */
double GExp(const double&);

#ifdef _GLIBCXX_HAVE_EXPL
/** @brief Calculates the exp value of a long double value */
long double GExp(const long double&);
#endif /* _GLIBCXX_HAVE_EXPL */

/** @brief Calculates the pow value of a float value */
float GPow(const float&, const float&);
/** @brief Calculates the pow value of a float value */
double GPow(const double&, const double&);
/** @brief Calculates the pow value of a float value */
#ifdef _GLIBCXX_HAVE_POWL
long double GPow(const long double&, const long double&);
#endif /* _GLIBCXX_HAVE_POWL */

/************************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GMATHHELPERFUNCTIONS_HPP_ */
