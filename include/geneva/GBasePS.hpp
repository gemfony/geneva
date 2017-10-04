/**
 * @file GBasePS.hpp
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
#include <fstream>
#include <memory>

// Boost headers go here

#ifndef GBASEPS_HPP_
#define GBASEPS_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GScanPar.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GPSPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// A number of typedefs that indicate the position and value of a parameter inside of an individual
using singleBPar     = std::tuple<bool,         std::size_t, std::string, std::size_t>;
using singleInt32Par = std::tuple<std::int32_t, std::size_t, std::string, std::size_t>;
using singleFPar     = std::tuple<float,        std::size_t, std::string, std::size_t>;
using singleDPar     = std::tuple<double,       std::size_t, std::string, std::size_t>;

/******************************************************************************/
/**
 * This struct holds the entire data to be updated inside of an individual
 */
struct parSet {
	std::vector<singleBPar>     bParVec;
	std::vector<singleInt32Par> iParVec;
	std::vector<singleFPar>     fParVec;
	std::vector<singleDPar>     dParVec;
};

/******************************************************************************/
/** @brief A simple output operator for parSet object, mostly meant for debugging */
G_API_GENEVA std::ostream& operator<<(std::ostream& os, const parSet& pS);

/******************************************************************************/
/** @brief The default number of "best" individuals to be kept during the algorithm run */
const std::size_t DEFAULTNMONITORINDS = 10;

/******************************************************************************/
/**
 * This algorithm scans a given parameter range, either in a random order,
 * or on a grid. On a grid, for each integer- or floating point-coordinate to be scanned,
 * it is given the lower and upper boundaries (both inclusive) and the number
 * of steps (including the boundaries). For boolean parameters, both true and
 * false will be tested. The algorithm only takes into consideration the first
 * individual that was registered. It will be duplicated for all possible
 * combinations, and the parameters adapted as required. The algorithm will
 * decide itself about the number of iterations, based on the number of required
 * tests and the desired population size. Please note that the amount of tests
 * required grows quickly with the number of steps and parameters and can easily
 * extend beyond the range where computation still makes sense. E.g., if you
 * plan to test but 4 values for each of 100 parameters, you'd have to evaluate
 * 4^100 individuals which, at a millisecond evaluation time per individual, would
 * require approximately 7*10^49 years to compute ... (on a side note, this is
 * the very reason why optimization algorithms are needed to search for the
 * best solution). So realistically, this algorithm can only be used for small
 * numbers of parameters and steps. In random sampling mode, the algorithm will
 * try to evenly scatter random individuals throughout the parameter space (defined
 * by those parameters intended to be modified). The optimization monitor associated
 * with this class will simply store all parameters and results in an XML file.
 */
class GBasePS
	:public GOptimizationAlgorithmT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>>(*this))
		& BOOST_SERIALIZATION_NVP(scanRandomly_)
		& BOOST_SERIALIZATION_NVP(nMonitorInds_)
		& BOOST_SERIALIZATION_NVP(bVec_)
		& BOOST_SERIALIZATION_NVP(int32Vec_)
		& BOOST_SERIALIZATION_NVP(dVec_)
		& BOOST_SERIALIZATION_NVP(fVec_)
		& BOOST_SERIALIZATION_NVP(simpleScanItems_)
		& BOOST_SERIALIZATION_NVP(scansPerformed_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GBasePS();
	/** @brief A standard copy constructor */
	G_API_GENEVA GBasePS(const GBasePS&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBasePS();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBasePS& operator=(const GBasePS&);

	/** @brief Checks for equality with another GBasePS object */
	virtual G_API_GENEVA bool operator==(const GBasePS&) const;
	/** @brief Checks for inequality with another GBasePS object */
	virtual G_API_GENEVA bool operator!=(const GBasePS&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Returns information about the type of optimization algorithm */
	virtual G_API_GENEVA std::string getOptimizationAlgorithm() const override;

	/** @brief Retrieves the number of processable items for the current iteration */
	virtual G_API_GENEVA std::size_t getNProcessableItems() const override;

	/** @brief Returns the name of this optimization algorithm */
	virtual G_API_GENEVA std::string getAlgorithmName() const override;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

	/** @brief Allows to set the number of "best" individuals to be monitored over the course of the algorithm run */
	G_API_GENEVA void setNMonitorInds(std::size_t);
	/** @brief Allows to retrieve  the number of "best" individuals to be monitored over the course of the algorithm run */
	G_API_GENEVA std::size_t getNMonitorInds() const;

	/** @brief Fills vectors with parameter specifications */
	G_API_GENEVA void setParameterSpecs(std::string);

	/** @brief Puts the class in "simple scan" mode */
	G_API_GENEVA void setNSimpleScans(std::size_t);
	/** @brief Retrieves the number of simple scans (or 0, if disabled) */
	G_API_GENEVA std::size_t getNSimpleScans() const;
	/** @brief Retrieves the number of scans performed so far */
	G_API_GENEVA std::size_t getNScansPerformed() const;

	/** @brief Allows to specify whether the parameter space should be scanned randomly or on a grid */
	G_API_GENEVA void setScanRandomly(bool);
	/** @brief Allows to check whether the parameter space should be scanned randomly or on a grid */
	G_API_GENEVA bool getScanRandomly() const;

protected:
	/***************************************************************************/
	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) override;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject *clone_() const override = 0;

	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual G_API_GENEVA std::tuple<double, double> cycleLogic() override;
	/** @brief Does some preparatory work before the optimization starts */
	virtual G_API_GENEVA void init() override;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() override;

	/** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	virtual G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

	/** @brief Resizes the population to the desired level and does some error checks */
	virtual G_API_GENEVA void adjustPopulation() override;

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual G_API_GENEVA void runFitnessCalculation() override = 0;

	/** @brief A custom halt criterion for the optimization, allowing to stop the loop when no items are left to be scanned */
	virtual G_API_GENEVA bool customHalt() const override;

private:
	/***************************************************************************/
	/**
	 * Adds a given data point to a data vector
	 */
	template <typename data_type>
	void addDataPoint(
		const std::tuple<data_type, std::size_t, std::string, std::size_t>& dataPoint
		, std::vector<data_type>& dataVec
	) {
#ifdef DEBUG
      if(0 != std::get<1>(dataPoint)) {
         glogger
         << "In GBasePS::addDataPoint(mode 0): Error!" << std::endl
         << "Function was called for invalid mode " << std::get<1>(dataPoint) << std::endl
         << GEXCEPTION;
      }
#endif

		data_type   lData = std::get<0>(dataPoint);
		std::size_t lPos  = std::get<3>(dataPoint);

		// Check that we haven't exceeded the size of the boolean data vector
		if(lPos >= dataVec.size()) {
			glogger
			<< "In GBasePS::addDataPoint(): Error!" << std::endl
			<< "Got position beyond end of data vector: " << lPos << " / " << dataVec.size() << std::endl
			<< GEXCEPTION;
		}

		dataVec.at(lPos) = lData;
	}

	/***************************************************************************/
	/**
	 * Adds a given data point to a data map
	 */
	template <typename data_type>
	void addDataPoint(
		const std::tuple<data_type, std::size_t, std::string, std::size_t>& dataPoint
		, std::map<std::string, std::vector<data_type>>& dataMap
	) {
		data_type   lData = std::get<0>(dataPoint);
		std::string lName = std::get<2>(dataPoint);
		std::size_t lPos  = std::get<3>(dataPoint);

		(Gem::Common::getMapItem(dataMap, lName)).at(lPos) = lData;
	}

	/***************************************************************************/
	/** @brief Resets all parameter objects */
	void resetParameterObjects();
	/** @brief Adds new parameter sets to the population */
	void updateSelectedParameters();
	/** @brief Randomly shuffle the work items a number of times */
	void randomShuffle();
	/** @brief Retrieves the next available parameter set */
	std::shared_ptr<parSet> getParameterSet(std::size_t&);
	/** @brief Switches to the next parameter set */
	bool switchToNextParameterSet();
	/** @brief Fills all parameter objects into the allParVec_ vector */
	void fillAllParVec();
	/** @brief Clears the allParVec_ vector */
	void clearAllParVec();

	bool cycleLogicHalt_ = false; ///< Temporary flag used to specify that the optimization should be halted
	bool scanRandomly_ = true;   ///< Determines whether the algorithm should scan the parameter space randomly or on a grid
	std::size_t nMonitorInds_ = DEFAULTNMONITORINDS; ///< The number of best individuals of the entire run to be kept

	std::vector<std::shared_ptr<bScanPar>>      bVec_;     ///< Holds boolean parameters to be scanned
	std::vector<std::shared_ptr<int32ScanPar>>  int32Vec_; ///< Holds 32 bit integer parameters to be scanned
	std::vector<std::shared_ptr<dScanPar>>      dVec_;     ///< Holds double values to be scanned
	std::vector<std::shared_ptr<fScanPar>>      fVec_;     ///< Holds float values to be scanned

	std::vector<std::shared_ptr<scanParInterface>> allParVec_; /// Holds pointers to all parameter objects

	std::size_t simpleScanItems_ = 0; ///< When set to a value > 0, a random scan of the entire parameter space will be made instead of individual parameters -- set through the configuration file
	std::size_t scansPerformed_  = 0; ///< Holds the number of processed items so far while a simple scan is performed

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBasePS)

#endif /* GBASEPS_HPP_ */
