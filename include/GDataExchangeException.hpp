/**
 * @file GDataExchangeException.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here
#include <string>
#include <exception>

#ifndef GDATAEXCHANGEEXCEPTION_HPP_
#define GDATAEXCHANGEEXCEPTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


namespace Gem
{
namespace Util
{

/************************************************************************************/
/**
 * This class implements an exception type to be thrown in case of
 * errors inside the GDataExchange class.
 */
class GDataExchangeException
	: public std::exception
{
public:
	GDataExchangeException() throw();
	GDataExchangeException(const std::string&) throw();
	virtual ~GDataExchangeException() throw();
	virtual const char* what() const throw();

private:
	std::string error_;
};

/************************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGEEXCEPTION_HPP_ */
