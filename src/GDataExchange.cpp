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
 * Adds a boost::shared_ptr to a GDoubleParameter to the corresponding vector
 *
 * @param gdp The boost::shared_ptr to the GDoubleParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GDoubleParameter>& gdp) {
	dArray_.push_back(gdp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GLongParameter to the corresponding vector
 *
 * @param glp The boost::shared_ptr to the GLongParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GLongParameter>& glp) {
	lArray_.push_back(ldp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GBoolParameter to the corresponding vector
 *
 * @param gbp The boost::shared_ptr to the GBoolParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GBoolParameter>& gbp) {
	bArray_.push_back(gbp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GCharParameter to the corresponding vector
 *
 * @param gcp The boost::shared_ptr to the GCharParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GCharParameter>& gcp) {
	cArray_.push_back(gcp);
}

/**************************************************************************/
/**
 * Writes the class'es data to a stream
 *
 * @param stream The external output stream to write to
 */
void GDataExchange::writeToStream(std::ostream& stream) const {
	std::size_t dArraySize = dArray_.size();
	stream << dArraySize;
	if(dArraySize) {
		std::vector<boost::shared_ptr<GDoubleParameter> >::iterator dit;
		for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) stream << **dit;
	}

	std::size_t lArraySize = lArray_.size();
	stream << lArraySize;
	if(lArraySize) {
		std::vector<boost::shared_ptr<GLongParameter> >::iterator lit;
		for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) stream << **lit;
	}

	std::size_t bArraySize = bArray_.size();
	stream << bArraySize;
	if(bArraySize) {
		std::vector<boost::shared_ptr<GBoolParameter> >::iterator bit;
		for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) stream << **bit;
	}

	std::size_t cArraySize = cArray_.size();
	stream << cArraySize;
	if(cArraySize) {
		std::vector<boost::shared_ptr<GCharParameter> >::iterator cit;
		for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) stream << **cit;
	}
}

/**************************************************************************/
/**
 * Reads the class'es data from a stream
 *
 * @param stream The external input stream to read from
 */
void GDataExchange::readFromStream(std::istream& stream) {
	std::size_t dArraySize;
	stream >> dArraySize;
	dArray_.clear();
	if(dArraySize) {
		for(std::size_t i=0; i<dArraySize; i++){
			boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter());
			stream >> *p;
			dArray_.push_back(p);
		}
	}

	std::size_t lArraySize;
	stream >> lArraySize;
	lArray_.clear();
	if(lArraySize) {
		for(std::size_t i=0; i<lArraySize; i++){
			boost::shared_ptr<GLongParameter> p(new GLongParameter());
			stream >> *p;
			lArray_.push_back(p);
		}
	}

	std::size_t bArraySize;
	stream >> bArraySize;
	bArray_.clear();
	if(bArraySize) {
		for(std::size_t i=0; i<bArraySize; i++){
			boost::shared_ptr<GBoolParameter> p(new GBoolParameter());
			stream >> *p;
			bArray_.push_back(p);
		}
	}

	std::size_t cArraySize;
	stream >> cArraySize;
	cArray_.clear();
	if(cArraySize) {
		for(std::size_t i=0; i<cArraySize; i++){
			boost::shared_ptr<GCharParameter> p(new GCharParameter());
			stream >> *p;
			cArray_.push_back(p);
		}
	}
}

/**************************************************************************/
/**
 * Helper function to aid IO  of this data set
 *
 * @param stream A reference to the output stream
 * @param A const reference to the data set
 * @return A copy of the stream
 */
std::ostream& operator<<(std::ostream& stream, const GDataExchange& object) {
	object.writeToStream(stream);
	return stream;
}

/*************************************************************************/
/**
 * Helper function to aid IO  of this data set
 *
 * @param stream A reference to the input stream
 * @param A reference to the data set
 * @return A copy of the stream
 */
std::istream& operator>>(std::istream& stream, GDataExchange& object) {
	object.readFromStream(stream);
	return stream;
}

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */
