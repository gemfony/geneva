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
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

// Boost header files go here
#include <boost/archive/basic_archive.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

// Geneva header files go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GDefaultValueT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializeTupleT.hpp"
#include "common/GTupleIO.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "hap/GRandomT.hpp"

#ifdef GEM_TESTING

#include "common/GUnitTestFrameworkT.hpp"

#endif /* GEM_TESTING */

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem::Geneva {

/******************************************************************************/
/**
 * GObject is the parent class for the majority of Geneva optimization classes.
 * Handling of optimization-related classes sometimes happens through a
 * std::shared_ptr<GObject> or std::unique_ptr<GObject>, hence this class has a
 * very central role.
 */
class GObject : public Gem::Common::GCommonInterfaceT<GObject> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive & /*ar*/, const unsigned int) {
        using boost::serialization::make_nvp;

        /* nothing */
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators -- rule of five

    GObject() = default;
    GObject(GObject const &cp) = default;
    GObject(GObject &&cp) = default;

    virtual ~GObject() = default;

    GObject &operator=(GObject const &) = default;
    GObject &operator=(GObject &&) = default;

    /***************************************************************************/
    /**
     * Checks whether a SIGHUP or CTRL_CLOSE_EVENT signal has been sent
     */
    static bool G_SIGHUP_SENT() {
        return (1 == GObject::GenevaSigHupSent);
    }

    /***************************************************************************/
    /**
     * A handler for SIGHUP or CTRL_CLOSE_EVENT signals. This function should work
     * both for Windows and Unix-Systems.
     */
    static void sigHupHandler(int signum) {
        if(G_SIGHUP == signum) {
            GObject::GenevaSigHupSent = 1;
        }
    }

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GObject */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void
    Gem::Common::compare_base_t<GObject>(GObject const &, GObject const &, Gem::Common::GToken &);

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override = 0;

    // Needed to allow interruption of the optimization run without loss of data
    // Npte that "volatile" is needed in order for the signal handler to work
    static volatile std::sig_atomic_t GenevaSigHupSent; // Initialized in GObject.cpp
};

/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GObject) // NOLINT
