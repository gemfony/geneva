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
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <thread>
#include <tuple>
#include <vector>

// Boost header files go here
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/tokenizer.hpp>

// Geneva header files go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GFactoryT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "hap/GRandomDistributionsT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * An exception to be thrown by the fitness function in order to simulate crashes
 */
class fitnessException : public std::exception {
public:
    using std::exception::exception;
};

/******************************************************************************/
/**
 * This individual waits for a predefined amount of time before returning the result of the evaluation
 * (which is random). Its purpose is to measure the overhead of the parallelization, compared
 * to the serial execution. It may also be used to track down problems in the broker, as the execution
 * time is well-defined, and the calculation of wait factors depends on fewer variables. Apart from fixed
 * "processing times" the individual may also wait random amounts of time in a predefined window, or may
 * crash with a predefined likelihood. This allows to test the stability of the communication between
 * clients and server.
 */
class GDelayIndividual
  : public GParameterSet // NOLINT(cppcoreguidelines-special-member-functions)
{
    /////////////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet) &
            BOOST_SERIALIZATION_NVP(fixedSleepTime_) & BOOST_SERIALIZATION_NVP(mayCrash_) &
            BOOST_SERIALIZATION_NVP(throwLikelihood_) & BOOST_SERIALIZATION_NVP(sleepRandomly_) &
            BOOST_SERIALIZATION_NVP(randSleepBoundaries_);
    }

    /////////////////////////////////////////////////////////////////////////////

public:
    /** The default constructor */
    GDelayIndividual();
    /** @brief A standard copy constructor */
    GDelayIndividual(const GDelayIndividual &);
    /** @brief The standard destructor */
    virtual ~GDelayIndividual();

    /** @brief Sets the sleep-time to a user-defined value */
    void setFixedSleepTime(const std::chrono::duration<double> &);
    /** @brief Retrieval of the current value of the fixedSleepTime_ variable */
    std::chrono::duration<double> getFixedSleepTime() const;

    /** @brief Indicate that the fitness function may crash at the end of the sleep time */
    void setMayCrash(bool, double);
    /** @brief Check whether the fitness function may crash at the end of the sleep time */
    bool getMayCrash() const;
    /** @brief Check the likelihood for a crash at the end of the sleep time */
    double getCrashLikelihood() const;

    /** @brief Indicates that the fitness function should sleep for a random time */
    void setRandomSleep(bool, std::tuple<double, double>);
    /** @brief Checks whether the fitness function has a random sleep schedule */
    bool getMaySleepRandomly() const;
    /** @brief Retrieves the time window for random sleeps */
    std::tuple<double, double> getSleepWindow() const;

protected:
    /** @brief Loads the data of another GDelayIndividual, camouflaged as a GObject */
    virtual void load_(const GObject *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDelayIndividual>(
        GDelayIndividual const &,
        GDelayIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    virtual void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual adaption operations */
    virtual std::size_t customAdaptions() final;
    /** @brief The actual fitness calculation takes place here */
    virtual double fitnessCalculation() final;

private:
    /** @brief Creates a deep clone of this object */
    virtual GObject *clone_() const final;

    double
        fixedSleepTime_; ///< The amount of time the evaluation function should sleep before continuing (seconds)

    bool mayCrash_ =
        false; ///< Indicates whether the fitness function may throw at the end of the sleep time
    double throwLikelihood_ =
        0.001; ///< The likelihood for an exception to be thrown from the fitness function

    bool sleepRandomly_ =
        false; /// Whether to sleep for a random amount of time instead of fixed amounts
    std::tuple<double, double> randSleepBoundaries_ = std::tuple<double, double>(
        0.,
        1.
    ); ///< Boundaries in seconds for random sleep (min/max amount of delay)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFMinIndividual objects
 */
class GDelayIndividualFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFactoryT<GParameterSet> {
public:
    /** @brief The standard constructor */
    GDelayIndividualFactory(std::filesystem::path const &);
    /** @brief The destructor */
    virtual ~GDelayIndividualFactory();

    /** @brief Allows to retrieve the name of the result file */
    std::string getResultFileName() const;
    /** @brief Allows to retrieve the name of the file holding the short measurement results */
    std::string getShortResultFileName() const;
    /** @brief Allows to retrieve the number of delays requested by the user */
    std::size_t getNDelays() const;
    /** @brief Allows to retrieve the number of measurements to be made for each delay */
    std::uint32_t getNMeasurements() const;
    /** @brief Retrieves the amount of seconds main() should wait between two measurements */
    std::uint32_t getInterMeasurementDelay() const;
    /** @brief Retrieves the sleep times */
    std::vector<std::tuple<unsigned int, unsigned int>> getSleepTimes() const;

protected:
    /** @brief Allows to describe local configuration options in derived classes */
    virtual void describeLocalOptions_(Gem::Common::GParserBuilder &) final;
    /** @brief Allows to act on the configuration options received from the configuration file */
    virtual void postProcess_(std::shared_ptr<GParameterSet> &) final;

private:
    /** @brief The default constructor. Only needed for (de-)serialization purposes */
    GDelayIndividualFactory() = default;

    /** @brief Creates individuals of this type */
    virtual std::shared_ptr<GParameterSet>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &) final;

    /** @brief Converts a tuple to a time format */
    std::chrono::duration<double> tupleToTime(const std::tuple<unsigned int, unsigned int> &);

    std::size_t nVariables_ = 100;
    std::string delays_ = "(0,1), (0,10), (0,100), (0,500), (1,0)";
    std::vector<std::tuple<unsigned int, unsigned int>> sleepTimes_;
    std::string resultFile_ = "fullResults.C";
    std::string shortResultFile_ = "shortDelayResults.txt";
    std::uint32_t nMeasurements_ = 10;        ///< The number of measurements for each delay
    std::uint32_t interMeasurementDelay_ = 1; ///< The delay between two measurements
    bool mayCrash_ =
        false; ///< Indicates whether the fitness function may throw at the end of the sleep time
    double throwLikelihood_ =
        0.001; ///< The likelihood for an exception to be thrown from the fitness function
    bool sleepRandomly_ =
        false; /// Whether to sleep for a random amount of time instead of fixed amounts
    double lowerRandSleepBoundary_ = 0.; ///< The lower boundary for random sleeps
    double upperRandSleepBoundary_ = 1.; ///< The upper boundary for random sleeps
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDelayIndividual) // NOLINT
