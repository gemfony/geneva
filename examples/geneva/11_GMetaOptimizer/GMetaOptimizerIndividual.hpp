/**
 * @file GMetaOptimizerIndividual.hpp
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

#ifndef GMETAOPTIMIZERINDIVIDUAL_HPP_
#define GMETAOPTIMIZERINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GMathHelperFunctionsT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// A number of default settings for the factory and individual

// Pertaining to the population
const std::size_t     GMETAOPT_DEF_INITNPARENTS = 1;        ///< The initial number of parents
const std::size_t     GMETAOPT_DEF_NPARENTS_LB = 1;         ///< The lower boundary for variations of the number of parents
const std::size_t     GMETAOPT_DEF_NPARENTS_UB = 6;         ///< The upper boundary for variations of the number of parents

const std::size_t     GMETAOPT_DEF_INITNCHILDREN = 100;     ///< The initial number of children
const std::size_t     GMETAOPT_DEF_NCHILDREN_LB = 5;        ///< The lower boundary for the variation of the number of children
const std::size_t     GMETAOPT_DEF_NCHILDREN_UB = 250;      ///< The upper boundary for the variation of the number of children

const double          GMETAOPT_DEF_INITAMALGLKLHOOD = 0.;      ///< The initial likelihood for an individual being created from cross-over rather than "just" duplication | !!! NEW
const double          GMETAOPT_DEF_AMALGLKLHOOD_LB = 0.;       ///< The lower boundary for the variation of the amalgamation likelihood | !!! NEW
const double          GMETAOPT_DEF_AMALGLKLHOOD_UB = 1.;       ///< The upper boundary for the variation of the amalgamation likelihood | !!! NEW

// Concerning the individual
const double          GMETAOPT_DEF_INITMINADPROB = 0.;         ///< The initial lower boundary for the variation of adProb
const double          GMETAOPT_DEF_MINADPROB_LB = 0.;          ///< The lower boundary for minAdProb
const double          GMETAOPT_DEF_MINADPROB_UB = 0.1;         ///< The upper boundary for minAdProb -- 0.1, effectively

const double          GMETAOPT_DEF_INITADPROBRANGE = 0.9;      ///< The initial upper boundary for the variation of adProb
const double          GMETAOPT_DEF_ADPROBRANGE_LB = 0.1;       ///< The lower boundary for adProbRange
const double          GMETAOPT_DEF_ADPROBRANGE_UB = 0.9;       ///< The upper boundary for adProbRange

const double          GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE = 1.; ///< Defines the place inside of the allowed value range where adProb starts. Boundaries are 0./1.

const double          GMETAOPT_DEF_INITADAPTADPROB = 0.1;   ///< The initial value of the strength of adProb_ adaption   | !!! NEW
const double          GMETAOPT_DEF_ADAPTADPROB_LB = 0.;     ///< The lower boundary for the variation of the strength of adProb_ adaption | !!! NEW
const double          GMETAOPT_DEF_ADAPTADPROB_UB = 1.;     ///< The upper boundary for the variation of the strength of adProb_ adaption | !!! NEW

const double          GMETAOPT_DEF_INITMINSIGMA = 0.001;    ///< The initial lower boundary for sigma
const double          GMETAOPT_DEF_MINSIGMA_LB = 0.001;     ///< The lower boundary for the variation of the lower boundary of sigma
const double          GMETAOPT_DEF_MINSIGMA_UB = 0.09999;   ///< The upper boundary for the variation of the lower boundary of sigma, means ~0.1

const double          GMETAOPT_DEF_INITSIGMARANGE = 0.2;    ///< The initial maximum range for sigma --> note that the initial start value for sigma will always be set to the upper boundary of its variation limits
const double          GMETAOPT_DEF_SIGMARANGE_LB = 0.1;     ///< The lower boundary for the variation of the maximum range of sigma --> maxSigma is 0.2
const double          GMETAOPT_DEF_SIGMARANGE_UB = 0.9;     ///< The upper boundary for the variation of the maximum range of sigma --> maxSigma is 1.

const double          GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE = 1.; ///< The initial percentage of the sigma range as a start value

const double          GMETAOPT_DEF_INITSIGMASIGMA = 0.1;    ///< The initial strength of sigma adaption
const double          GMETAOPT_DEF_SIGMASIGMA_LB = 0.;      ///< The lower boundary for the variation of the strength of sigma adaption
const double          GMETAOPT_DEF_SIGMASIGMA_UB = 1.;      ///< The upper boundary for the variation of the strength of sigma adaption

const double          GMETAOPT_DEF_INITCROSSOVERPROB = 0.;    ///< The likelihood for two data items to be exchanged in a cross-over operation | !!! NEW
const double          GMETAOPT_DEF_CROSSOVERPROB_LB  = 0.;   ///< The lower boundary for the variation of the cross-over probability  | !!! NEW
const double          GMETAOPT_DEF_CROSSOVERPROB_UB  = 1.;     ///< The upper boundary for the variation of the cross-over probability  | !!! NEW

// General meta-optimization parameters
const std::size_t     GMETAOPT_DEF_NRUNSPEROPT=10;             ///< The number of successive optimization runs
const double          GMETAOPT_DEF_FITNESSTARGET = 0.001;      ///< The fitness target
const boost::uint32_t GMETAOPT_DEF_ITERATIONTHRESHOLD = 10000; ///< The maximum allowed number of iterations

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GMetaOptimizerIndividual : public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		& BOOST_SERIALIZATION_NVP(nRunsPerOptimization_)
		& BOOST_SERIALIZATION_NVP(fitnessTarget_)
		& BOOST_SERIALIZATION_NVP(iterationThreshold_);
	}

	///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GMetaOptimizerIndividual();
	/** @brief A standard copy constructor */
	GMetaOptimizerIndividual(const GMetaOptimizerIndividual&);
	/** @brief The standard destructor */
	virtual ~GMetaOptimizerIndividual();

	/** @brief A standard assignment operator */
	const GMetaOptimizerIndividual& operator=(const GMetaOptimizerIndividual&);

   /** @brief Checks for equality with another GFunctionIndividual object */
   bool operator==(const GMetaOptimizerIndividual&) const;
   /** @brief Checks for inequality with another GFunctionIndividual object */
   bool operator!=(const GMetaOptimizerIndividual& cp) const;

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

	/** @brief Allows to specify how many optimizations should be performed for each (sub-)optimization */
	void setNRunsPerOptimization(std::size_t);
	/** @brief Allows to retrieve the number of optimizations to be performed for each (sub-)optimization */
	std::size_t getNRunsPerOptimization() const;

	/** @brief Allows to set the fitness target for each optimization */
	void setFitnessTarget(double);
	/** @brief Retrieves the fitnes target for each optimization */
	double getFitnessTarget() const;

	/** @brief Allows to set the iteration threshold */
	void setIterationThreshold(boost::uint32_t);
	/** @brief Allows to retrieve the iteration threshold */
	boost::uint32_t getIterationThreshold() const;

	/** @brief Retrieves the current number of parents */
	std::size_t getNParents() const;
   /** @brief Retrieves the current number of children */
   std::size_t getNChildren() const;
   /** @brief Retrieves the adaption probability */
   double getAdProb() const;
   /** @brief Retrieves the lower sigma boundary */
   double getMinSigma() const;
   /** @brief Retrieves the sigma range */
   double getSigmaRange() const;
   /** @brief Retrieves the sigma-sigma parameter */
   double getSigmaSigma() const;

	/** @brief Emit information about this individual */
	std::string print() const;

   /***************************************************************************/
   /**
    * This function is used to unify the setup from within the constructor
    * and factory.
    */
   static void addContent(
      boost::shared_ptr<GMetaOptimizerIndividual> p
      , const std::size_t& initNParents
      , const std::size_t& nParents_LB
      , const std::size_t& nParents_UB
      , const std::size_t& initNChildren
      , const std::size_t& nChildren_LB
      , const std::size_t& nChildren_UB
      , const double& initAmalgamationLklh
      , const double& amalgamationLklh_LB
      , const double& amalgamationLklh_UB
      , const double& initMinAdProb
      , const double& minAdProb_LB
      , const double& minAdProb_UB
      , const double& initAdProbRange
      , const double& adProbRange_LB
      , const double& adProbRange_UB
      , const double& initAdProbStartPercentage
      , const double& initAdaptAdProb
      , const double& adaptAdProb_LB
      , const double& adaptAdProb_UB
      , const double& initMinSigma
      , const double& minSigma_LB
      , const double& minSigma_UB
      , const double& initSigmaRange
      , const double& sigmaRange_LB
      , const double& sigmaRange_UB
      , const double& initSigmaRangePercentage
      , const double& initSigmaSigma
      , const double& sigmaSigma_LB
      , const double& sigmaSigma_UB
      , const double& initCrossOverProb
      , const double& crossOverProb_LB
      , const double& crossOverProb_UB
   ) {
      // We will add parameter types in the same order as the arguments

      //------------------------------------------------------------
      // nParents

      // Small number of possible values -- use a flip-adaptor
      boost::shared_ptr<GInt32FlipAdaptor> gifa_ptr(new GInt32FlipAdaptor());
      gifa_ptr->setAdaptionProbability(1.);

      boost::shared_ptr<GConstrainedInt32Object> npar_ptr(new GConstrainedInt32Object(boost::numeric_cast<boost::int32_t>(initNParents), nParents_LB, nParents_UB));
      npar_ptr->addAdaptor(gifa_ptr);
      npar_ptr->setParameterName("nParents");

      // Add to the individual
      p->push_back(npar_ptr);

      //------------------------------------------------------------
      // nChildren

      // Create a default standard gauss adaptor
      boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(
            0.025   // sigma
            , 0.2   // sigmaSigma
            , 0.001   // minSigma
            , 0.5    // maxSigma
            , 1.   // adProb
         )
      );

      boost::shared_ptr<GConstrainedInt32Object> nch_ptr(new GConstrainedInt32Object(boost::numeric_cast<boost::int32_t>(initNChildren), nChildren_LB, nChildren_UB));
      nch_ptr->addAdaptor(giga_ptr);
      nch_ptr->setParameterName("nChildren");

      // Add to the individual
      p->push_back(nch_ptr);

      //------------------------------------------------------------
      // amalgamationLklh

      // Create a default standard gauss adaptor
      boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(
            0.025    // sigma
            , 0.2    // sigmaSigma
            , 0.001  // minSigma
            , 0.5    // maxSigma
            , 1.     // adProb
         )
      );
      boost::shared_ptr<GConstrainedDoubleObject> amalgamationLklh_ptr(new GConstrainedDoubleObject(
            initAmalgamationLklh // initial value
            , amalgamationLklh_LB  // lower boundary
            , amalgamationLklh_UB // upper boundary
          )
      );
      // Add the gauss adaptor to the parameter
      amalgamationLklh_ptr->addAdaptor(gdga_ptr);
      amalgamationLklh_ptr->setParameterName("amalgamationLikelihood");

      // Add to the individual
      p->push_back(amalgamationLklh_ptr);

      //------------------------------------------------------------
      // minAdProb

      boost::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr(new GConstrainedDoubleObject(
            initMinAdProb  // initial value
            , minAdProb_LB // lower boundary
            , minAdProb_UB // upper boundary
          )
      );
      // Add the gauss adaptor to the parameter
      minAdProb_ptr->addAdaptor(gdga_ptr);
      minAdProb_ptr->setParameterName("minAdProb");

      // Add to the individual
      p->push_back(minAdProb_ptr);

      //------------------------------------------------------------
      // adProbRange

      boost::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr(new GConstrainedDoubleObject(
            initAdProbRange  // initial value
            , adProbRange_LB // lower boundary
            , adProbRange_UB // upper boundary
          )
      );
      // Add the gauss adaptor to the parameter
      adProbRange_ptr->addAdaptor(gdga_ptr);
      adProbRange_ptr->setParameterName("adProbRange");

      // Add to the individual
      p->push_back(adProbRange_ptr);

      //------------------------------------------------------------
      // adProbStartPercentage

      boost::shared_ptr<GConstrainedDoubleObject> adProbStartPercentage_ptr(new GConstrainedDoubleObject(
            initAdProbStartPercentage  // initial value
            , 0. // lower boundary
            , 1. // upper boundary
          )
      );
      // Add the gauss adaptor to the parameter
      adProbStartPercentage_ptr->addAdaptor(gdga_ptr);
      adProbStartPercentage_ptr->setParameterName("adProbStartPercentage");

      // Add to the individual
      p->push_back(adProbStartPercentage_ptr);

      //------------------------------------------------------------
      // adaptAdProb

      boost::shared_ptr<GConstrainedDoubleObject> adaptAdProb_ptr(new GConstrainedDoubleObject(
            initAdaptAdProb // initial value
            , adaptAdProb_LB // lower boundary
            , adaptAdProb_UB // upper boundary
      ));
      adaptAdProb_ptr->addAdaptor(gdga_ptr);
      adaptAdProb_ptr->setParameterName("adaptAdProb");

      // Add to the individual
      p->push_back(adaptAdProb_ptr);

      //------------------------------------------------------------
      // minSigma

      boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr(new GConstrainedDoubleObject(
            initMinSigma // initial value
            , minSigma_LB // lower boundary
            , minSigma_UB // upper boundary
      ));
      minsigma_ptr->addAdaptor(gdga_ptr);
      minsigma_ptr->setParameterName("minSigma");

      // Add to the individual
      p->push_back(minsigma_ptr);

      //------------------------------------------------------------
      // sigmaRange

      boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr(new GConstrainedDoubleObject(
            initSigmaRange // initial value
            , sigmaRange_LB // lower boundary
            , sigmaRange_UB // upper boundary
       ));
      sigmarange_ptr->addAdaptor(gdga_ptr);
      sigmarange_ptr->setParameterName("sigmaRange");

      // Add to the individual
      p->push_back(sigmarange_ptr);

      //------------------------------------------------------------
      // sigmaRangePercentage

      boost::shared_ptr<GConstrainedDoubleObject> sigmaRangePercentage_ptr(new GConstrainedDoubleObject(
            initSigmaRangePercentage  // initial value
            , 0. // lower boundary
            , 1. // upper boundary
          )
      );
      // Add the gauss adaptor to the parameter
      sigmaRangePercentage_ptr->addAdaptor(gdga_ptr);
      sigmaRangePercentage_ptr->setParameterName("sigmaRangePercentage");

      // Add to the individual
      p->push_back(sigmaRangePercentage_ptr);

      //------------------------------------------------------------
      // sigmaSigma

      // The sigma adaption strength may change between 0.01 and 1
      boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr(new GConstrainedDoubleObject(
            initSigmaSigma  // initial value
            , sigmaSigma_LB  // lower boundary
            , sigmaSigma_UB  // upper boundary
       ));
      sigmasigma_ptr->addAdaptor(gdga_ptr);
      sigmasigma_ptr->setParameterName("sigmaSigma");

      // Add to the individual
      p->push_back(sigmasigma_ptr);

      //------------------------------------------------------------
      // crossOverProb

      boost::shared_ptr<GConstrainedDoubleObject> crossOverProb_ptr(new GConstrainedDoubleObject(
            initCrossOverProb  // initial value
            , crossOverProb_LB  // lower boundary
            , crossOverProb_UB  // upper boundary
       ));
      crossOverProb_ptr->addAdaptor(gdga_ptr);
      crossOverProb_ptr->setParameterName("crossOverProb");

      // Add to the individual
      p->push_back(crossOverProb_ptr);

      //------------------------------------------------------------
   }

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GMetaOptimizerIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual value calculation takes place here */
	virtual double fitnessCalculation() OVERRIDE;

	/***************************************************************************/

private:
	std::size_t nRunsPerOptimization_; ///< The number of runs performed for each (sub-)optimization
	double fitnessTarget_; ///< The quality target to be reached by
	boost::uint32_t iterationThreshold_; ///< The maximum allowed number of iterations

public:
   /** @brief Applies modifications to this object. */
   virtual bool modify_GUnitTests();
   /** @brief Performs self tests that are expected to succeed. */
   virtual void specificTestsNoFailureExpected_GUnitTests();
   /** @brief Performs self tests that are expected to fail. */
   virtual void specificTestsFailuresExpected_GUnitTests();
};

/** @brief Allows to output a GMetaOptimizerIndividual or convert it to a string using boost::lexical_cast */
std::ostream& operator<<(std::ostream&, const GMetaOptimizerIndividual&);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GMetaOptimizerIndividual objects
 */
class GMetaOptimizerIndividualFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
public:
	/** @brief The standard constructor */
	GMetaOptimizerIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GMetaOptimizerIndividualFactory();

protected:
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to describe local configuration options in derived classes */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<GParameterSet>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GMetaOptimizerIndividualFactory();

	// Parameters pertaining to the ea population
	std::size_t initNParents_;  ///< The initial number of parents
	std::size_t nParents_LB_;    ///< The lower boundary for variations of the number of parents
	std::size_t nParents_UB_;    ///< The upper boundary for variations of the number of parents

	std::size_t initNChildren_; ///< The initial number of children
	std::size_t nChildren_LB_;   ///< The lower boundary for the variation of the number of children
	std::size_t nChildren_UB_;   ///< The upper boundary for the variation of the number of children

	double initAmalgamationLklh_;  ///< The initial likelihood for an individual being created from cross-over rather than "just" duplication | !!! NEW
	double amalgamationLklh_LB_;    ///< The upper boundary for the variation of the amalgamation likelihood | !!! NEW
	double amalgamationLklh_UB_;    ///< The upper boundary for the variation of the amalgamation likelihood | !!! NEW

	double initMinAdProb_;      ///< The initial lower boundary for the variation of adProb
	double minAdProb_LB_;       ///< The lower boundary for minAdProb
	double minAdProb_UB_;       ///< The upper boundary for minAdProb

	double initAdProbRange_;    ///< The initial range for the variation of adProb
	double adProbRange_LB_;     ///< The lower boundary for adProbRange
	double adProbRange_UB_;     ///< The upper boundary for adProbRange

	double initAdProbStartPercentage_; ///< The start value for adProb relative to the allowed value range

   double initAdaptAdProb_;    ///< The initial value of the strength of adProb_ adaption   | !!! NEW
   double adaptAdProb_LB_;      ///< The lower boundary for the variation of the strength of adProb_ adaption | !!! NEW
   double adaptAdProb_UB_;      ///< The upper boundary for the variation of the strength of adProb_ adaption | !!! NEW

	double initMinSigma_;       ///< The initial minimal value of sigma
	double minSigma_LB_;         ///< The lower boundary for the variation of the lower boundary of sigma
	double minSigma_UB_;         ///< The upper boundary for the variation of the lower boundary of sigma

	double initSigmaRange_;     ///< The initial range of sigma (beyond minSigma
	double sigmaRange_LB_;       ///< The lower boundary for the variation of the maximum range of sigma
	double sigmaRange_UB_;       ///< The upper boundary for the variation of the maximum range of sigma

	double initSigmaRangePercentage_; ///< The initial percentage of the sigma range as a start value

	double initSigmaSigma_;     ///< The initial strength of sigma adaption
	double sigmaSigma_LB_;       ///< The lower boundary for the variation of the strength of sigma adaption
	double sigmaSigma_UB_;       ///< The upper boundary for the variation of the strength of sigma adaption

	double initCrossOverProb_;     ///< The likelihood for two data items to be exchanged in a cross-over operation | !!! NEW
	double crossOverProb_LB_;       ///< The lower boundary for the variation of the cross-over probability | !!! NEW
	double crossOverProb_UB_;       ///< The upper boundary for the variation of the cross-over probability | !!! NEW
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMetaOptimizerIndividual)

#endif /* GMETAOPTIMIZERINDIVIDUAL_HPP_ */
