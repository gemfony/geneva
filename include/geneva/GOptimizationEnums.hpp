/**
 * @file GOptimizationEnums.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>
#include <limits>

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/logic/tribool.hpp>

#ifndef GOPTIMIZATIONENUMS_HPP_
#define GOPTIMIZATIONENUMS_HPP_

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default number of stalls, as of which Genevas swarm implementation switches
 * to repulsion mode. Setting this to 0 will force the swarm algorithm to always
 * use the attraction-mode
 */
const boost::uint32_t DEFREPULSIONTHRESHOLD = 0;

/******************************************************************************/
/**
 * The maximum number an adaption of an individual should be performed until a
 * valid parameter set was found
 */
const std::size_t DEFMAXRETRIESUNTILVALID=10;

/******************************************************************************/
// Indicates whether only active, inactive or all parameters should be extracted
enum G_API_GENEVA activityMode {
   ACTIVEONLY = 0 // Extract only active parameters
   , ALLPARAMETERS = 1 // Extract all parameters
   , INACTIVEONLY = 2 // Only extract inactive parameters
   , DEFAULTACTIVITYMODE = 1 // The default extraction mode
};

/******************************************************************************/
/**
 * The number of calls to the GOptimizableEntity::customAdaption() function
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
 * The worst allowed valid fitness value (positive or negative)
 */
const double WORSTALLOWEDVALIDFITNESS = 10000.;
const double FITNESSSIGMOIDSTEEPNESS  = 1000.;

/******************************************************************************/
/** @brief The optimization algorithm to be used if no others were found */
const std::string DEFAULTOPTALG="ea";

/******************************************************************************/
/** @brief The default number of threads for parallelization with boost */
const boost::uint16_t DEFAULTNBOOSTTHREADS = 2;

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
const boost::uint16_t DEFAULTXDIMOM=1024;

/**
 * The default dimension of the canvas in y-direction
 */
const boost::uint16_t DEFAULTYDIMOM=768;

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
 * The default maximum value for constrained boost::int32_t values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
const boost::int32_t GMAXCONSTRAINEDINT32 = (boost::numeric::bounds<boost::int32_t>::highest())/10;

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
 * The number of iterations after which information should be
 * emitted about the inner state of the optimization algorithm.
 */
const boost::uint32_t DEFAULTREPORTITER = 1;

/******************************************************************************/
/**
 * The number of iterations after which a checkpoint should be written.
 * 0 means that no checkpoints are written at all.
 */
const boost::uint32_t DEFAULTCHECKPOINTIT = 0;

/******************************************************************************/
/**
 * The default number of stalls as of which individuals are asked to update
 * their internal data structures by the optimization algorithm. A value of 0
 * means "disabled".
 */
const boost::uint32_t DEFAULTSTALLCOUNTERTHRESHOLD = 0;

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
const Gem::Common::serializationMode DEFAULTCPSERMODE = Gem::Common::SERIALIZATIONMODE_BINARY;


/******************************************************************************/
/**
 * The default offset for a new optimization run
 */
const boost::uint32_t DEFAULTOFFSET = 0;

/******************************************************************************/
/**
 * The default maximum number of iterations
 */
const boost::uint32_t DEFAULTMAXIT = 1000;

/******************************************************************************/
/**
 * The default maximum number of iterations without improvement. 0 means: ignore
 */
const boost::uint32_t DEFAULTMAXSTALLIT = 20;

/**
 * The default maximum number of iterations without improvement for paramneter
 * scans. As the algorithm has been instructed to scan an entire range, the
 * value is set to 0 (i.e. it is disabled).
 */
const boost::uint32_t DEFAULTMAXPARSCANSTALLIT = 0;

/******************************************************************************/
/**
 * The default maximization mode
 */
const bool DEFAULTMAXMODE = false; // means: "minimization"

/******************************************************************************/
/**
 * A 0 time period . timedHalt will not trigger if this duration is set
 */
const std::string EMPTYDURATION = "00:00:00.000"; // 0 - no duration

/******************************************************************************/
/**
 * The default factor applied to the turn-around time
 * of the first item in the current iteration. Used to
 * find a suitable timeout-value for following individuals.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
const boost::uint32_t DEFAULTBROKERWAITFACTOR = 0;

/******************************************************************************/
/**
 * The default number of processing units
 */
const boost::uint32_t DEFAULTNPROCESSINGUNITS = 0;

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
 * The default quality threshold
 */
const double DEFAULTQUALITYTHRESHOLD=0.;

/******************************************************************************/
/**
 * Selection of policy for validity-check combiner
 */
enum G_API_GENEVA validityCheckCombinerPolicy {
   MULTIPLYINVALID = 0   // Multiplies all invalid checks (i.e. return values > 1) or returns 0, if all checks are valid
   , ADDINVALID = 1      // Adds all invalid checks or returns 0, if all checks are valid
};

/******************************************************************************/
/**
 * Selection of policy for evaluation
 */
enum G_API_GENEVA evaluationPolicy {
   USESIMPLEEVALUATION = 0            // Run evaluation function even for invalid parameter sets
   , USEWORSTCASEFORINVALID = 1       // Assign the worst possible value to invalid individuals, evaluate valid solutions as usual
   , USESIGMOID = 2                   // Assign a multiple of validityLevel_ and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations
   , USEWORSTKNOWNVALIDFORINVALID = 3 // Assign "invalidityLevel*worstKnownValid" to invalid individuals
   , EVALUATIONPOLICY_LAST = USEWORSTKNOWNVALIDFORINVALID
};

// * Note that this might be accompanied by assistance from the optimization algorithm

/******************************************************************************/
/**
 * Specification of different parallelization modes
 */
enum G_API_GENEVA execMode {
	EXECMODE_SERIAL = 0
	, EXECMODE_MULTITHREADED = 1
	, EXECMODE_BROKERAGE = 2
	, EXECMODE_LAST = EXECMODE_BROKERAGE
};

/**
 * The default parallelization mode of optimization algorithms
 */
const execMode DEFAULTEXECMODE = EXECMODE_MULTITHREADED;

/******************************************************************************/
/**
 * Currently three types of duplication schemes are supported:
 * - DEFAULTDUPLICATIONSCHEME defaults to RANDOMDUPLICATIONSCHEME
 * - RANDOMDUPLICATIONSCHEME chooses the parents to be replicated randomly from all parents
 * - VALUEDUPLICATIONSCHEME prefers parents with a higher fitness
 */
enum G_API_GENEVA duplicationScheme {
	  DEFAULTDUPLICATIONSCHEME = 0
	, RANDOMDUPLICATIONSCHEME = 1
	, VALUEDUPLICATIONSCHEME = 2
	, DUPLICATIONSCHEME_LAST = VALUEDUPLICATIONSCHEME
};

/******************************************************************************/
/**
 * The info function can be called in these three modes
 */
enum G_API_GENEVA infoMode {
	  INFOINIT = 0
	, INFOPROCESSING = 1
	, INFOEND = 2
	, INFOMODE_LAST = INFOEND
};

/******************************************************************************/
/**
 * Ids that are assigned to adaptors and which should (by convention!) be unique for these
 */
enum G_API_GENEVA adaptorId {
	GDOUBLEBIGAUSSADAPTOR = 0
	, GDOUBLEGAUSSADAPTOR = 1
	, GFLOATGAUSSADAPTOR = 2
	, GFLOATBIGAUSSADAPTOR = 3
	, GINT32GAUSSADAPTOR = 4
	, GBOOLEANADAPTOR = 5
	, GINT32FLIPADAPTOR = 6
	, ADAPTORIDE_LAST = GINT32FLIPADAPTOR
};

/******************************************************************************/
/**
 * The selection mode in EA populations. MUPLUSNU_SINGLEEVAL means that new parents are selected from old
 * parents and their children. MUCOMMNU means that new parents are selected from children only.
 * MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation will also become a new parent
 * (unless a better child was found). All other parents are selected from children only.
 */
enum G_API_GENEVA sortingMode {
	  MUPLUSNU_SINGLEEVAL = 0
	, MUCOMMANU_SINGLEEVAL = 1
	, MUNU1PRETAIN_SINGLEEVAL = 2
	, MUPLUSNU_PARETO = 3
	, MUCOMMANU_PARETO = 4
	, SORTINGMODE_LAST = MUCOMMANU_PARETO
};

/******************************************************************************/
/**
 * The selection mode in MPEA populations.
 */
enum G_API_GENEVA sortingModeMP {
     MUPLUSNU_SINGLEEVAL_MP = 0
   , MUCOMMANU_SINGLEEVAL_MP = 1
   , MUNU1PRETAIN_SINGLEEVAL_MP = 2
   , SORTINGMODEMP_LAST = MUNU1PRETAIN_SINGLEEVAL_MP
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
const boost::uint32_t DEFAULTADAPTIONTHRESHOLD = 1; // Adaption parameters should be adapted whenever an adaption takes place
const double DEFAULTADAPTADAPTIONPROB = 0.1; // 10 percent probability for the adaption of adaption parameters

/******************************************************************************/
// Adaption modes
const boost::logic::tribool DEFAULTADAPTIONMODE = boost::logic::indeterminate; // Adapt should happen with a given probability
const boost::logic::tribool ADAPTALWAYS = true; // Always adapt, independent of probability settings
const boost::logic::tribool ADAPTWITHPROB = boost::logic::indeterminate; // Adapt according to the set probability
const boost::logic::tribool ADAPTNEVER = false; // Never adapt (effectively disables the adaptor)

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
enum updateRule {
	  SWARM_UPDATERULE_LINEAR = 0
	, SWARM_UPDATERULE_CLASSIC = 1
	, UPDATERULE_LAST = SWARM_UPDATERULE_CLASSIC
};

/******************************************************************************/

const updateRule DEFAULTUPDATERULE = SWARM_UPDATERULE_CLASSIC; ///< The default update rule in swarms

/******************************************************************************/

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

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONENUMS_HPP_ */
