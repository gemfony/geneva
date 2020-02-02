/**
 * @file
 */

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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/GParameterSetMultiConstraint.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSetFormulaConstraint)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterSetConstraint::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterSetConstraint reference independent of this object and convert the pointer
	const GParameterSetConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterSetConstraint>(cp, this);

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
void GParameterSetConstraint::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GPreEvaluationValidityCheckT<GParameterSet>::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetConstraint
 */
void GParameterSetConstraint::load_(const GObject *cp) {
	// Check that we are dealing with a GParameterSetConstraint reference independent of this object and convert the pointer
	const GParameterSetConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterSetConstraint>(cp, this);

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
GParameterSetFormulaConstraint::GParameterSetFormulaConstraint(std::string rawFormula)
	: rawFormula_(rawFormula)
{ /* nothing */ }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterSetFormulaConstraint::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterSetFormulaConstraint reference independent of this object and convert the pointer
	const GParameterSetFormulaConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterSetFormulaConstraint>(cp, this);

	GToken token("GParameterSetFormulaConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterSetConstraint>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(rawFormula_, p_load->rawFormula_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GParameterSetFormulaConstraint::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
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
double GParameterSetFormulaConstraint::check_(
	const GParameterSet *p
) const {
	std::map<std::string, std::vector<double>> parameterValues;

	p->streamline(parameterValues); // Extract the parameter values including names
	Gem::Common::GFormulaParserT<double> f(rawFormula_); // Create the parser

	try {
		return f(parameterValues); // Parse the formula. This may throw a Gem::Common::math_logic_error
	} catch (Gem::Common::math_logic_error &m) {
		glogger
		<< "In GParameterSetFormulaConstraint::check_(): WARNING" << std::endl
		<< "Caught Gem::Common::math_logic_error with message" << std::endl
		<< m.what() << std::endl
		<< "We will return MAX_DOUBLE" << std::endl
		<< GWARNING;

		return boost::numeric::bounds<double>::highest();
	}
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetFormulaConstraint
 */
void GParameterSetFormulaConstraint::load_(const GObject *cp) {
	// Check that we are dealing with a GParameterSetFormulaConstraint reference independent of this object and convert the pointer
	const GParameterSetFormulaConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterSetFormulaConstraint>(cp, this);

	// Load our parent class'es data ...
	GPreEvaluationValidityCheckT<GParameterSet>::load_(cp);

	// ... and then our local data
	rawFormula_ = p_load->rawFormula_;
}

/******************************************************************************/
/**
 * Returns a deep clone of this object
 */
GObject *GParameterSetFormulaConstraint::clone_() const {
	return new GParameterSetFormulaConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
