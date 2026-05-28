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
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <type_traits>
#include <typeinfo>
#include <vector>

// Boost headers go here
#include <boost/serialization/nvp.hpp>

// Gemfony headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A token to be handed to different comparators, so they can signal the violation
 * of expectations
 */
class GToken // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
    /** @brief The standard constructor -- initialization with class name and expectation */
    GToken(std::string, Gem::Common::expectation);

    /*************************************************************************/
    // Defaulted or deleted constructors, destructor and assignment operators
    // We enforce the usage of a signle constructor and prevent any assignment.

    GToken() = delete;
    GToken(GToken const &) = delete;
    GToken(GToken &&) = delete;

    GToken &operator=(GToken const &) = delete;
    GToken &operator=(GToken &&) = delete;

    /*************************************************************************/

    /** @brief Increments the test counter */
    void incrTestCounter();
    /** @brief Increments the counter of tests that met the expectation */
    void incrSuccessCounter();

    /** @brief Allows to retrieve the current state of the success counter */
    std::size_t getSuccessCounter() const;
    /** @brief Allows to retrieve the current state of the test counter */
    std::size_t getTestCounter() const;

    /** @brief Allows to check whether the expectation was met */
    bool expectationMet() const;
    /** @brief Conversion to a boolean indicating whether the expectation was met */
    operator bool() const; // NOLINT

    /** @brief Allows to retrieve the expectation token */
    Gem::Common::expectation getExpectation() const;
    /** @brief Allows to retrieve the expectation token as a string */
    std::string getExpectationStr() const;
    /** @brief Allows to retrieve the name of the caller */
    std::string getCallerName() const;

    /** @brief Allows to register an error message e.g. obtained from a failed check */
    void registerErrorMessage(std::string const &);
    /** @brief Allows to register an exception obtained from a failed check */
    void registerErrorMessage(g_expectation_violation const &);

    /** @brief Allows to retrieve the currently registered error messages */
    std::string getErrorMessages() const;

    /** @brief Conversion to a string indicating success or failure */
    std::string toString() const;

    /** @brief Evaluates the information in this object */
    void evaluate() const;

private:
    /** @brief Counts all tests vs. tests that have met the expectation */
    std::tuple<std::size_t, std::size_t> test_counter_;
    /** @brief Error messages obtained from failed checks */
    std::vector<std::string> error_messages_;

    /** @brief The name of the calling class */
    const std::string caller_;
    /** @brief The expectation to be met */
    const Gem::Common::expectation e_ = Gem::Common::expectation::INEQUALITY;
};

/******************************************************************************/
/**
 * This function facilitates the output of GToken objects, mostly for debugging purposes.
 */
std::ostream &operator<<(std::ostream &s, GToken const &g);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#define BASENAME(B) std::string(#B)

/**
 * This struct facilitates transfer of comparable items to comparators
 */
template <typename T>
struct identity // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
    /***************************************************************************/
    /**
     * The standard constructor
     */
    identity(
        T const &x_var,
        T const &y_var,
        std::string x_name_var,
        std::string y_name_var,
        double l_var
    )
      : x(x_var)
      , y(y_var)
      , x_name(std::move(x_name_var))
      , y_name(std::move(y_name_var))
      , limit(l_var) { /* nothing */
    }

    /***************************************************************************/
    // Deleted and defaulted functions / rule of five

    identity() = delete;
    identity(identity const &) = default;
    identity(identity &&) = default;

    // identity holds reference and const members, so the assignment operators
    // are implicitly deleted by the language. Declare the deletion explicitly
    // to surface that fact (a previous `= default` was misleading: defaulted
    // assignment on such a class is implicitly deleted, not user-provided).
    identity &operator=(identity const &) = delete;
    identity &operator=(identity &&) = delete;

    /***************************************************************************/
    /**
     * Explicit conversion to a base-class identity. Mark explicit so the
     * conversion only fires where intentionally requested (`identity<Base>{id}`
     * or `static_cast<identity<Base>>(id)`); implicit conversion would
     * widen the set of overload-resolution paths in surprising ways and
     * is rarely what the caller wants.
     */
    template <typename base_type>
    explicit operator identity<base_type>() const {
        // We use an internal function for the actual conversion
        // so we may check whether base_type is an actual base of T
        return to<base_type>();
    }

    /***************************************************************************/
    // The actual data
    const T &x;
    const T &y;
    const std::string x_name;
    const std::string y_name;
    const double limit;

private:
    /***************************************************************************/
    /**
     * Does the actual conversion, including a check that base_type is indeed a base of T
     */
    template <typename base_type>
        requires std::is_base_of_v<base_type, T>
    identity<base_type>
    to() const {
        auto const &x_conv = dynamic_cast<base_type const &>(x);
        auto const &y_conv = dynamic_cast<base_type const &>(y);

        const std::string x_name_conv = "(" + BASENAME(base_type) + ")" + x_name;
        const std::string y_name_conv = "(" + BASENAME(base_type) + ")" + y_name;

        return identity<base_type>(x_conv, y_conv, x_name_conv, y_name_conv, limit);
    }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Easy output of an identity object
 */
template <typename T>
std::ostream &operator<<(std::ostream &s, identity<T> const &i) {
    s << "Identity:" << '\n'
      << "x_name = " << i.x_name << '\n'
      << "y_name = " << i.y_name << '\n';
    return s;
}

/******************************************************************************/
/**
 * Returns an identity object. The function is needed as automatic type
 * deduction does not work for structs / classes. We assume a central default
 * value for the maximum allowed difference for "similar" floating point values.
 */
template <typename T>
identity<T> getIdentity(
    T const &x_var,
    T const &y_var,
    std::string const &x_name_var,
    std::string const &y_name_var
) {
    return identity<T>(
        x_var,
        y_var,
        x_name_var,
        y_name_var,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    );
}

/******************************************************************************/
/**
 * This macro facilitates the creation of an identity object including
 * variable names
 */
#define IDENTITY(x, y) Gem::Common::getIdentity((x), (y), std::string(#x), std::string(#y))

/******************************************************************************/
/**
 * Returns an identity object for base types of T
 */
template <typename T, typename base_type>
identity<base_type> getBaseIdentity(
    T const &x_var,
    T const &y_var,
    std::string const &x_name_var,
    std::string const &y_name_var
) {
    std::cout << "Creating base identity" << '\n';

    auto const &x_var_base = dynamic_cast<const base_type &>(x_var);
    auto const &y_var_base = dynamic_cast<const base_type &>(y_var);

    return identity<base_type>(
        x_var_base,
        y_var_base,
        x_name_var,
        y_name_var,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    );
}

/******************************************************************************/
/**
 * This macro helps to cast an object to its parent class before creating the identity object
 */
#define IDENTITY_CAST(t, x, y)                                                                     \
    Gem::Common::getBaseIdentity<decltype(x), t>(                                                  \
        (x),                                                                                       \
        (y),                                                                                       \
        std::string("(const " #t "&)" #x),                                                         \
        std::string("(const " #t "&)" #y)                                                          \
    )

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two "basic" types fulfill a given expectation.
 * It assumes that x and y understand the == and != operators and may be streamed.
 * If they do not fulfill this requirement, you need to provide a specialization
 * of these functions. A check for similarity is treated the same as a check for
 * equality. A specialization of this function is provided for floating point values.
 * The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename basic_type>
    requires (!std::is_floating_point_v<basic_type> &&
              !Gem::Common::has_gemfony_common_interface<basic_type>::value)
void compare(
    basic_type const &x,
    basic_type const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double = 0.
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY:
        expectation_str = "FP_SIMILARITY / EQUALITY";
        if(x == y) {
            expectation_met = true;
        }
        break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        break;
    };

    if(not expectation_met) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "Expectation of " << expectation_str << " was violated for parameters "
              << '\n'
              << "[" << '\n'
              << x_name << " = " << x << '\n'
              << y_name << " = " << y << '\n'
              << "]" << '\n';
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two std::chrono::time_point types fulfill a given
 * expectation. A check for similarity is treated the same as a check for
 * equality. The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename Clock, typename Duration = typename Clock::duration>
void compare(
    std::chrono::time_point<Clock, Duration> const &x,
    std::chrono::time_point<Clock, Duration> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double = 0.
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY:
        expectation_str = "FP_SIMILARITY / EQUALITY";
        if(x == y) {
            expectation_met = true;
        }
        break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        break;
    };

    if(not expectation_met) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "Expectation of " << expectation_str << " was violated for parameters "
              << '\n'
              << "[" << '\n'
              << x_name << " = " << x.time_since_epoch().count() << '\n'
              << y_name << " = " << y.time_since_epoch().count() << '\n'
              << "]" << '\n';
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two std::chrono::duration types fulfill a given
 * expectation. A check for similarity is treated the same as a check for
 * equality. The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename Rep, typename Period = std::ratio<1>>
void compare(
    std::chrono::duration<Rep, Period> const &x,
    std::chrono::duration<Rep, Period> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double = 0.
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY:
        expectation_str = "FP_SIMILARITY / EQUALITY";
        if(x == y) {
            expectation_met = true;
        }
        break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        break;
    };

    if(not expectation_met) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "Expectation of " << expectation_str << " was violated for parameters "
              << '\n'
              << "[" << '\n'
              << x_name << " = " << x.count() << '\n'
              << y_name << " = " << y.count() << '\n'
              << "]" << '\n';
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two floating point types meet a given expectation.
 * The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <std::floating_point fp_type>
void compare(
    fp_type const &x,
    fp_type const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double limit = CE_DEF_SIMILARITY_DIFFERENCE
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
        expectation_str = "FP_SIMILARITY";
        if(std::abs(x - y) < Gem::Common::narrow<fp_type>(limit)) {
            expectation_met = true;
        }
        break;
    case Gem::Common::expectation::EQUALITY:
        expectation_str = "EQUALITY";
        if(x == y) {
            expectation_met = true;
        }
        break;
    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        break;
    };

    if(not expectation_met) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)

        error << "Expectation of " << expectation_str << " was violated for parameters "
              << '\n'
              << "[" << '\n'
              << x_name << " = " << x << '\n'
              << y_name << " = " << y << '\n'
              << "]" << '\n';
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two containers of "basic" types meet a given expectation. It assumes that
 * these types understand the == and != operators. If they do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A specialization of this function is provided for floating point values. The function will throw a
 * g_expectation_violation exception if the expectation was violated.
 *
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename base_type, template <typename, typename> class c_type>
    requires (!std::is_floating_point_v<base_type>)
void compare(
    c_type<base_type, std::allocator<base_type>> const &x,
    c_type<base_type, std::allocator<base_type>> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double = 0.
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY:
        expectation_str = "FP_SIMILARITY / EQUALITY";
        if(x == y) {
            expectation_met = true;
        }
        break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        break;
    };

    if(not expectation_met) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "Expectation of " << expectation_str << " was violated for parameters " << x_name
              << " and " << y_name << "!" << '\n';

        if(Gem::Common::expectation::FP_SIMILARITY == e ||
           Gem::Common::expectation::EQUALITY == e) {
            if(x.size() != y.size()) {
                error << "Sizes of containers differ:" << '\n'
                      << x_name << ".size() == " << x.size() << " / " << y_name
                      << ".size() == " << y.size() << '\n';
            }
            else { // Some data member differs
                // Find out about the first entry that differs
                typename c_type<base_type, std::allocator<base_type>>::const_iterator x_it;
                typename c_type<base_type, std::allocator<base_type>>::const_iterator y_it;
                std::size_t failed_index = 0;
                for(x_it = x.begin(), y_it = y.begin(); x_it != x.end();
                    ++x_it, ++y_it, ++failed_index) {
                    if(*x_it != *y_it) {
                        error << "Found inequality at index " << failed_index << ": " << x_name
                              << "[" << failed_index << "] = " << *x_it << "; " << y_name << "["
                              << failed_index << "] = " << *y_it;
                        break; // break the loop
                    }
                }
            }
        }
        else { // Gem::Common::expectation::INEQUALITY == e
            error << "The two containers " << x_name << " and " << y_name << " are equal "
                  << "even though differences were expected" << '\n';
        }

        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two containers with a std::set-template interface holding "basic" types meet
 * a given expectation. It assumes that these types understand the == and != operators. If they do not fulfill
 * this requirement, you need to provide a specialization of this function. A check for similarity is treated
 * the same as a check for equality. A specialization of this function is provided for floating point values.
 * The function will throw a g_expectation_violation exception if the expectation was violated.
 *
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename base_type, template <typename, typename, typename> class s_type>
    requires (!std::is_floating_point_v<base_type>)
void compare(
    s_type<base_type, std::less<base_type>, std::allocator<base_type>> const &x,
    s_type<base_type, std::less<base_type>, std::allocator<base_type>> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double = 0.
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY:
        expectation_str = "FP_SIMILARITY / EQUALITY";
        if(x == y) {
            expectation_met = true;
        }
        break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        break;
    };

    if(not expectation_met) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "Expectation of " << expectation_str << " was violated for parameters " << x_name
              << " and " << y_name << "!" << '\n';

        if(Gem::Common::expectation::FP_SIMILARITY == e ||
           Gem::Common::expectation::EQUALITY == e) {
            if(x.size() != y.size()) {
                error << "Sizes of containers differ:" << '\n'
                      << x_name << ".size() == " << x.size() << " / " << y_name
                      << ".size() == " << y.size() << '\n';
            }
            else { // Some data member differs
                // Find out about the first entry that differs
                typename s_type<base_type, std::less<base_type>, std::allocator<base_type>>::
                    const_iterator x_it;
                typename s_type<base_type, std::less<base_type>, std::allocator<base_type>>::
                    const_iterator y_it;
                std::size_t failed_index = 0;
                for(x_it = x.begin(), y_it = y.begin(); x_it != x.end();
                    ++x_it, ++y_it, ++failed_index) {
                    if(*x_it != *y_it) {
                        error << "Found inequality at index " << failed_index << ": " << x_name
                              << "[" << failed_index << "] = " << *x_it << "; " << y_name << "["
                              << failed_index << "] = " << *y_it;
                        break; // break the loop
                    }
                }
            }
        }
        else { // Gem::Common::expectation::INEQUALITY == e
            error << "The two containers " << x_name << " and " << y_name << " are equal "
                  << "even though differences were expected" << '\n';
        }

        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two containers of floating point types meet a given expectation.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <std::floating_point fp_type, template <typename, typename> class c_type>
void compare(
    c_type<fp_type, std::allocator<fp_type>> const &x,
    c_type<fp_type, std::allocator<fp_type>> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double limit = CE_DEF_SIMILARITY_DIFFERENCE
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)
    std::size_t deviation_pos = 0;
    std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY: {
        if(Gem::Common::expectation::FP_SIMILARITY == e) {
            expectation_str = "FP_SIMILARITY";
        }
        else { // May only be FP_SIMILARITY or EQUALITY in this case
            expectation_str = "EQUALITY";
        }

        if(x.size() != y.size()) {
            error << "Different vector-sizes found : " << x_name << ".size() = " << x.size()
                  << '\n'
                  << y_name << ".size() = " << y.size() << '\n';
            break; // expectationMet is false here
        }

        // Do a per-position comparison
        bool found_deviation = false;
        typename c_type<fp_type, std::allocator<fp_type>>::const_iterator x_it;
        typename c_type<fp_type, std::allocator<fp_type>>::const_iterator y_it;
        if(Gem::Common::expectation::FP_SIMILARITY == e) {
            for(x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
                if(std::abs(*x_it - *y_it) >= Gem::Common::narrow<fp_type>(limit)) {
                    found_deviation = true;
                    deviation_pos =
                        Gem::Common::narrow<std::size_t>(std::distance(x.begin(), x_it));
                    error << "Found deviation between containers:" << '\n'
                          << x_name << "[" << deviation_pos << "] = " << *x_it << "; " << '\n'
                          << y_name << "[" << deviation_pos << "] = " << *y_it << "; " << '\n'
                          << "limit = " << Gem::Common::narrow<fp_type>(limit) << "; " << '\n'
                          << "deviation = " << std::abs(*x_it - *y_it) << '\n';
                    break; // break the loop
                }
            }
        }
        else { // May only be FP_SIMILARITY or EQUALITY in this case
            for(x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
                if(*x_it != *y_it) {
                    found_deviation = true;
                    deviation_pos =
                        Gem::Common::narrow<std::size_t>(std::distance(x.begin(), x_it));
                    error << "Found deviation between containers:" << '\n'
                          << x_name << "[" << deviation_pos << "] = " << *x_it << "; " << '\n'
                          << y_name << "[" << deviation_pos << "] = " << *y_it << "; " << '\n';
                    break; // break the loop
                }
            }
        }

        if(not found_deviation) {
            expectation_met = true;
        }
    } break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        else {
            error << "The containers " << x_name << " and " << y_name << '\n'
                  << "do not differ even though they should" << '\n';
        }
        break;
    };

    if(not expectation_met) {
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two containers with a std::set template interface,
 * holding floating point types meet a given expectation.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <std::floating_point fp_type, template <typename, typename, typename> class s_type>
void compare(
    s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>> const &x,
    s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double limit = CE_DEF_SIMILARITY_DIFFERENCE
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)
    std::size_t deviation_pos = 0;
    std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY: {
        if(Gem::Common::expectation::FP_SIMILARITY == e) {
            expectation_str = "FP_SIMILARITY";
        }
        else { // May only be FP_SIMILARITY or EQUALITY in this case
            expectation_str = "EQUALITY";
        }

        if(x.size() != y.size()) {
            error << "Different vector-sizes found : " << x_name << ".size() = " << x.size()
                  << '\n'
                  << y_name << ".size() = " << y.size() << '\n';
            break; // expectationMet is false here
        }

        // Do a per-position comparison
        bool found_deviation = false;
        typename s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>>::const_iterator x_it;
        typename s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>>::const_iterator y_it;
        if(Gem::Common::expectation::FP_SIMILARITY == e) {
            for(x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
                if(std::abs(*x_it - *y_it) >= Gem::Common::narrow<fp_type>(limit)) {
                    found_deviation = true;
                    deviation_pos =
                        Gem::Common::narrow<std::size_t>(std::distance(x.begin(), x_it));
                    error << "Found deviation between containers:" << '\n'
                          << x_name << "[" << deviation_pos << "] = " << *x_it << "; " << '\n'
                          << y_name << "[" << deviation_pos << "] = " << *y_it << "; " << '\n'
                          << "limit = " << Gem::Common::narrow<fp_type>(limit) << "; " << '\n'
                          << "deviation = " << std::abs(*x_it - *y_it) << '\n';
                    break; // break the loop
                }
            }
        }
        else { // May only be FP_SIMILARITY or EQUALITY in this case
            for(x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
                if(*x_it != *y_it) {
                    found_deviation = true;
                    deviation_pos =
                        Gem::Common::narrow<std::size_t>(std::distance(x.begin(), x_it));
                    error << "Found deviation between containers:" << '\n'
                          << x_name << "[" << deviation_pos << "] = " << *x_it << "; " << '\n'
                          << y_name << "[" << deviation_pos << "] = " << *y_it << "; " << '\n';
                    break; // break the loop
                }
            }
        }

        if(not found_deviation) {
            expectation_met = true;
        }
    } break;

    case Gem::Common::expectation::INEQUALITY:
        expectation_str = "INEQUALITY";
        if(x != y) {
            expectation_met = true;
        }
        else {
            error << "The containers " << x_name << " and " << y_name << '\n'
                  << "do not differ even though they should" << '\n';
        }
        break;
    };

    if(not expectation_met) {
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two complex types meet a given expectation. It is assumed that
 * these types have the standard Geneva interface with corresponding "compare" functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
    requires Gem::Common::gemfony_common_interface<geneva_type>
void compare(
    geneva_type const &x,
    geneva_type const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)
    std::ostringstream error;    // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY: {
        expectation_str = "FP_SIMILARITY / EQUALITY";

        // If we reach this line, then both pointers have content

        { // Check whether the content differs
            try {
                x.compare(y, e, limit);
            }
            catch(g_expectation_violation &g) {
                error << "Content of " << x_name << " and " << y_name << " differ." << '\n'
                      << "Thus the expectation of " << expectation_str
                      << " was violated:" << '\n'
                      << g.what() << '\n';
                break; // Terminate the switch statement
            }

            // If we reach this line, the expectation was met
            expectation_met = true;
        }
    } break;

    case Gem::Common::expectation::INEQUALITY: {
        expectation_str = "INEQUALITY";

        // Check whether the content differs
        try {
            x.compare(y, e, limit);
        }
        catch(g_expectation_violation &g) {
            // If we catch an expectation violation for expectation "inequality",
            // we simply break the switch statement so that expectationMet remains to be false
            error << "Content of " << x_name << " and " << y_name << " are equal/similar."
                  << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated:" << '\n'
                  << g.what() << '\n';
            break;
        }
        expectation_met = true;
    } break;
    };

    if(not expectation_met) {
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
    requires Gem::Common::gemfony_common_interface<geneva_type>
void compare(
    std::shared_ptr<geneva_type> const &x,
    std::shared_ptr<geneva_type> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)
    std::ostringstream error;    // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY: {
        expectation_str = "FP_SIMILARITY / EQUALITY";

        // Check whether the pointers hold content
        if(x && not y) {
            error << "Smart pointer " << x_name << " holds content while " << y_name << " does not."
                  << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated" << '\n';
            break; //
        }
        if(not x && y) {
            error << "Smart pointer " << x_name << " doesn't hold content while " << y_name
                  << " does." << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated" << '\n';
            break; // The expectation was clearly not met
        }
        else if(not x &&
                not y) { // No content to check. Both smart pointers can be considered equal
            expectation_met = true;
            break;
        }

        // If we reach this line, then both pointers have content

        { // Check whether the content differs
            try {
                x->compare(*y, e, limit);
            }
            catch(g_expectation_violation &g) {
                error << "Content of " << x_name << " and " << y_name << " differ." << '\n'
                      << "Thus the expectation of " << expectation_str
                      << " was violated:" << '\n'
                      << g.what() << '\n';
                break; // Terminate the switch statement
            }

            // If we reach this line, the expectation was met
            expectation_met = true;
        }
    } break;

    case Gem::Common::expectation::INEQUALITY: {
        expectation_str = "INEQUALITY";

        // Check whether the pointers hold content
        if((x && not y) || (not x && y)) {
            expectation_met = true;
            break;
        }
        if(not x &&
                not y) { // No content to check. Both smart pointers can be considered equal
            error << "Both smart pointers are empty and are thus considered equal." << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated:" << '\n';
            break; // The expectation was not met
        }

        // Check whether the content differs
        try {
            x->compare(*y, e, limit);
        }
        catch(g_expectation_violation &g) {
            // If we catch an expectation violation for expectation "inequality",
            // we simply break the switch statement so that expectationMet remains to be false
            error << "Content of " << x_name << " and " << y_name << " are equal/similar."
                  << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated:" << '\n'
                  << g.what() << '\n';
            break;
        }
        expectation_met = true;
    } break;
    };

    if(not expectation_met) {
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/**
 * This function checks whether two containers of smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions. For an idea of what the template specifier does, search for "template template" in conjunction
 * with containers.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type, template <typename, typename> class c_type>
    requires Gem::Common::gemfony_common_interface<geneva_type>
void compare(
    c_type<std::shared_ptr<geneva_type>, std::allocator<std::shared_ptr<geneva_type>>> const &x,
    c_type<std::shared_ptr<geneva_type>, std::allocator<std::shared_ptr<geneva_type>>> const &y,
    std::string const &x_name,
    std::string const &y_name,
    Gem::Common::expectation e,
    double limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
) {
    bool expectation_met = false;
    std::string expectation_str; // NOLINT(cppcoreguidelines-init-variables)
    std::ostringstream error;    // NOLINT(cppcoreguidelines-init-variables)

    switch(e) {
    case Gem::Common::expectation::FP_SIMILARITY:
    case Gem::Common::expectation::EQUALITY: {
        expectation_str = "FP_SIMILARITY / EQUALITY";

        // First check sizes
        if(x.size() != y.size()) {
            error << "containers " << x_name << " and " << y_name << " have different sizes "
                  << x.size() << " / " << y.size() << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated" << '\n';
            // Terminate the switch statement. expectationMet will be false then
            break;
        }

        // Now loop over all members of the containers
        bool found_deviation = false;
        typename c_type<
            std::shared_ptr<geneva_type>,
            std::allocator<std::shared_ptr<geneva_type>>>::const_iterator x_it;
        typename c_type<
            std::shared_ptr<geneva_type>,
            std::allocator<std::shared_ptr<geneva_type>>>::const_iterator y_it;
        std::size_t index = 0;
        for(x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it, ++index) {
            // First check that both pointers have content
            // Check whether the pointers hold content
            if(*x_it && not *y_it) {
                error << "Smart pointer " << x_name << "[" << index << "] holds content while "
                      << y_name << "[" << index << "]  does not." << '\n'
                      << "Thus the expectation of " << expectation_str << " was violated"
                      << '\n';
                found_deviation = true;
                break; // terminate the loop
            }
            if(not *x_it && *y_it) {
                error << "Smart pointer " << x_name << "[" << index
                      << "] doesn't hold content while " << y_name << "[" << index << "]  does."
                      << '\n'
                      << "Thus the expectation of " << expectation_str << " was violated"
                      << '\n';
                found_deviation = true;
                break; // terminate the loop
            }
            else if(not *x_it &&
                    not *y_it) { // No content to check. Both smart pointers can be considered equal
                continue;        // Go on with next iteration in the loop
            }

            // At this point we know that both pointers have content. We can now check the content
            // which is assumed to have the compare() function
            try {
                (*x_it)->compare(**y_it, e, limit);
            }
            catch(g_expectation_violation &g) {
                error << "Content of " << x_name << "[" << index << "] and " << y_name << "["
                      << index << "] differs." << '\n'
                      << "Thus the expectation of " << expectation_str
                      << " was violated:" << '\n'
                      << g.what() << '\n';
                found_deviation = true;
                break; // Terminate the loop
            }
        }

        if(not found_deviation) {
            expectation_met = true;
        }
    } break;

    case Gem::Common::expectation::INEQUALITY: {
        expectation_str = "INEQUALITY";

        // First check sizes. The expectation of inequality will be met if they differ
        if(x.size() != y.size()) {
            expectation_met = true;
            break; // Terminate the switch statement
        }

        // Now loop over all members of the containers
        bool found_inequality = false;
        typename c_type<
            std::shared_ptr<geneva_type>,
            std::allocator<std::shared_ptr<geneva_type>>>::const_iterator x_it;
        typename c_type<
            std::shared_ptr<geneva_type>,
            std::allocator<std::shared_ptr<geneva_type>>>::const_iterator y_it;
        for(x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
            // First check that both pointers have content
            // Check whether the pointers hold content
            if((*x_it && not *y_it) || (not *x_it && *y_it)) {
                found_inequality = true;
                break; // terminate the loop
            }
            if(not *x_it &&
                    not *y_it) { // No content to check. Both smart pointers can be considered equal
                continue; // Go on with next iteration in the loop - there is nothing to check here
            }

            // At this point we know that both pointers have content. We can now check this content
            // which is assumed to have the compare() function
            try {
                (*x_it)->compare(**y_it, e, limit);
                found_inequality = true;
                break; // terminate the loop
            }
            catch(g_expectation_violation &) {
                // Go on with the next item in the vector -- the content is equal or similar
                continue;
            }
        }

        if(found_inequality) {
            expectation_met = true;
        }
        else {
            error << "The two containers " << x_name << " and " << y_name << " are equal."
                  << '\n'
                  << "Thus the expectation of " << expectation_str << " was violated:" << '\n';
        }
    } break;
    };

    if(not expectation_met) {
        throw g_expectation_violation(error.str());
    }
}

/******************************************************************************/
/** @brief This function checks whether two objects of type Gem::Common::tribool meet a given expectation. */

void compare(
    Gem::Common::tribool const &,
    Gem::Common::tribool const &,
    std::string const &,
    std::string const &,
    Gem::Common::expectation,
    double limit = CE_DEF_SIMILARITY_DIFFERENCE
);

/******************************************************************************/
/**
 * This function checks whether two types fulfill a given expectation.
 *
 * @param data The identity struct
 * @param token The token holding information about the number of failed tests
 */
template <typename T>
void compare_t(identity<T> const &data, GToken &token) {
    try {
        token.incrTestCounter();
        compare(data.x, data.y, data.x_name, data.y_name, token.getExpectation(), data.limit);
        token.incrSuccessCounter();
    }
    catch(const g_expectation_violation &g) {
        token.registerErrorMessage(g);
    }
    catch(const std::exception &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "Caught std::exception with message " << '\n'
            << e.what() << '\n'
        );
    }
    catch(...) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << "Caught unknown exception" << '\n'
        );
    }
}

/******************************************************************************/
/**
 * This function checks whether two base types fulfill a given expectation.
 *
 * @param data The identity struct
 * @param token The token holding information about the number of failed tests
 */
template <typename base_type>
void compare_base_t(base_type const &x, base_type const &y, GToken &token) {
    try {
        token.incrTestCounter();
        x.base_type::compare_( // Force usage of the base-type compare_ function
            y
            , token.getExpectation()
            , Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
        );
        token.incrSuccessCounter();
    }
    catch(const g_expectation_violation &g) {
        token.registerErrorMessage(g);
    }
    catch(const std::exception &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "Caught std::exception with message" << '\n'
            << e.what() << '\n'
        );
    }
    catch(...) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << "Caught unknown exception" << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Single-source-of-truth machinery for a GCommonInterfaceT subclass's local data members.
 *
 * A class declares its local members exactly once through a localMembers() pair
 * (const + non-const) that returns a tuple of member_t entries (a name plus a
 * reference to the member). load_() and compare_() then derive their behaviour
 * from that single declaration via g_load_members() / g_compare_members(),
 * instead of each function enumerating the members separately. This removes the
 * "added a member but forgot to update load_()/compare_()" class of bugs.
 *
 * serialize() is intentionally NOT derived from this (yet) -- the wire format
 * is left untouched; see prompts/2026-05-25-serialize-single-source-followup.md.
 */
template <typename T>
struct member_t {
    std::string name;
    T &ref; // T& in a non-const context, T const& in a const context
};

/** @brief Builds one named member reference for a localMembers() tuple. */
template <typename T>
member_t<T> make_member(std::string name, T &ref) {
    return member_t<T>{std::move(name), ref};
}

/******************************************************************************/
/**
 * Tagged member variants for members whose in-memory copy is a deep clone rather
 * than a plain assignment. They carry the same { name, ref } shape as member_t -- so
 * serialize_members() and g_compare_members() treat them identically (a shared_ptr
 * and a container of shared_ptr each have their own serialize / compare support) --
 * but g_load_members() recognises the tag and performs a deep clone instead of a
 * shallow pointer assignment (which would alias shared state).
 *
 *  - cloneable_member_t           : a single std::shared_ptr<Cloneable> member,
 *                                   deep-copied via copyCloneableSmartPointer().
 *  - cloneable_container_member_t : a container (e.g. std::vector) of
 *                                   std::shared_ptr<Cloneable>, deep-copied via
 *                                   copyCloneableSmartPointerContainer().
 *
 * This lets a class list ALL of its data members -- plain and deep-cloned alike -- in
 * one localMembers() declaration, so serialize()/load_()/compare_() all derive from
 * the same single source with no hand-written tail.
 */
template <typename T>
struct cloneable_member_t {
    std::string name;
    T &ref; ///< a std::shared_ptr<Cloneable> (const& in a const context)
};

template <typename T>
struct cloneable_container_member_t {
    std::string name;
    T &ref; ///< a container of std::shared_ptr<Cloneable> (const& in a const context)
};

/**
 * An atomic member (e.g. std::atomic<bool>). Boost already serialises std::atomic<bool>
 * via a free serialization, so serialize_members() handles it through the common .ref
 * path; but an atomic is neither copy-assignable nor directly comparable through the
 * generic value path, so g_load_members() loads it via .store(.load()) and
 * g_compare_members() compares its loaded value.
 */
template <typename T>
struct atomic_member_t {
    std::string name;
    T &ref; ///< a std::atomic<...> (const& in a const context)
};

/** @brief Builds one named, deep-cloned single-pointer member for a localMembers() tuple. */
template <typename T>
cloneable_member_t<T> make_cloneable_member(std::string name, T &ref) {
    return cloneable_member_t<T>{std::move(name), ref};
}

/** @brief Builds one named, deep-cloned pointer-container member for a localMembers() tuple. */
template <typename T>
cloneable_container_member_t<T> make_cloneable_container_member(std::string name, T &ref) {
    return cloneable_container_member_t<T>{std::move(name), ref};
}

/** @brief Builds one named atomic member for a localMembers() tuple. */
template <typename T>
atomic_member_t<T> make_atomic_member(std::string name, T &ref) {
    return atomic_member_t<T>{std::move(name), ref};
}

/******************************************************************************/
/** @brief Loads a single member, dispatched on its kind. */
template <typename Dst, typename Src>
void g_load_one(member_t<Dst> &dst, const member_t<Src> &src) {
    dst.ref = src.ref; // plain assignment
}
template <typename Dst, typename Src>
void g_load_one(cloneable_member_t<Dst> &dst, const cloneable_member_t<Src> &src) {
    Gem::Common::copyCloneableSmartPointer(src.ref, dst.ref); // deep clone
}
template <typename Dst, typename Src>
void g_load_one(cloneable_container_member_t<Dst> &dst, const cloneable_container_member_t<Src> &src) {
    Gem::Common::copyCloneableSmartPointerContainer(src.ref, dst.ref); // deep clone of each element
}
template <typename Dst, typename Src>
void g_load_one(atomic_member_t<Dst> &dst, const atomic_member_t<Src> &src) {
    dst.ref.store(src.ref.load()); // atomic load/store (atomics are not copy-assignable)
}

template <typename DstTuple, typename SrcTuple, std::size_t... I>
void g_load_members_impl(DstTuple &dst, const SrcTuple &src, std::index_sequence<I...>) {
    (g_load_one(std::get<I>(dst), std::get<I>(src)), ...);
}

/******************************************************************************/
/** @brief Compares a single member pairwise, dispatched on its kind. */
template <typename A, typename B>
void g_compare_one(const A &a, const B &b, GToken &token) {
    compare_t(getIdentity(a.ref, b.ref, a.name, b.name), token);
}
template <typename A, typename B>
void g_compare_one(const atomic_member_t<A> &a, const atomic_member_t<B> &b, GToken &token) {
    // Compare the loaded values rather than the atomic objects themselves.
    compare_t(getIdentity(a.ref.load(), b.ref.load(), a.name, b.name), token);
}

/** @brief Copies each local member from src to dst, member by member. */
template <typename DstTuple, typename SrcTuple>
void g_load_members(DstTuple dst, SrcTuple src) {
    static_assert(
        std::tuple_size_v<DstTuple> == std::tuple_size_v<SrcTuple>,
        "g_load_members: localMembers() arity mismatch"
    );
    g_load_members_impl(dst, src, std::make_index_sequence<std::tuple_size_v<DstTuple>>{});
}

template <typename ATuple, typename BTuple, std::size_t... I>
void g_compare_members_impl(
    const ATuple &a, const BTuple &b, GToken &token, std::index_sequence<I...>
) {
    (g_compare_one(std::get<I>(a), std::get<I>(b), token), ...);
}

/** @brief Compares each local member pairwise, recording results in the token. */
template <typename ATuple, typename BTuple>
void g_compare_members(ATuple a, BTuple b, GToken &token) {
    static_assert(
        std::tuple_size_v<ATuple> == std::tuple_size_v<BTuple>,
        "g_compare_members: localMembers() arity mismatch"
    );
    g_compare_members_impl(a, b, token, std::make_index_sequence<std::tuple_size_v<ATuple>>{});
}

/******************************************************************************/
/**
 * Serializes each local member through the Boost archive, using the member's
 * name (from its member_t) as the NVP tag. This lets a class's serialize()
 * derive its member list from the same single localMembers() declaration that
 * load_() and compare_() already use, keeping the member list in one place.
 *
 * The temporary tuple passed by value keeps its name strings alive for the full
 * duration of the call, during which the (ar & ...) operations run -- so the
 * c_str() pointers handed to make_nvp remain valid.
 */
template <typename Archive, typename Tuple, std::size_t... I>
void serialize_members_impl(Archive& ar, Tuple& members, std::index_sequence<I...>) {
    ((ar & boost::serialization::make_nvp(std::get<I>(members).name.c_str(), std::get<I>(members).ref)), ...);
}
template <typename Archive, typename Tuple>
void serialize_members(Archive& ar, Tuple members) {
    serialize_members_impl(ar, members, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */
