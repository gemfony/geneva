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

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <numbers>
#include <string>

#include "common/GExceptions.hpp"
#include "common/GFormulaParserT.hpp"

using Catch::Approx;
using FP = Gem::Common::GFormulaParserT<double>;

// ---------------------------------------------------------------------------
// Basic literals and arithmetic

TEST_CASE("GFormulaParserT: integer literal evaluates",
          "[common][formula-parser]") {
    FP f("42");
    CHECK(f.evaluate() == Approx(42.0));
}

TEST_CASE("GFormulaParserT: floating-point literal evaluates",
          "[common][formula-parser]") {
    FP f("3.5");
    CHECK(f.evaluate() == Approx(3.5));
}

TEST_CASE("GFormulaParserT: addition and subtraction",
          "[common][formula-parser]") {
    CHECK(FP("1+2").evaluate()    == Approx(3.0));
    CHECK(FP("10-4-1").evaluate() == Approx(5.0));   // left-to-right
}

TEST_CASE("GFormulaParserT: multiplication and division",
          "[common][formula-parser]") {
    CHECK(FP("3*4").evaluate()      == Approx(12.0));
    CHECK(FP("10/2/5").evaluate()   == Approx(1.0));
}

TEST_CASE("GFormulaParserT: operator precedence (mul/div over add/sub)",
          "[common][formula-parser]") {
    CHECK(FP("1+2*3").evaluate()    == Approx(7.0));
    CHECK(FP("(1+2)*3").evaluate()  == Approx(9.0));
    CHECK(FP("8-4/2").evaluate()    == Approx(6.0));
}

TEST_CASE("GFormulaParserT: unary negation and plus",
          "[common][formula-parser]") {
    CHECK(FP("-5").evaluate()      == Approx(-5.0));
    CHECK(FP("+5").evaluate()      == Approx(5.0));
    CHECK(FP("-(-5)").evaluate()   == Approx(5.0));
}

// ---------------------------------------------------------------------------
// Constants

TEST_CASE("GFormulaParserT: built-in constants e and pi",
          "[common][formula-parser]") {
    CHECK(FP("e").evaluate()  == Approx(std::numbers::e_v<double>));
    CHECK(FP("pi").evaluate() == Approx(std::numbers::pi_v<double>));
}

TEST_CASE("GFormulaParserT: user-defined constants",
          "[common][formula-parser]") {
    FP::constants_map cm{{"gravity", 9.81}, {"answer", 42.0}};
    FP f("gravity + answer", cm);
    CHECK(f.evaluate() == Approx(9.81 + 42.0));
}

// ---------------------------------------------------------------------------
// Unary functions

TEST_CASE("GFormulaParserT: unary math functions",
          "[common][formula-parser]") {
    CHECK(FP("sqrt(16)").evaluate() == Approx(4.0));
    CHECK(FP("fabs(-7)").evaluate() == Approx(7.0));
    CHECK(FP("log(e)").evaluate()   == Approx(1.0));
    CHECK(FP("exp(0)").evaluate()   == Approx(1.0));
    CHECK(FP("sin(0)").evaluate()   == Approx(0.0));
    CHECK(FP("cos(0)").evaluate()   == Approx(1.0));
    CHECK(FP("floor(3.7)").evaluate() == Approx(3.0));
    CHECK(FP("ceil(3.2)").evaluate()  == Approx(4.0));
}

// ---------------------------------------------------------------------------
// Binary functions

TEST_CASE("GFormulaParserT: binary math functions",
          "[common][formula-parser]") {
    CHECK(FP("pow(2,10)").evaluate()  == Approx(1024.0));
    CHECK(FP("min(3,7)").evaluate()   == Approx(3.0));
    CHECK(FP("max(3,7)").evaluate()   == Approx(7.0));
    CHECK(FP("hypot(3,4)").evaluate() == Approx(5.0));
}

// ---------------------------------------------------------------------------
// Placeholder replacement via parameter_map

TEST_CASE("GFormulaParserT: parameter_map replaces single-value placeholders",
          "[common][formula-parser]") {
    FP f("{{x}} * 2 + {{y}}");
    FP::parameter_map pm{{"x", {3.0}}, {"y", {7.0}}};
    CHECK(f.evaluate(pm) == Approx(13.0));
}

TEST_CASE("GFormulaParserT::getFormula returns the formula with placeholders substituted",
          "[common][formula-parser]") {
    FP f("{{a}} + 1");
    FP::parameter_map pm{{"a", {5.0}}};
    auto s = f.getFormula(pm);
    CHECK(s.find("5") != std::string::npos);
    CHECK(s.find("{{a}}") == std::string::npos);
}

// ---------------------------------------------------------------------------
// Errors

TEST_CASE("GFormulaParserT: malformed expression triggers a geneva_exception on evaluate",
          "[common][formula-parser]") {
    FP f("1 +");
    CHECK_THROWS_AS(f.evaluate(), geneva_exception);
}

TEST_CASE("GFormulaParserT: unknown identifier triggers an exception",
          "[common][formula-parser]") {
    FP f("unknown_var + 1");
    CHECK_THROWS_AS(f.evaluate(), geneva_exception);
}

// ---------------------------------------------------------------------------
// setPrintCode is a setter — exercise the path so it isn't dead code.

TEST_CASE("GFormulaParserT::setPrintCode toggles diagnostic printing",
          "[common][formula-parser]") {
    FP f("1+2");
    CHECK_NOTHROW(f.setPrintCode(true));
    CHECK_NOTHROW(f.setPrintCode(false));
}

// ---------------------------------------------------------------------------
// Math-logic errors — these exercise the GFormulaParserT.cpp exception ctors
// (math_logic_error / division_by_0 / ...).

TEST_CASE("GFormulaParserT: division by zero throws math_logic_error",
          "[common][formula-parser][math-error]") {
    FP f("1.0 / 0.0");
    // math_logic_error is a geneva_exception subclass.
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

TEST_CASE("GFormulaParserT: division_by_0 is also catchable as geneva_exception",
          "[common][formula-parser][math-error]") {
    FP f("(2+3) / (1-1)");
    CHECK_THROWS_AS(f.evaluate(), geneva_exception);
}

TEST_CASE("GFormulaParserT: log of zero throws",
          "[common][formula-parser][math-error]") {
    FP f("log(0.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

TEST_CASE("GFormulaParserT: log of negative throws",
          "[common][formula-parser][math-error]") {
    FP f("log(-1.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

TEST_CASE("GFormulaParserT: log10 of zero throws",
          "[common][formula-parser][math-error]") {
    FP f("log10(0.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

TEST_CASE("GFormulaParserT: sqrt of negative throws",
          "[common][formula-parser][math-error]") {
    FP f("sqrt(-1.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

TEST_CASE("GFormulaParserT: acos outside [-1, 1] throws",
          "[common][formula-parser][math-error]") {
    FP f("acos(2.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

TEST_CASE("GFormulaParserT: asin outside [-1, 1] throws",
          "[common][formula-parser][math-error]") {
    FP f("asin(-2.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

// ---------------------------------------------------------------------------
// Deeply nested + alternative-laden formulas — exercise grammar paths that
// require Boost.Spirit backtracking; the swap() helpers in GFormulaParserT.cpp
// are needed by the AST-attribute machinery on the failure-then-retry path.

TEST_CASE("GFormulaParserT: deeply nested expression evaluates correctly",
          "[common][formula-parser][nesting]") {
    FP f("((1+2)*(3+(4-(5*6)/3))+sqrt(16))-fabs(-3)");
    // Spelled out: (1+2)=3; inner ((5*6)/3)=10; (4-10)=-6; (3-6)=-3; 3*(-3)=-9;
    // sqrt(16)=4; -9+4=-5; fabs(-3)=3; -5-3=-8
    CHECK(f.evaluate() == Approx(-8.0));
}

TEST_CASE("GFormulaParserT: mixed unary/binary/constant formula",
          "[common][formula-parser][nesting]") {
    FP f("pow(2, log(e)) + min(1, max(2, 3)) - hypot(3, 4)");
    // log(e)=1; pow(2,1)=2; max(2,3)=3; min(1,3)=1; hypot(3,4)=5; 2+1-5=-2
    CHECK(f.evaluate() == Approx(-2.0));
}

TEST_CASE("GFormulaParserT: signed factor in nested context",
          "[common][formula-parser][nesting]") {
    FP f("-(-(-(2+3)))");   // three nested unary minuses → -(2+3) = -5
    CHECK(f.evaluate() == Approx(-5.0));
}

TEST_CASE("GFormulaParserT: placeholder inside nested arithmetic",
          "[common][formula-parser][nesting]") {
    FP f("sqrt(pow({{x}}, 2) + pow({{y}}, 2))");
    FP::parameter_map pm{{"x", {3.0}}, {"y", {4.0}}};
    CHECK(f.evaluate(pm) == Approx(5.0));
}

// ---------------------------------------------------------------------------
// Indexed placeholder arrays — {{key[i]}} syntax

TEST_CASE("GFormulaParserT: indexed placeholder replaces the correct element",
          "[common][formula-parser]") {
    FP f("{{coords[0]}} + {{coords[1]}} + {{coords[2]}}");
    FP::parameter_map pm{{"coords", {1.0, 2.0, 3.0}}};
    CHECK(f.evaluate(pm) == Approx(6.0));
}

TEST_CASE("GFormulaParserT: indexed placeholders can skip indices",
          "[common][formula-parser]") {
    FP f("{{v[1]}} * {{v[3]}}");
    FP::parameter_map pm{{"v", {10.0, 20.0, 30.0, 40.0}}};
    CHECK(f.evaluate(pm) == Approx(800.0));
}

// ---------------------------------------------------------------------------
// Hyperbolic and atan functions (previously untested execution paths)

TEST_CASE("GFormulaParserT: atan unary function",
          "[common][formula-parser]") {
    CHECK(FP("atan(1)").evaluate() == Approx(std::atan(1.0)));
    CHECK(FP("atan(0)").evaluate() == Approx(0.0));
}

TEST_CASE("GFormulaParserT: sinh, cosh, tanh unary functions",
          "[common][formula-parser]") {
    CHECK(FP("sinh(0)").evaluate() == Approx(0.0));
    CHECK(FP("cosh(0)").evaluate() == Approx(1.0));
    CHECK(FP("tanh(0)").evaluate() == Approx(0.0));
}

// ---------------------------------------------------------------------------
// log10 of a negative value (not covered by the existing log10(0) test)

TEST_CASE("GFormulaParserT: log10 of a negative value throws math_logic_error",
          "[common][formula-parser][math-error]") {
    FP f("log10(-2.0)");
    CHECK_THROWS_AS(f.evaluate(), Gem::Common::math_logic_error);
}

// ---------------------------------------------------------------------------
// acos / asin at their valid boundary values (should NOT throw)

TEST_CASE("GFormulaParserT: acos at valid boundaries does not throw",
          "[common][formula-parser]") {
    CHECK_NOTHROW(FP("acos(-1.0)").evaluate());
    CHECK_NOTHROW(FP("acos(1.0)").evaluate());
    CHECK(FP("acos(1.0)").evaluate() == Approx(0.0));
    CHECK(FP("acos(-1.0)").evaluate() == Approx(std::numbers::pi_v<double>));
}

TEST_CASE("GFormulaParserT: asin at valid boundaries does not throw",
          "[common][formula-parser]") {
    CHECK_NOTHROW(FP("asin(-1.0)").evaluate());
    CHECK_NOTHROW(FP("asin(1.0)").evaluate());
    CHECK(FP("asin(1.0)").evaluate()  == Approx( std::numbers::pi_v<double> / 2.0));
    CHECK(FP("asin(-1.0)").evaluate() == Approx(-std::numbers::pi_v<double> / 2.0));
}

// ---------------------------------------------------------------------------
// Empty vector in parameter map must throw

TEST_CASE("GFormulaParserT: empty vector in parameter map throws",
          "[common][formula-parser]") {
    FP f("{{x}}");
    FP::parameter_map pm{{"x", {}}};
    CHECK_THROWS_AS(f.evaluate(pm), geneva_exception);
}
