/**
 * @file GRandomDistributionsT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <random>
#include <limits>

// Boost headers go here

#ifndef GENEVA_LIBRARY_COLLECTION_GRANDOMDISTRIBUTIONST_HPP_
#define GENEVA_LIBRARY_COLLECTION_GRANDOMDISTRIBUTIONST_HPP_

// Geneva headers go here

#include "hap/GRandomT.hpp"
#include "common/GMathHelperFunctions.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
/**
 * This class implements a random distribution consisting of two adjacent
 * normal distributions
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
class bi_normal_distribution
{
public:
	 typedef fp_type input_type;
	 typedef fp_type result_type;

	 /**************************************************************************/
	 /**
	  * This embedded class identifies parameters needed for the bi_normal_distribition
	  */
	 class param_type {
	 public:
		  typedef bi_normal_distribution<fp_type> distribution_type;

		  /**
 			* Constructs the parameters of a bi_normal_distribution<fp_type>
 			*
 			* Requires min <= max
 			*/
		  param_type(
			  fp_type mean
			  , fp_type sigma1
			  , fp_type sigma2
			  , fp_type distance
		  )
			  : m_mean(mean)
			  , m_sigma1(sigma1)
		  	  , m_sigma2(sigma2)
		  	  , m_distance(distance)
		  { /* nothing */ }

		  /**
		   * The copy constructor
		   */
		  param_type(const param_type& params)
			  : m_mean(params.m_mean)
			  , m_sigma1(params.m_sigma1)
			  , m_sigma2(params.m_sigma2)
			  , m_distance(params.m_distance)
		  { /* nothing */ }

		  /**
		   * The assignment operator
		   */
		  const param_type& operator=(const param_type& params) {
			  m_mean = params.m_mean;
			  m_sigma1 = params.m_sigma1;
			  m_sigma2 = params.m_sigma2;
			  m_distance = params.m_distance;

			  return *this;
		  }

		  /**
		   * Access to the mean() value
		   */
		  fp_type mean() const { return m_mean; }
		  /**
		   * Access to the sigma1 value
		   */
		  fp_type sigma1() const { return m_sigma1; }
		  /**
		   * Access to the sigma2 value
		   */
		  fp_type sigma2() const { return m_sigma2; }
		  /**
		   * Access to the distance value
		   */
		  fp_type distance() const { return m_distance; }

		  /**
		   * Compare for equality with another param_type object
		   */
		  bool operator==(const param_type& p) const {
			  if(m_mean != p.m_mean) return false;
			  if(m_sigma1 != p.sigma1()) return false;
			  if(m_sigma2 != p.sigma2()) return false;
			  if(m_distance != p.distance()) return false;

			  return true;
		  }

		  /**
 			* Compare for inequality with another param_type object
 			*/
		  bool operator!=(const param_type& p) const {
			  return !operator==(p);
		  }

	 private:
		  param_type() = delete;

		  const fp_type m_mean;
		  const fp_type m_sigma1;
		  const fp_type m_sigma2;
		  const fp_type m_distance;
	 };

	 /**
	  * Returns the middle between both peaks
	  */
	 fp_type mean() const { return m_params.mean(); }
	 /**
	  * Returns the sigma value of the first peak
	  */
	 fp_type sigma1() const { return m_params.sigma1(); }
	 /**
	  * Returns the sigma value of the second peak
	  */
	 fp_type sigma2() const { return m_params.sigma2(); }
	 /**
	  * Returns the distance between both peaks
	  */
	 fp_type distance() const { return m_params.distance(); }

	 /**
	  * Returns a parameter object holding information on the distribution parameters
	  */
	 const typename bi_normal_distribution<fp_type>::param_type& param() const {
		 return m_params;
	 }

	 /**
	  * Sets the distribution parameters from another param object
	  */
	 void param(const typename bi_normal_distribution<fp_type>::param_type& params) {
		 m_params = params;
	 }

	 /**
	  * Returns the minimum value of the distribution. As we are
	  * essentially dealing with gaussian distributions, any floating
	  * point value is allowed.
	  */
	 fp_type min() const {
		 return std::numeric_limits<fp_type>::lowest();
	 }

	 /**
 	  * Returns the minimum value of the distribution. As we are
	  * essentially dealing with gaussian distributions, any floating
	  * point value is allowed.
 	  */
	 fp_type max() const {
		 return std::numeric_limits<fp_type>::max();
	 }

	 /**
	  * Resets the distribution to the values used for its construction
	  */
	 void reset() {
		 m_params = m_params_store;
	 };

	 /**
 	  * Returns a the next random number with a bi_normal distribution
 	  * according to the data contained in the param_type object
 	  */
	 template <class T_Generator>
	 result_type operator()(T_Generator& g, const param_type& params) {
		 if (m_uniform_bool(g)) {
			 fp_type mean_left  = params.mean() - Gem::Common::gfabs(params.distance() / 2.);
		    fp_type sigma_left = params.sigma1();
			 return sigma_left*m_normal_distribution(g) + mean_left;
		 } else {
			 fp_type mean_right  = params.mean() + Gem::Common::gfabs(params.distance() / 2.);
			 fp_type sigma_right = params.sigma2();
			 return sigma_right*m_normal_distribution(g) + mean_right;
		 }
	 }

	 /**
 	  * Returns a the next random number with a bi_normal distribution,
 	  * using the distribution parameters stored internally.
     */
	 template <class T_Generator>
	 result_type operator()(T_Generator& g) {
	 	 return (*this)(g, m_params);
	 }

	 /**
	  * The standard constructor
	  */
	 bi_normal_distribution(
		 fp_type mean
		 , fp_type sigma1
		 , fp_type sigma2
		 , fp_type distance
	 )
		 : m_params(mean, sigma1, sigma2, distance)
	 	 , m_params_store(mean, sigma1, sigma2, distance)
	 { /* nothing */ }

	 /**
	  * Initialization with a param_type object
	  */
	 bi_normal_distribution(const param_type& params)
		 : m_params(params)
	 	 , m_params_store(params)
	 { /* nothing */ }

private:
	 bi_normal_distribution() = delete;
	 bi_normal_distribution(const bi_normal_distribution<fp_type>&) = delete;
	 const bi_normal_distribution<fp_type>& operator=(const bi_normal_distribution<fp_type>&) = delete;

	 std::normal_distribution<fp_type> m_normal_distribution; ///< Needed to form each gaussian "hill" of the distribution
	 std::bernoulli_distribution m_uniform_bool; ///< Needed to decide whether a gaussian is created for the left or right peak

	 param_type m_params; ///< The actual parameter values being used
	 const param_type m_params_store; ///< The values the distribution will be reset to when reset() is called
};

/******************************************************************************/
/**
 * Checks two bi_normal_distribution distributions for equality
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
bool operator==(
	const bi_normal_distribution<fp_type>& lhs
	, const bi_normal_distribution<fp_type>& rhs
) {
	return lhs.param() == rhs.param();
};

/******************************************************************************/
/**
 * Checks two bi_normal_distribution distributions for inequality
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
bool operator!=(
	const bi_normal_distribution<fp_type>& lhs
	, const bi_normal_distribution<fp_type>& rhs
) {
	return lhs.param() != rhs.param();
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GRANDOMDISTRIBUTIONST_HPP_ */
