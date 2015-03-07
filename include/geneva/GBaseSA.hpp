/**
 * @file GBaseSA.hpp
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

// Boost headers go here
#include <boost/tuple/tuple.hpp>

#ifndef GBASESA_HPP_
#define GBASESA_HPP_

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
#include "geneva/GSAPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

// Forward declaration
class GSAOptimizationMonitor;

/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. The class adds
 * an infrastructure for simulated annealing (Geneva-style, i.e. with larger populations).
 */
class GBaseSA
   :public GParameterSetParChild
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GParameterSetParChild", boost::serialization::base_object<GParameterSetParChild >(*this))
      & BOOST_SERIALIZATION_NVP(t0_)
      & BOOST_SERIALIZATION_NVP(t_)
      & BOOST_SERIALIZATION_NVP(alpha_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief An easy identifier for the class */
   static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

   /** @brief The default constructor */
   G_API_GENEVA GBaseSA();
   /** @brief A standard copy constructor */
   G_API_GENEVA GBaseSA(const GBaseSA&);
   /** @brief The destructor */
   virtual G_API_GENEVA ~GBaseSA();

   /** @brief A standard assignment operator */
   G_API_GENEVA const GBaseSA& operator=(const GBaseSA&);

   /** @brief Checks for equality with another GBaseSA object */
   G_API_GENEVA bool operator==(const GBaseSA&) const;
   /** @brief Checks for inequality with another GBaseSA object */
   G_API_GENEVA bool operator!=(const GBaseSA&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Returns information about the type of optimization algorithm */
   virtual G_API_GENEVA std::string getOptimizationAlgorithm() const OVERRIDE;

   /** @brief Returns the name of this optimization algorithm */
   virtual G_API_GENEVA std::string getAlgorithmName() const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual G_API_GENEVA void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE;

   /** @brief Determines the strength of the temperature degradation */
   G_API_GENEVA void setTDegradationStrength(double);
   /** @brief Retrieves the temperature degradation strength */
   G_API_GENEVA double getTDegradationStrength() const;
   /** @brief Sets the start temperature */
   G_API_GENEVA void setT0(double);
   /** @brief Retrieves the start temperature */
   G_API_GENEVA double getT0() const;
   /** @brief Retrieves the current temperature */
   G_API_GENEVA double getT() const;

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
   /** @brief Evaluates all children (and possibly parents) of this population */
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
   virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
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
      G_API_GENEVA GSAOptimizationMonitor();
      /** @brief The copy constructor */
      G_API_GENEVA GSAOptimizationMonitor(const GSAOptimizationMonitor&);
      /** @brief The destructor */
      virtual G_API_GENEVA ~GSAOptimizationMonitor();

      /** @brief A standard assignment operator */
      G_API_GENEVA const GSAOptimizationMonitor& operator=(const GSAOptimizationMonitor&);
      /** @brief Checks for equality with another GParameter Base object */
      virtual G_API_GENEVA bool operator==(const GSAOptimizationMonitor&) const;
      /** @brief Checks for inequality with another GSAOptimizationMonitor object */
      virtual G_API_GENEVA bool operator!=(const GSAOptimizationMonitor&) const;

      /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
      virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
      ) const OVERRIDE;

      /** @brief Set the dimension of the output canvas */
      G_API_GENEVA void setDims(const boost::uint16_t&, const boost::uint16_t&);
      /** @brief Retrieve the x-dimension of the output canvas */
      G_API_GENEVA boost::uint16_t getXDim() const;
      /** @brief Retrieve the y-dimension of the output canvas */
      G_API_GENEVA boost::uint16_t getYDim() const;

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
      boost::uint16_t xDim_;     ///< The dimension of the canvas in x-direction
      boost::uint16_t yDim_;     ///< The dimension of the canvas in y-direction
      std::size_t nMonitorInds_; ///< The number if individuals that should be monitored
      std::string resultFile_;     ///< The name of the file to which data is emitted

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

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseSA)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseSA::GSAOptimizationMonitor)

#endif /* GBASESA_HPP_ */
