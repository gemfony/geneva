/**
 * @file GRandomT.hpp
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

// Boost headers go here
#include <boost/thread/thread.hpp>

#ifndef GRANDOMT_HPP_
#define GRANDOMT_HPP_

// Geneva headers go here
#include "hap/GRandomBase.hpp"
#include "hap/GRandomDefines.hpp"
#include "common/GLogger.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Access to different random number distributions, whose "raw" material is
 * produced in different ways. We only define the interface here. The actual
 * implementation can be found in the (partial) specializations of this class.
 */
template<Gem::Hap::RANDFLAVOURS s = Gem::Hap::RANDFLAVOURS::RANDOMPROXY>
class GRandomT
	: public Gem::Hap::GRandomBase {
public:
	/** @brief The default constructor */
	GRandomT();

	/** @brief The destructor */
	virtual ~GRandomT();

protected:
	/** @brief Uniformly distributed integer random numbers */
	virtual GRandomBase::result_type int_random();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class retrieves random numbers
 * in batches from a global random number factory. The functions provided by
 * GRandomBase then produce different types of random numbers from this raw material.
 * As the class derives from boost::noncopyable, it is not possible to assign other
 * objects or use copy constructors.
 */
template<>
class GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>
	: public Gem::Hap::GRandomBase
{
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
	GRandomT()
		: Gem::Hap::GRandomBase()
	  	, p_( /* empty */ )
	  	, grf_(GRANDOMFACTORY) // Make sure we have a local pointer to the factory
	{
		// Make sure we have a first random number package available
		getNewRandomContainer();
	}

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRandomT() {
		if (p_) {
			grf_->returnUsedPackage(p_);
		}
		grf_.reset();
	}

protected:
	/***************************************************************************/
	/**
	 * This function retrieves random number packages from a global
	 * factory and emits them one by one. Once a package has been fully
	 * used, it is discarded and a new package is obtained from the factory.
	 * Essentially this class thus acts as a random number proxy -- to the
	 * caller it appears as if random numbers are created locally. This function
	 * assumes that a valid container is already available.
	 */
	virtual GRandomBase::result_type int_random() {
		if (p_->empty()) {
			// Get rid of the old container ...
			grf_->returnUsedPackage(p_);
			// ... then get a new one
			getNewRandomContainer();
		}
		return p_->next();
	}

private:
	/***************************************************************************/
	/**
	 * (Re-)Initialization of p_. Checks that a valid GRandomFactory still
	 * exists, then retrieves a new container.
	 */
	void getNewRandomContainer() {
		// Make sure we get rid of the old container
		p_.reset();

#ifdef DEBUG
		if(!grf_) {
		   glogger
		   << "In GRandomT<RANDOMPROXY>::getNewRandomContainer(): Error!" << std::endl
         << "No connection to GRandomFactory object." << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

#ifdef DEBUG
		std::uint32_t nRetries = 0;
#endif /* DEBUG */

		// Try until a valid container has been received. new01Container has
		// a timeout of DEFAULTFACTORYGETWAIT internally.
		while (!(p_ = grf_->getNewRandomContainer())) {
#ifdef DEBUG
		   nRetries++;
#endif /* DEBUG */
		}

#ifdef DEBUG
		if(nRetries>1) {
		   std::cout << "Info: Had to try " << nRetries << " times to retrieve a valid random number container." << std::endl;
		}
#endif /* DEBUG */
	}


	/***************************************************************************/
	/** @brief Holds the container of uniform random numbers */
	std::shared_ptr <random_container> p_;
	/** @brief A local copy of the global GRandomFactory */
	std::shared_ptr <Gem::Hap::GRandomFactory> grf_;
};

/** @brief Convenience typedef */
typedef GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> GRandom;

/***************************************************************************/
/** @brief Central access to a random number generator through thread-local storage */
boost::thread_specific_ptr<Gem::Hap::GRandom>& gr_tls_ptr();

// Syntactic sugar
#define GRANDOM_TLS (*(Gem::Hap::gr_tls_ptr()))
#define GRANDOM_TLS_PTR Gem::Hap::gr_tls_ptr()

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class produces random numbers
 * locally. The functions provided by GRandomBase<> then produce different types
 * of random numbers from this raw material. A seed can be provided either to
 * the constructor, or is taken from the global seed manager (recommended) in
 * case the default constructor is used.
 */
template<>
class GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL>
	: public Gem::Hap::GRandomBase
{
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
	GRandomT()
		: Gem::Hap::GRandomBase()
	  	, rng_(GRANDOMFACTORY->getSeed())
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRandomT()
	{ /* nothing */ }

protected:
	/***************************************************************************/
	/**
	 * This function produces uniform random numbers locally.
	 */
	virtual GRandomBase::result_type int_random() {
		return rng_();
	}

private:
	/***************************************************************************/
	/** @brief The actual generator for local random number creation */
	G_BASE_GENERATOR rng_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class produces integer random numbers, using GRandomT<RAMDONPROXY> as
 * the random number engine. The class may only be instantiated for integral
 * values.
 */
template <
	typename int_type
	, typename std::enable_if<std::is_integral<int_type>::value>::type* dummy = nullptr
>
class g_uniform_int {
public:
	 /**
	  * The default constructor
	  */
	 g_uniform_int()
		 : m_uniform_int_distribution(0,std::numeric_limits<int_type>::max())
	 { /* nothing */ }

	 /**
	  * Initialization with an upper limit
	  */
	 explicit g_uniform_int(int_type r)
		 : m_uniform_int_distribution(0,r)
	 { /* nothing */ }

	 /**
	  * Initialization with a lower and upper limit
	  */
	 g_uniform_int(int_type l, int_type r)
		 : m_uniform_int_distribution(l,r)
	 { /* nothing */ }

	 /**
	  * Initialization through a std::uniform_int_distribution<int_type>::param_type object
	  */
	 explicit g_uniform_int(const typename std::uniform_int_distribution<int_type>::param_type& params)
		 : m_uniform_int_distribution(params)
	 { /* nothing */ }

	 /**
	  * Returns uniformly distributed random numbers, using
	  * the boundaries specified in the constructor
	  */
	 inline int_type operator()() const {
		 return m_uniform_int_distribution(GRANDOM_TLS);
	 }

	 /**
     * Returns uniformly distributed random numbers between 0
     * and an upper boundary
     */
	 inline int_type operator()(int_type r) const {
		 return m_uniform_int_distribution (
			 GRANDOM_TLS
			 , typename std::uniform_int_distribution<int_type>::param_type(0,r)
		 );
	 }

	 /**
	  * Returns uniformly distributed random numbers using
	  * a new set of boundaries.
	  */
	 inline int_type operator()(int_type l, int_type r) const {
		 return m_uniform_int_distribution (
			 GRANDOM_TLS
			 , typename std::uniform_int_distribution<int_type>::param_type(l,r)
		 );
	 }

	 /**
 	  * Returns uniformly distributed random numbers using a param_type object
 	  */
	 inline int_type operator()(const typename std::uniform_int_distribution<int_type>::param_type& params) const {
		 return m_uniform_int_distribution (
			 GRANDOM_TLS, params
		 );
	 }

private:
	 /** @brief Uniformly distributed integer random numbers */
	 mutable std::uniform_int_distribution<int_type> m_uniform_int_distribution;

	 // Deleted copy and assignment operators. Copy-construction is possible
	 // through the std::uniform_int_distribution<int_type>::param_type object
	 g_uniform_int(const g_uniform_int<int_type>&) = delete;
	 g_uniform_int<int_type>& operator=(const g_uniform_int<int_type>&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class produces evenly distributed floating point random numbers, using
 * GRandomT<RAMDONPROXY> as the random number engine. The lower boundary is
 * included in the value range, the upper boundary is not. The class may only be
 * instantiated for floating point values.
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
class g_uniform_real {
public:
	 /**
	  * The default constructor. It will produce produce
	  * random numbers in the half-open range [0:1[ .
	  */
	 g_uniform_real()
		 : m_uniform_real_distribution(0.,1.)
	 { /* nothing */ }

	 /**
	  * Initialization with an upper limit
	  */
	 explicit g_uniform_real(fp_type r)
		 : m_uniform_real_distribution(0.,r)
	 { /* nothing */ }

	 /**
	  * Initialization with a lower and upper limit
	  */
	 g_uniform_real(fp_type l, fp_type r)
		 : m_uniform_real_distribution(l,r)
	 { /* nothing */ }

	 /**
 	  * Initialization through a std::uniform_real_distribution<fp_type>::param_type object
 	  */
	 explicit g_uniform_real(const typename std::uniform_real_distribution<fp_type>::param_type& params)
		 : m_uniform_real_distribution(params)
	 { /* nothing */ }

	 /**
	  * Returns uniformly distributed random numbers in the half-
	  * open range [0:1[ or using the boundaries specified in the constructor.
	  */
	 inline fp_type operator()() const {
		 return m_uniform_real_distribution(GRANDOM_TLS);
	 }

	 /**
     * Returns uniformly distributed random numbers between 0
     * and an upper boundary
     */
	 inline fp_type operator()(fp_type r) const {
		 return m_uniform_real_distribution (
			 GRANDOM_TLS
			 , typename std::uniform_real_distribution<fp_type>::param_type(0.,r)
		 );
	 }

	 /**
	  * Returns uniformly distributed random numbers using
	  * a new set of boundaries.
	  */
	 inline fp_type operator()(fp_type l, fp_type r) const {
		 return m_uniform_real_distribution (
			 GRANDOM_TLS
			 , typename std::uniform_real_distribution<fp_type>::param_type(l,r)
		 );
	 }

	 /**
     * Returns uniformly distributed floating point random numbers using the specifications found in
     * a param_type object
     */
	 inline fp_type operator()(const typename std::uniform_real_distribution<fp_type>::param_type& params) const {
		 return m_uniform_real_distribution (
			 GRANDOM_TLS, params
		 );
	 }

private:
	 /** @brief Uniformly distributed floating point random numbers */
	 mutable std::uniform_real_distribution<fp_type> m_uniform_real_distribution;

	 // Deleted copy and assignment operators. Copy-construction is possible
	 // through the std::uniform_real_distribution<fp_type>::param_type object
	 g_uniform_real(const g_uniform_real<fp_type>&) = delete;
	 g_uniform_real<fp_type>& operator=(const g_uniform_real<fp_type>&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class produces boolean random numbers, using GRandomT<RAMDONPROXY> as the
 * random number engine. The probability for true vs. false may be passed
 * either to the contructor or to the operator(), or a param_type object may be
 * provided.
 */
class g_boolean_distribution
{
public:
	 /**
	  * The default constructor. Implicit probability of 50% for "true".
	  */
	 g_boolean_distribution() : m_bernoulli_distribution()
	 { /* nothing */ }

	 /**
	  * Initialization with a probability for "true".
	  */
	 explicit g_boolean_distribution(double p) : m_bernoulli_distribution(p)
	 { /* nothing */ }

	 /**
 	  * Initialization through a std::uniform_real_distribution<fp_type>::param_type object
 	  */
	 explicit g_boolean_distribution(const std::bernoulli_distribution::param_type& params)
		 : m_bernoulli_distribution(params)
	 { /* nothing */ }

	 /**
	  * Returns boolean random numbers with a probability of 50% for true vs. false
	  */
	 inline bool operator()() const {
		 return m_bernoulli_distribution(GRANDOM_TLS);
	 }

	 /**
     * Returns boolean random numbers with a likelihood of "p" for "true" values
     */
	 inline bool operator()(double p) const {
		 return m_bernoulli_distribution (
			 GRANDOM_TLS
			 , std::bernoulli_distribution::param_type(p)
		 );
	 }

	 /**
 	  * Returns uniformly distributed floating point random numbers using the specifications found
 	  * in a param_type object
 	  */
	 inline bool operator()(const std::bernoulli_distribution::param_type& params) const {
		 return m_bernoulli_distribution (
			 GRANDOM_TLS, params
		 );
	 }
private:
	 /** @brief Boolean random numbers with configurable probability structure */
	 mutable std::bernoulli_distribution m_bernoulli_distribution;

	 // Deleted copy and assignment operators. Copy-construction is possible
	 // through the std::bernoulli_distribution::param_type object
	 g_boolean_distribution(const g_boolean_distribution&) = delete;
	 g_boolean_distribution& operator=(const g_boolean_distribution&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class produces floating point random numbers with a normal distribution,
 * using GRandomT<RAMDONPROXY> as the random number engine. The class may only be
 * instantiated for floating point values.
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
class g_normal_distribution {
public:
	 /**
	  * The default constructor (normal distribution with mean 0 and sigma 1)
	  */
	 g_normal_distribution()
		 : m_normal_distribution(0.,1.)
	 { /* nothing */ }

	 /**
	  * Initialization with mean and sigma
	  */
	 g_normal_distribution(fp_type mean, fp_type sigma)
		 : m_normal_distribution(mean,sigma)
	 { /* nothing */ }

	 /**
 	  * Initialization through a std::normal_distribution<fp_type>::param_type object
 	  */
	 explicit g_normal_distribution(const typename std::normal_distribution<fp_type>::param_type& params)
		 : m_normal_distribution(params)
	 { /* nothing */ }

	 /**
	  * Returns random numbers with a normal distribution, using
	  * the parameters specified in the constructor
	  */
	 inline fp_type operator()() const {
		 return m_normal_distribution(GRANDOM_TLS);
	 }

	 /**
	  * Returns random numbers with a normal distribution, using
	  * a new set of mean and sigma.
	  */
	 inline fp_type operator()(fp_type mean, fp_type sigma) const {
		 return m_normal_distribution (
			 GRANDOM_TLS
			 , typename std::normal_distribution<fp_type>::param_type(mean,sigma)
		 );
	 }

	 /**
     * Returns uniformly distributed floating point random numbers using the specifications found in
     * a param_type object
     */
	 inline fp_type operator()(const typename std::normal_distribution<fp_type>::param_type& params) const {
		 return m_normal_distribution (
			 GRANDOM_TLS, params
		 );
	 }

private:
	 /** @brief Uniformly distributed floating point random numbers */
	 mutable std::normal_distribution<fp_type> m_normal_distribution;

	 // Deleted copy and assignment operators. Copy-construction is possible
	 // through the std::normal_distribution<fp_type>::param_type object
	 g_normal_distribution(const g_normal_distribution<fp_type>&) = delete;
	 g_normal_distribution<fp_type>& operator=(const g_normal_distribution<fp_type>&) = delete;
};

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
/**
 * This class produces floating point random numbers with a bi-normal distribution,
 * using GRandomT<RAMDONPROXY> as the random number engine and Geneva's bi_normal_distribution
 * class. Essentially, compared to bi_normal_distribution, this class allows to access
 * bi_normal random numbers without knowledge of the underlying random number generator.
 * The class may only be instantiated for floating point values.
 */
template <
	typename fp_type
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* dummy = nullptr
>
class g_bi_normal_distribution {
public:
	 /**
	  * The default constructor (normal distribution with mean 0 and sigma 1)
	  */
	 g_bi_normal_distribution()
		 : m_bi_normal_distribution(0.,1.)
	 { /* nothing */ }

	 /**
	  * Initialization with mean and sigma
	  */
	 g_bi_normal_distribution(
		 fp_type mean
		 , fp_type sigma1
		 , fp_type sigma2
		 , fp_type distance
	 )
		 : m_bi_normal_distribution(mean,sigma1, sigma2, distance)
	 { /* nothing */ }

	 /**
 	  * Initialization through a std::normal_distribution<fp_type>::param_type object
 	  */
	 explicit g_bi_normal_distribution(const typename Gem::Hap::bi_normal_distribution<fp_type>::param_type& params)
		 : m_bi_normal_distribution(params)
	 { /* nothing */ }

	 /**
	  * Returns random numbers with a normal distribution, using
	  * the parameters specified in the constructor
	  */
	 inline fp_type operator()() const {
		 return m_bi_normal_distribution(GRANDOM_TLS);
	 }

	 /**
	  * Returns random numbers with a normal distribution, using
	  * a new set of mean and sigma.
	  */
	 inline fp_type operator()(
		 fp_type mean
		 , fp_type sigma1
		 , fp_type sigma2
		 , fp_type distance
    ) const {
		 return m_bi_normal_distribution (
			 GRANDOM_TLS
			 , typename Gem::Hap::bi_normal_distribution<fp_type>::param_type(
				mean, sigma1, sigma2, distance
		    )
		 );
	 }

	 /**
     * Returns uniformly distributed floating point random numbers using the specifications found in
     * a param_type object
     */
	 inline fp_type operator()(const typename Gem::Hap::bi_normal_distribution<fp_type>::param_type& params) const {
		 return m_bi_normal_distribution (
			 GRANDOM_TLS, params
		 );
	 }

private:
	 /** @brief Uniformly distributed floating point random numbers */
	 mutable Gem::Hap::bi_normal_distribution<fp_type> m_bi_normal_distribution;

	 // Deleted copy and assignment operators. Copy-construction is possible
	 // through the Gem::Hap::bi_normal_distribution<fp_type>::param_type object
	 g_bi_normal_distribution(const g_bi_normal_distribution<fp_type>&) = delete;
	 g_bi_normal_distribution<fp_type>& operator=(const g_bi_normal_distribution<fp_type>&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMT_HPP_ */
