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
#include "common/GFormulaParserT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/par/GParameterSet.hpp"
#include <limits>
#include <map>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GParameterSetFormulaConstraint) // NOLINT
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
    const GPreEvaluationValidityCheckT<GParameterSet> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParameterSetConstraint reference independent of this object and convert the pointer
    const GParameterSetConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GParameterSet>,
            GParameterSetConstraint>(cp, this);

    GToken token("GParameterSetConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPreEvaluationValidityCheckT<GParameterSet>>(*this, *p_load, token);

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
    GPreEvaluationValidityCheckT<GParameterSet>::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetConstraint
 */
void GParameterSetConstraint::load_(const GPreEvaluationValidityCheckT<GParameterSet> *cp) {
    // Check that we are dealing with a GParameterSetConstraint reference independent of this object and convert the pointer
    const GParameterSetConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GParameterSet>,
            GParameterSetConstraint>(cp, this);

    // Load our parent class'es data ...
    GPreEvaluationValidityCheckT<GParameterSet>::load_(cp);

    // no local data
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor that accepts a formula in string form as its argument
 */
GParameterSetFormulaConstraint::GParameterSetFormulaConstraint(std::string raw_formula)
  : raw_formula_(raw_formula) { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT object
 * @param e The expected outcome of the comparison
 */
void GParameterSetFormulaConstraint::compare_(
    const GPreEvaluationValidityCheckT<GParameterSet> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParameterSetFormulaConstraint reference independent of this object and convert the pointer
    const GParameterSetFormulaConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GParameterSet>,
            GParameterSetFormulaConstraint>(cp, this);

    GToken token("GParameterSetFormulaConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterSetConstraint>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GParameterSetFormulaConstraint::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GParameterSetConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * This function extracts all double parameter values including their names from the GParameterSet
 * objects. It then initiates replacement of parameter values in the formula string and
 * parsing of the string. If a math error occurs inside of the formula (such as division by 0),
 * the worst possible value will be returned (MAX_DOUBLE, taken from a Boost function).
 *
 * TODO: Make this work for all parameter types
 */
double GParameterSetFormulaConstraint::check_(const GParameterSet *p) const {
    // Parameters carry no intrinsic name anymore, so we bind the formula variables positionally:
    // the i-th double parameter (in streamline order) is exposed to the formula as "var<i>".
    // A formula such as "fabs({{var0}}) + fabs({{var1}})" therefore references the first two
    // double parameters, matching the historical "var0", "var1", ... naming convention.
    std::vector<double> d_values;
    p->streamline<double>(d_values);

    std::map<std::string, std::vector<double>> parameter_values;
    for(std::size_t i = 0; i < d_values.size(); ++i) {
        parameter_values[std::string("var") + Gem::Common::to_string(i)] =
            std::vector<double>{d_values[i]};
    }

    Gem::Common::GFormulaParserT<double> f(raw_formula_); // Create the parser

    try {
        return f(
            parameter_values
        ); // Parse the formula. This may throw a Gem::Common::math_logic_error
    }
    catch(
        Gem::Common::math_logic_error &m
    ) { // NOLINT(bugprone-empty-catch) — logs warning and returns MAX_DOUBLE sentinel
        glogger << "In GParameterSetFormulaConstraint::check_(): WARNING" << '\n'
                << "Caught Gem::Common::math_logic_error with message" << '\n'
                << m.what() << '\n'
                << "We will return MAX_DOUBLE" << '\n'
                << GWARNING;

        return std::numeric_limits<double>::max();
    }
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetFormulaConstraint
 */
void GParameterSetFormulaConstraint::load_(const GPreEvaluationValidityCheckT<GParameterSet> *cp) {
    // Check that we are dealing with a GParameterSetFormulaConstraint reference independent of this object and convert the pointer
    const GParameterSetFormulaConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<GParameterSet>,
            GParameterSetFormulaConstraint>(cp, this);

    // Load our parent class'es data ...
    GPreEvaluationValidityCheckT<GParameterSet>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Returns a deep clone of this object
 */
GPreEvaluationValidityCheckT<GParameterSet> *GParameterSetFormulaConstraint::clone_() const {
    return new GParameterSetFormulaConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
