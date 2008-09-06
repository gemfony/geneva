/**
 * @file GDoubleGaussAdaptor.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

#include "GDoubleGaussAdaptor.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GDoubleGaussAdaptor)

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * The standard constructor. It passes the adaptor's standard name to the
 * parent class and initializes the internal variables.
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor()
	:GAdaptorT<double> (GDGASTANDARDNAME),
	 sigma_(DEFAULTSIGMA),
	 sigmaSigma_(DEFAULTSIGMASIGMA),
	 minSigma_(DEFAULTMINSIGMA),
	 maxSigma_(DEFAULTMAXSIGMA)
{ /* nothing */ }

/*************************************************************************/
/**
 * In addition to passing the name of the adaptor to the parent class,
 * it is also possible to specify a value for the sigma_ parameter in
 * this constructor.
 *
 * @param sigma The initial value for the sigma_ parameter
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double& sigma)
	:GAdaptorT<double> (GDGASTANDARDNAME),
	 sigmaSigma_(DEFAULTSIGMASIGMA),
	 minSigma_(DEFAULTMINSIGMA),
	 maxSigma_(DEFAULTMAXSIGMA)
{
	// This functions does an error check on sigma, so we do not assign
	// the value directly to the private variable.
	setSigma(sigma);
}

/*************************************************************************/
/**
 * This constructor lets a user set all parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param name The name assigned to this adaptor
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double& sigma, const double& sigmaSigma,
										 const double& minSigma, const double& maxSigma)
	:GAdaptorT<double> (GDGASTANDARDNAME)
{
	// These functions do error checks on their values
	setSigmaAdaptionRate(sigmaSigma);
	setSigmaRange(minSigma, maxSigma);
	setSigma(sigma);
}

/*************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GDoubleGaussAdaptor object
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const GDoubleGaussAdaptor& cp)
	:GAdaptorT<double> (cp)
{
	setSigmaAdaptionRate(cp.sigmaSigma_);
	setSigmaRange(cp.minSigma_, cp.maxSigma_);
	setSigma(cp.sigma_);
}

/*************************************************************************/
/**
 * The standard destructor. Empty, as we have no local, dynamically
 * allocated data.
 */
GDoubleGaussAdaptor::~GDoubleGaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator for GDoubleGaussAdaptor objects,
 *
 * @param cp A copy of another GDoubleGaussAdaptor object
 */
const GDoubleGaussAdaptor& GDoubleGaussAdaptor::operator=(const GDoubleGaussAdaptor& cp)
{
	GDoubleGaussAdaptor::load(&cp);
	return *this;
}

/*************************************************************************/
/**
 * This is where the actual mutation of the supplied value takes place.
 * The sigma_ retrieved with GDoubleGaussAdaptor::getSigma() might get
 * mutated itself, if the sigmaSigma_ parameter is not 0. The function is
 * declared inline as it will be called very frequently. The gr obkject is
 * a protected member of GObject, one of our parents.
 *
 * @param value The value that is going to be mutated
 */
inline void GDoubleGaussAdaptor::customMutations(double &value)
{
	// adapt the value in situ. Note that this changes
	// the argument of this function
	value += gr.gaussRandom(0.,sigma_);
}

/*************************************************************************/
/**
 * This adaptor allows the evolutionary adaption of sigma_. This allows the
 * algorithm to adapt to changing geometries of the quality surface. The
 * function is declared inline, as it might be called very often.
 */
inline void GDoubleGaussAdaptor::adaptMutation()
{
	sigma_ *= exp(gr.gaussRandom(0.,sigmaSigma_));

	// make sure sigma_ doesn't get out of range
	if(fabs(sigma_) < minSigma_) sigma_ = minSigma_;
	else if(fabs(sigma_) > maxSigma_) sigma_ = maxSigma_;
}

/*************************************************************************/
/**
 * This function sets the value of the sigma_ parameter.
 *
 * @param sigma The new value of the sigma_ parameter
 */
void GDoubleGaussAdaptor::setSigma(const double& sigma)
{
	// Sigma must be in the allowed value range
	if(sigma < minSigma_ || sigma > maxSigma_)
	{
		std::ostringstream error;
		error << "In GDoubleGaussAdaptor::setSigma(const double&): Error!" << std::endl
		  	  << "sigma is not in the allowed range: " << std::endl
		  	  << sigma << " " << minSigma_ << " " << maxSigma_ << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		throw geneva_error_condition() << error_string(error.str());
	}

	sigma_ = sigma;
}

/*************************************************************************/
/**
 * Retrieves the current value of sigma_.
 *
 * @return The current value of sigma_
 */
double GDoubleGaussAdaptor::getSigma() const throw() {
	return sigma_;
}

/*************************************************************************/
/**
 * Sets the allowed value range of sigma_
 *
 * @param minSigma The minimum allowed value of sigma_
 * @param minSigma The maximum allowed value of sigma_
 */
void GDoubleGaussAdaptor::setSigmaRange(const double& minSigma, const double& maxSigma){
	// Do some error checks
	if(minSigma<=0. || minSigma >= maxSigma){ // maxSigma will automatically be > 0. now
		std::ostringstream error;
		error << "In GDoubleGaussAdaptor::setSigmaRange(const double&, const double&): Error!" << std::endl
			  << "Invalid values for minSigma and maxSigma given:" << minSigma << " " << maxSigma << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		throw geneva_error_condition() << error_string(error.str());
	}

	minSigma_ = minSigma;
	maxSigma_ = maxSigma;

	// Adapt sigma, if necessary
	if(sigma_ < minSigma_) sigma_ = minSigma_;
	else if(sigma_>maxSigma_) sigma_ = maxSigma_;
}

/*************************************************************************/
/**
 * Retrieves the allowed value range for sigma.
 *
 * @return The allowed value range for sigma
 */
std::pair<double,double> GDoubleGaussAdaptor::getSigmaRange() const throw() {
	return std::make_pair(minSigma_, maxSigma_);
}

/*************************************************************************/
/**
 * This function sets the values of the sigmaSigma_ parameter and the
 * minimal value allowed for sigma_.
 *
 * @param sigmaSigma The new value of the sigmaSigma_ parameter
 */
void GDoubleGaussAdaptor::setSigmaAdaptionRate(const double& sigmaSigma)
{
	// A value of sigmaSigma <= 0. is not useful.
	if(sigmaSigma <= 0.)
	{
		std::ostringstream error;
		error << "In GDoubleGaussAdaptor::setSigmaSigma(double, double): Error!" << std::endl
			  << "Bad value for sigmaSigma given: " << sigmaSigma << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		throw geneva_error_condition() << error_string(error.str());
	}

	sigmaSigma_ = sigmaSigma;
}

/*************************************************************************/
/**
 * Retrieves the value of sigmaSigma_ .
 *
 * @return The value of the sigmaSigma_ parameter
 */
double GDoubleGaussAdaptor::getSigmaAdaptionRate() const throw() {
	return sigmaSigma_;
}

/*************************************************************************/
/**
 * Convenience function that lets users set all relevant parameters of this class
 * at once.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param minSigma The maximum value allowed for sigma_
 */
void GDoubleGaussAdaptor::setAll(const double& sigma, const double& sigmaSigma, const double& minSigma, const double& maxSigma)
{
	setSigmaAdaptionRate(sigmaSigma);
	setSigmaRange(minSigma, maxSigma);
	setSigma(sigma);
}

/*************************************************************************/
/**
 * This function creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GDoubleGaussAdaptor::clone()
{
	return new GDoubleGaussAdaptor(*this);
}

/*************************************************************************/
/**
 * This function loads the data of another GDoubleGaussAdaptor, camouflaged
 * as a GObject.
 *
 * @param A copy of another GDoubleGaussAdaptor, camouflaged as a GObject
 */
void GDoubleGaussAdaptor::load(const GObject *cp)
{
	// Convert GObject pointer to local format
	const GDoubleGaussAdaptor *gdga = this->conversion_cast(cp, this);

	// Load the data of our parent class ...
	GAdaptorT<double>::load(cp);

	// ... and then our own data
	sigma_ = gdga->sigma_;
	sigmaSigma_ = gdga->sigmaSigma_;
	minSigma_ = gdga->minSigma_;
	maxSigma_ = gdga->maxSigma_;
}

/*************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
