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
 * The default constructor. Adds a single, empty data set to the collection.
 */
GDataExchange::GDataExchange()
	:currentValueSet_(0)
{
	boost::shared_ptr<parameterValuePair> p(new parameterValuePair());
	parameterValueSet_.push_back(p);
	currentValueSet_ = 0;
}

/**************************************************************************/
/**
 * A standard copy constructor. We need to take care to copy the contents of
 * the vector one by one, so that not only the pointers are copied.
 *
 * @param cp A constant reference to another GDataExchange object
 */
GDataExchange::GDataExchange(const GDataExchange& cp) {
	std::vector<parameterValuePair>::const_iterator cit;
	for(cit=cp.parameterValueSet_.begin(); cit!=parameterValueSet_.end(); ++cit) {
		boost::shared_ptr<parameterValuePair> p(new parameterValuePair(**cit));
		parameterValueSet_.push_back(p);
	}

	currentParameterSet_ = cp.currentParameterSet_;
}

/**************************************************************************/
/**
 * The destructor. As we use smart pointers, all we need to do is reset the
 * vector.
 */
GDataExchange::~GDataExchange() {
	parameterValueSet_.clear();
}

/**************************************************************************/
/**
 * A standard assignment operator. As it needs to take care of differing
 * vector sizes, it is far more complicated than the copy constructor. We
 * thus use an external helper function.
 *
 * @param cp A constant reference to another GDataExchange object
 */
const GDataExchange& GDataExchange::operator=(const GDataExchange& cp) {
	copySmartPointerVector<parameterValuePair>(cp.parameterValueSet_, parameterValueSet_);
	currentParameterSet_ = cp.currentParameterSet_;
}

/**************************************************************************/
/**
 * Gets the id of the current value set.
 */

std::size_t GDataExchange::getCurrentParameterSet(){
	return currentParameterSet_;
}

/**************************************************************************/
/**
 * Resets the current data structure.
 */
void GDataExchange::reset() {
	(parameterValueSet_.at(currentParameterSet_))->reset();
}

/**************************************************************************/
/**
 * Resets all data structures in sequence.
 */
void GDataExchange::resetAll() {
	std::vector<boost::shared_ptr<parameterValuePair> >::iterator it;
	for(it=parameterValueSet_.begin(); it!=parameterValueSet_.end(); ++it)	(*it)->reset();
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::setValue(): Error!" << std::endl
		          << "Trying to assign a value to the current" << std::endl
		          << "when one has already been assigned. Leaving ..." << std::endl;
		exit(1);
	}

	(parameterValueSet_.at(currentParameterSet_))->hasValue_ = true;
	(parameterValueSet_.at(currentParameterSet_))->value_ = value;
}

/**************************************************************************/
/**
 * Retrieves the value of the current data set.
 *
 * @return The value of the current data set
 */
double GDataExchange::value(){
	if(!(parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::value(): Warning" << std::endl
			      << "Retrieving value of a data set when" << std::endl
			      << "no value has been set yet." << std::endl;
	}

	return (parameterValueSet_.at(currentParameterSet_))->value_;
}

/**************************************************************************/
/**
 * Checks whether the current data set has a value.
 *
 * @return The value of the hasValue_ variable
 */
bool GDataExchange::hasValue() {
	return (parameterValueSet_.at(currentParameterSet_))->hasValue_;
}

/**************************************************************************/
/**
 * Goes to to the start of the list
 */
void GDataExchange::gotoStart() {

}

/**************************************************************************/
/**
 * Switches to the next available data set
 */
bool GDataExchange::nextDataSet() {

}

/**************************************************************************/
/**
 * Adds a new, empty data set and adjusts the counter
 */
void GDataExchange::newDataSet() {

}

/**************************************************************************/
/**
 * Gives access to a parameter set of type double, including its boundaries.
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::shared_ptr<GDoubleParameter> GDataExchange::parameterSet_at<double>(std::size_t pos) {
	boost::shared_ptr<GDoubleParameter> p = (parameterValueSet_.at(currentParameterSet_))->dArray_.at(pos);

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
	boost::shared_ptr<GLongParameter> p = (parameterValueSet_.at(currentParameterSet_))->lArray_.at(pos);

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
	boost::shared_ptr<GBoolParameter> p = (parameterValueSet_.at(currentParameterSet_))->bArray_.at(pos);

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
	boost::shared_ptr<GCharParameter> p = (parameterValueSet_.at(currentParameterSet_))->cArray_.at(pos);

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
	return (parameterValueSet_.at(currentParameterSet_))->dArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type boost::int32_t
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::int32_t GDataExchange::at<boost::int32_t>(std::size_t pos) {
	return (parameterValueSet_.at(currentParameterSet_))->lArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type bool
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> bool GDataExchange::at<bool>(std::size_t pos) {
	return (parameterValueSet_.at(currentParameterSet_))->bArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type char
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> char GDataExchange::at<char>(std::size_t pos) {
	return (parameterValueSet_.at(currentParameterSet_))->cArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to the size of the dArray_ vector
 *
 * @return The size of the dArray_ vector
 */
template <> std::size_t GDataExchange::size<double>() {
	return (parameterValueSet_.at(currentParameterSet_))->dArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the lArray_ vector
 *
 * @return The size of the lArray_ vector
 */
template <> std::size_t GDataExchange::size<boost::int32_t>() {
	return (parameterValueSet_.at(currentParameterSet_))->lArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the bArray_ vector
 *
 * @return The size of the bArray_ vector
 */
template <> std::size_t GDataExchange::size<bool>() {
	return (parameterValueSet_.at(currentParameterSet_))->bArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the cArray_ vector
 *
 * @return The size of the cArray_ vector
 */
template <> std::size_t GDataExchange::size<char>() {
	return (parameterValueSet_.at(currentParameterSet_))->cArray_.size();
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GDoubleParameter to the corresponding vector
 *
 * @param gdp The boost::shared_ptr to the GDoubleParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GDoubleParameter>& gdp) {
	// Prevent any changes if a value has already been calculated
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	(parameterValueSet_.at(currentParameterSet_))->dArray_.push_back(gdp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GLongParameter to the corresponding vector
 *
 * @param glp The boost::shared_ptr to the GLongParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GLongParameter>& glp) {
	// Prevent any changes if a value has already been calculated
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	(parameterValueSet_.at(currentParameterSet_))->lArray_.push_back(glp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GBoolParameter to the corresponding vector
 *
 * @param gbp The boost::shared_ptr to the GBoolParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GBoolParameter>& gbp) {
	// Prevent any changes if a value has already been calculated
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	(parameterValueSet_.at(currentParameterSet_))->bArray_.push_back(gbp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GCharParameter to the corresponding vector
 *
 * @param gcp The boost::shared_ptr to the GCharParameter to be added to the vector
 */
void GDataExchange::append(const boost::shared_ptr<GCharParameter>& gcp) {
	// Prevent any changes if a value has already been calculated
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	(parameterValueSet_.at(currentParameterSet_))->cArray_.push_back(gcp);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x));
	(parameterValueSet_.at(currentParameterSet_))->dArray_.push_back(p);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GLongParameter> p(new GLongParameter(x));
	(parameterValueSet_.at(currentParameterSet_))->lArray_.push_back(p);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GBoolParameter> p(new GBoolParameter(x));
	(parameterValueSet_.at(currentParameterSet_))->bArray_.push_back(p);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GCharParameter> p(new GCharParameter(x));
	(parameterValueSet_.at(currentParameterSet_))->cArray_.push_back(p);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x,x_l,x_u));
	(parameterValueSet_.at(currentParameterSet_))->dArray_.push_back(p);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GLongParameter> p(new GLongParameter(x,x_l,x_u));
	(parameterValueSet_.at(currentParameterSet_))->lArray_.push_back(p);
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
	if((parameterValueSet_.at(currentParameterSet_))->hasValue_) {
		std::cerr << "In GDataExchange::append(): Error!" << std::endl
			      << "Tried to assign new data when a value has already" << std::endl
				  << "been calculated. Leaving ..." << std::endl;
		exit(1);
	}

	boost::shared_ptr<GCharParameter> p(new GCharParameter(x,x_l,x_u));
	(parameterValueSet_.at(currentParameterSet_))->cArray_.push_back(p);
}

/**************************************************************************/
/**
 * Helper function to aid IO  of parameterValuePair objects
 *
 * @param stream A reference to the output stream
 * @param A const reference to a parameterValuePair object
 * @return A copy of the stream
 */
std::ostream& GDataExchange::operator<<(std::ostream& stream, const parameterValuePair& object) {
	object.writeToStream(stream);
	return stream;
}

/*************************************************************************/
/**
 * Helper function to aid IO of parameterValuePair objects
 *
 * @param stream A reference to the input stream
 * @param A reference to a parameterValuePair object
 * @return A copy of the stream
 */
std::istream& GDataExchange::operator>>(std::istream& stream, parameterValuePair& object) {
	object.readFromStream(stream);
	return stream;
}

/**************************************************************************/
/**
 * Writes the class'es data to a stream
 *
 * @param stream The external output stream to write to
 */
void GDataExchange::writeToStream(std::ostream& stream) const {

}

/**************************************************************************/
/**
 * Reads the class'es data from a stream
 *
 * @param stream The external input stream to read from
 */
void GDataExchange::readFromStream(std::istream& stream) {

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
