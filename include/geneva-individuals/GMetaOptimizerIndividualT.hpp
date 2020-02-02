/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <tuple>

// Boost header files go here

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GPluggableOptimizationMonitors.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// Different types of optimization targets
enum class metaOptimizationTarget :
    Gem::Common::ENUMBASETYPE
{
    BESTFITNESS = 0, MINSOLVERCALLS = 1, MC_MINSOLVER_BESTFITNESS
    = 2 // Multi-criterion optimization with least number of solver calls and best average fitness as targets
};

/******************************************************************************/
// Input and output of metaOptimizationTarget, so we can serialize this data

/** @brief Puts a Gem::Geneva::metaOptimizationTarget into a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, const Gem::Geneva::metaOptimizationTarget &);

/** @brief Reads a Gem::Geneva::metaOptimizationTarget from a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::istream &operator>>(std::istream &, Gem::Geneva::metaOptimizationTarget &);

/******************************************************************************/
// A number of default settings for the factory and individual

// Pertaining to the population
const std::size_t GMETAOPT_DEF_INITNPARENTS = 1;        ///< The initial number of parents
const std::size_t GMETAOPT_DEF_NPARENTS_LB = 1;         ///< The lower boundary for variations of the number of parents
const std::size_t GMETAOPT_DEF_NPARENTS_UB = 6;         ///< The upper boundary for variations of the number of parents

const std::size_t GMETAOPT_DEF_INITNCHILDREN = 100;     ///< The initial number of children
const std::size_t
    GMETAOPT_DEF_NCHILDREN_LB = 5;        ///< The lower boundary for the variation of the number of children
const std::size_t
    GMETAOPT_DEF_NCHILDREN_UB = 250;      ///< The upper boundary for the variation of the number of children

const double GMETAOPT_DEF_INITAMALGLKLHOOD
    = 0.;      ///< The initial likelihood for an individual being created from cross-over rather than "just" duplication
const double
    GMETAOPT_DEF_AMALGLKLHOOD_LB = 0.;       ///< The lower boundary for the variation of the amalgamation likelihood
const double
    GMETAOPT_DEF_AMALGLKLHOOD_UB = 1.;       ///< The upper boundary for the variation of the amalgamation likelihood

// Concerning the individual
const double GMETAOPT_DEF_INITMINADPROB = 0.;         ///< The initial lower boundary for the variation of adProb
const double GMETAOPT_DEF_MINADPROB_LB = 0.;          ///< The lower boundary for minAdProb
const double GMETAOPT_DEF_MINADPROB_UB = 0.1;         ///< The upper boundary for minAdProb -- 0.1, effectively

const double GMETAOPT_DEF_INITADPROBRANGE = 0.9;      ///< The initial upper boundary for the variation of adProb
const double GMETAOPT_DEF_ADPROBRANGE_LB = 0.1;       ///< The lower boundary for adProbRange
const double GMETAOPT_DEF_ADPROBRANGE_UB = 0.9;       ///< The upper boundary for adProbRange

const double GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE
    = 1.; ///< Defines the place inside of the allowed value range where adProb starts. Boundaries are 0./1.

const double GMETAOPT_DEF_INITADAPTADPROB = 0.1;   ///< The initial value of the strength of adProb_ adaption
const double
    GMETAOPT_DEF_ADAPTADPROB_LB = 0.;     ///< The lower boundary for the variation of the strength of adProb_ adaption
const double
    GMETAOPT_DEF_ADAPTADPROB_UB = 1.;     ///< The upper boundary for the variation of the strength of adProb_ adaption

const double GMETAOPT_DEF_INITMINSIGMA = 0.001;    ///< The initial lower boundary for sigma
const double
    GMETAOPT_DEF_MINSIGMA_LB = 0.001;     ///< The lower boundary for the variation of the lower boundary of sigma
const double GMETAOPT_DEF_MINSIGMA_UB
    = 0.09999;   ///< The upper boundary for the variation of the lower boundary of sigma, means ~0.1

const double GMETAOPT_DEF_INITSIGMARANGE
    = 0.2;    ///< The initial maximum range for sigma --> note that the initial start value for sigma will always be set to the upper boundary of its variation limits
const double GMETAOPT_DEF_SIGMARANGE_LB
    = 0.1;     ///< The lower boundary for the variation of the maximum range of sigma --> maxSigma is 0.2
const double GMETAOPT_DEF_SIGMARANGE_UB
    = 0.9;     ///< The upper boundary for the variation of the maximum range of sigma --> maxSigma is 1.

const double GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE = 1.; ///< The initial percentage of the sigma range as a start value

const double GMETAOPT_DEF_INITSIGMASIGMA = 0.1;    ///< The initial strength of sigma adaption
const double
    GMETAOPT_DEF_SIGMASIGMA_LB = 0.;      ///< The lower boundary for the variation of the strength of sigma adaption
const double
    GMETAOPT_DEF_SIGMASIGMA_UB = 1.;      ///< The upper boundary for the variation of the strength of sigma adaption

// General meta-optimization parameters
const std::size_t GMETAOPT_DEF_NRUNSPEROPT = 10;              ///< The number of successive optimization runs
const double GMETAOPT_DEF_FITNESSTARGET = 0.001;       ///< The fitness target
const std::uint32_t GMETAOPT_DEF_ITERATIONTHRESHOLD = 10000;  ///< The maximum allowed number of iterations
const metaOptimizationTarget
    GMETAOPT_DEF_MOTARGET = metaOptimizationTarget::BESTFITNESS;      ///< The target used for the meta optimization

const std::string GMETAOPT_DEF_INDCONFIG
    = "./config/GFunctionIndividual.json"; ///< The default configuration file for our individuals -- we follow the default template argument
const std::string GMETAOPT_DEF_SUBEACONFIG
    = "./config/GSubEvolutionaryAlgorithm.json"; ///< The default configuration file for the (sub-)evolutionary algorithms

const bool GMETAOPT_SUBEXEC_SERIAL = false;
const bool GMETAOPT_SUBEXEC_MULTITHREADED = true;
const bool GMETAOPT_DEF_SUBEXECMODE = GMETAOPT_SUBEXEC_MULTITHREADED;

// Make sure we do not mix parameter items
const std::size_t MOT_NPARENTS = 0;
const std::size_t MOT_NCHILDREN = 1;
const std::size_t MOT_AMALGAMATION = 2;
const std::size_t MOT_MINADPROB = 3;
const std::size_t MOT_ADPROBRANGE = 4;
const std::size_t MOT_ADPROBSTARTPERCENTAGE = 5;
const std::size_t MOT_ADAPTADPROB = 6;
const std::size_t MOT_MINSIGMA = 7;
const std::size_t MOT_SIGMARANGE = 8;
const std::size_t MOT_SIGMARANGEPERCENTAGE = 9;
const std::size_t MOT_SIGMASIGMA = 10;
const std::size_t MOT_NVAR = 11;

/******************************************************************************/
/**
 * This individual performs "meta optimization" in the sense that parameters of
 * an optimization algorithm are optimized alongside a given "sub-"individual.
 * The individual is meant for tuning the parameters of Evolutionary Algorithms,
 * but may be used to find better optima as well.
 */
template<typename ind_type = Gem::Geneva::GFunctionIndividual>
class GMetaOptimizerIndividualT
    : public GParameterSet
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
        & BOOST_SERIALIZATION_NVP(nRunsPerOptimization_)
        & BOOST_SERIALIZATION_NVP(fitnessTarget_)
        & BOOST_SERIALIZATION_NVP(iterationThreshold_)
        & BOOST_SERIALIZATION_NVP(moTarget_)
        & BOOST_SERIALIZATION_NVP(subEA_config_)
        & BOOST_SERIALIZATION_NVP(ind_factory_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The default constructor.
     */
    GMetaOptimizerIndividualT()
        :
        GParameterSet()
        , nRunsPerOptimization_(GMETAOPT_DEF_NRUNSPEROPT)
        , fitnessTarget_(GMETAOPT_DEF_FITNESSTARGET)
        , iterationThreshold_(GMETAOPT_DEF_ITERATIONTHRESHOLD)
        , moTarget_(GMETAOPT_DEF_MOTARGET)
        , subEA_config_(GMETAOPT_DEF_SUBEACONFIG)
        , ind_factory_() { /* nothing */ }

    /***************************************************************************/
    /**
     * A standard copy constructor
     *
     * @param cp A copy of another GFunctionIndidivual
     */
    GMetaOptimizerIndividualT(const GMetaOptimizerIndividualT<ind_type> &cp)
        :
        GParameterSet(cp)
        , nRunsPerOptimization_(cp.nRunsPerOptimization_)
        , fitnessTarget_(cp.fitnessTarget_)
        , iterationThreshold_(cp.iterationThreshold_)
        , moTarget_(cp.moTarget_)
        , subEA_config_(cp.subEA_config_)
        , ind_factory_(
        Gem::Common::convertSmartPointer<Gem::Common::GFactoryT<GParameterSet>, typename ind_type::FACTORYTYPE>(
            (cp.ind_factory_)->clone()
        )
    ) { /* nothing */   }

    /***************************************************************************/
    /**
     * The standard destructor
     */
    virtual ~GMetaOptimizerIndividualT() { /* nothing */   }

    /***************************************************************************/
    /**
     * Allows to specify the path and name of a configuration file passed to
     * the (sub-)evolutionary algorithm
     */
    void setSubEAConfig(std::string subEA_config) {
        subEA_config_ = subEA_config;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the path and name of a configuration file passed to
     * the (sub-)evolutionary algorithm
     */
    std::string getSubEAConfig() const {
        return subEA_config_;
    }

    /***************************************************************************/
    /**
     * Allows to specify how many optimizations should be performed for each (sub-)optimization
     */
    void setNRunsPerOptimization(std::size_t nRunsPerOptimization) {
#ifdef DEBUG
        if (0 == nRunsPerOptimization) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GMetaOptimizerIndividualT<ind_type>::setNRunsPerOptimization(): Error!" << std::endl
                    << "Requested number of sub-optimizations is 0" << std::endl
            );
        }
#endif

        nRunsPerOptimization_ = nRunsPerOptimization;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the number of optimizations to be performed for each (sub-)optimization
     */
    std::size_t getNRunsPerOptimization() const {
        return nRunsPerOptimization_;
    }

    /***************************************************************************/
    /**
     * Allows to set the fitness target for each optimization
     */
    void setFitnessTarget(double fitnessTarget) {
        fitnessTarget_ = fitnessTarget;
    }

    /***************************************************************************/
    /**
     * Retrieves the fitness target for each optimization
     */
    double getFitnessTarget() const {
        return fitnessTarget_;
    }

    /***************************************************************************/
    /**
     * Allows to set the iteration threshold
     */
    void setIterationThreshold(std::uint32_t iterationThreshold) {
        iterationThreshold_ = iterationThreshold;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the iteration threshold
     */
    std::uint32_t getIterationThreshold() const {
        return iterationThreshold_;
    }

    /***************************************************************************/
    /**
     * Allows to set the desired target of the meta-optimization
     */
    void setMetaOptimizationTarget(metaOptimizationTarget moTarget) {
        moTarget_ = moTarget;

        // multi-criterion optimization. We need to set the number of fitness criteria
        if (metaOptimizationTarget::MC_MINSOLVER_BESTFITNESS == moTarget_) {
            this->setNStoredResults(2);
        }
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the current target of the meta-optimization
     */
    metaOptimizationTarget getMetaOptimizationTarget() const {
        return moTarget_;
    }

    /***************************************************************************/
    /**
     * Retrieves the current number of parents. Needed for the optimization monitor.
     */
    std::size_t getNParents() const {
        std::shared_ptr<GConstrainedInt32Object> npar_ptr = this->at<GConstrainedInt32Object>(MOT_NPARENTS);
        return boost::numeric_cast<std::size_t>(npar_ptr->value());
    }

    /***************************************************************************/
    /**
     * Retrieves the current number of children. Needed for the optimization monitor.
     */
    std::size_t getNChildren() const {
        std::shared_ptr<GConstrainedInt32Object> nch_ptr = this->at<GConstrainedInt32Object>(MOT_NCHILDREN);
        return boost::numeric_cast<std::size_t>(nch_ptr->value());
    }

    /***************************************************************************/
    /**
     * Retrieves the adaption probability. Needed for the optimization monitor.
     */
    double getAdProb() const {
        std::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr = this->at<GConstrainedDoubleObject>(MOT_MINADPROB);
        std::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBRANGE);
        std::shared_ptr<GConstrainedDoubleObject>
            adProbStartPercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);

        return minAdProb_ptr->value() + adProbStartPercentage_ptr->value() * adProbRange_ptr->value();
    }

    /***************************************************************************/
    /**
     * Retrieves the lower sigma boundary. Needed for the optimization monitor.
     */
    double getMinSigma() const {
        std::shared_ptr<GConstrainedDoubleObject> minsigma_ptr
            = this->at<GConstrainedDoubleObject>(MOT_MINSIGMA);
        return minsigma_ptr->value();
    }

    /***************************************************************************/
    /**
     * Retrieves the sigma range. Needed for the optimization monitor.
     */
    double getSigmaRange() const {
        std::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr
            = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGE);
        return sigmarange_ptr->value();
    }

    /***************************************************************************/
    /**
     * Retrieves the sigma-sigma parameter. Needed for the optimization monitor.
     */
    double getSigmaSigma() const {
        std::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr
            = this->at<GConstrainedDoubleObject>(MOT_SIGMASIGMA);
        return sigmasigma_ptr->value();
    }

    /***************************************************************************/
    /**
     * This function is used to unify the setup from within the constructor
     * and factory.
     */
    static void addContent(
        std::shared_ptr<GMetaOptimizerIndividualT<ind_type>> p, const std::size_t &initNParents
        , const std::size_t &nParents_LB, const std::size_t &nParents_UB, const std::size_t &initNChildren
        , const std::size_t &nChildren_LB, const std::size_t &nChildren_UB, const double &initAmalgamationLklh
        , const double &amalgamationLklh_LB, const double &amalgamationLklh_UB, const double &initMinAdProb
        , const double &minAdProb_LB, const double &minAdProb_UB, const double &initAdProbRange
        , const double &adProbRange_LB, const double &adProbRange_UB, const double &initAdProbStartPercentage
        , const double &initAdaptAdProb, const double &adaptAdProb_LB, const double &adaptAdProb_UB
        , const double &initMinSigma, const double &minSigma_LB, const double &minSigma_UB, const double &initSigmaRange
        , const double &sigmaRange_LB, const double &sigmaRange_UB, const double &initSigmaRangePercentage
        , const double &initSigmaSigma, const double &sigmaSigma_LB, const double &sigmaSigma_UB
    ) {
        // We will add parameter types in the same order as the arguments

        // Make sure, p has the correct size
        p->clear();
        p->resize_empty(MOT_NVAR); // Will add empty smart pointers to the collection

        //------------------------------------------------------------
        // nParents

        // Small number of possible values -- use a flip-adaptor
        std::shared_ptr<GInt32FlipAdaptor> gifa_ptr(new GInt32FlipAdaptor());
        gifa_ptr->setAdaptionProbability(1.);

        std::shared_ptr<GConstrainedInt32Object>
            npar_ptr(
            new GConstrainedInt32Object(
                boost::numeric_cast<std::int32_t>(initNParents)
                , boost::numeric_cast<std::int32_t>(nParents_LB)
                , boost::numeric_cast<std::int32_t>(nParents_UB)
            )
        );
        npar_ptr->addAdaptor(gifa_ptr);
        npar_ptr->setParameterName("nParents");

        // Add to the individual
        p->at(MOT_NPARENTS) = npar_ptr;

        assert(p->at(MOT_NPARENTS));

        //------------------------------------------------------------
        // nChildren

        // Create a default standard gauss adaptor
        std::shared_ptr<GInt32GaussAdaptor> giga_ptr(
            new GInt32GaussAdaptor(
                0.025   // sigma
                , 0.2   // sigmaSigma
                , 0.001   // minSigma
                , 0.5    // maxSigma
                , 1.   // adProb
            )
        );

        std::shared_ptr<GConstrainedInt32Object> nch_ptr(
            new GConstrainedInt32Object(
                boost::numeric_cast<std::int32_t>(initNChildren)
                , boost::numeric_cast<std::int32_t>(nChildren_LB)
                , boost::numeric_cast<std::int32_t>(nChildren_UB)
            )
        );
        nch_ptr->addAdaptor(giga_ptr);
        nch_ptr->setParameterName("nChildren");

        // Add to the individual
        p->at(MOT_NCHILDREN) = nch_ptr;

        //------------------------------------------------------------
        // amalgamationLklh

        // Create a default standard gauss adaptor
        std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
            new GDoubleGaussAdaptor(
                0.025    // sigma
                , 0.2    // sigmaSigma
                , 0.001  // minSigma
                , 0.5    // maxSigma
                , 1.     // adProb
            )
        );
        std::shared_ptr<GConstrainedDoubleObject> amalgamationLklh_ptr(
            new GConstrainedDoubleObject(
                initAmalgamationLklh // initial value
                , amalgamationLklh_LB  // lower boundary
                , amalgamationLklh_UB // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        amalgamationLklh_ptr->addAdaptor(gdga_ptr);
        amalgamationLklh_ptr->setParameterName("amalgamationLikelihood");

        // Add to the individual
        p->at(MOT_AMALGAMATION) = amalgamationLklh_ptr;

        //------------------------------------------------------------
        // minAdProb

        std::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr(
            new GConstrainedDoubleObject(
                initMinAdProb  // initial value
                , minAdProb_LB // lower boundary
                , minAdProb_UB // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        minAdProb_ptr->addAdaptor(gdga_ptr);
        minAdProb_ptr->setParameterName("minAdProb");

        // Add to the individual
        p->at(MOT_MINADPROB) = minAdProb_ptr;

        //------------------------------------------------------------
        // adProbRange

        std::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr(
            new GConstrainedDoubleObject(
                initAdProbRange  // initial value
                , adProbRange_LB // lower boundary
                , adProbRange_UB // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        adProbRange_ptr->addAdaptor(gdga_ptr);
        adProbRange_ptr->setParameterName("adProbRange");

        // Add to the individual
        p->at(MOT_ADPROBRANGE) = adProbRange_ptr;

        //------------------------------------------------------------
        // adProbStartPercentage

        std::shared_ptr<GConstrainedDoubleObject> adProbStartPercentage_ptr(
            new GConstrainedDoubleObject(
                initAdProbStartPercentage  // initial value
                , 0. // lower boundary
                , 1. // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        adProbStartPercentage_ptr->addAdaptor(gdga_ptr);
        adProbStartPercentage_ptr->setParameterName("adProbStartPercentage");

        // Add to the individual
        p->at(MOT_ADPROBSTARTPERCENTAGE) = adProbStartPercentage_ptr;

        //------------------------------------------------------------
        // adaptAdProb

        std::shared_ptr<GConstrainedDoubleObject> adaptAdProb_ptr(
            new GConstrainedDoubleObject(
                initAdaptAdProb // initial value
                , adaptAdProb_LB // lower boundary
                , adaptAdProb_UB // upper boundary
            ));
        adaptAdProb_ptr->addAdaptor(gdga_ptr);
        adaptAdProb_ptr->setParameterName("adaptAdProb");

        // Add to the individual
        p->at(MOT_ADAPTADPROB) = adaptAdProb_ptr;

        //------------------------------------------------------------
        // minSigma

        std::shared_ptr<GConstrainedDoubleObject> minsigma_ptr(
            new GConstrainedDoubleObject(
                initMinSigma // initial value
                , minSigma_LB // lower boundary
                , minSigma_UB // upper boundary
            ));
        minsigma_ptr->addAdaptor(gdga_ptr);
        minsigma_ptr->setParameterName("minSigma");

        // Add to the individual
        p->at(MOT_MINSIGMA) = minsigma_ptr;

        //------------------------------------------------------------
        // sigmaRange

        std::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr(
            new GConstrainedDoubleObject(
                initSigmaRange // initial value
                , sigmaRange_LB // lower boundary
                , sigmaRange_UB // upper boundary
            ));
        sigmarange_ptr->addAdaptor(gdga_ptr);
        sigmarange_ptr->setParameterName("sigmaRange");

        // Add to the individual
        p->at(MOT_SIGMARANGE) = sigmarange_ptr;

        //------------------------------------------------------------
        // sigmaRangePercentage

        std::shared_ptr<GConstrainedDoubleObject> sigmaRangePercentage_ptr(
            new GConstrainedDoubleObject(
                initSigmaRangePercentage  // initial value
                , 0. // lower boundary
                , 1. // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        sigmaRangePercentage_ptr->addAdaptor(gdga_ptr);
        sigmaRangePercentage_ptr->setParameterName("sigmaRangePercentage");

        // Add to the individual
        p->at(MOT_SIGMARANGEPERCENTAGE) = sigmaRangePercentage_ptr;

        //------------------------------------------------------------
        // sigmaSigma

        // The sigma adaption strength may change between 0.01 and 1
        std::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr(
            new GConstrainedDoubleObject(
                initSigmaSigma  // initial value
                , sigmaSigma_LB  // lower boundary
                , sigmaSigma_UB  // upper boundary
            ));
        sigmasigma_ptr->addAdaptor(gdga_ptr);
        sigmasigma_ptr->setParameterName("sigmaSigma");

        // Add to the individual
        p->at(MOT_SIGMASIGMA) = sigmasigma_ptr;

        //------------------------------------------------------------
    }

    /***************************************************************************/
    /**
     * Emit information about this individual
     */
    std::string print(bool withFitness = true) const {
        std::ostringstream result;

        // Retrieve the parameters
        std::shared_ptr<GConstrainedInt32Object> npar_ptr = this->at<GConstrainedInt32Object>(MOT_NPARENTS);
        std::shared_ptr<GConstrainedInt32Object> nch_ptr = this->at<GConstrainedInt32Object>(MOT_NCHILDREN);
        std::shared_ptr<GConstrainedDoubleObject>
            amalgamation_ptr = this->at<GConstrainedDoubleObject>(MOT_AMALGAMATION);
        std::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr = this->at<GConstrainedDoubleObject>(MOT_MINADPROB);
        std::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBRANGE);
        std::shared_ptr<GConstrainedDoubleObject>
            adProbStartPercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);
        std::shared_ptr<GConstrainedDoubleObject> adaptAdprob_ptr = this->at<GConstrainedDoubleObject>(MOT_ADAPTADPROB);
        std::shared_ptr<GConstrainedDoubleObject> minsigma_ptr = this->at<GConstrainedDoubleObject>(MOT_MINSIGMA);
        std::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGE);
        std::shared_ptr<GConstrainedDoubleObject>
            sigmaRangePercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGEPERCENTAGE);
        std::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr = this->at<GConstrainedDoubleObject>(MOT_SIGMASIGMA);

        // Stream the results

        bool unprocessed = (not this->is_processed() || this->has_errors());
        double transformedPrimaryFitness = unprocessed ? this->getWorstCase() : this->transformed_fitness(0);

        result
            << "============================================================================================"
            << std::endl;

        if (withFitness) {
            result << "Fitness = " << transformedPrimaryFitness << (unprocessed ? " // unprocessed or error" : "")
                   << std::endl;
        }

        result
            << "Optimization target: " << getClearTextMOT(moTarget_) << std::endl
            << std::endl
            << "population::population size = " << npar_ptr->value() + nch_ptr->value() << std::endl
            << "population::nParents = " << npar_ptr->value() << std::endl
            << "population::nChildren = " << nch_ptr->value() << std::endl
            << "population::amalgamationLikelihood = " << amalgamation_ptr->value() << std::endl
            << "individual::adProbRange = " << adProbRange_ptr->value() << std::endl
            << "individual::adProbStartPercentage_ptr = " << adProbStartPercentage_ptr->value() << std::endl
            << "individual::adProb = " <<
            minAdProb_ptr->value() + adProbRange_ptr->value() * adProbStartPercentage_ptr->value() << std::endl
            << "individual::minAdProb = " << minAdProb_ptr->value() << std::endl
            << "individual::maxAdProb = " << minAdProb_ptr->value() + adProbRange_ptr->value() << std::endl
            << "individual::adaptAdProb = " << adaptAdprob_ptr->value() << std::endl
            << "individual::sigmarange_ptr = " << sigmarange_ptr->value() << std::endl
            << "individual::sigmaRangePercentage_ptr = " << sigmaRangePercentage_ptr->value() << std::endl
            << "individual::sigma1 = " <<
            minsigma_ptr->value() + sigmarange_ptr->value() * sigmaRangePercentage_ptr->value() << std::endl
            << "individual::minSigma1 = " << minsigma_ptr->value() << std::endl
            << "individual::maxSigma1 = " << minsigma_ptr->value() + sigmarange_ptr->value() << std::endl
            << "individual::sigmaSigma1 = " << sigmasigma_ptr->value() << std::endl
            << "============================================================================================"
            << std::endl
            << std::endl;

        return result.str();
    }

    /***************************************************************************/
    /**
     * Registers a factory class with this object. This function clones the factory,
     * so the individual can be sure to have a unique factory.
     */
    void registerIndividualFactory(std::shared_ptr<typename ind_type::FACTORYTYPE> factory) {
        if (not factory) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GMetaOptimizerIndividualT<T>::registerIndividualFactory(): Error!" << std::endl
                    << "Individual is empty" << std::endl
            );
        }

        ind_factory_
            = Gem::Common::convertSmartPointer<Gem::Common::GFactoryT<GParameterSet>, typename ind_type::FACTORYTYPE>(
            factory->clone());
    }

protected:
    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(
        Gem::Common::GParserBuilder &gpb
    ) override {
        // Call our parent class'es function
        GParameterSet::addConfigurationOptions_(gpb);

        // Add local data
        gpb.registerFileParameter<std::size_t>(
            "nRunsPerOptimization" // The name of the variable
            , GMETAOPT_DEF_NRUNSPEROPT // The default value
            , [this](std::size_t nrpo) { this->setNRunsPerOptimization(nrpo); }
        )
            << "Specifies the number of optimizations performed";

        gpb.registerFileParameter<double>(
            "fitnessTarget" // The name of the variable
            , GMETAOPT_DEF_FITNESSTARGET // The default value
            , [this](double ft) { this->setFitnessTarget(ft); }
        )
            << "The fitness below which optimization should stop";

        gpb.registerFileParameter<std::uint32_t>(
            "iterationThreshold" // The name of the variable
            , GMETAOPT_DEF_ITERATIONTHRESHOLD // The default value
            , [this](std::uint32_t dit) { this->setIterationThreshold(dit); }
        )
            << "The maximum number of iterations per sub-optimization";

        gpb.registerFileParameter<metaOptimizationTarget>(
            "metaOptimizationTarget" // The name of the variable
            , GMETAOPT_DEF_MOTARGET // The default value
            , [this](metaOptimizationTarget mot) { this->setMetaOptimizationTarget(mot); }
        )
            << "The target for the meta-optimization: best fitness (0)," << std::endl
            << "minimum number of solver calls (1), multi-criterion with best fitness" << std::endl
            << "and smallest number of solver calls as target (2);";

        gpb.registerFileParameter<std::string>(
            "subEAConfig" // The name of the variable
            , GMETAOPT_DEF_SUBEACONFIG // The default value
            , [this](std::string seac) { this->setSubEAConfig(seac); }
        )
            << "Path and name of the configuration file used for the (sub-)evolutionary algorithm";
    }

    /***************************************************************************/
    /**
     * Loads the data of another GMetaOptimizerIndividualT<ind_type>, camouflaged as a GObject
     *
     * @param cp A copy of another GMetaOptimizerIndividualT<ind_type>, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GMetaOptimizerIndividualT<ind_type> reference independent of this object and convert the pointer
        const GMetaOptimizerIndividualT<ind_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GMetaOptimizerIndividualT<ind_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GParameterSet::load_(cp);

        // ... and then our local data
        nRunsPerOptimization_ = p_load->nRunsPerOptimization_;
        fitnessTarget_ = p_load->fitnessTarget_;
        iterationThreshold_ = p_load->iterationThreshold_;
        moTarget_ = p_load->moTarget_;
        subEA_config_ = p_load->subEA_config_;

        // We simply keep our local individual factory, as all settings are made inside of fitnessCalculation
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GMetaOptimizerIndividualT<ind_type>>(
        GMetaOptimizerIndividualT<ind_type> const &
        , GMetaOptimizerIndividualT<ind_type> const &
        , Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    virtual void compare_(
        const GObject &cp, const Gem::Common::expectation &e, const double &limit
    ) const final {
        // Check that we are dealing with a GMetaOptimizerIndividualT<ind_type> reference independent of this object and convert the pointer
        const GMetaOptimizerIndividualT<ind_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GMetaOptimizerIndividualT<ind_type>>(
            cp
            , this
        );

        Gem::Common::GToken token(
            "GMetaOptimizerIndividualT<ind_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<Gem::Geneva::GParameterSet>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        Gem::Common::compare_t(
            IDENTITY(nRunsPerOptimization_
                     , p_load->nRunsPerOptimization_)
            , token
        );
        Gem::Common::compare_t(
            IDENTITY(fitnessTarget_
                     , p_load->fitnessTarget_)
            , token
        );
        Gem::Common::compare_t(
            IDENTITY(iterationThreshold_
                     , p_load->iterationThreshold_)
            , token
        );
        Gem::Common::compare_t(
            IDENTITY(moTarget_
                     , p_load->moTarget_)
            , token
        );
        Gem::Common::compare_t(
            IDENTITY(subEA_config_
                     , p_load->subEA_config_)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * The actual value calculation takes place here
     *
     * @param The id of the target function (ignored here)
     * @return The value of this object, as calculated with the evaluation function
     */
    double fitnessCalculation() override {
        // Retrieve the parameters
        std::shared_ptr<GConstrainedInt32Object> npar_ptr = this->at<GConstrainedInt32Object>(MOT_NPARENTS);
        std::shared_ptr<GConstrainedInt32Object> nch_ptr = this->at<GConstrainedInt32Object>(MOT_NCHILDREN);
        std::shared_ptr<GConstrainedDoubleObject>
            amalgamation_ptr = this->at<GConstrainedDoubleObject>(MOT_AMALGAMATION);
        std::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr = this->at<GConstrainedDoubleObject>(MOT_MINADPROB);
        std::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBRANGE);
        std::shared_ptr<GConstrainedDoubleObject>
            adProbStartPercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);
        std::shared_ptr<GConstrainedDoubleObject> adaptAdprob_ptr = this->at<GConstrainedDoubleObject>(MOT_ADAPTADPROB);
        std::shared_ptr<GConstrainedDoubleObject> minsigma_ptr = this->at<GConstrainedDoubleObject>(MOT_MINSIGMA);
        std::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGE);
        std::shared_ptr<GConstrainedDoubleObject>
            sigmaRangePercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGEPERCENTAGE);
        std::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr = this->at<GConstrainedDoubleObject>(MOT_SIGMASIGMA);

#ifdef DEBUG
        // Check that we have been given a factory
        if (not ind_factory_) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GMetaOptimizerIndividualT<T>::fitnessCalculation(): Error!" << std::endl
                    << "No factory class for individuals has been registered" << std::endl
            );
        }
#endif

        // Set the parameters
        double minSigma = minsigma_ptr->value();
        double sigmaRange = sigmarange_ptr->value();
        double maxSigma = minSigma + sigmaRange;
        double sigmaRangePercentage = sigmaRangePercentage_ptr->value();
        double startSigma = minSigma + sigmaRangePercentage * sigmaRange;

        ind_factory_->setSigma1Range(
            std::tuple<double, double>(
                minSigma
                , maxSigma
            ));
        ind_factory_->setSigma1(startSigma);
        ind_factory_->setSigmaSigma1(sigmasigma_ptr->value());

        double minAdProb = minAdProb_ptr->value();
        double adProbRange = adProbRange_ptr->value();
        double maxAdProb = minAdProb + adProbRange;
        double adProbStartPercentage = adProbStartPercentage_ptr->value();
        double startAdProb = minAdProb + adProbStartPercentage * adProbRange;

        double adaptAdProb = adaptAdprob_ptr->value();

        ind_factory_->setAdProbRange(
            minAdProb
            , maxAdProb
        );
        ind_factory_->setAdProb(startAdProb);
        ind_factory_->setAdaptAdProb(adaptAdProb);

        // Set up a population factory for serial execution
        GEvolutionaryAlgorithmFactory ea(subEA_config_);

        // Run the required number of optimizations
        std::shared_ptr<GEvolutionaryAlgorithm> ea_ptr;

        std::uint32_t nChildren = boost::numeric_cast<std::uint32_t>(nch_ptr->value());
        std::uint32_t nParents = boost::numeric_cast<std::uint32_t>(npar_ptr->value());
        std::uint32_t popSize = nParents + nChildren;
        std::uint32_t iterationsConsumed = 0;
        double amalgamationLikelihood = amalgamation_ptr->value();

        std::vector<double> solverCallsPerOptimization;
        std::vector<double> iterationsPerOptimization;
        std::vector<double> bestEvaluations;

        for (std::size_t opt = 0; opt < nRunsPerOptimization_; opt++) {
            std::cout << "Starting measurement " << opt + 1 << " / " << nRunsPerOptimization_ << std::endl;
            ea_ptr = ea.get<GEvolutionaryAlgorithm>();

            // Register an executor
            ea_ptr->registerExecutor(
                execMode::SERIAL
                , "./config/GMetaOptimizerSerialExecutor.json"
            );

            // Set the population parameters
            ea_ptr->setPopulationSizes(
                popSize
                , nParents
            );

            // Add the required number of individuals
            for (std::size_t ind = 0; ind < popSize; ind++) {
                // Retrieve an individual
                std::shared_ptr<GParameterSet> gi_ptr = ind_factory_->get();

                ea_ptr->push_back(gi_ptr);
            }

            // Set the likelihood for work items to be produced through cross-over rather than mutation alone
            ea_ptr->setAmalgamationLikelihood(amalgamationLikelihood);

            if (metaOptimizationTarget::MINSOLVERCALLS == moTarget_) {
                // Set the stop criteria (either maxIterations_ iterations or falling below the quality threshold
                ea_ptr->setQualityThreshold(
                    fitnessTarget_
                    , true
                );
                ea_ptr->setMaxIteration(iterationThreshold_);

                // Make sure the optimization does not emit the termination reason
                ea_ptr->setEmitTerminationReason(false);

                // Make sure the optimization does not stop due to stalls (which is the default in the EA-config
                ea_ptr->setMaxStallIteration(0);
            } else { // Optimization of best fitness found or multi-criterion optimization: BESTFITNESS / MC_MINSOLVER_BESTFITNESS
                // Set the stop criterion maxIterations only
                ea_ptr->setMaxIteration(iterationThreshold_);

                // Make sure the optimization does not emit the termination reason
                ea_ptr->setEmitTerminationReason(false);

                // Set a relatively high stall threshold
                ea_ptr->setMaxStallIteration(50);
            }

            // Make sure the optimization is quiet
            ea_ptr->setReportIteration(0);

            // Run the actual optimization
            ea_ptr->optimize();

            // Retrieve the best individual
            std::shared_ptr<GParameterSet> bestIndividual = ea_ptr->getBestGlobalIndividual<GParameterSet>();

            // Retrieve the number of iterations
            iterationsConsumed = ea_ptr->getIteration();

            // Do book-keeping
            solverCallsPerOptimization.push_back(double((iterationsConsumed + 1) * nChildren + nParents));
            iterationsPerOptimization.push_back(double(iterationsConsumed + 1));
            bestEvaluations.push_back(
                bestIndividual->transformed_fitness(0)
            ); // We use the transformed fitness to avoid MAX_DOUBLE
        }

        // Calculate the average number of iterations and solver calls
        std::tuple<double, double> sd = Gem::Common::GStandardDeviation(solverCallsPerOptimization);
        std::tuple<double, double> itmean = Gem::Common::GStandardDeviation(iterationsPerOptimization);
        std::tuple<double, double> bestMean = Gem::Common::GStandardDeviation(bestEvaluations);

        double evaluation = 0.;
        if (metaOptimizationTarget::MINSOLVERCALLS == moTarget_) {
            evaluation = std::get<0>(sd);
        } else if (metaOptimizationTarget::BESTFITNESS == moTarget_) {
            evaluation = std::get<0>(bestMean);
        } else if (metaOptimizationTarget::MC_MINSOLVER_BESTFITNESS == moTarget_) {
            evaluation = std::get<0>(bestMean);
            this->setResult(
                1
                , std::get<0>(sd)); // The secondary result
        }

        // Emit some information
        std::cout
            << std::endl
            << std::get<0>(sd) << " +/- " << std::get<1>(sd) << " solver calls with " << std::endl
            << std::get<0>(itmean) << " +/- " << std::get<1>(itmean) << " average iterations " << std::endl
            << "and a best evaluation of " << std::get<0>(bestMean) << " +/- " << std::get<1>(bestMean) << std::endl
            << "out of " << nRunsPerOptimization_ << " consecutive runs" << std::endl
            << "fitnessCalculation() will return the value " << evaluation << std::endl
            << this->print(false) << std::endl // print without fitness -- not defined at this stage
            << std::endl;

        // Let the audience know
        return evaluation;
    }

    /***************************************************************************/
    /**
     * Retrieves a clear-text description of the optimization target
     */
    std::string getClearTextMOT(const metaOptimizationTarget &mot) const {
        switch (mot) {
            case metaOptimizationTarget::BESTFITNESS: return std::string("\"best fitness\"");
                break;

            case metaOptimizationTarget::MINSOLVERCALLS: return std::string("\"minimum number of solver calls\"");
                break;

            case metaOptimizationTarget::MC_MINSOLVER_BESTFITNESS: return std::string("\"multi-criterion target with best fitness, minimum number of solver calls\"");
                break;
        }

        // Make the compiler happy
        return std::string();
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object.
     *
     * @return A boolean indicating whether
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        bool result = false;

        // Call the parent classes' functions
        if (Gem::Geneva::GParameterSet::modify_GUnitTests_()) { result = true; }

        // Change the parameter settings
        if (not this->empty()) {
            this->adapt();
            result = true;
        }

        // Let the audience know whether we have changed the content
        return result;

#else /* GEM_TESTING */
        Gem::Common::condnotset("GMetaOptimizerIndividualT<ind_type>::modify_GUnitTests()", "GEM_TESTING");
     return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed.
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        using namespace Gem::Geneva;

        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        // Call the parent classes' functions
        Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        {
            /* nothing. Add test cases here that are expected to succeed. */
        }

        //------------------------------------------------------------------------------
#else /* GEM_TESTING */
        Gem::Common::condnotset("GMetaOptimizerIndividualT<ind_type>::specificTestsNoFailureExpected_GUnitTests()", "GEM_TESTING");
#endif /* GEM_TESTING */
    }
    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail.
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        using namespace Gem::Geneva;

        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        // Call the parent classes' functions
        Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        {
            /* Nothing. Add test cases here that are expected to fail.
                Enclose with a BOOST_CHECK_THROW, using the expected
                exception type as an additional argument. See the
                documentation for the Boost.Test library for further
                information */
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */
        Gem::Common::condnotset("GMetaOptimizerIndividualT<ind_type>::specificTestsNoFailureExpected_GUnitTests()", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
     * Creates a deep clone of this object
     *
     * @return A deep clone of this object, camouflaged as a GObject
     */
    virtual GObject *clone_() const final {
        return new GMetaOptimizerIndividualT<ind_type>(*this);
    }

    /***************************************************************************/

    std::size_t nRunsPerOptimization_; ///< The number of runs performed for each (sub-)optimization
    double fitnessTarget_; ///< The quality target to be reached by
    std::uint32_t iterationThreshold_; ///< The maximum allowed number of iterations
    metaOptimizationTarget moTarget_; ///< The target used for the meta-optimization
    std::string individual_config_; ///< Path and name of the configuration file needed for the individual
    std::string subEA_config_; ///< Path and name of the configuration file needed for (sub-)evolutionary algorithms

    std::shared_ptr<typename ind_type::FACTORYTYPE> ind_factory_; ///< Holds a factory for our individuals
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to output a GMetaOptimizerIndividualT<ind_type> or convert it to a string using
 * boost::lexical_cast
 */
template<typename ind_type>
std::ostream &operator<<(
    std::ostream &stream, const GMetaOptimizerIndividualT<ind_type> &gsi
) {
    stream << gsi.print();
    return stream;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GMetaOptimizerIndividualT<ind_type> objects
 */
template<typename ind_type>
class GMetaOptimizerIndividualFactoryT
    : public Gem::Common::GFactoryT<GParameterSet>
{
public:
    /***************************************************************************/
    /**
     * A constructor with the ability to switch the parallelization mode. It initializes a
     * target item as needed.
     *
     * @param configFile The name of the configuration file
     */
    GMetaOptimizerIndividualFactoryT(boost::filesystem::path const &configFile)
        :
        Gem::Common::GFactoryT<GParameterSet>(configFile) { /* nothing */ }

    /***************************************************************************/
    /**
     * The destructor
     */
    virtual ~GMetaOptimizerIndividualFactoryT() = default;

    /***************************************************************************/
    /**
     * Registers a factory class with this object. This function clones the factory,
     * so the individual can be sure to have a unique factory.
     */
    void registerIndividualFactory(std::shared_ptr<typename ind_type::FACTORYTYPE> factory) {
        if (not factory) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GMetaOptimizerIndividualFactoryT<T>::registerIndividualFactory(): Error!" << std::endl
                    << "Individual is empty" << std::endl
            );
        }

        m_ind_factory
            = Gem::Common::convertSmartPointer<Gem::Common::GFactoryT<GParameterSet>, typename ind_type::FACTORYTYPE>(
            factory->clone());
    }

protected:
    /***************************************************************************/
    /**
     * Allows to describe local configuration options for gradient descents
     */
    void describeLocalOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Describe our own options
        using namespace Gem::Courtier;

        std::string comment;

        comment = "";
        comment += "The initial number of parents in a population;";
        gpb.registerFileParameter<std::size_t>(
            "initNParents"
            , initNParents_
            , GMETAOPT_DEF_INITNPARENTS
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for variations of the number of parents;";
        gpb.registerFileParameter<std::size_t>(
            "nParents_LB"
            , nParents_LB_
            , GMETAOPT_DEF_NPARENTS_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for variations of the number of parents;";
        gpb.registerFileParameter<std::size_t>(
            "nParents_UB"
            , nParents_UB_
            , GMETAOPT_DEF_NPARENTS_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial number of children in a population;";
        gpb.registerFileParameter<std::size_t>(
            "initNChildren"
            , initNChildren_
            , GMETAOPT_DEF_INITNCHILDREN
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the number of children;";
        gpb.registerFileParameter<std::size_t>(
            "nChildren_LB"
            , nChildren_LB_
            , GMETAOPT_DEF_NCHILDREN_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the number of children;";
        gpb.registerFileParameter<std::size_t>(
            "nChildren_UB"
            , nChildren_UB_
            , GMETAOPT_DEF_NCHILDREN_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial likelihood for an individual being created from cross-over rather than just duplication;";
        gpb.registerFileParameter<double>(
            "initAmalgamationLklh"
            , initAmalgamationLklh_
            , GMETAOPT_DEF_INITAMALGLKLHOOD
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the amalgamation likelihood ;";
        gpb.registerFileParameter<double>(
            "amalgamationLklh_LB"
            , amalgamationLklh_LB_
            , GMETAOPT_DEF_AMALGLKLHOOD_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the amalgamation likelihood ;";
        gpb.registerFileParameter<double>(
            "amalgamationLklh_UB"
            , amalgamationLklh_UB_
            , GMETAOPT_DEF_AMALGLKLHOOD_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial lower boundary for the variation of adProb;";
        gpb.registerFileParameter<double>(
            "initMinAdProb"
            , initMinAdProb_
            , GMETAOPT_DEF_INITMINADPROB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for minAdProb;";
        gpb.registerFileParameter<double>(
            "minAdProb_LB"
            , minAdProb_LB_
            , GMETAOPT_DEF_MINADPROB_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for minAdProb;";
        gpb.registerFileParameter<double>(
            "minAdProb_UB"
            , minAdProb_UB_
            , GMETAOPT_DEF_MINADPROB_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial range for the variation of adProb;";
        gpb.registerFileParameter<double>(
            "initAdProbRange"
            , initAdProbRange_
            , GMETAOPT_DEF_INITADPROBRANGE
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for adProbRange;";
        gpb.registerFileParameter<double>(
            "adProbRange_LB"
            , adProbRange_LB_
            , GMETAOPT_DEF_ADPROBRANGE_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for adProbRange;";
        gpb.registerFileParameter<double>(
            "adProbRange_UB"
            , adProbRange_UB_
            , GMETAOPT_DEF_ADPROBRANGE_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The start value for adProb relative to the allowed value range;";
        gpb.registerFileParameter<double>(
            "initAdProbStartPercentage"
            , initAdProbStartPercentage_
            , GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial value of the strength of m_adProb adaption;";
        gpb.registerFileParameter<double>(
            "initAdaptAdProb"
            , initAdaptAdProb_
            , GMETAOPT_DEF_INITADAPTADPROB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the strength of m_adProb adaption;";
        gpb.registerFileParameter<double>(
            "adaptAdProb_LB"
            , adaptAdProb_LB_
            , GMETAOPT_DEF_ADAPTADPROB_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the strength of m_adProb adaption;";
        gpb.registerFileParameter<double>(
            "adaptAdProb_UB"
            , adaptAdProb_UB_
            , GMETAOPT_DEF_ADAPTADPROB_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial minimum sigma for gauss-adaption in ES;";
        gpb.registerFileParameter<double>(
            "initMinSigma"
            , initMinSigma_
            , GMETAOPT_DEF_INITMINSIGMA
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the lower boundary of sigma;";
        gpb.registerFileParameter<double>(
            "minSigma_LB"
            , minSigma_LB_
            , GMETAOPT_DEF_MINSIGMA_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the lower boundary of sigma;";
        gpb.registerFileParameter<double>(
            "minSigma_UB"
            , minSigma_UB_
            , GMETAOPT_DEF_MINSIGMA_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial maximum range for sigma;";
        gpb.registerFileParameter<double>(
            "initSigmaRange"
            , initSigmaRange_
            , GMETAOPT_DEF_INITSIGMARANGE
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the maximum range of sigma;";
        gpb.registerFileParameter<double>(
            "sigmaRange_LB"
            , sigmaRange_LB_
            , GMETAOPT_DEF_SIGMARANGE_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the maximum range of sigma;";
        gpb.registerFileParameter<double>(
            "sigmaRange_UB"
            , sigmaRange_UB_
            , GMETAOPT_DEF_SIGMARANGE_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial percentage of the sigma range as a start value;";
        gpb.registerFileParameter<double>(
            "initSigmaRangePercentage"
            , initSigmaRangePercentage_
            , GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The initial strength of self-adaption of gauss-mutation in ES;";
        gpb.registerFileParameter<double>(
            "initSigmaSigma"
            , initSigmaSigma_
            , GMETAOPT_DEF_INITSIGMASIGMA
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the strength of sigma adaption;";
        gpb.registerFileParameter<double>(
            "sigmaSigma_LB"
            , sigmaSigma_LB_
            , GMETAOPT_DEF_SIGMASIGMA_LB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the strength of sigma adaption;";
        gpb.registerFileParameter<double>(
            "sigmaSigma_UB"
            , sigmaSigma_UB_
            , GMETAOPT_DEF_SIGMASIGMA_UB
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        // Allow our parent class to describe its options
        Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);
    }


    /***************************************************************************/
    /**
     * Allows to act on the configuration options received from the configuration file. Here
     * we can add the options described in describeLocalOptions to the object. In practice,
     * we will usually add the parameter objects here. Note that a very similar constructor
     * exists for GMetaOptimizerIndividualT<ind_type>, so it may be used independently of the factory.
     *
     * @param p A smart-pointer to be acted on during post-processing
     */
    void postProcess_(
        std::shared_ptr<GParameterSet> &p_base
    ) override {
        // Convert the base pointer to our local type
        std::shared_ptr<GMetaOptimizerIndividualT<ind_type>> p
            = Gem::Common::convertSmartPointer<GParameterSet, GMetaOptimizerIndividualT<ind_type>>(p_base);

        // We simply use a static function defined in GMetaOptimizerIndividualT<ind_type>
        GMetaOptimizerIndividualT<ind_type>::addContent(
            p
            , initNParents_
            , nParents_LB_
            , nParents_UB_
            , initNChildren_
            , nChildren_LB_
            , nChildren_UB_
            , initAmalgamationLklh_
            , amalgamationLklh_LB_
            , amalgamationLklh_UB_
            , initMinAdProb_
            , minAdProb_LB_
            , minAdProb_UB_
            , initAdProbRange_
            , adProbRange_LB_
            , adProbRange_UB_
            , initAdProbStartPercentage_
            , initAdaptAdProb_
            , adaptAdProb_LB_
            , adaptAdProb_UB_
            , initMinSigma_
            , minSigma_LB_
            , minSigma_UB_
            , initSigmaRange_
            , sigmaRange_LB_
            , sigmaRange_UB_
            , initSigmaRangePercentage_
            , initSigmaSigma_
            , sigmaSigma_LB_
            , sigmaSigma_UB_
        );

        // Finally add the individual factory to p
        p->registerIndividualFactory(m_ind_factory);
    }

private:
    /***************************************************************************/
    /** @brief The default constructor. Only needed for (de-)serialization */
    GMetaOptimizerIndividualFactoryT() = default;

    /***************************************************************************/
    /**
     * Creates items of this type
     *
     * @return Items of the desired type
     */
    std::shared_ptr<GParameterSet> getObject_(
            Gem::Common::GParserBuilder &gpb, const std::size_t &id
    ) override {
        // Will hold the result
        std::shared_ptr<GMetaOptimizerIndividualT<ind_type>>
                target(new GMetaOptimizerIndividualT<ind_type>());

        // Make the object's local configuration options known
        target->addConfigurationOptions(gpb);

        return target;
    }

    /***************************************************************************/
    // Data

    // Parameters pertaining to the ea population
    std::size_t initNParents_ = GMETAOPT_DEF_INITNPARENTS;  ///< The initial number of parents
    std::size_t
        nParents_LB_ = GMETAOPT_DEF_NPARENTS_LB;    ///< The lower boundary for variations of the number of parents
    std::size_t
        nParents_UB_ = GMETAOPT_DEF_NPARENTS_UB;    ///< The upper boundary for variations of the number of parents

    std::size_t initNChildren_ = GMETAOPT_DEF_INITNCHILDREN; ///< The initial number of children
    std::size_t
        nChildren_LB_ = GMETAOPT_DEF_NCHILDREN_LB;   ///< The lower boundary for the variation of the number of children
    std::size_t
        nChildren_UB_ = GMETAOPT_DEF_NCHILDREN_UB;   ///< The upper boundary for the variation of the number of children

    double initAmalgamationLklh_
        = GMETAOPT_DEF_INITAMALGLKLHOOD;  ///< The initial likelihood for an individual being created from cross-over rather than "just" duplication
    double amalgamationLklh_LB_
        = GMETAOPT_DEF_AMALGLKLHOOD_LB;    ///< The upper boundary for the variation of the amalgamation likelihood
    double amalgamationLklh_UB_
        = GMETAOPT_DEF_AMALGLKLHOOD_UB;    ///< The upper boundary for the variation of the amalgamation likelihood

    double initMinAdProb_ = GMETAOPT_DEF_INITMINADPROB;      ///< The initial lower boundary for the variation of adProb
    double minAdProb_LB_ = GMETAOPT_DEF_MINADPROB_LB;       ///< The lower boundary for minAdProb
    double minAdProb_UB_ = GMETAOPT_DEF_MINADPROB_UB;       ///< The upper boundary for minAdProb

    double initAdProbRange_ = GMETAOPT_DEF_INITADPROBRANGE;    ///< The initial range for the variation of adProb
    double adProbRange_LB_ = GMETAOPT_DEF_ADPROBRANGE_LB;     ///< The lower boundary for adProbRange
    double adProbRange_UB_ = GMETAOPT_DEF_ADPROBRANGE_UB;     ///< The upper boundary for adProbRange

    double initAdProbStartPercentage_
        = GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE; ///< The start value for adProb relative to the allowed value range

    double
        initAdaptAdProb_ = GMETAOPT_DEF_INITADAPTADPROB;    ///< The initial value of the strength of adProb_ adaption
    double adaptAdProb_LB_
        = GMETAOPT_DEF_ADAPTADPROB_LB;      ///< The lower boundary for the variation of the strength of adProb_ adaption
    double adaptAdProb_UB_
        = GMETAOPT_DEF_ADAPTADPROB_UB;      ///< The upper boundary for the variation of the strength of adProb_ adaption

    double initMinSigma_ = GMETAOPT_DEF_INITMINSIGMA;       ///< The initial minimal value of sigma
    double minSigma_LB_
        = GMETAOPT_DEF_MINSIGMA_LB;         ///< The lower boundary for the variation of the lower boundary of sigma
    double minSigma_UB_
        = GMETAOPT_DEF_MINSIGMA_UB;         ///< The upper boundary for the variation of the lower boundary of sigma

    double initSigmaRange_ = GMETAOPT_DEF_INITSIGMARANGE;     ///< The initial range of sigma (beyond minSigma
    double sigmaRange_LB_
        = GMETAOPT_DEF_SIGMARANGE_LB;       ///< The lower boundary for the variation of the maximum range of sigma
    double sigmaRange_UB_
        = GMETAOPT_DEF_SIGMARANGE_UB;       ///< The upper boundary for the variation of the maximum range of sigma

    double initSigmaRangePercentage_
        = GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE; ///< The initial percentage of the sigma range as a start value

    double initSigmaSigma_ = GMETAOPT_DEF_INITSIGMASIGMA;     ///< The initial strength of sigma adaption
    double sigmaSigma_LB_
        = GMETAOPT_DEF_SIGMASIGMA_LB;       ///< The lower boundary for the variation of the strength of sigma adaption
    double sigmaSigma_UB_
        = GMETAOPT_DEF_SIGMASIGMA_UB;       ///< The upper boundary for the variation of the strength of sigma adaption

    std::shared_ptr<typename ind_type::FACTORYTYPE>
        m_ind_factory; ///< Holds a factory for our individuals. It will be added to the individuals when needed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

const std::size_t P_XDIM = 1200;
const std::size_t P_YDIM = 1400;

/******************************************************************************/
/**
 * This class implements a pluggable optimization monitor for Evolutionary Algorithms. Its main purpose
 * is to find out information about the development of various properties (e.g. sigma, best evaluation)
 * over the course of the optimization for the best individuals. This monitor is thus targeted at a specific
 * type of individual. Note that the class uses ROOT scripts for the output of its results.
 *
 * TODO: templatize this class on executor_type, like is being done for the other optimization monitors
 */
template<typename ind_type>
class GOptOptMonitorT
    :
        public GBasePluggableOM
{
    // Make sure this class can only be instantiated if individual_type is a derivative of GParameterSet
    static_assert(
        std::is_base_of<GParameterSet, ind_type>::value
        , "GParameterSet is no base class of ind_type"
    );

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GBasePluggableOM"
            , boost::serialization::base_object<GBasePluggableOM>(*this))
        & BOOST_SERIALIZATION_NVP(m_fileName)
        & BOOST_SERIALIZATION_NVP(m_gpd)
        & BOOST_SERIALIZATION_NVP(m_progressPlotter)
        & BOOST_SERIALIZATION_NVP(m_nParentPlotter)
        & BOOST_SERIALIZATION_NVP(m_nChildrenPlotter)
        & BOOST_SERIALIZATION_NVP(m_adProbPlotter)
        & BOOST_SERIALIZATION_NVP(m_minSigmaPlotter)
        & BOOST_SERIALIZATION_NVP(m_maxSigmaPlotter)
        & BOOST_SERIALIZATION_NVP(m_sigmaRangePlotter)
        & BOOST_SERIALIZATION_NVP(m_sigmaSigmaPlotter);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GOptOptMonitorT(const std::string fileName)
        :
        m_fileName(fileName)
        , m_gpd(
        "Progress information"
        , 2
        , 4
    )
        , m_progressPlotter(new Gem::Common::GGraph2D())
        , m_nParentPlotter(new Gem::Common::GGraph2D())
        , m_nChildrenPlotter(new Gem::Common::GGraph2D())
        , m_adProbPlotter(new Gem::Common::GGraph2D())
        , m_minSigmaPlotter(new Gem::Common::GGraph2D())
        , m_maxSigmaPlotter(new Gem::Common::GGraph2D())
        , m_sigmaRangePlotter(new Gem::Common::GGraph2D())
        , m_sigmaSigmaPlotter(new Gem::Common::GGraph2D()) { /* nothing */ }

    /***************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GOptOptMonitorT object
     */
    GOptOptMonitorT(const GOptOptMonitorT<ind_type> &cp)
        :
        GBasePluggableOM(cp)
        , m_fileName(cp.m_fileName)
        , m_gpd(
        "Progress information"
        , 2
        , 4
    ) // We do not want to copy progress information of another object
        , m_progressPlotter(new Gem::Common::GGraph2D())
        , m_nParentPlotter(new Gem::Common::GGraph2D())
        , m_nChildrenPlotter(new Gem::Common::GGraph2D())
        , m_adProbPlotter(new Gem::Common::GGraph2D())
        , m_minSigmaPlotter(new Gem::Common::GGraph2D())
        , m_maxSigmaPlotter(new Gem::Common::GGraph2D())
        , m_sigmaRangePlotter(new Gem::Common::GGraph2D())
        , m_sigmaSigmaPlotter(new Gem::Common::GGraph2D()) { /* nothing */ }

    /***************************************************************************/
    /**
     * The destructor
     */
    virtual ~GOptOptMonitorT() { /* nothing */ }

    /***************************************************************************/
    /**
     * Sets the file name
     */
    void setFileName(std::string fileName) {
        m_fileName = fileName;
    }

    /***************************************************************************/
    /**
     * Retrieves the current file name
     */
    std::string getFileName() const {
        return m_fileName;
    }

protected:
    /***************************************************************************/
    /**
       * Loads the data of another object
       *
       * @param cp A copy of another GOptOptMonitorT object, camouflaged as a GObject
       */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GOptOptMonitorT<ind_type> reference independent of this object and convert the pointer
        const GOptOptMonitorT<ind_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GOptOptMonitorT<ind_type>>(
            cp
            , this
        );

        // Trigger loading of our parent class'es data
        // Load the parent classes' data ...
        GBasePluggableOM::load_(cp);

        // Load local data
        m_fileName = p_load->m_fileName;
        m_gpd = p_load->m_gpd;
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_progressPlotter
            , m_progressPlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_nParentPlotter
            , m_nParentPlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_nChildrenPlotter
            , m_nChildrenPlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_adProbPlotter
            , m_adProbPlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_minSigmaPlotter
            , m_minSigmaPlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_maxSigmaPlotter
            , m_maxSigmaPlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_sigmaRangePlotter
            , m_sigmaRangePlotter
        );
        Gem::Common::copyCloneableSmartPointer(
            p_load->m_sigmaSigmaPlotter
            , m_sigmaSigmaPlotter
        );
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptOptMonitorT<ind_type>>(
        GOptOptMonitorT<ind_type> const &
        , GOptOptMonitorT<ind_type> const &
        , Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    virtual void compare_(
        const GObject &cp
        , const Gem::Common::expectation &e
        , const double &limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GOptOptMonitorT<ind_type> reference independent of this object and convert the pointer
        const GOptOptMonitorT<ind_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GOptOptMonitorT<ind_type>>(
            cp
            , this
        );

        GToken token(
            "GOptOptMonitorT"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GBasePluggableOM>(
            *this
            , *p_load
            , token
        );

        // ... and then our local data
        compare_t(
            IDENTITY(m_fileName
                     , p_load->m_fileName)
            , token
        );
        compare_t(
            IDENTITY(m_gpd
                     , p_load->m_gpd)
            , token
        );
        compare_t(
            IDENTITY(m_progressPlotter
                     , p_load->m_progressPlotter)
            , token
        );
        compare_t(
            IDENTITY(m_nParentPlotter
                     , p_load->m_nParentPlotter)
            , token
        );
        compare_t(
            IDENTITY(m_nChildrenPlotter
                     , p_load->m_nChildrenPlotter)
            , token
        );
        compare_t(
            IDENTITY(m_adProbPlotter
                     , p_load->m_adProbPlotter)
            , token
        );
        compare_t(
            IDENTITY(m_minSigmaPlotter
                     , p_load->m_minSigmaPlotter)
            , token
        );
        compare_t(
            IDENTITY(m_maxSigmaPlotter
                     , p_load->m_maxSigmaPlotter)
            , token
        );
        compare_t(
            IDENTITY(m_sigmaRangePlotter
                     , p_load->m_sigmaRangePlotter)
            , token
        );
        compare_t(
            IDENTITY(m_sigmaSigmaPlotter
                     , p_load->m_sigmaSigmaPlotter)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        bool result = false;

        // Call the parent classes' functions
        if (GBasePluggableOM::modify_GUnitTests_()) {
            result = true;
        }

        // no local data -- nothing to change

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GOptOptMonitorT<ind_type>::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        // Call the parent classes' functions
        GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GOptOptMonitorT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        using boost::unit_test_framework::test_suite;
        using boost::unit_test_framework::test_case;

        // Call the parent classes' functions
        GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GOptOptMonitorT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }
    /***************************************************************************/

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GOptOptMonitorT<>");
    }

    /***************************************************************************/
    /**
       * Creates a deep clone of this object
       *
       * @return A deep clone of this object
       */
    GObject *clone_() const override {
        return new GOptOptMonitorT<ind_type>(*this);
    }

    /***************************************************************************/
    /**
     * Allows to emit information in different stages of the information cycle
     * (initialization, during each cycle and during finalization)
     */
    void informationFunction_(
        infoMode im
        , G_OptimizationAlgorithm_Base const * const goa
    ) override {
        using namespace Gem::Common;

        switch (im) {
            case Gem::Geneva::infoMode::INFOINIT: {
                // Initialize the plots we want to record
                m_progressPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_progressPlotter->setPlotLabel("Number of solver calls");
                m_progressPlotter->setXAxisLabel("Iteration");
                m_progressPlotter->setYAxisLabel("Best Result (lower is better)");

                m_nParentPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_nParentPlotter->setPlotLabel("Number of parents as a function of the iteration");
                m_nParentPlotter->setXAxisLabel("Iteration");
                m_nParentPlotter->setYAxisLabel("Number of parents");

                m_nChildrenPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_nChildrenPlotter->setPlotLabel("Number of children as a function of the iteration");
                m_nChildrenPlotter->setXAxisLabel("Iteration");
                m_nChildrenPlotter->setYAxisLabel("Number of children");

                m_adProbPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_adProbPlotter->setPlotLabel("Adaption probability as a function of the iteration");
                m_adProbPlotter->setXAxisLabel("Iteration");
                m_adProbPlotter->setYAxisLabel("Adaption probability");

                m_minSigmaPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_minSigmaPlotter->setPlotLabel("Lower sigma boundary as a function of the iteration");
                m_minSigmaPlotter->setXAxisLabel("Iteration");
                m_minSigmaPlotter->setYAxisLabel("Lower sigma boundary");

                m_maxSigmaPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_maxSigmaPlotter->setPlotLabel("Upper sigma boundary as a function of the iteration");
                m_maxSigmaPlotter->setXAxisLabel("Iteration");
                m_maxSigmaPlotter->setYAxisLabel("Upper sigma boundary");

                m_sigmaRangePlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_sigmaRangePlotter->setPlotLabel("Development of the sigma range as a function of the iteration");
                m_sigmaRangePlotter->setXAxisLabel("Iteration");
                m_sigmaRangePlotter->setYAxisLabel("Sigma range");

                m_sigmaSigmaPlotter->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                m_sigmaSigmaPlotter->setPlotLabel("Development of the adaption strength as a function of the iteration");
                m_sigmaSigmaPlotter->setXAxisLabel("Iteration");
                m_sigmaSigmaPlotter->setYAxisLabel("Sigma-Sigma");

                m_gpd.registerPlotter(m_progressPlotter);
                m_gpd.registerPlotter(m_nParentPlotter);
                m_gpd.registerPlotter(m_nChildrenPlotter);
                m_gpd.registerPlotter(m_adProbPlotter);
                m_gpd.registerPlotter(m_minSigmaPlotter);
                m_gpd.registerPlotter(m_maxSigmaPlotter);
                m_gpd.registerPlotter(m_sigmaRangePlotter);
                m_gpd.registerPlotter(m_sigmaSigmaPlotter);

                m_gpd.setCanvasDimensions(
                        P_XDIM
                        , P_YDIM
                );
            }
                break;

            case Gem::Geneva::infoMode::INFOPROCESSING: {
                // Convert the base pointer to the target type
                GEvolutionaryAlgorithm const * const ea = static_cast<GEvolutionaryAlgorithm const * const>(goa);

                // Extract the requested data. First retrieve the best individual.
                // It can always be found in the first position with evolutionary algorithms
                std::shared_ptr<GMetaOptimizerIndividualT<ind_type>>
                        p = ea->clone_at<GMetaOptimizerIndividualT<ind_type>>(0);

                // Retrieve the best fitness and average sigma value and add it to our local storage
                (*m_progressPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , p->raw_fitness(0));
                (*m_nParentPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , (double) p->getNParents());
                (*m_nChildrenPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , (double) p->getNChildren());
                (*m_adProbPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , p->getAdProb());

                double minSigma = p->getMinSigma();
                double sigmaRange = p->getSigmaRange();
                double maxSigma = minSigma + sigmaRange;

                (*m_minSigmaPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , minSigma
                );
                (*m_maxSigmaPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , maxSigma
                );
                (*m_sigmaRangePlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , sigmaRange
                );
                (*m_sigmaSigmaPlotter) & std::tuple<double, double>((double) ea->getIteration()
                        , p->getSigmaSigma());
            }
                break;

            case Gem::Geneva::infoMode::INFOEND: {
                // Write out the result
                m_gpd.writeToFile(m_fileName);
            }
                break;

            default: {
                throw gemfony_exception(
                        g_error_streamer(
                                DO_LOG
                                , time_and_place
                        )
                                << "In GOptOptMonitorT<ind_type>>: Received invalid infoMode " << im << std::endl
                );
            }
                break;
        };
    }

    /***************************************************************************/

    GOptOptMonitorT()
        : m_gpd(
            "empty"
            , 1
            , 1
        ) { /* empty */ }; ///< Default constructor; Intentionally private (only needed for serialization)

    std::string m_fileName; ///< The name of the output file

    Gem::Common::GPlotDesigner m_gpd; ///< Ease recording of essential information

    std::shared_ptr<Gem::Common::GGraph2D> m_progressPlotter; ///< Records progress information
    std::shared_ptr<Gem::Common::GGraph2D> m_nParentPlotter; ///< Records the number of parents in the individual
    std::shared_ptr<Gem::Common::GGraph2D> m_nChildrenPlotter; ///< Records the number of children in the individual
    std::shared_ptr<Gem::Common::GGraph2D> m_adProbPlotter; ///< Records the adaption probability for the individual
    std::shared_ptr<Gem::Common::GGraph2D> m_minSigmaPlotter; ///< Records the development of the lower sigma boundary
    std::shared_ptr<Gem::Common::GGraph2D> m_maxSigmaPlotter; ///< Records the development of the upper sigma boundary
    std::shared_ptr<Gem::Common::GGraph2D> m_sigmaRangePlotter; ///< Records the development of the sigma range
    std::shared_ptr<Gem::Common::GGraph2D> m_sigmaSigmaPlotter; ///< Records the development of the adaption strength
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMetaOptimizerIndividualT<Gem::Geneva::GFunctionIndividual>);

