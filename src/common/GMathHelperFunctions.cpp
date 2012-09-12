/**
 * @file GMathHelperFunctions.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General
 * Public License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS DUAL-LICENSING OPTION DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * version 3 and of version 2 of the GNU General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GMathHelperFunctions.hpp"

namespace Gem {
namespace Common {

/************************************************************************************/
/**
 * Calculates the floor value of a float value
 *
 * @param x The value for which floor should be calculated
 * @return The floor value of x
 */
float GFloor(const float& x) {
	return floorf(x);
}

/************************************************************************************/
/**
 * Calculates the floor value of a double value
 *
 * @param x The value for which floor should be calculated
 * @return The floor value of x
 */
double GFloor(const double& x) {
	return floor(x);
}

/************************************************************************************/
/**
 * Calculates the floor value of a long double value
 *
 * @param x The value for which floor should be calculated
 * @return The floor value of x
 */
long double GFloor(const long double& x) {
	return floorl(x);
}

/************************************************************************************/
/**
 * Calculates the fabs value of a float value
 *
 * @param x The value for which fabs should be calculated
 * @return The fabs value of x
 */
float GFabs(const float& x) {
	return fabsf(x);
}

/************************************************************************************/
/**
 * Calculates the fabs value of a double value
 *
 * @param x The value for which fabs should be calculated
 * @return The fabs value of x
 */
double GFabs(const double& x) {
	return fabs(x);
}

/************************************************************************************/
/**
 * Calculates the fabs value of a long double value
 *
 * @param x The value for which fabs should be calculated
 * @return The fabs value of x
 */
long double GFabs(const long double& x) {
	return fabsl(x);
}

/************************************************************************************/
/**
 * Calculates the abs value of an int value
 *
 * @param i The value of which the absolute value should be calculated
 * @return The absolute value of i
 */
int GIabs(const int& i) {
  return abs(i);
}

/************************************************************************************/
/**
 * Calculates the abs value of a long int value
 *
 * @param i The value of which the absolute value should be calculated
 * @return The absolute value of i
 */
long GIabs(const long& i) {
  return labs(i);
}

/************************************************************************************/
/**
 * Calculates the sqrt value of a float value
 *
 * @param x The value for which sqrt should be calculated
 * @return The sqrt value of x
 */
float GSqrt(const float& x) {
	return sqrtf(x);
}

/************************************************************************************/
/**
 * Calculates the sqrt value of a double value
 *
 * @param x The value for which sqrt should be calculated
 * @return The sqrt value of x
 */
double GSqrt(const double& x) {
	return sqrt(x);
}

/************************************************************************************/
/**
 * Calculates the sqrt value of a long double value
 *
 * @param x The value for which sqrt should be calculated
 * @return The sqrt value of x
 */
long double GSqrt(const long double& x) {
	return sqrtl(x);
}

/************************************************************************************/
/**
 * Calculates the sin value of a float value
 *
 * @param x The value for which sin should be calculated
 * @return The sin value of x
 */
float GSin(const float& x) {
	return sinf(x);
}

/************************************************************************************/
/**
 * Calculates the sin value of a double value
 *
 * @param x The value for which sin should be calculated
 * @return The sin value of x
 */
double GSin(const double& x) {
	return sin(x);
}

/************************************************************************************/
/**
 * Calculates the sin value of a long double value
 *
 * @param x The value for which sin should be calculated
 * @return The sin value of x
 */
long double GSin(const long double& x) {
	return sinl(x);
}

/************************************************************************************/
/**
 * Calculates the cos value of a float value
 *
 * @param x The value for which cos should be calculated
 * @return The cos value of x
 */
float GCos(const float& x) {
	return cosf(x);
}

/************************************************************************************/
/**
 * Calculates the cos value of a double value
 *
 * @param x The value for which cos should be calculated
 * @return The cos value of x
 */
double GCos(const double& x) {
	return cos(x);
}

/************************************************************************************/
/**
 * Calculates the cos value of a long double value
 *
 * @param x The value for which cos should be calculated
 * @return The cos value of x
 */
long double GCos(const long double& x) {
	return cosl(x);
}

/************************************************************************************/
/**
 * Calculates the log value of a float value
 *
 * @param x The value for which log should be calculated
 * @return The log value of x
 */
float GLog(const float& x) {
	return logf(x);
}

/************************************************************************************/
/**
 * Calculates the log value of a double value
 *
 * @param x The value for which log should be calculated
 * @return The log value of x
 */
double GLog(const double& x) {
	return log(x);
}

/************************************************************************************/
/**
 * Calculates the log value of a long double value
 *
 * @param x The value for which log should be calculated
 * @return The log value of x
 */
#ifdef _GLIBCXX_HAVE_LOGL
long double GLog(const long double& x) {
	return logl(x);
}
#endif /* _GLIBCXX_HAVE_LOGL */

/************************************************************************************/
/**
 * Calculates the exp value of a float value
 *
 * @param x The value for which exp should be calculated
 * @return The exp value of x
 */
float GExp(const float& x) {
	return expf(x);
}

/************************************************************************************/
/**
 * Calculates the exp value of a double value
 *
 * @param x The value for which exp should be calculated
 * @return The exp value of x
 */
double GExp(const double& x) {
	return exp(x);
}

/************************************************************************************/
/**
 * Calculates the exp value of a long double value
 *
 * @param x The value for which exp should be calculated
 * @return The exp value of x
 */
#ifdef _GLIBCXX_HAVE_EXPL
long double GExp(const long double& x) {
	return expl(x);
}
#endif /* _GLIBCXX_HAVE_EXPL */

/************************************************************************************/
/**
 * Calculates the pow value of a float value
 */
float GPow(const float& x, const float& y) {
   return powf(x, y);
}

/************************************************************************************/
/**
 * Calculates the pow value of a double value
 */
double GPow(const double& x, const double& y) {
   return pow(x, y);
}

/************************************************************************************/
/**
 * Calculates the pow value of a long value
 */
long double GPow(const long double& x, const long double& y) {
   return powl(x, y);
}

/************************************************************************************/
/**
 * Performs alpha blending for floats
 */
float GMix(
      const float& bottom
      , const float& top
      , const float& alpha
) {
#ifdef DEBUG
   if(alpha < 0.f || alpha > 1.f) {
      raiseException(
            "In GMix<float>(): Error!" << std::endl
            << "alpha should be in the range [0.f,1.f], but has value " << alpha << std::endl
      );
   }
#endif

   return bottom*(1.f-alpha) + top*alpha;
}

/************************************************************************************/
/**
 * Performs alpha blending for doubles
 */
double GMix(
      const double& bottom
      , const double& top
      , const double& alpha
) {
#ifdef DEBUG
   if(alpha < 0. || alpha > 1.) {
      raiseException(
            "In GMix<double>(): Error!" << std::endl
            << "alpha should be in the range [0.,1.], but has value " << alpha << std::endl
      );
   }
#endif

   return bottom*(1.-alpha) + top*alpha;
}

/************************************************************************************/
/**
 * Performs alpha blending for long doubles
 */
long double GMix(
      const long double& bottom
      , const long double& top
      , const long double& alpha
) {
#ifdef DEBUG
   if(alpha < 0.l || alpha > 1.l) {
      raiseException(
            "In GMix<long double>(): Error!" << std::endl
            << "alpha should be in the range [0.l,1.l], but has value " << alpha << std::endl
      );
   }
#endif

   return bottom*(1.l-alpha) + top*alpha;
}

/************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
