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
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

// Boost header files go here
#include <boost/serialization/nvp.hpp>

// Geneva header files go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "hap/GRandomDistributionsT.hpp"

namespace Gem::Common {
class GParserBuilder;
} // namespace Gem::Common

namespace Gem::Geneva::Individuals {

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
  : public gen::GFlatGenome // NOLINT(cppcoreguidelines-special-member-functions)
{
    /////////////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome) &
            BOOST_SERIALIZATION_NVP(fixed_sleep_time_) & BOOST_SERIALIZATION_NVP(may_crash_) &
            BOOST_SERIALIZATION_NVP(throw_likelihood_) & BOOST_SERIALIZATION_NVP(sleep_randomly_) &
            BOOST_SERIALIZATION_NVP(rand_sleep_boundaries_);
    }

    /////////////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GDelayIndividual();
    /**
     * @brief A standard copy constructor
     * @param cp The other GDelayIndividual object whose data is copied
     */
    GDelayIndividual(const GDelayIndividual &);
    /** @brief The standard destructor */
    ~GDelayIndividual() override;

    /**
     * @brief Sets the sleep-time to a user-defined value
     * @param sleepTime The fixed amount of time (in seconds) the fitness function should sleep
     */
    void setFixedSleepTime(const std::chrono::duration<double> &);
    /**
     * @brief Retrieval of the current value of the fixed_sleep_time_ variable
     * @return The fixed sleep time as a duration in seconds
     */
    std::chrono::duration<double> getFixedSleepTime() const;

    /**
     * @brief Indicate that the fitness function may crash at the end of the sleep time
     * @param mayCrash Whether the fitness function is allowed to throw
     * @param likelihood The probability with which a crash is triggered
     */
    void setMayCrash(bool, double);
    /**
     * @brief Check whether the fitness function may crash at the end of the sleep time
     * @return true if the fitness function may throw, false otherwise
     */
    bool getMayCrash() const;
    /**
     * @brief Check the likelihood for a crash at the end of the sleep time
     * @return The configured crash probability
     */
    double getCrashLikelihood() const;

    /**
     * @brief Indicates that the fitness function should sleep for a random time
     * @param randomSleep Whether random sleeps are enabled (instead of the fixed sleep time)
     * @param window The (min, max) time window in seconds within which random sleeps are drawn
     */
    void setRandomSleep(bool, std::tuple<double, double>);
    /**
     * @brief Checks whether the fitness function has a random sleep schedule
     * @return true if random sleeps are enabled, false otherwise
     */
    bool getMaySleepRandomly() const;
    /**
     * @brief Retrieves the time window for random sleeps
     * @return The (min, max) random-sleep window in seconds
     */
    std::tuple<double, double> getSleepWindow() const;

    /***************************************************************************/
    /**
     * The configuration the delay benchmarks read for a series of delay individuals. This individual
     * is only ever used by benchmarks/tests (it performs no real optimisation), so -- rather than going
     * through a factory -- it owns its own config parsing and construction through the static helpers
     * below: the benchmark reads a Config, then drives the delay sequence itself.
     */
    struct Config {
        std::size_t n_variables = 100; ///< Number of (transport-ballast) double parameters in the genome
        std::string delays =
            "(0,1), (0,10), (0,100), (0,500), (1,0)"; ///< The list of (s,ms) delays to cycle through
        bool sleep_randomly = false;                  ///< Sleep for a random time rather than a fixed one
        double lower_rand_sleep_boundary = 0.;        ///< Lower boundary for random sleeps (seconds)
        double upper_rand_sleep_boundary = 1.;        ///< Upper boundary for random sleeps (seconds)
        std::string result_file = "fullResults.C";    ///< File for the full results
        std::string short_result_file = "shortDelayResults.txt"; ///< File for the short results
        std::uint32_t n_measurements = 10;            ///< Number of measurements per delay
        std::uint32_t inter_measurement_delay = 1;    ///< Seconds to wait between two measurements
        bool may_crash = false;                       ///< Whether the fitness function may throw
        double throw_likelihood = 0.001;              ///< Likelihood of a throw from the fitness function
    };

    /**
     * @brief Registers the delay configuration options, binding them to the passed Config
     * @param gpb The parser builder the configuration options are registered with
     * @param c The Config object whose members the options are bound to
     */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c);
    /**
     * @brief Reads a delay configuration file (creating it with defaults if it does not exist)
     * @param configFile Path to the delay configuration file
     * @return The parsed Config object
     */
    static Config readConfig(std::filesystem::path const &configFile);
    /**
     * @brief Parses the textual "delays" list of a Config into (seconds, milliseconds) tuples
     * @param c The Config whose "delays" string is parsed
     * @return A vector of (seconds, milliseconds) tuples
     */
    static std::vector<std::tuple<unsigned int, unsigned int>> parseSleepTimes(const Config &c);
    /**
     * @brief Converts a (seconds, milliseconds) tuple to a duration
     * @param t The (seconds, milliseconds) tuple to convert
     * @return The corresponding duration in seconds
     */
    static std::chrono::duration<double> tupleToTime(const std::tuple<unsigned int, unsigned int> &);
    /**
     * @brief Builds a configured delay individual (genome = n_variables doubles) for one fixed sleep time
     * @param c The Config supplying genome size and crash / random-sleep settings
     * @param sleepTime The fixed sleep time (in seconds) assigned to the produced individual
     * @return A shared pointer to the newly built delay individual
     */
    static std::shared_ptr<GDelayIndividual>
    create(const Config &c, const std::chrono::duration<double> &sleepTime);

protected:
    /**
     * @brief Single declaration of this class'es local data members
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("fixed_sleep_time_", self.fixed_sleep_time_),
            Gem::Common::make_member("may_crash_", self.may_crash_),
            Gem::Common::make_member("throw_likelihood_", self.throw_likelihood_),
            Gem::Common::make_member("sleep_randomly_", self.sleep_randomly_),
            Gem::Common::make_member("rand_sleep_boundaries_", self.rand_sleep_boundaries_)
        );
    }
    auto localMembers() { return localMembers_(*this); }       // NOLINT -- intentionally hides the base localMembers()
    auto localMembers() const { return localMembers_(*this); } // NOLINT -- intentionally hides the base localMembers()

    /**
     * @brief Loads the data of another GDelayIndividual, camouflaged as a GFlatGenome
     * @param cp Pointer to the other object (a GDelayIndividual passed as a base-class pointer)
     */
    void load_(const gen::GOptimizableEntity *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDelayIndividual>(
        GDelayIndividual const &,
        GDelayIndividual const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp The other object to compare against (passed as a base-class reference)
     * @param e The expectation for this object, e.g. equality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        const gen::GOptimizableEntity & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /**
     * @brief The actual fitness calculation takes place here
     * @return A random fitness value, returned after the configured sleep time
     */
    double fitnessCalculation() final;

private:
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a newly allocated deep copy of this object
     */
    gen::GFlatGenome *clone_() const final;

    double
        fixed_sleep_time_; ///< The amount of time the evaluation function should sleep before continuing (seconds)

    bool may_crash_ =
        false; ///< Indicates whether the fitness function may throw at the end of the sleep time
    double throw_likelihood_ =
        0.001; ///< The likelihood for an exception to be thrown from the fitness function

    bool sleep_randomly_ =
        false; /// Whether to sleep for a random amount of time instead of fixed amounts
    std::tuple<double, double> rand_sleep_boundaries_ = std::tuple<double, double>(
        0.,
        1.
    ); ///< Boundaries in seconds for random sleep (min/max amount of delay)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GDelayIndividual) // NOLINT
