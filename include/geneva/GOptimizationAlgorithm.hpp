/**
 * @file GOptimizationAlgorithm.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <sstream>
#include <cmath>
#include <cfloat>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifndef GOPTIMIZATIONALGORITHM_HPP_
#define GOPTIMIZATIONALGORITHM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "hap/GRandomT.hpp"
#include "GIndividual.hpp"
#include "GMutableSetT.hpp"
#include "GObject.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************************/
/**
 * The default base name used for check-pointing. Derivatives of this
 * class can build distinguished filenames from this e.g. by adding
 * the current generation.
 */
const std::string DEFAULTCPBASENAME = "geneva.cp";

/******************************************************************************************/
/**
 * The default directory used for check-pointing. We choose a directory
 * that will always exist.
 */
const std::string DEFAULTCPDIR = "./";

/******************************************************************************************/
/**
 * The default serialization mode used for checkpointing
 */
const Gem::Common::serializationMode DEFAULTCPSERMODE = Gem::Common::SERIALIZATIONMODE_BINARY;

/******************************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions found in these
 * algorithms, such as a general call to "optimize()".
 */
class GOptimizationAlgorithm
	:public GMutableSetT<Gem::Geneva::GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GMutableSetT_GIndividual", boost::serialization::base_object<GMutableSetT<Gem::Geneva::GIndividual> >(*this))
	     & BOOST_SERIALIZATION_NVP(iteration_)
	     & BOOST_SERIALIZATION_NVP(maxIteration_)
	     & BOOST_SERIALIZATION_NVP(maxStallIteration_)
	     & BOOST_SERIALIZATION_NVP(reportIteration_)
	     & BOOST_SERIALIZATION_NVP(defaultPopulationSize_)
	     & BOOST_SERIALIZATION_NVP(bestPastFitness_)
	     & BOOST_SERIALIZATION_NVP(maximize_)
	     & BOOST_SERIALIZATION_NVP(stallCounter_)
  	     & BOOST_SERIALIZATION_NVP(cpInterval_)
	     & BOOST_SERIALIZATION_NVP(cpBaseName_)
	     & BOOST_SERIALIZATION_NVP(cpDirectory_)
	     & BOOST_SERIALIZATION_NVP(cpSerMode_)
	     & BOOST_SERIALIZATION_NVP(qualityThreshold_)
	     & BOOST_SERIALIZATION_NVP(hasQualityThreshold_)
	     & BOOST_SERIALIZATION_NVP(maxDuration_)
	     & BOOST_SERIALIZATION_NVP(emitTerminationReason_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GOptimizationAlgorithm();
	/** @brief The copy constructor */
	GOptimizationAlgorithm(const GOptimizationAlgorithm&);
	/** @brief The destructor */
	virtual ~GOptimizationAlgorithm();

	/** @brief Performs the necessary administratory work of doing check-pointing */
	void checkpoint(const bool&) const;
	/** @brief Loads the state of the class from disc */
	virtual void loadCheckpoint(const std::string&) = 0;

	/** @brief Check whether a better solution was found and update the stall counter as necessary */
	bool ifProgress(const double&);

	/** @brief Allows to set the number of generations after which a checkpoint should be written */
	void setCheckpointInterval(const boost::int32_t&);
	/** @brief Allows to retrieve the number of generations after which a checkpoint should be written */
	boost::uint32_t getCheckpointInterval() const;

	/** @brief Allows to set the base name of the checkpoint file */
	void setCheckpointBaseName(const std::string&, const std::string&);
	/** @brief Allows to retrieve the base name of the checkpoint file */
	std::string getCheckpointBaseName() const;
	/** @brief Allows to retrieve the directory where checkpoint files should be stored */
	std::string getCheckpointDirectory() const;

	/** @brief Determines whether checkpointing should be done in Text-, XML- or Binary-mode */
	void setCheckpointSerializationMode(const Gem::Common::serializationMode&);
	/** @brief Retrieves the current checkpointing serialization mode */
	Gem::Common::serializationMode getCheckpointSerializationMode() const;

	/** @brief Checks for equality with another GOptimizationAlgorithm object */
	bool operator==(const GOptimizationAlgorithm&) const;
	/** @brief Checks for inequality with another GOptimizationAlgorithm object */
	bool operator!=(const GOptimizationAlgorithm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Triggers the business logic of the optimization algorithm */
	virtual void optimize(const boost::uint32_t& startIteration = 0);

	/** @brief Emits information in regular intervals */
	virtual void doInfo(const infoMode& im);

	/** @brief Retrieves the default population size */
	std::size_t getDefaultPopulationSize() const;
	/** @brief Retrieve the current population size */
	std::size_t getPopulationSize() const;

	/** @brief Set the number of iterations after which the optimization should be stopped */
	void setMaxIteration(const boost::uint32_t&);
	/** @brief Retrieve the number of iterations after which optimization should be stopped */
	boost::uint32_t getMaxIteration() const;

	/** @brief Set the number of iterations after which sorting should be stopped */
	void setMaxStallIteration(const boost::uint32_t&);
	/** @brief Retrieve the number of iterations after which sorting should be stopped */
	boost::uint32_t getMaxStallIteration() const;

	/** @brief Sets the maximum allowed processing time */
	void setMaxTime(const boost::posix_time::time_duration&);
	/** @brief Retrieves the maximum allowed processing time */
	boost::posix_time::time_duration getMaxTime() const;

	/** @brief Sets a quality threshold beyond which optimization is expected to stop */
	void setQualityThreshold(const double&);
	/** @brief Retrieves the current value of the quality threshold */
	double getQualityThreshold(bool&) const;
	/** @brief Removes the quality threshold */
	void unsetQualityThreshold();
	/** @brief Checks whether a quality threshold has been set */
	bool hasQualityThreshold() const;

	/** @brief Retrieve the current iteration of the optimization run */
	boost::uint32_t getIteration() const;

	/** @brief Sets the number of iterations after which the algorithm should
	 * report about its inner state. */
	void setReportIteration(const boost::uint32_t&);
	/** @brief Returns the number of iterations after which the algorithm should
	 * report about its inner state. */
	boost::uint32_t getReportIteration() const;

	/** @brief Retrieve the current number of failed optimization attempts */
	boost::uint32_t getStallCounter() const;
	/** @brief Gives access to the best known fitness so far */
	double getBestFitness() const;

	/** @brief Specify whether we want to work in maximization or minimization mode */
	void setMaximize(const bool&);
	/** @brief Find out whether we work in maximization or minimization mode */
	bool getMaximize() const;

	/** @brief Specifies whether information about termination reasons should be emitted */
	void setEmitTerminationReason(const bool&);
	/** @brief Retrieves information on whether information about termination reasons should be emitted */
	bool getEmitTerminationReason() const;

	/**********************************************************************/
	/**
	 * This function converts an individual at a given position to the derived
	 * type and returns it. In DEBUG mode, the function will check whether the
	 * requested position exists.
	 *
	 * Note that this function will only be accessible to the compiler if individual_type
	 * is a derivative of GIndividual, thanks to the magic of Boost's enable_if and Type
	 * Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GIndividual object, as required by the user
	 */
	template <typename individual_type>
	inline boost::shared_ptr<individual_type> individual_cast(
			 const std::size_t& pos
		   , typename boost::enable_if<boost::is_base_of<GIndividual, individual_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		if(pos >= data.size()) {
			std::ostringstream error;
			error << "In GOptimizationAlgorithm::individual_cast<>() : Error" << std::endl
				  << "Tried to access position " << pos << " which is >= array size " << data.size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		boost::shared_ptr<individual_type> p = boost::dynamic_pointer_cast<individual_type>(data[pos]);

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GOptimizationAlgorithm::individual_cast<>() : Conversion error" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#else
		return boost::static_pointer_cast<individual_type>(data[pos]);
#endif /* DEBUG */
	}

protected:
	/**********************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/** @brief Allows derived classes to set the personality type of the individuals */
	virtual void setIndividualPersonalities() = 0;
	/** @brief Resets the personality type of all individuals */
	void resetIndividualPersonalities();

	/** @brief Saves the state of the class to disc */
	virtual void saveCheckpoint() const = 0;

	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic() = 0;

	/** @brief Sets the default size of the population */
	virtual void setDefaultPopulationSize(const std::size_t&);

	/** @brief user-defined halt-criterion for the optimization */
	virtual bool customHalt() const;
	/** @brief The adaption scheme for this population */
	virtual void customAdaptions();
	/** @brief The evaluation scheme for this population */
	virtual double fitnessCalculation();

	/** @brief Allows derived classes to reset the stallCounter */
	void resetStallCounter();

	/** @brief Helper function that determines whether a new value is better than an older one */
	bool isBetter(double, const double&) const;
	/** @brief Helper function that emits the worst case value depending on whether maximization or minimization is performed */
	double getWorstCase() const;

	/** @brief Allows derived classes to perform initialization work before the optimization clycle starts */
	virtual void init();
	/** @brief Allows derived classes to perform any remaining work after the optimization cycle has finished */
	virtual void finalize();

	/** @brief Resizes the population to the desired level and does some error checks */
	virtual void adjustPopulation() = 0;

	/***********************************************************************************/
    /**
     * A random number generator. Note that the actual calculation is possibly
     * done in a random number server, depending on the defines you have chosen.
     */
#ifdef USELOCALRANDOMADAPTION /* produce random numbers locally */
	Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL, double, boost::int32_t> gr;
#else /* act as a proxy, take random numbers from a factory */
	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY, double, boost::int32_t> gr;
#endif /* USEPROXYRANDOM */

	/**********************************************************************************/
private:
	/** @brief Emits true once a given time has passed */
	bool timedHalt() const;
	/** @brief Emits true once the quality is below or above a given threshold (depending on the optimization mode) */
	bool qualityHalt() const;
	/** @brief Determines when to stop the optimization */
	bool halt(const boost::uint32_t&) const;

	/** @brief Sets the maximization mode of all individuals */
	void setIndividualMaxMode();
	/** @brief Lets individuals know about the current iteration */
	void markIteration();
	/** @brief Marks the globally best known fitness in all individuals */
	void markBestFitness();
	/** @brief Marks the number of stalled optimization attempts in all individuals */
	void markNStalls();

	boost::uint32_t iteration_; ///< The current iteration
	boost::uint32_t maxIteration_; ///< The maximum number of iterations
	boost::uint32_t maxStallIteration_; ///< The maximum number of generations without improvement, after which optimization is stopped
	boost::uint32_t reportIteration_; ///< The number of generations after which a report should be issued
	std::size_t defaultPopulationSize_; ///< The nominal size of the population
	double bestPastFitness_; ///< Records the best fitness found in past generations
	bool maximize_; ///< The optimization mode (minimization/false vs. maximization/true)
	boost::uint32_t stallCounter_; ///< Counts the number of iterations without improvement
	boost::int32_t cpInterval_; ///< Number of generations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
	std::string cpBaseName_; ///< The base name of the checkpoint file
	std::string cpDirectory_; ///< The directory where checkpoint files should be stored
	Gem::Common::serializationMode cpSerMode_; ///< Determines whether checkpointing should be done in text-, XML, or binary mode
	double qualityThreshold_; ///< A threshold beyond which optimization is expected to stop
	bool hasQualityThreshold_; ///< Specifies whether a qualityThreshold has been set
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization
	mutable boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
	bool emitTerminationReason_; ///< Specifies whether information about reasons for termination should be emitted

#ifdef GENEVATESTING
public:
	/**********************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GOptimizationAlgorithm)

/**************************************************************************************************/

#endif /* GOPTIMIZATIONALGORITHM_HPP_ */
