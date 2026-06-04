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
#include <tuple>
#include <type_traits>

// Boost headers go here

// Geneva headers go here

#include "common/GCommonInterfaceT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomT.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The following applies mostly to evolutionary algorithms.
 *
 * In Geneva, two mechanisms exist that let the user specify the
 * type of adaption he wants to have executed on collections of
 * items (basic types or any other types).  The most basic
 * possibility is for the user to overload the GParameterSet::customAdaptions()
 * function and manually specify the types of adaptions (s)he
 * wants. This allows great flexibility, but is not very practicable
 * for standard adaptions.
 *
 * Classes derived from GParameterBaseWithAdaptorsT<T> can additionally store
 * "adaptors". These are templatized function objects that can act
 * on the items of a collection of user-defined types. Predefined
 * adaptors exist for standard types (with the most prominent
 * examples being bits and double values).
 *
 * The GAdaptorT class mostly acts as an interface for these
 * adaptors, but also implements some functionality of its own. E.g., it is possible
 * to specify a function that shall be called every adaption_threshold_ calls of the
 * adapt() function. It is also possible to set an adaption probability, so only a certain
 * percentage of adaptions is actually performed at run-time.
 *
 * In order to use this class, the user must derive a class from
 * GAdaptorT<T, fp_type> and specify the type of adaption he wishes to
 * have applied to items, by overloading of
 * GAdaptorT<T, fp_type>::customAdaptions(T&) .  T will often be
 * represented by a basic value (double, long, bool, ...). Where
 * this is not the case, the adaptor will only be able to access
 * public functions of T, unless T declares the adaptor as a friend.
 *
 * As part of the GObject-decomposition effort, the adaptor hierarchy is its own
 * category root: it derives directly from
 * Gem::Common::GCommonInterfaceT<GAdaptorT<T, fp_type>> instead of from GObject, so
 * a GAdaptorT pointer is an unrelated type to a GObject pointer. The common
 * infrastructure (clone/load/compare/name/IO/serialize) is supplied by the CRTP
 * base, instantiated for this (template) root.
 */
template <typename T, typename fp_type = double>
class GAdaptorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<GAdaptorT<T, fp_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        // This is the CRTP category root. Its CRTP base
        // (Gem::Common::GCommonInterfaceT<GAdaptorT<T, fp_type>>) carries no state
        // and is therefore not serialized as a base_object -- mirroring GObject,
        // whose serialize() is likewise empty. The polymorphic base_object chain
        // bottoms out here; only our own data is serialized.
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * Allows external callers to find out about the type stored in this object
	  */
    using adaption_type = T;

    /***************************************************************************/
    /**
     * This constructor allows to set the probability with which an adaption is indeed
     * performed.
     *
     * @param ad_prob The likelihood for a an adaption to be actually carried out
     */
    explicit GAdaptorT(const fp_type &ad_prob)
      : ad_prob_(ad_prob) {
        // Do some error checking
        // Check that adProb_ is in the allowed range. Adapt, if necessary
        if(not Gem::Common::checkRangeCompliance<fp_type>(
               ad_prob_,
               min_ad_prob_,
               max_ad_prob_,
               "GAdaptorT<>::GAdaptorT(" + Gem::Common::to_string(ad_prob) + ")"
           )) {
            glogger << "In GAdaptorT<T, fp_type>::GadaptorT(const fp_type& ad_prob):" << '\n'
                    << "ad_prob value " << ad_prob_ << " is outside of allowed value range ["
                    << min_ad_prob_ << ", " << max_ad_prob_ << "]" << '\n'
                    << "The value will be adapted to fit this range." << '\n'
                    << GWARNING;

            Gem::Common::enforceRangeConstraint<fp_type>(
                ad_prob_,
                min_ad_prob_,
                max_ad_prob_,
                "GAdaptorT<>::GAdaptorT(" + Gem::Common::to_string(ad_prob) + " / 1)"
            );
            Gem::Common::enforceRangeConstraint<fp_type>(
                ad_prob_reset_,
                min_ad_prob_,
                max_ad_prob_,
                "GAdaptorT<>::GAdaptorT(" + Gem::Common::to_string(ad_prob) + " / 2)"
            );
        }
    }

    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators -- rule of five

    GAdaptorT() = default;
    GAdaptorT(GAdaptorT<T, fp_type> const &cp) = default;
    GAdaptorT(GAdaptorT<T, fp_type> &&cp) = default;

    ~GAdaptorT() override = default;

    GAdaptorT<T, fp_type> &operator=(GAdaptorT<T, fp_type> const &) = default;
    GAdaptorT<T, fp_type> &operator=(GAdaptorT<T, fp_type> &&) = default;

    /***************************************************************************/
    /**
	  * Retrieves the id of the adaptor.
	  *
	  * @return The id of the adaptor
	  */
    Gem::Geneva::adaptorId getAdaptorId() const {
        return getAdaptorId_();
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GBooleanAdaptor
	  * Tested in GInt32FlipAdaptor
	  * Tested in GInt32GaussAdaptor
	  * Tested in GDoubleGaussAdaptor
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Sets the adaption probability to a given value. This function will throw
	  * if the probability is not in the allowed range.
	  *
	  * @param ad_prob The new value of the probability of adaptions taking place
	  */
    void setAdaptionProbability(const fp_type &ad_prob) {
        // Check the supplied probability value
        if(ad_prob < fp_type(0.) || ad_prob > fp_type(1.)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setAdaptionProbability(const fp_type&):" << '\n'
                << "Bad probability value given: " << ad_prob << '\n'
            );
        }

        // Check that the new value fits in the allowed value range
        if(not Gem::Common::checkRangeCompliance<fp_type>(
               ad_prob,
               min_ad_prob_,
               max_ad_prob_,
               "GAdaptorT<>::setAdaptionProbability(" + Gem::Common::to_string(ad_prob) + ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setAdaptionProbability(const fp_type& ad_prob):"
                << '\n'
                << "ad_prob value " << ad_prob << " is outside of allowed value range ["
                << min_ad_prob_ << ", " << max_ad_prob_ << "]" << '\n'
                << "Set new boundaries first before setting a new \"ad_prob\" value" << '\n'
            );
        }

        ad_prob_ = ad_prob;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of valid probabilities is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * Checks for setting of invalid probabilities is tested in GAdaptorT<T, fp_type>::specificTestsFailuresExpected_GUnitTests()
	  * The effects on the probability of adaptions actually taking place are tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the current value of the adaption probability
	  *
	  * @return The current value of the adaption probability
	  */
    fp_type getAdaptionProbability() const {
        return ad_prob_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of probabilities is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Sets the "reset" adaption probability to a given value. This is the probability
	  * to which adProb_ will be reset if updateOnStall() is called. This function will
	  * throw if the probability is not in the allowed range.
	  *
	  * @param ad_prob_reset The new value of the "reset" probability
	  */
    void setResetAdaptionProbability(const fp_type &ad_prob_reset) {
        // Check the supplied probability value
        if(not Gem::Common::checkRangeCompliance<fp_type>(
               ad_prob_reset,
               min_ad_prob_,
               max_ad_prob_,
               "GAdaptorT<>::setResetAdaptionProbability(" + Gem::Common::to_string(ad_prob_reset) +
                   ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setResetAdaptionProbability(const fp_type&):" << '\n'
                << "ad_prob_reset value " << ad_prob_reset << " is outside of allowed value range ["
                << min_ad_prob_ << ", " << max_ad_prob_ << "]" << '\n'
                << "Set new boundaries first before setting a new \"ad_prob_reset\" value" << '\n'
            );
        }

        ad_prob_reset_ = ad_prob_reset;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current value of the "reset" adaption probability
	  *
	  * @return The current value of the "reset" adaption probability
	  */
    fp_type getResetAdaptionProbability() const {
        return ad_prob_reset_;
    }

    /***************************************************************************/
    /**
	  * Sets the probability for the adaption of adaption parameters
	  *
	  * @param probability The new value of the probability of adaptions of adaption parameters
	  */
    void setAdaptAdaptionProbability(const fp_type &probability) {
        // Check the supplied probability value
        if(not Gem::Common::checkRangeCompliance<fp_type>(
               probability,
               0.,
               1.,
               "GAdaptorT<>::setAdaptAdaptionProbability(" + Gem::Common::to_string(probability) +
                   ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setAdaptAdaptionProbability(const fp_type&) :"
                << '\n'
                << "Probability " << probability << " not in allowed range [0.,1.]" << '\n'
            );
        }

        adapt_adaption_probability_ = probability;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of valid probabilities is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * Checks for setting of invalid probabilities is tested in GAdaptorT<T, fp_type>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the current value of the adapt_adaption_probability_ variable
	  *
	  * @return The current value of the adapt_adaption_probability_ variable
	  */
    fp_type getAdaptAdaptionProbability() const {
        return adapt_adaption_probability_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of probabilities is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to specify an adaption factor for adProb_ (or 0, if you do not
	  * want this feature)
	  */
    void setAdaptAdProb(fp_type adapt_ad_prob) {
        // adaptAdProb_ is used as the standard deviation of a normal
        // distribution (see customAdaptions()); 0 disables the feature.
        // A negative value is therefore invalid and must be rejected in
        // every build type, not only under DEBUG. There is deliberately no
        // upper bound: a Gaussian sigma may legitimately exceed 1.
        if(adapt_ad_prob < fp_type(0.)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<>::setAdaptAdProb(): Error!" << '\n'
                << "adapt_ad_prob < 0: " << adapt_ad_prob << '\n'
            );
        }

        adapt_ad_prob_ = adapt_ad_prob;
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the rate of evolutionary adaption of adProb_
	  */
    fp_type getAdaptAdProb() const {
        return adapt_ad_prob_;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current value of the adaption_counter_ variable.
	  *
	  * @return The value of the adaption_counter_ variable
	  */
    std::uint32_t getAdaptionCounter() const {
        return adaption_counter_;
    }

    /* ----------------------------------------------------------------------------------
	  * It is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests() that the
	  * adaption counter does not exceed the set adaption threshold
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Sets the value of adaption_threshold_. If set to 0, no adaption of the optimization
	  * parameters will take place
	  *
	  * @param adaption_threshold The value that should be assigned to the adaption_counter_ variable
	  */
    void setAdaptionThreshold(const std::uint32_t &adaption_threshold) {
        adaption_threshold_ = adaption_threshold;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of adaption thresholds is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the value of the adaption_threshold_ variable.
	  *
	  * @return The value of the adaption_threshold_ variable
	  */
    std::uint32_t getAdaptionThreshold() const {
        return adaption_threshold_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of adaption threshold is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to specify whether adaptions should happen always, never, or with a given
	  * probability. The function is declared virtual so adaptors requiring adaptions to
	  * happen always or never can prevent resetting of the adaption_mode_ variable.
	  *
	  * @param am The desired mode (always/never/with a given probability)
	  */
    virtual void setAdaptionMode(adaptionMode am) {
        adaption_mode_ = am;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of the adaption mode is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * The effect of setting the adaption mode is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Returns the current value of the adaption_mode_ variable
	  *
	  * @return The current value of the adaption_mode_ variable
	  */
    adaptionMode getAdaptionMode() const {
        return adaption_mode_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of the adaption mode is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to set the allowed range for adaption probability variation.
	  * NOTE that this function will silently adapt the values of adProb_ and
	  * ad_prob_reset_, if they fall outside of the new range.
	  */
    void setAdProbRange(fp_type min_ad_prob, fp_type max_ad_prob) {
#ifdef DEBUG
        if(min_ad_prob < 0.) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setAdProbRange(): Error!" << '\n'
                << "min_ad_prob < 0: " << min_ad_prob << '\n'
            );
        }

        if(max_ad_prob > 1.) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setAdProbRange(): Error!" << '\n'
                << "max_ad_prob > 1: " << max_ad_prob << '\n'
            );
        }

        if(min_ad_prob > max_ad_prob) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<T, fp_type>::setAdProbRange(): Error!" << '\n'
                << "Invalid min_ad_prob and/or max_ad_prob: " << min_ad_prob << " / " << max_ad_prob
                << '\n'
            );
        }
#endif /* DEBUG */

        // Store the new values
        min_ad_prob_ = min_ad_prob;
        if(min_ad_prob_ < DEFMINADPROB) {
            min_ad_prob_ = DEFMINADPROB;
        }
        max_ad_prob_ = max_ad_prob;

        // Make sure adProb_ and ad_prob_reset_ fit the new allowed range
        Gem::Common::enforceRangeConstraint<fp_type>(
            ad_prob_,
            min_ad_prob_,
            max_ad_prob_,
            "GAdaptorT<>::setAdProbRange() / 1"
        );
        Gem::Common::enforceRangeConstraint<fp_type>(
            ad_prob_reset_,
            min_ad_prob_,
            max_ad_prob_,
            "GAdaptorT<>::setAdProbRange() / 2"
        );
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the allowed range for adProb_ variation
	  */
    auto getAdProbRange() const {
        return std::tuple<fp_type, fp_type>{min_ad_prob_, max_ad_prob_};
    }

    /***************************************************************************/
    /**
	  * Common interface for all adaptors to the adaption functionality. The user
	  * specifies the actual actions in the customAdaptions() function.
	  *
	  * @param val The value that needs to be adapted
	  * @param range A typical value range for type T
	  * @param gr A reference to a random number generator
	  * @return The number of adaptions that were carried out
	  */
    std::size_t adapt(T &val, const T &range, Gem::Hap::GRandomBase &gr) {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        bool adapted = false;

        // Update the adaption probability, if requested by the user
        if(adapt_ad_prob_ > fp_type(0.)) {
            ad_prob_ *= std::exp(normal_distribution_(
                gr,
                typename std::normal_distribution<fp_type>::param_type(0., adapt_ad_prob_)
            ));
            Gem::Common::enforceRangeConstraint<fp_type>(
                ad_prob_,
                min_ad_prob_,
                max_ad_prob_,
                "GAdaptorT<>::adapt() / 1"
            );
        }

        if(adaptionMode::WITHPROBABILITY ==
           adaption_mode_) { // The most likely case is indeterminate (means: "sometimes" here)
            if(weighted_bool_(
                   gr,
                   std::bernoulli_distribution::param_type(std::abs(ad_prob_))
               )) { // Likelihood of adProb_ for the adaption
                adaptAdaption(range, gr);
                customAdaptions(val, range, gr);
                adapted = true;
            }
        }
        else if(adaptionMode::ALWAYS == adaption_mode_) { // always adapt
            adaptAdaption(range, gr);
            customAdaptions(val, range, gr);
            adapted = true;
        }

        // No need to test for "adaption_mode_ == adaptionMode::NEVER" as no action is needed in this case

        if(adapted) {
            return static_cast<std::size_t>(1);
        }
                    return static_cast<std::size_t>(0);
       
    }

    /* ----------------------------------------------------------------------------------
	  * Adaption is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Common interface for all adaptors to the adaption functionality. The user
	  * specifies the actual actions in the customAdaptions() function. This function
	  * deals with entire parameter vectors. The philosophy behind these vectors is
	  * that they represent a common logical entity and should thus be mutated together,
	  * using a single adaptor. However, it is not clear whether adaptions of mutation
	  * parameters (such as adaption of the sigma value) should happen whenever
	  * customAdaptions() is called (which would be equivalent to individual parameter
	  * objects) or only once, before customAdaptions is applied to each position in
	  * turn. As adaption e.g. of the sigma value slightly favors changes towards smaller
	  * values, we incur a small bias in the first case, where mutations of parameters
	  * at the end of the array might be smaller than at the beginning. In the second case,
	  * metaAdaption might not be called often enough to adapt the mutation process
	  * to different geometries of the quality surface. Our tests show that the latter
	  * might be more severe, so we have implemented repeated adaption of mutation parameters
	  * in this function.
	  *
	  * @return The number of adaptions that were carried out
	  */
    std::size_t adapt(std::vector<T> &val_vec, const T &range, Gem::Hap::GRandomBase &gr) {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        std::size_t n_adapted = 0;

        // Update the adaption probability, if requested by the user
        if(adapt_ad_prob_ > fp_type(0.)) {
            ad_prob_ *= std::exp(normal_distribution_(
                gr,
                typename std::normal_distribution<fp_type>::param_type(0., adapt_ad_prob_)
            ));
            Gem::Common::enforceRangeConstraint<fp_type>(
                ad_prob_,
                min_ad_prob_,
                max_ad_prob_,
                "GAdaptorT<>::adapt() / 2"
            );
        }

        if(adaptionMode::WITHPROBABILITY == adaption_mode_) { // The most likely case
            for(auto &val : val_vec) {
                // A likelihood of adProb_ for adaption
                if(weighted_bool_(
                       gr,
                       std::bernoulli_distribution::param_type(std::abs(ad_prob_))
                   )) {
                    adaptAdaption(range, gr);
                    customAdaptions(val, range, gr);

                    n_adapted += 1;
                }
            }
        }
        else if(adaptionMode::ALWAYS == adaption_mode_) { // always adapt
            for(auto &val : val_vec) {
                adaptAdaption(range, gr);
                customAdaptions(val, range, gr);

                n_adapted += 1;
            }
        }

        // No need to test for "adaption_mode_ == adaptionMode::NEVER" as no action is needed in this case

        return n_adapted;
    }

    /* ----------------------------------------------------------------------------------
	  * Adaption is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Triggers updates when the optimization process has stalled. This function
	  * resets the adaption probability to its original value
	  *
	  * @param n_stalls The number of consecutive stalls up to this point
	  * @return A boolean indicating whether updates were performed
	  */
    virtual bool updateOnStall(
        const std::size_t &n_stalls,
        [[maybe_unused]] const T & range
    ) {
#ifdef DEBUG
        if(0 == n_stalls) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorT<>::updateOnStall(" << n_stalls << "): Error!" << '\n'
                << "Function called for zero n_stalls" << '\n'
            );
        }
#endif

        // Reset the adaption probability
        if(ad_prob_ == ad_prob_reset_) {
            return false;
        }
                    ad_prob_ = ad_prob_reset_;
            return true;
       
    }

    /***************************************************************************/
    /**
	  * Allows derived classes to print diagnostic messages
	  *
	  * @return A diagnostic message
	  */
    virtual std::string printDiagnostics() const {
        return {};
    }

    /***************************************************************************/
    /**
	  * Allows to query specific properties of a given adaptor. Note that the
	  * adaptor must have implemented a "response" for the query, as the function
	  * will otherwise throw. This function is meant for debugging and profiling.
	  * It might e.g. be useful if you want to know why an EA-based optimization has
	  * stalled. Note that the permanent use of this function, e.g. from a permanently
	  * enabled "pluggable optimization monitor, will be inefficient due to the
	  * constant need to compare strings.
	  *
	  * @param adaptor_name The name of the adaptor to be queried
	  * @param property The property for which information is sought
	  * @param data A vector, to which the properties should be added
	  */
    void queryPropertyFrom(
        const std::string &adaptor_name,
        const std::string &property,
        std::vector<std::any> &data
    ) const {
        // Do nothing, if this query is not for us
        if(adaptor_name != this->name()) {
            return;
        }
                                // O.k., this query is for us!
            if(property == "ad_prob") { // The only property that can be queried for this class
                data.push_back(std::any(ad_prob_));
            }
            else { // Ask derived classes
                if(not this->customQueryProperty(property, data)) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GAdaptorT<T, fp_type>::queryPropertyFrom(): Error!" << '\n'
                        << "Function was called for unimplemented property " << property << '\n'
                        << "on adaptor " << adaptor_name << '\n'
                    );
                }
            }
       
    }

    /***************************************************************************/
    /** @brief Allows derived classes to randomly initialize parameter members */
    virtual bool randomInit(Gem::Hap::GRandomBase &) = 0;

protected:
    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("adaption_counter_", adaption_counter_),
            Gem::Common::make_member("adaption_threshold_", adaption_threshold_),
            Gem::Common::make_member("ad_prob_", ad_prob_),
            Gem::Common::make_member("adapt_ad_prob_", adapt_ad_prob_),
            Gem::Common::make_member("min_ad_prob_", min_ad_prob_),
            Gem::Common::make_member("max_ad_prob_", max_ad_prob_),
            Gem::Common::make_member("adaption_mode_", adaption_mode_),
            Gem::Common::make_member("adapt_adaption_probability_", adapt_adaption_probability_),
            Gem::Common::make_member("ad_prob_reset_", ad_prob_reset_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("adaption_counter_", adaption_counter_),
            Gem::Common::make_member("adaption_threshold_", adaption_threshold_),
            Gem::Common::make_member("ad_prob_", ad_prob_),
            Gem::Common::make_member("adapt_ad_prob_", adapt_ad_prob_),
            Gem::Common::make_member("min_ad_prob_", min_ad_prob_),
            Gem::Common::make_member("max_ad_prob_", max_ad_prob_),
            Gem::Common::make_member("adaption_mode_", adaption_mode_),
            Gem::Common::make_member("adapt_adaption_probability_", adapt_adaption_probability_),
            Gem::Common::make_member("ad_prob_reset_", ad_prob_reset_)
        );
    }

    /***************************************************************************/
    /**
	  * Loads the contents of another GAdaptorT<T, fp_type>. The function
	  * is similar to a copy constructor (but with a pointer as
	  * argument). As this function might be called in an environment
	  * where we do not know the exact type of the class, the
	  * GAdaptorT<T, fp_type> is camouflaged as a GAdaptorT . This implies the
	  * need for dynamic conversion.
	  *
	  * @param cp A pointer to another GAdaptorT<T, fp_type>, camouflaged as a GAdaptorT<T, fp_type>
	  */
    void load_(const GAdaptorT<T, fp_type> *cp) override {
        // Check that we are dealing with a GAdaptorT<T, fp_type> reference independent of this object and convert the pointer
        const GAdaptorT<T, fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<T, fp_type>, GAdaptorT<T, fp_type>>(cp, this);

        // This is the category root; there is no GObject parent class to load.

        // Our own data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GAdaptorT<T, fp_type>>(
        GAdaptorT<T, fp_type> const &,
        GAdaptorT<T, fp_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GAdaptorT<T, fp_type> object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GAdaptorT<T, fp_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GAdaptorT<T, fp_type> reference independent of this object and convert the pointer
        const GAdaptorT<T, fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GAdaptorT<T, fp_type>, GAdaptorT<T, fp_type>>(cp, this);

        GToken token("GAdaptorT<T, fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<Gem::Common::GCommonInterfaceT<GAdaptorT<T, fp_type>>>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * This function helps to adapt the adaption parameters, if certain conditions are met.
	  * Adaption is triggered by the parameter object.
	  */
    void adaptAdaption(const T &range, Gem::Hap::GRandomBase &gr) {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        // The adaption parameters are modified every adaption_threshold_ number of adaptions.
        if(adaption_threshold_ > 0) {
            if(++adaption_counter_ >= adaption_threshold_) {
                adaption_counter_ = 0;
                customAdaptAdaption(range, gr);
            }
        }
        else if(adapt_adaption_probability_) { // Do the same with probability settings
            // Likelihood of adapt_adaption_probability_ for the adaption
            if(weighted_bool_(
                   gr,
                   std::bernoulli_distribution::param_type(std::abs(adapt_adaption_probability_))
               )) {
                customAdaptAdaption(range, gr);
            }
        }
    }

    /***************************************************************************/
    /**
	  * Adds a given property value to the vector or returns false, if the property
	  * was not found. We do not check anymore if this query was for as, as this was
	  * already done by  queryPropertyFrom(). Thus function needs to be re-implemented
	  * by derived classes wishing to emit information. If there is no re-implementation,
	  * this function will simply return false.
	  */
    virtual bool customQueryProperty(
        [[maybe_unused]] const std::string & property
        ,
        std::vector<std::any> &data
    ) const {
        return false;
    }

    /***************************************************************************/
    /**
	  *  This function is re-implemented by derived classes, if they wish to
	  *  implement special behavior for a new adaption run. E.g., an internal
	  *  variable could be set to a new value.
	  *
	  */
    virtual void customAdaptAdaption(const T &, Gem::Hap::GRandomBase &gr) { /* nothing */
    }

    /***************************************************************************/

    /** @brief Adaption of values as specified by the user */
    virtual void customAdaptions(T &, const T &, Gem::Hap::GRandomBase &) = 0;

    /** @brief Creates a deep copy of this object */
    GAdaptorT<T, fp_type> *clone_() const override = 0;

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        // This is the category root; there is no modifiable GObject parent class.
        bool result = false;

        // Modify some local parameters
        if(this->getAdaptionProbability() <= 0.5) {
            this->setAdaptionProbability(0.75);
        }
        else {
            this->setAdaptionProbability(0.25);
        }

        result = true;

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // This is the category root; there is no GObject parent class to delegate to.

        // Retrieve a random number generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::set/getAdaptionProbability()
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // The adaption probability should have been cloned
            INFO(
                "\n"
                << "p_test->getAdaptionProbability() = " << p_test->getAdaptionProbability() << "\n"
                << "this->getAdaptionProbability() = " << this->getAdaptionProbability() << "\n"
            );
            CHECK(p_test->getAdaptionProbability() == this->getAdaptionProbability());

            // Set an appropriate range for the adaption
            p_test->setAdProbRange(0.001, 1.);

            // Set the adaption probability to a sensible value and check the new setting
            fp_type test_ad_prob = fp_type(0.5);
            CHECK_NOTHROW(p_test->setAdaptionProbability(test_ad_prob));
            INFO(
                "\n"
                << "p_test->getAdaptionProbability() = " << p_test->getAdaptionProbability() << "\n"
                << "test_ad_prob = " << test_ad_prob << "\n"
            );
            CHECK(p_test->getAdaptionProbability() == test_ad_prob);
        }

        //------------------------------------------------------------------------------

        { // Check that mutating a value with this class actually work with different likelihoods
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Make sure the adaption probability is taken into account
            p_test->setAdaptionMode(adaptionMode::WITHPROBABILITY);
            // Set an appropriate range for the adaption
            p_test->setAdProbRange(0.001, 1.);

            T test_val = T(0);
            for(fp_type prob = 0.001; prob < 1.; prob += 0.01) {
                // Account for rounding problems
                if(prob > 1.) {
                    prob = 1.;
                }

                p_test->setAdaptionProbability(prob);
                CHECK_NOTHROW(p_test->setAdaptionProbability(prob));
                CHECK_NOTHROW(p_test->adapt(test_val, T(1), gr));
            }
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptionProbability() regarding the effects on the likelihood for adaption of the variable
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Make sure the adaption probability is taken into account
            p_test->setAdaptionMode(adaptionMode::WITHPROBABILITY);
            // Prevent changes to adProb_
            p_test->setAdaptAdProb(0.);

            p_test->setAdProbRange(0., 1.);

            constexpr std::size_t n_tests = 100000;

            for(fp_type prob = 0.1; prob < 1.; prob += 0.1) {
                // Account for rounding problems
                if(prob > 1.) {
                    prob = 1.;
                }

                std::size_t n_changed = 0;

                T test_val = T(0);
                T prev_test_val = test_val;

                // Set the likelihood for adaption to "prob"
                p_test->setAdaptionProbability(prob);

                // Mutating a boolean value a number of times should now result in a certain number of changed values
                for(std::size_t i = 0; i < n_tests; i++) {
                    p_test->adapt(test_val, T(1), gr);
                    if(test_val != prev_test_val) {
                        n_changed++;
                        prev_test_val = test_val;
                    }
                }

                fp_type change_prob = fp_type(n_changed) / fp_type(n_tests);

                INFO(
                    "\n"
                    << "change_prob = " << change_prob << "\n"
                    << "prob = " << prob << "\n"
                    << "with allowed window = [" << 0.8 * prob << " : " << 1.2 * prob << "]" << "\n"
                );
                CHECK((change_prob > 0.8 * prob && change_prob < 1.2 * prob));
            }
        }

        //------------------------------------------------------------------------------

        { // Check setting and retrieval of the adaption mode
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Check setting of the different allowed values
            // false
            CHECK_NOTHROW(p_test->setAdaptionMode(adaptionMode::NEVER));
            INFO(
                "\n"
                << "p_test->getAdaptionMode() = " << p_test->getAdaptionMode() << "\n"
                << "required value            = adaptionMode::NEVER\n"
            );
            CHECK(p_test->getAdaptionMode() == adaptionMode::NEVER);

            // true
            CHECK_NOTHROW(p_test->setAdaptionMode(adaptionMode::ALWAYS));
            INFO(
                "\n"
                << "p_test->getAdaptionMode() = " << p_test->getAdaptionMode() << "\n"
                << "required value            = adaptionMode::ALWAYS\n"
            );
            CHECK(adaptionMode::ALWAYS == p_test->getAdaptionMode());

            // Gem::Common::tribool::Indeterminate
            CHECK_NOTHROW(p_test->setAdaptionMode(adaptionMode::WITHPROBABILITY));
            INFO(
                "\n"
                << "p_test->getAdaptionMode() = " << p_test->getAdaptionMode() << "\n"
                << "required value            = Gem::Common::tribool::Indeterminate\n"
            );
            CHECK(adaptionMode::WITHPROBABILITY == p_test->getAdaptionMode());
        }

        //------------------------------------------------------------------------------

        { // Check the effect of the adaption mode settings
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();
            p_test->setAdaptionProbability(0.5);

            constexpr std::size_t n_tests = 10000;

            // false: There should never be adaptions, independent of the adaption probability
            CHECK_NOTHROW(p_test->setAdaptionMode(adaptionMode::NEVER));
            T current_value = T(0);
            T old_value = current_value;
            for(std::size_t i = 0; i < n_tests; i++) {
                p_test->adapt(current_value, T(1), gr);
                INFO(
                    "\n"
                    << "Values differ, when they shouldn't:"
                    << "current_value = " << current_value << "\n"
                    << "old_value     = " << old_value << "\n"
                    << "iteration    = " << i << "\n"
                );
                CHECK(current_value == old_value);
            }

            // true: Adaptions should happen always, independent of the adaption probability
            CHECK_NOTHROW(p_test->setAdaptionMode(adaptionMode::ALWAYS));
            current_value = T(0);
            old_value = current_value;
            for(std::size_t i = 0; i < n_tests; i++) {
                p_test->adapt(current_value, T(1), gr);
                INFO(
                    "\n"
                    << "Values are identical when they shouldn't be:" << "\n"
                    << "current_value = " << current_value << "\n"
                    << "old_value     = " << old_value << "\n"
                    << "iteration    = " << i << "\n"
                    << (this->printDiagnostics()).c_str()
                );
                CHECK(current_value != old_value);
                old_value = current_value;
            }

            // Gem::Common::tribool::Indeterminate: Adaptions should happen with a certain adaption probability
            // No tests -- we already know that this works
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::set/getAdaptAdaptionProbability()
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // The adaption probability should have been cloned
            INFO(
                "\n"
                << "p_test->getAdaptAdaptionProbability() = "
                << p_test->getAdaptAdaptionProbability() << "\n"
                << "this->getAdaptAdaptionProbability() = " << this->getAdaptAdaptionProbability()
                << "\n"
            );
            CHECK(p_test->getAdaptAdaptionProbability() == this->getAdaptAdaptionProbability());

            // Set the adaption probability to a sensible value and check the new setting
            fp_type test_ad_prob = 0.5;
            CHECK_NOTHROW(p_test->setAdaptAdaptionProbability(test_ad_prob));
            INFO(
                "\n"
                << "p_test->getAdaptAdaptionProbability() = "
                << p_test->getAdaptAdaptionProbability() << "\n"
                << "test_ad_prob = " << test_ad_prob << "\n"
            );
            CHECK(p_test->getAdaptAdaptionProbability() == test_ad_prob);
        }

        //------------------------------------------------------------------------------

        { // Test retrieval and setting of the adaption threshold and whether the adaptionCounter behaves nicely
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Make sure we have the right adaption mode
            p_test->setAdaptionMode(adaptionMode::WITHPROBABILITY);
            // Make sure we always adapt
            p_test->setAdaptionProbability(1.0);

            // The value that will be adapted
            T test_val = T(0);
            T old_test_val = T(0);

            // The old adaption counter
            std::uint32_t old_adaption_counter = p_test->getAdaptionCounter();

            // Set the adaption threshold to a specific value
            for(std::uint32_t ad_thr = 10; ad_thr > 0; ad_thr--) {
                // Just make sure our logic is right and we stay in the right window
                CHECK(ad_thr <= 10);

                CHECK_NOTHROW(p_test->setAdaptionThreshold(ad_thr));
                INFO(
                    "\n"
                    << "p_test->getAdaptionThreshold() = " << p_test->getAdaptionThreshold() << "\n"
                    << "ad_thr = " << ad_thr << "\n"
                );
                CHECK(p_test->getAdaptionThreshold() == ad_thr);

                // Check that the adaption counter does not exceed the threshold by
                // adapting a value a number of times > ad_thr
                for(std::uint32_t ad_cnt = 0; ad_cnt < 3 * ad_thr; ad_cnt++) {
                    // Do the actual adaption
                    if(p_test->adapt(test_val, T(1), gr)) {
                        // Check that test_val has indeed been adapted
                        INFO(
                            "\n"
                            << "test_val = " << test_val << "\n"
                            << "old_test_val = " << old_test_val << "\n"
                            << "ad_thr = " << ad_thr << "\n"
                            << "ad_cnt = " << ad_cnt << "\n"
                        );
                        CHECK(test_val != old_test_val);
                        old_test_val = test_val;

                        // Check that the adaption counter has changed at all, as it should
                        // for adaption thresholds > 1
                        if(ad_thr > 1) {
                            INFO(
                                "\n"
                                << "p_test->getAdaptionCounter() = " << p_test->getAdaptionCounter()
                                << "\n"
                                << "old_adaption_counter = " << old_adaption_counter << "\n"
                                << "ad_thr = " << ad_thr << "\n"
                                << "ad_cnt = " << ad_cnt << "\n"
                            );
                            CHECK(p_test->getAdaptionCounter() != old_adaption_counter);
                            old_adaption_counter = p_test->getAdaptionCounter();
                        }

                        // Check that the adaption counter is behaving nicely
                        INFO(
                            "\n"
                            << "p_test->getAdaptionCounter() = " << p_test->getAdaptionCounter()
                            << "\n"
                            << "ad_thr = " << ad_thr << "\n"
                            << "ad_cnt = " << ad_cnt << "\n"
                        );
                        CHECK(p_test->getAdaptionCounter() < ad_thr);
                    }
                }
            }
        }

        //------------------------------------------------------------------------------

        { // Test that customAdaptions() in derived classes changes a test value on every call
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            std::size_t n_tests = 10000;

            T test_val = T(0);
            T old_test_val = T(0);
            for(std::size_t i = 0; i < n_tests; i++) {
                CHECK_NOTHROW(p_test->customAdaptions(test_val, T(1), gr));
                INFO(
                    "\n"
                    << "Found identical values after adaption took place" << "\n"
                    << "test_val = " << test_val << "\n"
                    << "old_test_val = " << old_test_val << "\n"
                    << "iteration = " << i << "\n"
                );
                CHECK(test_val != old_test_val);
                old_test_val = test_val;
            }
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GAdaptorT<>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes.
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // This is the category root; there is no GObject parent class to delegate to.

        // Retrieve a random number generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptionProbability(): Setting a value < 0. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Setting a probability < 0 should throw
            CHECK_THROWS_AS(p_test->setAdaptionProbability(-1.), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptionProbability(): Setting a value > 1. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Setting a probability > 1 should throw
            CHECK_THROWS_AS(p_test->setAdaptionProbability(2.), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptAdaptionProbability(): Setting a value < 0. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Setting a probability < 0 should throw
            CHECK_THROWS_AS(p_test->setAdaptAdaptionProbability(-1.), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptAdaptionProbability(): Setting a value > 1. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->template clone<GAdaptorT<T, fp_type>>();

            // Setting a probability > 1 should throw
            CHECK_THROWS_AS(p_test->setAdaptAdaptionProbability(2.), geneva_exception);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GAdaptorT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    // Protected data

    std::normal_distribution<fp_type> normal_distribution_; ///< Helps with gauss-type mutation
    std::uniform_real_distribution<fp_type>
        uniform_real_distribution_; ///< Access to uniformly distributed floating point random numbers
    std::bernoulli_distribution
        weighted_bool_; ///< Access to boolean random numbers with a given probability structure

private:
    /***************************************************************************/
    /**
     * Retrieves the id of the adaptor. Purely virtual, must be implemented by the
     * actual adaptors.
     *
     * @return The id of the adaptor
     */
    virtual Gem::Geneva::adaptorId getAdaptorId_() const = 0;

    /* ----------------------------------------------------------------------------------
     * Tested in GBooleanAdaptor
     * Tested in GInt32FlipAdaptor
     * Tested in GInt32GaussAdaptor
     * Tested in GDoubleGaussAdaptor
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GAdaptorT");
    }

    /***************************************************************************/

    std::uint32_t adaption_counter_ = 0; ///< A local counter
    std::uint32_t adaption_threshold_ =
        DEFAULTADAPTIONTHRESHOLD; ///< Specifies after how many adaptions the adaption itself should be adapted
    fp_type ad_prob_ = DEFAULTADPROB; ///< internal representation of the adaption probability
    fp_type adapt_ad_prob_ = DEFAUPTADAPTADPROB; ///< The rate, at which adProb_ should be adapted
    fp_type min_ad_prob_ = DEFMINADPROB; ///< The lower allowed value for adProb_ during variation
    fp_type max_ad_prob_ = DEFMAXADPROB; ///< The upper allowed value for adProb_ during variation
    adaptionMode adaption_mode_ = adaptionMode::
        WITHPROBABILITY; ///< Whether to adapt always, never, or with a given probability
    fp_type adapt_adaption_probability_ =
        DEFAULTADAPTADAPTIONPROB; ///< Influences the likelihood for the adaption of the adaption parameters
    fp_type ad_prob_reset_ =
        ad_prob_; ///< The value to which adProb_ will be reset if "updateOnStall()" is called
};

/******************************************************************************/
/** @brief Specialization of GAdaptorT<T,fp_type>::adapt(vec) for the T==bool */
template <>
std::size_t
GAdaptorT<bool, double>::adapt(std::vector<bool> &, const bool &, Gem::Hap::GRandomBase &);

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost::serialization {
template <typename T, typename fp_type>
struct is_abstract<Gem::Geneva::Parameters::GAdaptorT<T, fp_type>> : public boost::true_type {};
template <typename T, typename fp_type>
struct is_abstract<const Gem::Geneva::Parameters::GAdaptorT<T, fp_type>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
