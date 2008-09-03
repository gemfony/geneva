/**
 * @file GSerializationHelperFunctions.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the Geneva library.
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
 */

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

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

#ifndef GSERIALIZATIONHELPERFUNCTIONS_HPP_
#define GSERIALIZATIONHELPERFUNCTIONS_HPP_

// GenEvA headers go here

#include "GIndividual.hpp"
#include "GLogger.hpp"
#include "GEnums.hpp"

namespace Gem
{
namespace GenEvA
{

/***************************************************************************************************/

/** @brief Convert a shared_ptr<GIndividual> into its string representation */
std::string indptrToString(const boost::shared_ptr<GIndividual>&, const serializationMode&);

/** @brief Load a shared_ptr<GIndividual> from its string representation */
boost::shared_ptr<GIndividual> indptrFromString(const std::string&, const serializationMode&);

/** @brief Puts a Gem::GenEvA::serializationMode into a stream */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::serializationMode&);

/** @brief Reads a Gem::GenEvA::serializationMode item from a stream */
std::istream& operator>>(std::istream&, Gem::GenEvA::serializationMode&);

/***************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GSERIALIZATIONHELPERFUNCTIONS_HPP_ */
