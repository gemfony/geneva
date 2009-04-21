/**
 * @file GRandom.cpp
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

#include "GRandom.hpp"

namespace Gem {
namespace Util {

/*************************************************************************/
/**
 * The standard constructor. It stores the GRANDOMFACTORY
 * boost::shared_ptr<GRandomFactory> locally and in this way
 * makes sure that the factory doesn't get destroyed while it
 * is still needed.
 */
GRandom::GRandom() throw() :
	current01_(0),
	grf_(GRANDOMFACTORY)
{ /* nothing */ }

/*************************************************************************/
/**
 * A standard destructor
 */
GRandom::~GRandom() {
	p_raw_ = (double *)NULL;
	p01_.reset();
	grf_.reset();
}

/*************************************************************************/
/**
 * This function emits evenly distributed random numbers in the range [0,1[ .
 * Random numbers are usually not produced locally, but are taken from an array
 * provided by the the GRandomFactory class. Random numbers are only produced
 * locally if no valid array could be retrieved.
 *
 * @return Random numbers evenly distributed in the range [0,1[ .
 */
double GRandom::evenRandom() {
	// If the object has been newly created,
	// p01_ will be empty
	if (!p01_ || current01_ == DEFAULTARRAYSIZE) {
		getNewP01();
		current01_ = 0;
	}

	return p_raw_[current01_++];
}

/*************************************************************************/
/**
 * This function emits evenly distributed random numbers in the range [0,max[ .
 *
 * @param max The maximum (excluded) value of the range
 * @return Random numbers evenly distributed in the range [0,max[
 */
double GRandom::evenRandom(const double& max) {
#ifdef DEBUG
	// Check that min and max have appropriate values
	assert(max>0.);
#endif
	return GRandom::evenRandom() * max;
}

/*************************************************************************/
/**
 * This function produces evenly distributed random numbers in the range [min,max[ .
 *
 * @param min The minimum value of the range
 * @param max The maximum (excluded) value of the range
 * @return Random numbers evenly distributed in the range [min,max[
 */
double GRandom::evenRandom(const double& min, const double& max) {
#ifdef DEBUG
	// Check that min and max have appropriate values
	assert(min<=max);
#endif
	return GRandom::evenRandom() * (max - min) + min;
}

/*************************************************************************/
/**
 * Gaussian-distributed random numbers form the core of Evolutionary Strategies.
 * This function provides an easy means of producing such random numbers with
 * mean "mean" and sigma "sigma".
 *
 * @param mean The mean value of the Gaussian
 * @param sigma The sigma of the Gaussian
 * @return double random numbers with a gaussian distribution
 */
double GRandom::gaussRandom(const double& mean, const double& sigma) {
	return sigma * sqrt(fabs(-2. * log(1. - evenRandom()))) * sin(2. * M_PI	* evenRandom()) + mean;
}

/*************************************************************************/
/**
 * This function adds two gaussians with sigma "sigma" and a distance
 * "distance" from each other of distance, centered around mean.
 *
 * @param mean The mean value of the entire distribution
 * @param sigma The sigma of both gaussians
 * @param distance The distance between both peaks
 * @return Random numbers with a double-gaussian shape
 */
double GRandom::doubleGaussRandom(const double& mean, const double& sigma, const double& distance) {
	if (GRandom::boolRandom())
		return GRandom::gaussRandom(mean - fabs(distance / 2.), sigma);
	else
		return GRandom::gaussRandom(mean + fabs(distance / 2.), sigma);
}

/*************************************************************************/
/**
 * This function returns true with a probability "probability", otherwise false.
 *
 * @param p The probability for the value "true" to be returned
 * @return A boolean value, which will be true with a user-defined likelihood
 */
bool GRandom::boolRandom(const double& probability) {
#ifdef DEBUG
	assert(probability>=0. && probability<=1.);
#endif
	return ((GRandom::evenRandom()<probability)?true:false);
}

/*************************************************************************/
/**
 * This function produces boolean values with a 50% likelihood each for
 * true and false.
 *
 * @return Boolean values with a 50% likelihood for true/false respectively
 */
bool GRandom::boolRandom() {
	return boolRandom(0.5);
}

/*************************************************************************/
/**
 * This function produces random ASCII characters. Please note that that
 * includes also non-printable characters, if "printable" is set to false
 * (default is true).
 *
 * @param printable A boolean variable indicating whether only printable characters should be produced
 * @return Random ASCII characters
 */
char GRandom::charRandom(const bool& printable) {
	if (!printable) {
		return (char) discreteRandom<boost::uint8_t>(0, 128);
	} else {
		return (char) discreteRandom<boost::uint8_t>(33, 127);
	}
}

/*************************************************************************/
/**
 * In cases where GRandomFactory was not able to supply us with a suitable
 * array of [0,1[ random numbers we need to produce our own.
 */
void GRandom::fillContainer01() {
	boost::lagged_fibonacci607 lf(GRandomFactory::GSeed());
	boost::shared_array<double> p(new double[DEFAULTARRAYSIZE]);
	double *local_p = p.get();

	for (std::size_t i = 0; i < DEFAULTARRAYSIZE; i++) {
#ifdef DEBUG
		double value = lf();
		assert(value>=0. && value<1.);
		local_p[i]=value;
#else
		local_p[i] = lf();
#endif /* DEBUG */
	}

	p01_ = p;
}

/*************************************************************************/
/**
 * (Re-)Initialization of p01_. Checks that a valid GRandomFactory still
 * exists, then retrieves a new container.
 */
void GRandom::getNewP01() {
	if(grf_) p01_ = grf_->new01Container();

	if (!grf_ || !p01_) {
		// Something went wrong with the retrieval of the
		// random number container. We need to create
		// our own instead.
		GRandom::fillContainer01();
	}

	// We should now have a valid p01_ in any case
	p_raw_ = p01_.get();
}

/*************************************************************************/
/**
 * Specialization of the function for double to trap incorrect usage of
 * the function. Will throw immediately.
 */
template <>
double GRandom::discreteRandom(const double&){
	std::ostringstream error;

	error << "In GRandom::discreteRandom<double>(const double&): Error!" << std::endl
		  << "This function should not be used with double as a template type" << std::endl;

	throw Gem::GenEvA::geneva_error_condition(error.str());
}

/*************************************************************************/
/**
 * Specialization of the function for double to trap incorrect usage of
 * the function. Will throw immediately.
 */
template <>
double GRandom::discreteRandom(const double&, const double&){
	std::ostringstream error;

	error << "In GRandom::discreteRandom<double>(const double&, const double&): Error!" << std::endl
		  << "This function should not be used with double as a template type" << std::endl;

	throw Gem::GenEvA::geneva_error_condition(error.str());
}

/*************************************************************************/
/**
 * Specialization of the function for float to trap incorrect usage of
 * the function. Will throw immediately.
 */
template <>
float GRandom::discreteRandom(const float&){
	std::ostringstream error;

	error << "In GRandom::discreteRandom<float>(const float&): Error!" << std::endl
		  << "This function should not be used with float as a template type" << std::endl;

	throw Gem::GenEvA::geneva_error_condition(error.str());
}

/*************************************************************************/
/**
 * Specialization of the function for float to trap incorrect usage of
 * the function. Will throw immediately.
 */
template <>
float GRandom::discreteRandom(const float&, const float&){
	std::ostringstream error;

	error << "In GRandom::discreteRandom<float>(const float&, const float&): Error!" << std::endl
		  << "This function should not be used with float as a template type" << std::endl;

	throw Gem::GenEvA::geneva_error_condition(error.str());
}

/*************************************************************************/
/**
 * Specialization of the function for bool to trap incorrect usage of
 * the function. Will throw immediately.
 */
template <>
bool GRandom::discreteRandom(const bool&){
	std::ostringstream error;

	error << "In GRandom::discreteRandom<bool>(const bool&): Error!" << std::endl
		  << "This function should not be used with bool as a template type" << std::endl;

	throw Gem::GenEvA::geneva_error_condition(error.str());
}

/*************************************************************************/
/**
 * Specialization of the function for bool to trap incorrect usage of
 * the function. Will throw immediately.
 */
template <>
bool GRandom::discreteRandom(const bool&, const bool&){
	std::ostringstream error;

	error << "In GRandom::discreteRandom<bool>(const bool&, const bool&): Error!" << std::endl
		  << "This function should not be used with bool as a template type" << std::endl;

	throw Gem::GenEvA::geneva_error_condition(error.str());
}

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */
