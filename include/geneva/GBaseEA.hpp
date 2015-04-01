/**
 * @file GBaseEA.hpp
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
#include <vector>

// Boost headers go here
#include <boost/tuple/tuple.hpp>

#ifndef GBASEEA_HPP_
#define GBASEEA_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GParameterSetParChild.hpp"
#include "geneva/GEAPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

// Forward declaration
class GEAOptimizationMonitor;

/**
 * The default sorting mode
 */
const sortingMode DEFAULTSMODE = MUCOMMANU_SINGLEEVAL;

/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. It provides the
 * main infrastructure for evolutionary algorithms (except those that deal with
 * multi-populations).
 */
class GBaseEA
   :public GParameterSetParChild
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GParameterSetParChild", boost::serialization::base_object<GParameterSetParChild>(*this))
      & BOOST_SERIALIZATION_NVP(smode_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief An easy identifier for the class */
   static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

   /** @brief The default constructor */
   G_API_GENEVA GBaseEA();
   /** @brief A standard copy constructor */
   G_API_GENEVA GBaseEA(const GBaseEA&);
   /** @brief The destructor */
   virtual G_API_GENEVA ~GBaseEA();

   /** @brief The standard assignment operator */
   G_API_GENEVA const GBaseEA& operator=(const GBaseEA&);

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const OVERRIDE;

   /** @brief Returns information about the type of optimization algorithm */
   virtual G_API_GENEVA std::string getOptimizationAlgorithm() const OVERRIDE;

   /** @brief Set the sorting scheme for this population */
   G_API_GENEVA void setSortingScheme(sortingMode);
   /** @brief Retrieve the current sorting scheme for this population */
   G_API_GENEVA sortingMode getSortingScheme() const;

   /** @brief Extracts all individuals on the pareto front */
   G_API_GENEVA void extractCurrentParetoIndividuals(std::vector<boost::shared_ptr<Gem::Geneva::GParameterSet> >&);

   /** @brief Adds the individuals of this iteration to a priority queue */
   G_API_GENEVA void addIterationBests(GParameterSetFixedSizePriorityQueue&);

   /** @brief Returns the name of this optimization algorithm */
   virtual G_API_GENEVA std::string getAlgorithmName() const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual G_API_GENEVA void addConfigurationOptions (
         Gem::Common::GParserBuilder& gpb
   ) OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
   /***************************************************************************/
   /** @brief Loads the data of another population */
   virtual G_API_GENEVA void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual G_API_GENEVA GObject *clone_() const = 0;

   /** @brief Some error checks related to population sizes */
   virtual G_API_GENEVA void populationSanityChecks() const OVERRIDE;

   /** @brief Adapts all children of this population */
   virtual G_API_GENEVA void adaptChildren() = 0;
   /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
   virtual G_API_GENEVA void runFitnessCalculation() = 0;
   /** @brief Selects the best children of the population */
   virtual G_API_GENEVA void selectBest() OVERRIDE;

   /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
   virtual G_API_GENEVA boost::tuple<std::size_t,std::size_t> getEvaluationRange() const OVERRIDE;

   /** @brief Does some preparatory work before the optimization starts */
   virtual G_API_GENEVA void init() OVERRIDE;
   /** @brief Does any necessary finalization work */
   virtual G_API_GENEVA void finalize() OVERRIDE;

   /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
   virtual G_API_GENEVA boost::shared_ptr<GPersonalityTraits> getPersonalityTraits() const OVERRIDE;

private:
   /***************************************************************************/
   /**
    * A simple comparison operator that helps to sort individuals according to their
    * pareto status
    */
   class indParetoComp {
   public:
      bool operator()(boost::shared_ptr<GParameterSet> x, boost::shared_ptr<GParameterSet> y) {
         return x->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront() > y->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront();
      }
   };

   /***************************************************************************/
   /** @brief Selection according to the pareto tag in MUPLUSNU mode (i.e. taking into account the parents) */
   void sortMuPlusNuParetoMode();
   /** @brief Selection according to the pareto tag in MUCOMMANU mode (i.e. not taking into account the parents) */
   void sortMuCommaNuParetoMode();
   /** @brief Determines whether the first individual dominates the second */
   bool aDominatesB(boost::shared_ptr<GParameterSet>, boost::shared_ptr<GParameterSet>) const;

   /***************************************************************************/
   // Local data

   sortingMode smode_; ///< The chosen sorting scheme

public:
   /***************************************************************************/
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
   /** @brief Fills the collection with individuals */
   G_API_GENEVA void fillWithObjects(const std::size_t& = 10);
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;

public:
   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
   /**
    * This nested class defines the interface of optimization monitors, as used
    * by default in the Geneva library for evolutionary algorithms.
    */
   class GEAOptimizationMonitor
   : public GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT
     {
      ///////////////////////////////////////////////////////////////////////
      friend class boost::serialization::access;

      template<typename Archive>
      void serialize(Archive & ar, const unsigned int){
         using boost::serialization::make_nvp;

         ar
         & make_nvp("GOptimizationMonitorT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(*this))
         & BOOST_SERIALIZATION_NVP(xDim_)
         & BOOST_SERIALIZATION_NVP(yDim_)
         & BOOST_SERIALIZATION_NVP(nMonitorInds_)
         & BOOST_SERIALIZATION_NVP(resultFile_);
      }
      ///////////////////////////////////////////////////////////////////////

     public:
      /** @brief The default constructor */
      G_API_GENEVA GEAOptimizationMonitor();
      /** @brief The copy constructor */
      G_API_GENEVA GEAOptimizationMonitor(const GEAOptimizationMonitor&);
      /** @brief The destructor */
      virtual G_API_GENEVA ~GEAOptimizationMonitor();

      /** @brief The standard assignment operator */
      const GEAOptimizationMonitor& operator=(const GEAOptimizationMonitor&);

      /** @brief Checks for equality with another GParameter Base object */
      virtual G_API_GENEVA bool operator==(const GEAOptimizationMonitor&) const;
      /** @brief Checks for inequality with another GEAOptimizationMonitor object */
      virtual G_API_GENEVA bool operator!=(const GEAOptimizationMonitor&) const;

      /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
      virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
      ) const OVERRIDE;

      /** @brief Searches for compliance with expectations with respect to another object of the same type */
      virtual G_API_GENEVA void compare(
         const GObject& // the other object
         , const Gem::Common::expectation& // the expectation for this object, e.g. equality
         , const double& // the limit for allowed deviations of floating point types
      ) const OVERRIDE;

      /** @brief Set the dimension of the output canvas */
      G_API_GENEVA void setDims(const boost::uint32_t&, const boost::uint32_t&);
      /** @brief Retrieve the dimensions as a tuple */
      G_API_GENEVA boost::tuple<boost::uint32_t, boost::uint32_t> getDims() const;
      /** @brief Retrieve the x-dimension of the output canvas */
      G_API_GENEVA boost::uint32_t getXDim() const;
      /** @brief Retrieve the y-dimension of the output canvas */
      G_API_GENEVA boost::uint32_t getYDim() const;

      /** @brief Sets the number of individuals in the population that should be monitored */
      G_API_GENEVA void setNMonitorIndividuals(const std::size_t&);
      /** @brief Retrieves the number of individuals that are being monitored */
      G_API_GENEVA std::size_t getNMonitorIndividuals() const;

      /** @brief Allows to set the name of the result file */
      G_API_GENEVA void setResultFileName(const std::string&);
      /** @brief Allows to retrieve the name of the result file */
      G_API_GENEVA std::string getResultFileName() const;

     protected:
      /** @brief A function that is called once before the optimization starts */
      virtual G_API_GENEVA void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;
      /** @brief A function that is called during each optimization cycle */
      virtual G_API_GENEVA void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;
      /** @brief A function that is called once at the end of the optimization cycle */
      virtual G_API_GENEVA void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;

      /** @brief Loads the data of another object */
      virtual G_API_GENEVA void load_(const GObject*) OVERRIDE;
      /** @brief Creates a deep clone of this object */
      virtual G_API_GENEVA GObject* clone_() const OVERRIDE;

     private:
      boost::uint32_t xDim_; ///< The dimension of the canvas in x-direction
      boost::uint32_t yDim_; ///< The dimension of the canvas in y-direction
      std::size_t nMonitorInds_; ///< The number if individuals that should be monitored
      std::string resultFile_; ///< The name of the file to which data is emitted

      std::vector<boost::shared_ptr<Gem::Common::GGraph2D> > fitnessGraphVec_;

     public:
      /** @brief Applies modifications to this object. This is needed for testing purposes */
      virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
      /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
      virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
      /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
      virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
     };

   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseEA)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseEA::GEAOptimizationMonitor)

#endif /* GBASEEA_HPP_ */
