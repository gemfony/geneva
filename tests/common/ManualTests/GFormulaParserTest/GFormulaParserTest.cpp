/**
 * @file GLoggerTest.cpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <map>

// Boost headers go here
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>

// Geneva headers go here
#include "common/GFormulaParserT.hpp"

using namespace boost::assign;
using namespace Gem::Common;
using namespace std;

#define testFormula(FORMULA)\
{\
	std::string formula( #FORMULA );\
	GFormulaParserT<double> f(formula);\
	double fp_val = FORMULA;\
	double parse_val = f();\
	BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);\
}\

#define testFormulaFailure(FORMULA, EXCEPTION)\
{\
	std::string formula( #FORMULA );\
	GFormulaParserT<double> f(formula);\
	BOOST_CHECK_THROW(f(), EXCEPTION );\
}\

int test_main(int argc, char** const argv) {
	{ // Test replacement of variables and constants (1)
		std::map<std::string, std::vector<double>> parameterValues;

		std::string formula("sin({{0}})/{{1}}");

		std::vector<double> list0 = boost::assign::list_of(4.34343434343434);
		std::vector<double> list1 = boost::assign::list_of(8.98989898989899);

		parameterValues["0"] = list0;
		parameterValues["1"] = list1;

		GFormulaParserT<double> f(formula);

		double fp_val = sin(4.34343434343434)/8.98989898989899;
		double parse_val = f(parameterValues);

		BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);
	}

	{ // Test replacement of variables and constants (1)
		std::map<std::string, std::vector<double>> parameterValues;

		std::string formula("fabs(sin({{0}})/max({{1}}, 0.000001))");

		std::vector<double> list0 = boost::assign::list_of(4.34343434343434);
		std::vector<double> list1 = boost::assign::list_of(8.98989898989899);

		parameterValues["0"] = list0;
		parameterValues["1"] = list1;

		GFormulaParserT<double> f(formula);

		double fp_val = fabs(sin(4.34343434343434)/max(8.98989898989899, 0.000001));
		double parse_val = f(parameterValues);

		BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);
	}

	{ // Test replacement of variables and constants (1)
		std::map<std::string, std::vector<double>> parameterValues;

		std::string formula("sin({{var0[2]}})/{{var1}}");

		std::vector<double> list0 = boost::assign::list_of(1.5)(2.5)(3.5);
		std::vector<double> list1 = boost::assign::list_of(8.98989898989899);

		parameterValues["var0"] = list0;
		parameterValues["var1"] = list1;

		GFormulaParserT<double> f(formula);

		double fp_val = sin(list0[2])/list1[0];
		double parse_val = f(parameterValues);

		BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);
	}

	{ // Test replacement of variables and constants (2)
		std::map<std::string, std::vector<double>> parameterValues;
		std::map<std::string, double> userConstants;

		std::string formula("gem*sin({{var1}})*cos(pi)");

		std::vector<double> var1list = boost::assign::list_of(2.);
		parameterValues["var1"] = var1list;
		userConstants["gem"] = -1.;

		GFormulaParserT<double> f(formula, userConstants);

		double fp_val = -1.*sin(2.)*cos(boost::math::constants::pi<double>());
		double parse_val = f(parameterValues);

		BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);
	}

	// Test constants
	{
		std::string formula("pi");

		GFormulaParserT<double> f(formula);

		double fp_val = boost::math::constants::pi<double>();
		double parse_val = f();

		BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);
	}

	{
		std::string formula("e");

		GFormulaParserT<double> f(formula);

		double fp_val = boost::math::constants::e<double>();
		double parse_val = f();

		BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001);
	}

	// Test simple calculations
	testFormula(1.234);
	testFormula(1.2e3);
	testFormula(2e-03);
	testFormula(-1);
	testFormula(1*2+3*4);
	testFormula(1*(2+3)*4);
	testFormula((1*2)+(3*4));
	testFormula(-(1));
	testFormula(-(-1));
	testFormula(+(-1));
	testFormula(+(+1));

	// Test functions
	testFormula(fabs(-1.0));
	testFormula(acos(1.0));
	testFormula(asin(1.0));
	testFormula(atan(1.0));
	testFormula(ceil(0.5));
	testFormula(cos(1.0));
	testFormula(cosh(1.0));
	testFormula(exp(1.0));
	testFormula(floor(1.0));
	testFormula(log(1.0));
	testFormula(log10(1.0));
	testFormula(sin(1.0));
	testFormula(sinh(1.0));
	testFormula(sqrt(1.0));
	testFormula(tan(1.0));
	testFormula(tanh(1.0));
	testFormula(pow(2.0, 3.0));
	testFormula(max(2.0, 3.0));
	testFormula(min(2.0, 3.0));

	// Test synthesized formulas
	testFormula(sinh(1.0)*sin(1.0));

	// Test failures in formulas
	testFormulaFailure(1/0, gemfony_exception);
	testFormulaFailure(1/0, Gem::Common::math_logic_error);
	testFormulaFailure(1/0, Gem::Common::division_by_0);

	testFormulaFailure(acos(-2), gemfony_exception);
	testFormulaFailure(acos(-2), Gem::Common::math_logic_error);
	testFormulaFailure(acos(-2), Gem::Common::acos_invalid_range<double>);

	testFormulaFailure(acos(+2), gemfony_exception);
	testFormulaFailure(acos(+2), Gem::Common::math_logic_error);
	testFormulaFailure(acos(+2), Gem::Common::acos_invalid_range<double>);

	testFormulaFailure(asin(-2), gemfony_exception);
	testFormulaFailure(asin(-2), Gem::Common::math_logic_error);
	testFormulaFailure(asin(-2), Gem::Common::asin_invalid_range<double>);

	testFormulaFailure(asin(+2), gemfony_exception);
	testFormulaFailure(asin(+2), Gem::Common::math_logic_error);
	testFormulaFailure(asin(+2), Gem::Common::asin_invalid_range<double>);

	testFormulaFailure(log(0), gemfony_exception);
	testFormulaFailure(log(0), Gem::Common::math_logic_error);
	testFormulaFailure(log(0), Gem::Common::log_negative_value<double>);

	testFormulaFailure(log(-1), gemfony_exception);
	testFormulaFailure(log(-1), Gem::Common::math_logic_error);
	testFormulaFailure(log(-1), Gem::Common::log_negative_value<double>);

	testFormulaFailure(log10(0), gemfony_exception);
	testFormulaFailure(log10(0), Gem::Common::math_logic_error);
	testFormulaFailure(log10(0), Gem::Common::log10_negative_value<double>);

	testFormulaFailure(log10(-1), gemfony_exception);
	testFormulaFailure(log10(-1), Gem::Common::math_logic_error);
	testFormulaFailure(log10(-1), Gem::Common::log10_negative_value<double>);

	testFormulaFailure(sqrt(-1), gemfony_exception);
	testFormulaFailure(sqrt(-1), Gem::Common::math_logic_error);
	testFormulaFailure(sqrt(-1), Gem::Common::sqrt_negative_value<double>);

	return 0;
}
