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

#include "geneva-individuals/GMetaOptimizerIndividualT.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMetaOptimizerIndividualT<Gem::Geneva::GFunctionIndividual>)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Puts a Gem::Geneva::metaOptimizationTarget item into a stream
 *
 * @param o The ostream the item should be added to
 * @param mot the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::metaOptimizationTarget &mot) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(mot);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::metaOptimizationTarget item from a stream
 *
 * @param i The stream the item should be read from
 * @param mot The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::metaOptimizationTarget &mot) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
   mot = boost::numeric_cast<Gem::Geneva::metaOptimizationTarget>(tmp);
#else
	mot = static_cast<Gem::Geneva::metaOptimizationTarget>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
