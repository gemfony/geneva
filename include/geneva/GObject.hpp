/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <functional>
#include <memory>
#include <tuple>
#include <limits>

// Boost header files go here
#include <boost/any.hpp>
#include <boost/archive/basic_archive.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/numeric/conversion/bounds.hpp> // get rid of the numeric_limits<double>::min() vs. numeric_limits<int>::min() problem
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

#ifdef GEM_TESTING

#include <boost/test/unit_test.hpp>

#endif /* GEM_TESTING */

// Geneva header files go here
#include "common/GDefaultValueT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GSerializeTupleT.hpp"
#include "common/GTupleIO.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

#ifdef GEM_TESTING

#include "common/GUnitTestFrameworkT.hpp"

#endif /* GEM_TESTING */

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GObject is the parent class for the majority of Geneva optimization classes.
 * Handling of optimization-related classes sometimes happens through a
 * std::shared_ptr<GObject> or std::unique_ptr<GObject>, hence this class has a
 * very central role.
 */
class GObject
    :
        public Gem::Common::GCommonInterfaceT<GObject>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        /* nothing */
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    G_API_GENEVA GObject() = default;
    /** @brief The copy constructor */
    G_API_GENEVA GObject(const GObject &cp) = default;
    /** @brief The destructor */
    G_API_GENEVA virtual ~GObject() = default;

    /***************************************************************************/
    /**
     * Checks whether a SIGHUP or CTRL_CLOSE_EVENT signal has been sent
     */
    static G_API_GENEVA bool G_SIGHUP_SENT() {
        return (1 == GObject::GenevaSigHupSent);
    }

    /***************************************************************************/
    /**
     * A handler for SIGHUP or CTRL_CLOSE_EVENT signals. This function should work
     * both for Windows and Unix-Systems.
     */
    static G_API_GENEVA void sigHupHandler(int signum) {
        if (G_SIGHUP == signum) {
            GObject::GenevaSigHupSent = 1;
        }
    }

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GObject */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GObject>(
        GObject const &
        , GObject const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject & // the other object
        , const Gem::Common::expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override = 0;

    // Needed to allow interruption of the optimization run without loss of data
    // Npte that "volatile" is needed in order for the signal handler to work
    static volatile G_API_GENEVA std::sig_atomic_t GenevaSigHupSent;  // Initialized in GObject.cpp

public:
    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests() override;

    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;

    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GObject)

