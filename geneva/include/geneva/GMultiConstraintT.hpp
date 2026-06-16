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

// Standard header files go here
#include <tuple>
#include <type_traits>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonInterfaceT.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem::Geneva {

// Forward declaration
namespace Parameters { class GOptimizableEntity; }

/******************************************************************************/
/**
 * This is the base class of a hierarchy of classes dealing with inter-parameter
 * constraints. Objects representing the template parameter are evaluated for their
 * validity. Note that the classes in this hierarchy are meant to be used PRIOR
 * to the evaluation.
 */
template <typename ind_type>
class GPreEvaluationValidityCheckT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<GPreEvaluationValidityCheckT<ind_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // This is the CRTP category root. Its CRTP base
        // (Gem::Common::GCommonInterfaceT<GPreEvaluationValidityCheckT<ind_type>>)
        // carries no state and is therefore not serialized as a base_object --
        // mirroring GObject, whose serialize() is likewise empty. The polymorphic
        // base_object chain bottoms out here; only our own data is serialized.
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

    // We only accept validity checks for types derived directly or indirectly from GOptimizableEntity
    static_assert(
        std::is_base_of_v<gpar::GOptimizableEntity, ind_type>,
        "GOptimizableEntity is no base of ind_type"
    );

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GPreEvaluationValidityCheckT() = default;

    /***************************************************************************/
    /**
     * The copy constructor
     */
    GPreEvaluationValidityCheckT(const GPreEvaluationValidityCheckT<ind_type> &cp) = default;

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GPreEvaluationValidityCheckT() override = default;

    /***************************************************************************/
    /**
     * Checks whether a given parameter set is valid. The function returns a
     * double value which is expected to be >= 0. Values in the range [0,1]
     * indicate valid parameters (according to this constraint). Values above 1
     * indicate invalid parameters. The size of the return value can thus
     * be used to indicate the extent of the invalidity. Two policies are implemented
     * when check_() returns a value < 0: If allowNevative is set to true, such
     * evaluations are considered to be valid, and the function returns 0. If
     * allow_negative is set to false, am invalidity is calculated, and the return-
     * value will be > 1.
     */
    double check(const ind_type *cp) const {
        double result = check_(cp);

        if(allow_negative_) {
            if(result <= 1.) { // valid
                return 0.;
            }
                            return result;
           
        }
        else {
            if(result >= 0. && result <= 1.) { // valid
                return 0.;
            }
                           // invalid
                if(result < 0.) { // we need to calculate a replacement value
                    // Will be the more invalid the further below 0 "result" is
                    return 1. + std::abs(result);
                }
                else { // result > 1, we may just return the unmodified value
                    return result;
                }
           
        }
    }

    /***************************************************************************/
    /**
     * Checks whether the constraint is valid
     *
     * @param cp A pointer to the individual to be checked
     * @param validity_level Will be filled with the validity level of this individual
     * @return A boolean indicating whether a constraint is valid
     */
    bool isValid(const ind_type *cp, double &validity_level) const {
        // Set the external validity level
        validity_level = this->check(cp);

        if(std::numeric_limits<double>::max() == validity_level ||
           std::numeric_limits<double>::lowest() == validity_level) {
            return false;
        }

        if(allow_negative_) {
            return (validity_level <= 1.);
        }
                    return (validity_level >= 0. && validity_level <= 1.);
       
    }

    /***************************************************************************/
    /**
     * Checks whether a constraint it invalid
     *
     * @param cp A pointer to the individual to be checked
     * @param validity_level Will be filled with the validity level of this individual
     * @return A boolean indicating whether a constraint is invalid
     */
    bool isInvalid(const ind_type *cp, double &validity_level) const {
        return not this->isValid(cp, validity_level);
    }

    /***************************************************************************/
    /**
     * Allows to specify whether negative values are considered to be valid
     */
    bool getAllowNegative() const {
        return allow_negative_;
    }

    /***************************************************************************/
    /**
     * Allows to specify whether negative values are considered to be valid
     */
    void setAllowNegative(bool allow_negative) {
        allow_negative_ = allow_negative;
    }

protected:
    /***************************************************************************/
    /**
     * Checks whether a given parameter set is valid. The function returns a
     * double value which is expected to be >= 0., giving a level of confidence
     * that this is a valid solution. This function must be overloaded in
     * derived classes.
     */
    virtual double check_(const ind_type *) const = 0;

    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * TODO: Check whether it makes sense to provide custom configuration files -- if so, add allowNegative_ here
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Call our parent class'es function
        Gem::Common::GCommonInterfaceT<GPreEvaluationValidityCheckT<ind_type>>::addConfigurationOptions_(gpb);
    }

    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("allow_negative_", allow_negative_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("allow_negative_", allow_negative_)
        );
    }

    /***************************************************************************/
    /**
     * Loads the data of another GPreEvaluationValidityCheckT<ind_type>
     */
    void load_(const GPreEvaluationValidityCheckT<ind_type> *cp) override {
        // Check that we are dealing with a GPreEvaluationValidityCheckT<ind_type>  reference independent of this object and convert the pointer
        const GPreEvaluationValidityCheckT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GPreEvaluationValidityCheckT<ind_type>,
                GPreEvaluationValidityCheckT<ind_type>>(
                cp,
                this
            );

        // This is the category root; there is no GObject parent class to load.

        // Our own data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GPreEvaluationValidityCheckT<ind_type>>(
        GPreEvaluationValidityCheckT<ind_type> const &,
        GPreEvaluationValidityCheckT<ind_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GPreEvaluationValidityCheckT object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GPreEvaluationValidityCheckT<ind_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GPreEvaluationValidityCheckT<ind_type>  reference independent of this object and convert the pointer
        const GPreEvaluationValidityCheckT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GPreEvaluationValidityCheckT<ind_type>,
                GPreEvaluationValidityCheckT<ind_type>>(
                cp,
                this
            );

        GToken token("GPreEvaluationValidityCheckT<ind_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<Gem::Common::GCommonInterfaceT<GPreEvaluationValidityCheckT<ind_type>>>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        // This is the category root; there is no modifiable GObject parent class.
        bool result = false;

        if(not this->getAllowNegative()) {
            this->setAllowNegative(true);
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GPreEvaluationValidityCheckT<>::modify_GUnitTests", "GEM_TESTING");
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
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GPreEvaluationValidityCheckT<>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // This is the category root; there is no GObject parent class to delegate to.
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GPreEvaluationValidityCheckT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<ind_type> *clone_() const override = 0;

    /***************************************************************************/

    bool allow_negative_ = false; ///< Set to true if negative values are considered to be valid
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An collection of validity checks with the GPreEvaluationValidityCheckT interface
 */
template <typename ind_type>
class GValidityCheckContainerT : public GPreEvaluationValidityCheckT<ind_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &boost::serialization::make_nvp(
            "GPreEvaluationValidityCheckT_ind_type",
            boost::serialization::base_object<GPreEvaluationValidityCheckT<ind_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GValidityCheckContainerT() = default;

    /***************************************************************************/
    /**
     * Initialization from a vector of validity checks
     */
    explicit GValidityCheckContainerT(
        const std::vector<std::shared_ptr<GPreEvaluationValidityCheckT<ind_type>>> &validity_checks
    ) {
        Gem::Common::copyCloneableSmartPointerContainer(validity_checks, validity_checks_);
    }

    /***************************************************************************/
    /**
     * The copy constructor
     */
    GValidityCheckContainerT(const GValidityCheckContainerT<ind_type> &cp)
      : GPreEvaluationValidityCheckT<ind_type>(cp) {
        Gem::Common::copyCloneableSmartPointerContainer(cp.validity_checks_, validity_checks_);
    }

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GValidityCheckContainerT() override = default;

    /***************************************************************************/
    /**
     * The standard assignment operator
     */
    GValidityCheckContainerT<ind_type> &operator=(const GValidityCheckContainerT<ind_type> &cp) {
        if(this == &cp) {
            return *this;
        }
        this->load_(&cp);
        return *this;
    }

    /***************************************************************************/
    /**
     * Adds a validity check to this object. Note that we clone the check so
     * that it can be used multiple times.
     */
    void addCheck(std::shared_ptr<GPreEvaluationValidityCheckT<ind_type>> vc_ptr) {
        if(not vc_ptr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GValidityCheckContainerT<>::addCheck(): Error!" << '\n'
                << "Got empty check pointer" << '\n'
            );
        }

        validity_checks_.push_back(
            vc_ptr->template clone<GPreEvaluationValidityCheckT<ind_type>>()
        );
    }

protected:
    /***************************************************************************/
    /** @brief Checks whether a given parameter set is valid. To be specified in derived classes */
    double check_(const ind_type *) const override = 0;

    /***************************************************************************/
    /**
     * Loads the data of another GPreEvaluationValidityCheckT<ind_type>
     */
    void load_(const GPreEvaluationValidityCheckT<ind_type> *cp) override {
        // Check that we are dealing with a GValidityCheckContainerT<ind_type>  reference independent of this object and convert the pointer
        const GValidityCheckContainerT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GPreEvaluationValidityCheckT<ind_type>,
                GValidityCheckContainerT<ind_type>>(
                cp,
                this
            );

        // Load our parent class'es data ...
        GPreEvaluationValidityCheckT<ind_type>::load_(cp);

        // and then our local data
        Gem::Common::copyCloneableSmartPointerContainer(p_load->validity_checks_, validity_checks_);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GValidityCheckContainerT<ind_type>>(
        GValidityCheckContainerT<ind_type> const &,
        GValidityCheckContainerT<ind_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GPreEvaluationValidityCheckT object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GPreEvaluationValidityCheckT<ind_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GValidityCheckContainerT<ind_type>  reference independent of this object and convert the pointer
        const GValidityCheckContainerT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GPreEvaluationValidityCheckT<ind_type>,
                GValidityCheckContainerT<ind_type>>(
                cp,
                this
            );

        GToken token("GValidityCheckContainerT<ind_type>", e);

        // Compare our parent data ...
        compare_base_t<GPreEvaluationValidityCheckT<ind_type>>(*this, *p_load, token);

        // ... and then the local data
        compare_t(IDENTITY(validity_checks_, p_load->validity_checks_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /** @brief Holds all registered validity checks */
    std::vector<std::shared_ptr<GPreEvaluationValidityCheckT<ind_type>>> validity_checks_;

private:
    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<ind_type> *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A class which combines all values (i.e. values > 1) according to a
 * user-defined policy or returns 0, if all checks are valid.
 */
template <typename ind_type>
class GCheckCombinerT : public GValidityCheckContainerT<ind_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &boost::serialization::make_nvp(
            "GPreEvaluationValidityCheckT_ind_type",
            boost::serialization::base_object<GPreEvaluationValidityCheckT<ind_type>>(*this)
        ) &
            BOOST_SERIALIZATION_NVP(combiner_policy_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GCheckCombinerT() = default;

    /***************************************************************************/
    /**
     * Initialization from a vector of validity checks
     */
    explicit GCheckCombinerT(
        const std::vector<std::shared_ptr<GPreEvaluationValidityCheckT<ind_type>>> &validity_checks
    )
      : GValidityCheckContainerT<ind_type>(validity_checks) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The copy constructor
     */
    GCheckCombinerT(const GCheckCombinerT<ind_type> &) = default;

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GCheckCombinerT() override = default;

    /***************************************************************************/
    /**
     * The standard assignment operator
     */
    GCheckCombinerT<ind_type> &operator=(const GCheckCombinerT<ind_type> &cp) {
        if(this == &cp) {
            return *this;
        }
        this->load_(&cp);
        return *this;
    }

    /***************************************************************************/
    /**
     * Allows to set the combiner policy
     */
    void setCombinerPolicy(validityCheckCombinerPolicy combiner_policy) {
        combiner_policy_ = combiner_policy;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the combiner policy
     */
    validityCheckCombinerPolicy getCombinerPolicy() const {
        return combiner_policy_;
    }

protected:
    /***************************************************************************/
    /**
     * Combines all parameters according to a user-defined policy. Note that we
     * DO have to take care here of a situation where the invalidity equals
     * MIN- or MAX_DOUBLE.
     */
    double check_(const ind_type *cp) const override {
        // First identify invalid checks
        std::vector<double> invalid_checks;
        double validity_level = 0.;
        for(const auto &validity_check : GValidityCheckContainerT<ind_type>::validity_checks_) {
            if(not validity_check->isValid(cp, validity_level)) {
                invalid_checks.push_back(validity_level);
            }
        }

        // We can leave now, if no invalid checks were found
        if(invalid_checks.empty()) { // All checks were valid
            return 0.;
        }

        // Now act on the invalid tests
        using enum Gem::Geneva::validityCheckCombinerPolicy;
        switch(combiner_policy_) {
        // --------------------------------------------------------------------
        // Multiply all invalidities
        case MULTIPLYINVALID: {
            double result = 1.;
            for(const auto &invalidity : invalid_checks) {
                // If we encounter an invalidity at the numeric boundaries, we simply
                // return MAX_DOUBLE
                if(std::numeric_limits<double>::max() == invalidity ||
                   std::numeric_limits<double>::lowest() == invalidity) {
                    return std::numeric_limits<double>::max();
                }

                result *= invalidity;
            }
            return result;
        } break;

            // --------------------------------------------------------------------
            // Add all invalidities
        case ADDINVALID: {
            double result = 0.;
            for(const auto &invalidity : invalid_checks) {
                // If we encounter an invalidity at the numeric boundaries, we simply
                // return MAX_DOUBLE
                if(std::numeric_limits<double>::max() == invalidity ||
                   std::numeric_limits<double>::lowest() == invalidity) {
                    return std::numeric_limits<double>::max();
                }

                result += invalidity;
            }
            return result;
        } break;

            // --------------------------------------------------------------------
        default: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GCheckCombinerT<ind_type>::check_(): Error!" << '\n'
                << "Got invalid combiner_policy_ value: " << combiner_policy_ << '\n'
            );
        }
        }
    }

    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("combiner_policy_", combiner_policy_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("combiner_policy_", combiner_policy_)
        );
    }

    /***************************************************************************/
    /**
     * Loads the data of another GPreEvaluationValidityCheckT<ind_type>
     */
    void load_(const GPreEvaluationValidityCheckT<ind_type> *cp) override {
        // Check that we are dealing with a GCheckCombinerT<ind_type>  reference independent of this object and convert the pointer
        const GCheckCombinerT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GPreEvaluationValidityCheckT<ind_type>,
                GCheckCombinerT<ind_type>>(cp, this);

        // Load our parent class'es data ...
        GPreEvaluationValidityCheckT<ind_type>::load_(cp);

        // and then our local data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GCheckCombinerT<ind_type>>(
        GCheckCombinerT<ind_type> const &,
        GCheckCombinerT<ind_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GPreEvaluationValidityCheckT object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GPreEvaluationValidityCheckT<ind_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GCheckCombinerT<ind_type>  reference independent of this object and convert the pointer
        const GCheckCombinerT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GPreEvaluationValidityCheckT<ind_type>,
                GCheckCombinerT<ind_type>>(cp, this);

        GToken token("GCheckCombinerT<ind_type", e);

        // Compare our parent data ...
        compare_base_t<GValidityCheckContainerT<ind_type>>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

private:
    /***************************************************************************/
    /**
     * Creates a deep clone of this object
     */
    GPreEvaluationValidityCheckT<ind_type> *clone_() const override {
        return new GCheckCombinerT<ind_type>(*this);
    }

    /***************************************************************************/
    // Local data

    validityCheckCombinerPolicy combiner_policy_ = Gem::Geneva::validityCheckCombinerPolicy::
        MULTIPLYINVALID; ///< Indicates how validity checks should be combined
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost::serialization {
template <typename ind_type>
struct is_abstract<Gem::Geneva::GPreEvaluationValidityCheckT<ind_type>>
  : public std::true_type {};
template <typename ind_type>
struct is_abstract<const Gem::Geneva::GPreEvaluationValidityCheckT<ind_type>>
  : public std::true_type {};
} /* namespace boost::serialization */
namespace boost::serialization {
template <typename ind_type>
struct is_abstract<Gem::Geneva::GValidityCheckContainerT<ind_type>> : public std::true_type {};
template <typename ind_type>
struct is_abstract<const Gem::Geneva::GValidityCheckContainerT<ind_type>>
  : public std::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
