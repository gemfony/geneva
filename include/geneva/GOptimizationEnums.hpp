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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>
#include <limits>
#include <cmath>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Indicates whether an individual should be maximized or minimized
 */
enum class maxMode : Gem::Common::ENUMBASETYPE {
   MINIMIZE=0
	, MAXIMIZE=1
};

/******************************************************************************/
/** @brief Probability settings for random crashes */
const bool GPS_DEF_USE_RANDOMCRASH = false;
const double GPS_DEF_RANDOMCRASHPROB = 0.001;

/******************************************************************************/
/**
 * Default population sizes -- 100 by default (parents + children)
 */
const std::size_t DEFPARCHILDNPARENTS = 1;
const std::size_t DEFPARCHILDNCHILDREN = 99;
const std::size_t DEFPARCHILDPOPSIZE = DEFPARCHILDNPARENTS + DEFPARCHILDNCHILDREN;

/******************************************************************************/
/**
 * The default number of individuals to be monitored by GFitnessMonitorT<>
 */
const std::size_t DEFNMONITORINDS = 1;

/******************************************************************************/
/**
 * The default number of stalls, as of which Genevas swarm implementation switches
 * to repulsion mode. Setting this to 0 will force the swarm algorithm to always
 * use the attraction-mode
 */
const std::uint32_t DEFREPULSIONTHRESHOLD = 0;

/******************************************************************************/
/**
 * The maximum number an adaption of an individual should be performed until a
 * valid parameter set was found
 */
const std::size_t DEFMAXRETRIESUNTILVALID=10;

/******************************************************************************/
/**
 * Indicates whether only active, inactive or all parameters should be extracted
 */
enum class activityMode : Gem::Common::ENUMBASETYPE {
	ACTIVEONLY = 0 // Extract only active parameters
	, ALLPARAMETERS = 1 // Extract all parameters
	, INACTIVEONLY = 2 // Only extract inactive parameters
	, DEFAULTACTIVITYMODE = 1 // The default extraction mode
};

/******************************************************************************/
/**
 * The number of calls to the GParameterSet::customAdaption() function
 * without actual modifications
 */
const std::size_t DEFMAXUNSUCCESSFULADAPTIONS = 1000;

/******************************************************************************/
/**
 * Helps top better identify whether the object was marked as invalid
 */
const bool OE_NOT_MARKED_AS_INVALID = false;
const bool OE_MARKED_AS_INVALID = true;

/******************************************************************************/
/**
 * Helps to better identify raw and transformed fitness
 */
const std::size_t G_RAW_FITNESS = 0;
const std::size_t G_TRANSFORMED_FITNESS = 1;

/******************************************************************************/
/**
 * Whether to use raw or transformed fitness as return values or arguments
 */
const bool USERAWFITNESS         = false;
const bool USETRANSFORMEDFITNESS = true;

/******************************************************************************/
/**
 * Explicit permission or denial to perform re-evaluation
 */
const bool ALLOWREEVALUATION = true;
const bool PREVENTREEVALUATION = false;

/******************************************************************************/
/**
 * The number of individuals to be recorded in each iteration
 */
const std::size_t DEFNRECORDBESTINDIVIDUALS = 10;

/******************************************************************************/
/**
 * The worst allowed valid fitness value (positive or negative). This
 * value forms the upper and lower (negative) limit of a sigmoid function.
 */
const double WORSTALLOWEDVALIDFITNESS = 10000.;
const double FITNESSSIGMOIDSTEEPNESS  = 1000.;

/******************************************************************************/
/** @brief The optimization algorithm to be used if no others were found */
const std::string DEFAULTOPTALG="ea";

/******************************************************************************/
/** @brief The default number of threads for parallelization with threads */
const std::uint16_t DEFAULTNSTDTHREADS = 2;

/******************************************************************************/
/**
 * The general default population size
 */
const std::size_t DEFAULTPOPULATIONSIZE = 100;

/**
 * The default population size in evolutionary algorithms
 */
const std::size_t DEFAULTEAPOPULATIONSIZE = 42;

/**
 * The default number of parents in evolutionary algorithms
 */
const std::size_t DEFAULTEANPARENTS = 2;

/**
 * The default likelihood for an amalgamation of two obects to take place
 */
const double DEFAULTAMALGAMATIONLIKELIHOOD = 0.;

/**
 * The default likelihood for two items of a GParameterSet to be exchanged
 */
const double DEFAULTPERITEMEXCHANGELIKELIHOOD = 0.5;

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
const std::uint16_t DEFAULTXDIMOM=1024;

/**
 * The default dimension of the canvas in y-direction
 */
const std::uint16_t DEFAULTYDIMOM=768;

/******************************************************************************/
/**
 * The default maximum value for constrained double values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
const double GMAXCONSTRAINEDDOUBLE = (boost::numeric::bounds<double>::highest())/10.;

/******************************************************************************/
/**
 * The default maximum value for constrained float values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
const float GMAXCONSTRAINEDFLOAT = (boost::numeric::bounds<float>::highest())/10.f;


/******************************************************************************/
/**
 * The default maximum value for constrained std::int32_t values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
const std::int32_t GMAXCONSTRAINEDINT32 = (boost::numeric::bounds<std::int32_t>::highest())/10;

/******************************************************************************/
/**
 * The two const variables MAXIMIZE and MINIMIZE determine, whether the library
 * should work in maximization or minimization mode.
 */
const bool MAXIMIZE = true;
const bool MINIMIZE = false;

/******************************************************************************/
/**
 * Whether reasons for the termination of an optimization run should be emitted
 */
const bool DEFAULTEMITTERMINATIONREASON = true;

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
const std::uint32_t DEFAULTREPORTITER = 1;

/******************************************************************************/
/**
 * The number of iterations after which a checkpoint should be written.
 * 0 means that no checkpoints are written at all.
 */
const std::uint32_t DEFAULTCHECKPOINTIT = 0;

/******************************************************************************/
/**
 * The default number of stalls as of which individuals are asked to update
 * their internal data structures by the optimization algorithm. A value of 0
 * means "disabled".
 */
const std::uint32_t DEFAULTSTALLCOUNTERTHRESHOLD = 0;

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
const std::uint32_t DEFAULTOFFSET = 0;

/******************************************************************************/
/**
 * The default maximum number of iterations
 */
const std::uint32_t DEFAULTMAXIT = 1000;

/******************************************************************************/
/**
 * The default minimum number of iterations
 */
const std::uint32_t DEFAULTMINIT = 0;

/******************************************************************************/
/**
 * The default maximum number of iterations without improvement. 0 means: ignore
 */
const std::uint32_t DEFAULTMAXSTALLIT = 20;

/**
 * The default maximum number of iterations without improvement for paramneter
 * scans. As the algorithm has been instructed to scan an entire range, the
 * value is set to 0 (i.e. it is disabled).
 */
const std::uint32_t DEFAULTMAXPARSCANSTALLIT = 0;

/******************************************************************************/
/**
 * The default maximization mode
 */
const bool DEFAULTMAXMODE = false; // means: "minimization"

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
const std::uint32_t DEFAULTBROKERWAITFACTOR = 0;

/******************************************************************************/
/**
 * The default number of processing units
 */
const std::uint32_t DEFAULTNPROCESSINGUNITS = 0;

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
const double DEFAULTQUALITYTHRESHOLD=0.;

/******************************************************************************/
/**
 * Selection of policy for validity-check combiner
 */
enum class validityCheckCombinerPolicy : Gem::Common::ENUMBASETYPE {
	MULTIPLYINVALID = 0   // Multiplies all invalid checks (i.e. return values > 1) or returns 0, if all checks are valid
	, ADDINVALID = 1      // Adds all invalid checks or returns 0, if all checks are valid
};

/******************************************************************************/
/**
 * Selection of policy for evaluation
 */
enum class evaluationPolicy : Gem::Common::ENUMBASETYPE {
	USESIMPLEEVALUATION = 0            // Run evaluation function even for invalid parameter sets
	, USEWORSTCASEFORINVALID = 1       // Assign the worst possible value to invalid individuals, evaluate valid solutions as usual
	, USESIGMOID = 2                   // Assign a multiple of m_validity_level and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations
	, EVALUATIONPOLICY_LAST = static_cast<Gem::Common::ENUMBASETYPE>(evaluationPolicy::USESIGMOID)
};

// * Note that this might be accompanied by assistance from the optimization algorithm

/******************************************************************************/
/**
 * Specification of different parallelization modes used by optimization algorithms.
 * Some algorithms, in particular evolutionary algorithms, may perform multithreaded
 * and serial execution without the broker.
 */
enum class execMode : Gem::Common::ENUMBASETYPE {
	SERIAL = 0
	, MULTITHREADED = 1
	, BROKER = 2
	, LAST = static_cast<Gem::Common::ENUMBASETYPE>(execMode::BROKER)
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
	DEFAULTDUPLICATIONSCHEME = 0
	, RANDOMDUPLICATIONSCHEME = 1
	, VALUEDUPLICATIONSCHEME = 2
	, DUPLICATIONSCHEME_LAST = static_cast<Gem::Common::ENUMBASETYPE>(duplicationScheme::VALUEDUPLICATIONSCHEME)
};

/******************************************************************************/
/**
 * The info function can be called in these three modes
 */
enum class infoMode : Gem::Common::ENUMBASETYPE {
	INFOINIT = 0
	, INFOPROCESSING = 1
	, INFOEND = 2
	, INFOMODE_LAST = static_cast<Gem::Common::ENUMBASETYPE>(infoMode::INFOEND)
};

/******************************************************************************/
/**
 * Ids that are assigned to adaptors and which should (by convention!) be unique for these
 */
enum class adaptorId : Gem::Common::ENUMBASETYPE {
	GDOUBLEBIGAUSSADAPTOR = 0
	, GDOUBLEGAUSSADAPTOR = 1
	, GFLOATGAUSSADAPTOR = 2
	, GFLOATBIGAUSSADAPTOR = 3
	, GINT32GAUSSADAPTOR = 4
	, GBOOLEANADAPTOR = 5
	, GINT32FLIPADAPTOR = 6
	, ADAPTORIDE_LAST = static_cast<Gem::Common::ENUMBASETYPE>(adaptorId::GINT32FLIPADAPTOR)
};

/******************************************************************************/
/**
 * The selection mode in EA populations. MUPLUSNU_SINGLEEVAL means that new parents are selected from old
 * parents and their children. MUCOMMNU means that new parents are selected from children only.
 * MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation will also become a new parent
 * (unless a better child was found). All other parents are selected from children only.
 */
enum class sortingMode : Gem::Common::ENUMBASETYPE {
	MUPLUSNU_SINGLEEVAL = 0
	, MUCOMMANU_SINGLEEVAL = 1
	, MUNU1PRETAIN_SINGLEEVAL = 2
	, MUPLUSNU_PARETO = 3
	, MUCOMMANU_PARETO = 4
	, SORTINGMODE_LAST = static_cast<Gem::Common::ENUMBASETYPE>(sortingMode::MUCOMMANU_PARETO)
};

/******************************************************************************/
/**
 * The selection mode in MPEA populations.
 */
enum class sortingModeMP : Gem::Common::ENUMBASETYPE {
	MUPLUSNU_SINGLEEVAL_MP = 0
	, MUCOMMANU_SINGLEEVAL_MP = 1
	, MUNU1PRETAIN_SINGLEEVAL_MP = 2
	, SORTINGMODEMP_LAST = static_cast<Gem::Common::ENUMBASETYPE>(sortingModeMP::MUNU1PRETAIN_SINGLEEVAL_MP)
};

/******************************************************************************/
/**
 * Settings for simulated annealing
 */
const double SA_T0 = 1000.; ///< The default start temperature in simulated annealing
const double SA_ALPHA = 0.95; ///< The degradation strength in simulated annealing

/******************************************************************************/
/**
 * The default value for the GSerialEA::markOldParents_ flag. This is used mostly
 * for logging purposes. If set, the algorithm will keep a copy of the parents from which the
 * children originated and will mark their id in the individual's personality traits.
 */
const bool DEFAULTMARKOLDPARENTS = false;

/******************************************************************************/

const double DEFAULTSIGMA = 0.025; ///< Default start value for sigma_
const double DEFAULTINT32SIGMA = 0.1; ///< Default sigma start value for GInt32GaussAdaptor
const double DEFAULTSIGMASIGMA = 0.2; ///< Default width of the gaussian used for sigma adaption
const double DEFAULTMINSIGMA = 0.001; ///< Default minimum allowed value for sigma_
const double DEFAULTMAXSIGMA = 1; ///< Default maximum allowed value for sigma_
const double DEFAULTDELTA = 0.05; ///< Default value of distance between two gaussians in GNumBiGaussAdaptorT
const double DEFAULTSIGMADELTA = 0.2; ///< Default width of the gaussian used for delta adaption in GNumBiGaussAdaptorT
const double DEFAULTMINDELTA = 0.; ///< Default minimum value of distance between two gaussians in GNumBiGaussAdaptorT
const double DEFAULTMAXDELTA = 0.5; ///< Default maximum value of distance between two gaussians in GNumBiGaussAdaptorT
const double DEFAULTBITADPROB = 0.05; ///< 5 percent adaption probability for bits
const double DEFAULTADPROB = 1.0; ///< 100 percent adaption probability for all other cases
const double DEFAUPTADAPTADPROB = 0.1; ///< Whether adProb_ should undergo evolutionary adaption
const double DEFMINADPROB = 0.001; ///< The lower boundary for variations of adProb_
const double DEFMAXADPROB = 1.0; ///< The upper boundary for variations of adProb_
const std::uint32_t DEFAULTADAPTIONTHRESHOLD = 1; // Adaption parameters should be adapted whenever an adaption takes place
const double DEFAULTADAPTADAPTIONPROB = 0.1; // 10 percent probability for the adaption of adaption parameters

/******************************************************************************/
// Adaption modes

enum class adaptionMode : Gem::Common::ENUMBASETYPE {
	 ALWAYS = 0
	 , WITHPROBABILITY = 1
	 , NEVER = 2
};

/******************************************************************************/

const double DEFAULTCPERSONAL = 1.49; ///< Default multiplier for personal distances (swarm)
const double DEFAULTCNEIGHBORHOOD = 1.49; ///< Default multiplier for neighborhood distances (swarm)
const double DEFAULTCGLOBAL = 1.; ///< Default multiplier for global distances (swarm)
const double DEFAULTCVELOCITY = 0.72; ///< Default multiplier for velocities (swarm)
const double DEFAULTVELOCITYRANGEPERCENTAGE = 0.15; ///< Default percentage of velocity range used for initialization of velocities
const std::size_t DEFAULTNNEIGHBORHOODS = 5; ///< The default size of each neighborhood in swarm algorithms
const std::size_t DEFAULTNNEIGHBORHOODMEMBERS = 10; ///< The default number of members in each neighborhood

/******************************************************************************/
/**
 * Specifies different update rules in swarms
 */
enum class updateRule : Gem::Common::ENUMBASETYPE {
	SWARM_UPDATERULE_LINEAR = 0
	, SWARM_UPDATERULE_CLASSIC = 1
	, UPDATERULE_LAST = static_cast<Gem::Common::ENUMBASETYPE>(SWARM_UPDATERULE_CLASSIC)
};

/******************************************************************************/

const updateRule DEFAULTUPDATERULE = updateRule::SWARM_UPDATERULE_CLASSIC; ///< The default update rule in swarms

/******************************************************************************/
/**
 * Specifies between parallel, sequential or simplex Bee Colony Onlooker Phase
 *
 * Default mode is parallel, which is an experimental parallel approach to the usual sequential onlooker phase
 * sequential mode is following the usual Bee Colony Algorithm
 * Simplex is a very experimental approach to optimize the bee colony algorithm even further
 */
enum class abcParallelRule : Gem::Common::ENUMBASETYPE {
    ABC_PARALLEL = 0,
    ABC_SEQUENTIAL = 1,
    ABC_SIMPLEX = 2
};

/******************************************************************************/

const std::uint32_t DEFAULTMAXTRIAL = 15;
const abcParallelRule DEFAULTPARALLELRULE = abcParallelRule::ABC_PARALLEL;

/******************************************************************************/


/** @brief Puts a Gem::Geneva::maxMode into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::maxMode&);

/** @brief Reads a Gem::Geneva::maxMode from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::maxMode&);

/** @brief Puts a Gem::Geneva::activityMode into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::activityMode&);

/** @brief Reads a Gem::Geneva::activityMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::activityMode&);

/** @brief Puts a Gem::Geneva::evaluationPolicy into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::evaluationPolicy&);

/** @brief Reads a Gem::Geneva::evaluationPolicy item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::evaluationPolicy&);

/** @brief Puts a Gem::Geneva::validityCheckCombinerPolicy into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::validityCheckCombinerPolicy&);

/** @brief Reads a Gem::Geneva::validityCheckCombinerPolicy item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::validityCheckCombinerPolicy&);

/** @brief Puts a Gem::Geneva::execMode into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::execMode&);

/** @brief Reads a Gem::Geneva::execMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::execMode&);

/** @brief Puts a Gem::Geneva::duplicationScheme into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::duplicationScheme&);

/** @brief Reads a Gem::Geneva::duplicationScheme item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::duplicationScheme&);

/** @brief Puts a Gem::Geneva::infoMode into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::infoMode&);

/** @brief Reads a Gem::Geneva::infoMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::infoMode&);

/** @brief Puts a Gem::Geneva::adaptorId into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::adaptorId&);

/** @brief Reads a Gem::Geneva::adaptorId item from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::adaptorId&);

/** @brief Puts a Gem::Geneva::sortingMode into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::sortingMode&);

/** @brief Reads a Gem::Geneva::sortingMode from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::sortingMode&);

/** @brief Puts a Gem::Geneva::sortingModeMP into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::sortingModeMP&);

/** @brief Reads a Gem::Geneva::sortingModeMP from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::sortingModeMP&);

/** @brief Puts a Gem::Geneva::updateRule into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::updateRule&);

/** @brief Reads a Gem::Geneva::updateRule from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::updateRule&);

/** @brief Puts a Gem::Geneva::updateRule into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::abcParallelRule&);

/** @brief Reads a Gem::Geneva::updateRule from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::abcParallelRule&);

/** @brief Puts a Gem::Geneva::adaptionMode into a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::ostream& operator<<(std::ostream&, const Gem::Geneva::adaptionMode&);

/** @brief Reads a Gem::Geneva::adaptionMode from a stream. Needed also for boost::lexical_cast<> */
G_API_GENEVA std::istream& operator>>(std::istream&, Gem::Geneva::adaptionMode&);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

