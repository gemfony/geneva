/**
 * @file GDemoProcessingContainers.cpp
 */

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

#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "hap/GHapEnums.hpp"
#include "hap/GRandomT.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GSimpleContainer)       // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GRandomNumberContainer) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GFaultyContainer)       // NOLINT

namespace Gem::Courtier {

/********************************************************************************************/
/**
* The standard constructor -- Initialization with a single number (can e.g. be used as an id).
*
* @param snr The number to be stored in the object
*/
GSimpleContainer::GSimpleContainer(const std::size_t &snr)
  : Gem::Courtier::GProcessingContainerT<GSimpleContainer, bool>(1)
  , stored_number_(snr) { /* nothing */
}

/********************************************************************************************/
/**
* @brief Performs this object's processing task -- a no-op, as this class exists only for
* debugging and benchmarking. The input vector is ignored.
*/
void GSimpleContainer::process_([[maybe_unused]] const std::vector<bool> &res_vec) { /* nothing */
}

/********************************************************************************************/
/**
* @brief Prints this object's stored number to std::cout.
*/
void GSimpleContainer::print() const {
    std::cout << "storedNumber_ = " << stored_number_ << '\n';
}

/********************************************************************************************/
/**
 * The standard constructor -- Initialization with an amount of random numbers
 *
 * @param nrnr The desired amount of random numbers to be added to the random_numbers_ vector
 */
GRandomNumberContainer::GRandomNumberContainer(const std::size_t &nrnr)
  : Gem::Courtier::GProcessingContainerT<GRandomNumberContainer, bool>(1) {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;
    std::uniform_real_distribution<double> uniform_real_distribution;
    for(std::size_t i = 0; i < nrnr; i++) {
        random_numbers_.push_back(uniform_real_distribution(gr));
    }
}

/********************************************************************************************/
/**
 * @brief Performs this object's processing task: sorts the stored array of random numbers in place.
 * The input vector is ignored.
 */
void GRandomNumberContainer::process_([[maybe_unused]] const std::vector<bool> &res_vec) {
    std::sort(random_numbers_.begin(), random_numbers_.end());
}

/********************************************************************************************/
/**
 * @brief Prints this object's random-number container (index and value per line) to std::cout.
 */
void GRandomNumberContainer::print() const {
    for(std::size_t i = 0; i < random_numbers_.size(); i++) {
        std::cout << i << ": " << random_numbers_[i] << '\n';
    }
}

/********************************************************************************************/
/**
 * The standard constructor for the fault-injecting container.
 *
 * @param stored_number An id, used by tests to check item conservation
 * @param fm The fault to exhibit during process_()
 * @param sleep_ms The sleep length (ms) used by fault_mode::SLEEP
 */
GFaultyContainer::GFaultyContainer(std::size_t stored_number, fault_mode fm, unsigned int sleep_ms)
  : Gem::Courtier::GProcessingContainerT<GFaultyContainer, bool>(1)
  , stored_number_(stored_number)
  , fault_mode_(fm)
  , sleep_ms_(sleep_ms) { /* nothing */
}

/********************************************************************************************/
/**
 * @brief Performs this object's processing task by exhibiting the configured (mis-)behaviour.
 *
 * THROW_PROCESSING raises the dedicated g_processing_exception, which the worker is expected to
 * catch and turn into a flagged item; THROW_FATAL raises a plain std::runtime_error, which (pre-T1)
 * escapes the worker thread. The input vector is ignored.
 */
void GFaultyContainer::process_([[maybe_unused]] const std::vector<bool> &res_vec) {
    switch(fault_mode_) {
    case fault_mode::NONE:
        registerResult(0, true);
        break;

    case fault_mode::SLEEP:
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms_));
        registerResult(0, true);
        break;

    case fault_mode::FLAG_ERROR:
        force_set_error("GFaultyContainer: injected FLAG_ERROR");
        break;

    case fault_mode::THROW_PROCESSING:
        throw g_processing_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GFaultyContainer: injected g_processing_exception" << '\n'
        );

    case fault_mode::THROW_FATAL:
        throw std::runtime_error("GFaultyContainer: injected fatal std::runtime_error");
    }
}

/********************************************************************************************/
/**
 * @brief Retrieves the configured fault mode.
 *
 * @return The fault this container exhibits during process_()
 */
fault_mode GFaultyContainer::get_fault_mode() const {
    return fault_mode_;
}

/********************************************************************************************/
/**
 * @brief Retrieves the stored id number.
 *
 * @return The id used by tests to check item conservation
 */
std::size_t GFaultyContainer::get_stored_number() const {
    return stored_number_;
}

/********************************************************************************************/

} // namespace Gem::Courtier
