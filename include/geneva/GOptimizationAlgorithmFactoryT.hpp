/**
 * @file GOptimizationAlgorithmFactoryT.hpp
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

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

#ifndef GOPTIMIZATIONALGORITHMFACTORYT
#define GOPTIMIZATIONALGORITHMFACTORYT

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GFactoryT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GBaseEA.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"

namespace Gem
{
namespace Geneva
{

/*******************************************************************************************/
// Default settings
const boost::uint16_t FACT_DEF_NEVALUATIONTHREADS=0;

/*******************************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for optimization algorithms
 */
template <typename T>
class GOptimizationAlgorithmFactoryT
	: public Gem::Common::GFactoryT<T>
{
public:
	/***************************************************************************************/
	/**
	 * The standard constructor
	 */
	GOptimizationAlgorithmFactoryT(const std::string& configFile, const parMode& pm)
		: Gem::Common::GFactoryT<T>(configFile)
		, pm_(pm)
	{ /* nothing */ }

	/***************************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GOptimizationAlgorithmFactoryT()
	{ /* nothing */ }

protected:
	/***************************************************************************************/
	/**
	 * Allows to describe configuration options
	 *
	 * @param gpb A reference to the parser-builder
	 */
	virtual void describeLocalOptions_(
		Gem::Common::GParserBuilder& gpb
	) {
		using namespace Gem::Courtier;

		std::string comment;

		comment = "";
		comment += "Determines the number of threads simultaneously running;";
		comment += "evaluations in multi-threaded mode. 0 means \"automatic\";";
		gpb.registerFileParameter<boost::uint16_t>(
			"nEvaluationThreads"
			, nEvaluationThreads_
			, FACT_DEF_NEVALUATIONTHREADS
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);


		// add local data
		comment = ""; // Reset the comment string
		comment += "The timeout for the retrieval of an;";
		comment += "iteration's first timeout;";
		{
			using namespace boost::posix_time;
			time_duration td(duration_from_string(DEFAULTBROKERFIRSTTIMEOUT));
			gpb.registerFileParameter<boost::posix_time::time_duration>(
				"firstTimeOut" // The name of the variable
				, firstTimeOut_
				, td // The default value
				, Gem::Common::VAR_IS_ESSENTIAL
				, comment
			);
		}

		comment = ""; // Reset the first comment string
		comment += "The lower boundary for the adaption;";
		comment += "of the waitFactor variable;";
		gpb.registerFileParameter<double>(
			"minWaitFactor" // The name of the first variable
			, minWaitFactor_
			, DEFAULTMINBROKERWAITFACTOR // The first default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the second comment string
		comment += "The upper boundary for the adaption;";
		comment += "of the waitFactor variable;";
		gpb.registerFileParameter<double>(
			"maxWaitFactor" // The name of the second variable
			, maxWaitFactor_
			, DEFAULTMAXBROKERWAITFACTOR // The second default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Activates (1) or de-activates (0) logging;";
		comment += "iteration's first timeout;";
		gpb.registerFileParameter<bool>(
			"doLogging" // The name of the variable
			, doLogging_
			, false // The default value
			, Gem::Common::VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Indicates that the broker connector should wait endlessly;";
		comment += "for further arrivals of individuals in an iteration;";
		gpb.registerFileParameter<bool>(
			"boundlessWait" // The name of the variable
			, boundlessWait_
			, false // The default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment =  ""; // 	Reset the comment string
		comment += "Specifies the amount by which the wait factor gets;";
		comment += "incremented or decremented during automatic adaption;";
		gpb.registerFileParameter<double>(
			"waitFactorIncrement" // The name of the variable
			, waitFactorIncrement_
			, DEFAULTBROKERWAITFACTORINCREMENT // The default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);
	}

	/***************************************************************************************/
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<T> getObject_(Gem::Common::GParserBuilder&, const std::size_t&) = 0;
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<T>&) = 0;

	parMode pm_; ///< Holds information about the desired parallelization mode

	boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multithreaded execution

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
	double minWaitFactor_; ///< The minimum allowed wait factor
	double maxWaitFactor_; ///< The maximum allowed wait factor
	bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
	bool boundlessWait_; ///< Indicates whether the retrieveItem call should wait for an unlimited amount of time
	double waitFactorIncrement_; ///< The amount by which the waitFactor_ may be incremented or decremented

private:
	/***************************************************************************************/
	/** @brief The default constructor. Intentionally private and undefined */
	GOptimizationAlgorithmFactoryT();
};

/*******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONALGORITHMFACTORYT */
