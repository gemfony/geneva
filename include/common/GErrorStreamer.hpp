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

// Standard header files go here
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GLogger.hpp"

/******************************************************************************/
// Syntactic sugar. `inline constexpr` so the constants have a single
// definition across all translation units that include this header
// (plain `const` at namespace scope gives each TU its own copy and
// risks ODR mismatches in a future refactor).
inline constexpr bool DO_LOG = true;
inline constexpr bool NO_LOG = false;

/******************************************************************************/

#define time_and_place                                                                             \
    std::string(                                                                                   \
        std::string("Recorded on ") + Gem::Common::currentTimeAsString() + "\n" + "in File " +     \
        __FILE__ + " at line " + std::to_string(__LINE__) + " :\n"                                 \
    )

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple wrapper for a string_error_streamer, so we can more easily send data to a string
 * when throwing an exception. The class may optionally duplicate data and send it
 * to the global logger. This will happen during string conversion, so we may simply
 * construct a g_error_streamer object inside of a throw()-call.
 */
class g_error_streamer {
public:
    /**************************************************************************/
    /**
	  * The default constructor. We may optionally instruct the class to
	  * also log to the global logger during string conversion.
	  *
	  * @param do_log Instructs the object to also send data to the logger
	  */
    explicit g_error_streamer(bool do_log, std::string where_and_when)
      : do_log_(do_log)
      , where_and_when_(std::move(where_and_when)) { /* nothing */
    }

    /*************************************************************************/
    // Defaulted or deleted constructors, destructor and assignment operators

    g_error_streamer() = default;

    g_error_streamer(const g_error_streamer &) = delete;
    g_error_streamer &operator=(g_error_streamer &) = delete;
    g_error_streamer(const g_error_streamer &&) = delete;
    g_error_streamer &operator=(g_error_streamer &&) = delete;

    ~g_error_streamer() = default;

    /**************************************************************************/
    /**
	  * This function allows us to stream virtually any type of streamable data
	  * to this class.
	  *
	  * @tparam value_type The parameter type of a value streamed into the class
	  * @param value The value streamed into this class
	  * @return A pointer to this object
	  */
    template <typename value_type>
    g_error_streamer &operator<<(const value_type &val) {
        ostream_ << val;
        return *this;
    }

    /******************************************************************************/
    /**
	  * Needed for stringstream
	  */
    g_error_streamer &operator<<(std::ostream &(*val)(std::ostream &)) {
        ostream_ << val;
        return *this;
    }

    /******************************************************************************/
    /**
	  * Needed for stringstream
	  */
    g_error_streamer &operator<<(std::ios &(*val)(std::ios &)) {
        ostream_ << val;
        return *this;
    }

    /******************************************************************************/
    /**
	  *  Needed for stringstream
	  */
    g_error_streamer &operator<<(std::ios_base &(*val)(std::ios_base &)) {
        ostream_ << val;
        return *this;
    }

    /**************************************************************************/
    /**
	  * Automatic conversion to a string. The function will optionally send the
	  * output to the global logger.
	  *
	  * IMPORTANT — exception-safety contract: this conversion runs file I/O
	  * and re-enters the global logger when @c do_log_ is true. The canonical
	  * use site is inside @c throw geneva_exception(g_error_streamer(...) <<
	  * ... ); — i.e. the conversion happens while building the exception, *before*
	  * stack-unwinding starts, so any throw originating here is the primary
	  * (and only) in-flight exception. Callers MUST NOT trigger this conversion
	  * from inside a destructor that is itself running during unwinding (a
	  * second throw under those conditions calls @c std::terminate). If you
	  * need an allocation-free read of the wrapped message during unwinding,
	  * call @c content() (no logging) instead of converting.
	  *
	  * @return A string with the content of the wrapped string_error_streamer object.
	  */
    /**************************************************************************/
    /**
	  * Returns the wrapped streamer's contents WITHOUT triggering any I/O
	  * or logger re-entry. Safe to call from unwinding contexts where the
	  * implicit @c operator std::string() conversion would be hazardous.
	  */
    [[nodiscard]] std::string content() const {
        return ostream_.str();
    }

    operator std::string() const { // NOLINT
        using namespace Gem::Common;
        if(do_log_) {
            glogger(std::filesystem::path(exception_file))
                << "========================================================" << '\n'
                << "Error!" << '\n'
                << '\n'
                << where_and_when_ << '\n'
                << ostream_.str() << '\n'
                << '\n'
                << "If you suspect that there is an underlying problem with the" << '\n'
                << "Ge library collection, then please consider filing a bug." << '\n'
                << '\n'
                << "We appreciate your help!" << '\n'
                << "The Geneva team" << '\n'
                << '\n'
                << "========================================================" << '\n'
                << GFILE;
        }
        return ostream_.str();
    }

private:
    /**************************************************************************/
    // Data
    std::ostringstream ostream_;
    bool do_log_ = NO_LOG;
    const std::string exception_file = "./GENEVA-EXCEPTION.log";
    std::string where_and_when_;

    /**************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
