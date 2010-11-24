/**
 * @file GDataExchange.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


#include "dataexchange/GDataExchange.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Dataexchange::GDataExchange)

namespace Gem
{
namespace Dataexchange
{

/**************************************************************************/
/**
 * The default constructor. Adds a single, empty data set to the collection.
 */
GDataExchange::GDataExchange()
	: precision_(DEFAULTPRECISION)
{
	boost::shared_ptr<GParameterValuePair> p(new GParameterValuePair());
	parameterValueSet_.push_back(p);
	current_ = parameterValueSet_.begin();
}

/**************************************************************************/
/**
 * A standard copy constructor. We need to take care to copy the contents of
 * the vector one by one, so that not only the pointers are copied.
 *
 * @param cp A constant reference to another GDataExchange object
 */
GDataExchange::GDataExchange(const GDataExchange& cp)
	: precision_(cp.precision_)
{
	std::vector<boost::shared_ptr<GParameterValuePair> >::const_iterator cit;
	for(cit=cp.parameterValueSet_.begin(); cit!=parameterValueSet_.end(); ++cit) {
		boost::shared_ptr<GParameterValuePair> p(new GParameterValuePair(**cit));
		parameterValueSet_.push_back(p);
	}

	current_ = parameterValueSet_.begin() + (cp.current_ - cp.parameterValueSet_.begin());
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
	Gem::Common::copySmartPointerVector<GParameterValuePair>(cp.parameterValueSet_, parameterValueSet_);
	current_ = parameterValueSet_.begin() + (cp.current_ - cp.parameterValueSet_.begin());
	precision_ = cp.precision_;

	return cp;
}

/**************************************************************************/
/**
 * Check equality with another GDataExchange object. Equality means that the
 * values of all parameters are equal.
 *
 * @param cp A constant reference to another GDataExchange object
 */
bool GDataExchange::operator==(const GDataExchange& cp) const {
	// Check sizes of the vectors
	if(parameterValueSet_.size() != cp.parameterValueSet_.size()) {
#ifdef GENEVATESTING
		std::cerr << "Found inequality: parameterValueSet_.size() != cp.parameterValueSet_.size() : " << parameterValueSet_.size() << " " <<  cp.parameterValueSet_.size() << std::endl;
#endif /* GENEVATESTING */
		return false;
	}

	// Check all parameter sets
	std::vector<boost::shared_ptr<GParameterValuePair> >::const_iterator cit, cp_cit;
	for(cit=parameterValueSet_.begin(), cp_cit=cp.parameterValueSet_.begin();
		cit!=parameterValueSet_.end(), cp_cit!=cp.parameterValueSet_.end();
		++cit, ++cp_cit) {
		if(**cit != **cp_cit) {
#ifdef GENEVATESTING
		std::cerr << "Found inequality: **cit differs from **cp_dcit" << std::endl;
#endif /* GENEVATESTING */
			return false;
		}
	}

	std::size_t localOffset = current_ - parameterValueSet_.begin();
	std::size_t cpOffset = cp.current_ - cp.parameterValueSet_.begin();
	if(localOffset != cpOffset) {
#ifdef GENEVATESTING
		std::cerr << "Found inequality: localOffset != cpOffset : " << localOffset << " " <<  cpOffset << std::endl;
#endif /* GENEVATESTING */
		return false;
	}

	if(precision_ != cp.precision_) {
#ifdef GENEVATESTING
		std::cerr << "Found inequality: precision_ != cp.precision_ : " << precision_ << " " <<  cp.precision_ << std::endl;
#endif /* GENEVATESTING */
		return false;
	}

	// Now we are sure that all components are equal
	return true;
}

/**************************************************************************/
/**
 * Check inequality with another GDataExchange object.
 *
 * @param cp A constant reference to another GDataExchange object
 */
bool GDataExchange::operator!=(const GDataExchange& cp) const {
	return !operator==(cp);
}

/**************************************************************************/
/**
 * Checks whether this object is similar to another. Similarity means that
 * double values only differ from other double values by an amout smaller
 * than the limit parameter.
 *
 * @param  A constant reference to another GDataExchange object
 * @param limit Indicates the acceptable level of differences between both objects
 * @return A boolean indicating whether both objects share similarities
 */
bool GDataExchange::isSimilarTo(const GDataExchange& cp, const double& limit) {
	// Check sizes of the vectors
	if(parameterValueSet_.size() != cp.parameterValueSet_.size()) {
#ifdef GENEVATESTING
		std::cerr << "Found dissimilarity: parameterValueSet_.size() != cp.parameterValueSet_.size() : " << parameterValueSet_.size() << " " <<  cp.parameterValueSet_.size() << std::endl;
#endif /* GENEVATESTING */

		return false;
	}

	// Check all parameter sets
	std::vector<boost::shared_ptr<GParameterValuePair> >::const_iterator cit, cp_cit;
	for(cit=parameterValueSet_.begin(), cp_cit=cp.parameterValueSet_.begin();
		cit!=parameterValueSet_.end(), cp_cit!=cp.parameterValueSet_.end();
		++cit, ++cp_cit) {
		if(!(*cit)->isSimilarTo(**cp_cit, limit)) {
#ifdef GENEVATESTING
		std::cerr << "Found dissimilarity: **cit differs from **cp_dcit" << std::endl;
#endif /* GENEVATESTING */

			return false;
		}
	}

	std::size_t localOffset = current_ - parameterValueSet_.begin();
	std::size_t cpOffset = cp.current_ - cp.parameterValueSet_.begin();
	if(localOffset != cpOffset) {
#ifdef GENEVATESTING
		std::cerr << "Found dissimilarity: localOffset != cpOffset : " << localOffset << " " <<  cpOffset << std::endl;
#endif /* GENEVATESTING */

		return false;
	}

	if(precision_ != cp.precision_) {
#ifdef GENEVATESTING
		std::cerr << "Found inequality: precision_ != cp.precision_ : " << precision_ << " " <<  cp.precision_ << std::endl;
#endif /* GENEVATESTING */
		return false;
	}

	// Now we are sure that all components are equal
	return true;
}

/**************************************************************************/
/**
 * Resets the current data structure. The iterator stays at the same
 * position.
 */
void GDataExchange::reset() {
	(*current_)->reset();
}

/**************************************************************************/
/**
 * Resets all data structures in sequence, resizes the collection to
 * one entry and sets the iterator to the beginning of the sequence.
 */
void GDataExchange::resetAll() {
	std::vector<boost::shared_ptr<GParameterValuePair> >::iterator it;
	for(it=parameterValueSet_.begin(); it!=parameterValueSet_.end(); ++it)	(*it)->reset();
	parameterValueSet_.resize(1);
	current_ = parameterValueSet_.begin();
	precision_ = DEFAULTPRECISION;
}

/**************************************************************************/
/**
 * Sorts the data sets according to their assigned values
 *
 * @param ascending Determines whether items should be sorted in ascending or descending order
 */
void GDataExchange::sort(const bool& ascending) {
	// Check that all data sets have their value set
	std::vector<boost::shared_ptr<GParameterValuePair> >::iterator it;
	for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it) {
		if(!(*it)->hasValue_) {
			std::ostringstream error;
			error << "In GDataExchange::sort() : Error!" << std::endl
			         << "A data set without proper value was found. Leaving ..." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
	}

	// Now sort the entries according to their value.
	if(ascending) { // lowest value needs to be in the front position
		std::sort(parameterValueSet_.begin(), parameterValueSet_.end(), boost::bind(&GParameterValuePair::value,_1) < boost::bind(&GParameterValuePair::value,_2));
	}
	else {
		std::sort(parameterValueSet_.begin(), parameterValueSet_.end(), boost::bind(&GParameterValuePair::value,_1) > boost::bind(&GParameterValuePair::value,_2));
	}

	// Set the iterator to the start of the sequence
    current_ = parameterValueSet_.begin();
}

/**************************************************************************/
/**
 * Switches the iterator to the best available data set. This
 * implies that the collection of data sets gets sorted.
 *
 * @param ascending Determines whether items with lowest or highest value are considered to be best
 */
void GDataExchange::switchToBestDataSet(const bool& ascending) {
	this->sort(ascending);
	current_= parameterValueSet_.begin();
}

/**************************************************************************/
/**
 * Sets the precision of FP IO in ASCII mode
 *
 * @param precision The desired precision of FP IO in ASCII mode
 */
void GDataExchange::setPrecision(const std::streamsize& precision) {
	precision_ = precision;
}

/**************************************************************************/
/**
 * Retrieves the current precision of FP IO in ASCII mode
 *
 * @return The current precision of FP IO in ASCII mode
 */
std::streamsize GDataExchange::getPrecision() const {
	return precision_;
}

/**************************************************************************/
/**
 * Assigns a value to the current data set. Doing so is allowed only once,
 * unless the object is reset.
 *
 * @param value The value to assign to the current data set
 */
void GDataExchange::setValue(double value){
	// Prevent setting of a value if one has already been assigned
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::setValue(): Error!" << std::endl
		        << "Trying to assign a value to the current data set" << std::endl
		        << "when one has already been assigned. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	(*current_)->hasValue_ = true;
	(*current_)->value_ = value;
}

/**************************************************************************/
/**
 * Retrieves the value of the current data set.
 *
 * @return The value of the current data set
 */
double GDataExchange::value(){
	if(!(*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::value(): Warning" << std::endl
			    << "Retrieving value of a data set when" << std::endl
			    << "no value has been set yet." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	return (*current_)->value_;
}

/**************************************************************************/
/**
 * Checks whether the current data set has a value.
 *
 * @return The value of the hasValue_ variable
 */
bool GDataExchange::hasValue() {
	return (*current_)->hasValue_;
}

/**************************************************************************/
/**
 * Sets the iterator to the start of the vector
 */
void GDataExchange::gotoStart() {
	current_ = parameterValueSet_.begin();
}

/**************************************************************************/
/**
 * Switches to the next available data set, if possible. If the end of
 * the list is reached, false is returned, else true. The iterator is returned to the start
 * position, if the end of the list was reached.
 *
 * @return A bool indicating whether the end of the list was reached.
 *
 */
bool GDataExchange::nextDataSet() {
	current_++;
	if(current_ != parameterValueSet_.end()) {
		return true;
	} else {
		current_ = parameterValueSet_.begin();
		return false;
	}
}

/**************************************************************************/
/**
 * Appends a new, empty data set and sets the iterator to its position.
 */
void GDataExchange::newDataSet() {
	boost::shared_ptr<GParameterValuePair> p(new GParameterValuePair());
	parameterValueSet_.push_back(p);
	current_ = parameterValueSet_.end() - 1;
}

/**************************************************************************/
/**
 * Retrieves the number of data sets in the collection.
 *
 * @return The number of data sets in the collection
 */
std::size_t GDataExchange::nDataSets() {
	return parameterValueSet_.size();
}

/**************************************************************************/
/**
 * Checks whether any data sets are present
 *
 * @return A boolean indicating whether data sets are present
 */
bool GDataExchange::dataIsAvailable() {
	if(this->nDataSets() == 1) {
		return (parameterValueSet_.front())->hasData();
	}
	else { // Check all data sets, complain if one without data was found
		std::vector<boost::shared_ptr<GParameterValuePair> >::iterator it;
		for(it=parameterValueSet_.begin(); it!=parameterValueSet_.end(); ++it) {
			if(!(*it)->hasData()) {
				std::ostringstream error;
				error << "In GDataExchange::dataIsAvailable(): Error!" << std::endl
				<< "Multiple data sets, but no data." << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
		}

		return true;
	}
}

/**************************************************************************/
/**
 * Gives access to the number of double parameters in the current parameter set.
 *

 */
template <> std::size_t GDataExchange::numberOfParameterSets<double>() const {
	return (*current_)->dArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the number of long parameters in the current parameter set
 *
 * @return The number of long parameters in the current parameter set
 */
template <> std::size_t GDataExchange::numberOfParameterSets<boost::int32_t>() const {
	return (*current_)->lArray_.size();
}
/**************************************************************************/
/**
 * Gives access to the number of bool parameters in the current parameter set
 *
 * @return The number of bool parameters in the current parameter set
 */
template <> std::size_t GDataExchange::numberOfParameterSets<bool>() const {
	return (*current_)->bArray_.size();
}


/**************************************************************************/
/**
 * Gives access to a parameter set of type double, including its boundaries.
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::shared_ptr<GDoubleParameter> GDataExchange::parameterSet_at<double>(std::size_t pos) {
	boost::shared_ptr<GDoubleParameter> p = (*current_)->dArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if((*current_)->hasValue_) {
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
	boost::shared_ptr<GLongParameter> p = (*current_)->lArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if((*current_)->hasValue_) {
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
	boost::shared_ptr<GBoolParameter> p = (*current_)->bArray_.at(pos);

	// If a value has already been assigned, we return a copy of the original data set,
	// so no changes of the already evaluated data can occur
	if((*current_)->hasValue_) {
		boost::shared_ptr<GBoolParameter> p_clone(new GBoolParameter(*(p.get())));
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
	return (*current_)->dArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type boost::int32_t
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> boost::int32_t GDataExchange::at<boost::int32_t>(std::size_t pos) {
	return (*current_)->lArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to a parameter of type bool
 *
 * @param pos The position of the data access is sought for
 * @return The data access was sought for
 */
template <> bool GDataExchange::at<bool>(std::size_t pos) {
	return (*current_)->bArray_.at(pos)->getParameter();
}

/**************************************************************************/
/**
 * Gives access to the size of the dArray_ vector
 *
 * @return The size of the dArray_ vector
 */
template <> std::size_t GDataExchange::size<double>() {
	return (*current_)->dArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the lArray_ vector
 *
 * @return The size of the lArray_ vector
 */
template <> std::size_t GDataExchange::size<boost::int32_t>() {
	return (*current_)->lArray_.size();
}

/**************************************************************************/
/**
 * Gives access to the size of the bArray_ vector
 *
 * @return The size of the bArray_ vector
 */
template <> std::size_t GDataExchange::size<bool>() {
	return (*current_)->bArray_.size();
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GDoubleParameter to the corresponding vector
 *
 * @param gdp The boost::shared_ptr to the GDoubleParameter to be added to the vector
 */
void GDataExchange::append(boost::shared_ptr<GDoubleParameter> gdp) {
	// Prevent any changes if a value has already been calculated
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			     << "Tried to assign new data when a value has already" << std::endl
				 << "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	(*current_)->dArray_.push_back(gdp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GLongParameter to the corresponding vector
 *
 * @param glp The boost::shared_ptr to the GLongParameter to be added to the vector
 */
void GDataExchange::append(boost::shared_ptr<GLongParameter> glp) {
	// Prevent any changes if a value has already been calculated
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			    << "Tried to assign new data when a value has already" << std::endl
				<< "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	(*current_)->lArray_.push_back(glp);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr to a GBoolParameter to the corresponding vector
 *
 * @param gbp The boost::shared_ptr to the GBoolParameter to be added to the vector
 */
void GDataExchange::append(boost::shared_ptr<GBoolParameter> gbp) {
	// Prevent any changes if a value has already been calculated
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			    << "Tried to assign new data when a value has already" << std::endl
				<< "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	(*current_)->bArray_.push_back(gbp);
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
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			     << "Tried to assign new data when a value has already" << std::endl
				 << "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x));
	(*current_)->dArray_.push_back(p);
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
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			     << "Tried to assign new data when a value has already" << std::endl
				 << "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	boost::shared_ptr<GLongParameter> p(new GLongParameter(x));
	(*current_)->lArray_.push_back(p);
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
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			    << "Tried to assign new data when a value has already" << std::endl
				<< "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	boost::shared_ptr<GBoolParameter> p(new GBoolParameter(x));
	(*current_)->bArray_.push_back(p);
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
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			     << "Tried to assign new data when a value has already" << std::endl
				 << "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// The GDoubleParameter constructor will do error checks
	boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x,x_l,x_u));
	(*current_)->dArray_.push_back(p);
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
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			     << "Tried to assign new data when a value has already" << std::endl
				 << "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// The GLongParameter constructor will do error checks
	boost::shared_ptr<GLongParameter> p(new GLongParameter(x,x_l,x_u));
	(*current_)->lArray_.push_back(p);
}

/**************************************************************************/
/**
 * Adds a boost::shared_ptr<GBoolParameter> object to the corresponding array.
 * Note that this function is provided for completeness only. The upper and lower
 * boundaries will always be fixed to true and false respectively.
 *
 * @param x The initial value of the bool parameter
 * @param x_l The lower boundary of the bool parameter (always set to false)
 * @param x_u The upper boundary of the bool parameter (always set to true)
 */
template <> void GDataExchange::append<bool>(const bool& x, const bool& x_l, const bool& x_u) {
	// Prevent any changes if a value has already been calculated
	if((*current_)->hasValue_) {
		std::ostringstream error;
		error << "In GDataExchange::append(): Error!" << std::endl
			    << "Tried to assign new data when a value has already" << std::endl
				<< "been calculated. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	boost::shared_ptr<GBoolParameter> p(new GBoolParameter(x,false,true));
	(*current_)->bArray_.push_back(p);
}

/**************************************************************************/
/**
 * Writes the class'es data to a stream.
 *
 * @param stream The external output stream to write to
 */
void GDataExchange::writeToStream(std::ostream& stream) const {
	// Back up the current precision
	std::streamsize precisionStore = stream.precision();

	// Now set the output precision to the desired value
	stream.precision(precision_);

	// Let the audience know about the number of data sets
	stream <<  parameterValueSet_.size() << std::endl;

	// Then write all data sets to the stream. We assume that each of them terminates with a std::endl itself
	std::vector<boost::shared_ptr<GParameterValuePair> >::const_iterator cit;
	for(cit=parameterValueSet_.begin(); cit!=parameterValueSet_.end(); ++cit)
		(*cit)->writeToStream(stream);

	// Store the offset of the current_ iterator
	std::size_t offset = current_ - parameterValueSet_.begin();
	stream << offset << std::endl;

	// Restore the original precision
	stream.precision(precisionStore);
}

/**************************************************************************/
/**
 * Reads the class'es data from a stream
 *
 * @param stream The external input stream to read from
 */
void GDataExchange::readFromStream(std::istream& stream) {
	// Back up the current precision
	std::streamsize precisionStore = stream.precision();

	// Now set the stream precision to the desired value
	stream.precision(precision_);

	// Find out about the number of data sets in the stream
	std::size_t nDataSets = 0;
	stream >> nDataSets;

	std::size_t localSize = parameterValueSet_.size();

	// Check whether we have the same number of items or whether we
	// need to make any adjustments. Then read in the correct number
	// of data sets
	std::vector<boost::shared_ptr<GParameterValuePair> >::iterator it;
	if(nDataSets == localSize) {
		for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it)
			(*it)->readFromStream(stream);
	}
	else if(nDataSets > localSize) {
		for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it)
			(*it)->readFromStream(stream);

		for(std::size_t i=localSize; i<nDataSets; i++) {
			boost::shared_ptr<GParameterValuePair> p(new GParameterValuePair());
			p->readFromStream(stream);
			parameterValueSet_.push_back(p);
		}
	}
	else if(nDataSets < parameterValueSet_.size()) {
		parameterValueSet_.resize(nDataSets);
		for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it)
			(*it)->readFromStream(stream);
	}

	// Make the original offset known to the current_ iterator
	std::size_t offset = 0;
	stream >> offset;
	current_ = parameterValueSet_.begin() + offset;

	// Restore the stream's original precision
	stream.precision(precisionStore);
}

/**************************************************************************/
/**
 * Writes the class'es data to a stream in binary mode
 *
 * @param stream The external output stream to write to
 */
void GDataExchange::binaryWriteToStream(std::ostream& stream) const {
	// Let the audience know about the number of data sets
	std::size_t nDataSets = parameterValueSet_.size();
	stream.write(reinterpret_cast<const char *>(&nDataSets), sizeof(nDataSets));

	// Then write all data sets to the stream
	std::vector<boost::shared_ptr<GParameterValuePair> >::const_iterator cit;
	for(cit=parameterValueSet_.begin(); cit!=parameterValueSet_.end(); ++cit) {
		(*cit)->binaryWriteToStream(stream);
	}

	// Store the offset of the current_ iterator
	std::size_t offset = current_ - parameterValueSet_.begin();
	stream.write(reinterpret_cast<const char *>(&offset), sizeof(offset));
}

/**************************************************************************/
/**
 * Reads the class'es data from a stream in binary mode
 *
 * @param stream The external input stream to read from
 */
void GDataExchange::binaryReadFromStream(std::istream& stream) {
	// Find out about the number of data sets in the stream
	std::size_t nDataSets = 0;
	stream.read(reinterpret_cast<char *>(&nDataSets), sizeof(nDataSets));
	std::size_t localSize = parameterValueSet_.size();

	// Check whether we have the same number of items or whether we
	// need to make any adjustments. Then read in the correct number
	// of data sets
	std::vector<boost::shared_ptr<GParameterValuePair> >::iterator it;
	if(nDataSets == localSize) {
		for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it)
			(*it)->binaryReadFromStream(stream);
	}
	else if(nDataSets > localSize) {
		for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it) {
			(*it)->binaryReadFromStream(stream);
		}

		for(std::size_t i=localSize; i<nDataSets; i++) {
			boost::shared_ptr<GParameterValuePair> p(new GParameterValuePair());
			p->binaryReadFromStream(stream);
			parameterValueSet_.push_back(p);
		}
	}
	else if(nDataSets < parameterValueSet_.size()) {
		parameterValueSet_.resize(nDataSets);
		for(it=parameterValueSet_.begin(); it != parameterValueSet_.end(); ++it)
			(*it)->binaryReadFromStream(stream);
	}

	// Make the original offset known to the current_ iterator
	std::size_t offset = 0;
	stream.read(reinterpret_cast<char *>(&offset), sizeof(offset));
	current_ = parameterValueSet_.begin() + offset;
}

/**************************************************************************/
/**
 * Writes the class'es data to a file in binary or text mode.
 *
 * @param fileName The name of the output file
 * @param binary Indicates whether output should be done in binary- or text-mode
 * @param nDataSets The number of data sets to write to the file
 */
void GDataExchange::writeToFile(const std::string& fileName, const bool& binary, const std::size_t& nDataSets, const bool& ascending) {
	std::ofstream output(fileName.c_str());
	if(!output.good()) {
		std::ostringstream error;
		error << "In GDataExchange::writeToFile(): Error!" << std::endl
			          << "Output stream is in a bad state. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// We have been asked to resize the container. For this
	// we need to make sure that the nDatasets best items are in the front position
	if(nDataSets && parameterValueSet_.size() > nDataSets) {
		this->sort(ascending);
		parameterValueSet_.resize(nDataSets);
	}

	if(binary)
		this->binaryWriteToStream(output);
	else
		this->writeToStream(output);

	output.close();
}

/**************************************************************************/
/**
 * Reads the class'es data to a file in binary or text mode
 *
 * @param fileName The name of the input file
 * @param binary Indicates whether data reading should be done in binary- or text-mode
 */
void GDataExchange::readFromFile(const std::string& fileName, bool binary) {
	std::ifstream input(fileName.c_str());
	if(!input.good()) {
		std::ostringstream error;
		error << "In GDataExchange::readFromFile(): Error!" << std::endl
 			     << "Input stream is in a bad state. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	if(binary)
		this->binaryReadFromStream(input);
	else
		this->readFromStream(input);

	input.close();

	// Check that data is available
	if(!this->dataIsAvailable()) {
		std::ostringstream error;
		error << "In GDataExchange::readFromFile(): Error!" << std::endl
 			     << "Read data from file, but no data is available ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// Make sure the iterator is at the start position
	this->gotoStart();
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

} /* namespace Dataexchange */
} /* namespace Gem */
