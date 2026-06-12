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

#include "geneva/par/GParameterSetMultiConstraint.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/ind/GTreeGenome.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT object
 * @param e The expected outcome of the comparison
 */
void GParameterSetConstraint::compare_(
    const GPreEvaluationValidityCheckT<GTreeGenome> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParameterSetConstraint reference independent of this object and convert the pointer
    const GParameterSetConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GTreeGenome>,
            GParameterSetConstraint>(cp, this);

    GToken token("GParameterSetConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPreEvaluationValidityCheckT<GTreeGenome>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GParameterSetConstraint::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GPreEvaluationValidityCheckT<GTreeGenome>::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetConstraint
 */
void GParameterSetConstraint::load_(const GPreEvaluationValidityCheckT<GTreeGenome> *cp) {
    // Check that we are dealing with a GParameterSetConstraint reference independent of this object and convert the pointer
    const GParameterSetConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GTreeGenome>,
            GParameterSetConstraint>(cp, this);

    // Load our parent class'es data ...
    GPreEvaluationValidityCheckT<GTreeGenome>::load_(cp);

    // no local data
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
