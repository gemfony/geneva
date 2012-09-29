/**
 * @file GRandomBase.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "hap/GRandomBase.hpp"

namespace Gem
{
namespace Hap
{

/******************************************************************************/
/**
 * The standard constructor
 */
GRandomBase::GRandomBase()
	: min_value(GRandomBase::result_type(0.))
	, max_value(GRandomBase::result_type(1.))
	, fltGaussCache_(float(0.))
	, dblGaussCache_(double(0.))
	, ldblGaussCache_((long double)0.)
	, fltGaussCacheAvailable_(false)
	, dblGaussCacheAvailable_(false)
	, ldblGaussCacheAvailable_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GRandomBase::~GRandomBase()
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves an uniform_01 item. Thus function, together with the min() and
 * max() functions make it possible to use GRandomBase as a generator for
 * boost's random distributions.
 *
 * @return A random number taken from the evenRandom function
 */
GRandomBase::result_type GRandomBase::operator()() {
	return this->uniform_01<GRandomBase::result_type>();
}

/******************************************************************************/
/**
 * Returns the minimum value returned by evenRandom(). This
 * makes it possible to use GRandomBase as a generator for Boost's random
 * distributions.
 *
 * @return The minimum value returned by evenRandom()
 */
GRandomBase::result_type GRandomBase::min() const {
	return min_value;
}

/******************************************************************************/
/**
 * Returns the maximum value returned by evenRandom(). This
 * makes it possible to use GRandomBase as a generator for Boost's random
 * distributions.
 *
 * @return The maximum value returned by evenRandom()
 */
GRandomBase::result_type GRandomBase::max() const {
	return max_value;
}

/******************************************************************************/
/**
 * This function returns true with a probability "probability", otherwise false.
 *
 * @param p The probability for the value "true" to be returned
 * @return A boolean value, which will be true with a user-defined likelihood
 */
bool GRandomBase::weighted_bool(const double& probability) {
#ifdef DEBUG
	assert(probability>=0. && probability<=1.);
#endif
	return ((uniform_01<double>()<probability)?true:false);
}

/******************************************************************************/
/**
 * This function produces boolean values with a 50% likelihood each for
 * true and false.
 *
 * @return Boolean values with a 50% likelihood for true/false respectively
 */
bool GRandomBase::uniform_bool() {
	return this->weighted_bool(0.5);
}

/******************************************************************************/
/**
 * Produces gaussian-distributed float random numbers with sigma 1 and mean 0
 *
 * @return float random numbers with a gaussian distribution
 */
template<>
float GRandomBase::normal_distribution<float>() {
	using namespace Gem::Common;

	if(fltGaussCacheAvailable_) {
		fltGaussCacheAvailable_ = false;
		return fltGaussCache_;
	}
	else {
#ifdef GEM_HAP_USE_BOXMULLER
		float rnr1 = uniform_01<float>();
		float rnr2 = uniform_01<float>();
		dblGaussCache_ = GSqrt(GFabs(-2.f * GLog(1.f - rnr1))) * GCos(2.f * (float)M_PI	* rnr2);
		dblGaussCacheAvailable_ = true;
		return GSqrt(GFabs(-2.f * GLog(1.f - rnr1))) * GSin(2.f * (float)M_PI	* rnr2);
#else // GEM_HAP_USE_BOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than GEM_HAP_USE_BOXMULLER
		float q, u1, u2;
		do {
			u1 = 2.f* uniform_01<float>() - 1.f;
			u2 = 2.f* uniform_01<float>() - 1.f;
			q = u1*u1 + u2*u2;
		} while (q > 1.f);
		q = GSqrt((-2.f*GLog(q))/q);
		fltGaussCache_ = u2 * q;
		fltGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/******************************************************************************/
/**
 * Produces gaussian-distributed double random numbers with sigma 1 and mean 0
 *
 * @return double random numbers with a gaussian distribution
 */
template<>
double GRandomBase::normal_distribution<double>() {
	using namespace Gem::Common;

	if(dblGaussCacheAvailable_) {
		dblGaussCacheAvailable_ = false;
		return dblGaussCache_;
	}
	else {
#ifdef GEM_HAP_USE_BOXMULLER
		double rnr1 = uniform_01<double>();
		double rnr2 = uniform_01<double>();
		dblGaussCache_ = GSqrt(GFabs(-2. * GLog(1. - rnr1))) * GCos(2. * M_PI	* rnr2);
		dblGaussCacheAvailable_ = true;
		return GSqrt(GFabs(-2. * GLog(1. - rnr1))) * GSin(2. * M_PI	* rnr2);
#else // GEM_HAP_USE_BOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than GEM_HAP_USE_BOXMULLER
		double q, u1, u2;
		do {
			u1 = 2.* uniform_01<double>() - 1.;
			u2 = 2.* uniform_01<double>() - 1.;
			q = u1*u1 + u2*u2;
		} while (q > 1.0);
		q = GSqrt((-2.*GLog(q))/q);
		dblGaussCache_ = u2 * q;
		dblGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/******************************************************************************/
/**
 * Produces gaussian-distributed long double random numbers with sigma 1 and mean 0
 *
 * @return double random numbers with a gaussian distribution
 */
#ifdef _GLIBCXX_HAVE_LOGL
template<>
long double GRandomBase::normal_distribution<long double>() {
	using namespace Gem::Common;

	if(ldblGaussCacheAvailable_) {
		ldblGaussCacheAvailable_ = false;
		return ldblGaussCache_;
	}
	else {
#ifdef GEM_HAP_USE_BOXMULLER
		long double rnr1 = uniform_01<long double>();
		long double rnr2 = uniform_01<long double>();
		ldblGaussCache_ = GSqrt(GFabs(-2. * GLog(1.l - rnr1))) * GCos(2.l * (long double)M_PI	* rnr2);
		ldblGaussCacheAvailable_ = true;
		return GSqrt(GFabs(-2.l * GLog(1.l - rnr1))) * GSin(2. * (long double)M_PI	* rnr2);
#else // GEM_HAP_USE_BOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than GEM_HAP_USE_BOXMULLER
		long double q, u1, u2;
		do {
			u1 = 2.l* uniform_01<long double>() - 1.l;
			u2 = 2.l* uniform_01<long double>() - 1.l;
			q = u1*u1 + u2*u2;
		} while (q > 1.0l);
		q = GSqrt((-2.l*GLog(q))/q);
		ldblGaussCache_ = u2 * q;
		ldblGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}
#endif /* _GLIBCXX_HAVE_LOGL */


/******************************************************************************/

/** @brief Uniformly distributed random numbers in the range [0,1[ */
template <>
double GRandomBase::uniform_01(
      boost::enable_if<boost::is_floating_point<double> >::type* dummy
) {
   return dbl_random01();
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
