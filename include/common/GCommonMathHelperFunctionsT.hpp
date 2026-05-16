/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * Enforces a value inside of a given range (both boundaries inclusive) for the
 * first parameter. Note that the value of this parameter may change.
 *
 * @param val The value to be adapted
 * @param lower The lower boundary of the allowed value range
 * @param upper The upper (inclusive) boundary of the allowed value range
 */
template <std::floating_point fp_type>
fp_type enforceRangeConstraint(
    fp_type &val,
    const fp_type &lower,
    const fp_type &upper,
    const std::string &caller = "empty",
    bool verbose = false
) {
    if(lower > upper) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << (caller == "empty" ? "" : ("[" + caller + "] "))
            << "In enforceRangeConstraint<fp_type>(): Error!" << '\n'
            << "Lower boundary > upper boundary: " << lower << " / " << upper << '\n'
        );
    }

    if(val < lower) {
        if(verbose) {
            glogger << (caller == "empty" ? "" : ("[" + caller + "] "))
                    << "In Gem::Common::enforceRangeConstraint(): " << '\n'
                    << "value " << val << " < lower boundary " << lower << '\n'
                    << "Will be adapted to " << lower << '\n'
                    << GWARNING;
        }
        val = lower;
    }
    else if(val > upper) {
        if(verbose) {
            glogger << (caller == "empty" ? "" : ("[" + caller + "] "))
                    << "In Gem::Common::enforceRangeConstraint(): " << '\n'
                    << "value " << val << " > upper boundary " << upper << '\n'
                    << "Will be adapted to " << upper << '\n'
                    << GWARNING;
        }
        val = upper;
    }

    return val;
}

/******************************************************************************/
/**
 * Checks that a given floating point value is inside of a given set of boundaries (both inclusive)
 *
 * @param val The value to be check
 * @param lower The lower boundary of the allowed value range
 * @param upper The upper boundary of the allowed value range
 */
template <std::floating_point fp_type>
bool checkRangeCompliance(
    const fp_type &val,
    const fp_type &lower,
    const fp_type &upper,
    const std::string &caller = "empty"
) {
    if(lower > upper) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << (caller == "empty" ? "" : ("[" + caller + "] "))
            << "In checkRangeCompliance<fp_type>(...): Error!" << '\n'
            << "Lower boundary > upper boundary: " << lower << " / " << upper << '\n'
        );
    }

    return not(val < lower || val > upper);
}

/******************************************************************************/
/**
 * Checks that a given floating point value is inside of a given set of boundaries (both inclusive)
 *
 * @param val The value to be check
 * @param lower The lower boundary of the allowed value range
 * @param upper The upper boundary of the allowed value range
 */
template <std::integral int_type>
bool checkRangeCompliance(
    const int_type &val,
    const int_type &lower,
    const int_type &upper,
    const std::string &caller = "empty"
) {
    if(lower > upper) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << (caller == "empty" ? "" : ("[" + caller + "] "))
            << "In checkRangeCompliance<int_type>(...): Error!" << '\n'
            << "Lower boundary > upper boundary: " << lower << " / " << upper << '\n'
        );
    }

    return not(val < lower || val > upper);
}

/******************************************************************************/
/**
 * Retrieves the worst known value for a given floating point type, depending
 * on whether maximal or minimal values are considered to be better
 */
template <std::floating_point fp_type>
fp_type getWorstCase(bool max_mode) {
    return (
        max_mode ? std::numeric_limits<fp_type>::lowest() : std::numeric_limits<fp_type>::max()
    );
}

/******************************************************************************/
/**
 * Retrieves the best known value for a given floating point type, depending
 * on whether maximal or minimal values are considered to be better
 */
template <std::floating_point fp_type>
fp_type getBestCase(bool max_mode) {
    return (
        max_mode ? std::numeric_limits<fp_type>::max() : std::numeric_limits<fp_type>::lowest()
    );
}

/******************************************************************************/
/**
 * Retrieves the worst known value for a given floating point type, depending
 * on whether maximal or minimal values are considered to be better
 */
template <std::floating_point fp_type>
fp_type getWorstCase(Gem::Common::sortOrder sort_order) {
    return (
        sort_order == Gem::Common::sortOrder::HIGHERISBETTER
            ? std::numeric_limits<fp_type>::lowest()
            : std::numeric_limits<fp_type>::max()
    );
}

/******************************************************************************/
/**
 * Retrieves the best known value for a given floating point type, depending
 * on whether maximal or minimal values are considered to be better
 */
template <std::floating_point fp_type>
fp_type getBestCase(Gem::Common::sortOrder sort_order) {
    return (
        sort_order == Gem::Common::sortOrder::HIGHERISBETTER
            ? std::numeric_limits<fp_type>::max()
            : std::numeric_limits<fp_type>::lowest()
    );
}

/******************************************************************************/
/**
 * Checks that a floating point value is contained in a given range
 *
 * @param val The value to be checked for containment
 * @param min The lower boundary (included)
 * @param max The upper boundary (possibly included)
 * @param lower_open Determines whether the lower boundary must be smaller or may be equal to val (default: closed)
 * @param upper_open Determines whether the upper boundary must be larger or may be equal to val (default: closed)
 * @param warn_only Will warn only if the condition isn't met
 * @return The value being checked
 */
const bool GFPLOWERCLOSED = false;
const bool GFPLOWEROPEN = true;
const bool GFPUPPERCLOSED = false;
const bool GFPUPPEROPEN = true;
const bool GFNOWARNING = false;

template <std::floating_point fp_type>
fp_type checkValueRange(
    fp_type val,
    fp_type min,
    fp_type max,
    bool lower_open = false,
    bool upper_open = false,
    bool warn_only = false,
    std::string var_name = std::string()
) {
    bool in_value_range = true;

    if(lower_open) {
        if(val < std::nextafter(min, std::numeric_limits<fp_type>::infinity()))
            in_value_range = false;
    }
    else {
        if(val < min)
            in_value_range = false;
    }

    if(upper_open) {
        if(val > std::nextafter(max, -std::numeric_limits<fp_type>::infinity()))
            in_value_range = false;
    }
    else {
        if(val > max)
            in_value_range = false;
    }

    if(not in_value_range) {
        if(warn_only) {
            glogger << "In checkValueRange<fp_type>(): Error!" << '\n'
                    << "Value " << val << (var_name.empty() ? "" : (" of variable " + var_name))
                    << " outside of recommended range " << '\n'
                    << min << (lower_open ? " (open) - " : " (closed) - ") << max
                    << (upper_open ? " (open)" : " (closed)") << '\n'
                    << GWARNING;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In checkValueRange<fp_type>(): Error!" << '\n'
                << "Value " << val << (var_name.empty() ? "" : (" of variable " + var_name))
                << " outside of allowed range " << '\n'
                << min << (lower_open ? " (open) - " : " (closed) - ") << max
                << (upper_open ? " (open)" : " (closed)") << '\n'
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
 * @param lower_open Determines whether the lower boundary must be smaller or may be equal to val (default: closed)
 * @param upper_open Determines whether the upper boundary must be larger or may be equal to val (default: closed)
 * @param warn_only Will warn only if the condition isn't met
 * @return The value being checked
 */
template <std::integral int_type>
int_type checkValueRange(
    int_type val,
    int_type min,
    int_type max,
    bool lower_open = false,
    bool upper_open = false,
    bool warn_only = false
) {
    bool in_value_range = true;

    if(lower_open) {
        if(val <= min)
            in_value_range = false;
    }
    else {
        if(val < min)
            in_value_range = false;
    }

    if(upper_open) {
        if(val >= max)
            in_value_range = false;
    }
    else {
        if(val > max)
            in_value_range = false;
    }

    if(not in_value_range) {
        if(warn_only) {
            glogger << "Warning:" << '\n'
                    << "In checkValueRange<int_type>(): Error!" << '\n'
                    << "Value " << val << " outside of recommended range " << '\n'
                    << min << (lower_open ? " (open) - " : " (closed) - ") << max
                    << (upper_open ? " (open)" : " (closed)") << '\n'
                    << GWARNING;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In checkValueRange<int_type>(): Error!" << '\n'
                << "Value " << val << " outside of allowed range " << '\n'
                << min << (lower_open ? " (open) - " : " (closed) - ") << max
                << (upper_open ? " (open)" : " (closed)") << '\n'
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
 * @param ext_dat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template <typename x_type_undet>
auto getMinMax(const std::vector<x_type_undet> &ext_dat) {
    if(ext_dat.size() < std::size_t(2)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBasePlotter::getMinMax(1D): Error!" << '\n'
            << "Got vector of invalid size " << ext_dat.size() << '\n'
        );
    }

    auto [min_it, max_it] = std::minmax_element(ext_dat.begin(), ext_dat.end());
    return std::tuple<x_type_undet, x_type_undet>{*min_it, *max_it};
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 2d-Tuples of undefined types.
 * This function requires that x_type_undet and y_type_undet can be compared using the
 * usual operators
 *
 * @param ext_dat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template <typename x_type_undet, typename y_type_undet>
auto getMinMax(const std::vector<std::tuple<x_type_undet, y_type_undet>> &ext_dat) {
    // Do some error checking
    if(ext_dat.size() < (std::size_t)2) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBasePlotter::getMinMax(2D): Error!" << '\n'
            << "Got vector of invalid size " << ext_dat.size() << '\n'
        );
    }

    x_type_undet min_x = std::get<0>(ext_dat.at(0)), max_x = min_x;
    y_type_undet min_y = std::get<1>(ext_dat.at(0)), max_y = min_y;

    for(std::size_t i = 1; i < ext_dat.size(); i++) {
        if(std::get<0>(ext_dat.at(i)) < min_x)
            min_x = std::get<0>(ext_dat.at(i));
        if(std::get<0>(ext_dat.at(i)) > max_x)
            max_x = std::get<0>(ext_dat.at(i));
        if(std::get<1>(ext_dat.at(i)) < min_y)
            min_y = std::get<1>(ext_dat.at(i));
        if(std::get<1>(ext_dat.at(i)) > max_y)
            max_y = std::get<1>(ext_dat.at(i));
    }

    return std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>{
        min_x,
        max_x,
        min_y,
        max_y
    };
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 3d-Tuples of undefined types.
 * This function requires that x_type_undet, y_type_undet and z_type_undet can be compared
 * using the usual operators
 *
 * @param ext_dat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template <typename x_type_undet, typename y_type_undet, typename z_type_undet>
auto getMinMax(const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>> &ext_dat) {
    // Do some error checking
    if(ext_dat.size() < (std::size_t)2) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBasePlotter::getMinMax(3D): Error!" << '\n'
            << "Got vector of invalid size " << ext_dat.size() << '\n'
        );
    }

    x_type_undet min_x = std::get<0>(ext_dat.at(0)), max_x = min_x;
    y_type_undet min_y = std::get<1>(ext_dat.at(0)), max_y = min_y;
    z_type_undet min_z = std::get<2>(ext_dat.at(0)), max_z = min_z;

    for(std::size_t i = 1; i < ext_dat.size(); i++) {
        if(std::get<0>(ext_dat.at(i)) < min_x)
            min_x = std::get<0>(ext_dat.at(i));
        if(std::get<0>(ext_dat.at(i)) > max_x)
            max_x = std::get<0>(ext_dat.at(i));
        if(std::get<1>(ext_dat.at(i)) < min_y)
            min_y = std::get<1>(ext_dat.at(i));
        if(std::get<1>(ext_dat.at(i)) > max_y)
            max_y = std::get<1>(ext_dat.at(i));
        if(std::get<2>(ext_dat.at(i)) < min_z)
            min_z = std::get<2>(ext_dat.at(i));
        if(std::get<2>(ext_dat.at(i)) > max_z)
            max_z = std::get<2>(ext_dat.at(i));
    }

    return std::
        tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet, z_type_undet, z_type_undet>{
            min_x,
            max_x,
            min_y,
            max_y,
            min_z,
            max_z
        };
}

/******************************************************************************/
/**
 * Find the minimum and maximum component in a vector of 4d-Tuples of undefined types.
 * This function requires that x_type_undet, y_type_undet, z_type_undet and w_type_undet
 * can be compared using the usual operators
 *
 * @param ext_dat The vector holding the data, for which extreme values should be calculated
 * @return A std::tuple holding the extreme values
 */
template <
    typename x_type_undet,
    typename y_type_undet,
    typename z_type_undet,
    typename w_type_undet>
auto getMinMax(
    const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet>> &ext_dat
) {
    // Do some error checking
    if(ext_dat.size() < (std::size_t)2) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBasePlotter::getMinMax(4D): Error!" << '\n'
            << "Got vector of invalid size " << ext_dat.size() << '\n'
        );
    }

    x_type_undet min_x = std::get<0>(ext_dat.at(0)), max_x = min_x;
    y_type_undet min_y = std::get<1>(ext_dat.at(0)), max_y = min_y;
    z_type_undet min_z = std::get<2>(ext_dat.at(0)), max_z = min_z;
    w_type_undet min_w = std::get<3>(ext_dat.at(0)), max_w = min_w;

    for(std::size_t i = 1; i < ext_dat.size(); i++) {
        if(std::get<0>(ext_dat.at(i)) < min_x)
            min_x = std::get<0>(ext_dat.at(i));
        if(std::get<0>(ext_dat.at(i)) > max_x)
            max_x = std::get<0>(ext_dat.at(i));
        if(std::get<1>(ext_dat.at(i)) < min_y)
            min_y = std::get<1>(ext_dat.at(i));
        if(std::get<1>(ext_dat.at(i)) > max_y)
            max_y = std::get<1>(ext_dat.at(i));
        if(std::get<2>(ext_dat.at(i)) < min_z)
            min_z = std::get<2>(ext_dat.at(i));
        if(std::get<2>(ext_dat.at(i)) > max_z)
            max_z = std::get<2>(ext_dat.at(i));
        if(std::get<3>(ext_dat.at(i)) < min_w)
            min_w = std::get<3>(ext_dat.at(i));
        if(std::get<3>(ext_dat.at(i)) > max_w)
            max_w = std::get<3>(ext_dat.at(i));
    }

    return std::tuple<
        x_type_undet,
        x_type_undet,
        y_type_undet,
        y_type_undet,
        z_type_undet,
        z_type_undet,
        w_type_undet,
        w_type_undet>{min_x, max_x, min_y, max_y, min_z, max_z, min_w, max_w};
}

/******************************************************************************/
/**
 * Calculates the mean value from a std::vector of floating point values
 *
 * @param par_vec The vector of values for which the mean should be calculated
 * @return The mean value of par_vec
 */
template <typename T>
T GMean(const std::vector<T> &par_vec) {
#ifdef DEBUG
    if(par_vec.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In T GMean(const std::vector<T>&): Error!" << '\n'
            << "par_vec has size 0" << '\n'
        );
    }
#endif /* DEBUG */

    T mean = T(0);
    for(const auto &v : par_vec)
        mean += v;
    return mean / static_cast<T>(par_vec.size());
}

/******************************************************************************/
/**
 * Calculates the mean and standard deviation for a std::vector of floating point values
 *
 * @param par_vec The vector of values for which the standard deviation should be calculated
 * @return A std::tuple holding the mean value and the standard deviation of the values stored in par_vec
 */
template <typename T>
auto GStandardDeviation(const std::vector<T> &par_vec) {
    // GMean will throw in DEBUG mode if par_vec is empty
    T mean = GMean(par_vec), sigma = T(0);

    // It is easy if the size is 1
    if(par_vec.size() == 1) {
        return std::tuple<T, T>{par_vec.at(0), T(0)};
    }

    for(const auto &v : par_vec) {
        const auto d = v - mean;
        sigma += d * d;
    }
    sigma /= T(par_vec.size() - 1);
    sigma = std::sqrt(sigma);

    return std::tuple<T, T>{mean, sigma};
}

/******************************************************************************/
/**
 * Compile-time integer power: B^E.
 * Replaces the old PowSmallPosInt struct template metaprogramming.
 */
template <std::size_t B, std::size_t E>
constexpr std::size_t PowSmallPosInt() {
    if constexpr(E == 0)
        return std::size_t(1);
    else if constexpr(E == 1)
        return B;
    else
        return B * PowSmallPosInt<B, E - 1>();
}

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
void subtractVec(std::vector<T> &a, const std::vector<T> &b) {
#ifdef DEBUG
    if(a.size() != b.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In subtractVec(std::vector<T>, const std::vector<T>&): Error!" << '\n'
            << "Found invalid sizes: " << a.size() << " / " << b.size() << '\n'
        );
    }
#endif /* DEBUG */

    std::transform(a.begin(), a.end(), b.begin(), a.begin(), [](const T &x, const T &y) {
        return x - y;
    });
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
void addVec(std::vector<T> &a, const std::vector<T> &b) {
#ifdef DEBUG
    if(a.size() != b.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In addVec(std::vector<T>, const std::vector<T>&): Error!" << '\n'
            << "Found invalid sizes: " << a.size() << " / " << b.size() << '\n'
        );
    }
#endif /* DEBUG */

    std::transform(a.begin(), a.end(), b.begin(), a.begin(), [](const T &x, const T &y) {
        return x + y;
    });
}

/******************************************************************************/
/**
 * Multiplies each position of a std::vector<> with a constant. Note that we assume here that
 * T understands the operator*= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector whose elements will be multiplied by c
 * @param c The constant which will be multiplied with each position of a
 */
template <typename T>
void multVecConst(std::vector<T> &a, const T &c) {
    for(auto &v : a)
        v *= c;
}

/******************************************************************************/
/**
 * Assigns a constant value to each position of the vector.
 *
 * @param a The vector to whose elements c will be assigned
 * @param c The constant which will be assigned each position of a
 */
template <typename T>
void assignVecConst(std::vector<T> &a, const T &c) {
    std::fill(a.begin(), a.end(), c);
}

/******************************************************************************/
/**
 * Sums up the x- and y-components individually of a vector of 2d-tuples
 */
template <typename fp_type>
std::tuple<fp_type, fp_type>
sumTupleVec(const std::vector<std::tuple<fp_type, fp_type>> &data_points) {
    std::tuple<fp_type, fp_type> result{fp_type(0.), fp_type(0.)};
    for(const auto &p : data_points) {
        std::get<0>(result) += std::get<0>(p);
        std::get<1>(result) += std::get<1>(p);
    }
    return result;
}

/******************************************************************************/
/**
 * Sums up the squares of x- and y-components individually of a vector of 2d-tuples
 */
template <typename fp_type>
std::tuple<fp_type, fp_type>
squareSumTupleVec(const std::vector<std::tuple<fp_type, fp_type>> &data_points) {
    std::tuple<fp_type, fp_type> result{fp_type(0.), fp_type(0.)};
    for(const auto &p : data_points) {
        const auto x = std::get<0>(p);
        const auto y = std::get<1>(p);
        std::get<0>(result) += x * x;
        std::get<1>(result) += y * y;
    }
    return result;
}

/******************************************************************************/
/**
 * Sums up the product of x- and y-components of a vector of 2d-tuples
 */
template <std::floating_point fp_type>
fp_type productSumTupleVec(const std::vector<std::tuple<fp_type, fp_type>> &data_points) {
    fp_type result = fp_type(0.);
    for(const auto &p : data_points) {
        result += std::get<0>(p) * std::get<1>(p);
    }
    return result;
}

/******************************************************************************/
/**
 * Calculates the "square deviation" of a set of floating point tuples from
 * a line defined through a + b*x .
 *
 * @param data_points A vector of bi-tuples with x-y data points
 * @param a The offset of a line
 * @param b The slope of a line
 * @return The square deviation of the data points from the line
 */
template <std::floating_point fp_type>
fp_type squareDeviation(
    const std::vector<std::tuple<fp_type, fp_type>> &data_points,
    const fp_type &a,
    const fp_type &b
) {
    fp_type result = fp_type(0);
    for(const auto &p : data_points) {
        const auto d = std::get<1>(p) - a - b * std::get<0>(p);
        result += d * d;
    }
    return result;
}

/******************************************************************************/
/**
 * Calculates the parameters a and b of a regression line, plus errors. The return
 * value is a std::tuple of four fp_type values: a, error_a, b, error_b, with the
 * line being defined by L(x)=a+b*x .
 *
 * @param data_points A vector of data points to which the lines parameters should fit
 * @return Regression parameters for a line defined by the input data points
 */
template <typename fp_type>
auto getRegressionParameters(const std::vector<std::tuple<fp_type, fp_type>> &data_points) {
    if(data_points.empty()) {
        return std::tuple<fp_type, fp_type, fp_type, fp_type>{
            fp_type(0.),
            fp_type(0.),
            fp_type(0.),
            fp_type(0.)
        };
    }

    fp_type a = fp_type(0), b = fp_type(0);
    fp_type n = fp_type(data_points.size());

    std::tuple<fp_type, fp_type> sum_xy = sumTupleVec(data_points);
    fp_type sum_x = std::get<0>(sum_xy);
    fp_type sum_y = std::get<1>(sum_xy);

    std::tuple<fp_type, fp_type> sq_sum_xy = squareSumTupleVec(data_points);
    fp_type sq_sum_x = std::get<0>(sq_sum_xy);

    fp_type prod_sum_xy = productSumTupleVec(data_points);

    const fp_type denom = n * sq_sum_x - sum_x * sum_x;
    a = (sum_y * sq_sum_x - sum_x * prod_sum_xy) / denom;
    b = (n * prod_sum_xy - sum_x * sum_y) / denom;

    fp_type dev = squareDeviation(data_points, a, b);

    fp_type sigma_a = std::sqrt(dev / (n - fp_type(2.))) * std::sqrt(sq_sum_x / denom);
    fp_type sigma_b = std::sqrt(dev / (n - fp_type(2.))) * std::sqrt(n / denom);

    return std::tuple<fp_type, fp_type, fp_type, fp_type>{a, sigma_a, b, sigma_b};
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
template <typename fp_type>
auto getRatioError(
    const std::tuple<fp_type, fp_type, fp_type, fp_type> &s,
    const std::tuple<fp_type, fp_type, fp_type, fp_type> &p
) {
    // p may not be 0
    if(0. == std::get<2>(p)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place) << "In getRatioError(): Error!" << '\n'
                                                     << "Attempted division by 0." << '\n'
        );
    }

    fp_type sleep_time = std::get<0>(s);

    // Check that the sleep-times for s and p are the same
    if(sleep_time != std::get<0>(p)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In getRatioError(): Error!" << '\n'
            << "Sleep times differ: " << sleep_time << " / " << std::get<0>(p) << '\n'
        );
    }

    const fp_type s_val = std::get<2>(s);
    const fp_type s_err = std::get<3>(s);
    const fp_type p_val = std::get<2>(p);
    const fp_type p_err = std::get<3>(p);

    const fp_type s_err_term = s_err / p_val;
    const fp_type p_err_term = s_val * p_err / (p_val * p_val);

    return std::tuple<fp_type, fp_type, fp_type, fp_type>{
        sleep_time,
        fp_type(0.),
        s_val / p_val,
        std::sqrt(s_err_term * s_err_term + p_err_term * p_err_term)
    };
}

/******************************************************************************/
/**
 * Calculates the error for a function f=s/p , where s and p are independent
 * quantities, each with its own error. The function is applied to a std::vector
 * of different s and p (together with their errors). It returns a vector
 * of s/p together with their errors.
 */
template <typename fp_type>
std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> getRatioErrors(
    const std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> &sn,
    const std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> &pn
) {
    // Check that both vectors have the same size, otherwise complain
    if(sn.size() != pn.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In getRatioErrors(): Error!" << '\n'
            << "Vectors have invalid sizes: " << sn.size() << " / " << pn.size() << '\n'
        );
    }

    std::vector<std::tuple<fp_type, fp_type, fp_type, fp_type>> spn;
    spn.reserve(sn.size());
    for(std::size_t i = 0; i < sn.size(); ++i) {
        spn.push_back(getRatioError(sn[i], pn[i]));
    }

    return spn;
}

/******************************************************************************/
/**
 * This function checks whether a given floating point value is "close" to a given
 * target value, with a maximum difference provided as a parameter
 */
template <std::floating_point fp_type>
bool isClose(
    fp_type val,
    fp_type target = 0,
    fp_type margin = fp_type(0.00001)
) {
    return (std::abs(val - target) <= margin);
}

/******************************************************************************/
/**
 * Rational (algebraic) sigmoid — the Gjl-softsign function.
 * See http://en.wikipedia.org/wiki/File:Gjl-t%28x%29.svg .
 *
 * NOT the logistic sigmoid (1/(1+e^-x)). This is a softsign:
 *   f(var) = barrier * var / (steepness + |var|)
 * which maps ℝ → (-barrier, +barrier) antisymmetrically and approaches its
 * asymptotes polynomially (not exponentially). Uses long double internally
 * for precision near the barrier. Precondition: steepness > 0.
 *
 * @param var       Input value
 * @param barrier   Asymptotic limit; output stays strictly within (-barrier, +barrier)
 * @param steepness Controls convergence speed; larger → slower approach to barrier
 */
template <std::floating_point fp_type>
fp_type grational_sigmoid(fp_type var, fp_type barrier, fp_type steepness) {
#ifdef DEBUG
    if(steepness <= fp_type(0)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "grational_sigmoid(): steepness must be > 0, got " << steepness << '\n'
        );
    }
#endif
    const auto lvar = static_cast<long double>(var);
    const auto lbarrier = static_cast<long double>(barrier);
    const auto lsteepness = static_cast<long double>(steepness);
    return static_cast<fp_type>(lbarrier * lvar / (lsteepness + std::abs(lvar)));
}

/******************************************************************************/

} /* namespace Gem::Common */
