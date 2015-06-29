/**
 * @file GMathHelperFunctionsT.hpp
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
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <typeinfo>
#include <tuple>

// Boost headers go here
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/math/special_functions.hpp>
#include <boost/limits.hpp>
#include <boost/numeric/conversion/bounds.hpp>

#ifndef GMATHHELPERFUNCTIONST_HPP_
#define GMATHHELPERFUNCTIONST_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GMathHelperFunctions.hpp"

namespace Gem {
namespace Common {

const bool GWARNINGONLY = true;
const bool GERRORONLY = false;

/******************************************************************************/
/**
 * Enforces a value inside of a given range for the first parameter. Note that
 * the value of this parameter may change.
 *
 * @param val The value to be adapted
 * @param lower The lower boundary of the allowed value range
 * @param upper The upper boundary of the allowed value range
 */
template<typename fp_type>
void enforceRangeConstraint(
	fp_type &val, const fp_type &lower, const fp_type &upper,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
#ifdef DEBUG
   if(lower > upper) {
      glogger
      << "In enforceRangeConstraint<fp_type>(...): Error!" << std::endl
      << "Lower boundary > upper boundary: " << lower << " / " << upper << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	if (val < lower) {
		val = lower;
	} else if (val > upper) {
		val = upper;
	}
}

/******************************************************************************/
/**
 * Checks that a given value is inside of a given set of boundaries (both inclusive)
 *
 * @param val The value to be check
 * @param lower The lower boundary of the allowed value range
 * @param upper The upper boundary of the allowed value range
 */
template<typename fp_type>
bool checkRangeCompliance(
	const fp_type &val, const fp_type &lower, const fp_type &upper,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
#ifdef DEBUG
   if(lower > upper) {
      glogger
      << "In enforceRangeConstraint<fp_type>(...): Error!" << std::endl
      << "Lower boundary > upper boundary: " << lower << " / " << upper << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	if (val < lower || val > upper) {
		return false;
	} else {
		return true;
	}
}

/******************************************************************************/
/**
 * Retrieves the worst known value for a given floating point type, depending
 * on whether maximal or minimal values are considered to be better
 */
template<typename fp_type>
fp_type getWorstCase(
	bool maxMode, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	return (maxMode ? boost::numeric::bounds<fp_type>::lowest() : boost::numeric::bounds<fp_type>::highest());
}

/******************************************************************************/
/**
 * Retrieves the worst known value for a given floating point type, depending
 * on whether maximal or minimal values are considered to be better
 */
template<typename fp_type>
fp_type getBestCase(
	bool maxMode, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	return (maxMode ? boost::numeric::bounds<fp_type>::highest() : boost::numeric::bounds<fp_type>::lowest());
}

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
const bool GFWARNONLY = true;
const bool GFNOWARNING = false;

template<typename fp_type>
fp_type checkValueRange(
	fp_type val, fp_type min, fp_type max, bool lowerOpen = false, bool upperOpen = false, bool warnOnly = false,
	std::string varName = std::string(), typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	bool result = true;

	if (lowerOpen) {
		if (val < boost::math::float_next<fp_type>(min)) result = false;
	} else {
		if (val < min) result = false;
	}

	if (upperOpen) {
		if (val > boost::math::float_prior<fp_type>(max)) result = false;
	} else {
		if (val > max) result = false;
	}

	if (false == result) {
		if (warnOnly) {
			glogger
			<< "In checkValueRange<fp_type>(): Error!" << std::endl
			<< "Value " << val << (varName.empty() ? "" : (" of variable " + varName)) <<
			" outside of recommended range " << std::endl
			<< min << (lowerOpen ? " (open) - " : " (closed) - ") << max << (upperOpen ? " (open)" : " (closed)") <<
			std::endl
			<< GWARNING;
		} else {
			glogger
			<< "In checkValueRange<fp_type>(): Error!" << std::endl
			<< "Value " << val << (varName.empty() ? "" : (" of variable " + varName)) << " outside of allowed range " <<
			std::endl
			<< min << (lowerOpen ? " (open) - " : " (closed) - ") << max << (upperOpen ? " (open)" : " (closed)") <<
			std::endl
			<< GEXCEPTION;
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

template<typename int_type>
int_type checkValueRange(
	int_type val, int_type min, int_type max, bool lowerOpen = false, bool upperOpen = false, bool warnOnly = false,
	typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
) {
	bool result = true;

	if (lowerOpen) {
		if (val <= min) result = false;
	} else {
		if (val < min) result = false;
	}

	if (upperOpen) {
		if (val >= max) result = false;
	} else {
		if (val > max) result = false;
	}

	if (false == result) {
		if (warnOnly) {
			glogger
			<< "Warning:" << std::endl
			<< "In checkValueRange<int_type>(): Error!" << std::endl
			<< "Value " << val << " outside of recommended range " << std::endl
			<< min << (lowerOpen ? " (open) - " : " (closed) - ") << max << (upperOpen ? " (open)" : " (closed)") <<  std::endl
			<< GWARNING;
		} else {
			glogger
			<< "In checkValueRange<int_type>(): Error!" << std::endl
			<< "Value " << val << " outside of allowed range " << std::endl
			<< min << (lowerOpen ? " (open) - " : " (closed) - ") << max << (upperOpen ? " (open)" : " (closed)") <<
			std::endl
			<< GEXCEPTION;
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
 * @return A std::tuple holding the extreme values
 */
template<typename x_type_undet>
std::tuple<x_type_undet, x_type_undet> getMinMax(const std::vector<x_type_undet> &extDat) {
	// Do some error checking
	if (extDat.size() < (std::size_t) 2) {
		glogger
		<< "In GBasePlotter::getMinMax(1D): Error!" << std::endl
		<< "Got vector of invalid size " << extDat.size() << std::endl
		<< GEXCEPTION;
	}

	x_type_undet min = extDat.at(0), max = min;

	for (std::size_t i = 1; i < extDat.size(); i++) {
		if (extDat.at(i) < min) min = extDat.at(i);
		if (extDat.at(i) > max) max = extDat.at(i);
	}

	return std::tuple<x_type_undet, x_type_undet>(min, max);
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 2d-Tuples of undefined types.
 * This function requires that x_type_undet and y_type_undet can be compared using the
 * usual operators
 *
 * @param extDat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template<typename x_type_undet, typename y_type_undet>
std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>
getMinMax(const std::vector<std::tuple<x_type_undet, y_type_undet>> &extDat) {
	// Do some error checking
	if (extDat.size() < (std::size_t) 2) {
		glogger
		<< "In GBasePlotter::getMinMax(2D): Error!" << std::endl
		<< "Got vector of invalid size " << extDat.size() << std::endl
		<< GEXCEPTION;
	}

	x_type_undet minX = std::get<0>(extDat.at(0)), maxX = minX;
	y_type_undet minY = std::get<1>(extDat.at(0)), maxY = minY;

	for (std::size_t i = 1; i < extDat.size(); i++) {
		if (std::get<0>(extDat.at(i)) < minX) minX = std::get<0>(extDat.at(i));
		if (std::get<0>(extDat.at(i)) > maxX) maxX = std::get<0>(extDat.at(i));
		if (std::get<1>(extDat.at(i)) < minY) minY = std::get<1>(extDat.at(i));
		if (std::get<1>(extDat.at(i)) > maxY) maxY = std::get<1>(extDat.at(i));
	}

	return std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>(minX, maxX, minY, maxY);
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 3d-Tuples of undefined types.
 * This function requires that x_type_undet, y_type_undet and z_type_undet can be compared
 * using the usual operators
 *
 * @param extDat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template<typename x_type_undet, typename y_type_undet, typename z_type_undet>
std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet>
getMinMax(const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>> &extDat) {
	// Do some error checking
	if (extDat.size() < (std::size_t) 2) {
		glogger
		<< "In GBasePlotter::getMinMax(3D): Error!" << std::endl
		<< "Got vector of invalid size " << extDat.size() << std::endl
		<< GEXCEPTION;
	}

	x_type_undet minX = std::get<0>(extDat.at(0)), maxX = minX;
	y_type_undet minY = std::get<1>(extDat.at(0)), maxY = minY;
	z_type_undet minZ = std::get<2>(extDat.at(0)), maxZ = minZ;

	for (std::size_t i = 1; i < extDat.size(); i++) {
		if (std::get<0>(extDat.at(i)) < minX) minX = std::get<0>(extDat.at(i));
		if (std::get<0>(extDat.at(i)) > maxX) maxX = std::get<0>(extDat.at(i));
		if (std::get<1>(extDat.at(i)) < minY) minY = std::get<1>(extDat.at(i));
		if (std::get<1>(extDat.at(i)) > maxY) maxY = std::get<1>(extDat.at(i));
		if (std::get<2>(extDat.at(i)) < minZ) minZ = std::get<2>(extDat.at(i));
		if (std::get<2>(extDat.at(i)) > maxZ) maxZ = std::get<2>(extDat.at(i));
	}

	return std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet>(minX, maxX,
																																			  minY, maxY,
																																			  minZ, maxZ);
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 3d-Tuples of undefined types.
 * This function requires that x_type_undet, y_type_undet and z_type_undet can be compared
 * using the usual operators
 *
 * @param extDat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template<typename x_type_undet, typename y_type_undet, typename z_type_undet, typename w_type_undet>
std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet, w_type_undet, w_type_undet>
getMinMax(const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet>> &extDat) {
	// Do some error checking
	if (extDat.size() < (std::size_t) 2) {
		glogger
		<< "In GBasePlotter::getMinMax(3D): Error!" << std::endl
		<< "Got vector of invalid size " << extDat.size() << std::endl
		<< GEXCEPTION;
	}

	x_type_undet minX = std::get<0>(extDat.at(0)), maxX = minX;
	y_type_undet minY = std::get<1>(extDat.at(0)), maxY = minY;
	z_type_undet minZ = std::get<2>(extDat.at(0)), maxZ = minZ;
	w_type_undet minW = std::get<3>(extDat.at(0)), maxW = minW;

	for (std::size_t i = 1; i < extDat.size(); i++) {
		if (std::get<0>(extDat.at(i)) < minX) minX = std::get<0>(extDat.at(i));
		if (std::get<0>(extDat.at(i)) > maxX) maxX = std::get<0>(extDat.at(i));
		if (std::get<1>(extDat.at(i)) < minY) minY = std::get<1>(extDat.at(i));
		if (std::get<1>(extDat.at(i)) > maxY) maxY = std::get<1>(extDat.at(i));
		if (std::get<2>(extDat.at(i)) < minZ) minZ = std::get<2>(extDat.at(i));
		if (std::get<2>(extDat.at(i)) > maxZ) maxZ = std::get<2>(extDat.at(i));
		if (std::get<3>(extDat.at(i)) < minW) minW = std::get<3>(extDat.at(i));
		if (std::get<3>(extDat.at(i)) > maxW) maxW = std::get<3>(extDat.at(i));
	}

	return std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet, w_type_undet, w_type_undet>(
		minX, maxX, minY, maxY, minZ, maxZ, minW, maxW);
}

/******************************************************************************/
/**
 * Calculates the mean value from a std::vector of floating point values
 *
 * @param parVec The vector of values for which the mean should be calculated
 * @return The mean value of parVec
 */
template<typename T>
T GMean(
	const std::vector<T> &parVec, typename std::enable_if<std::is_floating_point<T>::value>::type *dummy = 0
) {
	T mean = 0.;

#ifdef DEBUG
   if(parVec.empty()) {
      glogger
      << "In T GMean(const std::vector<T>&): Error!" << std::endl
      << "parVec has size 0" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	typename std::vector<T>::const_iterator cit;
	for (cit = parVec.begin(); cit != parVec.end(); ++cit) {
		mean += *cit;
	}

	return mean / boost::numeric_cast<T>(parVec.size());
}


/******************************************************************************/
/**
 * Calculates the mean and standard deviation for a std::vector of floating point values
 *
 * @param parVec The vector of values for which the standard deviation should be calculated
 * @return A std::tuple holding the mean value and the standard deviation of the values stored in parVec
 */
template<typename T>
std::tuple<T, T> GStandardDeviation(
	const std::vector<T> &parVec, typename std::enable_if<std::is_floating_point<T>::value>::type *dummy = 0
) {
	T mean = GMean(parVec), sigma = 0.;

#ifdef DEBUG
   if(parVec.size() == 0) {
      glogger
      << "In std::tuple<T,T> GStandardDeviation(const std::vector<T>&): Error!" << std::endl
      << "parVec is empty" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	// It is easy if the size is 1
	if (parVec.size() == 1) {
		return std::tuple<T, T>(parVec.at(0), 0.);
	}

	typename std::vector<T>::const_iterator cit;
	for (cit = parVec.begin(); cit != parVec.end(); ++cit) {
		sigma += GSQUARED(*cit - mean);

	}
	sigma /= parVec.size() - 1;
	sigma = sqrt(sigma);

	return std::tuple<T, T>(mean, sigma);
}

/******************************************************************************/
/**
 * Calculates the mean and standard deviation for each row of a "matrix" made up from several
 * std:vector<T> objects of equal size. E.g., if you have 5 std::vector<double> of size 10, you will
 * get back a std::vector<std::tuple<double, double>>, of size 10, holding the mean and standard
 * deviation of the corresponding positions in the 5 vectors.
 *
 * @param parVec The vectors for which the standard deviations should be calculated
 * @param result A std::vector holdung tuples with the mean and sigma values for each row
 */
template<typename T>
void GVecStandardDeviation(
	const std::vector<std::vector<T>> &parVec, std::vector<std::tuple<T, T>> &result,
	typename std::enable_if<std::is_floating_point<T>::value>::type *dummy = 0
) {

#ifdef DEBUG
   // Check that there are entries in the vector
   if(parVec.size() == 0) {
      glogger
      << "In void GVecStandardDeviation(): Error!" << std::endl
      << "parVec is empty" << std::endl
      << GEXCEPTION;
   }

   // Check that the first entry has at least one component
   if(parVec.at(0).empty()) {
      glogger
      << "In void GVecStandardDeviation(): Error!" << std::endl
      << "parVec has empty component" << std::endl
      << GEXCEPTION;
   }

   // Check that all entries have the same size
   if(parVec.size() > 1) {
      std::size_t sizeFirst = parVec.at(0).size();
      for(std::size_t pos=1; pos<parVec.size(); pos++) {
         if(parVec.at(pos).size() != sizeFirst) {
            glogger
            << "In void GVecStandardDeviation(): Error!" << std::endl
            << "Found parVec component of different size: " << sizeFirst << " / " << " / " << pos << " / " << parVec.at(pos).size() << std::endl
            << GEXCEPTION;
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

	typename std::vector<std::vector<T>>::const_iterator cit;
	for (std::size_t pos = 0; pos < parVec.at(0).size(); pos++) {
		std::vector<T> indPar;

		// Extract the individual parameters
		for (cit = parVec.begin(); cit != parVec.end(); ++cit) {
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
struct PowSmallPosInt {
	enum {
		result = B * PowSmallPosInt<B, E - 1>::result
	};
};

template<std::size_t B>
struct PowSmallPosInt<B, 2> {
	enum {
		result = B * B
	};
};

template<std::size_t B>
struct PowSmallPosInt<B, 1> {
	enum {
		result = B
	};
};

template<std::size_t B>
struct PowSmallPosInt<B, 0> {
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
template<typename T>
void subtractVec(
	std::vector<T> &a, const std::vector<T> &b
) {
#ifdef DEBUG
   // Do some error checking
   if(a.size() != b.size()) {
      glogger
      << "In subtractVec(std::vector<T>, const std::vector<T>&): Error!" << std::endl
      << "Found invalid sizes: " << a.size() << " / " << b.size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	typename std::vector<T>::iterator it_a;
	typename std::vector<T>::const_iterator cit_b;

	// Subtract the elements
	for (it_a = a.begin(), cit_b = b.begin(); it_a != a.end(); ++it_a, ++cit_b) {
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
template<typename T>
void addVec(
	std::vector<T> &a, const std::vector<T> &b
) {
#ifdef DEBUG
   // Do some error checking
   if(a.size() != b.size()) {
      glogger
      << "In addVec(std::vector<T>, const std::vector<T>&): Error!" << std::endl
      << "Found invalid sizes: " << a.size() << " / " << b.size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	typename std::vector<T>::iterator it_a;
	typename std::vector<T>::const_iterator cit_b;

	// Subtract the elements
	for (it_a = a.begin(), cit_b = b.begin(); it_a != a.end(); ++it_a, ++cit_b) {
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
template<typename T>
void multVecConst(
	std::vector<T> &a, const T &c
) {
	typename std::vector<T>::iterator it_a;

	// Subtract the elements
	for (it_a = a.begin(); it_a != a.end(); ++it_a) {
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
template<typename T>
void assignVecConst(
	std::vector<T> &a, const T &c
) {
	typename std::vector<T>::iterator it_a;

	// Assign c to each element
	for (it_a = a.begin(); it_a != a.end(); ++it_a) {
		(*it_a) = c;
	}
}

/******************************************************************************/
/**
 * Summs up the x- and y-components individually of a vector of 2d-tuples
 */
template<typename fp_type>
std::tuple<fp_type, fp_type> sumTupleVec(
	const std::vector<std::tuple<fp_type, fp_type>> &dataPoints,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	std::tuple<fp_type, fp_type> result = std::tuple<fp_type, fp_type>(fp_type(0.), fp_type(0.));;

	typename std::vector<std::tuple<fp_type, fp_type>>::const_iterator cit;
	for (cit = dataPoints.begin(); cit != dataPoints.end(); ++cit) {
		std::get<0>(result) += std::get<0>(*cit);
		std::get<1>(result) += std::get<1>(*cit);
	}

	return result;
}

/******************************************************************************/
/**
 * Summs up the squares of x- and y-components individually of a vector of 2d-tuples
 */
template<typename fp_type>
std::tuple<fp_type, fp_type> squareSumTupleVec(
	const std::vector<std::tuple<fp_type, fp_type>> &dataPoints,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	std::tuple<fp_type, fp_type> result = std::tuple<fp_type, fp_type>(fp_type(0.), fp_type(0.));

	typename std::vector<std::tuple<fp_type, fp_type>>::const_iterator cit;
	for (cit = dataPoints.begin(); cit != dataPoints.end(); ++cit) {
		std::get<0>(result) += gpow(std::get<0>(*cit), 2.);
		std::get<1>(result) += gpow(std::get<1>(*cit), 2.);
	}

	return result;
}

/******************************************************************************/
/**
 * Summs up the product of x- and y-components of a vector of 2d-tuples
 */
template<typename fp_type>
fp_type productSumTupleVec(
	const std::vector<std::tuple<fp_type, fp_type>> &dataPoints,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	fp_type result = fp_type(0.);

	typename std::vector<std::tuple<fp_type, fp_type>>::const_iterator cit;
	for (cit = dataPoints.begin(); cit != dataPoints.end(); ++cit) {
		result += std::get<0>(*cit) * std::get<1>(*cit);
	}

	return result;
}

/******************************************************************************/
/**
 * Calculates the "square deviation" of a set of floating point tuples from
 * a line defined through a + b*x .
 *
 * @param dataPoints A vector of bi-tuples with x-y data points
 * @param a The offset of a line
 * @param b The slope of a line
 * @return The square deviation of the data points from the line
 */
template<typename fp_type>
fp_type squareDeviation(
	const std::vector<std::tuple<fp_type, fp_type>> &dataPoints, const fp_type &a, const fp_type &b,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	fp_type result = fp_type(0);
	typename std::vector<std::tuple<fp_type, fp_type>>::const_iterator cit;
	for (cit = dataPoints.begin(); cit != dataPoints.end(); ++cit) {
		result += gpow(std::get<1>(*cit) - a - b * std::get<0>(*cit), 2.);
	}
	return result;
}

/******************************************************************************/
/**
 * Calculates the parameters a and b of a regression line, plus errors. The return
 * value is a std::tuple of four fp_type values: a, error_a, b, error_b, with the
 * line being defined by L(x)=a+b*x .
 *
 * @param dataPoints A vector of data points to which the lines parameters should fit
 * @return Regression parameters for a line defined by the input data points
 */
template<typename fp_type>
std::tuple<fp_type, fp_type, fp_type, fp_type> getRegressionParameters(
	const std::vector<std::tuple<fp_type, fp_type>> &dataPoints,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	if (dataPoints.empty()) {
		return std::tuple<fp_type, fp_type, fp_type, fp_type>(fp_type(0.), fp_type(0.), fp_type(0.), fp_type(0.));
	}

	fp_type a = fp_type(0), b = fp_type(0);
	fp_type n = fp_type(dataPoints.size());

	std::tuple<fp_type, fp_type> sum_xy = sumTupleVec(dataPoints);
	fp_type sum_x = std::get<0>(sum_xy);
	fp_type sum_y = std::get<1>(sum_xy);

	std::tuple<fp_type, fp_type> sq_sum_xy = squareSumTupleVec(dataPoints);
	fp_type sq_sum_x = std::get<0>(sq_sum_xy);
	fp_type sq_sum_y = std::get<1>(sq_sum_xy);

	fp_type prod_sum_xy = productSumTupleVec(dataPoints);

	a = (sum_y * sq_sum_x - sum_x * prod_sum_xy) / (n * sq_sum_x - gpow(sum_x, 2.));
	b = (n * prod_sum_xy - sum_x * sum_y) / (n * sq_sum_x - gpow(sum_x, 2.));

	fp_type dev = squareDeviation(dataPoints, a, b);

	fp_type sigma_a = gsqrt(dev / (n - 2.)) * gsqrt(sq_sum_x / (n * sq_sum_x - gpow(sum_x, 2.)));
	fp_type sigma_b = gsqrt(dev / (n - 2.)) * gsqrt(n / (n * sq_sum_x - gpow(sum_x, 2.)));

	return std::tuple<fp_type, fp_type, fp_type, fp_type>(a, sigma_a, b, sigma_b);
}

/******************************************************************************/
/**
 * Calculates the error of a function f=s/p , where s and p are independent
 * quantities, each with its own error. Returns:
 * - The sleep time
 * - The error on the sleep-time (always 0)
 * - the quantity s/p
 * - error on s/p
 * s and p have the same structure
 */
template<typename fp_type>
std::tuple<fp_type, fp_type, fp_type, fp_type> getRatioError(
	const std::tuple<fp_type, fp_type, fp_type, fp_type> &s, const std::tuple<fp_type, fp_type, fp_type, fp_type> &p,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	// p may not ne 0
	if (0. == std::get<2>(p)) {
		glogger
		<< "In getRatioError(): Error!" << std::endl
		<< "Attempted division by 0." << std::endl
		<< GEXCEPTION;
	}

	fp_type sleep_time = std::get<0>(s);

	// Check that the sleep-times for s and p are the same
	if (sleep_time != std::get<0>(p)) {
		glogger
		<< "In getRatioError(): Error!" << std::endl
		<< "Sleep times differ: " << sleep_time << " / " << std::get<0>(p) << std::endl
		<< GEXCEPTION;
	}

	fp_type s_val = std::get<2>(s);
	fp_type s_err = std::get<3>(s);
	fp_type p_val = std::get<2>(p);
	fp_type p_err = std::get<3>(p);

	return std::tuple<fp_type, fp_type, fp_type, fp_type>(
		sleep_time, 0., s_val / p_val,
		gsqrt(gpow(s_err / p_val, fp_type(2.)) + gpow(s_val * p_err / gpow(p_val, fp_type(2.)), fp_type(2.)))
	);
}

/******************************************************************************/
/**
 * Calculates the error for a function f=s/p , where s and p are independent
 * quantities, each with its own error. The function is applied to a std::vector
 * of different s and p (together with their errors). It returns a vector
 * of s/p together with their errors.
 */
template<typename fp_type>
std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> getRatioErrors(
	const std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> &sn,
	const std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> &pn,
	typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
) {
	// Check that both vectors have the same size, otherwise complain
	if (sn.size() != pn.size()) {
		glogger
		<< "In getRatioErrors(): Error!" << std::endl
		<< "Vectors have invalid sizes: " << sn.size() << " / " << pn.size() << std::endl
		<< GEXCEPTION;
	}

	std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> spn;
	typename std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>>::const_iterator s_cit, p_cit;
	for (s_cit = sn.begin(), p_cit = pn.begin(); s_cit != sn.end(); ++s_cit, ++p_cit) {
		spn.push_back(getRatioError(*s_cit, *p_cit));
	}

	return spn;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GMATHHELPERFUNCTIONST_HPP_ */
