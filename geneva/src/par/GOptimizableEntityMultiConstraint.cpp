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

#include "geneva/par/GOptimizableEntityMultiConstraint.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT<GOptimizableEntity> object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating point comparisons (unused here)
 */
void GOptimizableEntityConstraint::compare_(
    const GPreEvaluationValidityCheckT<GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GOptimizableEntityConstraint reference independent of this object and convert the pointer
    const GOptimizableEntityConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GOptimizableEntity>,
            GOptimizableEntityConstraint>(cp, this);

    GToken token("GOptimizableEntityConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPreEvaluationValidityCheckT<GOptimizableEntity>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options are added
 */
void GOptimizableEntityConstraint::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GPreEvaluationValidityCheckT<GOptimizableEntity>::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GOptimizableEntityConstraint
 *
 * @param cp A pointer to another object to load from, camouflaged as a GPreEvaluationValidityCheckT<GOptimizableEntity>
 */
void GOptimizableEntityConstraint::load_(const GPreEvaluationValidityCheckT<GOptimizableEntity> *cp) {
    // Check that we are dealing with a GOptimizableEntityConstraint reference independent of this object and convert the pointer
    const GOptimizableEntityConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GOptimizableEntity>,
            GOptimizableEntityConstraint>(cp, this);

    // Load our parent class'es data ...
    GPreEvaluationValidityCheckT<GOptimizableEntity>::load_(cp);

    // no local data
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
