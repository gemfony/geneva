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
#include "geneva/ind/GGenome.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The sole CRTP base a concrete flat individual derives. It is a thin adaptor over the GBoilerplateT
 * mixin (with CloneReturn = GGenome so the generated clone_ returns GGenome* covariantly): clone_, name_,
 * load_, compare_ and serialize are all GENERATED, so a minimal flat individual is just a constructor that
 * builds its genome, an evaluate(), the opt-in flat tag, and one BOOST_CLASS_EXPORT(Derived).
 *
 * Because GGenome holds all genome state generically, a genome-only individual adds no members of its own
 * and inherits GGenomeT's empty member list. An individual that genuinely carries extra (non-genome)
 * members instead declares its OWN localMembers_(), listing them (a documented contract today; see the b2
 * TODO on GGenomeT::localMembers_() for the planned compile-time enforcement).
 *
 * Usage (genome-only):
 * @code
 *   class MyIndividual : public GGenomeT<MyIndividual> {
 *   public:
 *       MyIndividual() {
 *           GGenomeBuilder b;
 *           b.addDouble(0., -10., 10.); // structure only; the adaptor lives on the OA-owned config
 *           this->setGenome(b.build());
 *       }
 *       std::vector<double> evaluate() override { ... }
 *   };
 *   BOOST_CLASS_EXPORT(MyIndividual)
 * @endcode
 *
 * Usage (with extra state): declare localMembers_() listing the extra members, e.g.
 * @code
 *   friend struct Gem::Common::GBoilerplateAccess;
 *   template <typename Self> auto localMembers_(this Self &self) {
 *       return std::make_tuple(Gem::Common::make_member("my_extra_", self.my_extra_));
 *   }
 * @endcode
 *
 * @tparam Derived The concrete flat individual type (CRTP), supplying its constructor and evaluate()
 */
template <class Derived>
class GGenomeT : public Gem::Common::GBoilerplateT<Derived, GGenome, GGenome> {
    ///////////////////////////////////////////////////////////////////////
    friend struct Gem::Common::GBoilerplateAccess;

    /** @brief A genome-only leaf carries no data beyond its genome, so it inherits this empty member list
     *  and the GBoilerplateT mixin generates clone_ / name_ / load_ / compare_ / serialize from it (serialize
     *  additionally emits base_object<GGenome>; clone_ returns GGenome* covariantly). An individual that
     *  genuinely adds non-genome members declares its OWN localMembers_(), which hides this one; it MUST do
     *  so (documented contract), or that state is silently dropped from serialize/compare/load.
     *
     *  TODO (b2, deferred): constrain this to an opt-in tag (`requires { typename Self::gemfony_flat_individual; }`)
     *  so a stateful leaf that forgets its localMembers_() fails to compile instead. That turns the contract
     *  into a compile-time guard, at the cost of tagging every genome-only leaf (~40 today). Deferred as a
     *  dedicated sweep; until C++26 reflection can enumerate members, the tag is the only compile-time guard. */
    template <typename Self>
    auto localMembers_(this Self &) { return std::make_tuple(); }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Inherit the GGenome constructors (default + n-fitness-criteria). */
    using Gem::Common::GBoilerplateT<Derived, GGenome, GGenome>::GBoilerplateT;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
