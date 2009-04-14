/**
 * @file GBoundedNumT.cpp
 */

/* Copyright (C) 2004-2009 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
	:GParameterT<double>(0.),
	 lowerBoundary_(0.),
	 upperBoundary_(1.),
	 internalValue_(0.)
{
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
	:GParameterT<boost::int32_t>(0),
	 lowerBoundary_(0),
 	 upperBoundary_(1000),
	 internalValue_(1)
{
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
	:GParameterT<double>(0.),
	 lowerBoundary_(-std::numeric_limits<double>::max()),
	 upperBoundary_(std::numeric_limits<double>::max()),
	 internalValue_(0.)
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
	:GParameterT<boost::int32_t>(0),
	 lowerBoundary_(std::numeric_limits<boost::int32_t>::min()),
	 upperBoundary_(std::numeric_limits<boost::int32_t>::max()),
	 internalValue_(0)
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
	:GParameterT<double>(0.),
	 lowerBoundary_(lowerBoundary),
	 upperBoundary_(upperBoundary),
	 internalValue_(0.)
{
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
	:GParameterT<boost::int32_t>(0),
	 lowerBoundary_(lowerBoundary),
	 upperBoundary_(upperBoundary),
	 internalValue_(0)
{
		// This function also sets the internalValue_ variable.
		// discreteRandom returns values in the range [lower,upper[ , i.e., the upper boundary
		// is not included. Needs to be checked in the testing code!!!
		setExternalValue(gr.discreteRandom(lowerBoundary_,upperBoundary_+1));
}

/***********************************************************************************************/
/**
 * This function works differently if double is the base type of GBoundedNumT<T>.
 *
 * @param cp A copy of another GBoundedNumT<double>, camouflaged as a GObject
 * @param limit The allowed deviation of doubles between cp and the current object
 * @return A boolean indiciating similarity or dissimilarity
 */
template <> bool GBoundedNumT<double>::isSimilarTo(const GObject& cp, const double& limit) const {
	// Check that we are indeed dealing with a GBoundedNumT<T> reference
	const GBoundedNumT<double> *gbnt_load = GObject::conversion_cast(&cp,  this);

	// Check the parent class'es similarity
	if(!GParameterT<double>::isSimilarTo(*gbnt_load, limit)) { // [1]
#ifdef GENEVATESTING
	std::cout << "Dissimilarity in GBoundedNumT<double>::[1]" << std::endl;
#endif /* GENEVATESTING */
		return false;
	}

	// Check our local data
	if(fabs(lowerBoundary_  - gbnt_load->lowerBoundary_) > fabs(limit)) { // [2]
#ifdef GENEVATESTING
	std::cout << "Dissimilarity in GBoundedNumT<double>::[2]" << std::endl;
#endif /* GENEVATESTING */
		return false;
	}
	if(fabs(upperBoundary_ - gbnt_load->upperBoundary_) > fabs(limit)) { // [3]
#ifdef GENEVATESTING
	std::cout << "Dissimilarity in GBoundedNumT<double>::[3]" << std::endl;
#endif /* GENEVATESTING */
		return false;
	}
	if(fabs(internalValue_ - gbnt_load->internalValue_) > fabs(limit)) { // [4]
#ifdef GENEVATESTING
	std::cout << "Dissimilarity in GBoundedNumT<double>::[4]" << std::endl;
#endif /* GENEVATESTING */
		return false;
	}

	return true;
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
