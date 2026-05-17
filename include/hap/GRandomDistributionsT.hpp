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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <concepts>
#include <limits>
#include <random>

// Boost headers go here

// Geneva headers go here

#include "hap/GRandomT.hpp"

namespace Gem::Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements a random distribution consisting of two adjacent
 * normal distributions. It models the API common for std C++11 random distributions.
 */
template <std::floating_point fp_type>
class bi_normal_distribution {
public:
    using input_type = fp_type;
    using result_type = fp_type;

    /**************************************************************************/
    /**
	  * This embedded class identifies parameters needed for the bi_normal_distribution
	  */
    class param_type { // NOLINT(cppcoreguidelines-special-member-functions)
    public:
        using distribution_type = bi_normal_distribution<fp_type>;

        param_type() = delete;

        // Do not allow "empty" parameter types
        param_type(param_type &&) = delete;
        param_type &operator=(param_type &&) = delete;

        /**
 			* Constructs the parameters of a bi_normal_distribution<fp_type>
 			*
 			* Requires min <= max
 			*/
        param_type(fp_type mean, fp_type sigma1, fp_type sigma2, fp_type distance)
          : mean_(mean)
          , sigma1_(sigma1)
          , sigma2_(sigma2)
          , distance_(distance) { /* nothing */
        }

        /**
		   * The copy constructor
		   */
        param_type(const param_type &params)
          : mean_(params.mean_)
          , sigma1_(params.sigma1_)
          , sigma2_(params.sigma2_)
          , distance_(params.distance_) { /* nothing */
        }

        /**
		   * The assignment operator
		   */
        param_type &operator=(param_type const &params) {
            if(this == &params) {
                return *this;
            }
            mean_ = params.mean_;
            sigma1_ = params.sigma1_;
            sigma2_ = params.sigma2_;
            distance_ = params.distance_;

            return *this;
        }

        /**
		   * Access to the mean() value
		   */
        fp_type mean() const {
            return mean_;
        }
        /**
		   * Access to the sigma1 value
		   */
        fp_type sigma1() const {
            return sigma1_;
        }
        /**
		   * Access to the sigma2 value
		   */
        fp_type sigma2() const {
            return sigma2_;
        }
        /**
		   * Access to the distance value
		   */
        fp_type distance() const {
            return distance_;
        }

        /**
		   * Compare for equality with another param_type object
		   */
        bool operator==(const param_type &p) const {
            if(mean_ != p.mean_) {
                return false;
            }
            if(sigma1_ != p.sigma1()) {
                return false;
            }
            if(sigma2_ != p.sigma2()) {
                return false;
            }
            return distance_ == p.distance();
        }

        /**
 			* Compare for inequality with another param_type object
 			*/
        bool operator!=(const param_type &p) const {
            return not operator==(p);
        }

    private:
        fp_type mean_ = 0.;
        fp_type sigma1_ = 0.;
        fp_type sigma2_ = 0.;
        fp_type distance_ = 0.;
    };

    /********************************************************************************************/
    // Deleted or defaulted constructors and assignment operators

    bi_normal_distribution() = default;

    bi_normal_distribution(bi_normal_distribution<fp_type> &&) = delete;
    bi_normal_distribution<fp_type> &operator=(bi_normal_distribution<fp_type> &&) = delete;

    /********************************************************************************************/

    /**
	  * The standard constructor
	  */
    bi_normal_distribution(fp_type mean, fp_type sigma1, fp_type sigma2, fp_type distance)
      : params_(mean, sigma1, sigma2, distance)
      , params_store_(params_) { /* nothing */
    }

    /**
	  * Initialization with a param_type object
	  */
    explicit bi_normal_distribution(param_type const &params)
      : params_(params)
      , params_store_(params) { /* nothing */
    }

    /**
	  * The copy constructor
	  */
    bi_normal_distribution(bi_normal_distribution<fp_type> const &cp)
      : params_(cp.params_)
      , params_store_(cp.params_store_) { /* nothing */
    }

    /**
	  * Assignment operator
	  */
    bi_normal_distribution<fp_type> &operator=(bi_normal_distribution<fp_type> const &cp) {
        params_ = cp.params_;
        params_store_ = cp.params_store_;

        return *this;
    }

    /**
* Returns the middle between both peaks
*/
    fp_type mean() const {
        return params_.mean();
    }
    /**
     * Returns the sigma value of the first peak
     */
    fp_type sigma1() const {
        return params_.sigma1();
    }
    /**
     * Returns the sigma value of the second peak
     */
    fp_type sigma2() const {
        return params_.sigma2();
    }
    /**
     * Returns the distance between both peaks
     */
    fp_type distance() const {
        return params_.distance();
    }

    /**
     * Returns a parameter object holding information on the distribution parameters
     */
    const typename bi_normal_distribution<fp_type>::param_type &param() const {
        return params_;
    }

    /**
     * Sets the distribution parameters from another param object
     */
    void param(const typename bi_normal_distribution<fp_type>::param_type &params) {
        params_ = params;
    }

    /**
     * Returns the minimum value of the distribution. As we are
     * essentially dealing with gaussian distributions, any floating
     * point value is allowed.
     */
    fp_type(min)() const {
        return std::numeric_limits<fp_type>::lowest();
    }

    /**
      * Returns the minimum value of the distribution. As we are
     * essentially dealing with gaussian distributions, any floating
     * point value is allowed.
      */
    fp_type(max)() const {
        return (std::numeric_limits<fp_type>::max)();
    }

    /**
     * Resets the distribution to the values used for its construction
     */
    void reset() {
        params_ = params_store_;
    };

    /**
      * Returns a the next random number with a bi_normal distribution
      * according to the data contained in the param_type object
      */
    template <class T_Generator>
    result_type operator()(T_Generator &g, const param_type &params) {
        if(uniform_bool_(g)) {
            fp_type mean_left = params.mean() - std::abs(params.distance() / 2.);
            fp_type sigma_left = params.sigma1();
            return sigma_left * normal_distribution_(g) + mean_left;
        }
        else {
            fp_type mean_right = params.mean() + std::abs(params.distance() / 2.);
            fp_type sigma_right = params.sigma2();
            return sigma_right * normal_distribution_(g) + mean_right;
        }
    }

    /**
      * Returns a the next random number with a bi_normal distribution,
      * using the distribution parameters stored internally.
    */
    template <class T_Generator>
    result_type operator()(T_Generator &g) {
        return (*this)(g, params_);
    }

private:
    std::normal_distribution<fp_type>
        normal_distribution_{}; ///< Needed to form each gaussian "hill" of the distribution
    std::bernoulli_distribution
        uniform_bool_; ///< Needed to decide whether a gaussian is created for the left or right peak

    param_type params_{
        fp_type(DEF_BINORM_MEAN),
        fp_type(DEF_BINORM_SIGMA1),
        fp_type(DEF_BINORM_SIGMA2),
        fp_type(DEF_BINORM_DISTANCE)
    }; ///< The actual parameter values being used
    const param_type params_store_{
        params_
    }; ///< The values the distribution will be reset to when reset() is called
};

/******************************************************************************/
/**
 * Checks two bi_normal_distribution distributions for equality
 */
template <std::floating_point fp_type>
bool operator==(
    const bi_normal_distribution<fp_type> &lhs,
    const bi_normal_distribution<fp_type> &rhs
) {
    return lhs.param() == rhs.param();
};

/******************************************************************************/
/**
 * Checks two bi_normal_distribution distributions for inequality
 */
template <std::floating_point fp_type>
bool operator!=(
    const bi_normal_distribution<fp_type> &lhs,
    const bi_normal_distribution<fp_type> &rhs
) {
    return lhs.param() != rhs.param();
};

/******************************************************************************/

} /* namespace Gem::Hap */
