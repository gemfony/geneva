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
#include "common/GFactoryT.hpp"
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
 * The standard constructor for this class
 */
GDelayIndividualFactory::GDelayIndividualFactory(std::filesystem::path const &c_f)
  : Gem::Common::GFactoryT<gpar::GOptimizableEntity>(c_f) { /* nothing */
}

/******************************************************************************/
/**
 * The destructor
 */
GDelayIndividualFactory::~GDelayIndividualFactory() { /* nothing */
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the result file
 *
 * @return The Name of the result file
 */
std::string GDelayIndividualFactory::getResultFileName() const {
    return result_file_;
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the file holding the short measurement results
 *
 * @return The file name holding short measurement results
 */
std::string GDelayIndividualFactory::getShortResultFileName() const {
    return short_result_file_;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of delays provided by the user
 *
 * @return The number of delays provided by the user
 */
std::size_t GDelayIndividualFactory::getNDelays() const {
    return sleep_times_.size();
}

/******************************************************************************/
/**
 * Allows to retrieve the number of measurements to be made for each delay
 *
 * @return The number of measurements to be made for each delay
 */
std::uint32_t GDelayIndividualFactory::getNMeasurements() const {
    return n_measurements_;
}

/******************************************************************************/
/**
 * Retrieves the amount of seconds main() should wait between two measurements
 */
std::uint32_t GDelayIndividualFactory::getInterMeasurementDelay() const {
    return inter_measurement_delay_;
}

/******************************************************************************/
/**
 * Retrieves the sleep times
 *
 * @return The sleep times, as determined by this object
 */
std::vector<std::tuple<unsigned int, unsigned int>> GDelayIndividualFactory::getSleepTimes() const {
    return sleep_times_;
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<gpar::GOptimizableEntity> GDelayIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
) {
    // Will hold the result
    std::shared_ptr<GDelayIndividual> target(new GDelayIndividual());

    // Make the object's local configuration options known
    target->addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to describe configuration options of GDelayIndividual objects
 */
void GDelayIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
    gpb.registerFileParameter(
        "n_variables",
        n_variables_,
        n_variables_ // The default value
    ) << "The number of variables to act on";

    gpb.registerFileParameter(
        "delays",
        delays_,
        delays_ // The default value
    ) << "A list of delays through which main() should cycle. Format: seconds:milliseconds";

    gpb.registerFileParameter(
        "sleep_randomly",
        sleep_randomly_,
        sleep_randomly_ // The default value
    ) << "Indicates whether the individual should sleep for a random amount of time"
      << '\n'
      << "rather than a fixed amount of time";

    gpb.registerFileParameter(
        "lower_rand_sleep_boundary",
        lower_rand_sleep_boundary_,
        lower_rand_sleep_boundary_ // The default value
    ) << "The lower boundary for random sleep times in the"
      << '\n'
      << "fitness function (seconds, double value)";

    gpb.registerFileParameter(
        "upper_rand_sleep_boundary",
        upper_rand_sleep_boundary_,
        upper_rand_sleep_boundary_ // The default value
    ) << "The upper boundary for random sleep times in the"
      << '\n'
      << "fitness function (seconds, double value)";

    gpb.registerFileParameter(
        "result_file",
        result_file_,
        result_file_ // The default value
    ) << "The name of a file to which results should be stored";

    gpb.registerFileParameter(
        "short_result_file",
        short_result_file_,
        short_result_file_ // The default value
    ) << "The name of a file to which short results should be stored";

    gpb.registerFileParameter(
        "n_measurements",
        n_measurements_,
        n_measurements_ // The default value
    ) << "The number of measurements for each delay";

    gpb.registerFileParameter(
        "inter_measurement_delay",
        inter_measurement_delay_,
        inter_measurement_delay_ // The default value
    ) << "The amount of seconds to wait between two measurements";

    gpb.registerFileParameter<bool, double>(
        "may_throw" // The name of the variable
        ,
        "throw_likelihood",
        may_crash_ // The default value
        ,
        throw_likelihood_,
        [this](bool may_crash, double throw_likelihood) {
            may_crash_ = may_crash;
            // Enforce a throw_likelihood in the allowed value range
            throw_likelihood_ = Gem::Common::enforceRangeConstraint(
                throw_likelihood,
                0.,
                1.,
                "GDelayIndividual::describeLocalOptions_()"
            );
        },
        "throw_behaviour"
    ) << "Indicates whether the fitness function may throw after the sleep time"
      << Gem::Common::nextComment() << "Indicates the likelihood that the fitness function throws";
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p_raw A smart-pointer to be acted on during post-processing
 */
void GDelayIndividualFactory::postProcess_(std::shared_ptr<gpar::GOptimizableEntity> &p_raw) {
    // Retrieve information about our id
    std::size_t id = this->getId();

    // Make sure the textual delays are converted to time measurements
    sleep_times_ = Gem::Common::stringToUIntTupleVec(delays_);

    // Convert the base pointer to the target type
    std::shared_ptr<GDelayIndividual> p =
        Gem::Common::convertSmartPointer<gpar::GOptimizableEntity, GDelayIndividual>(p_raw);

    if(Gem::Common::GFACTORYWRITEID == id) {
        // Calculate the current sleep time
        std::chrono::duration<double> sleep_time = this->tupleToTime(sleep_times_.at(0));

        std::cout << "Producing individual in write mode with sleep time = " << sleep_time.count()
                  << " s" << '\n';

        p->setFixedSleepTime(sleep_time);

        p->setMayCrash(may_crash_, throw_likelihood_);
        p->setRandomSleep(
            sleep_randomly_,
            std::tuple<double, double>(lower_rand_sleep_boundary_, upper_rand_sleep_boundary_)
        );

        // Set up nVariables unbounded double parameters (structure only). This genome is pure transport
        // ballast for the overhead measurement -- the benchmark never adapts it, so no OA adaption config
        // is authored for it (it carries no adaptor, and customAdaptions() is a no-op).
        gpar::GGenomeBuilder gb;
        for(std::size_t var = 0; var < n_variables_; var++) {
            gb.addDouble(0.5);
        }
        p->setGenome(gb.build());
    }
    else if((id - Gem::Common::GFACTTORYFIRSTID) < sleep_times_.size()) {
        // Calculate the current sleep time
        std::chrono::duration<double> sleep_time =
            this->tupleToTime(sleep_times_.at(id - Gem::Common::GFACTTORYFIRSTID));

        std::cout << "Producing individual " << (id - Gem::Common::GFACTTORYFIRSTID)
                  << " with sleep time = " << sleep_time.count() << " s" << '\n';

        p->setFixedSleepTime(sleep_time);

        p->setMayCrash(may_crash_, throw_likelihood_);
        p->setRandomSleep(
            sleep_randomly_,
            std::tuple<double, double>(lower_rand_sleep_boundary_, upper_rand_sleep_boundary_)
        );

        // Set up nVariables unbounded double parameters (structure only). This genome is pure transport
        // ballast for the overhead measurement -- the benchmark never adapts it, so no OA adaption config
        // is authored for it (it carries no adaptor, and customAdaptions() is a no-op).
        gpar::GGenomeBuilder gb;
        for(std::size_t var = 0; var < n_variables_; var++) {
            gb.addDouble(0.5);
        }
        p->setGenome(gb.build());
    }
    else {
        // Return an empty pointer
        p_raw.reset();
    }
}

/******************************************************************************/
/**
 * Converts a tuple to a time format
 *
 * @param time_tuple A tuple of seconds and milliseconds in unsigned int format, to be converted to a time_duration object
 */
std::chrono::duration<double>
GDelayIndividualFactory::tupleToTime(const std::tuple<unsigned int, unsigned int> &time_tuple) {
    std::chrono::duration<double> t =
        std::chrono::seconds(Gem::Common::narrow<long>(std::get<0>(time_tuple))) +
        std::chrono::milliseconds(Gem::Common::narrow<long>(std::get<1>(time_tuple)));

    return t;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
