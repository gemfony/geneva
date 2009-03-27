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
	:value_(0.),
	 hasValue_(false)
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
	std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit;
	for(dcit=cp.dArray_.begin(); dcit!=cp.dArray_.end(); ++dcit) {
		boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(*(dcit->get())));
		dArray_.push_back(p);
	}

	// Copy the long vector's content
	std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit;
	for(lcit=cp.lArray_.begin(); lcit!=cp.lArray_.end(); ++lcit) {
		boost::shared_ptr<GLongParameter> p(new GLongParameter(*(lcit->get())));
		lArray_.push_back(p);
	}

	// Copy the bool vector's content
	std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit;
	for(bcit=cp.bArray_.begin(); bcit!=cp.bArray_.end(); ++bcit) {
		boost::shared_ptr<GBoolParameter> p(new GBoolParameter(*(bcit->get())));
		bArray_.push_back(p);
	}

	// Copy the char vector's content
	std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit;
	for(ccit=cp.cArray_.begin(); ccit!=cp.cArray_.end(); ++ccit) {
		boost::shared_ptr<GCharParameter> p(new GCharParameter(*(ccit->get())));
		cArray_.push_back(p);
	}

	value_ = cp.value_;
	hasValue_ = cp.hasValue_;
}

/**************************************************************************/
/**
 * The destructor.
 */
GDataExchange::~GDataExchange() {
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
	std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit;
	for(dcit=cp.dArray_.begin(); dcit!=cp.dArray_.end(); ++dcit) {
		boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(*(dcit->get())));
		dArray_.push_back(p);
	}

	// Copy the long vector's content
	std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit;
	for(lcit=cp.lArray_.begin(); lcit!=cp.lArray_.end(); ++lcit) {
		boost::shared_ptr<GLongParameter> p(new GLongParameter(*(lcit->get())));
		lArray_.push_back(p);
	}

	// Copy the bool vector's content
	std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit;
	for(bcit=cp.bArray_.begin(); bcit!=cp.bArray_.end(); ++bcit) {
		boost::shared_ptr<GBoolParameter> p(new GBoolParameter(*(bcit->get())));
		bArray_.push_back(p);
	}

	// Copy the char vector's content
	std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit;
	for(ccit=cp.cArray_.begin(); ccit!=cp.cArray_.end(); ++ccit) {
		boost::shared_ptr<GCharParameter> p(new GCharParameter(*(ccit->get())));
		cArray_.push_back(p);
	}

	value_ = cp.value_;
	hasValue_ = cp.hasValue_;
}

/**************************************************************************/
/**
 * Resets all data structures of the object
 */
void GDataExchange::reset() {
	dArray_.clear();
	lArray_.clear();
	bArray_.clear();
	cArray_.clear();
	value_ = 0.;
	hasValue_ = false;
}

/**************************************************************************/
/**
 * Assigns a value to the current data set. Doing so is allowed only once,
 * unless the object is reset.
 *
 * @param value The value to assign to the current data set
 */
void GDataExchange::setValue(double value){
	// Prevent setting of a value of one has already been assigned
	if(hasValue_) {
		std::cerr << "In GDataExchange::setValue(): Error!" << std::endl
		          << "Trying to assign a value when one has" << std::endl
		          << "already been assigned. Leaving ..." << std::endl;
		exit(1);
	}

	hasValue_ = true;
	value_ = value;
}

/**************************************************************************/
/**
 * Retrieves the value of the current data set.
 *
 * @return The value of the current data set
 */
double GDataExchange::value(){
	if(!hasValue_) {
		std::cerr << "In GDataExchange::value(): Warning" << std::endl
			      << "Retrieving value of data set when" << std::endl
			      << "no value has been set yet." << std::endl;
	}

	return value_;
}

/**************************************************************************/
/**
 * Checks whether the current data set has a value.
 *
 * @return The value of the hasValue_ variable
 */
bool GDataExchange::hasValue() {
	return hasValue_;
}

/**************************************************************************/
/**
 * Gives access to a parameter set of type double, including its boundaries.
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::shared_ptr<GDoubleParameter> GDataExchange::parameterSet_at<double>(std::size_t pos) {
	boost::shared_ptr<GDoubleParameter> p = dArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if(hasValue_) {
		boost::shared_ptr<GDoubleParameter> p_clone(new GDoubleParameter(*(p.get())));
		return p_clone;
	}

	return p;
}

/**************************************************************************/
/**
 * Gives access to a parameter set of type boost::int32_t, including its boundaries.
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::shared_ptr<GLongParameter> GDataExchange::parameterSet_at<boost::int32_t>(std::size_t pos) {
	boost::shared_ptr<GLongParameter> p = lArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if(hasValue_) {
		boost::shared_ptr<GLongParameter> p_clone(new GLongParameter(*(p.get())));
		return p_clone;
	}

	return p;
}

/**************************************************************************/
/**
 * Gives access to a parameter set of type bool, including its boundaries.
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::shared_ptr<GBoolParameter> GDataExchange::parameterSet_at<bool>(std::size_t pos) {
	boost::shared_ptr<GBoolParameter> p = bArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if(hasValue_) {
		boost::shared_ptr<GBoolParameter> p_clone(new GBoolParameter(*(p.get())));
		return p_clone;
	}

	return p;
}

/**************************************************************************/
/**
 * Gives access to a parameter set of type char, including its boundaries.
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::shared_ptr<GCharParameter> GDataExchange::parameterSet_at<char>(std::size_t pos) {
	boost::shared_ptr<GCharParameter> p = cArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if(hasValue_) {
		boost::shared_ptr<GCharParameter> p_clone(new GCharParameter(*(p.get())));
		return p_clone;
	}

	return p;
}


/**************************************************************************/
/**
 * Gives access to a parameter of type double
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> double GDataExchange::at<double>(std::size_t pos) {
	return dArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type boost::int32_t
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::int32_t GDataExchange::at<boost::int32_t>(std::size_t pos) {
	return lArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type bool
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> bool GDataExchange::at<bool>(std::size_t pos) {
	return bArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type char
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> char GDataExchange::at<char>(std::size_t pos) {
	return cArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to the size of the dArray_ vector
 *
 * @return The size of the dArray_ vector
 */
template <> std::size_t GDataExchange::size<double>() {
	return dArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the lArray_ vector
 *
 * @return The size of the lArray_ vector
 */
template <> std::size_t GDataExchange::size<boost::int32_t>() {
	return lArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the bArray_ vector
 *
 * @return The size of the bArray_ vector
 */
template <> std::size_t GDataExchange::size<bool>() {
	return bArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the cArray_ vector
 *
 * @return The size of the cArray_ vector
 */
template <> std::size_t GDataExchange::size<char>() {
	return cArray_.size();
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GDoubleParameter to the corresponding vector
 *
 * @param gdp The boost::shared_ptr to the GDoubleParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GDoubleParameter>& gdp) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	dArray_.push_back(gdp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GLongParameter to the corresponding vector
 *
 * @param glp The boost::shared_ptr to the GLongParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GLongParameter>& glp) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	lArray_.push_back(glp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GBoolParameter to the corresponding vector
 *
 * @param gbp The boost::shared_ptr to the GBoolParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GBoolParameter>& gbp) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	bArray_.push_back(gbp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GCharParameter to the corresponding vector
 *
 * @param gcp The boost::shared_ptr to the GCharParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GCharParameter>& gcp) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	cArray_.push_back(gcp);
}

/**************************************************************************/
/**
 * Creates a boost::shared_ptr<GDoubleParameter> object without boundaries
 * and appends it to the corresponding vector.
 *
 * @param x The initial parameter value
 */
template <> void GDataExchange::append<double>(const double& x) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x));
	dArray_.push_back(p);
}

/**************************************************************************/
/**
 * Creates a boost::shared_ptr<GLongParameter> object without boundaries
 * and appends it to the corresponding vector.
 *
 * @param x The initial parameter value
 */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t& x) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GLongParameter> p(new GLongParameter(x));
	lArray_.push_back(p);
}

/**************************************************************************/
/**
 * Creates a boost::shared_ptr<GBoolParameter> object without boundaries
 * and appends it to the corresponding vector.
 *
 * @param x The initial parameter value
 */
template <> void GDataExchange::append<bool>(const bool& x) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GBoolParameter> p(new GBoolParameter(x));
	bArray_.push_back(p);
}

/**************************************************************************/
/**
 * Creates a boost::shared_ptr<GCharParameter> object without boundaries
 * and appends it to the corresponding vector.
 *
 * @param x The initial parameter value
 */
template <> void GDataExchange::append<char>(const char& x) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GCharParameter> p(new GCharParameter(x));
	cArray_.push_back(p);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr<GDoubleParameter> object to the corresponding array
 *
 * @param x The initial value of the double parameter
 * @param x_l The lower boundary of the double parameter
 * @param x_u The upper boundary of the double parameter
 */
template <> void GDataExchange::append<double>(const double& x, const double& x_l, const double& x_u) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x,x_l,x_u));
	dArray_.push_back(p);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr<GLongParameter> object to the corresponding array
 *
 * @param x The initial value of the boost::in32_t parameter
 * @param x_l The lower boundary of the boost::in32_t parameter
 * @param x_u The upper boundary of the boost::in32_t parameter
 */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t& x, const boost::int32_t& x_l, const boost::int32_t& x_u) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GLongParameter> p(new GLongParameter(x,x_l,x_u));
	lArray_.push_back(p);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr<GCharParameter> object to the corresponding array
 *
 * @param x The initial value of the char parameter
 * @param x_l The lower boundary of the char parameter
 * @param x_u The upper boundary of the char parameter
 */
template <> void GDataExchange::append<char>(const char& x, const char& x_l, const char& x_u) {
	// Prevent any changes if a value has already been calculated
	if(hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GCharParameter> p(new GCharParameter(x,x_l,x_u));
	cArray_.push_back(p);
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
		std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dit;
		for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) stream << **dit;
	}

	std::size_t lArraySize = lArray_.size();
	stream << lArraySize;
	if(lArraySize) {
		std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lit;
		for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) stream << **lit;
	}

	std::size_t bArraySize = bArray_.size();
	stream << bArraySize;
	if(bArraySize) {
		std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bit;
		for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) stream << **bit;
	}

	std::size_t cArraySize = cArray_.size();
	stream << cArraySize;
	if(cArraySize) {
		std::vector<boost::shared_ptr<GCharParameter> >::const_iterator cit;
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
