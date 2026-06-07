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
#include <any>
#include <cmath>
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "geneva/par/GAdaptorT.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * GNumGaussAdaptorT represents an adaptor used for the adaption of numeric
 * types, by the addition of gaussian-distributed random numbers. Different numeric
 * types may be used, including Boost's integer representations.
 * The type used needs to be specified as a template parameter.
 */
template <typename parameter_type, typename adaption_fp_type>
class GNumGaussAdaptorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GAdaptorT<parameter_type, adaption_fp_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GAdaptorT_num",
            boost::serialization::base_object<GAdaptorT<parameter_type>>(*this)
        );
        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if adaption_fp_type really is a floating point type
    static_assert(
        std::is_floating_point_v<adaption_fp_type>,
        "adaption_fp_type should be a floating point type"
    );

public:
    /***************************************************************************/
    /**
	  * The standard constructor.
	  */
    GNumGaussAdaptorT() = default;

    /***************************************************************************/
    /**
	  * Initialization of the parent class'es adaption probability.
	  *
	  * @param probability The likelihood for a adaption actually taking place
	  */
    GNumGaussAdaptorT(const double &probability)
      : GAdaptorT<parameter_type>(probability) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * This constructor lets a user set all sigma parameters in one go.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigma_sigma The initial value for the sigmaSigma_ parameter
	  * @param min_sigma The minimal value allowed for sigma_
	  * @param max_sigma The maximal value allowed for sigma_
	  */
    GNumGaussAdaptorT(
        const adaption_fp_type &sigma,
        const adaption_fp_type &sigma_sigma,
        const adaption_fp_type &min_sigma,
        const adaption_fp_type &max_sigma
    ) {
        // These functions do error checks on their values
        setSigmaAdaptionRate(sigma_sigma);
        setSigmaRange(min_sigma, max_sigma);
        setSigma(
            sigma
        ); // Must be set last so an error check for compliance with the boundaries can be made

        sigma_reset_ = sigma_;
    }

    /***************************************************************************/
    /**
	  * This constructor lets a user set all parameters in one go.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigma_sigma The initial value for the sigmaSigma_ parameter
	  * @param min_sigma The minimal value allowed for sigma_
	  * @param max_sigma The maximal value allowed for sigma_
	  * @param probability The likelihood for a adaption actually taking place
	  */
    GNumGaussAdaptorT(
        const adaption_fp_type &sigma,
        const adaption_fp_type &sigma_sigma,
        const adaption_fp_type &min_sigma,
        const adaption_fp_type &max_sigma,
        const double &probability
    )
      : GAdaptorT<parameter_type>(probability) {
        // These functions do error checks on their values
        setSigmaAdaptionRate(sigma_sigma);
        setSigmaRange(min_sigma, max_sigma);
        setSigma(
            sigma
        ); // Must be set last so an error check for compliance with the boundaries can be made

        sigma_reset_ = sigma_;
    }

    /***************************************************************************/
    /**
	  * A standard copy constructor. It assumes that the values of the other object are correct
	  * and does no additional error checks.
	  *
	  * @param cp Another GNumGaussAdaptorT object
	  */
    GNumGaussAdaptorT(const GNumGaussAdaptorT<parameter_type, adaption_fp_type> &cp) = default;

    /***************************************************************************/
    /**
	  * The standard destructor. Empty, as we have no local, dynamically
	  * allocated data.
	  */
    ~GNumGaussAdaptorT() override = default;

    /***************************************************************************/
    /**
	  * This function sets the value of the sigma_ parameter. It is recommended
	  * that the value lies in the range [0.:1.]. A value below 0 is not allowed.
	  * Sigma is interpreted as a percentage of the allowed or desired value range
	  * of the target variable. Setting the allowed value range will enforce
	  * a constraint of [0,1], so it is not necessary in this function.
	  *
	  * @param sigma The new value of the sigma_ parameter
	  */
    void setSigma(const adaption_fp_type &sigma) {
        // Sigma must be in the allowed value range.
        if(not Gem::Common::checkRangeCompliance<adaption_fp_type>(
               sigma,
               min_sigma_,
               max_sigma_,
               "GNumGaussAdaptorT<>::setSigma(" + Gem::Common::to_string(sigma) + ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumGaussAdaptorT::setSigma(const adaption_fp_type&):" << '\n'
                << "sigma is not in the allowed range: " << '\n'
                << min_sigma_ << " <= " << sigma << " < " << max_sigma_ << '\n'
                << "If you want to use these values you need to" << '\n'
                << "adapt the allowed range first." << '\n'
            );
        }

        sigma_ = sigma;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current value of "reset" sigma_.
	  *
	  * @return The current value of sigma_reset_
	  */
    adaption_fp_type getResetSigma() const {
        return sigma_reset_;
    }

    /***************************************************************************/
    /**
	  * This function sets the value of the sigma_reset_ parameter. It is used
	  * to rall back sigma_, if the optimization process has stalled
	  *
	  * @param sigma_reset The new value of the sigma_ parameter
	  */
    void setResetSigma(const adaption_fp_type &sigma_reset) {
        // Sigma must be in the allowed value range.
        if(not Gem::Common::checkRangeCompliance<adaption_fp_type>(
               sigma_reset,
               min_sigma_,
               max_sigma_,
               "GNumGaussAdaptorT<>::setResetSigma(" + Gem::Common::to_string(sigma_reset) + ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumGaussAdaptorT::setResetSigma(const adaption_fp_type&):" << '\n'
                << "sigma_reset is not in the allowed range: " << '\n'
                << min_sigma_ << " <= " << sigma_reset << " < " << max_sigma_ << '\n'
                << "If you want to use these values you need to" << '\n'
                << "adapt the allowed range first." << '\n'
            );
        }

        sigma_reset_ = sigma_reset;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current value of sigma_.
	  *
	  * @return The current value of sigma_
	  */
    adaption_fp_type getSigma() const {
        return sigma_;
    }

    /***************************************************************************/
    /**
	  * Sets the allowed value range of sigma_. A minimum sigma of 0 will silently be adapted
	  * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	  * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	  * reasons. Note that this function will also adapt sigma itself, if it falls outside of the
	  * allowed range. It is not recommended (but not enforced) to set a max_sigma > 1, as sigma
	  * is interpreted as a percentage of the allowed or desired value range of the target variable.
	  *
	  * @param min_sigma The minimum allowed value of sigma_
	  * @param max_sigma The maximum allowed value of sigma_
	  */
    void setSigmaRange(const adaption_fp_type &min_sigma, const adaption_fp_type &max_sigma) {
        using namespace Gem::Common;

        if(min_sigma < adaption_fp_type(0.) || min_sigma > max_sigma || max_sigma > adaption_fp_type(1.)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumGaussAdaptorT::setSigmaRange(const adaption_fp_type&, const adaption_fp_type&):" << '\n'
                << "Invalid values for min_sigma and max_sigma given: " << min_sigma << " / "
                << max_sigma << '\n'
                << "Expected a range [0:1]. Note: Sigma is a percentage of the allowed or" << '\n'
                << "preferred value range."
            );
        }

        min_sigma_ = min_sigma;
        if(min_sigma_ < DEFAULTMINSIGMA) {
            min_sigma_ = DEFAULTMINSIGMA; // Silently adapt min_sigma
        }
        max_sigma_ = max_sigma;

        // Rectify sigma_ and reset_sigma_, if necessary
        Gem::Common::enforceRangeConstraint<adaption_fp_type>(
            sigma_,
            std::max(adaption_fp_type(min_sigma_), adaption_fp_type(DEFAULTMINSIGMA)),
            max_sigma_,
            "GNumGaussAdaptorT<>::setSigmaRange() / 1"
        );
        Gem::Common::enforceRangeConstraint<adaption_fp_type>(
            sigma_reset_,
            std::max(adaption_fp_type(min_sigma_), adaption_fp_type(DEFAULTMINSIGMA)),
            max_sigma_,
            "GNumGaussAdaptorT<>::setSigmaRange() / 2"
        );
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of valid ranges is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * Setting of invalid ranges is tested in GNumGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the allowed value range for sigma. You can retrieve the values
	  * like this: std::get<0>(getSigmaRange()) , std::get<1>(getSigmaRange()) .
	  *
	  * @return The allowed value range for sigma
	  */
    std::tuple<adaption_fp_type, adaption_fp_type> getSigmaRange() const {
        return std::make_tuple(min_sigma_, max_sigma_);
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * This function sets the values of the sigmaSigma_ parameter. Values <= 0 mean "do not adapt
	  * sigma". If you do want to prevent adaption of sigma, you can also use the
	  * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	  * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	  *
	  * @param sigma_sigma The new value of the sigmaSigma_ parameter
	  */
    void setSigmaAdaptionRate(const adaption_fp_type &sigma_sigma) {
        // sigma_sigma <= 0 is a valid contract ("do not adapt sigma"); only
        // non-finite values (NaN / +-inf) are rejected -- they would otherwise
        // propagate silently through the adaption and corrupt the whole run.
        if(not std::isfinite(sigma_sigma)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNumGaussAdaptorT::setSigmaAdaptionRate(): Error!" << '\n'
                << "Received a non-finite sigma_sigma value: " << sigma_sigma << '\n'
            );
        }
        sigma_sigma_ = sigma_sigma;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of valid adaption rates is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * Setting of invalid adaption rates is tested in GNumGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the value of sigmaSigma_ .
	  *
	  * @return The value of the sigmaSigma_ parameter
	  */
    adaption_fp_type getSigmaAdaptionRate() const {
        return sigma_sigma_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of adaption rates is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Convenience function that lets users set all relevant parameters of this class
	  * at once.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigma_sigma The initial value for the sigmaSigma_ parameter
	  * @param min_sigma The minimal value allowed for sigma_
	  * @param max_sigma The maximum value allowed for sigma_
	  */
    void setAll(
        const adaption_fp_type &sigma,
        const adaption_fp_type &sigma_sigma,
        const adaption_fp_type &min_sigma,
        const adaption_fp_type &max_sigma
    ) {
        setSigmaAdaptionRate(sigma_sigma);
        setSigmaRange(min_sigma, max_sigma);
        setSigma(sigma);
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GNumGaussAdaptorT<parameter_type, adaption_fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Prints diagnostic messages
	  *
	  * @return The diagnostic message
	  */
    std::string printDiagnostics() const override {
        std::ostringstream diag; // NOLINT(cppcoreguidelines-init-variables)
        std::tuple<adaption_fp_type, adaption_fp_type> sigma_range = getSigmaRange();

        diag << "Diagnostic message by GNumAdaptorT<parameter_type,adaption_fp_type>" << '\n'
             << "with typeid(parameter_type).name() = " << typeid(parameter_type).name() << '\n'
             << "and typeid(adaption_fp_type).name() = " << typeid(adaption_fp_type).name() << " :" << '\n'
             << "getSigma() = " << getSigma() << '\n'
             << "getResetSigma() = " << getResetSigma() << '\n'
             << "getSigmaRange() = " << std::get<0>(sigma_range) << " --> "
             << std::get<1>(sigma_range) << '\n'
             << "getSigmaAdaptionRate() = " << getSigmaAdaptionRate() << '\n';

        return diag.str();
    }

    /***************************************************************************/
    /**
	  * Triggers updates when the optimization process has stalled. This function
	  * resets the sigma value to its original value and calls the parent class'es function
	  *
	  * @param n_stalls The number of consecutive stalls up to this point
	  * @param range A typical value range for type T
	  * @return A boolean indicating whether updates were performed
	  */
    bool updateOnStall(const std::size_t &n_stalls, const parameter_type &range) override {
        // Call our parent class'es function
        GAdaptorT<parameter_type>::updateOnStall(n_stalls, range);

        // Reset the adaption probability
        if(sigma_ == sigma_reset_) {
            return false;
        }
                    sigma_ = sigma_reset_;
            return true;
       
    }

protected:
    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("sigma_", sigma_),
            Gem::Common::make_member("sigma_reset_", sigma_reset_),
            Gem::Common::make_member("sigma_sigma_", sigma_sigma_),
            Gem::Common::make_member("min_sigma_", min_sigma_),
            Gem::Common::make_member("max_sigma_", max_sigma_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("sigma_", sigma_),
            Gem::Common::make_member("sigma_reset_", sigma_reset_),
            Gem::Common::make_member("sigma_sigma_", sigma_sigma_),
            Gem::Common::make_member("min_sigma_", min_sigma_),
            Gem::Common::make_member("max_sigma_", max_sigma_)
        );
    }

    /***************************************************************************/
    /**
	  * This function loads the data of another GNumGaussAdaptorT<parameter_type, adaption_fp_type>, camouflaged as a GAdaptorT.
	  * We assume that the values given to us by the other object are correct and do no error checks.
	  *
	  * @param cp A copy of another GNumGaussAdaptorT<parameter_type, adaption_fp_type>, camouflaged as a GAdaptorT
	  */
    void load_(const GAdaptorT<parameter_type, adaption_fp_type> *cp) override {
        // Check that we are dealing with a GNumGaussAdaptorT<parameter_type, adaption_fp_type> reference independent of this object and convert the pointer
        const GNumGaussAdaptorT<parameter_type, adaption_fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<parameter_type, adaption_fp_type>, GNumGaussAdaptorT<parameter_type, adaption_fp_type>>(
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
    friend void Gem::Common::compare_base_t<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>(
        GNumGaussAdaptorT<parameter_type, adaption_fp_type> const &,
        GNumGaussAdaptorT<parameter_type, adaption_fp_type> const &,
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

        // Check that we are dealing with a GNumGaussAdaptorT<parameter_type, adaption_fp_type> reference independent of this object and convert the pointer
        const GNumGaussAdaptorT<parameter_type, adaption_fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<parameter_type, adaption_fp_type>, GNumGaussAdaptorT<parameter_type, adaption_fp_type>>(
                cp,
                this
            );

        GToken token("GNumGaussAdaptorT<parameter_type, adaption_fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GAdaptorT<parameter_type, adaption_fp_type>>(*this, *p_load, token);

        // ... and then the local data
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * This adaptor allows the evolutionary adaption of sigma_. This allows the
	  * algorithm to adapt to changing geometries of the quality surface.
	  *
	  */
    void customAdaptAdaption(
        [[maybe_unused]] const parameter_type & val
        ,
        Gem::Hap::GRandomBase &gr
    ) override {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        // The following random distribution slightly favours values < 1. Selection pressure
        // will keep the values higher if needed
        sigma_ *= std::exp(
            GAdaptorT<parameter_type, adaption_fp_type>::normal_distribution_(
                gr,
                typename std::normal_distribution<adaption_fp_type>::param_type(0., std::abs(sigma_sigma_))
            )
        );

        // make sure sigma_ doesn't get out of range
        Gem::Common::enforceRangeConstraint<adaption_fp_type>(
            sigma_,
            min_sigma_,
            max_sigma_,
            "GNumGaussAdaptorT<>::customAdaptAdaption()",
            false /* silent */
        );
    }

    /***************************************************************************/
    /**
	  * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
	  * adaptions are defined in the derived classes.
	  */
    void customAdaptions(parameter_type &, const parameter_type &, Gem::Hap::GRandomBase &) override = 0;

    /***************************************************************************/
    /**
	  * Allows to randomly initialize parameter members
	  */
    bool randomInit(Gem::Hap::GRandomBase &gr) override {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        sigma_ = GAdaptorT<parameter_type, adaption_fp_type>::uniform_real_distribution_(
            gr,
            typename std::uniform_real_distribution<adaption_fp_type>::param_type(min_sigma_, max_sigma_)
        );

        return true;
    }

    /***************************************************************************/
    /**
	  * Adds a given property value to the vector or returns false, if the property
	  * was not found.
	  */
    bool
    customQueryProperty(const std::string &property, std::vector<std::any> &data) const override {
        if(property == "sigma") {
            data.push_back(std::any(sigma_));
        }
        else {
            return false;
        }

        return true;
    }

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
        if(GAdaptorT<parameter_type>::modify_GUnitTests_()) {
            result = true;
        }

        // A relatively harmless change
        sigma_sigma_ *= adaption_fp_type(1.1);
        result = true;

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
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
        GAdaptorT<parameter_type>::specificTestsNoFailureExpected_GUnitTests_();

        // Get a random number generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Test setting and retrieval of the sigma range
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            for(adaption_fp_type dlower = adaption_fp_type(0.); dlower < adaption_fp_type(0.8); dlower += adaption_fp_type(0.1)) {
                adaption_fp_type dupper = std::min(adaption_fp_type(2.) * dlower, adaption_fp_type(1.));
                if(0 == dupper) {
                    dupper = 1.;
                }

                CHECK_NOTHROW(p_test->setSigmaRange(dlower, dupper));
                typename std::tuple<adaption_fp_type, adaption_fp_type> range;
                CHECK_NOTHROW(range = p_test->getSigmaRange());

                using namespace boost;

                if(dlower ==
                   0.) { // Account for the fact that a lower boundary of 0. will be silently changed
                    INFO(
                        std::get<0>(range) << " / " << Gem::Common::narrow<adaption_fp_type>(DEFAULTMINSIGMA)
                    );
                    CHECK(std::get<0>(range) == Gem::Common::narrow<adaption_fp_type>(DEFAULTMINSIGMA));
                    INFO(std::get<1>(range) << " / " << Gem::Common::narrow<adaption_fp_type>(1.));
                    CHECK(std::get<1>(range) == Gem::Common::narrow<adaption_fp_type>(1.));
                }
                else {
                    CHECK(std::get<0>(range) == dlower);
                }
            }
        }

        //------------------------------------------------------------------------------

        { // Test that setting a sigma of 0. will result in a sigma with value DEFAULTMINSIGMA
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_NOTHROW(p_test->setSigmaRange(adaption_fp_type(0.), adaption_fp_type(1.)));
            CHECK_NOTHROW(p_test->setSigma(adaption_fp_type(DEFAULTMINSIGMA)));
            CHECK(p_test->getSigma() == adaption_fp_type(DEFAULTMINSIGMA));
        }

        //------------------------------------------------------------------------------

        { // Tests setting and retrieval of the sigma parameter
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_NOTHROW(p_test->setSigmaRange(adaption_fp_type(0.), adaption_fp_type(1.)));

            for(adaption_fp_type d = adaption_fp_type(0.1); d < adaption_fp_type(0.9); d += adaption_fp_type(0.1)) {
                CHECK_NOTHROW(p_test->setSigma(d));
                CHECK(p_test->getSigma() == d);
            }
        }

        //------------------------------------------------------------------------------

        { // Test setting and retrieval of the sigma adaption rate
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            for(adaption_fp_type d = adaption_fp_type(0.1); d < adaption_fp_type(0.9); d += adaption_fp_type(0.1)) {
                CHECK_NOTHROW(p_test->setSigmaAdaptionRate(d));
                CHECK(p_test->getSigmaAdaptionRate() == d);
            }
        }

        //------------------------------------------------------------------------------

        { // Check that simultaneous setting of all "sigma-values" has an effect
            using namespace boost;

            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_NOTHROW(p_test->setAll(adaption_fp_type(0.5), adaption_fp_type(0.8), adaption_fp_type(0.), adaption_fp_type(1.)));
            CHECK(p_test->getSigma() == adaption_fp_type(0.5));
            CHECK(p_test->getSigmaAdaptionRate() == adaption_fp_type(0.8));
            std::tuple<adaption_fp_type, adaption_fp_type> range;
            CHECK_NOTHROW(range = p_test->getSigmaRange());
            CHECK(std::get<0>(range) == adaption_fp_type(DEFAULTMINSIGMA));
            CHECK(std::get<1>(range) == adaption_fp_type(1.));
        }

        //------------------------------------------------------------------------------

        { // Test sigma adaption
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            // true: Adaptions should happen always, independent of the adaption probability
            CHECK_NOTHROW(p_test->setAdaptionMode(adaptionMode::ALWAYS));

            const adaption_fp_type min_sigma = adaption_fp_type(0.0001);
            const adaption_fp_type max_sigma = adaption_fp_type(1.);
            const adaption_fp_type sigma_start = adaption_fp_type(1.);
            const adaption_fp_type sigma_sigma = adaption_fp_type(0.001);

            CHECK_NOTHROW(p_test->setSigmaRange(min_sigma, max_sigma));
            CHECK_NOTHROW(p_test->setSigma(sigma_start));
            CHECK_NOTHROW(p_test->setSigmaAdaptionRate(sigma_sigma));

            adaption_fp_type old_sigma = p_test->getSigma();
            adaption_fp_type new_sigma = 0.;
            CHECK(old_sigma == sigma_start);

            std::size_t n_tests = 10000;
            std::size_t max_counter = 0;
            std::size_t max_max_counter = 500;
            for(std::size_t i = 0; i < n_tests; i++) {
                CHECK_NOTHROW(p_test->adaptAdaption(parameter_type(1), gr));
                new_sigma = p_test->getSigma();
                CHECK((new_sigma >= min_sigma && new_sigma <= max_sigma));

                if(new_sigma != min_sigma && new_sigma != max_sigma) {
                    INFO(
                        "\n"
                        << "old_sigma = " << old_sigma << "\n"
                        << "new_sigma = " << new_sigma << "\n"
                        << "iteration = " << i << "\n"
                    );
                    CHECK(new_sigma != old_sigma);
                    old_sigma = new_sigma;
                }
                else {
                    // We want to know how often we have exceeded the boundaries
                    max_counter++;
                }
            }

            INFO(
                "\n"
                << "max_counter = " << max_counter << "\n"
                << "max_max_counter = " << max_max_counter << "\n"
            );
            CHECK(max_counter < max_max_counter);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GNumGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests",
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
        GAdaptorT<parameter_type>::specificTestsFailuresExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        { // Test that setting a minimal sigma < 0. throws
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_THROWS_AS((p_test->setSigmaRange(adaption_fp_type(-1.), adaption_fp_type(2.))), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test that setting a minimal sigma > the maximum sigma throws
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_THROWS_AS((p_test->setSigmaRange(adaption_fp_type(2.), adaption_fp_type(1.))), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test that setting a negative sigma throws
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_THROWS_AS((p_test->setSigma(adaption_fp_type(-1.))), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test that setting a sigma below the allowed range throws
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_NOTHROW(p_test->setSigmaRange(adaption_fp_type(0.5), adaption_fp_type(1.)));
            CHECK_THROWS_AS((p_test->setSigma(adaption_fp_type(0.1))), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test that setting a sigma above the allowed range throws
            std::shared_ptr<GNumGaussAdaptorT<parameter_type, adaption_fp_type>> p_test =
                this->template clone<GNumGaussAdaptorT<parameter_type, adaption_fp_type>>();

            CHECK_NOTHROW(p_test->setSigmaRange(adaption_fp_type(0.5), adaption_fp_type(1.)));
            CHECK_THROWS_AS((p_test->setSigma(adaption_fp_type(3.))), geneva_exception);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GNumGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    // "protected" for performance reasons, so we do not have to go through access functions
    /***************************************************************************/
    adaption_fp_type sigma_ = adaption_fp_type(DEFAULTSIGMA); ///< The width of the gaussian used to adapt values
    adaption_fp_type sigma_reset_ =
        sigma_; ///< The value to which sigma_ will be reset if "updateOnStall()" is called
    adaption_fp_type sigma_sigma_ = adaption_fp_type(DEFAULTSIGMASIGMA); ///< affects sigma_ adaption
    adaption_fp_type min_sigma_ = adaption_fp_type(DEFAULTMINSIGMA);     ///< minimum allowed value for sigma_
    adaption_fp_type max_sigma_ = adaption_fp_type(DEFAULTMAXSIGMA);     ///< maximum allowed value for sigma_

private:
    /***************************************************************************/
    /**
     * @brief Retrieves the id of the adaptor. */
    Gem::Geneva::adaptorId getAdaptorId_() const override = 0;

    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GNumGaussAdaptorT");
    }

    /***************************************************************************/
    /**
	  * This function creates a deep copy of this object. Purely virtual so this class cannot
	  * be instantiated directly.
	  *
	  * @return A deep copy of this object
	  */
    GAdaptorT<parameter_type, adaption_fp_type> *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost::serialization {
template <typename parameter_type, typename adaption_fp_type>
struct is_abstract<Gem::Geneva::Parameters::GNumGaussAdaptorT<parameter_type, adaption_fp_type>> : public boost::true_type {};
template <typename parameter_type, typename adaption_fp_type>
struct is_abstract<const Gem::Geneva::Parameters::GNumGaussAdaptorT<parameter_type, adaption_fp_type>>
  : public boost::true_type {};
} /* namespace boost::serialization */
