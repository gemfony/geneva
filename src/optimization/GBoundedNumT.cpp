/**
 * @file GBoundedNumT.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GBoundedNumT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * The default constructor. As this class uses the adaptor scheme
 * (see GTemplateAdaptor<T>), you will need to add your own adaptors,
 * such as the GDoubleGaussAdaptor.  Specialization for typeof(T) == typeof(double).
 */
template <>  GBoundedNumT<double>::GBoundedNumT()
	: GParameterT<double>(0.)
	, lowerBoundary_(0.)
	, upperBoundary_(1.)
	, internalValue_(0.)
{
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// This function also sets the internalValue_ variable.
		setExternalValue(gr.evenRandom(lowerBoundary_,upperBoundary_));
}

/***********************************************************************************************/
/**
 * The default constructor. As this class uses the adaptor scheme
 * (see GTemplateAdaptor<T>), you will need to add your own adaptors,
 * such as the GDoubleGaussAdaptor.  Specialization for typeof(T) == typeof(boost::int32_t).
 */
template <>  GBoundedNumT<boost::int32_t>::GBoundedNumT()
	: GParameterT<boost::int32_t>(0)
	, lowerBoundary_(0)
 	, upperBoundary_(1000)
	, internalValue_(1)
{
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// This function also sets the internalValue_ variable.
		// discreteRandom returns values in the range [lower,upper[ , i.e., the upper boundary
		// is not included. Needs to be checked in the testing code!!!
		setExternalValue(gr.discreteRandom(lowerBoundary_,upperBoundary_+1));
}

/***********************************************************************************************/
	/**
	 * A constructor that initializes the external value only. The boundaries will
	 * be set to the maximum and minimum values of the double type.
	 *
	 * @param val The desired external value of this object
	 */
template <> GBoundedNumT<double>::GBoundedNumT(const double& val)
	: GParameterT<double>(0.)
	, lowerBoundary_(-0.999*0.5*std::numeric_limits<double>::max())
	, upperBoundary_(0.999*0.5*std::numeric_limits<double>::max())
	, internalValue_(0.)
{
		// This function also sets the internalValue_ variable.
		setExternalValue(val);
}

/***********************************************************************************************/
/**
 * A constructor that initializes the external value only. The boundaries will
 * be set to the maximum and minimum values of the boost::int32_t type.
 *
 * @param val The desired external value of this object
 */
template <> GBoundedNumT<boost::int32_t>::GBoundedNumT(const boost::int32_t& val)
	: GParameterT<boost::int32_t>(0)
	, lowerBoundary_(std::numeric_limits<boost::int32_t>::min())
	, upperBoundary_(std::numeric_limits<boost::int32_t>::max())
	, internalValue_(0)
{
		// This function also sets the internalValue_ variable.
		setExternalValue(val);
}

/***********************************************************************************************/
/**
 * Initializes the boundaries and sets the external value to a random number
 * inside the allowed value range.  Specialization for typeof(T) == typeof(double)
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
template <> GBoundedNumT<double>::GBoundedNumT(const double& lowerBoundary, const double& upperBoundary)
	: GParameterT<double>(0.)
	, lowerBoundary_(0.)
	, upperBoundary_(1.)
	, internalValue_(0.)
{
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// Check that the boundaries make sense
		if(lowerBoundary >= upperBoundary) {
			std::ostringstream error;
			error << "In GBoundedNumT<double>::GBoundedNumT(const double&, const double&)" << std::endl
				     << "Error: Lower and/or upper boundary has invalid value : " << lowerBoundary << " " << upperBoundary << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// Check size of the boundaries
		if(lowerBoundary <= -0.5*std::numeric_limits<double>::max() || upperBoundary >= 0.5*std::numeric_limits<double>::max()) {
			std::ostringstream error;
			error << "In GBoundedNumT<double>::GBoundedNumT(const double&, const double&)" << std::endl
				     << "Error: Lower and/or upper boundaries have too high values: " << lowerBoundary << " " << upperBoundary << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		lowerBoundary_ = lowerBoundary;
		upperBoundary_ = upperBoundary;

		// This function also sets the internalValue_ variable.
		setExternalValue(gr.evenRandom(lowerBoundary_,upperBoundary_));
}

/***********************************************************************************************/
/**
 * Initializes the boundaries and sets the external value to a random number
 * inside the allowed value range.  Specialization for typeof(T) == typeof(boost::int32_t)
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
template <> GBoundedNumT<boost::int32_t>::GBoundedNumT(const boost::int32_t& lowerBoundary, const boost::int32_t& upperBoundary)
	: GParameterT<boost::int32_t>(0)
	, lowerBoundary_(lowerBoundary)
	, upperBoundary_(upperBoundary)
	, internalValue_(0)
{
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// Complain if the boundaries are invalid
		if(lowerBoundary >= upperBoundary) {
			std::ostringstream error;
			error << "In GBoundedNumT<double>::GBoundedNumT(const boost::int32_t&, const boost::int32_t&)" << std::endl
				     << "Error: Lower and/or upper boundary has invalid value : " << lowerBoundary << " " << upperBoundary << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// Check size of the boundaries
		if(lowerBoundary <= -std::numeric_limits<boost::int32_t>::max()/2 || upperBoundary >= std::numeric_limits<boost::int32_t>::max()/2) {
			std::ostringstream error;
			error << "In GBoundedNumT<double>::GBoundedNumT(const boost::int32_t&, const boost::int32_t&)" << std::endl
				     << "Error: Lower and/or upper boundaries have too high values: " << lowerBoundary << " " << upperBoundary << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// This function also sets the internalValue_ variable.
		// discreteRandom returns values in the range [lower,upper[ , i.e., the upper boundary
		// is not included. Needs to be checked in the testing code!!!
		setExternalValue(gr.discreteRandom(lowerBoundary_,upperBoundary_+1));
}

/***********************************************************************************************/
/**
 * Resets the boundaries to the maximum allowed value. Specialization for
 * typeof(T) == typeof(double).
 */
template <> void GBoundedNumT<double>::resetBoundaries() {
	double currentValue = this->value();

	lowerBoundary_ = -0.999*0.5*std::numeric_limits<double>::max();
	upperBoundary_ = 0.999*0.5*std::numeric_limits<double>::max();

	this->setExternalValue(currentValue);
}

/***********************************************************************************************/
/**
 * Resets the boundaries to the maximum allowed value. Specialization for
 * typeof(T) == typeof(boost::int32_t).
 */
template <> void GBoundedNumT<boost::int32_t>::resetBoundaries() {
	boost::int32_t currentValue = this->value();

	lowerBoundary_ = std::numeric_limits<boost::int32_t>::min();
	upperBoundary_ =std::numeric_limits<boost::int32_t>::max();

	this->setExternalValue(currentValue);
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
