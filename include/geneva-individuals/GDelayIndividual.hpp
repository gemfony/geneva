/**
 * @file GDelayIndividual.hpp
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

#ifndef GDELAYINDIVIDUAL_HPP_
#define GDELAYINDIVIDUAL_HPP_

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

	 /** @brief A standard assignment operator */
	 G_API_INDIVIDUALS GDelayIndividual& operator=(const GDelayIndividual&);

	 /** @brief Checks for equality with another GDelayIndividual object */
	 G_API_INDIVIDUALS bool operator==(const GDelayIndividual&) const;
	 /** @brief Checks for inequality with another GDelayIndividual object */
	 G_API_INDIVIDUALS bool operator!=(const GDelayIndividual&) const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_INDIVIDUALS void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const final;

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
	 G_API_INDIVIDUALS GDelayIndividualFactory(const std::string&);
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
	 /** @brief Creates individuals of this type */
	 virtual G_API_INDIVIDUALS std::shared_ptr<GParameterSet> getObject_(
		 Gem::Common::GParserBuilder&
		 , const std::size_t&
	 ) final;
	 /** @brief Allows to describe local configuration options in derived classes */
	 virtual G_API_INDIVIDUALS void describeLocalOptions_(Gem::Common::GParserBuilder&) final;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual G_API_INDIVIDUALS void postProcess_(std::shared_ptr<GParameterSet>&) final;

private:
	 /** @brief The default constructor. Intentionally private and undefined */
	 GDelayIndividualFactory() = delete;

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

#endif /* GDELAYINDIVIDUAL_HPP_ */
