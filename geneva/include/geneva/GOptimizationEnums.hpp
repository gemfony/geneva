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

// Standard headers go here
#include <cmath>
#include <istream>
#include <limits>
#include <ostream>
#include <string>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Indicates whether an individual should be maximized or minimized
 */
enum class maxMode : Gem::Common::ENUMBASETYPE {
    MINIMIZE = 0,
    MAXIMIZE = 1
};

/******************************************************************************/
/** @brief Probability settings for random crashes */
constexpr bool GPS_DEF_USE_RANDOMCRASH = false;
constexpr double GPS_DEF_RANDOMCRASHPROB = 0.001;

/******************************************************************************/
/**
 * Default population sizes -- 100 by default (parents + children)
 */
constexpr std::size_t DEFPARCHILDNPARENTS = 1;
constexpr std::size_t DEFPARCHILDNCHILDREN = 99;
const std::size_t DEFPARCHILDPOPSIZE = DEFPARCHILDNPARENTS + DEFPARCHILDNCHILDREN;

/******************************************************************************/
/**
 * The default number of individuals to be monitored by GFitnessMonitorT<>
 */
constexpr std::size_t DEFNMONITORINDS = 1;

/******************************************************************************/
/**
 * The default number of stalls, as of which Genevas swarm implementation switches
 * to repulsion mode. Setting this to 0 will force the swarm algorithm to always
 * use the attraction-mode
 */
constexpr std::uint32_t DEFREPULSIONTHRESHOLD = 0;

/******************************************************************************/
/**
 * The maximum number an adaption of an individual should be performed until a
 * valid parameter set was found
 */
constexpr std::size_t DEFMAXRETRIESUNTILVALID = 10;

/******************************************************************************/
/**
 * Indicates whether only active, inactive or all parameters should be extracted
 */
enum class activityMode : Gem::Common::ENUMBASETYPE {
    ACTIVEONLY = 0 // Extract only active parameters
        ,
    ALLPARAMETERS = 1 // Extract all parameters
        ,
    INACTIVEONLY = 2 // Only extract inactive parameters
        ,
    DEFAULTACTIVITYMODE = 1 // The default extraction mode
};

/******************************************************************************/
/**
 * The number of calls to the genome's adaption function
 * without actual modifications
 */
constexpr std::size_t DEFMAXUNSUCCESSFULADAPTIONS = 1000;

/******************************************************************************/
/**
 * Helps top better identify whether the object was marked as invalid
 */
constexpr bool OE_NOT_MARKED_AS_INVALID = false;
constexpr bool OE_MARKED_AS_INVALID = true;

/******************************************************************************/
/**
 * Helps to better identify raw and transformed fitness
 */
constexpr std::size_t G_RAW_FITNESS = 0;
constexpr std::size_t G_TRANSFORMED_FITNESS = 1;

/******************************************************************************/
/**
 * Whether to use raw or transformed fitness as return values or arguments
 */
constexpr bool USERAWFITNESS = false;
constexpr bool USETRANSFORMEDFITNESS = true;

/******************************************************************************/
/**
 * Explicit permission or denial to perform re-evaluation
 */
constexpr bool ALLOWREEVALUATION = true;
constexpr bool PREVENTREEVALUATION = false;

/******************************************************************************/
/**
 * The number of individuals to be recorded in each iteration
 */
constexpr std::size_t DEFNRECORDBESTINDIVIDUALS = 10;

/******************************************************************************/
/**
 * The worst allowed valid fitness value (positive or negative). This
 * value forms the upper and lower (negative) limit of a sigmoid function.
 */
constexpr double WORSTALLOWEDVALIDFITNESS = 10000.;
constexpr double FITNESSSIGMOIDSTEEPNESS = 1000.;

/******************************************************************************/
/** @brief The optimization algorithm to be used if no others were found */
const std::string DEFAULTOPTALG = "ea";

/******************************************************************************/
// The default number of threads for parallelization with threads lives in the courtier layer as
// Gem::Courtier::DEFAULTNSTDTHREADS (courtier/GCourtierEnums.hpp) -- qualify it at the use site.

/******************************************************************************/
/**
 * The general default population size
 */
constexpr std::size_t DEFAULTPOPULATIONSIZE = 100;

/**
 * The default population size in evolutionary algorithms
 */
constexpr std::size_t DEFAULTEAPOPULATIONSIZE = 42;

/**
 * The default number of parents in evolutionary algorithms
 */
constexpr std::size_t DEFAULTEANPARENTS = 2;

/**
 * The default likelihood for an amalgamation of two obects to take place
 */
constexpr double DEFAULTAMALGAMATIONLIKELIHOOD = 0.;

/**
 * The default likelihood for two items of a flat genome to be exchanged
 */
constexpr double DEFAULTPERITEMEXCHANGELIKELIHOOD = 0.5;

/******************************************************************************/
/**
 * The default name of the output file of the optimization monitor base class
 * for output in ROOT format
 */
const std::string DEFAULTROOTRESULTFILEOM = "./result.C";

/**
 * The default name of the output file of the optimization monitor base class
 * for output in CSV format
 */
const std::string DEFAULTCSVRESULTFILEOM = "./result.csv";

/**
 * The default dimension of the canvas in x-direction
 */
constexpr std::uint16_t DEFAULTXDIMOM = 1024;

/**
 * The default dimension of the canvas in y-direction
 */
constexpr std::uint16_t DEFAULTYDIMOM = 768;

/******************************************************************************/
/**
 * The default maximum value for constrained double values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
constexpr double GMAXCONSTRAINEDDOUBLE = std::numeric_limits<double>::max() / 10.;

/******************************************************************************/
/**
 * The default maximum value for constrained float values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
constexpr float GMAXCONSTRAINEDFLOAT = std::numeric_limits<float>::max() / 10.f;

/******************************************************************************/
/**
 * The default maximum value for constrained std::int32_t values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
constexpr std::int32_t GMAXCONSTRAINEDINT32 = std::numeric_limits<std::int32_t>::max() / 10;

/******************************************************************************/
/**
 * The two const variables MAXIMIZE and MINIMIZE determine, whether the library
 * should work in maximization or minimization mode.
 */
constexpr bool MAXIMIZE = true;
constexpr bool MINIMIZE = false;

/******************************************************************************/
/**
 * Whether reasons for the termination of an optimization run should be emitted
 */
constexpr bool DEFAULTEMITTERMINATIONREASON = true;

/******************************************************************************/
/**
 * The name of a termination file
 */
const std::string DEFAULTTERMINATIONFILE = "empty";

/******************************************************************************/
/**
 * The number of iterations after which information should be
 * emitted about the inner state of the optimization algorithm.
 */
constexpr std::uint32_t DEFAULTREPORTITER = 1;

/******************************************************************************/
/**
 * The number of iterations after which a checkpoint should be written.
 * 0 means that no checkpoints are written at all.
 */
constexpr std::uint32_t DEFAULTCHECKPOINTIT = 0;

/******************************************************************************/
/**
 * The default number of stalls as of which individuals are asked to update
 * their internal data structures by the optimization algorithm. A value of 0
 * means "disabled".
 */
constexpr std::uint32_t DEFAULTSTALLCOUNTERTHRESHOLD = 0;

/******************************************************************************/
/**
 * The default base name used for check-pointing. Derivatives of this
 * class can build distinguished filenames from this e.g. by adding
 * the current generation.
 */
const std::string DEFAULTCPBASENAME = "geneva.cp";

/******************************************************************************/
/**
 * The default directory used for check-pointing. We choose a directory
 * that will always exist.
 */
const std::string DEFAULTCPDIR = "./checkpoints/";

/******************************************************************************/
/**
 * The default serialization mode used for check-pointing
 */
const Gem::Common::serializationMode DEFAULTCPSERMODE = Gem::Common::serializationMode::BINARY;

/******************************************************************************/
/**
 * The default offset for a new optimization run
 */
constexpr std::uint32_t DEFAULTOFFSET = 0;

/******************************************************************************/
/**
 * The default maximum number of iterations
 */
constexpr std::uint32_t DEFAULTMAXIT = 1000;

/******************************************************************************/
/**
 * The default minimum number of iterations
 */
constexpr std::uint32_t DEFAULTMINIT = 0;

/******************************************************************************/
/**
 * The default maximum number of iterations without improvement. 0 means: ignore
 */
constexpr std::uint32_t DEFAULTMAXSTALLIT = 20;

/**
 * The default time-to-live (in dispatch rounds) of an entry in a networked consumer's late-return
 * buffer: a buffered late return that is not reaped within this many rounds is evicted (and the drop
 * counted/logged). Kept deliberately small -- a late return is only useful for the iteration that
 * immediately follows its submission.
 */
constexpr std::uint64_t DEFAULTLATERETURNTTL = 2;

/**
 * The default capacity of a networked consumer's late-return buffer, expressed as a MULTIPLE of the
 * population size (the cap scales with the population so it is independent of how a generation is
 * chunked into submission batches). 2.0 lets a full generation's worth of returns linger while the
 * next generation is in flight. 0.0 disables late-return buffering entirely.
 */
constexpr double DEFAULTLATERETURNCAPFACTOR = 2.0;

/**
 * The default maximum number of iterations without improvement for paramneter
 * scans. As the algorithm has been instructed to scan an entire range, the
 * value is set to 0 (i.e. it is disabled).
 */
constexpr std::uint32_t DEFAULTMAXPARSCANSTALLIT = 0;

/******************************************************************************/
/**
 * The default maximization mode
 */
constexpr bool DEFAULTMAXMODE = false; // means: "minimization"

/******************************************************************************/
/**
 * A 0 time period . timedHalt will not trigger if this duration is set
 */
const std::string EMPTYDURATION = "00:00:00"; // 0 - no duration

/******************************************************************************/
/**
 * The default factor applied to the turn-around time
 * of the first item in the current iteration. Used to
 * find a suitable timeout-value for following individuals.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
constexpr std::uint32_t DEFAULTBROKERWAITFACTOR = 0;

/******************************************************************************/
/**
 * The default number of processing units
 */
constexpr std::uint32_t DEFAULTNPROCESSINGUNITS = 0;

/******************************************************************************/
/**
 * The default allowed time in seconds for the first individual
 * in generation 0 to return. Set it to 0 to disable this timeout.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
const std::string DEFAULTBROKERFIRSTTIMEOUT = EMPTYDURATION;

/******************************************************************************/
/**
 * The default maximum duration of the calculation.
 */
const std::string DEFAULTDURATION = EMPTYDURATION;

/******************************************************************************/
/**
 * The default minimum duration of the duration
 */
const std::string DEFAULTMINDURATION = EMPTYDURATION;

/******************************************************************************/
/**
 * The default quality threshold
 */
constexpr double DEFAULTQUALITYTHRESHOLD = 0.;

/******************************************************************************/
/**
 * Selection of policy for validity-check combiner
 */
enum class validityCheckCombinerPolicy : Gem::Common::ENUMBASETYPE {
    MULTIPLYINVALID =
        0 // Multiplies all invalid checks (i.e. return values > 1) or returns 0, if all checks are valid
        ,
    ADDINVALID = 1 // Adds all invalid checks or returns 0, if all checks are valid
};

/******************************************************************************/
/**
 * Selection of policy for evaluation
 */
enum class evaluationPolicy : Gem::Common::ENUMBASETYPE {
    USESIMPLEEVALUATION = 0 // Run evaluation function even for invalid parameter sets
        ,
    USEWORSTCASEFORINVALID =
        1 // Assign the worst possible value to invalid individuals, evaluate valid solutions as usual
        ,
    USESIGMOID =
        2 // Assign a multiple of validity_level_ and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations
        ,
    EVALUATIONPOLICY_LAST = evaluationPolicy::USESIGMOID
};

// * Note that this might be accompanied by assistance from the optimization algorithm

/******************************************************************************/
/**
 * Specification of different parallelization modes used by optimization algorithms.
 * Some algorithms, in particular evolutionary algorithms, may perform multithreaded
 * and serial execution without the broker.
 */
enum class execMode : Gem::Common::ENUMBASETYPE {
    SERIAL = 0,
    MULTITHREADED = 1,
    BROKER = 2,
    LAST = execMode::BROKER
};

/**
 * The default parallelization mode of optimization algorithms
 */
const execMode DEFAULT_EXEC_MODE = execMode::MULTITHREADED;

/******************************************************************************/
/**
 * Currently three types of duplication schemes are supported:
 * - DEFAULTDUPLICATIONSCHEME defaults to RANDOMDUPLICATIONSCHEME
 * - RANDOMDUPLICATIONSCHEME chooses the parents to be replicated randomly from all parents
 * - VALUEDUPLICATIONSCHEME prefers parents with a higher fitness
 */
enum class duplicationScheme : Gem::Common::ENUMBASETYPE {
    DEFAULTDUPLICATIONSCHEME = 0,
    RANDOMDUPLICATIONSCHEME = 1,
    VALUEDUPLICATIONSCHEME = 2,
    DUPLICATIONSCHEME_LAST =
        duplicationScheme::VALUEDUPLICATIONSCHEME
};

/******************************************************************************/
/**
 * The info function can be called in these three modes
 */
enum class infoMode : Gem::Common::ENUMBASETYPE {
    INFOINIT = 0,
    INFOPROCESSING = 1,
    INFOEND = 2,
    INFOMODE_LAST = infoMode::INFOEND
};

/******************************************************************************/
/**
 * Ids that are assigned to adaptors and which should (by convention!) be unique for these
 */
enum class adaptorId : Gem::Common::ENUMBASETYPE {
    GDOUBLEBIGAUSSADAPTOR = 0,
    GDOUBLEGAUSSADAPTOR = 1,
    GFLOATGAUSSADAPTOR = 2,
    GFLOATBIGAUSSADAPTOR = 3,
    GINT32GAUSSADAPTOR = 4,
    GBOOLEANADAPTOR = 5,
    GINT32FLIPADAPTOR = 6,
    ADAPTORIDE_LAST = adaptorId::GINT32FLIPADAPTOR
};

/******************************************************************************/
/**
 * The selection mode in EA populations. MUPLUSNU_SINGLEEVAL means that new parents are selected from old
 * parents and their children. MUCOMMNU means that new parents are selected from children only.
 * MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation will also become a new parent
 * (unless a better child was found). All other parents are selected from children only.
 */
enum class sortingMode : Gem::Common::ENUMBASETYPE {
    MUPLUSNU_SINGLEEVAL = 0,
    MUCOMMANU_SINGLEEVAL = 1,
    MUNU1PRETAIN_SINGLEEVAL = 2,
    MUPLUSNU_PARETO = 3,
    MUCOMMANU_PARETO = 4,
    SORTINGMODE_LAST = sortingMode::MUCOMMANU_PARETO
};

/******************************************************************************/
/**
 * The selection mode in MPEA populations.
 */
enum class sortingModeMP : Gem::Common::ENUMBASETYPE {
    MUPLUSNU_SINGLEEVAL_MP = 0,
    MUCOMMANU_SINGLEEVAL_MP = 1,
    MUNU1PRETAIN_SINGLEEVAL_MP = 2,
    SORTINGMODEMP_LAST =
        sortingModeMP::MUNU1PRETAIN_SINGLEEVAL_MP
};

/******************************************************************************/
/**
 * Settings for simulated annealing
 */
constexpr double SA_T0 = 1000.;   ///< The default start temperature in simulated annealing
constexpr double SA_ALPHA = 0.95; ///< The degradation strength in simulated annealing

/******************************************************************************/
/**
 * The default value for the GSerialEA::markOldParents_ flag. This is used mostly
 * for logging purposes. If set, the algorithm will keep a copy of the parents from which the
 * children originated and will mark their id in the individual's personality traits.
 */
constexpr bool DEFAULTMARKOLDPARENTS = false;

/******************************************************************************/

constexpr double DEFAULTSIGMA = 0.025;    ///< Default start value for sigma_
constexpr double DEFAULTINT32SIGMA = 0.1; ///< Default sigma start value for GInt32GaussAdaptor
constexpr double DEFAULTSIGMASIGMA = 0.2; ///< Default width of the gaussian used for sigma adaption
constexpr double DEFAULTMINSIGMA = 0.001; ///< Default minimum allowed value for sigma_
constexpr double DEFAULTMAXSIGMA = 1;     ///< Default maximum allowed value for sigma_
constexpr double DEFAULTDELTA =
    0.05; ///< Default value of distance between two gaussians in GNumBiGaussAdaptorT
constexpr double DEFAULTSIGMADELTA =
    0.2; ///< Default width of the gaussian used for delta adaption in GNumBiGaussAdaptorT
constexpr double DEFAULTMINDELTA =
    0.; ///< Default minimum value of distance between two gaussians in GNumBiGaussAdaptorT
constexpr double DEFAULTMAXDELTA =
    0.5; ///< Default maximum value of distance between two gaussians in GNumBiGaussAdaptorT
constexpr double DEFAULTBITADPROB = 0.05;  ///< 5 percent adaption probability for bits
constexpr double DEFAULTADPROB = 1.0;      ///< 100 percent adaption probability for all other cases
constexpr double DEFAUPTADAPTADPROB = 0.1; ///< Whether adProb_ should undergo evolutionary adaption
constexpr double DEFMINADPROB = 0.001;     ///< The lower boundary for variations of adProb_
constexpr double DEFMAXADPROB = 1.0;       ///< The upper boundary for variations of adProb_
constexpr std::uint32_t DEFAULTADAPTIONTHRESHOLD =
    1; // Adaption parameters should be adapted whenever an adaption takes place
constexpr double DEFAULTADAPTADAPTIONPROB =
    0.1; // 10 percent probability for the adaption of adaption parameters

/******************************************************************************/
// Adaption modes

enum class adaptionMode : Gem::Common::ENUMBASETYPE {
    ALWAYS = 0,
    WITHPROBABILITY = 1,
    NEVER = 2
};

/******************************************************************************/

constexpr double DEFAULTCPERSONAL = 1.49;     ///< Default multiplier for personal distances (swarm)
constexpr double DEFAULTCNEIGHBORHOOD = 1.49; ///< Default multiplier for neighborhood distances (swarm)
constexpr double DEFAULTCGLOBAL = 1.;         ///< Default multiplier for global distances (swarm)
constexpr double DEFAULTCVELOCITY = 0.72;     ///< Default multiplier for velocities (swarm)
constexpr double DEFAULTVELOCITYRANGEPERCENTAGE =
    0.15; ///< Default percentage of velocity range used for initialization of velocities
constexpr std::size_t DEFAULTNNEIGHBORHOODS =
    5; ///< The default size of each neighborhood in swarm algorithms
constexpr std::size_t DEFAULTNNEIGHBORHOODMEMBERS =
    10; ///< The default number of members in each neighborhood

/******************************************************************************/
/**
 * Specifies different update rules in swarms
 */
enum class updateRule : Gem::Common::ENUMBASETYPE {
    SWARM_UPDATERULE_LINEAR = 0,
    SWARM_UPDATERULE_CLASSIC = 1,
    UPDATERULE_LAST = SWARM_UPDATERULE_CLASSIC
};

/******************************************************************************/

const updateRule DEFAULTUPDATERULE =
    updateRule::SWARM_UPDATERULE_CLASSIC; ///< The default update rule in swarms

/******************************************************************************/

/**
 * @brief Puts a Gem::Geneva::maxMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the maxMode value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::maxMode &am);

/**
 * @brief Reads a Gem::Geneva::maxMode from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed maxMode value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::maxMode &am);

/**
 * @brief Puts a Gem::Geneva::activityMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the activityMode value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::activityMode &am);

/**
 * @brief Reads a Gem::Geneva::activityMode item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed activityMode value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::activityMode &am);

/**
 * @brief Puts a Gem::Geneva::evaluationPolicy into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the evaluationPolicy value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::evaluationPolicy &iip);

/**
 * @brief Reads a Gem::Geneva::evaluationPolicy item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed evaluationPolicy value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::evaluationPolicy &iip);

/**
 * @brief Puts a Gem::Geneva::validityCheckCombinerPolicy into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the validityCheckCombinerPolicy value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &
operator<<(std::ostream &o, const Gem::Geneva::validityCheckCombinerPolicy &vccp);

/**
 * @brief Reads a Gem::Geneva::validityCheckCombinerPolicy item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed validityCheckCombinerPolicy value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::validityCheckCombinerPolicy &vccp);

/**
 * @brief Puts a Gem::Geneva::execMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the execMode value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::execMode &pm);

/**
 * @brief Reads a Gem::Geneva::execMode item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed execMode value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::execMode &pm);

/**
 * @brief Puts a Gem::Geneva::duplicationScheme into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the duplicationScheme value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::duplicationScheme &rc);

/**
 * @brief Reads a Gem::Geneva::duplicationScheme item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed duplicationScheme value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::duplicationScheme &rc);

/**
 * @brief Puts a Gem::Geneva::infoMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the infoMode value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::infoMode &im);

/**
 * @brief Reads a Gem::Geneva::infoMode item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed infoMode value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::infoMode &im);

/**
 * @brief Puts a Gem::Geneva::adaptorId into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the adaptorId value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::adaptorId &aid);

/**
 * @brief Reads a Gem::Geneva::adaptorId item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed adaptorId value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::adaptorId &aid);

/**
 * @brief Puts a Gem::Geneva::sortingMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the sortingMode value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::sortingMode &smode);

/**
 * @brief Reads a Gem::Geneva::sortingMode from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed sortingMode value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::sortingMode &smode);

/**
 * @brief Puts a Gem::Geneva::sortingModeMP into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the sortingModeMP value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::sortingModeMP &smode);

/**
 * @brief Reads a Gem::Geneva::sortingModeMP from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed sortingModeMP value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::sortingModeMP &smode);

/**
 * @brief Puts a Gem::Geneva::updateRule into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the updateRule value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::updateRule &ur);

/**
 * @brief Reads a Gem::Geneva::updateRule from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed updateRule value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::updateRule &ur);

/**
 * @brief Puts a Gem::Geneva::adaptionMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the output stream to write to; the second is the adaptionMode value to emit.
 * @return A reference to the same output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::adaptionMode &am);

/**
 * @brief Reads a Gem::Geneva::adaptionMode from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * The first argument is the input stream to read from; the second receives the parsed adaptionMode value.
 * @return A reference to the same input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::adaptionMode &am);

/******************************************************************************/

} /* namespace Gem::Geneva */
