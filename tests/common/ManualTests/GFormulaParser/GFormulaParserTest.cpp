/**
 * @file GLoggerTest.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Standard headers go here
#include <iostream>
#include <map>

// Boost headers go here
// #include <boost/test/minimal.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
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


int test_main(int argc, char** argv) {
   {  // Test replacement of variables and constants (1)
      std::map<std::string, std::vector<double> > parameterValues;

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

   {  // Test replacement of variables and constants (1)
      std::map<std::string, std::vector<double> > parameterValues;

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

   {  // Test replacement of variables and constants (2)
      std::map<std::string, std::vector<double> > parameterValues;
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

   return 0;
}
