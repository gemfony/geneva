/**
 * @file GBaseParameterScan.hpp
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

// Boost headers go here

#ifndef GBASEPARAMETERSCAN_HPP_
#define GBASEPARAMETERSCAN_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GScanPar.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GEAPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva-individuals/GFunctionIndividual.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

// Forward declaration
class GParameterScanOptimizationMonitor;

/******************************************************************************/
// A number of typedefs that indicate the position and value of a parameter
// inside of an individual
typedef boost::tuple<bool,           std::size_t> singleBPar;
typedef boost::tuple<boost::int32_t, std::size_t> singleInt32Par;
typedef boost::tuple<float,          std::size_t> singleFPar;
typedef boost::tuple<double,         std::size_t> singleDPar;

/******************************************************************************/
/**
 * This struct holds the entire data to be updated inside of an individual
 */
struct parSet {
  std::vector<singleBPar> bParVec;
  std::vector<singleInt32Par> iParVec;
  std::vector<singleFPar> fParVec;
  std::vector<singleDPar> dParVec;
};

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
class GBaseParameterScan
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
      & BOOST_SERIALIZATION_NVP(bVec_)
      & BOOST_SERIALIZATION_NVP(int32Vec_)
      & BOOST_SERIALIZATION_NVP(dVec_)
      & BOOST_SERIALIZATION_NVP(fVec_);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GBaseParameterScan();
   /** @brief A standard copy constructor */
   GBaseParameterScan(const GBaseParameterScan&);
   /** @brief The destructor */
   virtual ~GBaseParameterScan();

   /** @brief A standard assignment operator */
   const GBaseParameterScan& operator=(const GBaseParameterScan&);

   /** @brief Checks for equality with another GBaseParameterScan object */
   bool operator==(const GBaseParameterScan&) const;
   /** @brief Checks for inequality with another GBaseParameterScan object */
   bool operator!=(const GBaseParameterScan&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const;

   /** @brief Loads a checkpoint */
   virtual void loadCheckpoint(const std::string&);

   /** @brief Returns information about the type of optimization algorithm */
   virtual personality_oa getOptimizationAlgorithm() const;

   /** @brief Retrieves the number of processable items for the current iteration */
   virtual std::size_t getNProcessableItems() const;

   /** @brief Returns the name of this optimization algorithm */
   virtual std::string getAlgorithmName() const;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions (
         Gem::Common::GParserBuilder& gpb
         , const bool& showOrigin
   );

   /** @brief Emits a name for this class / object */
   virtual std::string name() const;

protected:
   /***************************************************************************/
   /** @brief Loads the data of another population */
   virtual void load_(const GObject *);
   /** @brief Creates a deep clone of this object */
   virtual GObject *clone_() const = 0;

   /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
   virtual double cycleLogic();
   /** @brief Does some preparatory work before the optimization starts */
   virtual void init();
   /** @brief Does any necessary finalization work */
   virtual void finalize();
   /** @brief Performs final optimization work */
   // virtual void optimizationFinalize();

   /** @brief Resizes the population to the desired level and does some error checks */
   virtual void adjustPopulation();

   /** @brief Saves the state of the class to disc. */
   virtual void saveCheckpoint() const;

   /** @brief Triggers fitness calculation of a number of individuals */
   virtual double doFitnessCalculation(const std::size_t&) = 0;

   /** @brief Retrieves the best individual found */
   virtual boost::shared_ptr<GIndividual> getBestIndividual();
   /** @brief Retrieves a list of the best individuals found */
   virtual std::vector<boost::shared_ptr<GIndividual> > getBestIndividuals();

   /** @brief A custom halt criterion for the optimization, allowing to stop the loop when no items are left to be scanned */
   virtual bool customHalt() const;

private:
   /***************************************************************************/
   /**
    * Adds a given data point to a data vector
    */
   template <typename dType>
   void addDataPoint(
         const boost::tuple<dType, std::size_t>& dataPoint
         , std::vector<dType> dataVec
   ) {
      dType       lData = boost::get<0>(dataPoint);
      std::size_t lPos  = boost::get<1>(dataPoint);

#ifdef DEBUG
         // Check that we haven't exceeded the size of the boolean data vector
         if(lPos >= dataVec.size()) {
            glogger
            << "In GBaseParameterScan::addDataPoint(): Error!" << std::endl
            << "Got position beyond end of data vector: " << lPos << " / " << dataVec.size() << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */

         dataVec.at(lPos) = lData;
   }

   /***************************************************************************/
   /** @brief Resets all parameter objects */
   void resetParameterObjects();
   /** @brief Adds new parameter sets to the population */
   void updateIndividuals();
   /** @brief Retrieves the next available parameter set */
   boost::shared_ptr<parSet> getParameterSet();
   /** @brief Switches to the next parameter set */
   bool switchToNextParameterSet();
   /** @brief Sorts the population according to the primary fitness values */
   void sortPopulation();
   /** @brief Fills all parameter objects into the allParVec_ vector */
   void fillAllParVec();
   /** @brief Clears the allParVec_ vector */
   void clearAllParVec();
   /** @brief Fills vectors with parameter values */
   void parseParameterValues(std::vector<std::string>);

   bool cycleLogicHalt_; ///< Temporary flag used to specify that the optimization should be halted
   bool scanRandomly_;   ///< Determines whether the algorithm should scan the parameter space randomly or on a grid

   std::vector<boost::shared_ptr<bScanPar> >     bVec_; ///< Holds boolean parameters to be scanned
   std::vector<boost::shared_ptr<int32ScanPar> > int32Vec_; ///< Holds 32 bit integer parameters to be scanned
   std::vector<boost::shared_ptr<dScanPar> >     dVec_; ///< Holds double values to be scanned
   std::vector<boost::shared_ptr<fScanPar> >     fVec_; ///< Holds float values to be scanned

   std::vector<boost::shared_ptr<scanParI> >     allParVec_; /// Holds pointers to all parameter objects

public:
   /***************************************************************************/
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual bool modify_GUnitTests();
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual void specificTestsNoFailureExpected_GUnitTests();
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual void specificTestsFailuresExpected_GUnitTests();

public:
   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
   /**
    * This class defines the interface of optimization monitors, as used
    * by default in the Geneva library for evolutionary algorithms.
    */
   class GParameterScanOptimizationMonitor
      :public GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT
   {
      ///////////////////////////////////////////////////////////////////////
      friend class boost::serialization::access;

      template<typename Archive>
      void serialize(Archive & ar, const unsigned int){
         using boost::serialization::make_nvp;

         ar
         & make_nvp("GOptimizationMonitorT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(*this))
         & BOOST_SERIALIZATION_NVP(resultFile_);
      }
      ///////////////////////////////////////////////////////////////////////

     public:
      /** @brief The default constructor */
      GParameterScanOptimizationMonitor();
      /** @brief The copy constructor */
      GParameterScanOptimizationMonitor(const GParameterScanOptimizationMonitor&);
      /** @brief The destructor */
      virtual ~GParameterScanOptimizationMonitor();

      /** @brief A standard assignment operator */
      const GParameterScanOptimizationMonitor& operator=(const GParameterScanOptimizationMonitor&);
      /** @brief Checks for equality with another GParameter Base object */
      virtual bool operator==(const GParameterScanOptimizationMonitor&) const;
      /** @brief Checks for inequality with another GParameterScanOptimizationMonitor object */
      virtual bool operator!=(const GParameterScanOptimizationMonitor&) const;

      /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
      virtual boost::optional<std::string> checkRelationshipWith(
            const GObject&
            , const Gem::Common::expectation&
            , const double&
            , const std::string&
            , const std::string&
            , const bool&
      ) const;

      /** @brief Allows to set the name of the result file */
      void setResultFileName(const std::string&);
      /** @brief Allows to retrieve the name of the result file */
      std::string getResultFileName() const;

     protected:
      /** @brief A function that is called once before the optimization starts */
      virtual void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const);
      /** @brief A function that is called during each optimization cycle */
      virtual void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const);
      /** @brief A function that is called once at the end of the optimization cycle */
      virtual void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const);

      /** @brief Loads the data of another object */
      virtual void load_(const GObject*);
      /** @brief Creates a deep clone of this object */
      virtual GObject* clone_() const;

     private:
      std::string resultFile_; ///< The name of the file to which data is emitted

     public:
      /** @brief Applies modifications to this object. This is needed for testing purposes */
      virtual bool modify_GUnitTests();
      /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
      virtual void specificTestsNoFailureExpected_GUnitTests();
      /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
      virtual void specificTestsFailuresExpected_GUnitTests();

      /************************************************************************/
   };

   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseParameterScan)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseParameterScan::GParameterScanOptimizationMonitor)

#endif /* GBASEPARAMETERSCAN_HPP_ */
