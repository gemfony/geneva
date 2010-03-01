/**
 * @file GSerializationHelperFunctions.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>

#ifndef GSERIALIZATIONHELPERFUNCTIONS_HPP_
#define GSERIALIZATIONHELPERFUNCTIONS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here

#include "GIndividual.hpp"
#include "GEnums.hpp"

namespace Gem
{
namespace GenEvA
{

/***************************************************************************************************/

/** @brief Convert a shared_ptr<GIndividual> into its string representation */
std::string indptrToString(boost::shared_ptr<GIndividual>, const serializationMode&);

/** @brief Load a shared_ptr<GIndividual> from its string representation */
boost::shared_ptr<GIndividual> indptrFromString(const std::string&, const serializationMode&);

/** @brief Puts a Gem::GenEvA::serializationMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::serializationMode&);

/** @brief Reads a Gem::GenEvA::serializationMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::serializationMode&);

/** @brief Puts a Gem::GenEvA::dataExchangeMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::dataExchangeMode&);

/** @brief Reads a Gem::GenEvA::dataExchangeMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::dataExchangeMode&);

/** @brief Puts a Gem::GenEvA::recoScheme into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::recoScheme&);

/** @brief Reads a Gem::GenEvA::recoScheme item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::recoScheme&);

/** @brief Puts a Gem::GenEvA::infoMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::infoMode&);

/** @brief Reads a Gem::GenEvA::infoMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::infoMode&);

/** @brief Puts a Gem::GenEvA::adaptorId into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::adaptorId&);

/** @brief Reads a Gem::GenEvA::adaptorId item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::adaptorId&);

/** @brief Puts a Gem::GenEvA::sortingMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::sortingMode&);

/** @brief Reads a Gem::GenEvA::sortingMode from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::sortingMode&);

/***************************************************************************************************/

} /* namespace GenEvA */

namespace Util
{
/***************************************************************************************************/

/** @brief Puts a Gem::Util::rnrGenerationMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Util::rnrGenerationMode&);

/** @brief Reads a Gem::Util::rnrGenerationMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Util::rnrGenerationMode&);

/** @brief Puts a Gem::Util::triboolStates into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Util::triboolStates&);

/** @brief Reads a Gem::Util::triboolStates from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Util::triboolStates&);

/***************************************************************************************************/

} /* namespace Util */

} /* namespace Gem */

#endif /* GSERIALIZATIONHELPERFUNCTIONS_HPP_ */
