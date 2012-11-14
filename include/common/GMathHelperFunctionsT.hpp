/**
 * @file GMathHelperFunctionsT.hpp
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
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits.hpp>

#ifndef GMATHHELPERFUNCTIONST_HPP_
#define GMATHHELPERFUNCTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

namespace Gem {
namespace Common {

const bool GWARNINGONLY = true;
const bool GERRORONLY = false;

/******************************************************************************/
/**
 * Checks that a floating point value is contained in a given range
 *
 * @param val The value to be checked for containment
 * @param min The lower boundary (included)
 * @param max The upper boundary (possibly included)
 * @param lowerOpen Determines whether the lower boundary must be smaller or may be equal to val (default: closed)
 * @param upperOpen Determines whether the upper boundary must be larger or may be equal to val (default: closed)
 * @param warnOnly Will warn only if the condition isn't met
 * @return The value being checked
 */
const bool GFPLOWERCLOSED = false;
const bool GFPLOWEROPEN = true;
const bool GFPUPPERCLOSED = false;
const bool GFPUPPEROPEN = true;

template <typename fp_type>
fp_type checkValueRange(
   fp_type val
   , fp_type min
   , fp_type max
   , bool lowerOpen = false
   , bool upperOpen = false
   , bool warnOnly = false
   , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
) {
   bool result = true;

   if(lowerOpen) {
      if(val < boost::math::float_next<fp_type>(min)) result=false;
   } else {
      if(val < min) result=false;
   }

   if(upperOpen) {
      if(val > boost::math::float_prior<fp_type>(max)) result=false;
   } else {
      if(val > max) result=false;
   }

   if(false==result) {
      if(warnOnly) {
         std::cerr << "Warning:" << std::endl
               << "In checkValueRange<fp_type>(): Error!" << std::endl
               << "Value " << val << " outside of recommended range " << std::endl
               << min << (lowerOpen?" (open) - ":" (closed) - ") << max << (upperOpen?" (open)":" (closed)") << std::endl;
      } else {
         raiseException(
               "In checkValueRange<fp_type>(): Error!" << std::endl
               << "Value " << val << " outside of allowed range " << std::endl
               << min << (lowerOpen?" (open) - ":" (closed) - ") << max << (upperOpen?" (open)":" (closed)") << std::endl
         );
      }
   }

   return val;
}

/******************************************************************************/
/**
 * Checks that an integral value is contained in a given range
 *
 * @param val The value to be checked for containment
 * @param min The lower boundary (included)
 * @param max The upper boundary (possibly included)
 * @param lowerOpen Determines whether the lower boundary must be smaller or may be equal to val (default: closed)
 * @param upperOpen Determines whether the upper boundary must be larger or may be equal to val (default: closed)
 * @param warnOnly Will warn only if the condition isn't met
 * @return The value being checked
 */
const bool GINTLOWERCLOSED = false;
const bool GINTLOWEROPEN = true;
const bool GINTUPPERCLOSED = false;
const bool GINTUPPEROPEN = true;

template <typename int_type>
int_type checkValueRange(
   int_type val
   , int_type min
   , int_type max
   , bool lowerOpen = false
   , bool upperOpen = false
   , bool warnOnly = false
   , typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
) {
   bool result = true;

   if(lowerOpen) {
      if(val <= min) result=false;
   } else {
      if(val < min) result=false;
   }

   if(upperOpen) {
      if(val >= max) result=false;
   } else {
      if(val > max) result=false;
   }

   if(false==result) {
      if(warnOnly) {
         std::cerr << "Warning:" << std::endl
               << "In checkValueRange<int_type>(): Error!" << std::endl
               << "Value " << val << " outside of recommended range " << std::endl
               << min << (lowerOpen?" (open) - ":" (closed) - ") << max << (upperOpen?" (open)":" (closed)") << std::endl;
      } else {
         raiseException(
               "In checkValueRange<int_type>(): Error!" << std::endl
               << "Value " << val << " outside of allowed range " << std::endl
               << min << (lowerOpen?" (open) - ":" (closed) - ") << max << (upperOpen?" (open)":" (closed)") << std::endl
         );
      }
   }

   return val;
}

/******************************************************************************/
/**
 * Finds the minimum and maximum component in a vector of undefined types. This
 * function requires that x_type_undet can be compared using the usual operators.
 *
 * @param extDat The vector holding the data, for which extreme values should be calculated
 * @return A boost::tuple holding the extreme values
 */
template <typename x_type_undet>
boost::tuple<x_type_undet, x_type_undet> getMinMax(const std::vector<x_type_undet>& extDat) {
   // Do some error checking
   if(extDat.size() < (std::size_t)2) {
      raiseException(
            "In GBasePlotter::getMinMax(1D): Error!" << std::endl
            << "Got vector of invalid size " << extDat.size() << std::endl
      );
   }

   x_type_undet min=extDat.at(0), max=min;

   for(std::size_t i=1; i<extDat.size(); i++) {
      if(extDat.at(i) < min) min = extDat.at(i);
      if(extDat.at(i) > max) max = extDat.at(i);
   }

   return boost::tuple<x_type_undet, x_type_undet>(min, max);
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 2d-Tuples of undefined types.
 * This function requires that x_type_undet and y_type_undet can be compared using the
 * usual operators
 *
 * @param extDat The vector holding the data, for which extreme values should be calculated
 * @return A boost::tuple holding the extreme values
 */
template <typename x_type_undet, typename y_type_undet>
boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>
getMinMax(const std::vector<boost::tuple<x_type_undet, y_type_undet> >& extDat) {
   // Do some error checking
   if(extDat.size() < (std::size_t)2) {
      raiseException(
            "In GBasePlotter::getMinMax(2D): Error!" << std::endl
            << "Got vector of invalid size " << extDat.size() << std::endl
      );
   }

   x_type_undet minX=boost::get<0>(extDat.at(0)), maxX = minX;
   y_type_undet minY=boost::get<1>(extDat.at(0)), maxY = minY;

   for(std::size_t i=1; i<extDat.size(); i++) {
      if(boost::get<0>(extDat.at(i)) < minX) minX = boost::get<0>(extDat.at(i));
      if(boost::get<0>(extDat.at(i)) > maxX) maxX = boost::get<0>(extDat.at(i));
      if(boost::get<1>(extDat.at(i)) < minY) minY = boost::get<1>(extDat.at(i));
      if(boost::get<1>(extDat.at(i)) > maxY) maxY = boost::get<1>(extDat.at(i));
   }

   return boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>(minX, maxX, minY, maxY);
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 3d-Tuples of undefined types.
 * This function requires that x_type_undet, y_type_undet and z_type_undet can be compared
 * using the usual operators
 *
 * @param extDat The vector holding the data, for which extreme values should be calculated
 * @return A boost::tuple holding the extreme values
 */
template <typename x_type_undet, typename y_type_undet, typename z_type_undet>
boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet>
getMinMax(const std::vector<boost::tuple<x_type_undet, y_type_undet, z_type_undet> >& extDat) {
   // Do some error checking
   if(extDat.size() < (std::size_t)2) {
      raiseException(
            "In GBasePlotter::getMinMax(3D): Error!" << std::endl
            << "Got vector of invalid size " << extDat.size() << std::endl
      );
   }

   x_type_undet minX=boost::get<0>(extDat.at(0)), maxX = minX;
   y_type_undet minY=boost::get<1>(extDat.at(0)), maxY = minY;
   z_type_undet minZ=boost::get<2>(extDat.at(0)), maxZ = minZ;

   for(std::size_t i=1; i<extDat.size(); i++) {
      if(boost::get<0>(extDat.at(i)) < minX) minX = boost::get<0>(extDat.at(i));
      if(boost::get<0>(extDat.at(i)) > maxX) maxX = boost::get<0>(extDat.at(i));
      if(boost::get<1>(extDat.at(i)) < minY) minY = boost::get<1>(extDat.at(i));
      if(boost::get<1>(extDat.at(i)) > maxY) maxY = boost::get<1>(extDat.at(i));
      if(boost::get<2>(extDat.at(i)) < minZ) minZ = boost::get<2>(extDat.at(i));
      if(boost::get<2>(extDat.at(i)) > maxZ) maxZ = boost::get<2>(extDat.at(i));
   }

   return boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet>(minX, maxX, minY, maxY, minZ, maxZ);
}

/******************************************************************************/
/**
 * Calculates the mean value from a std::vector of floating point values
 *
 * @param parVec The vector of values for which the mean should be calculated
 * @return The mean value of parVec
 */
template <typename T>
T GMean(
   const std::vector<T>& parVec
   , typename boost::enable_if<boost::is_floating_point<T> >::type* dummy = 0
) {
   T mean = 0.;

#ifdef DEBUG
   if(parVec.empty()) {
      raiseException(
         "In T GMean(const std::vector<T>&): Error!" << std::endl
         << "parVec has size 0" << std::endl
      );
   }
#endif /* DEBUG */

   typename std::vector<T>::const_iterator cit;
   for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
      mean += *cit;
   }

   return mean/boost::numeric_cast<T>(parVec.size());
}


/******************************************************************************/
/**
 * Calculates the mean and standard deviation for a std::vector of floating point values
 *
 * @param parVec The vector of values for which the standard deviation should be calculated
 * @return A boost::tuple holding the mean value and the standard deviation of the values stored in parVec
 */
template <typename T>
boost::tuple<T,T> GStandardDeviation(
   const std::vector<T>& parVec
   , typename boost::enable_if<boost::is_floating_point<T> >::type* dummy = 0
) {
   T mean = GMean(parVec), sigma = 0.;

#ifdef DEBUG
   if(parVec.size() == 0) {
      raiseException(
         "In boost::tuple<T,T> GStandardDeviation(const std::vector<T>&): Error!" << std::endl
         << "parVec is empty" << std::endl
      );
   }
#endif /* DEBUG */

   // It is easy if the size is 1
   if(parVec.size() == 1) {
      return boost::tuple<T,T>(parVec.at(0), 0.);
   }

   typename std::vector<T>::const_iterator cit;
   for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
      sigma += GSQUARED(*cit - mean);

   }
   sigma /= parVec.size() - 1;
   sigma = sqrt(sigma);

   return boost::tuple<T,T>(mean, sigma);
}

/******************************************************************************/
/**
 * Calculates the mean and standard deviation for each row of a "matrix" made up from several
 * std:vector<T> objects of equal size. E.g., if you have 5 std::vector<double> of size 10, you will
 * get back a std::vector<boost::tuple<double, double> >, of size 10, holding the mean and standard
 * deviation of the corresponding positions in the 5 vectors.
 *
 * @param parVec The vectors for which the standard deviations should be calculated
 * @param result A std::vector holdung tuples with the mean and sigma values for each row
 */
template <typename T>
void GVecStandardDeviation(
   const std::vector<std::vector<T> > & parVec
   , std::vector<boost::tuple<T,T> > & result
   , typename boost::enable_if<boost::is_floating_point<T> >::type* dummy = 0
) {

#ifdef DEBUG
   // Check that there are entries in the vector
   if(parVec.size() == 0) {
      raiseException(
         "In void GVecStandardDeviation(): Error!" << std::endl
         << "parVec is empty" << std::endl
      );
   }

   // Check that the first entry has at least one component
   if(parVec.at(0).empty()) {
      raiseException(
         "In void GVecStandardDeviation(): Error!" << std::endl
         << "parVec has empty component" << std::endl
      );
   }

   // Check that all entries have the same size
   if(parVec.size() > 1) {
      std::size_t sizeFirst = parVec.at(0).size();
      for(std::size_t pos=1; pos<parVec.size(); pos++) {
         if(parVec.at(pos).size() != sizeFirst) {
            raiseException(
               "In void GVecStandardDeviation(): Error!" << std::endl
               << "Found parVec component of different size: " << sizeFirst << " / " << " / " << pos << " / " << parVec.at(pos).size() << std::endl
            );
         }
      }
   }

   // Now we know that there is at least one component with at least one entry,
   // and that all components have the same size. This is sufficient to calculate
   // the mean and standard deviation for each row of the "matrix" (made up from
   // the vectors,
#endif /* DEBUG */

   // Make sure our result vector is empty
   result.clear();

   typename std::vector<std::vector<T> >::const_iterator cit;
   for(std::size_t pos=0; pos<parVec.at(0).size(); pos++) {
      std::vector<T> indPar;

      // Extract the individual parameters
      for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
         indPar.push_back(cit->at(pos));
      }

      // Calculate the mean and standard deviation
      result.push_back(GStandardDeviation(indPar));
   }
}

/******************************************************************************/
/**
 * Calculation of pow for small positive integers using template metaprogramming
 */
template<std::size_t B, std::size_t E>
struct PowSmallPosInt
{
    enum {
       result = B*PowSmallPosInt<B,E-1>::result
    };
};

template<std::size_t B>
struct PowSmallPosInt<B,2>
{
    enum {
       result = B*B
    };
};

template<std::size_t B>
struct PowSmallPosInt<B,1>
{
    enum {
       result = B
    };
};

template<std::size_t B>
struct PowSmallPosInt<B,0>
{
    enum {
       result = 1
    };
};

/******************************************************************************/
/**
 * Takes two std::vector<> and subtracts each position of the second vector from the
 * corresponding position of the first vector. Note that we assume here that T understands
 * the operator-= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector from whose elements numbers will be subtracted
 * @param b The vector whose elements will be subtracted from the elements of a
 */
template <typename T>
void subtractVec (
      std::vector<T>& a
      , const std::vector<T>& b
) {
#ifdef DEBUG
   // Do some error checking
   if(a.size() != b.size()) {
      raiseException(
            "In subtractVec(std::vector<T>, const std::vector<T>&): Error!" << std::endl
            << "Found invalid sizes: " << a.size() << " / " << b.size() << std::endl
      );
   }
#endif /* DEBUG */

   typename std::vector<T>::iterator it_a;
   typename std::vector<T>::const_iterator cit_b;

   // Subtract the elements
   for(it_a=a.begin(), cit_b=b.begin(); it_a != a.end(); ++it_a, ++cit_b) {
      (*it_a) -= (*cit_b);
   }
}

/******************************************************************************/
/**
 * Takes two std::vector<> and adds each position of the second vector to the
 * corresponding position of the first vector. Note that we assume here that T understands
 * the operator+= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector to whose elements numbers will be added
 * @param b The vector whose elements will be added to the elements of a
 */
template <typename T>
void addVec (
      std::vector<T>& a
      , const std::vector<T>& b
) {
#ifdef DEBUG
   // Do some error checking
   if(a.size() != b.size()) {
      raiseException(
            "In addVec(std::vector<T>, const std::vector<T>&): Error!" << std::endl
            << "Found invalid sizes: " << a.size() << " / " << b.size() << std::endl
      );
   }
#endif /* DEBUG */

   typename std::vector<T>::iterator it_a;
   typename std::vector<T>::const_iterator cit_b;

   // Subtract the elements
   for(it_a=a.begin(), cit_b=b.begin(); it_a != a.end(); ++it_a, ++cit_b) {
      (*it_a) += (*cit_b);
   }
}

/******************************************************************************/
/**
 * Multiplies each position of a std::vector<> with a constant. Note that we assume here that
 * T understands the operator*= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector to whose elements numbers will be added
 * @param c The constant which will be multiplied with each position of a
 */
template <typename T>
void multVecConst (
      std::vector<T>& a
      , const T& c
) {
   typename std::vector<T>::iterator it_a;

   // Subtract the elements
   for(it_a=a.begin(); it_a != a.end(); ++it_a) {
      (*it_a) *= c;
   }
}

/******************************************************************************/
/**
 * Assigns a constant value to each position of the vector. Note that we assume here that
 * T understands the operator*= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector to whose elements c will be assigned
 * @param c The constant which will be assigned each position of a
 */
template <typename T>
void assignVecConst (
      std::vector<T>& a
      , const T& c
) {
   typename std::vector<T>::iterator it_a;

   // Assign c to each element
   for(it_a=a.begin(); it_a != a.end(); ++it_a) {
      (*it_a) = c;
   }
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GMATHHELPERFUNCTIONST_HPP_ */
