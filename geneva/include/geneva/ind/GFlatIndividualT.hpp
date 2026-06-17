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

// Boost header files go here
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>

// Geneva headers go here
#include "geneva/ind/GFlatGenome.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A CRTP convenience base for concrete flat individuals. Because GFlatGenome holds all genome state
 * generically, a typical individual adds no extra data members, so its deep-clone is purely
 * mechanical -- this base generates it (clone_ = new Derived(*this)). load_ / compare_ are inherited
 * from GFlatGenome unchanged (they copy / compare the value arrays, which is all such an individual
 * has). The result: a minimal flat individual is a constructor that builds its genome plus a
 * fitnessCalculation(), with one BOOST_CLASS_EXPORT(Derived) for serialisation.
 *
 * An individual that genuinely carries extra (non-genome) members simply does not use this base and
 * overrides clone_ / load_ / compare_ / serialize itself.
 *
 * Usage:
 * @code
 *   class MyIndividual : public GFlatIndividualT<MyIndividual> {
 *   public:
 *       MyIndividual() {
 *           GGenomeBuilder b;
 *           b.addDouble(0., -10., 10.); // structure only; the adaptor lives on the OA-owned config
 *           this->setGenome(b.build());
 *       }
 *       double fitnessCalculation() override { ... }
 *   private:
 *       friend class boost::serialization::access;
 *       template <typename Archive> void serialize(Archive& ar, const unsigned int) {
 *           ar & boost::serialization::make_nvp(
 *               "GFlatIndividualT",
 *               boost::serialization::base_object<GFlatIndividualT<MyIndividual>>(*this));
 *       }
 *   };
 *   BOOST_CLASS_EXPORT(MyIndividual)
 * @endcode
 *
 * @tparam Derived The concrete flat individual type (CRTP), supplying its constructor and fitnessCalculation()
 */
template <class Derived>
class GFlatIndividualT : public GFlatGenome {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Serialises this individual through its GFlatGenome base (no extra members to add).
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read from / write to
     * @param unsigned The (unused) serialization version number
     */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatGenome",
            boost::serialization::base_object<GFlatGenome>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Inherit the GFlatGenome constructors (default + n-fitness-criteria) */
    using GFlatGenome::GFlatGenome;

protected:
    /** @brief Creates a deep clone of this object via the Derived copy constructor.
     *  @return A heap-allocated deep copy of this individual (as a GFlatGenome base pointer) */
    GFlatGenome *clone_() const override {
        return new Derived(*static_cast<const Derived *>(this));
    }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
