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

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
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
		 upperBoundary_(T(NULL))
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization of the parameter only, no boundaries
	 *
	 * @param param The value of the parameter
	 */
	GNumericParameterT(T param)
		:param_(param),
		 lowerBoundary_(T(NULL)),
		 upperBoundary_(T(NULL))
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * Initialization of the parameter plus its boundaries
	 *
	 * @param param The value of the parameter
	 * @param lower The value of the lower boundary
	 * @param upper The value of the upper boundary
	 */
	GNumericParameterT(T param, T lower, T upper)
		:param_(param),
		 lowerBoundary_(lower),
		 upperBoundary_(upper)
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
		 upperBoundary_(cp.upperBoundary_)
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
	 * Allow implicit conversion from this type to the target type
	 *
	 * @return The current value of the param_ parameter
	 */
	operator T() {
		return param_;
	}

	/***************************************************************************/
	/**
	 * Sets the parameter (and optionally also the boundaries) to a
	 * user-defined value
	 *
	 * @param param The value of the parameter
	 * @param lower The value of the lower boundary (optional)
	 * @param upper The value of the upper boundary (optional)
	 */
	void setParameter(T param, T lower = T(NULL), T upper = T(NULL)){
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
	T& getParameter() {
		return param_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the lower boundary assigned to this parameter
	 *
	 * @return The current value of the lower boundary
	 */
	T& getLowerBoundary() {
		return lowerBoundary_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the upper boundary assigned to this parameter
	 *
	 * @return The current value of the upper boundary
	 */
	T& getUpperBoundary() {
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
	 * Writes the class'es data to a stream
	 *
	 * @param stream The external output stream to write to
	 */
	void writeToStream(std::ostream& stream) const {
		stream << param_ << lowerBoundary_ << upperBoundary_;
	}

	/***************************************************************************/
	/**
	 * Reads the class'es data from a stream
	 *
	 * @param stream The external input stream to read from
	 */
	void readFromStream(std::istream& stream) {
		stream >> param_ >> lowerBoundary_ >> upperBoundary_;
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
};

// Declaration of specializations for various "allowed types
template<> double GNumericParameterT<double>::unknownParameterTypeTrap(double);
template<> boost::int32_t GNumericParameterT<boost::int32_t>::unknownParameterTypeTrap(boost::int32_t);
template<> char GNumericParameterT<char>::unknownParameterTypeTrap(char);
template<> bool GNumericParameterT<bool>::unknownParameterTypeTrap(bool);

} /* namespace Util */
} /* namespace Gem */

#endif /* GNUMERICPARAMETERT_HPP_ */
