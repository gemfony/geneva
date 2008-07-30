/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 * 
 * This file is part of Geneva, Gemfony scientific's optimization library.
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

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <cfloat>
#include <climits>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/strong_typedef.hpp>

#ifndef GHELPERFUNCTIONS_H_
#define GHELPERFUNCTIONS_H_

using namespace std;

#include "GException.hpp"

namespace GenEvA
{

  /** \brief The size of the command header. */
  enum { checksum_length = 32 };
  /** \brief The size of the command header. */
  enum { command_length = 32 };
  /** \brief The size of a fixed length header. */
  enum { header_length = 8 };

  /**************************************************************************************************/
  /**
   * \brief Emits a given number of white space characters
   */
  string ws(uint16_t);

  /**************************************************************************************************/
  /**
   * \brief Convert a double value into a string containing its bit
   * representation (01100...)
   */
  string d2s(double);

  /**************************************************************************************************/
  /**
   * \brief Convert a string containing sizeof(double)*8 0's and 1's
   * into a double. Characters other than 1 are interpreted as 0.
   */
  double s2d(string);

  /**************************************************************************************************/
  /**
   * \brief Find the smallest double for which "x+getMinDouble(x) > x"
   */
  double getMinDouble(double);

  /**************************************************************************************************/
  /**
   * \brief Checks whether string s1 ends with string s2
   */
  bool endsWith(string s1, string s2);

  /**************************************************************************************************/
  /**
   * \brief Calculates the checksum of a string.
   */
  // string checksum(const string&);

  /**************************************************************************************************/
  /**
   * \brief Assembles a query string from a command 
   */
  string assembleQueryString(const string&, uint16_t);

  /**************************************************************************************************/
  /**
   * \brief Extracts the size of a data section from a C string
   */
  size_t extractDataSize(const char*);

  /**************************************************************************************************/

} /* namespace GenEvA */

#endif /*GHELPERFUNCTIONS_H_*/
