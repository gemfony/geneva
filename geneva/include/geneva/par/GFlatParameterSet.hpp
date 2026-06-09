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
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Boost headers go here
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>

// Geneva headers go here
#include "geneva/par/GFlatParameters.hpp"
#include "geneva/par/GParameterSet.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * A convenience base for individuals whose genome is stored flat (struct-of-arrays)
 * rather than as a tree of parameter objects. Subclass this exactly as you would
 * GParameterSet: add the parameter objects (GDoubleObject, GConstrainedDoubleObject,
 * collections, ...) in the constructor, then call compileToFlat() once at the end.
 * That replaces the freshly-added tree with a single GFlatParameters node holding all
 * the values flat -- so the individual then clones, serialises, transports and adapts
 * via the flat path, while the rest of your code (fitnessCalculation via streamline()
 * or the typed accessors below) is unchanged.
 *
 * Only fitnessCalculation() and clone_() remain to be implemented by the subclass, as
 * for any GParameterSet.
 */
class GFlatParameterSet // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterSet {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GParameterSet",
            boost::serialization::base_object<GParameterSet>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    GFlatParameterSet() = default;
    GFlatParameterSet(const GFlatParameterSet &) = default;
    ~GFlatParameterSet() override = default;
    GFlatParameterSet &operator=(const GFlatParameterSet &) = default;

    /** @brief Compiles the parameter objects added so far into a single flat node */
    void compileToFlat();

    /** @brief Read access to the flat value arrays (valid after compileToFlat) */
    [[nodiscard]] const std::vector<double> &flatDoubleValues() const;
    [[nodiscard]] const std::vector<float> &flatFloatValues() const;
    [[nodiscard]] const std::vector<std::int32_t> &flatInt32Values() const;
    [[nodiscard]] const std::vector<std::uint8_t> &flatBoolValues() const;

protected:
    /** @brief Access to the single flat node (after compileToFlat) */
    [[nodiscard]] GFlatParameters &flatNode();
    [[nodiscard]] const GFlatParameters &flatNode() const;
};

/******************************************************************************/
/**
 * Builds a row-major [ items.size() x dim ] buffer of the floating-point genome
 * values of @p items, ready to be handed to a batched (e.g. GPU) evaluator. This
 * is the "zero-copy flatten": for a flat individual (GFlatParameterSet) each row
 * is a straight copy out of the contiguous genome array -- no per-item tree walk
 * and no intermediate scratch vector. Tree-based individuals are still supported
 * and fall back transparently to streamline(), so a marshaller can call this
 * unconditionally and simply gets the fast path whenever the genome is flat.
 *
 * @tparam scalar_type Either double or float -- the element type of the batch buffer.
 * @tparam ItemPtr     Any pointer-like handle to a GParameterSet (raw / shared / unique).
 * @param items   The work items, all of which must expose the same parameter count.
 * @param out     Receives the flattened buffer (resized to items.size() * dim).
 */
template <typename scalar_type, typename ItemPtr>
void flattenGenomesRowMajor(const std::vector<ItemPtr> &items, std::vector<scalar_type> &out) {
    static_assert(
        std::is_same_v<scalar_type, double> || std::is_same_v<scalar_type, float>,
        "flattenGenomesRowMajor: scalar_type must be double or float"
    );

    if(items.empty()) {
        out.clear();
        return;
    }

    std::vector<scalar_type> scratch;
    items.front()->template streamline<scalar_type>(scratch); // establishes the per-item dimension
    const std::size_t dim = scratch.size();
    out.resize(items.size() * dim);

    for(std::size_t i = 0; i < items.size(); ++i) {
        scalar_type *dst = out.data() + (i * dim);

        // Fast path: a flat individual hands out its contiguous value array directly.
        const auto *flat = dynamic_cast<const GFlatParameterSet *>(&*items[i]);
        if(flat != nullptr) {
            const std::vector<scalar_type> *vals = nullptr;
            if constexpr(std::is_same_v<scalar_type, double>) {
                vals = &flat->flatDoubleValues();
            } else {
                vals = &flat->flatFloatValues();
            }
            if(vals->size() == dim) {
                std::copy(vals->begin(), vals->end(), dst);
                continue;
            }
        }

        // Fallback: walk the tree into the scratch buffer, then copy the row out.
        items[i]->template streamline<scalar_type>(scratch);
        std::copy(scratch.begin(), scratch.end(), dst);
    }
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
