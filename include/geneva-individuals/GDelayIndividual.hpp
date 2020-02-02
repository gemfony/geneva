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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <tuple>

// Boost header files go here
#include <boost/serialization/nvp.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * An exception to be thrown by the fitness function in order to simulate crashes
 */
class fitnessException : public std::exception
{
public:
	 using std::exception::exception;
};

/******************************************************************************/
/**
 * This individual waits for a predefined amount of time before returning the result of the evaluation
 * (which is random). Its purpose is to measure the overhead of the parallelization, compared
 * to the serial execution. It may also be used to track down problems in the broker, as the execution
 * time is well-defined, and the calculation of wait factors depends on fewer variables. Apart from fixed
 * "processing times" the individual may also wait random amounts of time in a predefined window, or may
 * crash with a predefined likelihood. This allows to test the stability of the communication between
 * clients and server.
 */
class GDelayIndividual: public GParameterSet
{
	 /////////////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<class Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		 & BOOST_SERIALIZATION_NVP(m_fixedSleepTime)
		 & BOOST_SERIALIZATION_NVP(m_mayCrash)
		 & BOOST_SERIALIZATION_NVP(m_throwLikelihood)
		 & BOOST_SERIALIZATION_NVP(m_sleepRandomly)
		 & BOOST_SERIALIZATION_NVP(m_randSleepBoundaries);
	 }

	 /////////////////////////////////////////////////////////////////////////////

public:
	 /** The default constructor */
	 G_API_INDIVIDUALS GDelayIndividual();
	 /** @brief A standard copy constructor */
	 G_API_INDIVIDUALS GDelayIndividual(const GDelayIndividual&);
	 /** @brief The standard destructor */
	 virtual G_API_INDIVIDUALS ~GDelayIndividual();

	 /** @brief Sets the sleep-time to a user-defined value */
	 G_API_INDIVIDUALS void setFixedSleepTime(const std::chrono::duration<double>&);
	 /** @brief Retrieval of the current value of the m_fixedSleepTime variable */
	 G_API_INDIVIDUALS std::chrono::duration<double> getFixedSleepTime() const;

	 /** @brief Indicate that the fitness function may crash at the end of the sleep time */
	 G_API_INDIVIDUALS void setMayCrash(bool, double);
	 /** @brief Check whether the fitness function may crash at the end of the sleep time */
	 G_API_INDIVIDUALS bool getMayCrash() const;
	 /** @brief Check the likelihood for a crash at the end of the sleep time */
	 G_API_INDIVIDUALS double getCrashLikelihood() const;

	 /** @brief Indicates that the fitness function should sleep for a random time */
	 G_API_INDIVIDUALS void setRandomSleep(bool, std::tuple<double,double>);
	 /** @brief Checks whether the fitness function has a random sleep schedule */
	 G_API_INDIVIDUALS bool getMaySleepRandomly() const;
	 /** @brief Retrieves the time window for random sleeps */
	 G_API_INDIVIDUALS std::tuple<double,double> getSleepWindow() const;

protected:
	 /** @brief Loads the data of another GDelayIndividual, camouflaged as a GObject */
	 G_API_INDIVIDUALS virtual void load_(const GObject*) final;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GDelayIndividual>(
		GDelayIndividual const &
		, GDelayIndividual const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_INDIVIDUALS void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const final;

	/** @brief The actual adaption operations */
	 virtual G_API_INDIVIDUALS std::size_t customAdaptions() final;
	 /** @brief The actual fitness calculation takes place here */
	 virtual G_API_INDIVIDUALS double fitnessCalculation() final;

private:
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_INDIVIDUALS GObject* clone_() const final;

	 double m_fixedSleepTime; ///< The amount of time the evaluation function should sleep before continuing (seconds)

	 bool m_mayCrash = false; ///< Indicates whether the fitness function may throw at the end of the sleep time
	 double m_throwLikelihood = 0.001; ///< The likelihood for an exception to be thrown from the fitness function

	 bool m_sleepRandomly = false; /// Whether to sleep for a random amount of time instead of fixed amounts
	 std::tuple<double,double> m_randSleepBoundaries = std::tuple<double,double>(0.,1.); ///< Boundaries in seconds for random sleep (min/max amount of delay)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFMinIndividual objects
 */
class GDelayIndividualFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
public:
	 /** @brief The standard constructor */
	 G_API_INDIVIDUALS GDelayIndividualFactory(boost::filesystem::path const&);
	 /** @brief The destructor */
	 virtual G_API_INDIVIDUALS ~GDelayIndividualFactory();

	 /** @brief Allows to retrieve the name of the result file */
	 G_API_INDIVIDUALS std::string getResultFileName() const;
	 /** @brief Allows to retrieve the name of the file holding the short measurement results */
	 G_API_INDIVIDUALS std::string getShortResultFileName() const;
	 /** @brief Allows to retrieve the number of delays requested by the user */
	 G_API_INDIVIDUALS std::size_t getNDelays() const;
	 /** @brief Allows to retrieve the number of measurements to be made for each delay */
	 G_API_INDIVIDUALS std::uint32_t getNMeasurements() const;
	 /** @brief Retrieves the amount of seconds main() should wait between two measurements */
	 G_API_INDIVIDUALS std::uint32_t getInterMeasurementDelay() const;
	 /** @brief Retrieves the sleep times */
	 G_API_INDIVIDUALS std::vector<std::tuple<unsigned int, unsigned int>> getSleepTimes() const;

protected:
	 /** @brief Allows to describe local configuration options in derived classes */
	 virtual G_API_INDIVIDUALS void describeLocalOptions_(Gem::Common::GParserBuilder&) final;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual G_API_INDIVIDUALS void postProcess_(std::shared_ptr<GParameterSet>&) final;

private:
	 /** @brief The default constructor. Only needed for (de-)serialization purposes */
	 GDelayIndividualFactory() = default;

	 /** @brief Creates individuals of this type */
	 virtual G_API_INDIVIDUALS std::shared_ptr<GParameterSet> getObject_(
		Gem::Common::GParserBuilder&
		, const std::size_t&
	 ) final;

	 /** @brief Converts a tuple to a time format */
	 std::chrono::duration<double> tupleToTime(const std::tuple<unsigned int, unsigned int>&);

	 std::size_t m_nVariables = 100;
	 std::string m_delays = "(0,1), (0,10), (0,100), (0,500), (1,0)";
	 std::vector<std::tuple<unsigned int, unsigned int>> m_sleepTimes;
	 std::string m_resultFile = "fullResults.C";
	 std::string m_shortResultFile = "shortDelayResults.txt";
	 std::uint32_t m_nMeasurements = 10; ///< The number of measurements for each delay
	 std::uint32_t m_interMeasurementDelay = 1; ///< The delay between two measurements
	 bool m_mayCrash = false; ///< Indicates whether the fitness function may throw at the end of the sleep time
	 double m_throwLikelihood = 0.001; ///< The likelihood for an exception to be thrown from the fitness function
	 bool m_sleepRandomly = false; /// Whether to sleep for a random amount of time instead of fixed amounts
	 double m_lowerRandSleepBoundary = 0.; ///< The lower boundary for random sleeps
	 double m_upperRandSleepBoundary = 1.; ///< The upper boundary for random sleeps
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDelayIndividual)

