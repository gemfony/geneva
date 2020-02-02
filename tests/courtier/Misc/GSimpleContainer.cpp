/**
 * @file GSimpleContainer.cpp
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

#include "GSimpleContainer.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::Tests::GSimpleContainer)

namespace Gem {
namespace Courtier {
namespace Tests {

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

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */
