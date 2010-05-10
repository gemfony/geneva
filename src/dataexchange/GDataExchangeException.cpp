/**
 * @file GDataExchangeException.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRSPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GDataExchangeException.hpp"

namespace Gem
{
namespace Util
{

/****************************************************************************************/
/**
 * The default constructor. Initializes the error_ string with a sensible value.
 */
GDataExchangeException::GDataExchangeException() throw()
	:error_("An error occurred inside of the GDataExchange mechnism")
{ /* nothing */ }

/****************************************************************************************/
/**
 * A constructor that initializes the error_ string with a user-defined value
 */
GDataExchangeException::GDataExchangeException(const std::string& error) throw()
	:error_(error)
{ /* nothing */ }

/****************************************************************************************/
/**
 * The destructor
 */
GDataExchangeException::~GDataExchangeException() throw()
{ /* nothing */ }

/****************************************************************************************/
/**
 * Allows the retrieval of the error string
 *
 * @return The value of the error_ string
 */
const char* GDataExchangeException::what() const throw() {
	return error_.c_str();
}

/****************************************************************************************/

} /* namespace Util */
} /* namespace Gem */
