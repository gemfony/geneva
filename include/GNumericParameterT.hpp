/**
 * @file GNumericParameterT.hpp
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
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

// Boost headers go here
#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/shared_ptr.hpp>


#ifndef GNUMERICPARAMETERT_HPP_
#define GNUMERICPARAMETERT_HPP_

namespace Gem
{
namespace Util
{

// Set to the average number of digits of a double number
const std::streamsize DEFAULTPRECISION=std::numeric_limits< double >::digits10;

/*******************************************************************************/
/**
 * This class allows to store a numeric parameter plus possible boundaries (both
 * included in the allowed value range). If the upper- and lower boundary are both
 * set to equal values, then no boundaries are assumed to be present.
 */
template <typename T>
class GNumericParameterT {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;

      ar & make_nvp("param_", param_);
      ar & make_nvp("lowerBoundary_", lowerBoundary_);
      ar & make_nvp("upperBoundary_", upperBoundary_);
      ar & make_nvp("precision_", precision_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GNumericParameterT()
		:param_(T(NULL)),
		 lowerBoundary_(T(NULL)),
		 upperBoundary_(T(NULL)),
		 precision_(DEFAULTPRECISION)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization of the parameter only, no boundaries
	 *
	 * @param param The value of the parameter
	 */
	GNumericParameterT(const T& param)
		:param_(param),
		 lowerBoundary_(T(NULL)),
		 upperBoundary_(T(NULL)),
		 precision_(DEFAULTPRECISION)
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * Initialization of the parameter plus its boundaries
	 *
	 * @param param The value of the parameter
	 * @param lower The value of the lower boundary
	 * @param upper The value of the upper boundary
	 */
	GNumericParameterT(const T& param, const T& lower, const T& upper)
		:param_(param),
		 lowerBoundary_(lower),
		 upperBoundary_(upper),
		 precision_(DEFAULTPRECISION)
	{
		// Check the validity of the boundaries.
		// ATTENTION: Is false<true standard compliant ? Works with g++ 4.2.3 and switch -Wall
		if(lowerBoundary_ != upperBoundary_ &&
		   (param_ < lowerBoundary_ || param_ > upperBoundary_ || lowerBoundary_ >= upperBoundary_)) {
			std::cout << "In GNumericParameterT<T>::GNumericParameterT(param, lower, upper): Error!" << std::endl
				      << "Invalid boundary and/or parameter values:" << std:: endl
				      << "param_ = " << param_ << std::endl
				      << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				      << "upperBoundary_ = " << upperBoundary_ << std::endl
				      << "Leaving ... " << std::endl;
			exit(1);
		}
	}

	/***************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param A const reference to another GNumericParameterT object
	 */
	GNumericParameterT(const GNumericParameterT& cp)
		:param_(cp.param_),
		 lowerBoundary_(cp.lowerBoundary_),
		 upperBoundary_(cp.upperBoundary_),
		 precision_(cp.precision_)
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GNumericParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param A const reference to another GNumericParameterT object
	 */
	const GNumericParameterT& operator=(const GNumericParameterT& cp) {
		param_ = cp.param_;
		lowerBoundary_ = cp.lowerBoundary_;
		upperBoundary_ = cp.upperBoundary_;
		precision_ = cp.precision_;

		return *this;
	}

	/***************************************************************************/
	/**
	 * An assignment operator for the parameter type
	 *
	 * @param A const reference to a variable of template type T
	 */
	const T& operator=(const T& x) {
		param_ = x;

		// Check the validity of the new variable
		if(lowerBoundary_ != upperBoundary_ && (param_ < lowerBoundary_ || param_ > upperBoundary_)) {
			std::cout << "In GNumericParameterT<T>::operator=(const T&): Error" << std::endl
				      << "Invalid value received:" << std::endl
				      << "param_ = " << param_ << std::endl
				      << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				      << "upperBoundary_ = " << upperBoundary_ << std::endl
				      << "Leaving ..." << std::endl;
			exit(1);
		}

		return param_;
	}

	/***************************************************************************/
	/**
	 * Checks whether this object is equal to a second object. Equality
	 * means that all parameters, boundaries and FP IO precision  are equal.
	 *
	 * @param cp A constant reference to another GNumericParameter<T> object
	 */
	bool operator==(const GNumericParameterT<T>& cp) const {
		bool result = true; // Everything o.k. by default

#ifdef GENEVATESTING
		if(param_ != cp.param_) {
			std::cerr << "param_ != cp.param_ : " << param_ << " " << cp.param_ << std::endl;
			result = false;
		}

		if(lowerBoundary_ != cp.lowerBoundary_) {
			std::cerr << "lowerBoundary_ != cp.lowerBoundary_ : " << lowerBoundary_ << " " << cp.lowerBoundary_ << std::endl;
			result = false;
		}

		if(upperBoundary_ != cp.upperBoundary_) {
			std::cerr << "upperBoundary_ != cp.upperBoundary_ : " << upperBoundary_ << " " << cp.upperBoundary_ << std::endl;
			result = false;
		}

		if(precision_ != cp.precision_) {
			std::cerr << "precision_ != cp.precision_ : " << precision_ << " " << cp.precision_ << std::endl;
			result = false;
		}
#else
		if(param_ != cp.param_ || lowerBoundary_ != cp.lowerBoundary_ ||
		   upperBoundary_ != cp.upperBoundary_ || precision_ != cp.precision_) {
			result = false;
		}
#endif /* GENEVATESTING */

		return result;
	}

	/***************************************************************************/
	/**
	 * Checks whether this object is unequal to a second object.
	 *
	 * @param cp A constant reference to another GNumericParameter<T> object
	 */
	bool operator!=(const GNumericParameterT<T>& cp) const {
		return !operator==(cp);
	}

	/***************************************************************************/
	/**
	 * Checks for similarity between two objects. For most types
	 * the same as equality, but different for doubles (particularly
	 * in the case of text io. This function is mainly needed for
	 * testing purposes. There is a speecialization for double.
	 *
	 * @param cp A constant reference to another GNumericParameterT<T> object
	 * @param limit The maximum acceptable level of difference between both objects
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	bool isSimilarTo(const GNumericParameterT<T>& cp, const T& limit=(T)NULL) const {
		return operator==(cp);
	}

	/***************************************************************************/
	/**
	 * Erases all previous values
	 */
	void reset() {
		param_ = T(NULL);
		lowerBoundary_ = T(NULL);
		upperBoundary_ = T(NULL);
		precision_ = DEFAULTPRECISION;
	}

	/***************************************************************************/
	/**
	 * Sets the parameter to a user-defined value. This function requires that
	 * either the new value is inside existing boundaries or that boundaries
	 * have not been set.
	 *
	 * @param param The value of the parameter
	 */
	void setParameter(const T& param){
		// Check the validity of the boundaries
		if(lowerBoundary_ != upperBoundary_ &&
		   (param_ < lowerBoundary_ || param_ > upperBoundary_)) {
			std::cout << "In GNumericParameterT<T>::setParameter(param): Error!" << std::endl
				      << "Invalid boundary and/or parameter values:" << std:: endl
				      << "param_ = " << param_ << std::endl
				      << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				      << "upperBoundary_ = " << upperBoundary_ << std::endl
				      << "Leaving ... " << std::endl;
			exit(1);
		}

		// Set the parameter- and boundary values;
		param_ = param;
	}

	/***************************************************************************/
	/**
	 * Sets the parameter and boundaries to a user-defined value
	 *
	 * @param param The value of the parameter
	 * @param lower The value of the lower boundary
	 * @param upper The value of the upper boundary
	 */
	void setParameter(const T& param, const T& lower, const T& upper){
		// Check the validity of the boundaries
		if(lowerBoundary_ != upperBoundary_ &&
		   (param_ < lowerBoundary_ || param_ > upperBoundary_ || lowerBoundary_ >= upperBoundary_)) {
			std::cout << "In GNumericParameterT<T>::setParameter(param, lower, upper): Error!" << std::endl
				      << "Invalid boundary and/or parameter values:" << std:: endl
				      << "param_ = " << param_ << std::endl
				      << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				      << "upperBoundary_ = " << upperBoundary_ << std::endl
				      << "Leaving ... " << std::endl;
			exit(1);
		}

		// Set the parameter- and boundary values;
		param_ = param;
		lowerBoundary_ = lower;
		upperBoundary_ = upper;
	}

	/***************************************************************************/
	/**
	 * Retrieve the parameter value
	 *
	 * @return The current value of the param_ variable
	 */
	T getParameter() {
		return param_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the lower boundary assigned to this parameter
	 *
	 * @return The current value of the lower boundary
	 */
	T getLowerBoundary() {
		return lowerBoundary_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the upper boundary assigned to this parameter
	 *
	 * @return The current value of the upper boundary
	 */
	T getUpperBoundary() {
		return upperBoundary_;
	}

	/***************************************************************************/
	/**
	 * Checks if the parameter has boundaries defined
	 *
	 * @return A boolean indicating whether boundaries have been defined
	 */
	bool hasBoundaries() {
		if(lowerBoundary_ == upperBoundary_) return false;
		return true;
	}

	/***************************************************************************/
	/**
	 * Sets the precision of floating point IO to a new value.
	 *
	 * @param The new value of the precision_ variable
	 */
	void setPrecision(const std::streamsize& precision) {
		precision_ = precision;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current precision of floating point IO.
	 *
	 * @return The current value of the precision_ variable
	 */
	std::streamsize getPrecision() {
		return precision_;
	}

	/***************************************************************************/
	/**
	 * Writes the class'es data to a stream in ASCII format. Note that
	 * there is a specialization for typeof(T) == double, as the precision
	 * matters in this case. It will be written out and read back in in that
	 * mode only. Another specialization exists for typeof(T)==bool, as
	 * boundaries do not matter for this type.
	 *
	 * @param stream The external output stream to write to
	 */
	void writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<T>::writeToStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		stream << param_ << std::endl
		            << lowerBoundary_ << std::endl
		            << upperBoundary_ << std::endl;
	}

	/***************************************************************************/
	/**
	 * Reads the class'es data from a stream in ASCII format. Note that
	 * there is a specialization for typeof(T) == double, as the precision
	 * matters in this case. It will be written out and read back in in that
	 * mode only. Another specialization exists for typeof(T)==bool, as
	 * boundaries do not matter for this type.
	 *
	 * @param stream The external input stream to read from
	 */
	void readFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<T>::readFromStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		stream >> param_;
		stream >> lowerBoundary_;
		stream >>  upperBoundary_;
	}

	/***************************************************************************/
	/**
	 * Writes the class'es data to a stream in binary format. Note that
	 * there is a specialization for typeof(T) == double, as the precision
	 * matters in this case. It will be written out and read back in in that
	 * mode only. Another specialization exists for typeof(T)==bool, as
	 * boundaries do not matter for this type.
	 *
	 * @param stream The external output stream to write to
	 */
	void binaryWriteToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<T>::binaryWriteToStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		// Write the data out in binary mode
		stream.write(reinterpret_cast<const char *>(&param_), sizeof(param_));
		stream.write(reinterpret_cast<const char *>(&lowerBoundary_), sizeof(lowerBoundary_));
		stream.write(reinterpret_cast<const char *>(&upperBoundary_), sizeof(upperBoundary_));
	}

	/***************************************************************************/
	/**
	 * Reads the class'es data from a stream in binary format. Note that
	 * there is a specialization for typeof(T) == double, as the precision
	 * matters in this case. It will be written out and read back in in that
	 * mode only. Another specialization exists for typeof(T)==bool, as
	 * boundaries do not matter for this type.
	 *
	 * @param stream The external input stream to read from
	 */
	void binaryReadFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<T>::binaryReadFromStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		// Read data from the stream in binary mode
		stream.read(reinterpret_cast<char *>(&param_), sizeof(param_));
		stream.read(reinterpret_cast<char *>(&lowerBoundary_), sizeof(lowerBoundary_));
		stream.read(reinterpret_cast<char *>(&upperBoundary_), sizeof(upperBoundary_));
	}

protected:
	/***************************************************************************/
	/**
	 * A trap designed to catch attempts to use this class with types it was
	 * not designed for. If not implemented, an error message will be generated
	 * during execution.
	 *
	 * @param unknown A dummy parameter
	 */
	virtual T unknownParameterTypeTrap(T unknown){
		std::cerr << "In GNumericParameterT<T>::unknownParameterTypeTrap(): Error" << std::endl
					  << "You seem to have instantiated this class with a type it was" << std::endl
					  << "not designed for. Leaving ..." << std::endl;
		exit(1);
	}

private:
	/***************************************************************************/
	T param_; ///< The actual parameter value
	T lowerBoundary_; ///< The lower boundary allowed for param_
	T upperBoundary_; ///< The upper boundary allowed for param_

	std::streamsize precision_; ///< The precision used for floating point i/o
};

// Declaration of specializations for various "allowed types
template<> double GNumericParameterT<double>::unknownParameterTypeTrap(double);
template<> boost::int32_t GNumericParameterT<boost::int32_t>::unknownParameterTypeTrap(boost::int32_t);
template<> char GNumericParameterT<char>::unknownParameterTypeTrap(char);
template<> bool GNumericParameterT<bool>::unknownParameterTypeTrap(bool);

template<> void GNumericParameterT<double>::writeToStream(std::ostream&) const;
template<> void GNumericParameterT<double>::readFromStream(std::istream&);
template<> void GNumericParameterT<double>::binaryWriteToStream(std::ostream&) const;
template<> void GNumericParameterT<double>::binaryReadFromStream(std::istream&);

template<> void GNumericParameterT<bool>::writeToStream(std::ostream&) const;
template<> void GNumericParameterT<bool>::readFromStream(std::istream&);
template<> void GNumericParameterT<bool>::binaryWriteToStream(std::ostream&) const;
template<> void GNumericParameterT<bool>::binaryReadFromStream(std::istream&);

template<> void GNumericParameterT<char>::writeToStream(std::ostream&) const;
template<> void GNumericParameterT<char>::readFromStream(std::istream&);

template <> bool GNumericParameterT<double>::isSimilarTo(const GNumericParameterT<double>& cp, const double& limit) const;

template <> GNumericParameterT<bool>::GNumericParameterT();
template <> GNumericParameterT<bool>::GNumericParameterT(const bool&);
template <> GNumericParameterT<bool>::GNumericParameterT(const bool&, const bool&, const bool&);
template <> GNumericParameterT<bool>::GNumericParameterT(const GNumericParameterT<bool>&);
template <> void GNumericParameterT<bool>::setParameter(const bool&, const bool&, const bool&);

} /* namespace Util */
} /* namespace Gem */

#endif /* GNUMERICPARAMETERT_HPP_ */

// TODO:
// Tests für ASCII und binary I/O
// Serialisierungstest
// Switch to exceptions for errors
// Check why GRandom emits so few "true" values
