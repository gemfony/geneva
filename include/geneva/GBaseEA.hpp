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

// Standard headers go here
#include <vector>

// Boost headers go here
#include <boost/tuple/tuple.hpp>

#ifndef GBASEEA_HPP_
#define GBASEEA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
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
 * This is a specialization of the GBaseParChildT<ind_type> class for GParameterSet
 * objects. Almost all of Geneva's EA-algorithms will use this class as their
 * base class. The class also adds an infrastructure for simulated annealing
 * to the class.
 */
class GBaseEA
   :public GBaseParChildT<GParameterSet>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GBaseParChildT_GParameterSet", boost::serialization::base_object<GBaseParChildT<GParameterSet> >(*this))
      & BOOST_SERIALIZATION_NVP(smode_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief An easy identifier for the class */
   static const std::string nickname; // Initialized in the .cpp definition file

   /** @brief The default constructor */
   GBaseEA();
   /** @brief A standard copy constructor */
   GBaseEA(const GBaseEA&);
   /** @brief The destructor */
   virtual ~GBaseEA();

   /** @brief A standard assignment operator */
   const GBaseEA& operator=(const GBaseEA&);

   /** @brief Checks for equality with another GBaseEA object */
   bool operator==(const GBaseEA&) const;
   /** @brief Checks for inequality with another GBaseEA object */
   bool operator!=(const GBaseEA&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const OVERRIDE;

   /** @brief Returns information about the type of optimization algorithm */
   virtual std::string getOptimizationAlgorithm() const OVERRIDE;

   /** @brief Set the sorting scheme for this population */
   void setSortingScheme(sortingMode);
   /** @brief Retrieve the current sorting scheme for this population */
   sortingMode getSortingScheme() const;

   /** @brief Returns the name of this optimization algorithm */
   virtual std::string getAlgorithmName() const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions (
         Gem::Common::GParserBuilder& gpb
         , const bool& showOrigin
   ) OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
   /***************************************************************************/
   /** @brief Loads the data of another population */
   virtual void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual GObject *clone_() const = 0;

   /** @brief Some error checks related to population sizes */
   virtual void populationSanityChecks() const OVERRIDE;

   /** @brief Adapts all children of this population */
   virtual void adaptChildren() = 0;
   /** @brief Evaluates all children (and possibly parents) of this population */
   virtual void evaluateChildren() = 0;
   /** @brief Selects the best children of the population */
   virtual void selectBest() OVERRIDE;

   /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
   virtual boost::tuple<std::size_t,std::size_t> getEvaluationRange() const OVERRIDE;

   /** @brief Does some preparatory work before the optimization starts */
   virtual void init() OVERRIDE;
   /** @brief Does any necessary finalization work */
   virtual void finalize() OVERRIDE;

   /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
   virtual boost::shared_ptr<GPersonalityTraits> getPersonalityTraits() const OVERRIDE;

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
   virtual bool modify_GUnitTests() OVERRIDE;
   /** @brief Fills the collection with individuals */
   void fillWithObjects(const std::size_t& = 10);
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;

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
      GEAOptimizationMonitor();
      /** @brief The copy constructor */
      GEAOptimizationMonitor(const GEAOptimizationMonitor&);
      /** @brief The destructor */
      virtual ~GEAOptimizationMonitor();

      /** @brief A standard assignment operator */
      const GEAOptimizationMonitor& operator=(const GEAOptimizationMonitor&);
      /** @brief Checks for equality with another GParameter Base object */
      virtual bool operator==(const GEAOptimizationMonitor&) const;
      /** @brief Checks for inequality with another GEAOptimizationMonitor object */
      virtual bool operator!=(const GEAOptimizationMonitor&) const;

      /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
      virtual boost::optional<std::string> checkRelationshipWith(
            const GObject&
            , const Gem::Common::expectation&
            , const double&
            , const std::string&
            , const std::string&
            , const bool&
      ) const OVERRIDE;

      /** @brief Set the dimension of the output canvas */
      void setDims(const boost::uint32_t&, const boost::uint32_t&);
      /** @brief Retrieve the dimensions as a tuple */
      boost::tuple<boost::uint32_t, boost::uint32_t> getDims() const;
      /** @brief Retrieve the x-dimension of the output canvas */
      boost::uint32_t getXDim() const;
      /** @brief Retrieve the y-dimension of the output canvas */
      boost::uint32_t getYDim() const;

      /** @brief Sets the number of individuals in the population that should be monitored */
      void setNMonitorIndividuals(const std::size_t&);
      /** @brief Retrieves the number of individuals that are being monitored */
      std::size_t getNMonitorIndividuals() const;

      /** @brief Allows to set the name of the result file */
      void setResultFileName(const std::string&);
      /** @brief Allows to retrieve the name of the result file */
      std::string getResultFileName() const;

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
      boost::uint32_t xDim_; ///< The dimension of the canvas in x-direction
      boost::uint32_t yDim_; ///< The dimension of the canvas in y-direction
      std::size_t nMonitorInds_; ///< The number if individuals that should be monitored
      std::string resultFile_; ///< The name of the file to which data is emitted

      std::vector<boost::shared_ptr<Gem::Common::GGraph2D> > fitnessGraphVec_;

     public:
      /** @brief Applies modifications to this object. This is needed for testing purposes */
      virtual bool modify_GUnitTests() OVERRIDE;
      /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
      virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
      /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
      virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
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
