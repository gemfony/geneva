/**
 * @file GStarterIndividual.hpp
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

#ifndef GSTARTERINDIVIDUAL_HPP_
#define GSTARTERINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GMathHelperFunctionsT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "common/GParserBuilder.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum targetFunction {
	  PARABOLA=0
	, NOISYPARABOLA=1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::targetFunction&);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::targetFunction&);

/******************************************************************************/
// A number of default settings for the factory
const double GSI_DEF_ADPROB = 0.05;
const double GSI_DEF_SIGMA = 0.5;
const double GSI_DEF_SIGMASIGMA = 0.8;
const double GSI_DEF_MINSIGMA = 0.001;
const double GSI_DEF_MAXSIGMA = 2;
const targetFunction GO_DEF_TARGETFUNCTION = boost::numeric_cast<targetFunction>(0);

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GStarterIndividual : public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(targetFunction_);
	}

	///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GStarterIndividual();
	/** @brief A constructor that receives all arguments */
	GStarterIndividual(
      const std::size_t&
      , const std::vector<double>&
      , const std::vector<double>&
      , const std::vector<double>&
      , const double&
      , const double&
      , const double&
      , const double&
      , const double&
	);
	/** @brief A standard copy constructor */
	GStarterIndividual(const GStarterIndividual&);
	/** @brief The standard destructor */
	virtual ~GStarterIndividual();

	/** @brief A standard assignment operator */
	const GStarterIndividual& operator=(const GStarterIndividual&);

   /** @brief Checks for equality with another GFunctionIndividual object */
   bool operator==(const GStarterIndividual&) const;
   /** @brief Checks for inequality with another GFunctionIndividual object */
   bool operator!=(const GStarterIndividual& cp) const;

   /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled. */
   virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

	/** @brief Allows to set the demo function */
	void setTargetFunction(targetFunction);
	/** @brief Allows to retrieve the current demo function */
	targetFunction getTargetFunction() const;

	/** @brief Retrieves the average value of the sigma used in local Gauss adaptors */
	double getAverageSigma() const;

	/** @brief Emit information about this individual */
	std::string print() const;

	/***************************************************************************/
	/**
	 * This function is used to unify the setup from within the constructor
	 * and factory.
	 */
	static void addContent(
      GStarterIndividual& p
      , const std::size_t& prod_id
      , const std::vector<double>& startValues
      , const std::vector<double>& lowerBoundaries
      , const std::vector<double>& upperBoundaries
      , const double& sigma
      , const double& sigmaSigma
      , const double& minSigma
      , const double& maxSigma
      , const double& adProb
	) {
	   // Some error checking
#ifdef DEBUG
	   // Check whether values have been provided
	   if(startValues.empty()) {
	      std::cerr
	      << "In GStarterIndividual::addContent(): Error!" << std::endl
	      << "No parameters given" << std::endl;
	      std::terminate();
	   }

	   // Check whether all sizes match
	   if(startValues.size() != lowerBoundaries.size() || startValues.size() != upperBoundaries.size()) {
	      std::cerr
	      << "In GStarterIndividual::addContent(): Error!" << std::endl
	      << "Invalid sizes" << startValues.size() << " / " << lowerBoundaries.size() << " / " << upperBoundaries.size() << std::endl;
	      std::terminate();
	   }

	   // Check that start values and boundaries have valid values
	   for(std::size_t i=0; i<startValues.size(); i++) {
	      Gem::Common::checkValueRange( // We expect the start value to be in the range [lower, upper[
	            startValues.at(i)
	            , lowerBoundaries.at(i)
	            , upperBoundaries.at(i)
	            , false // closed lower boundary
	            , true  // open upper boundary
	      );
	   }

#endif /* DEBUG */

	   // Add the required number of GConstrainedDoubleObject objects to the individual
	   for(std::size_t i=0; i<startValues.size(); i++) {
	      boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr;
	      if(0 == prod_id) { // First individual, initialization with standard values
	         gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject> (
               new GConstrainedDoubleObject(
                      startValues.at(i)
                      , lowerBoundaries.at(i)
                      , upperBoundaries.at(i)
                )
	         );
	      } else { // Random initialization for all other individuals
            gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject> (
               new GConstrainedDoubleObject(
                      lowerBoundaries.at(i)
                      , upperBoundaries.at(i)
                )
            );
	      }

	      boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
	            new GDoubleGaussAdaptor(
	                  sigma
	                  , sigmaSigma
	                  , minSigma
	                  , maxSigma
	            )
	      );

	      gdga_ptr->setAdaptionProbability(adProb);
	      gcdo_ptr->addAdaptor(gdga_ptr);

	      p.push_back(gcdo_ptr);
	   }
	}

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GStarterIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual value calculation takes place here */
	virtual double fitnessCalculation();

	/***************************************************************************/

private:
	targetFunction targetFunction_; ///< Specifies which demo function should be used

	/***************************************************************************/
	/** @brief A simple n-dimensional parabola */
	double parabola(const std::vector<double>& parVec) const;
	/** @brief A "noisy" parabola */
	double noisyParabola(const std::vector<double>& parVec) const;

	/***************************************************************************/

public:
   /** @brief Applies modifications to this object. */
   virtual bool modify_GUnitTests();
   /** @brief Performs self tests that are expected to succeed. */
   virtual void specificTestsNoFailureExpected_GUnitTests();
   /** @brief Performs self tests that are expected to fail. */
   virtual void specificTestsFailuresExpected_GUnitTests();
};

/** @brief Allows to output a GStarterIndividual or convert it to a string using boost::lexical_cast */
std::ostream& operator<<(std::ostream&, const GStarterIndividual&);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GStarterIndividual objects
 */
class GStarterIndividualFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
public:
	/** @brief The standard constructor */
	GStarterIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GStarterIndividualFactory();

protected:
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to describe local configuration options in derived classes */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<GParameterSet>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GStarterIndividualFactory();

	double adProb_; ///< Probability for a parameter to be mutated
	double sigma_; ///< Step-width
	double sigmaSigma_; ///< Speed of sigma_-adaption
	double minSigma_; ///< Minimum allowed sigma value
	double maxSigma_; ///< Maximum allowed sigma value

	std::vector<double> startValues_; ///< Start values for all parameters
	std::vector<double> lowerBoundaries_; ///< Lower boundaries for all parameters
	std::vector<double> upperBoundaries_; ///< Upper boundaroes for all parameters
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStarterIndividual)

#endif /* GSTARTERINDIVIDUAL_HPP_ */
