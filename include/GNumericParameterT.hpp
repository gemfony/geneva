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
		// Check the validity of the boundaries
		if(lowerBoundary_ != upperBoundary_ && (param_ < lower || param_ > upper_ || lower_ >= upper_)) {
			std::cout << "In GNumericParameterT::GNumericParameterT(param, lower, upper): Error!" << std::endl
				      << "Invalid boundary and/or parameter values:" << std:: endl
				      << "param_ = " << param_ << std::endl
				      << "lower_ = " << lower_ << std::endl
				      << "upper_ = " << upper_ << std::endl
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
	 * Sets the parameter (and optionally also the boundaries) to a
	 * user-defined value
	 *
	 * @param param The value of the parameter
	 * @param lower The value of the lower boundary (optional)
	 * @param upper The value of the upper boundary (optional)
	 */
	void setParameter(T param, T lower = T(NULL), T upper = T(NULL)){
		// Check the validity of the boundaries
		if(lowerBoundary_ != upperBoundary_ && (param_ < lower || param_ > upper_ || lower_ >= upper_)) {
			std::cout << "In GNumericParameterT::setParameter(param, lower, upper): Error!" << std::endl
				      << "Invalid boundary and/or parameter values:" << std:: endl
				      << "param_ = " << param_ << std::endl
				      << "lower_ = " << lower_ << std::endl
				      << "upper_ = " << upper_ << std::endl
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
	T getLower() {
		return lowerBoundary_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the upper boundary assigned to this parameter
	 *
	 * @return The current value of the upper boundary
	 */
	T getUpper() {
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

protected:
	/***************************************************************************/
	/**
	 * A trap designed to catch attempts to use this class with types it was
	 * not designed for. If not implemented, compilation will fail.
	 */
	virtual T unknownParameterTypeTrap(T) = 0;

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
