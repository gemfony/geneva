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
#include <any>
#include <type_traits>

// Boost headers go here

// Geneva headers go here

#include "common/GSerializationHelperFunctionsT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomT.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva {

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
 * to specify a function that shall be called every adaptionThreshold_ calls of the
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
 * As a derivative of GObject, this class follows similar rules as
 * the other Geneva classes.
 */
template <typename T, typename fp_type = double>
class GAdaptorT : public GObject {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject) &
            BOOST_SERIALIZATION_NVP(adaptionCounter_) &
            BOOST_SERIALIZATION_NVP(adaptionThreshold_) & BOOST_SERIALIZATION_NVP(adProb_) &
            BOOST_SERIALIZATION_NVP(adaptAdProb_) & BOOST_SERIALIZATION_NVP(minAdProb_) &
            BOOST_SERIALIZATION_NVP(maxAdProb_) & BOOST_SERIALIZATION_NVP(adaptionMode_) &
            BOOST_SERIALIZATION_NVP(adaptAdaptionProbability_) &
            BOOST_SERIALIZATION_NVP(adProb_reset_);
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
      : GObject()
      , adProb_(ad_prob) {
        // Do some error checking
        // Check that adProb_ is in the allowed range. Adapt, if necessary
        if(not Gem::Common::checkRangeCompliance<fp_type>(
               adProb_,
               minAdProb_,
               maxAdProb_,
               "GAdaptorT<>::GAdaptorT(" + Gem::Common::to_string(ad_prob) + ")"
           )) {
            glogger << "In GAdaptorT<T, fp_type>::GadaptorT(const fp_type& ad_prob):" << '\n'
                    << "ad_prob value " << adProb_ << " is outside of allowed value range ["
                    << minAdProb_ << ", " << maxAdProb_ << "]" << '\n'
                    << "The value will be adapted to fit this range." << '\n'
                    << GWARNING;

            Gem::Common::enforceRangeConstraint<fp_type>(
                adProb_,
                minAdProb_,
                maxAdProb_,
                "GAdaptorT<>::GAdaptorT(" + Gem::Common::to_string(ad_prob) + " / 1)"
            );
            Gem::Common::enforceRangeConstraint<fp_type>(
                adProb_reset_,
                minAdProb_,
                maxAdProb_,
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
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setAdaptionProbability(const fp_type&):" << '\n'
                << "Bad probability value given: " << ad_prob << '\n'
            );
        }

        // Check that the new value fits in the allowed value range
        if(not Gem::Common::checkRangeCompliance<fp_type>(
               ad_prob,
               minAdProb_,
               maxAdProb_,
               "GAdaptorT<>::setAdaptionProbability(" + Gem::Common::to_string(ad_prob) + ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setAdaptionProbability(const fp_type& ad_prob):"
                << '\n'
                << "ad_prob value " << ad_prob << " is outside of allowed value range ["
                << minAdProb_ << ", " << maxAdProb_ << "]" << '\n'
                << "Set new boundaries first before setting a new \"ad_prob\" value" << '\n'
            );
        }

        adProb_ = ad_prob;
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
        return adProb_;
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
               minAdProb_,
               maxAdProb_,
               "GAdaptorT<>::setResetAdaptionProbability(" + Gem::Common::to_string(ad_prob_reset) +
                   ")"
           )) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setResetAdaptionProbability(const fp_type&):" << '\n'
                << "ad_prob_reset value " << ad_prob_reset << " is outside of allowed value range ["
                << minAdProb_ << ", " << maxAdProb_ << "]" << '\n'
                << "Set new boundaries first before setting a new \"ad_prob_reset\" value" << '\n'
            );
        }

        adProb_reset_ = ad_prob_reset;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current value of the "reset" adaption probability
	  *
	  * @return The current value of the "reset" adaption probability
	  */
    fp_type getResetAdaptionProbability() const {
        return adProb_reset_;
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
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setAdaptAdaptionProbability(const fp_type&) :"
                << '\n'
                << "Probability " << probability << " not in allowed range [0.,1.]" << '\n'
            );
        }

        adaptAdaptionProbability_ = probability;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of valid probabilities is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * Checks for setting of invalid probabilities is tested in GAdaptorT<T, fp_type>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the current value of the adaptAdaptionProbability_ variable
	  *
	  * @return The current value of the adaptAdaptionProbability_ variable
	  */
    fp_type getAdaptAdaptionProbability() const {
        return adaptAdaptionProbability_;
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
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<>::setAdaptAdProb(): Error!" << '\n'
                << "adapt_ad_prob < 0: " << adapt_ad_prob << '\n'
            );
        }

        adaptAdProb_ = adapt_ad_prob;
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the rate of evolutionary adaption of adProb_
	  */
    fp_type getAdaptAdProb() const {
        return adaptAdProb_;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current value of the adaptionCounter_ variable.
	  *
	  * @return The value of the adaptionCounter_ variable
	  */
    std::uint32_t getAdaptionCounter() const {
        return adaptionCounter_;
    }

    /* ----------------------------------------------------------------------------------
	  * It is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests() that the
	  * adaption counter does not exceed the set adaption threshold
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Sets the value of adaptionThreshold_. If set to 0, no adaption of the optimization
	  * parameters will take place
	  *
	  * @param adaptionCounter The value that should be assigned to the adaptionCounter_ variable
	  */
    void setAdaptionThreshold(const std::uint32_t &adaption_threshold) {
        adaptionThreshold_ = adaption_threshold;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of adaption thresholds is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Retrieves the value of the adaptionThreshold_ variable.
	  *
	  * @return The value of the adaptionThreshold_ variable
	  */
    std::uint32_t getAdaptionThreshold() const {
        return adaptionThreshold_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of adaption threshold is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to specify whether adaptions should happen always, never, or with a given
	  * probability. The function is declared virtual so adaptors requiring adaptions to
	  * happen always or never can prevent resetting of the adaptionMode_ variable.
	  *
	  * @param adaptionMode The desired mode (always/never/with a given probability)
	  */
    virtual void setAdaptionMode(adaptionMode am) {
        adaptionMode_ = am;
    }

    /* ----------------------------------------------------------------------------------
	  * Setting of the adaption mode is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * The effect of setting the adaption mode is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Returns the current value of the adaptionMode_ variable
	  *
	  * @return The current value of the adaptionMode_ variable
	  */
    adaptionMode getAdaptionMode() const {
        return adaptionMode_;
    }

    /* ----------------------------------------------------------------------------------
	  * Retrieval of the adaption mode is tested in GAdaptorT<T, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to set the allowed range for adaption probability variation.
	  * NOTE that this function will silently adapt the values of adProb_ and
	  * adProb_reset_, if they fall outside of the new range.
	  */
    void setAdProbRange(fp_type min_ad_prob, fp_type max_ad_prob) {
#ifdef DEBUG
        if(min_ad_prob < 0.) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setAdProbRange(): Error!" << '\n'
                << "min_ad_prob < 0: " << min_ad_prob << '\n'
            );
        }

        if(max_ad_prob > 1.) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setAdProbRange(): Error!" << '\n'
                << "max_ad_prob > 1: " << max_ad_prob << '\n'
            );
        }

        if(min_ad_prob > max_ad_prob) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<T, fp_type>::setAdProbRange(): Error!" << '\n'
                << "Invalid min_ad_prob and/or max_ad_prob: " << min_ad_prob << " / " << max_ad_prob
                << '\n'
            );
        }
#endif /* DEBUG */

        // Store the new values
        minAdProb_ = min_ad_prob;
        if(minAdProb_ < DEFMINADPROB) {
            minAdProb_ = DEFMINADPROB;
        }
        maxAdProb_ = max_ad_prob;

        // Make sure adProb_ and adProb_reset_ fit the new allowed range
        Gem::Common::enforceRangeConstraint<fp_type>(
            adProb_,
            minAdProb_,
            maxAdProb_,
            "GAdaptorT<>::setAdProbRange() / 1"
        );
        Gem::Common::enforceRangeConstraint<fp_type>(
            adProb_reset_,
            minAdProb_,
            maxAdProb_,
            "GAdaptorT<>::setAdProbRange() / 2"
        );
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the allowed range for adProb_ variation
	  */
    auto getAdProbRange() const {
        return std::tuple<fp_type, fp_type>{minAdProb_, maxAdProb_};
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
        if(adaptAdProb_ > fp_type(0.)) {
            adProb_ *= std::exp(normal_distribution_(
                gr,
                typename std::normal_distribution<fp_type>::param_type(0., adaptAdProb_)
            ));
            Gem::Common::enforceRangeConstraint<fp_type>(
                adProb_,
                minAdProb_,
                maxAdProb_,
                "GAdaptorT<>::adapt() / 1"
            );
        }

        if(adaptionMode::WITHPROBABILITY ==
           adaptionMode_) { // The most likely case is indeterminate (means: "sometimes" here)
            if(weighted_bool_(
                   gr,
                   std::bernoulli_distribution::param_type(std::abs(adProb_))
               )) { // Likelihood of adProb_ for the adaption
                adaptAdaption(range, gr);
                customAdaptions(val, range, gr);
                adapted = true;
            }
        }
        else if(adaptionMode::ALWAYS == adaptionMode_) { // always adapt
            adaptAdaption(range, gr);
            customAdaptions(val, range, gr);
            adapted = true;
        }

        // No need to test for "adaptionMode_ == adaptionMode::NEVER" as no action is needed in this case

        if(adapted) {
            return static_cast<std::size_t>(1);
        }
        else {
            return static_cast<std::size_t>(0);
        }
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
	  * @param val_vec A vector of values that need to be adapted
	  * @param range A typical value range for type T
	  * @return The number of adaptions that were carried out
	  */
    std::size_t adapt(std::vector<T> &val_vec, const T &range, Gem::Hap::GRandomBase &gr) {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        std::size_t n_adapted = 0;

        // Update the adaption probability, if requested by the user
        if(adaptAdProb_ > fp_type(0.)) {
            adProb_ *= std::exp(normal_distribution_(
                gr,
                typename std::normal_distribution<fp_type>::param_type(0., adaptAdProb_)
            ));
            Gem::Common::enforceRangeConstraint<fp_type>(
                adProb_,
                minAdProb_,
                maxAdProb_,
                "GAdaptorT<>::adapt() / 2"
            );
        }

        if(adaptionMode::WITHPROBABILITY == adaptionMode_) { // The most likely case
            for(auto &val : val_vec) {
                // A likelihood of adProb_ for adaption
                if(weighted_bool_(
                       gr,
                       std::bernoulli_distribution::param_type(std::abs(adProb_))
                   )) {
                    adaptAdaption(range, gr);
                    customAdaptions(val, range, gr);

                    n_adapted += 1;
                }
            }
        }
        else if(adaptionMode::ALWAYS == adaptionMode_) { // always adapt
            for(auto &val : val_vec) {
                adaptAdaption(range, gr);
                customAdaptions(val, range, gr);

                n_adapted += 1;
            }
        }

        // No need to test for "adaptionMode_ == adaptionMode::NEVER" as no action is needed in this case

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
	  * @param range A typical value range for type T
	  * @return A boolean indicating whether updates were performed
	  */
    virtual bool updateOnStall(
        const std::size_t &n_stalls,
        const T & /*range*/
    ) {
#ifdef DEBUG
        if(0 == n_stalls) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorT<>::updateOnStall(" << n_stalls << "): Error!" << '\n'
                << "Function called for zero n_stalls" << '\n'
            );
        }
#endif

        // Reset the adaption probability
        if(adProb_ == adProb_reset_) {
            return false;
        }
        else {
            adProb_ = adProb_reset_;
            return true;
        }
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
	  * @param adaoptorName The name of the adaptor to be queried
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
        else {                         // O.k., this query is for us!
            if(property == "ad_prob") { // The only property that can be queried for this class
                data.push_back(std::any(adProb_));
            }
            else { // Ask derived classes
                if(not this->customQueryProperty(property, data)) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "In GAdaptorT<T, fp_type>::queryPropertyFrom(): Error!" << '\n'
                        << "Function was called for unimplemented property " << property << '\n'
                        << "on adaptor " << adaptor_name << '\n'
                    );
                }
            }
        }
    }

    /***************************************************************************/
    /** @brief Allows derived classes to randomly initialize parameter members */
    virtual bool randomInit(Gem::Hap::GRandomBase &) = 0;

protected:
    /***************************************************************************/
    /**
	  * Loads the contents of another GAdaptorT<T, fp_type>. The function
	  * is similar to a copy constructor (but with a pointer as
	  * argument). As this function might be called in an environment
	  * where we do not know the exact type of the class, the
	  * GAdaptorT<T, fp_type> is camouflaged as a GObject . This implies the
	  * need for dynamic conversion.
	  *
	  * @param gb A pointer to another GAdaptorT<T, fp_type>, camouflaged as a GObject
	  */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GAdaptorT<T, fp_type> reference independent of this object and convert the pointer
        const GAdaptorT<T, fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GObject, GAdaptorT<T, fp_type>>(cp, this);

        // Load the parent class'es data
        GObject::load_(cp);

        // Then our own data
        adaptionCounter_ = p_load->adaptionCounter_;
        adaptionThreshold_ = p_load->adaptionThreshold_;
        adProb_ = p_load->adProb_;
        adaptAdProb_ = p_load->adaptAdProb_;
        minAdProb_ = p_load->minAdProb_;
        maxAdProb_ = p_load->maxAdProb_;
        adaptionMode_ = p_load->adaptionMode_;
        adaptAdaptionProbability_ = p_load->adaptAdaptionProbability_;
        adProb_reset_ = p_load->adProb_reset_;
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
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const fp_type & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GAdaptorT<T, fp_type> reference independent of this object and convert the pointer
        const GAdaptorT<T, fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GObject, GAdaptorT<T, fp_type>>(cp, this);

        GToken token("GAdaptorT<T, fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GObject>(*this, *p_load, token);

        // ... and then the local data
        compare_t(IDENTITY(adaptionCounter_, p_load->adaptionCounter_), token);
        compare_t(IDENTITY(adaptionThreshold_, p_load->adaptionThreshold_), token);
        compare_t(IDENTITY(adProb_, p_load->adProb_), token);
        compare_t(IDENTITY(adaptAdProb_, p_load->adaptAdProb_), token);
        compare_t(IDENTITY(minAdProb_, p_load->minAdProb_), token);
        compare_t(IDENTITY(maxAdProb_, p_load->maxAdProb_), token);
        compare_t(IDENTITY(adaptionMode_, p_load->adaptionMode_), token);
        compare_t(IDENTITY(adaptAdaptionProbability_, p_load->adaptAdaptionProbability_), token);
        compare_t(IDENTITY(adProb_reset_, p_load->adProb_reset_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * This function helps to adapt the adaption parameters, if certain conditions are met.
	  * Adaption is triggered by the parameter object.
	  *
	  *  @param range A typical range for the parameter with type T
	  */
    void adaptAdaption(const T &range, Gem::Hap::GRandomBase &gr) {
        using namespace Gem::Common;
        using namespace Gem::Hap;

        // The adaption parameters are modified every adaptionThreshold_ number of adaptions.
        if(adaptionThreshold_ > 0) {
            if(++adaptionCounter_ >= adaptionThreshold_) {
                adaptionCounter_ = 0;
                customAdaptAdaption(range, gr);
            }
        }
        else if(adaptAdaptionProbability_) { // Do the same with probability settings
            // Likelihood of adaptAdaptionProbability_ for the adaption
            if(weighted_bool_(
                   gr,
                   std::bernoulli_distribution::param_type(std::abs(adaptAdaptionProbability_))
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
        const std::string & /*property*/
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
	  *  @param range A typical range for the parameter with type T
	  */
    virtual void customAdaptAdaption(const T &, Gem::Hap::GRandomBase &gr) { /* nothing */
    }

    /***************************************************************************/

    /** @brief Adaption of values as specified by the user */
    virtual void customAdaptions(T &, const T &, Gem::Hap::GRandomBase &) = 0;

    /** @brief Creates a deep copy of this object */
    GObject *clone_() const override = 0;

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
        if(GObject::modify_GUnitTests_()) {
            result = true;
        }

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

        // Call the parent classes' functions
        GObject::specificTestsNoFailureExpected_GUnitTests_();

        // Retrieve a random number generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::set/getAdaptionProbability()
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

            // Make sure the adaption probability is taken into account
            p_test->setAdaptionMode(adaptionMode::WITHPROBABILITY);
            // Prevent changes to adProb_
            p_test->setAdaptAdProb(0.);

            p_test->setAdProbRange(0., 1.);

            const std::size_t n_tests = 100000;

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();
            p_test->setAdaptionProbability(0.5);

            const std::size_t n_tests = 10000;

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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

        // Call the parent classes' functions
        GObject::specificTestsFailuresExpected_GUnitTests_();

        // Retrieve a random number generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptionProbability(): Setting a value < 0. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

            // Setting a probability < 0 should throw
            CHECK_THROWS_AS(p_test->setAdaptionProbability(-1.), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptionProbability(): Setting a value > 1. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

            // Setting a probability > 1 should throw
            CHECK_THROWS_AS(p_test->setAdaptionProbability(2.), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptAdaptionProbability(): Setting a value < 0. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

            // Setting a probability < 0 should throw
            CHECK_THROWS_AS(p_test->setAdaptAdaptionProbability(-1.), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Test of GAdaptorT<T, fp_type>::setAdaptAdaptionProbability(): Setting a value > 1. should throw
            std::shared_ptr<GAdaptorT<T, fp_type>> p_test = this->clone<GAdaptorT<T, fp_type>>();

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

    std::uint32_t adaptionCounter_ = 0; ///< A local counter
    std::uint32_t adaptionThreshold_ =
        DEFAULTADAPTIONTHRESHOLD; ///< Specifies after how many adaptions the adaption itself should be adapted
    fp_type adProb_ = DEFAULTADPROB; ///< internal representation of the adaption probability
    fp_type adaptAdProb_ = DEFAUPTADAPTADPROB; ///< The rate, at which adProb_ should be adapted
    fp_type minAdProb_ = DEFMINADPROB; ///< The lower allowed value for adProb_ during variation
    fp_type maxAdProb_ = DEFMAXADPROB; ///< The upper allowed value for adProb_ during variation
    adaptionMode adaptionMode_ = adaptionMode::
        WITHPROBABILITY; ///< Whether to adapt always, never, or with a given probability
    fp_type adaptAdaptionProbability_ =
        DEFAULTADAPTADAPTIONPROB; ///< Influences the likelihood for the adaption of the adaption parameters
    fp_type adProb_reset_ =
        adProb_; ///< The value to which adProb_ will be reset if "updateOnStall()" is called
};

/******************************************************************************/
/** @brief Specialization of GAdaptorT<T,fp_type>::adapt(vec) for the T==bool */
template <>
std::size_t
GAdaptorT<bool, double>::adapt(std::vector<bool> &, const bool &, Gem::Hap::GRandomBase &);

/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost::serialization {
template <typename T, typename fp_type>
struct is_abstract<Gem::Geneva::GAdaptorT<T, fp_type>> : public boost::true_type {};
template <typename T, typename fp_type>
struct is_abstract<const Gem::Geneva::GAdaptorT<T, fp_type>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
