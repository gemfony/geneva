/**
 * @file GOptimizationEnums.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>
#include <limits>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/logic/tribool.hpp>

#ifndef GOPTIMIZATIONENUMS_HPP_
#define GOPTIMIZATIONENUMS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** @brief The default number of threads for parallelization with boost */
const boost::uint16_t DEFAULTNBOOSTTHREADS = 2;

/******************************************************************************/
/**
 * The default population size in evolutionary algorithms
 */
const std::size_t DEFAULTEAPOPULATIONSIZE = 100;

/**
 * The default number of parents in evolutionary algorithms
 */
const std::size_t DEFAULTEANPARENTS = 1;

/******************************************************************************/
/**
 * The default name of the output file of the optimization monitor base class
 */
const std::string DEFAULTRESULTFILEOM = "./result.C";

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
const double MAXCONSTRAINEDDOUBLE = (boost::numeric::bounds<double>::highest())/10.;

/******************************************************************************/
/**
 * The default maximum value for constrained float values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
const double MAXCONSTRAINEDFLOAT = (boost::numeric::bounds<float>::highest())/10.f;


/******************************************************************************/
/**
 * The default maximum value for constrained boost::int32_t values. It needs to be smaller
 * than the maximum allowed value for the underlying type in order to allow statements like
 * (max - min) without leaving the allowed value range.
 */
const boost::int32_t MAXCONSTRAINEDINT32 = (boost::numeric::bounds<boost::int32_t>::highest())/10.;

/******************************************************************************/
/**
 * The two const variables MAXIMIZE and MINIMIZE determine, whether the library
 * should work in maximization or minimization mode.
 */
const bool MAXIMIZE = true;
const bool MINIMIZE = false;

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
const boost::uint32_t DEFAULTMAXSTALLIT = 0;

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
 * Specification of different parallelization modes
 */
enum parMode {
	PARMODE_SERIAL = 0
	, PARMODE_MULTITHREADED = 1
	, PARMODE_BROKERAGE = 2
};

/**
 * The default parallelization mode of optimization algorithms
 */
const parMode DEFAULTPARALLELIZATIONMODE = PARMODE_MULTITHREADED;

/******************************************************************************/
/**
 * Currently three types of duplication schemes are supported:
 * - DEFAULTDUPLICATIONSCHEME defaults to RANDOMDUPLICATIONSCHEME
 * - RANDOMDUPLICATIONSCHEME chooses the parents to be replicated randomly from all parents
 * - VALUEDUPLICATIONSCHEME prefers parents with a higher fitness
 */
enum duplicationScheme {
	  DEFAULTDUPLICATIONSCHEME = 0
	, RANDOMDUPLICATIONSCHEME = 1
	, VALUEDUPLICATIONSCHEME = 2
};

/******************************************************************************/
/**
 * The info function can be called in these three modes
 */
enum infoMode {
	  INFOINIT
	, INFOPROCESSING
	, INFOEND
};

/******************************************************************************/
/**
 * Ids that are assigned to adaptors and which should (by convention!) be unique for these
 */
enum adaptorId {
	GDOUBLEBIGAUSSADAPTOR
	, GDOUBLEGAUSSADAPTOR
	, GFLOATGAUSSADAPTOR
	, GFLOATBIGAUSSADAPTOR
	, GINT32GAUSSADAPTOR
	, GBOOLEANADAPTOR
	, GINT32FLIPADAPTOR
};

/******************************************************************************/
/**
 * The selection mode in EA populations. MUPLUSNU_SINGLEEVAL means that new parents are selected from old
 * parents and their children. MUCOMMNU means that new parents are selected from children only.
 * MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation will also become a new parent
 * (unless a better child was found). All other parents are selected from children only.
 */
enum sortingMode {
	  MUPLUSNU_SINGLEEVAL = 0
	, MUCOMMANU_SINGLEEVAL = 1
	, MUNU1PRETAIN_SINGLEEVAL = 2
	, MUPLUSNU_PARETO = 3
	, MUCOMMANU_PARETO = 4
};

/******************************************************************************/
/**
 * The selection mode in MPEA populations.
 */
enum sortingModeMP {
     MUPLUSNU_SINGLEEVAL_MP = 0
   , MUCOMMANU_SINGLEEVAL_MP = 1
   , MUNU1PRETAIN_SINGLEEVAL_MP = 2
};

/******************************************************************************/
/**
 * Settings for simulated annealing
 */
const double SA_T0 = 1000.; ///< The default start temperature in simulated annealing
const double SA_ALPHA = 0.95; ///< The degradation strength in simulated annealing

/******************************************************************************/
/**
 * Different optimization algorithms need to assign different information to individuals. They
 * can thus assume different personalities, resulting in different data structures to be stored
 * in them (see the GPersonalityTraits classes).
 */
enum personality_oa {
	  PERSONALITY_NONE = 0
	, PERSONALITY_EA = 1 // Evolutionary algorithms
	, PERSONALITY_GD = 2 // Gradient descents
	, PERSONALITY_SWARM = 3 // Swarm algorithms
	, PERSONALITY_SA = 4 // Simulated annealing
	, PERSONALITY_MPEA = 5 // Multi population
	, PERSONALITY_PS = 6 // Parameter scans
};

/******************************************************************************/
/**
 * The maximum number of resubmissions allowed in broker-based gradient descents
 */
const std::size_t DEFAULTMAXGDRESUBMISSIONS = 5;

/******************************************************************************/
/**
 * The default value for the GSerialEA::markOldParents_ flag. This is used mostly
 * for logging purposes. If set, the algorithm will keep a copy of the parents from which the
 * children originated and will mark their id in the individual's personality traits.
 */
const bool DEFAULTMARKOLDPARENTS = false;

/******************************************************************************/

const double DEFAULTSIGMA = 1; ///< Default start value for sigma_
const double DEFAULTINT32SIGMA = 5; ///< Default sigma start value for GInt32GaussAdaptor
const double DEFAULTSIGMASIGMA = 0.8; ///< Default width of the gaussian used for sigma adaption
const double DEFAULTMINSIGMA = 0.0001; ///< Default minimum allowed value for sigma_
const double DEFAULTMAXSIGMA = 5; ///< Default maximum allowed value for sigma_
const double DEFAULTDELTA = 1; ///< Default value of distance between two gaussians in GNumBiGaussAdaptorT
const double DEFAULTSIGMADELTA = 0.8; ///< Default width of the gaussian used for delta adaption in GNumBiGaussAdaptorT
const double DEFAULTMINDELTA = 0.; ///< Default minimum value of distance between two gaussians in GNumBiGaussAdaptorT
const double DEFAULTMAXDELTA = 0.; ///< Default maximum value of distance between two gaussians in GNumBiGaussAdaptorT
const double DEFAULTBITADPROB = 0.05; // 5 percent adaption probability for bits
const double DEFAULTADPROB = 1.0; // 100 percent adaption probability for all other cases
const boost::uint32_t DEFAULTADAPTIONTHRESHOLD = 1; // Adaption parameters should be adapted whenever an adaption takes place
const double DEFAULTADAPTADAPTIONPROB = 0.; // 0 percent probability for the adaption of adaption parameters

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
};

/******************************************************************************/

const updateRule DEFAULTUPDATERULE = SWARM_UPDATERULE_CLASSIC; ///< The default update rule in swarms

/******************************************************************************/

/** @brief Puts a Gem::Geneva::parMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::parMode&);

/** @brief Reads a Gem::Geneva::parMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::parMode&);

/** @brief Puts a Gem::Geneva::duplicationScheme into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::duplicationScheme&);

/** @brief Reads a Gem::Geneva::duplicationScheme item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::duplicationScheme&);

/** @brief Puts a Gem::Geneva::infoMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::infoMode&);

/** @brief Reads a Gem::Geneva::infoMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::infoMode&);

/** @brief Puts a Gem::Geneva::adaptorId into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::adaptorId&);

/** @brief Reads a Gem::Geneva::adaptorId item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::adaptorId&);

/** @brief Puts a Gem::Geneva::sortingMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::sortingMode&);

/** @brief Reads a Gem::Geneva::sortingMode from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::sortingMode&);

/** @brief Puts a Gem::Geneva::sortingModeMP into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::sortingModeMP&);

/** @brief Reads a Gem::Geneva::sortingModeMP from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::sortingModeMP&);

/** @brief Puts a Gem::Geneva::personality_oa into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::personality_oa&);

/** @brief Reads a Gem::Geneva::personality_oa from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::personality_oa&);

/** @brief Puts a Gem::Geneva::updateRule into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::updateRule&);

/** @brief Reads a Gem::Geneva::updateRule from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::updateRule&);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONENUMS_HPP_ */
