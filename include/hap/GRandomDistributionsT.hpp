/**
 * @file
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <random>
#include <limits>

// Boost headers go here

// Geneva headers go here

#include "hap/GRandomT.hpp"
#include "common/GCommonMathHelperFunctions.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements a random distribution consisting of two adjacent
 * normal distributions. It models the API common for std C++11 random distributions.
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
class bi_normal_distribution
{
public:
	 using input_type = fp_type;
	 using result_type = fp_type;

	 /**************************************************************************/
	 /**
	  * This embedded class identifies parameters needed for the bi_normal_distribution
	  */
	 class param_type {
	 public:
		  using distribution_type = bi_normal_distribution<fp_type>;

		  param_type() = delete;

		  // Do not allow "empty" parameter types
		  param_type(param_type &&) = delete;
	  	  param_type& operator=(param_type &&) = delete;

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
		  param_type& operator=(param_type const& params) {
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
			  return m_distance == p.distance();
		  }

		  /**
 			* Compare for inequality with another param_type object
 			*/
		  bool operator!=(const param_type& p) const {
			  return not operator==(p);
		  }

	 private:
	 	  fp_type m_mean = 0.;
		  fp_type m_sigma1 = 0.;
		  fp_type m_sigma2 = 0.;
		  fp_type m_distance = 0.;
	 };

	 /********************************************************************************************/
	 // Deleted or defaulted constructors and assignment operators

	 bi_normal_distribution() = default;

	 bi_normal_distribution(bi_normal_distribution<fp_type> &&) = delete;
	 bi_normal_distribution<fp_type> & operator=(bi_normal_distribution<fp_type> &&) = delete;

	 /********************************************************************************************/

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
		 , m_params_store(m_params)
	 { /* nothing */ }

	 /**
	  * Initialization with a param_type object
	  */
	 explicit bi_normal_distribution(param_type const & params)
		 : m_params(params)
		 , m_params_store(params)
	 { /* nothing */ }

	 /**
	  * The copy constructor
	  */
	 bi_normal_distribution(bi_normal_distribution<fp_type> const & cp)
	    : m_params(cp.m_params)
        , m_params_store(cp.m_params_store)
	 { /* nothing */ }

	 /**
	  * Assignment operator
	  */
	 bi_normal_distribution<fp_type>& operator=(bi_normal_distribution<fp_type> const & cp) {
	     m_params = cp.m_params;
	     m_params_store = cp.m_params_store;

	     return *this;
	 }

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
	fp_type (min)() const {
		return std::numeric_limits<fp_type>::lowest();
	}

	/**
      * Returns the minimum value of the distribution. As we are
     * essentially dealing with gaussian distributions, any floating
     * point value is allowed.
      */
	fp_type (max)() const {
		return (std::numeric_limits<fp_type>::max)();
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

private:
	 std::normal_distribution<fp_type> m_normal_distribution{}; ///< Needed to form each gaussian "hill" of the distribution
	 std::bernoulli_distribution m_uniform_bool{}; ///< Needed to decide whether a gaussian is created for the left or right peak

	 param_type m_params {
		 fp_type(DEF_BINORM_MEAN)
		 ,fp_type(DEF_BINORM_SIGMA1)
		 ,fp_type(DEF_BINORM_SIGMA2)
		 ,fp_type(DEF_BINORM_DISTANCE)
	 }; ///< The actual parameter values being used
	 const param_type m_params_store{m_params}; ///< The values the distribution will be reset to when reset() is called
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

