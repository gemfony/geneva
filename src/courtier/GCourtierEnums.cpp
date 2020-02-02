/**
 * @file
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

#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************************/
/**
 * Puts a Gem::Courtier::run_state item into a stream
 *
 * @param o The ostream the item should be added to
 * @param rs the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::run_state &rs) {
	auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(rs);
	o << tmp;
	return o;
}

/******************************************************************************************/
/**
 * Reads a Gem::Courtier::run_state item from a stream
 *
 * @param i The stream the item should be read from
 * @param rs The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::run_state &rs) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	rs = boost::numeric_cast<Gem::Courtier::run_state>(tmp);
#else
	rs = static_cast<Gem::Courtier::run_state>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************************/
/**
 * Puts a Gem::Courtier::beast_payload_command item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ps the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::networked_consumer_payload_command &ps) {
	auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ps);
	o << tmp;
	return o;
}

/******************************************************************************************/
/**
 * Reads a Gem::Courtier::beast_payload_command item from a stream
 *
 * @param i The stream the item should be read from
 * @param ps The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::networked_consumer_payload_command &ps) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	ps = boost::numeric_cast<Gem::Courtier::networked_consumer_payload_command>(tmp);
#else
	ps = static_cast<Gem::Courtier::networked_consumer_payload_command>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************************/
/**
 * Puts a Gem::Courtier::beast_ping_state item into a stream
 *
 * @param o The ostream the item should be added to
 * @param srm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::beast_ping_state &ps) {
	auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ps);
	o << tmp;
	return o;
}

/******************************************************************************************/
/**
 * Reads a Gem::Courtier::beast_ping_state item from a stream
 *
 * @param i The stream the item should be read from
 * @param srm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::beast_ping_state &ps) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	ps = boost::numeric_cast<Gem::Courtier::beast_ping_state>(tmp);
#else
	ps = static_cast<Gem::Courtier::beast_ping_state>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Courtier::submissionReturnMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param srm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::submissionReturnMode &srm) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(srm);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Courtier::submissionReturnMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param srm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::submissionReturnMode &srm) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	srm = boost::numeric_cast<Gem::Courtier::submissionReturnMode>(tmp);
#else
	srm = static_cast<Gem::Courtier::submissionReturnMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Courtier::processingStatus item into a stream
 *
 * @param o The ostream the item should be added to
 * @param srm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::processingStatus &srm) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(srm);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Courtier::processingStatus item from a stream
 *
 * @param i The stream the item should be read from
 * @param srm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::processingStatus &srm) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	srm = boost::numeric_cast<Gem::Courtier::processingStatus>(tmp);
#else
	srm = static_cast<Gem::Courtier::processingStatus>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Courtier::brokerMode into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream& operator<<(std::ostream& o, const Gem::Courtier::consumerType& bm) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(bm);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Courtier::brokerMode item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream& operator>>(std::istream& i, Gem::Courtier::consumerType& bm) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	bm = boost::numeric_cast<Gem::Courtier::consumerType>(tmp);
#else
	bm = static_cast<Gem::Courtier::consumerType>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */
