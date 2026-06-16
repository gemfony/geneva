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

#include "geneva/individuals/GDelayIndividual.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <tuple>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GDelayIndividual) // NOLINT
namespace Gem::Geneva::Individuals {

/******************************************************************************/
/**
 * The default constructor. Intentionally private -- needed only for (de-)serialization.
 */
GDelayIndividual::GDelayIndividual()
  : fixed_sleep_time_(1.) { /* nothing */
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GDelayIndividual
 */
GDelayIndividual::GDelayIndividual(const GDelayIndividual &cp)
  : gpar::GFlatGenome(cp)
  , fixed_sleep_time_(cp.fixed_sleep_time_)
  , may_crash_(cp.may_crash_)
  , throw_likelihood_(cp.throw_likelihood_)
  , sleep_randomly_(cp.sleep_randomly_)
  , rand_sleep_boundaries_(cp.rand_sleep_boundaries_) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GDelayIndividual::~GDelayIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GFlatGenome object
 * @param e The expected outcome of the comparison
 */
void GDelayIndividual::compare_(
    const gpar::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDelayIndividual reference independent of this object and convert the pointer
    const GDelayIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GDelayIndividual>(cp, this);

    Gem::Common::GToken token("GDelayIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GFlatGenome>(*this, *p_load, token);

    // ... and then the local data
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GDelayIndividual, camouflaged as a GFlatGenome
 *
 * @param cp A copy of another GDelayIndividual, camouflaged as a GFlatGenome
 */
void GDelayIndividual::load_(const gpar::GOptimizableEntity *cp) {
    // Check that we are dealing with a GDelayIndividual reference independent of this object and convert the pointer
    const GDelayIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GDelayIndividual>(cp, this);

    // Load our parent class'es data ...
    gpar::GFlatGenome::load_(cp);

    // ... and then our own, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gpar::GFlatGenome *GDelayIndividual::clone_() const {
    return new GDelayIndividual(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GDelayIndividual::fitnessCalculation() {
    std::uniform_real_distribution<double> uniform_real_distribution;

    if(sleep_randomly_) {
        // Calculate the sleep time
        double sleep_time = uniform_real_distribution(
            gr_,
            std::uniform_real_distribution<double>::param_type(
                std::get<0>(rand_sleep_boundaries_),
                std::get<1>(rand_sleep_boundaries_)
            )
        );

        std::chrono::duration<double> random_sleep_time(sleep_time);

        // Sleep for a random amount of time in a given time window
        std::this_thread::sleep_for(random_sleep_time);
    }
    else {
        // Sleep for a fixed amount of time
        std::this_thread::sleep_for(std::chrono::duration<double>(fixed_sleep_time_));
    }

    // Throw if we were asked to do so
    if(may_crash_) {
        if(uniform_real_distribution(
               gr_,
               std::uniform_real_distribution<double>::param_type(0., 1.)
           ) < throw_likelihood_) {
            throw fitnessException();
        }
    }

    // Return a random value - we do not perform any real optimization
    return uniform_real_distribution(
        gr_,
        std::uniform_real_distribution<double>::param_type(0., 1.)
    );
}

/******************************************************************************/
/**
 * Retrieval of the current value of the fixed_sleep_time_ variable
 *
 * @return The current value of the fixed_sleep_time_ variable
 */
std::chrono::duration<double> GDelayIndividual::getFixedSleepTime() const {
    return std::chrono::duration<double>(fixed_sleep_time_);
}

/******************************************************************************/
/**
 * Sets the sleep-time to a user-defined value
 */
void GDelayIndividual::setFixedSleepTime(const std::chrono::duration<double> &sleep_time) {
    fixed_sleep_time_ = sleep_time.count();
}

/******************************************************************************/
/**
 * Indicate that the fitness function may crash at the end of the sleep time,
 * and set the likelihood for such a crash. The likelihood may assume values
 * between (and including) 0 (no crash) and 1 (always crash).
 */
void GDelayIndividual::setMayCrash(bool may_crash, double throw_likelihood) {
    may_crash_ = may_crash;

    // Enforce a throw_likelihood in the allowed value range
    throw_likelihood_ = Gem::Common::enforceRangeConstraint(
        throw_likelihood,
        0.,
        1.,
        "GDelayIndividual::setMayCrash()"
    );
}

/******************************************************************************/
/**
 * Check whether the fitness function may crash at the end of the sleep time
 */
bool GDelayIndividual::getMayCrash() const {
    return may_crash_;
}

/******************************************************************************/
/**
 * Check the likelihood for a crash at the end of the sleep time
 */
double GDelayIndividual::getCrashLikelihood() const {
    return throw_likelihood_;
}

/******************************************************************************/
/**
 * Indicates that the fitness function should sleep for a random time. The lower
 * and upper boundaries for the sleep period are passed as a std::tuple, double
 * values indicate seconds (and fractions thereof).
 */
void GDelayIndividual::setRandomSleep(
    bool sleep_randomly,
    std::tuple<double, double> rand_sleep_boundaries
) {
    sleep_randomly_ = sleep_randomly;

    // Enforce that the sanity of the lower and upper boundaries
    if(std::get<0>(rand_sleep_boundaries) < 0. ||
       std::get<0>(rand_sleep_boundaries) >= std::get<1>(rand_sleep_boundaries)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDelayIndividual::setRandomSleep(): Error!" << '\n'
            << "Got invalid boundaries for the sleep time: " << std::get<0>(rand_sleep_boundaries)
            << " / " << std::get<1>(rand_sleep_boundaries) << '\n'
        );
    }

    rand_sleep_boundaries_ = rand_sleep_boundaries;
}

/******************************************************************************/
/**
 * Checks whether the fitness function has a random sleep schedule
 */
bool GDelayIndividual::getMaySleepRandomly() const {
    return sleep_randomly_;
}

/******************************************************************************/
/**
 * Retrieves the time window for random sleeps
 */
std::tuple<double, double> GDelayIndividual::getSleepWindow() const {
    return rand_sleep_boundaries_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Registers the delay configuration options, binding them to the passed Config. This is the body of the
 * former GDelayIndividualFactory::describeLocalOptions_, now owned by the individual itself.
 */
void GDelayIndividual::describeConfig(Gem::Common::GParserBuilder &gpb, Config &c) {
    gpb.registerFileParameter("n_variables", c.n_variables, c.n_variables)
        << "The number of variables to act on";

    gpb.registerFileParameter("delays", c.delays, c.delays)
        << "A list of delays through which the benchmark should cycle. Format: seconds:milliseconds";

    gpb.registerFileParameter("sleep_randomly", c.sleep_randomly, c.sleep_randomly)
        << "Indicates whether the individual should sleep for a random amount of time" << '\n'
        << "rather than a fixed amount of time";

    gpb.registerFileParameter(
        "lower_rand_sleep_boundary",
        c.lower_rand_sleep_boundary,
        c.lower_rand_sleep_boundary
    ) << "The lower boundary for random sleep times in the" << '\n'
      << "fitness function (seconds, double value)";

    gpb.registerFileParameter(
        "upper_rand_sleep_boundary",
        c.upper_rand_sleep_boundary,
        c.upper_rand_sleep_boundary
    ) << "The upper boundary for random sleep times in the" << '\n'
      << "fitness function (seconds, double value)";

    gpb.registerFileParameter("result_file", c.result_file, c.result_file)
        << "The name of a file to which results should be stored";

    gpb.registerFileParameter("short_result_file", c.short_result_file, c.short_result_file)
        << "The name of a file to which short results should be stored";

    gpb.registerFileParameter("n_measurements", c.n_measurements, c.n_measurements)
        << "The number of measurements for each delay";

    gpb.registerFileParameter("inter_measurement_delay", c.inter_measurement_delay, c.inter_measurement_delay)
        << "The amount of seconds to wait between two measurements";

    gpb.registerFileParameter<bool, double>(
        "may_throw",
        "throw_likelihood",
        c.may_crash,
        c.throw_likelihood,
        [&c](bool may_crash, double throw_likelihood) {
            c.may_crash = may_crash;
            // Enforce a throw_likelihood in the allowed value range
            c.throw_likelihood = Gem::Common::enforceRangeConstraint(
                throw_likelihood,
                0.,
                1.,
                "GDelayIndividual::describeConfig()"
            );
        },
        "throw_behaviour"
    ) << "Indicates whether the fitness function may throw after the sleep time"
      << Gem::Common::nextComment() << "Indicates the likelihood that the fitness function throws";
}

/******************************************************************************/
/**
 * Reads a delay configuration file, creating it with default values if it does not yet exist. Replaces
 * the legacy factory's config parsing; the benchmark calls this once and then drives the delay sequence.
 */
GDelayIndividual::Config GDelayIndividual::readConfig(std::filesystem::path const &configFile) {
    Config c;
    Gem::Common::GParserBuilder gpb;
    describeConfig(gpb, c);

    if(not gpb.parseConfigFile(configFile)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDelayIndividual::readConfig(): Error!" << '\n'
            << "Could not parse configuration file " << configFile.string() << '\n'
        );
    }

    return c;
}

/******************************************************************************/
/**
 * Parses the textual "delays" list of a Config into (seconds, milliseconds) tuples
 */
std::vector<std::tuple<unsigned int, unsigned int>> GDelayIndividual::parseSleepTimes(const Config &c) {
    return Gem::Common::stringToUIntTupleVec(c.delays);
}

/******************************************************************************/
/**
 * Converts a tuple to a time format
 *
 * @param time_tuple A tuple of seconds and milliseconds in unsigned int format
 */
std::chrono::duration<double>
GDelayIndividual::tupleToTime(const std::tuple<unsigned int, unsigned int> &time_tuple) {
    std::chrono::duration<double> t =
        std::chrono::seconds(Gem::Common::narrow<long>(std::get<0>(time_tuple))) +
        std::chrono::milliseconds(Gem::Common::narrow<long>(std::get<1>(time_tuple)));

    return t;
}

/******************************************************************************/
/**
 * Builds a configured delay individual for one fixed sleep time. Replaces the legacy factory's
 * postProcess_: the genome is n_variables unbounded double parameters (structure only) -- pure transport
 * ballast for the overhead measurement, carrying no adaptor (customAdaptions() is a no-op).
 */
std::shared_ptr<GDelayIndividual>
GDelayIndividual::create(const Config &c, const std::chrono::duration<double> &sleepTime) {
    std::shared_ptr<GDelayIndividual> p(new GDelayIndividual());

    p->setFixedSleepTime(sleepTime);
    p->setMayCrash(c.may_crash, c.throw_likelihood);
    p->setRandomSleep(
        c.sleep_randomly,
        std::tuple<double, double>(c.lower_rand_sleep_boundary, c.upper_rand_sleep_boundary)
    );

    gpar::GGenomeBuilder gb;
    for(std::size_t var = 0; var < c.n_variables; var++) {
        gb.addDouble(0.5);
    }
    p->setGenome(gb.build());

    return p;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
