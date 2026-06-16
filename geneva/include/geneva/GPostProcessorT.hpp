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
#include <set>
#include <string>
#include <tuple>

// Boost headers go here
#include <boost/serialization/serialization.hpp> // See last comment at https://svn.boost.org/trac/boost/ticket/12126 . Fixes "sole" inclusion of set.hpp
#include <boost/serialization/set.hpp>

// Geneva headers go here
#include "common/GSerializableFunctionObjectT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for post-optimizers holding information about which optimization
 * algorithms are eligible for post-processing of individuals. By default, all
 * algorithms are forbidden. Once allowed algorithms are registered through their
 * mnemonics, optimzation algorithms with these mnemonics are allowed. Registering
 * a mnemonic "all" will lead to all restrictions being lifted. This functionality
 * is required as not all optimization algorithms will react gracefully to a silent
 * change of their individuals (think e.g. of gradient descents). The mnemonics
 * must be accessible via the base_type-object through the member-function getMnemonic().
 */
template <typename base_type>
class GPostProcessorBaseT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GSerializableFunctionObjectT<base_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GSerializableFunctionObjectT_base_type",
            boost::serialization::base_object<Gem::Common::GSerializableFunctionObjectT<base_type>>(
                *this
            )
        ) & BOOST_SERIALIZATION_NVP(allowed_mnemonics_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /**************************************************************************/
    /**
	  * The default constructor
	  */
    GPostProcessorBaseT() = default;

    /**************************************************************************/
    /**
	  * The copy constructor
	  */
    GPostProcessorBaseT(const GPostProcessorBaseT<base_type> &cp) = default;

    /**************************************************************************/
    /**
	  * The destructor
	  */
    ~GPostProcessorBaseT() override = default;

    /**************************************************************************/
    /**
	  * Permits postprocessing for a specific type
	  */
    void allowPostProcessingFor(const std::string &oa_mnemonic) {
        allowed_mnemonics_.insert(oa_mnemonic);
    }

    /**************************************************************************/
    /**
	  * Allows to check whether this post-processor is allowed to run under an optimization algorithm with
	  * the given mnemonic. The optimization algorithm queries this at setup (it knows its own mnemonic) and
	  * vetoes post-processing on the work items where it returns false; the individual itself carries no
	  * knowledge of which algorithm owns it.
	  */
    bool postProcessingAllowedFor(const std::string &oa_mnemonic) const {
        if(allowed_mnemonics_.contains("all")) {
            return true;
        }

        // Check whether the given mnemonic was registered with this class
        return allowed_mnemonics_.count(oa_mnemonic) != 0;
    }

protected:
    /**************************************************************************/
    /** @brief Raw post-processing (no checks for eligibility); purely virtual */
    virtual bool raw_processing_(base_type &p_raw) = 0;

    /**************************************************************************/
    /**
	  * Post-processing is triggered here. Eligibility (the mnemonic gate) is decided by the optimization
	  * algorithm at setup, which vetoes post-processing on ineligible work items (see
	  * GProcessingContainerT::vetoPostProcessing); by the time we are invoked the veto has already gated us.
	  */
    bool process_(base_type &p) override {
        return raw_processing_(p);
    }

    /**************************************************************************/
    /**
	  * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
	  */
    void load_(const Gem::Common::GSerializableFunctionObjectT<base_type> *cp) override {
        using namespace Gem::Common;

        // Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
        const GPostProcessorBaseT<base_type> *p_load = g_convert_and_compare<
            GSerializableFunctionObjectT<base_type>,
            GPostProcessorBaseT<base_type>>(cp, this);

        // Load our parent class'es data ...
        Gem::Common::GSerializableFunctionObjectT<base_type>::load_(cp);

        // ... and then our local data
        allowed_mnemonics_ = p_load->allowed_mnemonics_;
    }

    /**************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GPostProcessorBaseT<base_type>>(
        GPostProcessorBaseT<base_type> const &,
        GPostProcessorBaseT<base_type> const &,
        Gem::Common::GToken &
    );

    /**************************************************************************/
    /**
     * Checks for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const Gem::Common::GSerializableFunctionObjectT<base_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
        const GPostProcessorBaseT<base_type> *p_load = Gem::Common::g_convert_and_compare<
            GSerializableFunctionObjectT<base_type>,
            GPostProcessorBaseT<base_type>>(cp, this);

        GToken token("GPostProcessorBaseT", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GSerializableFunctionObjectT<base_type>>(*this, *p_load, token);

        // ... and then our local data
        compare_t<std::set<std::string>>(
            IDENTITY(allowed_mnemonics_, p_load->allowed_mnemonics_),
            token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /**************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        bool result = false;

        // Call the parent classes' functions
        if(Gem::Common::GSerializableFunctionObjectT<base_type>::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GPostProcessorBaseT<base_type>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /**************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        Gem::Common::GSerializableFunctionObjectT<
            base_type>::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GPostProcessorBaseT<base_type>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /**************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        Gem::Common::GSerializableFunctionObjectT<
            base_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GPostProcessorBaseT<base_type>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

private:
    /**************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GPostProcessorBaseT");
    }

    /**************************************************************************/
    /** @brief Creates a deep clone of this object; purely virtual */
    Gem::Common::GSerializableFunctionObjectT<base_type> *clone_() const override = 0;

    /**************************************************************************/
    // Data

    std::set<std::string>
        allowed_mnemonics_; ///< A list of mnemonics for which optimization is allowed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This post-processor runs an evolutionary algorithm, trying to improve the
 * quality of a given individual.
 */
class GEvolutionaryAlgorithmPostOptimizer // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPostProcessorBaseT<gen::GOptimizableEntity> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GPostProcessorBaseT_GOptimizableEntity",
            boost::serialization::base_object<GPostProcessorBaseT<gen::GOptimizableEntity>>(*this)
        ) & BOOST_SERIALIZATION_NVP(oa_config_file_) &
            BOOST_SERIALIZATION_NVP(executor_config_file_) &
            BOOST_SERIALIZATION_NVP(execution_mode_);

        // TODO: How to initialize the ea factory
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /**************************************************************************/
    /** @brief Initialization with the execution mode and configuration file */
    GEvolutionaryAlgorithmPostOptimizer(
        execMode execution_mode,
        const std::string &oa_config_file,
        const std::string &executor_config_file
    );
    /** @brief The copy constructor */
    
    GEvolutionaryAlgorithmPostOptimizer(const GEvolutionaryAlgorithmPostOptimizer &cp) = default;
    /** @brief The destructor */
    ~GEvolutionaryAlgorithmPostOptimizer() override = default;

    /** @brief Allows to set the execution mode for this post-processor (serial vs. multi-threaded) */
    void setExecMode(execMode execution_mode);
    /** @brief Allows to retrieve the current execution mode */
    execMode getExecMode() const;

    /** @brief Allows to specify the name of a configuration file for the optimization algorithm */
    void setOAConfigFile(const std::string &oa_config_file);
    /** @brief Allows to retrieve the configuration file for the optimization algorithm */
    std::string getOAConfigFile() const;

    /** @brief Allows to specify the name of a configuration file for the executor */
    void setExecutorConfigFile(const std::string &executor_config_file);
    /** @brief Allows to retrieve the configuration file for the executor */
    std::string getExecutorConfigFile() const;

protected:
    /**************************************************************************/
    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("oa_config_file_", oa_config_file_),
            Gem::Common::make_member("executor_config_file_", executor_config_file_),
            Gem::Common::make_member("execution_mode_", execution_mode_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("oa_config_file_", oa_config_file_),
            Gem::Common::make_member("executor_config_file_", executor_config_file_),
            Gem::Common::make_member("execution_mode_", execution_mode_)
        );
    }

    /** @brief Loads the data of another GEvolutionaryAlgorithmPostOptimizer object */
    void
    load_(const Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity> *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GEvolutionaryAlgorithmPostOptimizer>(
        GEvolutionaryAlgorithmPostOptimizer const &,
        GEvolutionaryAlgorithmPostOptimizer const &,
        Gem::Common::GToken &
    );

    /** @brief Checks for compliance with expectations with respect to another object of the same type */
    void compare_(
        const Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity> &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief The actual post-processing takes place here (no further checks) */
    bool raw_processing_(gen::GOptimizableEntity &p) override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Returns the name of this class */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    Gem::Common::GSerializableFunctionObjectT<gen::GOptimizableEntity> *clone_() const override;

    /** @brief The standard constructor */
    GEvolutionaryAlgorithmPostOptimizer();

    /**************************************************************************/
    // Data
    std::string
        oa_config_file_; ///< The name of the configuration file for this evolutionary algorithm
    std::string executor_config_file_; ///< The name of the configuration file for the executor
    execMode execution_mode_ =
        execMode::SERIAL; ///< Whether to run the post-optimizer in serial or multi-threaded mode
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Mark this class as abstract.
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GPostProcessorBaseT<gen::GOptimizableEntity>) // NOLINT
// Export of GEvolutionaryAlgorithmPostOptimizer
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithmPostOptimizer) // NOLINT
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
