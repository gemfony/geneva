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
#include <cstdint>
#include <memory>
#include <vector>

// Boost headers go here
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterLayout.hpp"

namespace Gem::Geneva::Parameters {

class GParameterSet; // forward declaration (compileFrom source)

/******************************************************************************/
/**
 * A single GParameterBase node that stores an individual's *entire* parameter
 * set as flat, contiguous, struct-of-arrays storage (one std::vector per scalar
 * type) instead of a tree of per-parameter objects. A shared GParameterLayout
 * describes the structure (slot counts, boundaries, kinds); only the values live
 * here, per individual.
 *
 * This is the "Genome" half of the Layout + Genome split (see
 * prompts/2026-06-08-flat-genome-proposal.md, Model A). Because it derives from
 * GParameterBase and implements the per-type streamline / boundaries / count /
 * assign virtuals, an individual (a GParameterSet) holding a single
 * GFlatParameters node works transparently with the existing optimization-
 * algorithm and courtier stack -- the container is simply iterated and finds one
 * flat node that emits/absorbs all values at once.
 *
 * STATUS (Phase 0): value channels (streamline / boundaries / countParameters /
 * assignValueVector) for all four scalar types, plus clone / load / compare /
 * serialize and a tree -> flat compileFrom(). Adaption (adapt_) is a no-op for
 * now and constrained transfer is not yet applied (every slot is treated as
 * Plain). Subsequent phases add the flat adaptor state + adapt(), constrained
 * transfer, and a GParameterSet-level convenience wrapper.
 */
class GFlatParameters // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterBase {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this)) &
            make_nvp("layout_", layout_) & make_nvp("dv_", dv_) & make_nvp("fv_", fv_) &
            make_nvp("iv_", iv_) & make_nvp("bv_", bv_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GFlatParameters() = default;
    /** @brief The copy constructor (shares the immutable layout, deep-copies the values) */
    GFlatParameters(const GFlatParameters &) = default;
    /** @brief The destructor */
    ~GFlatParameters() override = default;

    /** @brief Copy-assignment (shares the immutable layout, deep-copies the values) */
    GFlatParameters &operator=(const GFlatParameters &) = default;

    /***************************************************************************/
    /**
     * Builds a flat parameter node from an existing (tree-based) individual by
     * streamlining its values and boundaries per scalar type. The resulting node
     * reproduces the source's streamlined values, boundaries and parameter counts.
     */
    static std::unique_ptr<GFlatParameters> compileFrom(const GParameterSet &src);

    /** @brief Read access to the flat value arrays (e.g. for fitnessCalculation) */
    [[nodiscard]] const std::vector<double> &doubleValues() const {
        return dv_;
    }
    [[nodiscard]] const std::vector<float> &floatValues() const {
        return fv_;
    }
    [[nodiscard]] const std::vector<std::int32_t> &int32Values() const {
        return iv_;
    }
    [[nodiscard]] const std::vector<std::uint8_t> &boolValues() const {
        return bv_;
    }

    /** @brief Read access to the shared layout */
    [[nodiscard]] const std::shared_ptr<GParameterLayout> &layout() const {
        return layout_;
    }

protected:
    /** @brief Loads the data of another GParameterBase */
    void load_(const GParameterBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFlatParameters>(
        GFlatParameters const &,
        GFlatParameters const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(const GParameterBase &, const Gem::Common::expectation &, const double &)
        const override;

    /** @brief Streamline channels -- append this node's flat values to the vector */
    void floatStreamline(std::vector<float> &, const activityMode &) const override;
    void doubleStreamline(std::vector<double> &, const activityMode &) const override;
    void int32Streamline(std::vector<std::int32_t> &, const activityMode &) const override;
    void booleanStreamline(std::vector<bool> &, const activityMode &) const override;

    /** @brief Boundary channels -- append per-slot boundaries from the layout */
    void floatBoundaries(std::vector<float> &, std::vector<float> &, const activityMode &)
        const override;
    void doubleBoundaries(std::vector<double> &, std::vector<double> &, const activityMode &)
        const override;
    void int32Boundaries(
        std::vector<std::int32_t> &,
        std::vector<std::int32_t> &,
        const activityMode &
    ) const override;
    void
    booleanBoundaries(std::vector<bool> &, std::vector<bool> &, const activityMode &) const override;

    /** @brief Count channels -- number of slots of each type */
    std::size_t countFloatParameters(const activityMode &) const override;
    std::size_t countDoubleParameters(const activityMode &) const override;
    std::size_t countInt32Parameters(const activityMode &) const override;
    std::size_t countBoolParameters(const activityMode &) const override;

    /** @brief Assign-back channels -- absorb values from a flat vector */
    void assignFloatValueVector(const std::vector<float> &, std::size_t &, const activityMode &)
        override;
    void assignDoubleValueVector(const std::vector<double> &, std::size_t &, const activityMode &)
        override;
    void assignInt32ValueVector(
        const std::vector<std::int32_t> &,
        std::size_t &,
        const activityMode &
    ) override;
    void
    assignBooleanValueVector(const std::vector<bool> &, std::size_t &, const activityMode &) override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GParameterBase *clone_() const override;

    /** @brief Writes the values into a property tree (for result output) */
    void toPropertyTree(pt::ptree &, const std::string &) const override;
    /** @brief Random (re-)initialization of all values within their boundaries */
    bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &) override;
    /** @brief The actual adaption logic (Phase 0: no-op) */
    std::size_t adapt_(Gem::Hap::GRandomBase &) override;
    /** @brief Triggers updates when the optimization process has stalled (no adaptors yet) */
    bool updateAdaptorsOnStall_(std::size_t) override;
    /** @brief Retrieves information from an adaptor on a given property (no adaptors yet) */
    void queryAdaptor_(const std::string &, const std::string &, std::vector<std::any> &)
        const override;

    /***************************************************************************/
    // The shared, immutable structure description (boundaries / kinds / counts).
    std::shared_ptr<GParameterLayout> layout_ = std::make_shared<GParameterLayout>();

    // The per-individual flat values (struct-of-arrays). Booleans are stored as
    // bytes (0/1) to avoid the std::vector<bool> proxy in the hot storage.
    std::vector<double> dv_;
    std::vector<float> fv_;
    std::vector<std::int32_t> iv_;
    std::vector<std::uint8_t> bv_;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::GFlatParameters) // NOLINT
