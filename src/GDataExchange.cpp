/**
 * @file GDataExchange.cpp
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
 */

#include "GDataExchange.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Util::GDataExchange)

namespace Gem
{
namespace Util
{

/**************************************************************************/
/**
 * The default constructor
 */
GDataExchange::GDataExchange()
{ /* nothing */ }

/**************************************************************************/
/**
 * A standard copy constructor. We need to take care to copy the contents of
 * the vectors one by one, so that not only the pointers are copied.
 *
 * @param cp A constant reference to another GDataExchange object
 */
GDataExchange::GDataExchange(const GDataExchange& cp) {
	// Copy the double vector's content
	std::vector<GDoubleParameter>::const_iterator dcit;
	for(dcit=cp.dArray_.begin(); dcit!=cp.dArray_.end(); ++dcit) {
		boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(*(dcit->get())));
		dArray_.push_back(p);
	}

	// Copy the long vector's content
	std::vector<GLongParameter>::const_iterator lcit;
	for(lcit=cp.lArray_.begin(); lcit!=cp.lArray_.end(); ++lcit) {
		boost::shared_ptr<GLongParameter> p(new GLongParameter(*(lcit->get())));
		lArray_.push_back(p);
	}

	// Copy the bool vector's content
	std::vector<GBoolParameter>::const_iterator bcit;
	for(bcit=cp.bArray_.begin(); bcit!=cp.bArray_.end(); ++bcit) {
		boost::shared_ptr<GBoolParameter> p(new GBoolParameter(*(bcit->get())));
		bArray_.push_back(p);
	}

	// Copy the char vector's content
	std::vector<GCharParameter>::const_iterator ccit;
	for(ccit=cp.cArray_.begin(); ccit!=cp.cArray_.end(); ++ccit) {
		boost::shared_ptr<GCharParameter> p(new GCharParameter(*(ccit->get())));
		cArray_.push_back(p);
	}
}

/**************************************************************************/
/**
 * The destructor.
 */
virtual GDataExchange::~GDataExchange() {
	dArray_.clear();
	lArray_.clear();
	bArray_.clear();
	cArray_.clear();
}

/**************************************************************************/
/**
 * A standard assignment operator. Works like the copy constructor.
 *
 * @param cp A constant reference to another GDataExchange object
 */
const GDataExchange& GDataExchange::operator=(const GDataExchange& cp) {
	// Copy the double vector's content
	std::vector<GDoubleParameter>::const_iterator dcit;
	for(dcit=cp.dArray_.begin(); dcit!=cp.dArray_.end(); ++dcit) {
		boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(*(dcit->get())));
		dArray_.push_back(p);
	}

	// Copy the long vector's content
	std::vector<GLongParameter>::const_iterator lcit;
	for(lcit=cp.lArray_.begin(); lcit!=cp.lArray_.end(); ++lcit) {
		boost::shared_ptr<GLongParameter> p(new GLongParameter(*(lcit->get())));
		lArray_.push_back(p);
	}

	// Copy the bool vector's content
	std::vector<GBoolParameter>::const_iterator bcit;
	for(bcit=cp.bArray_.begin(); bcit!=cp.bArray_.end(); ++bcit) {
		boost::shared_ptr<GBoolParameter> p(new GBoolParameter(*(bcit->get())));
		bArray_.push_back(p);
	}

	// Copy the char vector's content
	std::vector<GCharParameter>::const_iterator ccit;
	for(ccit=cp.cArray_.begin(); ccit!=cp.cArray_.end(); ++ccit) {
		boost::shared_ptr<GCharParameter> p(new GCharParameter(*(ccit->get())));
		cArray_.push_back(p);
	}
}

/**************************************************************************/
/**
 * Saves the data of this class to a file
 */
bool GDataExchange::saveToFile(const std::string& fileName) {

}

/**************************************************************************/
/**
 * Loads the data of this class from a file
 */
bool GDataExchange::loadFromFile(const std::string& fileName) {

}

/**************************************************************************/
/**
 * Gives access to the data of type double
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> double&
GDataExchange::at<double>(std::size_t pos) {

}

/**************************************************************************/
/**
 * Gives access to the data of type boost::int32_t
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::int32_t&
GDataExchange::at<boost::int32_t>(std::size_t pos) {

}

/**************************************************************************/
/**
 * Gives access to the data of type bool
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> bool&
GDataExchange::at<bool>(std::size_t pos) {

}

/**************************************************************************/
/**
 * Gives access to the data of type char
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> char&
GDataExchange::at<char>(std::size_t pos) {

}

/**************************************************************************/
/**
 * Gives access to the size of a vector of type double
 *
 * @return The size of the vector of type double
 */
template <> std::size_t
GDataExchange::size<double>() {

}

/**************************************************************************/
/**
 * Gives access to the size of a vector of type boost::int32_t
 *
 * @return The size of the vector of type boost::int32_t
 */
template <> std::size_t
GDataExchange::size<boost::int32_t>() {

}

/**************************************************************************/
/**
 * Gives access to the size of a vector of type bool
 *
 * @return The size of the vector of type bool
 */
template <> std::size_t
GDataExchange::size<bool>() {

}

/**************************************************************************/
/**
 * Gives access to the size of a vector of type char
 *
 * @return The size of the vector of type char
 */
template <> std::size_t
GDataExchange::size<char>() {

}

/**************************************************************************/
/**
 * Allows to append data of type double to the corresponding vector.
 *
 * @param x The data to be appended to the corresponding vector
 */
template <> void
GDataExchange::append<double>(const double& x) {

}

/**************************************************************************/
/**
 * Allows to append data of type boost::int32_t to the corresponding vector.
 *
 * @param x The data to be appended to the corresponding vector
 */
template <> void
GDataExchange::append<boost::int32_t>(const boost::int32_t x) {

}

/**************************************************************************/
/**
 * Allows to append data of type bool to the corresponding vector.
 *
 * @param x The data to be appended to the corresponding vector
 */
template <> void
GDataExchange::append<bool>(const bool& x) {

}

/**************************************************************************/
/**
 * Allows to append data of type char to the corresponding vector.
 *
 * @param x The data to be appended to the corresponding vector
 */
template <> void
GDataExchange::append<char>(const char& x) {

}

/**************************************************************************/

} /* namespace Util */
} /* namespace Gem */
