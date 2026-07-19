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
#include <string>
#include <string_view>

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp" // Gem::Common::condnotset
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * A CRTP scaffold that generates the boilerplate every concrete optimization algorithm otherwise
 * repeats by hand: clone_(), name_(), getAlgorithmName_(), getAlgorithmPersonalityType_(), and the
 * three default GUnitTests stubs. A concrete algorithm derives as
 *
 *     class GFoo : public GOptimizationAlgorithmT<GFoo>             // direct-from-base algorithms
 *     class GBar : public GOptimizationAlgorithmT<GBar, GParChild>  // mu/lambda algorithms
 *
 * and supplies three static string identifiers (public, so this layer can read them):
 *
 *     static constexpr std::string_view oa_class_name       = "GFoo";
 *     static constexpr std::string_view oa_algorithm_name   = "Foo Optimizer";
 *     static constexpr std::string_view oa_personality_type = "PERSONALITY_FOO";
 *
 * This layer holds NO data members and is never serialized directly: the concrete class keeps
 * serializing base_object<GOptimizationAlgorithmBase> (or <GParChild>), so the on-the-wire / on-disk
 * archive structure is identical to the hand-written version. The GUnitTests stubs are ordinary
 * virtuals, so an algorithm with real algorithm-specific tests simply overrides them.
 *
 * @tparam Derived The concrete algorithm (CRTP).
 * @tparam Parent  The class to derive from -- GOptimizationAlgorithmBase by default, or an
 *                 intermediate such as GParChild for the mu/lambda algorithms.
 */
template <typename Derived, typename Parent = GOptimizationAlgorithmBase>
class GOptimizationAlgorithmT : public Parent {
public:
    /** @brief The default constructor */
    GOptimizationAlgorithmT() = default;
    /**
     * @brief The copy constructor
     *
     * @param The object to be copied (default member-wise copy).
     */
    GOptimizationAlgorithmT(const GOptimizationAlgorithmT &) = default;
    /** @brief The destructor */
    ~GOptimizationAlgorithmT() override = default;

protected:
    /** @brief Applies modifications to this object (test scaffold). Override for real tests. */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        return Parent::modify_GUnitTests_();
#else  /* GEM_TESTING */
        Gem::Common::condnotset(std::string(Derived::oa_class_name) + "::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif /* GEM_TESTING */
    }

    /** @brief Performs self tests that are expected to succeed. Override for real tests. */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        Parent::specificTestsNoFailureExpected_GUnitTests_();
#else  /* GEM_TESTING */
        Gem::Common::condnotset(
            std::string(Derived::oa_class_name) + "::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif /* GEM_TESTING */
    }

    /** @brief Performs self tests that are expected to fail. Override for real tests. */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        Parent::specificTestsFailuresExpected_GUnitTests_();
#else  /* GEM_TESTING */
        Gem::Common::condnotset(
            std::string(Derived::oa_class_name) + "::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif /* GEM_TESTING */
    }

private:
    /**
     * @brief Emits a name for this class / object
     *
     * @return The concrete algorithm's class name, taken from Derived::oa_class_name.
     */
    [[nodiscard]] std::string name_() const override { return std::string(Derived::oa_class_name); }

    /**
     * @brief Creates a deep clone of this object
     *
     * @return A heap-allocated deep copy of this object, as a GOptimizationAlgorithmBase pointer (caller owns it).
     */
    [[nodiscard]] GOptimizationAlgorithmBase *clone_() const override {
        return new Derived(static_cast<const Derived &>(*this));
    }

    /**
     * @brief Returns the human-readable name of this optimization algorithm
     *
     * @return The algorithm's human-readable name, taken from Derived::oa_algorithm_name.
     */
    [[nodiscard]] std::string getAlgorithmName_() const override { return std::string(Derived::oa_algorithm_name); }

    /**
     * @brief Returns the personality-type tag of this optimization algorithm
     *
     * @return The algorithm's personality-type tag, taken from Derived::oa_personality_type.
     */
    [[nodiscard]] std::string getAlgorithmPersonalityType_() const override {
        return std::string(Derived::oa_personality_type);
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
