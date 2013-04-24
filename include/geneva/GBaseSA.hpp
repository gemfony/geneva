/**
 * @file GBaseSA.hpp
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
#include <boost/tuple/tuple.hpp>

#ifndef GBASESA_HPP_
#define GBASESA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GSAPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

// Forward declaration
class GSAOptimizationMonitor;

/******************************************************************************/
/**
 * This is a specialization of the GBaseParChildT<ind_type> class for GParameterSet
 * objects. Almost all of Geneva's EA-algorithms will use this class as their
 * base class. The class also adds an infrastructure for simulated annealing
 * to the class.
 */
class GBaseSA
   :public GBaseParChildT<GParameterSet>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GBaseParChildT_GParameterSet", boost::serialization::base_object<GBaseParChildT<GParameterSet> >(*this))
      & BOOST_SERIALIZATION_NVP(t0_)
      & BOOST_SERIALIZATION_NVP(t_)
      & BOOST_SERIALIZATION_NVP(alpha_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GBaseSA();
   /** @brief A standard copy constructor */
   GBaseSA(const GBaseSA&);
   /** @brief The destructor */
   virtual ~GBaseSA();

   /** @brief A standard assignment operator */
   const GBaseSA& operator=(const GBaseSA&);

   /** @brief Checks for equality with another GBaseSA object */
   bool operator==(const GBaseSA&) const;
   /** @brief Checks for inequality with another GBaseSA object */
   bool operator!=(const GBaseSA&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const;

   /** @brief Returns information about the type of optimization algorithm */
   virtual personality_oa getOptimizationAlgorithm() const;

   /** @brief Returns the name of this optimization algorithm */
   virtual std::string getAlgorithmName() const;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions (
         Gem::Common::GParserBuilder& gpb
         , const bool& showOrigin
   );

   /** @brief Determines the strength of the temperature degradation */
   void setTDegradationStrength(double);
   /** @brief Retrieves the temperature degradation strength */
   double getTDegradationStrength() const;
   /** @brief Sets the start temperature */
   void setT0(double);
   /** @brief Retrieves the start temperature */
   double getT0() const;
   /** @brief Retrieves the current temperature */
   double getT() const;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const;

protected:
   /***************************************************************************/
   /** @brief Loads the data of another population */
   virtual void load_(const GObject *);
   /** @brief Creates a deep clone of this object */
   virtual GObject *clone_() const = 0;

   /** @brief Some error checks related to population sizes */
   virtual void populationSanityChecks() const;

   /** @brief Adapts all children of this population */
   virtual void adaptChildren() = 0;
   /** @brief Evaluates all children (and possibly parents) of this population */
   virtual void evaluateChildren() = 0;
   /** @brief Selects the best children of the population */
   virtual void selectBest();

   /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
   virtual boost::tuple<std::size_t,std::size_t> getEvaluationRange() const;

   /** @brief Does some preparatory work before the optimization starts */
   virtual void init();
   /** @brief Does any necessary finalization work */
   virtual void finalize();

   /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
   virtual boost::shared_ptr<GPersonalityTraits> getPersonalityTraits() const;

private:
   /** Performs a simulated annealing style sorting and selection */
   void sortSAMode();
   /** @brief Calculates the Simulated Annealing probability for a child to replace a parent */
   double saProb(const double&, const double&);
   /** @brief Updates the temperature (used for simulated annealing) */
   void updateTemperature();

   double t0_; ///< The start temperature, used in simulated annealing
   double t_; ///< The current temperature, used in simulated annealing
   double alpha_; ///< A constant used in the cooling schedule in simulated annealing

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
    * This nested class defines the interface of optimization monitors, as used
    * by default in the Geneva library for evolutionary algorithms.
    */
   class GSAOptimizationMonitor
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
      GSAOptimizationMonitor();
      /** @brief The copy constructor */
      GSAOptimizationMonitor(const GSAOptimizationMonitor&);
      /** @brief The destructor */
      virtual ~GSAOptimizationMonitor();

      /** @brief A standard assignment operator */
      const GSAOptimizationMonitor& operator=(const GSAOptimizationMonitor&);
      /** @brief Checks for equality with another GParameter Base object */
      virtual bool operator==(const GSAOptimizationMonitor&) const;
      /** @brief Checks for inequality with another GSAOptimizationMonitor object */
      virtual bool operator!=(const GSAOptimizationMonitor&) const;

      /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
      virtual boost::optional<std::string> checkRelationshipWith(
            const GObject&
            , const Gem::Common::expectation&
            , const double&
            , const std::string&
            , const std::string&
            , const bool&
      ) const;

      /** @brief Set the dimension of the output canvas */
      void setDims(const boost::uint16_t&, const boost::uint16_t&);
      /** @brief Retrieve the x-dimension of the output canvas */
      boost::uint16_t getXDim() const;
      /** @brief Retrieve the y-dimension of the output canvas */
      boost::uint16_t getYDim() const;

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
      boost::uint16_t xDim_;     ///< The dimension of the canvas in x-direction
      boost::uint16_t yDim_;     ///< The dimension of the canvas in y-direction
      std::size_t nMonitorInds_; ///< The number if individuals that should be monitored
      std::string resultFile_;     ///< The name of the file to which data is emitted

      std::vector<boost::shared_ptr<Gem::Common::GGraph2D> > fitnessGraphVec_;

     public:
      /** @brief Applies modifications to this object. This is needed for testing purposes */
      virtual bool modify_GUnitTests();
      /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
      virtual void specificTestsNoFailureExpected_GUnitTests();
      /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
      virtual void specificTestsFailuresExpected_GUnitTests();
     };

   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseSA)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseSA::GSAOptimizationMonitor)

#endif /* GBASESA_HPP_ */
