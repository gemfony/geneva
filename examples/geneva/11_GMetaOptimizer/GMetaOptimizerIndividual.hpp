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
const std::size_t     GMETAOPT_DEF_INITSUBNPARENTS = 1;        ///< The initial number of parents
const std::size_t     GMETAOPT_DEF_SUBNPARENTSLB = 1;          ///< The lower boundary for variations of the number of parents
const std::size_t     GMETAOPT_DEF_SUBNPARENTSUB = 6;          ///< The upper boundary for variations of the number of parents
const std::size_t     GMETAOPT_DEF_INITSUBNCHILDREN = 100;     ///< The initial number of children
const std::size_t     GMETAOPT_DEF_SUBNCHILDRENLB = 5;         ///< The lower boundary for the variation of the number of children
const std::size_t     GMETAOPT_DEF_SUBNCHILDRENUB = 200;       ///< The upper boundary for the variation of the number of children
const double          GMETAOPT_DEF_INITSUBADPROB = 0.99;       ///< The initial adaption probability
const double          GMETAOPT_DEF_SUBADPROBLB = 0.0001;       ///< The lower boundary for the variation of the adaption probability
const double          GMETAOPT_DEF_SUBADPROBUB = 0.9999;       ///< The upper boundary for the variation of the adaption probability
const double          GMETAOPT_DEF_INITSUBMINSIGMA = 0.00001;  ///< The initial lower boundary for sigma
const double          GMETAOPT_DEF_SUBMINSIGMALB = 0.00001;    ///< The lower boundary for the variation of the lower boundary of sigma
const double          GMETAOPT_DEF_SUBMINSIGMAUB = 0.4999;     ///< The upper boundary for the variation of the lower boundary of sigma
const double          GMETAOPT_DEF_INITSUBSIGMARANGE = 1.;     ///< The initial maximum range for sigma
const double          GMETAOPT_DEF_SUBSIGMARANGELB = 0.5;      ///< The lower boundary for the variation of the maximum range of sigma
const double          GMETAOPT_DEF_SUBSIGMARANGEUB = 1.;       ///< The upper boundary for the variation of the maximum range of sigma
const double          GMETAOPT_DEF_INITSUBSIGMASIGMA = 0.999;  ///< The initial strength of sigma adaption
const double          GMETAOPT_DEF_SUBSIGMASIGMALB = 0.01;     ///< The lower boundary for the variation of the strength of sigma adaption
const double          GMETAOPT_DEF_SUBSIGMASIGMAUB = 1.;       ///< The upper boundary for the variation of the strength of sigma adaption
const std::size_t     GMETAOPT_DEF_NRUNSPEROPT=50;             ///< The number of successive optimization runs
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
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
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
         const GObject&,
         const Gem::Common::expectation&,
         const double&,
         const std::string&,
         const std::string&,
         const bool&
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
         , const std::size_t& initSubNParents
         , const std::size_t& subNParentsLB
         , const std::size_t& subNParentsUB
         , const std::size_t& initSubNChildren
         , const std::size_t& subNChildrenLB
         , const std::size_t& subNChildrenUB
         , const double& initSubAdProb
         , const double& subAdProbLB
         , const double& subAdProbUB
         , const double& initSubMinSigma
         , const double& subMinSigmaLB
         , const double& subMinSigmaUB
         , const double& initSubSigmaRange
         , const double& subSigmaRangeLB
         , const double& subSigmaRangeUB
         , const double& initSubSigmaSigma
         , const double& subSigmaSigmaLB
         , const double& subSigmaSigmaUB
   ) {
      // We will add parameter types in the same order as the arguments

      // Small number of possible values -- use a flip-adaptor
      boost::shared_ptr<GInt32FlipAdaptor> gifa_ptr(new GInt32FlipAdaptor());
      gifa_ptr->setAdaptionProbability(0.5);

      // We allow between 1 and 5 parents
      boost::shared_ptr<GConstrainedInt32Object> npar_ptr(new GConstrainedInt32Object(boost::numeric_cast<boost::int32_t>(initSubNParents), subNParentsLB, subNParentsUB));
      npar_ptr->addAdaptor(gifa_ptr);
      // Add to the individual
      p->push_back(npar_ptr);

      // Create a default standard gauss adaptor
      boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(
            4.99    // sigma
            , 0.5   // sigmaSigma
            , 0.5   // minSigma
            , 5.    // maxSigma
            , 0.5   // adProb
         )
      );

      // We allow between 5 and 1000 children
      boost::shared_ptr<GConstrainedInt32Object> nch_ptr(new GConstrainedInt32Object(boost::numeric_cast<boost::int32_t>(initSubNChildren), subNChildrenLB, subNChildrenUB));
      nch_ptr->addAdaptor(giga_ptr);
      // Add to the individual
      p->push_back(nch_ptr);

      // Create a default standard gauss adaptor
      boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(
            0.5    // sigma
            , 0.5  // sigmaSigma
            , 0.01 // minSigma
            , 2.   // maxSigma
            , 0.5   // adProb
         )
      );

      // We allow adProb values between 0.001 and 1.
      boost::shared_ptr<GConstrainedDoubleObject> adprob_ptr(new GConstrainedDoubleObject(
            initSubAdProb // initial value
            , subAdProbLB  // lower boundary
            , subAdProbUB // upper boundary
          )
      );
      // Add the gauss adaptor to the parameter
      adprob_ptr->addAdaptor(gdga_ptr);
      // Add to the individual
      p->push_back(adprob_ptr);

      // MinSigma may change between 0.0001 and 5
      boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr(new GConstrainedDoubleObject(
            initSubMinSigma // initial value
            , subMinSigmaLB // lower boundary
            , subMinSigmaUB // upper boundary
      ));
      minsigma_ptr->addAdaptor(gdga_ptr);
      // Add to the individual
      p->push_back(minsigma_ptr);

      // The sigma-range (aka maxSigma) may change between 0.1 and 5
      boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr(new GConstrainedDoubleObject(
            initSubSigmaRange // initial value
            , subSigmaRangeLB // lower boundary
            , subSigmaRangeUB // upper boundary
       ));
      sigmarange_ptr->addAdaptor(gdga_ptr);
      // Add to the individual
      p->push_back(sigmarange_ptr);

      // The sigma adaption strength may change between 0.01 and 1
      boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr(new GConstrainedDoubleObject(
            initSubSigmaSigma  // initial value
            , subSigmaSigmaLB  // lower boundary
            , subSigmaSigmaUB  // upper boundary
       ));
      sigmasigma_ptr->addAdaptor(gdga_ptr);
      // Add to the individual
      p->push_back(sigmasigma_ptr);
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

	std::size_t initSubNParents_;  ///< The initial number of parents
	std::size_t subNParentsLB_;    ///< The lower boundary for variations of the number of parents
	std::size_t subNParentsUB_;    ///< The upper boundary for variations of the number of parents

	std::size_t initSubNChildren_; ///< The initial number of children
	std::size_t subNChildrenLB_;   ///< The lower boundary for the variation of the number of children
	std::size_t subNChildrenUB_;   ///< The upper boundary for the variation of the number of children

	double initSubAdProb_;         ///< The initial adaption probability
	double subAdProbLB_;           ///< The lower boundary for the variation of the adaption probability
	double subAdProbUB_;           ///< The upper boundary for the variation of the adaption probability

	double initSubMinSigma_;       ///< The initial minimal value of sigma
	double subMinSigmaLB_;         ///< The lower boundary for the variation of the lower boundary of sigma
	double subMinSigmaUB_;         ///< The upper boundary for the variation of the lower boundary of sigma

	double initSubSigmaRange_;     ///< The initial range of sigma (beyond minSigma
	double subSigmaRangeLB_;       ///< The lower boundary for the variation of the maximum range of sigma
	double subSigmaRangeUB_;       ///< The upper boundary for the variation of the maximum range of sigma

	double initSubSigmaSigma_;     ///< The initial strength of sigma adaption
	double subSigmaSigmaLB_;       ///< The lower boundary for the variation of the strength of sigma adaption
	double subSigmaSigmaUB_;       ///< The upper boundary for the variation of the strength of sigma adaption
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMetaOptimizerIndividual)

#endif /* GMETAOPTIMIZERINDIVIDUAL_HPP_ */
