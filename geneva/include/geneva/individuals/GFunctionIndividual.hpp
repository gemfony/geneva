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

// Standard header files go here
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GParserBuilder.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/par/GOptimizableEntityFactory.hpp"
#include "geneva/par/GOptimizableEntityMultiConstraint.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace Gem::Geneva::OptimizationAlgorithms

namespace Gem::Geneva::Individuals {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Enumerates all available benchmark test functions.
 *
 * The functions span a range of difficulty classes, landscape topologies, and
 * algorithmic challenges, making them suitable for systematic algorithm benchmarking
 * across multiple dimensions. Each entry documents its landscape properties and the
 * class of algorithmic weakness it is designed to expose.
 *
 * Unless stated otherwise, all functions are defined for n ≥ 1 dimensions.
 * Recommended search domains per function are documented individually below.
 */
enum class solverFunction : Gem::Common::ENUMBASETYPE {
    /**
	 * Simple n-dimensional parabola: f(x) = Σxᵢ².
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: any (default [-10,10]).
	 * Unimodal, convex, separable. Serves as a baseline sanity check to verify
	 * that any algorithm can locate a trivially placed, convex global optimum.
	 * Performance on this function establishes the lower bound of expected runtime.
	 */
    PARABOLA = 0,

    /**
	 * Berlich noisy parabola: f(x) = (cos(‖x‖²)+2)·‖x‖².
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-4π, 4π].
	 * Multimodal, radially symmetric, non-separable. The cosine overlay creates
	 * a dense ring structure of local optima around the origin. Tests whether
	 * an algorithm can escape near-origin local optima while the overall
	 * gradient-free global structure remains simple (decreasing with ‖x‖).
	 */
    NOISYPARABOLA = 1,

    /**
	 * Generalized Rosenbrock function: f(x) = Σ[100(xᵢ₊₁-xᵢ²)²+(1-xᵢ)²].
	 * Requires n ≥ 2. Global minimum: f=0 at x=(1,...,1). Domain: [-2, 2].
	 * Unimodal for n≤3, strongly non-separable, with a narrow, curved banana-shaped
	 * valley. The gradient along the valley floor is nearly zero, making gradient
	 * descent slow and gradient-free methods prone to overshooting. A classic
	 * benchmark for non-separable slow-convergence scenarios.
	 */
    ROSENBROCK = 2,

    /**
	 * Modified Ackley variant (Geneva-specific, pairwise sum form).
	 * f(x) = Σ[exp(-0.2)·√(xᵢ²+xᵢ₊₁²) + 3·(cos(2xᵢ)+sin(2xᵢ₊₁))].
	 * Requires n ≥ 2. Non-separable, multimodal.
	 * NOTE: This is NOT the canonical Ackley function. It accumulates n-1 pairwise
	 * terms and has a different, numerically approximated global minimum location.
	 * Retained for backward compatibility with existing configurations.
	 * For the standard CEC/BBOB benchmark formulation, use ACKLEY_CANONICAL (8).
	 */
    ACKLEY = 3,

    /**
	 * Rastrigin function: f(x) = 10n + Σ[xᵢ²-10·cos(2πxᵢ)].
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-5.12, 5.12].
	 * Highly multimodal, separable. Contains approximately 10ⁿ regularly spaced
	 * local minima arranged in a regular grid, all of similar depth. Each dimension
	 * can in principle be treated independently, but the density of minima makes
	 * high-dimensional instances very challenging for population-based methods.
	 * A standard benchmark for multimodal optimisation in evolutionary computation.
	 */
    RASTRIGIN = 4,

    /**
	 * Schwefel function: f(x) = -1/n · Σ xᵢ·sin(√|xᵢ|).
	 * Global minimum: f≈-418.9829/n at xᵢ≈420.9687. Recommended domain: [-500, 500].
	 * NOTE: Geneva normalises by 1/n; the standard formulation does not.
	 * Deceptive: the global optimum lies far from the origin and near the boundary
	 * of the search space. Secondary optima cluster near the origin, attracting
	 * population members that initialise or drift inward. Tests resistance to
	 * deceptive attractor basins and global exploration far from the initialisation.
	 */
    SCHWEFEL = 5,

    /**
	 * Salomon function: f(x) = -cos(2π‖x‖) + 0.1·‖x‖ + 1.
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-100, 100].
	 * Multimodal, radially symmetric, non-separable. Creates concentric spherical
	 * shells of local optima at ‖x‖ = k for integer k. The very gradual linear
	 * increase of 0.1·‖x‖ provides weak directional guidance. Tests the ability
	 * to follow a radial gradient in the presence of strong perpendicular
	 * oscillations across the shells.
	 */
    SALOMON = 6,

    /**
	 * Negative parabola: f(x) = -Σxᵢ².
	 * Global maximum: f=0 at x=(0,...,0). Recommended domain: any.
	 * Used exclusively for maximisation tests to verify that Geneva's internal
	 * maximisation mode (Go2 maxMode) functions correctly. The
	 * landscape is identical to PARABOLA but sign-inverted.
	 */
    NEGPARABOLA = 7,

    /**
	 * Canonical n-dimensional Ackley function.
	 * f(x) = -20·exp(-0.2·√(1/n·Σxᵢ²)) - exp(1/n·Σcos(2πxᵢ)) + 20 + e.
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-32.768, 32.768].
	 * Non-separable, multimodal. The outer region of the landscape is almost flat
	 * (nearly zero gradient for large ‖x‖), suddenly dropping into a narrow global
	 * basin at the origin. Gradient-based methods stall on the plateau; stochastic
	 * and population-based methods are generally more effective. The canonical
	 * formulation used in all CEC and BBOB benchmark suites.
	 */
    ACKLEY_CANONICAL = 8,

    /**
	 * Griewank function: f(x) = 1/4000·Σxᵢ² - Πcos(xᵢ/√i) + 1.
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-600, 600].
	 * Weakly non-separable (the product term introduces cross-dimension coupling),
	 * multimodal. At large scale the landscape is nearly quadratic; the product
	 * of cosines creates a fine-grained multimodal structure at smaller scale.
	 * With growing dimension the local minima become denser but the global basin
	 * remains identifiable through the quadratic envelope. Distinguishes algorithms
	 * that exploit global structure from purely local explorers.
	 */
    GRIEWANK = 9,

    /**
	 * Lévy function.
	 * f(x) = sin²(πw₁) + Σᵢ₌₁ⁿ⁻¹[(wᵢ-1)²(1+10sin²(πwᵢ₊₁))] + (wₙ-1)²(1+sin²(2πwₙ)),
	 * with wᵢ = 1+(xᵢ-1)/4.
	 * Global minimum: f=0 at x=(1,...,1). Recommended domain: [-10, 10].
	 * Separable, multimodal. The regular sin²-based ripple structure creates narrow,
	 * closely spaced local minima around each dimension's valley. Tests fine-grained
	 * local search precision: the algorithm must resolve the global minimum from its
	 * immediate neighbours, which become progressively closer in value as dimension
	 * grows. Widely used in the IEEE CEC benchmark suite.
	 */
    LEVY = 10,

    /**
	 * Styblinski-Tang function: f(x) = 1/2·Σ(xᵢ⁴-16xᵢ²+5xᵢ).
	 * Global minimum: f≈-39.166·n at xᵢ≈-2.9035. Recommended domain: [-5, 5].
	 * Separable, multimodal, asymmetric. Each dimension has two local minima at
	 * different depths (≈-39.17 at x≈-2.90 and ≈-21.25 at x≈+2.75). Because the
	 * global minimum is not located at the origin, symmetric Gaussian mutation
	 * centred on the initial population introduces a systematic bias against the
	 * global basin. This function exposes initialisation and mutation symmetry
	 * artefacts that PARABOLA and RASTRIGIN cannot reveal.
	 */
    STYBLINSKI_TANG = 11,

    /**
	 * Axis-parallel Ellipsoid function: f(x) = Σ 10^(6i/(n-1))·xᵢ².
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-5, 5].
	 * Unimodal, separable, condition number 10⁶ (ratio of largest to smallest
	 * Hessian eigenvalue). All algorithms eventually find the minimum; the
	 * discriminating metric is convergence speed and final precision. Algorithms
	 * with isotropic mutation (uniform σ for all dimensions) converge slowly
	 * because x₀ requires very large steps while xₙ₋₁ requires very small steps.
	 * Tests the effectiveness of Geneva's self-adaptive σ mechanisms in the EA.
	 */
    ELLIPSOID = 12,

    /**
	 * Michalewicz function: f(x) = -Σ sin(xᵢ)·sin²ᵐ(i·xᵢ²/π), with m=10.
	 * Global minimum: dimension-dependent, not analytically known
	 * (≈-1.8013 for n=2, ≈-4.6877 for n=5, ≈-9.660 for n=10).
	 * Recommended domain: [0, π].
	 * NOTE: Unlike all other functions, the natural domain is [0, π].
	 * Set min_var=0 and max_var≈3.14159 explicitly in the factory configuration.
	 * The exponent m=10 creates extremely narrow ridges. The global minimum lies
	 * inside a steep, razor-thin valley; approaching it requires precise alignment
	 * of the search direction. Tests fine-grained local search and the ability to
	 * follow narrow ridges reliably. The analytically unknown optimum also makes
	 * this function useful for benchmarking solution quality across releases.
	 */
    MICHALEWICZ = 13,

    /**
	 * Zakharov function: f(x) = Σxᵢ² + (Σ0.5·i·xᵢ)² + (Σ0.5·i·xᵢ)⁴.
	 * Global minimum: f=0 at x=(0,...,0). Recommended domain: [-5, 10].
	 * Unimodal, non-separable. The quadratic and quartic terms of the weighted
	 * linear combination 0.5·Σi·xᵢ introduce dimension-weighted interactions:
	 * later dimensions (large index i) are penalised more strongly, creating an
	 * asymmetric, non-separable bowl. Gradient descent handles this well; gradient-
	 * free methods must adapt step sizes per dimension. Tests non-separable
	 * interaction without the confounding effect of multimodality.
	 */
    ZAKHAROV = 14
};

const solverFunction MAXDEMOFUNCTION = solverFunction::ZAKHAROV;

// Make sure solverFunction can be streamed
/** @brief Puts a Gem::Geneva::Individuals::solverFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::Individuals::solverFunction &);

/** @brief Reads a Gem::Geneva::Individuals::solverFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream &operator>>(std::istream &, Gem::Geneva::Individuals::solverFunction &);

/**
 * This enum describes different parameter types that may be used to fill the object with data
 */
enum class parameterType : Gem::Common::ENUMBASETYPE {
    USEGDOUBLECOLLECTION = 0,
    USEGCONSTRAINEDOUBLECOLLECTION = 1,
    USEGDOUBLEOBJECTCOLLECTION = 2,
    USEGCONSTRAINEDDOUBLEOBJECTCOLLECTION = 3,
    USEGCONSTRAINEDDOUBLEOBJECT = 4
};

// Make sure parameterType can be streamed
/** @brief Puts a Gem::Geneva::Individuals::parameterType into a stream. Needed also for boost::lexical_cast<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::Individuals::parameterType &);

/** @brief Reads a Gem::Geneva::Individuals::parameterType from a stream. Needed also for boost::lexical_cast<> */
std::istream &operator>>(std::istream &, Gem::Geneva::Individuals::parameterType &);

/**
 * This enum describes several ways of initializing the data collections
 */
enum class initMode : Gem::Common::ENUMBASETYPE {
    INITRANDOM = 0 // random values for all variables
        ,
    INITPERIMETER = 1 // Uses a parameter set on the perimeter of the allowed or common value range
};

// Make sure initMode can be streamed
/** @brief Puts a Gem::Geneva::Individuals::initMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::Individuals::initMode &);

/** @brief Reads a Gem::Geneva::Individuals::initMode from a stream. Needed also for boost::lexical_cast<> */
std::istream &operator>>(std::istream &, Gem::Geneva::Individuals::initMode &);

/******************************************************************************/
// A number of default settings for the factory
constexpr double GFI_DEF_ADPROB = 1.0;
constexpr double GFI_DEF_ADAPTADPROB = 0.1;
constexpr double GFI_DEF_MINADPROB = 0.05;
constexpr double GFI_DEF_MAXADPROB = 1.;
constexpr std::uint32_t GFI_DEF_ADAPTIONTHRESHOLD = 1;
constexpr bool GFI_DEF_USEBIGAUSSIAN = false;
constexpr double GFI_DEF_SIGMA1 = 0.025;
constexpr double GFI_DEF_SIGMASIGMA1 = 0.2;
constexpr double GFI_DEF_MINSIGMA1 = 0.001;
constexpr double GFI_DEF_MAXSIGMA1 = 1;
constexpr double GFI_DEF_SIGMA2 = 0.025;
constexpr double GFI_DEF_SIGMASIGMA2 = 0.2;
constexpr double GFI_DEF_MINSIGMA2 = 0.001;
constexpr double GFI_DEF_MAXSIGMA2 = 1;
constexpr double GFI_DEF_DELTA = 0.05;
constexpr double GFI_DEF_SIGMADELTA = 0.2;
constexpr double GFI_DEF_MINDELTA = 0.001;
constexpr double GFI_DEF_MAXDELTA = 1.;
constexpr std::size_t GFI_DEF_PARDIM = 2;
constexpr double GFI_DEF_MINVAR = -10.;
constexpr double GFI_DEF_MAXVAR = 10.;
constexpr bool GFI_DEF_USECONSTRAINEDDOUBLECOLLECTION = false;
const parameterType GFI_DEF_PARAMETERTYPE = parameterType::USEGCONSTRAINEDDOUBLEOBJECT;
const initMode GFI_DEF_INITMODE = initMode::INITPERIMETER;
const solverFunction GO_DEF_EVALFUNCTION = solverFunction::PARABOLA;
constexpr double GFI_DEF_CROSSOVERPROB = 0.5;

/******************************************************************************/
// Forward declaration
class GFunctionIndividualFactory;

/******************************************************************************/
/**
 * @brief An individual that evaluates one of several standard benchmark test functions.
 *
 * GFunctionIndividual is the standard benchmark vehicle for Geneva's optimisation algorithms.
 * It supports 15 test functions (solverFunction enum, IDs 0–14) covering unimodal, multimodal,
 * separable, non-separable, ill-conditioned, deceptive, and asymmetric landscapes. The active
 * function is selected via setDemoFunction() or through the factory configuration file.
 *
 * All functions accept arbitrary parameter dimensionality n ≥ 1 (some require n ≥ 2).
 * The factory (GFunctionIndividualFactory) populates the individual with n GConstrainedDoubleObject
 * parameters within [min_var, max_var]; these bounds should match the recommended domain of the
 * selected function (see solverFunction enum documentation).
 *
 * @note For MICHALEWICZ the natural domain is [0, π]. Set min_var=0 and max_var≈3.14159
 *       explicitly; the factory default of [-10, 10] is not suitable for that function.
 */
class GFunctionIndividual
  : public gpar::GFlatGenome // NOLINT(cppcoreguidelines-special-member-functions)
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GFlatGenome) &
            BOOST_SERIALIZATION_NVP(demo_function_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    using FACTORYTYPE = GFunctionIndividualFactory;

    /** @brief The default constructor */
    GFunctionIndividual() = default;
    /** @brief Initialization with the desired demo function */
    explicit GFunctionIndividual(const solverFunction &);
    /** @brief A standard copy constructor */
    GFunctionIndividual(const GFunctionIndividual &cp) = default;

    /** @brief The standard destructor */
    ~GFunctionIndividual() override = default;

    /** @brief Allows external entities to set the fitness */
    void setFitness(std::vector<double> const &);

    /** @brief Allows to set the demo function */
    void setDemoFunction(solverFunction);
    /** @brief Allows to retrieve the current demo function */
    solverFunction getDemoFunction() const;

    /** @brief Allows to cross check the parameter size */
    std::size_t getParameterSize() const;

    //---------------------------------------------------------------------------
    /**
	  * @brief Converts a solverFunction id to a human-readable name string.
	  *
	  * Used primarily for plot labels in GOptimizationBenchmark and similar tools.
	  * The returned string matches the conventional name of the function in the
	  * evolutionary computation literature.
	  *
	  * | ID | Enum                | String                       |
	  * |----|---------------------|------------------------------|
	  * |  0 | PARABOLA            | "Parabola"                   |
	  * |  1 | NOISYPARABOLA       | "Berlich noisy parabola"     |
	  * |  2 | ROSENBROCK          | "Rosenbrock"                 |
	  * |  3 | ACKLEY              | "Ackley (pairwise variant)"  |
	  * |  4 | RASTRIGIN           | "Rastrigin"                  |
	  * |  5 | SCHWEFEL            | "Schwefel"                   |
	  * |  6 | SALOMON             | "Salomon"                    |
	  * |  7 | NEGPARABOLA         | "Negative parabola"          |
	  * |  8 | ACKLEY_CANONICAL    | "Ackley (canonical)"         |
	  * |  9 | GRIEWANK            | "Griewank"                   |
	  * | 10 | LEVY                | "Levy"                       |
	  * | 11 | STYBLINSKI_TANG     | "Styblinski-Tang"            |
	  * | 12 | ELLIPSOID           | "Ellipsoid"                  |
	  * | 13 | MICHALEWICZ         | "Michalewicz (m=10)"         |
	  * | 14 | ZAKHAROV            | "Zakharov"                   |
	  *
	  * @param df The solverFunction identifier
	  * @return Human-readable name of the function
	  */
    static std::string getStringRepresentation(const solverFunction &df) {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        switch(df) {
        case solverFunction::PARABOLA:
            result = "Parabola";
            break;
        case solverFunction::NOISYPARABOLA:
            result = "Berlich noisy parabola";
            break;
        case solverFunction::ROSENBROCK:
            result = "Rosenbrock";
            break;
        case solverFunction::ACKLEY:
            result = "Ackley (pairwise variant)";
            break;
        case solverFunction::RASTRIGIN:
            result = "Rastrigin";
            break;
        case solverFunction::SCHWEFEL:
            result = "Schwefel";
            break;
        case solverFunction::SALOMON:
            result = "Salomon";
            break;
        case solverFunction::NEGPARABOLA:
            result = "Negative parabola";
            break;
        case solverFunction::ACKLEY_CANONICAL:
            result = "Ackley (canonical)";
            break;
        case solverFunction::GRIEWANK:
            result = "Griewank";
            break;
        case solverFunction::LEVY:
            result = "Levy";
            break;
        case solverFunction::STYBLINSKI_TANG:
            result = "Styblinski-Tang";
            break;
        case solverFunction::ELLIPSOID:
            result = "Ellipsoid";
            break;
        case solverFunction::MICHALEWICZ:
            result = "Michalewicz (m=10)";
            break;
        case solverFunction::ZAKHAROV:
            result = "Zakharov";
            break;
        }

        return result;
    }

    //---------------------------------------------------------------------------
    /**
	  * @brief Returns the 2D version of a function as a ROOT TFormula-compatible string.
	  *
	  * The returned string is suitable for use with ROOT's TF2 class
	  * (see https://root.cern.ch) and can be passed directly to GPlotDesigner.
	  * All formulas use ROOT syntax: "pi" for π, "exp(1.)" for e, "^" for power,
	  * and "sqrt"/"abs" for the respective standard functions.
	  *
	  * @param df The solverFunction identifier
	  * @return ROOT TFormula string for the 2D (n=2) version of the function
	  */
    static std::string get2DROOTFunction(const solverFunction &df) {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        switch(df) {
        case solverFunction::PARABOLA:
            result = "x^2+y^2";
            break;
        case solverFunction::NOISYPARABOLA:
            result = "(cos(x^2+y^2)+2.)*(x^2+y^2)";
            break;
        case solverFunction::ROSENBROCK:
            result = "100.*(x^2-y)^2+(1.-x)^2";
            break;
        case solverFunction::ACKLEY:
            result = "exp(-0.2)*sqrt(x^2+y^2)+3.*(cos(2.*x)+sin(2.*y))";
            break;
        case solverFunction::RASTRIGIN:
            result = "20.+(x^2-10.*cos(2.*pi*x))+(y^2-10.*cos(2.*pi*y))";
            break;
        case solverFunction::SCHWEFEL:
            result = "-0.5*(x*sin(sqrt(abs(x)))+y*sin(sqrt(abs(y))))";
            break;
        case solverFunction::SALOMON:
            result = "-cos(2.*pi*sqrt(x^2+y^2))+0.1*sqrt(x^2+y^2)+1.";
            break;
        case solverFunction::NEGPARABOLA:
            result = "-(x^2+y^2)";
            break;
        case solverFunction::ACKLEY_CANONICAL:
            result =
                "-20.*exp(-0.2*sqrt((x^2+y^2)/2.))-exp((cos(2.*pi*x)+cos(2.*pi*y))/2.)+20.+exp(1.)";
            break;
        case solverFunction::GRIEWANK:
            result = "(x^2+y^2)/4000.-cos(x)*cos(y/sqrt(2.))+1.";
            break;
        case solverFunction::LEVY:
            result = "sin(pi*(1.+0.25*(x-1.)))^2+((0.25*(x-1.))^2)*(1.+10.*sin(pi*(1.+0.25*(y-1.)))"
                     "^2)+((0.25*(y-1.))^2)*(1.+sin(2.*pi*(1.+0.25*(y-1.)))^2)";
            break;
        case solverFunction::STYBLINSKI_TANG:
            result = "0.5*(x^4-16.*x^2+5.*x+y^4-16.*y^2+5.*y)";
            break;
        case solverFunction::ELLIPSOID:
            result = "x^2+1000000.*y^2";
            break;
        case solverFunction::MICHALEWICZ:
            result = "-sin(x)*pow(sin(x^2/pi),20.)-sin(y)*pow(sin(2.*y^2/pi),20.)";
            break;
        case solverFunction::ZAKHAROV:
            result = "x^2+y^2+(0.5*x+y)^2+(0.5*x+y)^4";
            break;
        }

        return result;
    }

    //---------------------------------------------------------------------------
    /**
	  * @brief Returns the x-coordinate(s) of the global optimum for the 2D version of a function.
	  *
	  * Used to annotate plots produced by GFitnessMonitor and GOptimizationBenchmark.
	  * Multiple values are returned only when the function has more than one global
	  * optimum in 2D. Coordinates are for the first parameter (x-axis in 2D plots).
	  *
	  * For MICHALEWICZ the global minimum location is known only approximately.
	  * For SCHWEFEL each dimension's optimum is at ≈420.9687; Geneva normalises
	  * by n so the function value at the optimum is ≈-418.9829/n.
	  *
	  * @param df The solverFunction identifier
	  * @return x-coordinate(s) of the global optimum in 2D
	  */
    static std::vector<double> getXMin(const solverFunction &df) {
        std::vector<double> result;

        switch(df) {
        case solverFunction::PARABOLA:
            result.push_back(0.);
            break;
        case solverFunction::NOISYPARABOLA:
            result.push_back(0.);
            break;
        case solverFunction::ROSENBROCK:
            result.push_back(1.);
            break;
        case solverFunction::ACKLEY:
            // Pairwise-variant: two numerically determined global optima in 2D
            result.push_back(-1.5096201);
            result.push_back(1.5096201);
            break;
        case solverFunction::RASTRIGIN:
            result.push_back(0.);
            break;
        case solverFunction::SCHWEFEL:
            result.push_back(420.968746);
            break;
        case solverFunction::SALOMON:
            result.push_back(0.);
            break;
        case solverFunction::NEGPARABOLA:
            result.push_back(0.);
            break;
        case solverFunction::ACKLEY_CANONICAL:
            result.push_back(0.);
            break;
        case solverFunction::GRIEWANK:
            result.push_back(0.);
            break;
        case solverFunction::LEVY:
            result.push_back(1.);
            break;
        case solverFunction::STYBLINSKI_TANG:
            result.push_back(-2.903534);
            break;
        case solverFunction::ELLIPSOID:
            result.push_back(0.);
            break;
        case solverFunction::MICHALEWICZ:
            // Approximate; exact value not analytically known
            result.push_back(2.2029);
            break;
        case solverFunction::ZAKHAROV:
            result.push_back(0.);
            break;
        }

        return result;
    }

    //---------------------------------------------------------------------------
    /**
	  * @brief Returns the y-coordinate(s) of the global optimum for the 2D version of a function.
	  *
	  * Used to annotate plots produced by GFitnessMonitor and GOptimizationBenchmark.
	  * Coordinates are for the second parameter (y-axis in 2D plots). For functions
	  * with a single global optimum this returns a single value; the ACKLEY pairwise
	  * variant has one numerically determined y-coordinate for its 2D optimum.
	  *
	  * @param df The solverFunction identifier
	  * @return y-coordinate(s) of the global optimum in 2D
	  */
    static std::vector<double> getYMin(const solverFunction &df) {
        std::vector<double> result;

        switch(df) {
        case solverFunction::PARABOLA:
            result.push_back(0.);
            break;
        case solverFunction::NOISYPARABOLA:
            result.push_back(0.);
            break;
        case solverFunction::ROSENBROCK:
            result.push_back(1.);
            break;
        case solverFunction::ACKLEY:
            // Pairwise-variant: numerically determined y-coordinate of 2D optimum
            result.push_back(-0.7548651);
            break;
        case solverFunction::RASTRIGIN:
            result.push_back(0.);
            break;
        case solverFunction::SCHWEFEL:
            result.push_back(420.968746);
            break;
        case solverFunction::SALOMON:
            result.push_back(0.);
            break;
        case solverFunction::NEGPARABOLA:
            result.push_back(0.);
            break;
        case solverFunction::ACKLEY_CANONICAL:
            result.push_back(0.);
            break;
        case solverFunction::GRIEWANK:
            result.push_back(0.);
            break;
        case solverFunction::LEVY:
            result.push_back(1.);
            break;
        case solverFunction::STYBLINSKI_TANG:
            result.push_back(-2.903534);
            break;
        case solverFunction::ELLIPSOID:
            result.push_back(0.);
            break;
        case solverFunction::MICHALEWICZ:
            // Approximate; exact value not analytically known
            result.push_back(1.5708);
            break;
        case solverFunction::ZAKHAROV:
            result.push_back(0.);
            break;
        }

        return result;
    }

protected:
    //---------------------------------------------------------------------------
    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(Gem::Common::make_member("demo_function_", demo_function_));
    }
    auto localMembers() const {
        return std::make_tuple(Gem::Common::make_member("demo_function_", demo_function_));
    }

    /** @brief Loads the data of another GFunctionIndividual */
    void load_(const gpar::GOptimizableEntity *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFunctionIndividual>(
        GFunctionIndividual const &,
        GFunctionIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const gpar::GOptimizableEntity & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual value calculation takes place here */
    double fitnessCalculation() final;

    //---------------------------------------------------------------------------

    /** @brief Applies modifications to this object. */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    //---------------------------------------------------------------------------
    /** @brief Creates a deep clone of this object */
    gpar::GFlatGenome *clone_() const final;

    //---------------------------------------------------------------------------
    // Data

    solverFunction demo_function_ =
        solverFunction::PARABOLA; ///< Specifies which demo function should be used
};

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream &
operator<<(std::ostream &, const Gem::Geneva::Individuals::GFunctionIndividual &);

std::ostream &
operator<<(std::ostream &, std::shared_ptr<Gem::Geneva::Individuals::GFunctionIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFunctionIndividual objects
 */
class GFunctionIndividualFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public gpar::GOptimizableEntityFactory {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GOptimizableEntityFactory) &
            BOOST_SERIALIZATION_NVP(ad_prob_) & BOOST_SERIALIZATION_NVP(adapt_ad_prob_) &
            BOOST_SERIALIZATION_NVP(min_ad_prob_) & BOOST_SERIALIZATION_NVP(max_ad_prob_) &
            BOOST_SERIALIZATION_NVP(adaption_threshold_) & BOOST_SERIALIZATION_NVP(use_bi_gaussian_) &
            BOOST_SERIALIZATION_NVP(sigma1_) & BOOST_SERIALIZATION_NVP(sigma_sigma1_) &
            BOOST_SERIALIZATION_NVP(min_sigma1_) & BOOST_SERIALIZATION_NVP(max_sigma1_) &
            BOOST_SERIALIZATION_NVP(sigma2_) & BOOST_SERIALIZATION_NVP(sigma_sigma2_) &
            BOOST_SERIALIZATION_NVP(min_sigma2_) & BOOST_SERIALIZATION_NVP(max_sigma2_) &
            BOOST_SERIALIZATION_NVP(delta_) & BOOST_SERIALIZATION_NVP(sigma_delta_) &
            BOOST_SERIALIZATION_NVP(min_delta_) & BOOST_SERIALIZATION_NVP(max_delta_) &
            BOOST_SERIALIZATION_NVP(par_dim_) & BOOST_SERIALIZATION_NVP(min_var_) &
            BOOST_SERIALIZATION_NVP(max_var_) & BOOST_SERIALIZATION_NVP(p_t_) &
            BOOST_SERIALIZATION_NVP(i_m_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    explicit GFunctionIndividualFactory(std::filesystem::path const &);
    /** @brief The copy constructor */
    GFunctionIndividualFactory(const GFunctionIndividualFactory &cp) = default;

    /** @brief The destructor */
    ~GFunctionIndividualFactory() override = default;

    /** @brief Builds the OA-owned adaption config for a genome produced by this factory: every double
     *  group gets the configured single-Gauss or bi-Gauss adaptor. The adaptor settings live on the
     *  OA-owned config (this factory's parameters), not in the structure-only genome layout. */
    std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
    getAdaptionConfig(const gpar::GFlatGenome &sample) const;

    //---------------------------------------------------------------------------
    // Getters and setters

    /** @brief Allows to retrieve the adaption_threshold_ variable */
    std::uint32_t getAdaptionThreshold() const;
    /** @brief Set the value of the adaption_threshold_ variable */
    void setAdaptionThreshold(std::uint32_t adaption_threshold);

    /** @brief Allows to retrieve the adProb_ variable */
    double getAdProb() const;
    /** @brief Set the value of the adProb_ variable */
    void setAdProb(double ad_prob);

    /** @brief Allows to retrieve the iM_ variable */
    initMode getIM() const;
    /** @brief Set the value of the iM_ variable */
    void setIM(initMode im);

    /** @brief Allows to retrieve the parDim_ variable */
    std::size_t getParDim() const;
    /** @brief (Re-)Set the dimension of the function */
    void setParDim(std::size_t);

    /** @brief Allows to retrieve the pT_ variable */
    parameterType getPT() const;
    /** @brief Set the value of the pT_ variable */
    void setPT(parameterType pt);

    /** @brief Allows to retrieve the use_bi_gaussian_ variable */
    bool getUseBiGaussian() const;
    /** @brief Set the value of the use_bi_gaussian_ variable */
    void setUseBiGaussian(bool use_bi_gaussian);

    /** @brief Allows to retrieve the minVar_ variable */
    double getMinVar() const;
    /** @brief Allows to retrieve the maxVar_ variable */
    double getMaxVar() const;
    /** @brief Extract the minimum and maximum boundaries of the variables */
    std::tuple<double, double> getVarBoundaries() const;
    /** @brief Set the minimum and maximum boundaries of the variables */
    void setVarBoundaries(std::tuple<double, double>);

    /** @brief Allows to retrieve the delta_ variable */
    double getDelta() const;
    /** @brief Set the value of the delta_ variable */
    void setDelta(double delta);
    /** @brief Allows to retrieve the min_delta_ variable */
    double getMinDelta() const;
    /** @brief Allows to retrieve the max_delta_ variable */
    double getMaxDelta() const;
    /** @brief Allows to retrieve the allowed value range of delta */
    std::tuple<double, double> getDeltaRange() const;
    /** @brief Allows to set the allowed value range of delta */
    void setDeltaRange(std::tuple<double, double>);

    /** @brief Allows to retrieve the min_sigma1_ variable */
    double getMinSigma1() const;
    /** @brief Allows to retrieve the max_sigma1_ variable */
    double getMaxSigma1() const;
    /** @brief Allows to retrieve the allowed value range of sigma1_ */
    std::tuple<double, double> getSigma1Range() const;
    /** @brief Allows to set the allowed value range of sigma1_ */
    void setSigma1Range(std::tuple<double, double>);

    /** @brief Allows to retrieve the min_sigma2_ variable */
    double getMinSigma2() const;
    /** @brief Allows to retrieve the max_sigma2_ variable */
    double getMaxSigma2() const;
    /** @brief Allows to retrieve the allowed value range of sigma2_ */
    std::tuple<double, double> getSigma2Range() const;
    /** @brief Allows to set the allowed value range of sigma2_ */
    void setSigma2Range(std::tuple<double, double>);

    /** @brief Allows to retrieve the sigma1_ variable */
    double getSigma1() const;
    /** @brief Set the value of the sigma1_ variable */
    void setSigma1(double sigma1);

    /** @brief Allows to retrieve the sigma2_ variable */
    double getSigma2() const;
    /** @brief Set the value of the sigma2_ variable */
    void setSigma2(double sigma2);

    /** @brief Allows to retrieve the sigma_delta_ variable */
    double getSigmaDelta() const;
    /** @brief Set the value of the sigma_delta_ variable */
    void setSigmaDelta(double sigma_delta);

    /** @brief Allows to retrieve the sigma_sigma1_ variable */
    double getSigmaSigma1() const;
    /** @brief Set the value of the sigma_sigma1_ variable */
    void setSigmaSigma1(double sigma_sigma1);

    /** @brief Allows to retrieve the sigma_sigma2_ variable */
    double getSigmaSigma2() const;
    /** @brief Set the value of the sigma_sigma2_ variable */
    void setSigmaSigma2(double sigma_sigma2);

    /** @brief Allows to retrieve the rate of evolutionary adaption of adProb_ */
    double getAdaptAdProb() const;
    /** @brief Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature) */
    void setAdaptAdProb(double adapt_ad_prob);

    /** @brief Allows to retrieve the allowed range for adProb_ variation */
    std::tuple<double, double> getAdProbRange() const;
    /** @brief Allows to set the allowed range for adaption probability variation */
    void setAdProbRange(double min_ad_prob, double max_ad_prob);

    // End of public getters and setters
    //--------------------------------------------------------------------------

    /** @brief Loads the data of another GFunctionIndividualFactory object */
    void load(std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>>) override;
    /** @brief Creates a deep clone of this object */
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> clone() const override;

protected:
    /** @brief Allows to describe local configuration options in derived classes */
    void describeLocalOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Allows to act on the configuration options received from the configuration file */
    void postProcess_(std::shared_ptr<gpar::GOptimizableEntity> &) override;

private:
    /** @brief Creates individuals of this type */
    std::shared_ptr<gpar::GOptimizableEntity>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override;

    /** @brief Set the value of the minVar_ variable */
    void setMinVar(double min_var);

    /** @brief Set the value of the maxVar_ variable */
    void setMaxVar(double max_var);

    /** @brief Set the value of the min_delta_ variable */
    void setMinDelta(double min_delta);

    /** @brief Set the value of the max_delta_ variable */
    void setMaxDelta(double max_delta);

    /** @brief Set the value of the min_sigma1_ variable */
    void setMinSigma1(double min_sigma1);

    /** @brief Set the value of the max_sigma1_ variable */
    void setMaxSigma1(double max_sigma1);

    /** @brief Set the value of the min_sigma2_ variable */
    void setMinSigma2(double min_sigma2);

    /** @brief Set the value of the max_sigma2_ variable */
    void setMaxSigma2(double max_sigma2);

    /** @brief The default constructor; Only needed for (de-)serialization purposes. */
    GFunctionIndividualFactory();

    Gem::Common::GOneTimeRefParameterT<double> ad_prob_{GFI_DEF_ADPROB};
    Gem::Common::GOneTimeRefParameterT<double> adapt_ad_prob_{GFI_DEF_ADAPTADPROB};
    Gem::Common::GOneTimeRefParameterT<double> min_ad_prob_{GFI_DEF_MINADPROB};
    Gem::Common::GOneTimeRefParameterT<double> max_ad_prob_{GFI_DEF_MAXADPROB};
    Gem::Common::GOneTimeRefParameterT<std::uint32_t> adaption_threshold_{GFI_DEF_ADAPTIONTHRESHOLD};
    Gem::Common::GOneTimeRefParameterT<bool> use_bi_gaussian_{GFI_DEF_USEBIGAUSSIAN};
    Gem::Common::GOneTimeRefParameterT<double> sigma1_{GFI_DEF_SIGMA1};
    Gem::Common::GOneTimeRefParameterT<double> sigma_sigma1_{GFI_DEF_SIGMASIGMA1};
    Gem::Common::GOneTimeRefParameterT<double> min_sigma1_{GFI_DEF_MINSIGMA1};
    Gem::Common::GOneTimeRefParameterT<double> max_sigma1_{GFI_DEF_MAXSIGMA1};
    Gem::Common::GOneTimeRefParameterT<double> sigma2_{GFI_DEF_SIGMA2};
    Gem::Common::GOneTimeRefParameterT<double> sigma_sigma2_{GFI_DEF_SIGMASIGMA2};
    Gem::Common::GOneTimeRefParameterT<double> min_sigma2_{GFI_DEF_MINSIGMA2};
    Gem::Common::GOneTimeRefParameterT<double> max_sigma2_{GFI_DEF_MAXSIGMA2};
    Gem::Common::GOneTimeRefParameterT<double> delta_{GFI_DEF_DELTA};
    Gem::Common::GOneTimeRefParameterT<double> sigma_delta_{GFI_DEF_SIGMADELTA};
    Gem::Common::GOneTimeRefParameterT<double> min_delta_{GFI_DEF_MINDELTA};
    Gem::Common::GOneTimeRefParameterT<double> max_delta_{GFI_DEF_MAXDELTA};
    Gem::Common::GOneTimeRefParameterT<std::size_t> par_dim_{GFI_DEF_PARDIM};
    Gem::Common::GOneTimeRefParameterT<double> min_var_{GFI_DEF_MINVAR};
    Gem::Common::GOneTimeRefParameterT<double> max_var_{GFI_DEF_MAXVAR};
    Gem::Common::GOneTimeRefParameterT<parameterType> p_t_{GFI_DEF_PARAMETERTYPE};
    Gem::Common::GOneTimeRefParameterT<initMode> i_m_{GFI_DEF_INITMODE};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple constraint checker searching for valid solutions that fulfill
 * a given constraint. Here, the sum of all double variables needs to be smaller
 * than a given constant.
 */
class GDoubleSumConstraint
  : public gpar::GOptimizableEntityConstraint { // NOLINT(cppcoreguidelines-special-member-functions)
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(Gem::Common::make_member("c_", c_));
    }
    auto localMembers() const {
        return std::make_tuple(Gem::Common::make_member("c_", c_));
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GOptimizableEntityConstraint);
        Gem::Common::serialize_members(ar, localMembers());
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    GDoubleSumConstraint() = default;
    /** @brief Initialization with the constant */
    explicit GDoubleSumConstraint(const double &);
    /** @brief The copy constructor */
    GDoubleSumConstraint(const GDoubleSumConstraint &cp) = default;

    /** @brief The destructor */
    ~GDoubleSumConstraint() override = default;

protected:
    double check_(const gpar::GOptimizableEntity *) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;

    /** @brief Loads the data of another GOptimizableEntityMultiConstraint */
    void load_(const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDoubleSumConstraint>(
        GDoubleSumConstraint const &,
        GDoubleSumConstraint const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

private:
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *clone_() const override;

    double c_ = 1.; ///< The constant that should not be exceeded by the sum of parameters
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constraint checker trying to enforce a condition x+y+z=C (note the equal
 * sign!) for double variables
 */
class GDoubleSumGapConstraint
  : public gpar::GOptimizableEntityConstraint { // NOLINT(cppcoreguidelines-special-member-functions)
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief The single declaration of this class'es local data members. */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("c_", c_),
            Gem::Common::make_member("gap_", gap_));
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("c_", c_),
            Gem::Common::make_member("gap_", gap_));
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GOptimizableEntityConstraint);
        Gem::Common::serialize_members(ar, localMembers());
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    GDoubleSumGapConstraint() = default;
    /** @brief Initialization with the constant */
    GDoubleSumGapConstraint(const double &, const double &);
    /** @brief The copy constructor */
    GDoubleSumGapConstraint(const GDoubleSumGapConstraint &cp) = default;

    /** @brief The destructor */
    ~GDoubleSumGapConstraint() override = default;

protected:
    double check_(const gpar::GOptimizableEntity *) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;

    /** @brief Loads the data of another GOptimizableEntityMultiConstraint */
    void load_(const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDoubleSumGapConstraint>(
        GDoubleSumGapConstraint const &,
        GDoubleSumGapConstraint const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

private:
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *clone_() const override;

    double c_ = 1.;    ///< The constant that should not be exceeded by the sum of parameters
    double gap_ = 0.5; ///< A tolerance around C_ that is still considered to be valid
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple constraint checker searching for valid solutions that fulfill
 * a given constraint. Here, valid solutions lie in a sphere around 0
 */
class GSphereConstraint
  : public gpar::GOptimizableEntityConstraint { // NOLINT(cppcoreguidelines-special-member-functions)
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(Gem::Common::make_member("diameter_", diameter_));
    }
    auto localMembers() const {
        return std::make_tuple(Gem::Common::make_member("diameter_", diameter_));
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GOptimizableEntityConstraint);
        // diameter_ was previously not serialized at all -- it was silently lost on
        // (de)serialization. Derive it from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    GSphereConstraint() = default;
    /** @brief Initialization with the diameter */
    explicit GSphereConstraint(const double &cp);
    /** @brief The copy constructor */
    GSphereConstraint(const GSphereConstraint &) = default;

    /** @brief The destructor */
    ~GSphereConstraint() override = default;

protected:
    double check_(const gpar::GOptimizableEntity *) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;

    /** @brief Loads the data of another GOptimizableEntityMultiConstraint */
    void load_(const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSphereConstraint>(
        GSphereConstraint const &,
        GSphereConstraint const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

private:
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *clone_() const override;

    /** @brief The diameter of the sphere */
    double diameter_ = 1.;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GFunctionIndividual)        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GFunctionIndividualFactory) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GDoubleSumConstraint)       // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GDoubleSumGapConstraint)    // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GSphereConstraint)          // NOLINT
