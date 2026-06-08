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
#include "common/GCommonMathHelperFunctionsT.hpp"

// Standard headers go here
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "geneva/par/GAdaptorT.hpp"
#include "hap/GRandomDistributionsT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * GNumBiGaussAdaptorT is used for the adaption of numeric types, by the addition of random numbers
 * distributed as two adjacent gaussians. Different numeric types may be used, including Boost's
 * integer representations. The type used needs to be specified as a template parameter. In comparison
 * to GNumGaussAdaptorT, an additional parameter "delta" is added, which represents the distance between
 * both gaussians. Just like sigma, delta can be subject to mutations. It is also possible to use
 * two different sigma/sigma_sigma values and adaption rates for both gaussians. Note that this adaptor
 * is experimental. Your mileage may vary.
 */
template <typename parameter_type, typename adaption_fp_type>
class GNumBiGaussAdaptorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GAdaptorT<parameter_type, adaption_fp_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // Save all necessary data
        ar &make_nvp(
            "GAdaptorT_num",
            boost::serialization::base_object<GAdaptorT<parameter_type, adaption_fp_type>>(*this)
        );
        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * Default constructor
     */
    GNumBiGaussAdaptorT() = default;

    /***************************************************************************/
    /**
     * Initialization of the parent class'es adaption probability.
     *
     * @param probability The likelihood for a adaption actually taking place
     */
    explicit GNumBiGaussAdaptorT(const adaption_fp_type &probability)
      : GAdaptorT<parameter_type, adaption_fp_type>(probability) { /* nothing */
    }

    /***************************************************************************/
    /**
     * A standard copy constructor. It assumes that the values of the other object are correct
     * and does no additional error checks.
     *
     * @param cp Another GNumBiGaussAdaptorT object
     */
    GNumBiGaussAdaptorT(const GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor. Empty, as we have no local, dynamically
     * allocated data.
     */
    ~GNumBiGaussAdaptorT() override = default;

    /***************************************************************************/
    /**
     * Determines whether the two sigmas of the double-gaussian should be identical
     *
     * @param use_symmetric_sigmas A boolean which determines whether the two sigmas of the double-gaussian should be identical
     */
    void setUseSymmetricSigmas(const bool &use_symmetric_sigmas) {
        use_symmetric_sigmas_ = use_symmetric_sigmas;
    }

    /***************************************************************************/
    /**
     * Retrieves the value of the "use_symmetric_sigmas_" variable
     *
     * @return The value of the "use_symmetric_sigmas_" variable
     */
    bool getUseSymmetricSigmas() const {
        return use_symmetric_sigmas_;
    }

    /***************************************************************************/
    /**
     * This function sets the value of the sigma1_ parameter. It is recommended
     * that the value lies in the range [0.:1.]. A value below 0 is not allowed.
     * Sigma is interpreted as a percentage of the allowed or desired value range
     * of the target variable.
     *
     * @param sigma1 The new value of the sigma_ parameter
     */
    void setSigma1(const adaption_fp_type &sigma1) {
        // Sigma1 must be in the allowed value range
        if(sigma1 < min_sigma1_ || sigma1 > max_sigma1_ || sigma1 < adaption_fp_type(0)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>::setSigma1(const adaption_fp_type&):"
                << '\n'
                << "sigma1 is not in the allowed range: " << '\n'
                << min_sigma1_ << " <= " << sigma1 << " < " << max_sigma1_ << '\n'
                << "If you want to use these values you need to" << '\n'
                << "adapt the allowed range first." << '\n'
            );
        }

        sigma1_ = sigma1;
    }

    /***************************************************************************/
    /**
     * Retrieves the current value of sigma1_.
     *
     * @return The current value of sigma1_
     */
    adaption_fp_type getSigma1() const {
        return sigma1_;
    }

    /***************************************************************************/
    /**
     * Sets the allowed value range of sigma1_. A minimum sigma1 of 0 will silently be adapted
     * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
     * which does not make sense.  Using 0. as lower boundary is however allowed for practical
     * reasons. Note that this function will also adapt sigma1 itself, if it falls outside of the
     * allowed range. It is not recommended (but not enforced) to set a max_sigma1 > 1, as sigma
     * is interpreted as a percentage of the allowed or desired value range of the target variable.
     *
     * @param min_sigma1 The minimum allowed value of sigma1_
     * @param max_sigma1 The maximum allowed value of sigma1_
     */
    void setSigma1Range(const adaption_fp_type &min_sigma1, const adaption_fp_type &max_sigma1) {
        using namespace Gem::Common;

        if(min_sigma1 < adaption_fp_type(0.) || min_sigma1 > max_sigma1 ||
           max_sigma1 < Gem::Common::narrow<adaption_fp_type>(DEFAULTMINSIGMA)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumBiGaussAdaptorT::setSigma1Range(const adaption_fp_type&, const adaption_fp_type&):" << '\n'
                << "Invalid values for min_sigma1 and max_sigma1 given: " << min_sigma1 << " / "
                << max_sigma1 << '\n'
            );
        }

        min_sigma1_ = min_sigma1;
        max_sigma1_ = max_sigma1;

        // Silently adapt min_sigma1_, if it is smaller than DEFAULTMINSIGMA. E.g., a value of 0 does not make sense
        if(min_sigma1_ < adaption_fp_type(DEFAULTMINSIGMA)) {
            min_sigma1_ = adaption_fp_type(DEFAULTMINSIGMA);
        }

        // Rectify sigma1_, if necessary
        enforceRangeConstraint(
            sigma1_,
            min_sigma1_,
            max_sigma1_,
            "GNumBiGaussAdaptorT<>::setSigma1Range()"
        );
    }

    /***************************************************************************/
    /**
     * Retrieves the allowed value range for sigma1. You can retrieve the values
     * like this: getSigma1Range().first , getSigmaRange().second .
     *
     * @return The allowed value range for sigma1
     */
    std::tuple<adaption_fp_type, adaption_fp_type> getSigma1Range() const {
        return std::make_tuple(min_sigma1_, max_sigma1_);
    }

    /***************************************************************************/
    /**
     * This function sets the values of the sigma_sigma1_ parameter. Values <= 0 mean "do not adapt
     * sigma1_". If you do want to prevent adaption of sigma1, you can also use the
     * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
     * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
     *
     * TODO: Cross-check suitable values
     *
     * @param sigma_sigma1 The new value of the sigma_sigma1_ parameter
     */
    void setSigma1AdaptionRate(const adaption_fp_type &sigma_sigma1) {
        sigma_sigma1_ = sigma_sigma1;
    }

    /***************************************************************************/
    /**
     * Retrieves the value of sigma_sigma1_ .
     *
     * @return The value of the sigma_sigma1_ parameter
     */
    adaption_fp_type getSigma1AdaptionRate() const {
        return sigma_sigma1_;
    }

    /***************************************************************************/
    /**
     * Convenience function that lets users set all relevant parameters of the sigma1_ parameter
     * at once
     *
     * @param sigma1 The initial value for the sigma1_ parameter
     * @param sigma_sigma1 The initial value for the sigma_sigma1_ parameter
     * @param min_sigma1 The minimal value allowed for sigma1_
     * @param max_sigma1 The maximum value allowed for sigma1_
     */
    void setAllSigma1(
        const adaption_fp_type &sigma1,
        const adaption_fp_type &sigma_sigma1,
        const adaption_fp_type &min_sigma1,
        const adaption_fp_type &max_sigma1
    ) {
        setSigma1AdaptionRate(sigma_sigma1);
        setSigma1Range(min_sigma1, max_sigma1);
        setSigma1(sigma1);
    }

    /***************************************************************************/
    /**
     * This function sets the value of the sigma2_ parameter. It is recommended
     * that the value lies in the range [0.:1.]. A value below 0 is not allowed.
     * Sigma is interpreted as a percentage of the allowed or desired value range
     * of the target variable.
     *
     * @param sigma2 The new value of the sigma_ parameter
     */
    void setSigma2(const adaption_fp_type &sigma2) {
        // Sigma2 must be in the allowed value range
        if(sigma2 < min_sigma2_ || sigma2 > max_sigma2_ || sigma2 < adaption_fp_type(0)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>::setSigma2(const adaption_fp_type&):"
                << '\n'
                << "sigma2 is not in the allowed range: " << '\n'
                << min_sigma2_ << " <= " << sigma2 << " < " << max_sigma2_ << '\n'
                << "If you want to use this value for sigma you need to" << '\n'
                << "adapt the allowed range first." << '\n'
            );
        }

        sigma2_ = sigma2;
    }

    /***************************************************************************/
    /**
     * Retrieves the current value of sigma2_.
     *
     * @return The current value of sigma2_
     */
    adaption_fp_type getSigma2() const {
        return sigma2_;
    }

    /***************************************************************************/
    /**
     * Sets the allowed value range of sigma2_. A minimum sigma2 of 0 will silently be adapted
     * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
     * which does not make sense.  Using 0. as lower boundary is however allowed for practical
     * reasons. Note that this function will also adapt sigma2 itself, if it falls outside of the
     * allowed range. It is not recommended (but not enforced) to set a max_sigma2 > 1, as sigma
     * is interpreted as a percentage of the allowed or desired value range of the target variable.
     *
     * @param min_sigma2 The minimum allowed value of sigma2_
     * @param max_sigma2 The maximum allowed value of sigma2_
     */
    void setSigma2Range(const adaption_fp_type &min_sigma2, const adaption_fp_type &max_sigma2) {
        using namespace Gem::Common;

        if(min_sigma2 < adaption_fp_type(0.) || min_sigma2 > max_sigma2 ||
           max_sigma2 < Gem::Common::narrow<adaption_fp_type>(DEFAULTMINSIGMA)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumBiGaussAdaptorT::setSigma2Range(const adaption_fp_type&, const adaption_fp_type&):" << '\n'
                << "Invalid values for min_sigma2 and max_sigma2 given: " << min_sigma2 << " / "
                << max_sigma2 << '\n'
            );
        }

        min_sigma2_ = min_sigma2;
        max_sigma2_ = max_sigma2;

        // Silently adapt min_sigma1_, if it is smaller than DEFAULTMINSIGMA. E.g., a value of 0 does not make sense
        if(min_sigma2_ < adaption_fp_type(DEFAULTMINSIGMA)) {
            min_sigma2_ = adaption_fp_type(DEFAULTMINSIGMA);
        }

        // Rectify sigma1_, if necessary
        enforceRangeConstraint(
            sigma2_,
            min_sigma2_,
            max_sigma2_,
            "GNumBiGaussAdaptorT<>::setSigma2Range()"
        );
    }

    /***************************************************************************/
    /**
     * Retrieves the allowed value range for sigma2. You can retrieve the values
     * like this: getSigma2Range().first , getSigmaRange().second .
     *
     * @return The allowed value range for sigma2
     */
    std::tuple<adaption_fp_type, adaption_fp_type> getSigma2Range() const {
        return std::make_tuple(min_sigma2_, max_sigma2_);
    }

    /***************************************************************************/
    /**
     * This function sets the values of the sigma_sigma2_ parameter. Values <= 0 mean "do not adapt
     * sigma2_". If you do want to prevent adaption of sigma1, you can also use the
     * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
     * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
     *
     * @param sigma_sigma2 The new value of the sigma_sigma2_ parameter
     */
    void setSigma2AdaptionRate(const adaption_fp_type &sigma_sigma2) {
        sigma_sigma2_ = sigma_sigma2;
    }

    /***************************************************************************/
    /**
     * Retrieves the value of sigma_sigma2_ .
     *
     * @return The value of the sigma_sigma2_ parameter
     */
    adaption_fp_type getSigma2AdaptionRate() const {
        return sigma_sigma2_;
    }

    /***************************************************************************/
    /**
     * Convenience function that lets users set all relevant parameters of the sigma2_ parameter
     * at once
     *
     * @param sigma2 The initial value for the sigma2_ parameter
     * @param sigma_sigma2 The initial value for the sigma_sigma2_ parameter
     * @param min_sigma2 The minimal value allowed for sigma2_
     * @param max_sigma2 The maximum value allowed for sigma2_
     */
    void setAllSigma2(
        const adaption_fp_type &sigma2,
        const adaption_fp_type &sigma_sigma2,
        const adaption_fp_type &min_sigma2,
        const adaption_fp_type &max_sigma2
    ) {
        setSigma2AdaptionRate(sigma_sigma2);
        setSigma2Range(min_sigma2, max_sigma2);
        setSigma2(sigma2);
    }

    /***************************************************************************/
    /**
     * This function sets the value of the delta_ parameter. It is recommended
     * that the value lies in the range [0.:0.5]. A value below 0 is not allowed.
     *
     * @param delta The new value of the sigma_ parameter
     */
    void setDelta(const adaption_fp_type &delta) {
        // Delta must be in the allowed value range
        if(delta < min_delta_ || delta > max_delta_ || delta < adaption_fp_type(0)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumBiGaussAdaptorT::setDelta(const adaption_fp_type&):" << '\n'
                << "delta is not in the allowed range: " << '\n'
                << min_delta_ << " <= " << delta << " < " << max_delta_ << '\n'
                << "If you want to use these values you need to" << '\n'
                << "adapt the allowed range first." << '\n'
            );
        }

        delta_ = delta;
    }

    /***************************************************************************/
    /**
     * Retrieves the current value of delta_.
     *
     * @return The current value of delta_
     */
    adaption_fp_type getDelta() const {
        return delta_;
    }

    /***************************************************************************/
    /**
     * Sets the allowed value range of delta_. A minimum delta of 0 will silently be adapted
     * to DEFAULTMINSIGMA, if that value is > 0. Note that this function will also adapt delta
     * itself, if it falls outside of the allowed range. A maximum of 0.5 for max_delta_ is
     * recommended, but not enforced.delta is interpreted as a percentage of the allowed or
     * desired value range of the target variable.
     *
     * @param min_delta The minimum allowed value of delta_
     * @param max_delta The maximum allowed value of delta_
     */
    void setDeltaRange(const adaption_fp_type &min_delta, const adaption_fp_type &max_delta) {
        if(min_delta < adaption_fp_type(0.) || min_delta > max_delta ||
           max_delta < Gem::Common::narrow<adaption_fp_type>(DEFAULTMINDELTA)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumBiGaussAdaptorT::setDeltaRange(const adaption_fp_type&, const adaption_fp_type&):" << '\n'
                << "Invalid values for min_delta and max_delta given: " << min_delta << " / "
                << max_delta << '\n'
            );
        }

        min_delta_ = min_delta;
        max_delta_ = max_delta;

        // Note: In contrast to setSigmaXRange(...) we allow a delta < DEFAULTMINDELTA
        // (as long as it is >= 0), as a delta of 0 makes sense

        // Rectify delta_, if necessary
        if(delta_ < min_delta_) {
            delta_ = min_delta_;
        }
        else if(delta_ > max_delta_) {
            delta_ = max_delta_;
        }
    }

    /***************************************************************************/
    /**
     * Retrieves the allowed value range for delta. You can retrieve the values
     * like this: getDeltaRange().first , getSigmaRange().second .
     *
     * @return The allowed value range for delta
     */
    std::tuple<adaption_fp_type, adaption_fp_type> getDeltaRange() const {
        return std::make_tuple(min_delta_, max_delta_);
    }

    /***************************************************************************/
    /**
     * This function sets the values of the sigma_sigma2_ parameter. Values <= 0 mean "do not adapt
     * delta_". If you do want to prevent adaption of delta_, you can also use the
     * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
     * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
     *
     * @param sigma_delta The new value of the sigma_delta_ parameter
     */
    void setDeltaAdaptionRate(const adaption_fp_type &sigma_delta) {
        sigma_delta_ = sigma_delta;
    }

    /***************************************************************************/
    /**
     * Retrieves the value of sigma_delta_ .
     *
     * @return The value of the sigma_delta_ parameter
     */
    adaption_fp_type getDeltaAdaptionRate() const {
        return sigma_delta_;
    }

    /***************************************************************************/
    /**
     * Convenience function that lets users set all relevant parameters of the delta_ parameter
     * at once
     *
     * @param delta The initial value for the delta_ parameter
     * @param sigma_delta The initial value for the sigma_delta_ parameter
     * @param min_delta The minimal value allowed for delta_
     * @param max_delta The maximum value allowed for delta_
     */
    void setAllDelta(
        const adaption_fp_type &delta,
        const adaption_fp_type &sigma_delta,
        const adaption_fp_type &min_delta,
        const adaption_fp_type &max_delta
    ) {
        setDeltaAdaptionRate(sigma_delta);
        setDeltaRange(min_delta, max_delta);
        setDelta(delta);
    }

    /***************************************************************************/
    /**
     * Allows to randomly initialize parameter members
     */
    bool randomInit(Gem::Hap::GRandomBase &gr) override {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        sigma1_ = GAdaptorT<parameter_type, adaption_fp_type>::uniform_real_distribution_(
            gr,
            typename std::uniform_real_distribution<adaption_fp_type>::param_type(min_sigma1_, max_sigma1_)
        );
        sigma2_ = GAdaptorT<parameter_type, adaption_fp_type>::uniform_real_distribution_(
            gr,
            typename std::uniform_real_distribution<adaption_fp_type>::param_type(min_sigma2_, max_sigma2_)
        );
        delta_ = GAdaptorT<parameter_type, adaption_fp_type>::uniform_real_distribution_(
            gr,
            typename std::uniform_real_distribution<adaption_fp_type>::param_type(min_delta_, max_delta_)
        );

        return true;
    }

protected:
    /***************************************************************************/
    // For performance reasons, so we do not have to go through access functions
    bool use_symmetric_sigmas_ =
        true; ///< Determines whether the sigmas of both gaussians should be the same

    adaption_fp_type sigma1_ = DEFAULTSIGMA; ///< The width of the first gaussian used to adapt values
    adaption_fp_type sigma_sigma1_ = DEFAULTSIGMASIGMA; ///< affects sigma1_ adaption
    adaption_fp_type min_sigma1_ = DEFAULTMINSIGMA;     ///< minimum allowed value for sigma1_
    adaption_fp_type max_sigma1_ = DEFAULTMAXSIGMA;     ///< maximum allowed value for sigma1_
    adaption_fp_type sigma2_ = DEFAULTSIGMA; ///< The width of the second gaussian used to adapt values
    adaption_fp_type sigma_sigma2_ = DEFAULTSIGMASIGMA; ///< affects sigma2_ adaption
    adaption_fp_type min_sigma2_ = DEFAULTMINSIGMA;     ///< minimum allowed value for sigma2_
    adaption_fp_type max_sigma2_ = DEFAULTMAXSIGMA;     ///< maximum allowed value for sigma2_
    adaption_fp_type delta_ = DEFAULTDELTA;            ///< The distance between both gaussians
    adaption_fp_type sigma_delta_ = DEFAULTSIGMADELTA;  ///< affects the adaption of delta_
    adaption_fp_type min_delta_ = DEFAULTMINDELTA;      ///< minimum allowed value for delta_
    adaption_fp_type max_delta_ = DEFAULTMAXDELTA;      ///< maximum allowed value for delta_

    Gem::Hap::bi_normal_distribution<adaption_fp_type>
        bi_normal_distribution_; ///< Access to random numbers with a bi_normal distribution

    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("use_symmetric_sigmas_", use_symmetric_sigmas_),
            Gem::Common::make_member("sigma1_", sigma1_),
            Gem::Common::make_member("sigma_sigma1_", sigma_sigma1_),
            Gem::Common::make_member("min_sigma1_", min_sigma1_),
            Gem::Common::make_member("max_sigma1_", max_sigma1_),
            Gem::Common::make_member("sigma2_", sigma2_),
            Gem::Common::make_member("sigma_sigma2_", sigma_sigma2_),
            Gem::Common::make_member("min_sigma2_", min_sigma2_),
            Gem::Common::make_member("max_sigma2_", max_sigma2_),
            Gem::Common::make_member("delta_", delta_),
            Gem::Common::make_member("sigma_delta_", sigma_delta_),
            Gem::Common::make_member("min_delta_", min_delta_),
            Gem::Common::make_member("max_delta_", max_delta_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("use_symmetric_sigmas_", use_symmetric_sigmas_),
            Gem::Common::make_member("sigma1_", sigma1_),
            Gem::Common::make_member("sigma_sigma1_", sigma_sigma1_),
            Gem::Common::make_member("min_sigma1_", min_sigma1_),
            Gem::Common::make_member("max_sigma1_", max_sigma1_),
            Gem::Common::make_member("sigma2_", sigma2_),
            Gem::Common::make_member("sigma_sigma2_", sigma_sigma2_),
            Gem::Common::make_member("min_sigma2_", min_sigma2_),
            Gem::Common::make_member("max_sigma2_", max_sigma2_),
            Gem::Common::make_member("delta_", delta_),
            Gem::Common::make_member("sigma_delta_", sigma_delta_),
            Gem::Common::make_member("min_delta_", min_delta_),
            Gem::Common::make_member("max_delta_", max_delta_)
        );
    }

    /***************************************************************************/
    /**
     * This function loads the data of another GNumBiGaussAdaptorT, camouflaged as a GAdaptorT.
     * We assume that the values given to us by the other object are correct and do no error checks.
     *
     * @param cp A copy of another GNumBiGaussAdaptorT, camouflaged as a GAdaptorT
     */
    void load_(const GAdaptorT<parameter_type, adaption_fp_type> *cp) override {
        // Check that we are dealing with a GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> reference independent of this object and convert the pointer
        const GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<parameter_type, adaption_fp_type>, GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>>(
                cp,
                this
            );

        // Load the data of our parent class ...
        GAdaptorT<parameter_type, adaption_fp_type>::load_(cp);

        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>>(
        GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> const &,
        GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GAdaptorT object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GAdaptorT<parameter_type, adaption_fp_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> reference independent of this object and convert the pointer
        const GNumBiGaussAdaptorT<parameter_type, adaption_fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<parameter_type, adaption_fp_type>, GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>>(
                cp,
                this
            );

        GToken token("GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GAdaptorT<parameter_type, adaption_fp_type>>(*this, *p_load, token);

        // ... and then the local data
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Adds a given property value to the vector or returns false, if the property
     * was not found.
     */
    bool
    customQueryProperty(const std::string &property, std::vector<std::any> &data) const override {
        if(property == "sigma1") {
            data.push_back(std::any(sigma1_));
        }
        else if(property == "sigma2") {
            data.push_back(std::any(sigma2_));
        }
        else if(property == "delta") {
            data.push_back(std::any(delta_));
        }
        else {
            return false;
        }

        return true;
    }

    /***************************************************************************/
    /**
     * This adaptor allows the evolutionary adaption of sigma_. This allows the
     * algorithm to adapt to changing geometries of the quality surface.
     *
     */
    void customAdaptAdaption(const parameter_type &, Gem::Hap::GRandomBase &gr) override {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        // The following random distribution slightly favours values < 1. Selection pressure
        // will keep the values higher if needed
        sigma1_ *= std::exp(
            GAdaptorT<parameter_type, adaption_fp_type>::normal_distribution_(
                gr,
                typename Gem::Hap::g_normal_distribution<adaption_fp_type>::param_type(0., std::abs(sigma_sigma1_))
            )
        );
        sigma2_ *= std::exp(
            GAdaptorT<parameter_type, adaption_fp_type>::normal_distribution_(
                gr,
                typename Gem::Hap::g_normal_distribution<adaption_fp_type>::param_type(0., std::abs(sigma_sigma2_))
            )
        );
        delta_ *= std::exp(
            GAdaptorT<parameter_type, adaption_fp_type>::normal_distribution_(
                gr,
                typename Gem::Hap::g_normal_distribution<adaption_fp_type>::param_type(0., std::abs(sigma_delta_))
            )
        );

        // Make sure valued don't get out of range
        enforceRangeConstraint(
            sigma1_,
            min_sigma1_,
            max_sigma1_,
            "GNumBiGaussAdaptorT<>::customAdaptAdaption() / 1"
        );
        enforceRangeConstraint(
            sigma2_,
            min_sigma2_,
            max_sigma2_,
            "GNumBiGaussAdaptorT<>::customAdaptAdaption() / 2"
        );
        enforceRangeConstraint(
            delta_,
            min_delta_,
            max_delta_,
            "GNumBiGaussAdaptorT<>::customAdaptAdaption() / 3"
        );
    }

    /***************************************************************************/
    /**
     * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
     * adaptions are defined in the derived classes.
     */
    void customAdaptions(parameter_type &, const parameter_type &, Gem::Hap::GRandomBase &gr) override = 0;

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        bool result = false;

        // Call the parent classes' functions
        if(GAdaptorT<parameter_type, adaption_fp_type>::modify_GUnitTests_()) {
            result = true;
        }

        // A relatively harmless change
        sigma_sigma1_ *= 1.1;
        result = true;

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumBiGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        GAdaptorT<parameter_type, adaption_fp_type>::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GNumBiGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        GAdaptorT<parameter_type, adaption_fp_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GNumBiGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

private:
    /***********************************************************************************/
    /** @brief Retrieves the id of the adaptor */
    Gem::Geneva::adaptorId getAdaptorId_() const override = 0;

    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GNumBiGaussAdaptorT");
    }

    /***************************************************************************/
    /** @brief This function creates a deep copy of this object */
    GAdaptorT<parameter_type, adaption_fp_type> *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost::serialization {
template <typename parameter_type, typename adaption_fp_type>
struct is_abstract<Gem::Geneva::Parameters::GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>>
  : public boost::true_type {};
template <typename parameter_type, typename adaption_fp_type>
struct is_abstract<const Gem::Geneva::Parameters::GNumBiGaussAdaptorT<parameter_type, adaption_fp_type>>
  : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
