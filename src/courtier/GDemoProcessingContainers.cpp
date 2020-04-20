/**
 * @file GDemoProcesiingContainers.cpp
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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "courtier/GDemoProcessingContainers.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GSimpleContainer)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GRandomNumberContainer)

namespace Gem {
namespace Courtier {

/********************************************************************************************/
/**
* The standard constructor -- Initialization with a single number (can e.g. be used as an id).
*
* @param snr The number to be stored in the object
*/
GSimpleContainer::GSimpleContainer(const std::size_t& snr)
    : Gem::Courtier::GProcessingContainerT<GSimpleContainer, bool>(1)
    , m_stored_number(snr)
{ /* nothing */ }

/********************************************************************************************/
/**
* Allows to specify the tasks to be performed for this object. We simply do nothing,
* as this class is for debugging and benchmarking purposes only.
*/
void GSimpleContainer::process_() { /* nothing */ }

/********************************************************************************************/
/**
* Prints out this functions stored number
*/
void GSimpleContainer::print() {
    std::cout << "storedNumber_ = " << m_stored_number << std::endl;
}

/********************************************************************************************/
/**
 * The standard constructor -- Initialization with an amount of random numbers
 *
 * @param nrnr The desired amount of random numbers to be added to the randomNumbers_ vector
 */
GRandomNumberContainer::GRandomNumberContainer(const std::size_t& nrnr)
    : Gem::Courtier::GProcessingContainerT<GRandomNumberContainer, bool>(1)
{
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;
    std::uniform_real_distribution<double> uniform_real_distribution;
    for(std::size_t i=0; i<nrnr; i++) {
        randomNumbers_.push_back(uniform_real_distribution(gr));
    }
}

/********************************************************************************************/
/**
 * Allows to specify the tasks to be performed for this object. We simply sort the array of
 * random numbers.
 */
void GRandomNumberContainer::process_() {
    std::sort(randomNumbers_.begin(), randomNumbers_.end());
}

/********************************************************************************************/
/**
 * Prints out this functions random number container
 */
void GRandomNumberContainer::print() {
    for(std::size_t i=0; i<randomNumbers_.size(); i++) {
        std::cout << i << ": " << randomNumbers_[i] << std::endl;
    }
}

/********************************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */
