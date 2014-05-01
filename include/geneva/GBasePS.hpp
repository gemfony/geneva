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

// Standard headers go here
#include <fstream>

// Boost headers go here

#ifndef GBASEPS_HPP_
#define GBASEPS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
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

// Forward declaration
class GPSOptimizationMonitor;

/******************************************************************************/
// A number of typedefs that indicate the position and value of a parameter inside of an individual
typedef boost::tuple<bool,           std::size_t, std::string, std::size_t> singleBPar;
typedef boost::tuple<boost::int32_t, std::size_t, std::string, std::size_t> singleInt32Par;
typedef boost::tuple<float,          std::size_t, std::string, std::size_t> singleFPar;
typedef boost::tuple<double,         std::size_t, std::string, std::size_t> singleDPar;

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
std::ostream& operator<<(std::ostream& os, const parSet& pS);

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
      & make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet> >(*this))
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
   /** @brief An easy identifier for the class */
   static const std::string nickname; // Initialized in the .cpp definition file

   /** @brief The default constructor */
   GBasePS();
   /** @brief A standard copy constructor */
   GBasePS(const GBasePS&);
   /** @brief The destructor */
   virtual ~GBasePS();

   /** @brief A standard assignment operator */
   const GBasePS& operator=(const GBasePS&);

   /** @brief Checks for equality with another GBasePS object */
   bool operator==(const GBasePS&) const;
   /** @brief Checks for inequality with another GBasePS object */
   bool operator!=(const GBasePS&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Loads a checkpoint */
   virtual void loadCheckpoint(const std::string&) OVERRIDE;

   /** @brief Returns information about the type of optimization algorithm */
   virtual std::string getOptimizationAlgorithm() const OVERRIDE;

   /** @brief Retrieves the number of processable items for the current iteration */
   virtual std::size_t getNProcessableItems() const OVERRIDE;

   /** @brief Returns the name of this optimization algorithm */
   virtual std::string getAlgorithmName() const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

   /** @brief Allows to set the number of "best" individuals to be monitored over the course of the algorithm run */
   void setNMonitorInds(std::size_t);
   /** @brief Allows to retrieve  the number of "best" individuals to be monitored over the course of the algorithm run */
   std::size_t getNMonitorInds() const;

   /** @brief Fills vectors with parameter specifications */
   void setParameterSpecs(std::string);

   /** @brief Puts the class in "simple scan" mode */
   void setNSimpleScans(std::size_t);
   /** @brief Retrieves the number of simple scans (or 0, if disabled) */
   std::size_t getNSimpleScans() const;
   /** @brief Retrieves the number of scans performed so far */
   std::size_t getNScansPerformed() const;

   /** @brief Allows to specify whether the parameter space should be scanned randomly or on a grid */
   void setScanRandomly(bool);
   /** @brief Allows to check whether the parameter space should be scanned randomly or on a grid */
   bool getScanRandomly() const;

protected:
   /***************************************************************************/
   /** @brief Loads the data of another population */
   virtual void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual GObject *clone_() const = 0;

   /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
   virtual boost::tuple<double, double> cycleLogic() OVERRIDE;
   /** @brief Does some preparatory work before the optimization starts */
   virtual void init() OVERRIDE;
   /** @brief Does any necessary finalization work */
   virtual void finalize() OVERRIDE;

   /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
   virtual boost::shared_ptr<GPersonalityTraits> getPersonalityTraits() const OVERRIDE;

   /** @brief Resizes the population to the desired level and does some error checks */
   virtual void adjustPopulation() OVERRIDE;

   /** @brief Saves the state of the class to disc. */
   virtual void saveCheckpoint() const OVERRIDE;

   /** @brief Triggers fitness calculation of a number of individuals */
   virtual void runFitnessCalculation() = 0;

   /** @brief A custom halt criterion for the optimization, allowing to stop the loop when no items are left to be scanned */
   virtual bool customHalt() const OVERRIDE;

private:
   /***************************************************************************/
   /**
    * Adds a given data point to a data vector
    */
   template <typename data_type>
   void addDataPoint(
      const boost::tuple<data_type, std::size_t, std::string, std::size_t>& dataPoint
      , std::vector<data_type>& dataVec
   ) {
#ifdef DEBUG
      if(0 != boost::get<1>(dataPoint)) {
         glogger
         << "In GBasePS::addDataPoint(mode 0): Error!" << std::endl
         << "Function was called for invalid mode " << boost::get<1>(dataPoint) << std::endl
         << GEXCEPTION;
      }
#endif

      data_type   lData = boost::get<0>(dataPoint);
      std::size_t lPos  = boost::get<3>(dataPoint);

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
         const boost::tuple<data_type, std::size_t, std::string, std::size_t>& dataPoint
         , std::map<std::string, std::vector<data_type> >& dataMap
   ) {
      data_type   lData = boost::get<0>(dataPoint);
      std::string lName = boost::get<2>(dataPoint);
      std::size_t lPos  = boost::get<3>(dataPoint);

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
   boost::shared_ptr<parSet> getParameterSet(std::size_t&);
   /** @brief Switches to the next parameter set */
   bool switchToNextParameterSet();
   /** @brief Sorts the population according to the primary fitness values */
   void sortPopulation();
   /** @brief Fills all parameter objects into the allParVec_ vector */
   void fillAllParVec();
   /** @brief Clears the allParVec_ vector */
   void clearAllParVec();

   bool cycleLogicHalt_; ///< Temporary flag used to specify that the optimization should be halted
   bool scanRandomly_;   ///< Determines whether the algorithm should scan the parameter space randomly or on a grid
   std::size_t nMonitorInds_; ///< The number of best individuals of the entire run to be kept

   std::vector<boost::shared_ptr<bScanPar> >      bVec_;     ///< Holds boolean parameters to be scanned
   std::vector<boost::shared_ptr<int32ScanPar> >  int32Vec_; ///< Holds 32 bit integer parameters to be scanned
   std::vector<boost::shared_ptr<dScanPar> >      dVec_;     ///< Holds double values to be scanned
   std::vector<boost::shared_ptr<fScanPar> >      fVec_;     ///< Holds float values to be scanned

   std::vector<boost::shared_ptr<scanParInterface> > allParVec_; /// Holds pointers to all parameter objects

   std::size_t simpleScanItems_; ///< When set to a value > 0, a random scan of the entire parameter space will be made instead of individual parameters -- set through the configuration file
   std::size_t scansPerformed_;  ///< Holds the number of processed items so far while a simple scan is performed

public:
   /***************************************************************************/
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual bool modify_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;

public:
   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
   /**
    * This class defines the interface of optimization monitors, as used
    * by default in the Geneva library for evolutionary algorithms.
    */
   class GPSOptimizationMonitor
      :public GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT
   {
      ///////////////////////////////////////////////////////////////////////
      friend class boost::serialization::access;

      template<typename Archive>
      void serialize(Archive & ar, const unsigned int){
         using boost::serialization::make_nvp;

         ar
         & make_nvp("GOptimizationMonitorT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(*this))
         & BOOST_SERIALIZATION_NVP(csvResultFile_);
      }
      ///////////////////////////////////////////////////////////////////////

     public:
      /** @brief The default constructor */
      GPSOptimizationMonitor();
      /** @brief The copy constructor */
      GPSOptimizationMonitor(const GPSOptimizationMonitor&);
      /** @brief The destructor */
      virtual ~GPSOptimizationMonitor();

      /** @brief A standard assignment operator */
      const GPSOptimizationMonitor& operator=(const GPSOptimizationMonitor&);
      /** @brief Checks for equality with another GParameter Base object */
      virtual bool operator==(const GPSOptimizationMonitor&) const;
      /** @brief Checks for inequality with another GPSOptimizationMonitor object */
      virtual bool operator!=(const GPSOptimizationMonitor&) const;

      /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
      virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
      ) const OVERRIDE;

      /** @brief Allows to set the name of the result file */
      void setCSVResultFileName(const std::string&);
      /** @brief Allows to retrieve the name of the result file */
      std::string getCSVResultFileName() const;
      /** @brief Allows to specify whether explanations should be printe
       * d for parameter- and fitness values. */
      void setPrintWithNameAndType(bool);
      /** @brief Allows to check whether explanations should be printed for parameter- and fitness values */
      bool getPrintWithNameAndType() const;

      /** @brief Allows to specify whether commas should be printed in-between values */
      void setPrintWithCommas(bool);
      /** @brief Allows to check whether commas should be printed in-between values */
      bool getPrintWithCommas() const;

      /** @brief Allows to specify whether the true (instead of the transformed) fitness should be shown */
      void setUseTrueFitness(bool);
      /** @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown */
      bool getUseTrueFitness() const;

      /** @brief Allows to specify whether the validity of a solution should be shown */
      void setShowValidity(bool);
      /** @brief Allows to check whether the validity of a solution will be shown */
      bool getShowValidity() const;

     protected:
      /** @brief A function that is called once before the optimization starts */
      virtual void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;
      /** @brief A function that is called during each optimization cycle */
      virtual void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;
      /** @brief A function that is called once at the end of the optimization cycle */
      virtual void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;

      /** @brief Loads the data of another object */
      virtual void load_(const GObject*) OVERRIDE;
      /** @brief Creates a deep clone of this object */
      virtual GObject* clone_() const OVERRIDE;

     private:
      std::string csvResultFile_; ///< The name of the file to which data is emitted
      bool withNameAndType_; ///< When set to true, explanations for values are printed
      bool withCommas_; ///< When set to true, commas will be printed in-between values
      bool useRawFitness_; ///< Indicates whether true- or transformed fitness should be output
      bool showValidity_; ///< Indicates whether the validity of a solution should be shown

     public:
      /** @brief Applies modifications to this object. This is needed for testing purposes */
      virtual bool modify_GUnitTests() OVERRIDE;
      /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
      virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
      /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
      virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;

      /************************************************************************/
   };

   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBasePS)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBasePS::GPSOptimizationMonitor)

#endif /* GBASEPS_HPP_ */
