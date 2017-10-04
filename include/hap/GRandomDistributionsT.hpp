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
#include "common/GCommonMathHelperFunctions.hpp"

namespace Gem {
namespace Hap {

///******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
///******************************************************************************/
///**
// * This class produces integer random numbers, using GRandomT<RAMDONPROXY> as
// * the random number engine. The class may only be instantiated for integral
// * values.
// */
//template <
//	typename int_type
//	, typename std::enable_if<std::is_integral<int_type>::value>::type* dummy = nullptr
//>
//class g_uniform_int {
//public:
//	 /**
//	  * The default constructor
//	  */
//	 g_uniform_int()
//		 : m_uniform_int_distribution(0,(std::numeric_limits<int_type>::max)())
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with an upper limit
//	  */
//	 explicit g_uniform_int(int_type r)
//		 : m_uniform_int_distribution(0,r)
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with a lower and upper limit
//	  */
//	 g_uniform_int(int_type l, int_type r)
//		 : m_uniform_int_distribution(l,r)
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization through a std::uniform_int_distribution<int_type>::param_type object
//	  */
//	 explicit g_uniform_int(const typename std::uniform_int_distribution<int_type>::param_type& params)
//		 : m_uniform_int_distribution(params)
//	 { /* nothing */ }
//
//	 /**
//	  * Returns uniformly distributed random numbers, using
//	  * the boundaries specified in the constructor
//	  */
//	 inline int_type operator()() const {
//		 return m_uniform_int_distribution(randomProxy());
//	 }
//
//	 /**
//     * Returns uniformly distributed random numbers between 0
//     * and an upper boundary
//     */
//	 inline int_type operator()(int_type r) const {
//		 return m_uniform_int_distribution (
//			 randomProxy()
//			 , typename std::uniform_int_distribution<int_type>::param_type(0,r)
//		 );
//	 }
//
//	 /**
//	  * Returns uniformly distributed random numbers using
//	  * a new set of boundaries.
//	  */
//	 inline int_type operator()(int_type l, int_type r) const {
//		 return m_uniform_int_distribution (
//			 randomProxy()
//			 , typename std::uniform_int_distribution<int_type>::param_type(l,r)
//		 );
//	 }
//
//	 /**
// 	  * Returns uniformly distributed random numbers using a param_type object
// 	  */
//	 inline int_type operator()(const typename std::uniform_int_distribution<int_type>::param_type& params) const {
//		 return m_uniform_int_distribution (
//			 randomProxy()
//			 , params
//		 );
//	 }
//
//private:
//	 /** @brief Uniformly distributed integer random numbers */
//	 mutable std::uniform_int_distribution<int_type> m_uniform_int_distribution;
//
//	 // Deleted copy and assignment operators. Copy-construction is possible
//	 // through the std::uniform_int_distribution<int_type>::param_type object
//	 g_uniform_int(const g_uniform_int<int_type>&) = delete;
//	 g_uniform_int<int_type>& operator=(const g_uniform_int<int_type>&) = delete;
//};
//
///******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
///******************************************************************************/
///**
// * This class produces evenly distributed floating point random numbers, using
// * GRandomT<RAMDONPROXY> as the random number engine. The lower boundary is
// * included in the value range, the upper boundary is not. The class may only be
// * instantiated for floating point values.
// */
//template <
//	typename fp_type
//	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
//>
//class g_uniform_real {
//public:
//	 /**
//	  * The default constructor. It will produce produce
//	  * random numbers in the half-open range [0:1[ .
//	  */
//	 g_uniform_real()
//		 : m_uniform_real_distribution(0.,1.)
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with an upper limit
//	  */
//	 explicit g_uniform_real(fp_type r)
//		 : m_uniform_real_distribution(0.,r)
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with a lower and upper limit
//	  */
//	 g_uniform_real(fp_type l, fp_type r)
//		 : m_uniform_real_distribution(l,r)
//	 { /* nothing */ }
//
//	 /**
// 	  * Initialization through a std::uniform_real_distribution<fp_type>::param_type object
// 	  */
//	 explicit g_uniform_real(const typename std::uniform_real_distribution<fp_type>::param_type& params)
//		 : m_uniform_real_distribution(params)
//	 { /* nothing */ }
//
//	 /**
//	  * Returns uniformly distributed random numbers in the half-
//	  * open range [0:1[ or using the boundaries specified in the constructor.
//	  */
//	 inline fp_type operator()() const {
//		 return m_uniform_real_distribution(randomProxy());
//	 }
//
//	 /**
//     * Returns uniformly distributed random numbers between 0
//     * and an upper boundary
//     */
//	 inline fp_type operator()(fp_type r) const {
//		 return m_uniform_real_distribution (
//			 randomProxy()
//			 , typename std::uniform_real_distribution<fp_type>::param_type(0.,r)
//		 );
//	 }
//
//	 /**
//	  * Returns uniformly distributed random numbers using
//	  * a new set of boundaries.
//	  */
//	 inline fp_type operator()(fp_type l, fp_type r) const {
//		 return m_uniform_real_distribution (
//			 randomProxy()
//			 , typename std::uniform_real_distribution<fp_type>::param_type(l,r)
//		 );
//	 }
//
//	 /**
//     * Returns uniformly distributed floating point random numbers using the specifications found in
//     * a param_type object
//     */
//	 inline fp_type operator()(const typename std::uniform_real_distribution<fp_type>::param_type& params) const {
//		 return m_uniform_real_distribution (
//			 randomProxy(), params
//		 );
//	 }
//
//private:
//	 /** @brief Uniformly distributed floating point random numbers */
//	 mutable std::uniform_real_distribution<fp_type> m_uniform_real_distribution;
//
//	 // Deleted copy and assignment operators. Copy-construction is possible
//	 // through the std::uniform_real_distribution<fp_type>::param_type object
//	 g_uniform_real(const g_uniform_real<fp_type>&) = delete;
//	 g_uniform_real<fp_type>& operator=(const g_uniform_real<fp_type>&) = delete;
//};
//
///******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
///******************************************************************************/
///**
// * This class produces boolean random numbers, using GRandomT<RAMDONPROXY> as the
// * random number engine. The probability for true vs. false may be passed
// * either to the contructor or to the operator(), or a param_type object may be
// * provided.
// */
//class g_boolean_distribution
//{
//public:
//	 /**
//	  * The default constructor. Implicit probability of 50% for "true".
//	  */
//	 g_boolean_distribution() : m_bernoulli_distribution()
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with a probability for "true".
//	  */
//	 explicit g_boolean_distribution(double p) : m_bernoulli_distribution(p)
//	 { /* nothing */ }
//
//	 /**
// 	  * Initialization through a std::uniform_real_distribution<fp_type>::param_type object
// 	  */
//	 explicit g_boolean_distribution(const std::bernoulli_distribution::param_type& params)
//		 : m_bernoulli_distribution(params)
//	 { /* nothing */ }
//
//	 /**
//	  * Returns boolean random numbers with a probability of 50% for true vs. false
//	  */
//	 inline bool operator()() const {
//		 return m_bernoulli_distribution(randomProxy());
//	 }
//
//	 /**
//     * Returns boolean random numbers with a likelihood of "p" for "true" values
//     */
//	 inline bool operator()(double p) const {
//		 return m_bernoulli_distribution (randomProxy(), std::bernoulli_distribution::param_type(p));
//	 }
//
//	 /**
// 	  * Returns uniformly distributed floating point random numbers using the specifications found
// 	  * in a param_type object
// 	  */
//	 inline bool operator()(const std::bernoulli_distribution::param_type& params) const {
//		 return m_bernoulli_distribution (randomProxy(), params);
//	 }
//private:
//	 /** @brief Boolean random numbers with configurable probability structure */
//	 mutable std::bernoulli_distribution m_bernoulli_distribution;
//
//	 // Deleted copy and assignment operators. Copy-construction is possible
//	 // through the std::bernoulli_distribution::param_type object
//	 g_boolean_distribution(const g_boolean_distribution&) = delete;
//	 g_boolean_distribution& operator=(const g_boolean_distribution&) = delete;
//};
//
///******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
///******************************************************************************/
///**
// * This class produces floating point random numbers with a normal distribution,
// * using GRandomT<RAMDONPROXY> as the random number engine. The class may only be
// * instantiated for floating point values.
// */
//template <
//	typename fp_type
//	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
//>
//class g_normal_distribution {
//public:
//	 /**
//	  * The default constructor (normal distribution with mean 0 and sigma 1)
//	  */
//	 g_normal_distribution()
//		 : m_normal_distribution(0.,1.)
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with mean and sigma
//	  */
//	 g_normal_distribution(fp_type mean, fp_type sigma)
//		 : m_normal_distribution(mean,sigma)
//	 { /* nothing */ }
//
//	 /**
// 	  * Initialization through a std::normal_distribution<fp_type>::param_type object
// 	  */
//	 explicit g_normal_distribution(const typename std::normal_distribution<fp_type>::param_type& params)
//		 : m_normal_distribution(params)
//	 { /* nothing */ }
//
//	 /**
//	  * Returns random numbers with a normal distribution, using
//	  * the parameters specified in the constructor
//	  */
//	 inline fp_type operator()() const {
//		 return m_normal_distribution(randomProxy());
//	 }
//
//	 /**
//	  * Returns random numbers with a normal distribution, using
//	  * a new set of mean and sigma.
//	  */
//	 inline fp_type operator()(fp_type mean, fp_type sigma) const {
//		 return m_normal_distribution (
//			 randomProxy()
//			 , typename std::normal_distribution<fp_type>::param_type(mean,sigma)
//		 );
//	 }
//
//	 /**
//     * Returns uniformly distributed floating point random numbers using the specifications found in
//     * a param_type object
//     */
//	 inline fp_type operator()(const typename std::normal_distribution<fp_type>::param_type& params) const {
//		 return m_normal_distribution (
//			 randomProxy(), params
//		 );
//	 }
//
//private:
//	 /** @brief Uniformly distributed floating point random numbers */
//	 mutable std::normal_distribution<fp_type> m_normal_distribution;
//
//	 // Deleted copy and assignment operators. Copy-construction is possible
//	 // through the std::normal_distribution<fp_type>::param_type object
//	 g_normal_distribution(const g_normal_distribution<fp_type>&) = delete;
//	 g_normal_distribution<fp_type>& operator=(const g_normal_distribution<fp_type>&) = delete;
//};

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
	  * This embedded class identifies parameters needed for the bi_normal_distribition
	  */
	 class param_type {
	 public:
		  using distribution_type = bi_normal_distribution<fp_type>;

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

	 /**
	  * The default constructor
	  */
	 bi_normal_distribution()
		 : m_params(fp_type(DEF_BINORM_MEAN),fp_type(DEF_BINORM_SIGMA1),fp_type(DEF_BINORM_SIGMA2),fp_type(DEF_BINORM_DISTANCE))
			, m_params_store(m_params)
	 { /* nothing */ }

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
	 bi_normal_distribution(const param_type& params)
		 : m_params(params)
			, m_params_store(params)
	 { /* nothing */ }

private:
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
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
///**
// * This class produces floating point random numbers with a bi-normal distribution,
// * using GRandomT<RAMDONPROXY> as the random number engine and Geneva's bi_normal_distribution
// * class. Essentially, compared to bi_normal_distribution, this class allows to access
// * bi_normal random numbers without knowledge of the underlying random number generator.
// * The class may only be instantiated for floating point values.
// */
//template <
//	typename fp_type
//	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
//>
//class g_bi_normal_distribution {
//public:
//	 /**
//	  * The default constructor (normal distribution with mean 0 and sigma 1)
//	  */
//	 g_bi_normal_distribution()
//		 : m_bi_normal_distribution(0.,1.,1.,0.5)
//	 { /* nothing */ }
//
//	 /**
//	  * Initialization with mean and sigma
//	  */
//	 g_bi_normal_distribution(
//		 fp_type mean
//		 , fp_type sigma1
//		 , fp_type sigma2
//		 , fp_type distance
//	 )
//		 : m_bi_normal_distribution(mean,sigma1, sigma2, distance)
//	 { /* nothing */ }
//
//	 /**
// 	  * Initialization through a std::normal_distribution<fp_type>::param_type object
// 	  */
//	 explicit g_bi_normal_distribution(const typename Gem::Hap::bi_normal_distribution<fp_type>::param_type& params)
//		 : m_bi_normal_distribution(params)
//	 { /* nothing */ }
//
//	 /**
//	  * Returns random numbers with a normal distribution, using
//	  * the parameters specified in the constructor
//	  */
//	 inline fp_type operator()() const {
//		 return m_bi_normal_distribution(randomProxy());
//	 }
//
//	 /**
//	  * Returns random numbers with a normal distribution, using
//	  * a new set of mean and sigma.
//	  */
//	 inline fp_type operator()(
//		 fp_type mean
//		 , fp_type sigma1
//		 , fp_type sigma2
//		 , fp_type distance
//    ) const {
//		 return m_bi_normal_distribution (
//			 randomProxy()
//			 , typename Gem::Hap::bi_normal_distribution<fp_type>::param_type(
//				mean, sigma1, sigma2, distance
//		    )
//		 );
//	 }
//
//	 /**
//     * Returns uniformly distributed floating point random numbers using the specifications found in
//     * a param_type object
//     */
//	 inline fp_type operator()(const typename Gem::Hap::bi_normal_distribution<fp_type>::param_type& params) const {
//		 return m_bi_normal_distribution (
//			 randomProxy()
//			 , params
//		 );
//	 }
//
//private:
//	 /** @brief Uniformly distributed floating point random numbers */
//	 mutable Gem::Hap::bi_normal_distribution<fp_type> m_bi_normal_distribution;
//
//	 // Deleted copy and assignment operators. Copy-construction is possible
//	 // through the Gem::Hap::bi_normal_distribution<fp_type>::param_type object
//	 g_bi_normal_distribution(const g_bi_normal_distribution<fp_type>&) = delete;
//	 g_bi_normal_distribution<fp_type>& operator=(const g_bi_normal_distribution<fp_type>&) = delete;
//};
//
///******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
///******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GRANDOMDISTRIBUTIONST_HPP_ */
