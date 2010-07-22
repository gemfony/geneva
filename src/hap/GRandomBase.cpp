/**
 * @file GRandomBase.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Hap library, Gemfony scientific's library of
 * random number routines.
 *
 * Hap is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Hap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Hap library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Hap, visit
 * http://www.gemfony.com .
 */

#include "hap/GRandomBase.hpp"

namespace Gem {
namespace Hap {

/****************************************************************************/
/**
 * The standard constructor
 */
GRandomBase::GRandomBase()
	: gaussCache_(0.)
	, gaussCacheAvailable_(false)
{ /* nothing */ }

/****************************************************************************/
/**
 * The standard destructor
 */
GRandomBase::~GRandomBase()
{ /* nothing */ }

/****************************************************************************/
/**
 * Emits evenly distributed random numbers in the range [0,max[
 *
 * @param max The maximum (excluded) value of the range
 * @return Random numbers evenly distributed in the range [0,max[
 */
double GRandomBase::fpUniform(const double& max) {
#ifdef DEBUG
	// Check that min and max have appropriate values
	assert(max>0.);
#endif
	return fpUniform() * max;
}

/****************************************************************************/
/**
 * Produces evenly distributed random numbers in the range [min,max[
 *
 * @param min The minimum value of the range
 * @param max The maximum (excluded) value of the range
 * @return Random numbers evenly distributed in the range [min,max[
 */
double GRandomBase::fpUniform(const double& min, const double& max) {
#ifdef DEBUG
	// Check that min and max have appropriate values
	assert(min<=max);
#endif
	return fpUniform() * (max - min) + min;
}

/****************************************************************************/
/**
 * Produces gaussian-distributed random numbers.
 *
 * @param mean The mean value of the Gaussian
 * @param sigma The sigma of the Gaussian
 * @return double random numbers with a gaussian distribution
 */
double GRandomBase::fpGaussian(const double& mean, const double& sigma) {
	if(gaussCacheAvailable_) {
		gaussCacheAvailable_ = false;
		return sigma*gaussCache_ + mean;
	}
	else {
#ifdef USEBOXMULLER
		double rnr1 = fpUniform();
		double rnr2 = fpUniform();
		gaussCache_ = sqrt(fabs(-2. * log(1. - rnr1))) * cos(2. * M_PI	* rnr2);
		gaussCacheAvailable_ = true;
		return sigma * sqrt(fabs(-2. * log(1. - rnr1))) * sin(2. * M_PI	* rnr2) + mean;
#else // USEBOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than USEBOXMULLER
		double q, u1, u2;
        do {
        	u1 = 2.* fpUniform() - 1.;
        	u2 = 2.* fpUniform() - 1.;
        	q = u1*u1 + u2*u2;
        } while (q > 1.0);
        q = sqrt((-2.*log(q))/q);
        gaussCache_ = u2 * q;
        gaussCacheAvailable_ = true;
		return sigma * u1 * q + mean;
#endif
	}
}

/****************************************************************************/
/**
 * This function adds two gaussians with sigma "sigma" and a distance
 * "distance" from each other of distance, centered around mean.
 *
 * @param mean The mean value of the entire distribution
 * @param sigma The sigma of both gaussians
 * @param distance The distance between both peaks
 * @return Random numbers with a double-gaussian shape
 */
double GRandomBase::fpDoubleGaussian(
		const double& mean,
		const double& sigma,
		const double& distance
) {
	if (boolUniform()) {
		return fpGaussian(mean - fabs(distance / 2.), sigma);
	}
	else {
		return fpGaussian(mean + fabs(distance / 2.), sigma);
	}
}

/****************************************************************************/
/**
 * This function returns true with a probability "probability", otherwise false.
 *
 * @param p The probability for the value "true" to be returned
 * @return A boolean value, which will be true with a user-defined likelihood
 */
bool GRandomBase::boolWeighted(const double& probability) {
#ifdef DEBUG
	assert(probability>=0. && probability<=1.);
#endif
	return ((fpUniform()<probability)?true:false);
}

/****************************************************************************/
/**
 * This function produces boolean values with a 50% likelihood each for
 * true and false.
 *
 * @return Boolean values with a 50% likelihood for true/false respectively
 */
bool GRandomBase::boolUniform() {
	return boolWeighted(0.5);
}

/****************************************************************************/
/**
 * This function produces random ASCII characters. Please note that that
 * includes also non-printable characters, if "printable" is set to false
 * (default is true).
 *
 * @param printable A boolean variable indicating whether only printable characters should be produced
 * @return Random ASCII characters
 */
char GRandomBase::charUniform(const bool& printable) {
	if (!printable) {
		return intUniform<char>(0, 128);
	} else {
		return intUniform<char>(33, 127);
	}
}

/****************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
