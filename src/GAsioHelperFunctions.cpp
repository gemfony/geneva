/**
 * @file GAsioHelperFunctions.cpp
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

#include "GAsioHelperFunctions.hpp"

namespace Gem
{
namespace GenEvA
{

/**********************************************************************************/
/**
 * Assembles a query string from a given command, emitting a string of a given size.
 * This function is used in conjunction with Boost::Asio .
 *
 * \param str The string from which the size should be extracted
 * \param sz Resulting size of the query string
 * \return The query string
 */
std::string assembleQueryString(const std::string& query, const std::size_t& sz){
  std::ostringstream query_stream;
  query_stream << std::setw(sz) << query;
  return query_stream.str();
}

/**********************************************************************************/
/**
 * Extracts the size of ASIO's data section from a C string.
 * Used in conjunction with Boost::Asio. See e.g. GAsioTCPClient.
 *
 * \param ds The data string holding the data size
 * \param sz The size of the data string
 * \return The size if the data
 */
std::size_t extractDataSize(const char* ds, const std::size_t& sz){
  std::istringstream is(std::string(ds, sz));
  std::size_t inboundDataSize = 0;
  if (!(is >> std::hex >> inboundDataSize)) {
	  std::ostringstream error;
	  error << "In extractDataSize: Got invalid header!" << std::endl;
	  throw geneva_error_condition(error.str());
  }

  return inboundDataSize;
}

/**********************************************************************************/


} /* namespace GenEvA */
} /* namespace Gem */
