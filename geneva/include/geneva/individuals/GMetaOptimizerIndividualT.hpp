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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/par/GConstrainedDoubleCollection.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GDoubleBiGaussAdaptor.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GInt32FlipAdaptor.hpp"
#include "geneva/par/GInt32GaussAdaptor.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/GPluggableOptimizationMonitors.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"

namespace Gem::Geneva::Individuals {

/******************************************************************************/
// Different types of optimization targets
enum class metaOptimizationTarget : Gem::Common::ENUMBASETYPE {
    BESTFITNESS = 0,
    MINSOLVERCALLS = 1,
    MC_MINSOLVER_BESTFITNESS =
        2 // Multi-criterion optimization with least number of solver calls and best average fitness as targets
};

/******************************************************************************/
// Input and output of metaOptimizationTarget, so we can serialize this data

/** @brief Puts a Gem::Geneva::Individuals::metaOptimizationTarget into a stream. Needed also for boost::lexical_cast<> */
std::ostream &
operator<<(std::ostream &, const Gem::Geneva::Individuals::metaOptimizationTarget &);

/** @brief Reads a Gem::Geneva::Individuals::metaOptimizationTarget from a stream. Needed also for boost::lexical_cast<> */
std::istream &operator>>(std::istream &, Gem::Geneva::Individuals::metaOptimizationTarget &);

/******************************************************************************/
// A number of default settings for the factory and individual

// Pertaining to the population
constexpr std::size_t GMETAOPT_DEF_INITNPARENTS = 1; ///< The initial number of parents
constexpr std::size_t GMETAOPT_DEF_NPARENTS_LB =
    1; ///< The lower boundary for variations of the number of parents
constexpr std::size_t GMETAOPT_DEF_NPARENTS_UB =
    6; ///< The upper boundary for variations of the number of parents

constexpr std::size_t GMETAOPT_DEF_INITNCHILDREN = 100; ///< The initial number of children
constexpr std::size_t GMETAOPT_DEF_NCHILDREN_LB =
    5; ///< The lower boundary for the variation of the number of children
constexpr std::size_t GMETAOPT_DEF_NCHILDREN_UB =
    250; ///< The upper boundary for the variation of the number of children

constexpr double GMETAOPT_DEF_INITAMALGLKLHOOD =
    0.; ///< The initial likelihood for an individual being created from cross-over rather than "just" duplication
constexpr double GMETAOPT_DEF_AMALGLKLHOOD_LB =
    0.; ///< The lower boundary for the variation of the amalgamation likelihood
constexpr double GMETAOPT_DEF_AMALGLKLHOOD_UB =
    1.; ///< The upper boundary for the variation of the amalgamation likelihood

// Concerning the individual
constexpr double GMETAOPT_DEF_INITMINADPROB =
    0.; ///< The initial lower boundary for the variation of ad_prob
constexpr double GMETAOPT_DEF_MINADPROB_LB = 0.; ///< The lower boundary for min_ad_prob
constexpr double GMETAOPT_DEF_MINADPROB_UB =
    0.1; ///< The upper boundary for min_ad_prob -- 0.1, effectively

constexpr double GMETAOPT_DEF_INITADPROBRANGE =
    0.9; ///< The initial upper boundary for the variation of ad_prob
constexpr double GMETAOPT_DEF_ADPROBRANGE_LB = 0.1; ///< The lower boundary for ad_prob_range
constexpr double GMETAOPT_DEF_ADPROBRANGE_UB = 0.9; ///< The upper boundary for ad_prob_range

constexpr double GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE =
    1.; ///< Defines the place inside of the allowed value range where ad_prob starts. Boundaries are 0./1.

constexpr double GMETAOPT_DEF_INITADAPTADPROB =
    0.1; ///< The initial value of the strength of adProb_ adaption
constexpr double GMETAOPT_DEF_ADAPTADPROB_LB =
    0.; ///< The lower boundary for the variation of the strength of adProb_ adaption
constexpr double GMETAOPT_DEF_ADAPTADPROB_UB =
    1.; ///< The upper boundary for the variation of the strength of adProb_ adaption

constexpr double GMETAOPT_DEF_INITMINSIGMA = 0.001; ///< The initial lower boundary for sigma
constexpr double GMETAOPT_DEF_MINSIGMA_LB =
    0.001; ///< The lower boundary for the variation of the lower boundary of sigma
constexpr double GMETAOPT_DEF_MINSIGMA_UB =
    0.09999; ///< The upper boundary for the variation of the lower boundary of sigma, means ~0.1

constexpr double GMETAOPT_DEF_INITSIGMARANGE =
    0.2; ///< The initial maximum range for sigma --> note that the initial start value for sigma will always be set to the upper boundary of its variation limits
constexpr double GMETAOPT_DEF_SIGMARANGE_LB =
    0.1; ///< The lower boundary for the variation of the maximum range of sigma --> max_sigma is 0.2
constexpr double GMETAOPT_DEF_SIGMARANGE_UB =
    0.9; ///< The upper boundary for the variation of the maximum range of sigma --> max_sigma is 1.

constexpr double GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE =
    1.; ///< The initial percentage of the sigma range as a start value

constexpr double GMETAOPT_DEF_INITSIGMASIGMA = 0.1; ///< The initial strength of sigma adaption
constexpr double GMETAOPT_DEF_SIGMASIGMA_LB =
    0.; ///< The lower boundary for the variation of the strength of sigma adaption
constexpr double GMETAOPT_DEF_SIGMASIGMA_UB =
    1.; ///< The upper boundary for the variation of the strength of sigma adaption

// General meta-optimization parameters
constexpr std::size_t GMETAOPT_DEF_NRUNSPEROPT = 10; ///< The number of successive optimization runs
constexpr double GMETAOPT_DEF_FITNESSTARGET = 0.001; ///< The fitness target
constexpr std::uint32_t GMETAOPT_DEF_ITERATIONTHRESHOLD =
    10000; ///< The maximum allowed number of iterations
const metaOptimizationTarget GMETAOPT_DEF_MOTARGET =
    metaOptimizationTarget::BESTFITNESS; ///< The target used for the meta optimization

const std::string GMETAOPT_DEF_INDCONFIG =
    "./config/GFunctionIndividual.json"; ///< The default configuration file for our individuals -- we follow the default template argument
const std::string GMETAOPT_DEF_SUBEACONFIG =
    "./config/GSubEvolutionaryAlgorithm.json"; ///< The default configuration file for the (sub-)evolutionary algorithms

constexpr bool GMETAOPT_SUBEXEC_SERIAL = false;
constexpr bool GMETAOPT_SUBEXEC_MULTITHREADED = true;
const bool GMETAOPT_DEF_SUBEXECMODE = GMETAOPT_SUBEXEC_MULTITHREADED;

// Make sure we do not mix parameter items
constexpr std::size_t MOT_NPARENTS = 0;
constexpr std::size_t MOT_NCHILDREN = 1;
constexpr std::size_t MOT_AMALGAMATION = 2;
constexpr std::size_t MOT_MINADPROB = 3;
constexpr std::size_t MOT_ADPROBRANGE = 4;
constexpr std::size_t MOT_ADPROBSTARTPERCENTAGE = 5;
constexpr std::size_t MOT_ADAPTADPROB = 6;
constexpr std::size_t MOT_MINSIGMA = 7;
constexpr std::size_t MOT_SIGMARANGE = 8;
constexpr std::size_t MOT_SIGMARANGEPERCENTAGE = 9;
constexpr std::size_t MOT_SIGMASIGMA = 10;
constexpr std::size_t MOT_NVAR = 11;

/******************************************************************************/
/**
 * This individual performs "meta optimization" in the sense that parameters of
 * an optimization algorithm are optimized alongside a given "sub-"individual.
 * The individual is meant for tuning the parameters of Evolutionary Algorithms,
 * but may be used to find better optima as well.
 */
template <typename ind_type = Gem::Geneva::Individuals::GFunctionIndividual>
class GMetaOptimizerIndividualT // NOLINT(cppcoreguidelines-special-member-functions)
  : public gpar::GParameterSet {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GParameterSet) &
            BOOST_SERIALIZATION_NVP(n_runs_per_optimization_) &
            BOOST_SERIALIZATION_NVP(fitness_target_) & BOOST_SERIALIZATION_NVP(iteration_threshold_) &
            BOOST_SERIALIZATION_NVP(mo_target_) & BOOST_SERIALIZATION_NVP(sub_ea_config_) &
            BOOST_SERIALIZATION_NVP(ind_factory_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The default constructor.
     */
    GMetaOptimizerIndividualT()
      : gpar::GParameterSet()
      , n_runs_per_optimization_(GMETAOPT_DEF_NRUNSPEROPT)
      , fitness_target_(GMETAOPT_DEF_FITNESSTARGET)
      , iteration_threshold_(GMETAOPT_DEF_ITERATIONTHRESHOLD)
      , mo_target_(GMETAOPT_DEF_MOTARGET)
      , sub_ea_config_(GMETAOPT_DEF_SUBEACONFIG)
      , ind_factory_() { /* nothing */
    }

    /***************************************************************************/
    /**
     * A standard copy constructor
     *
     * @param cp A copy of another GFunctionIndidivual
     */
    GMetaOptimizerIndividualT(const GMetaOptimizerIndividualT<ind_type> &cp)
      : gpar::GParameterSet(cp)
      , n_runs_per_optimization_(cp.n_runs_per_optimization_)
      , fitness_target_(cp.fitness_target_)
      , iteration_threshold_(cp.iteration_threshold_)
      , mo_target_(cp.mo_target_)
      , sub_ea_config_(cp.sub_ea_config_)
      , ind_factory_(
            // A default-constructed meta-optimizer has no factory yet; deep-clone
            // it only when present, otherwise copy the (null) factory as-is. The
            // previous unconditional (cp.ind_factory_)->clone() dereferenced a null
            // pointer when copying/cloning a default-constructed object.
            cp.ind_factory_
                ? Gem::Common::convertSmartPointer<
                      Gem::Common::GFactoryT<gpar::GParameterSet>,
                      typename ind_type::FACTORYTYPE>((cp.ind_factory_)->clone())
                : std::shared_ptr<typename ind_type::FACTORYTYPE>()
        ) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GMetaOptimizerIndividualT() override = default;

    /***************************************************************************/
    /**
     * Allows to specify the path and name of a configuration file passed to
     * the (sub-)evolutionary algorithm
     */
    void setSubEAConfig(std::string sub_ea_config) {
        sub_ea_config_ = sub_ea_config;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the path and name of a configuration file passed to
     * the (sub-)evolutionary algorithm
     */
    std::string getSubEAConfig() const {
        return sub_ea_config_;
    }

    /***************************************************************************/
    /**
     * Allows to specify how many optimizations should be performed for each (sub-)optimization
     */
    void setNRunsPerOptimization(std::size_t n_runs_per_optimization) {
#ifdef DEBUG
        if(0 == n_runs_per_optimization) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GMetaOptimizerIndividualT<ind_type>::setNRunsPerOptimization(): Error!"
                << '\n'
                << "Requested number of sub-optimizations is 0" << '\n'
            );
        }
#endif

        n_runs_per_optimization_ = n_runs_per_optimization;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the number of optimizations to be performed for each (sub-)optimization
     */
    std::size_t getNRunsPerOptimization() const {
        return n_runs_per_optimization_;
    }

    /***************************************************************************/
    /**
     * Allows to set the fitness target for each optimization
     */
    void setFitnessTarget(double fitness_target) {
        fitness_target_ = fitness_target;
    }

    /***************************************************************************/
    /**
     * Retrieves the fitness target for each optimization
     */
    double getFitnessTarget() const {
        return fitness_target_;
    }

    /***************************************************************************/
    /**
     * Allows to set the iteration threshold
     */
    void setIterationThreshold(std::uint32_t iteration_threshold) {
        iteration_threshold_ = iteration_threshold;
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the iteration threshold
     */
    std::uint32_t getIterationThreshold() const {
        return iteration_threshold_;
    }

    /***************************************************************************/
    /**
     * Allows to set the desired target of the meta-optimization
     */
    void setMetaOptimizationTarget(metaOptimizationTarget mo_target) {
        mo_target_ = mo_target;

        // multi-criterion optimization. We need to set the number of fitness criteria
        if(metaOptimizationTarget::MC_MINSOLVER_BESTFITNESS == mo_target_) {
            this->setNStoredResults(2);
        }
    }

    /***************************************************************************/
    /**
     * Allows to retrieve the current target of the meta-optimization
     */
    metaOptimizationTarget getMetaOptimizationTarget() const {
        return mo_target_;
    }

    /***************************************************************************/
    /**
     * Retrieves the current number of parents. Needed for the optimization monitor.
     */
    std::size_t getNParents() const {
        std::shared_ptr<gpar::GConstrainedInt32Object> npar_ptr =
            this->at<gpar::GConstrainedInt32Object>(MOT_NPARENTS);
        return Gem::Common::narrow<std::size_t>(npar_ptr->value());
    }

    /***************************************************************************/
    /**
     * Retrieves the current number of children. Needed for the optimization monitor.
     */
    std::size_t getNChildren() const {
        std::shared_ptr<gpar::GConstrainedInt32Object> nch_ptr =
            this->at<gpar::GConstrainedInt32Object>(MOT_NCHILDREN);
        return Gem::Common::narrow<std::size_t>(nch_ptr->value());
    }

    /***************************************************************************/
    /**
     * Retrieves the adaption probability. Needed for the optimization monitor.
     */
    double getAdProb() const {
        std::shared_ptr<gpar::GConstrainedDoubleObject> min_ad_prob_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_MINADPROB);
        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_range_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADPROBRANGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_start_percentage_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);

        return min_ad_prob_ptr->value() +
               ad_prob_start_percentage_ptr->value() * ad_prob_range_ptr->value();
    }

    /***************************************************************************/
    /**
     * Retrieves the lower sigma boundary. Needed for the optimization monitor.
     */
    double getMinSigma() const {
        std::shared_ptr<gpar::GConstrainedDoubleObject> minsigma_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_MINSIGMA);
        return minsigma_ptr->value();
    }

    /***************************************************************************/
    /**
     * Retrieves the sigma range. Needed for the optimization monitor.
     */
    double getSigmaRange() const {
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmarange_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMARANGE);
        return sigmarange_ptr->value();
    }

    /***************************************************************************/
    /**
     * Retrieves the sigma-sigma parameter. Needed for the optimization monitor.
     */
    double getSigmaSigma() const {
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmasigma_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMASIGMA);
        return sigmasigma_ptr->value();
    }

    /***************************************************************************/
    /**
     * This function is used to unify the setup from within the constructor
     * and factory.
     */
    static void addContent(
        std::shared_ptr<GMetaOptimizerIndividualT<ind_type>> p,
        const std::size_t &init_n_parents,
        const std::size_t &n_parents_lb,
        const std::size_t &n_parents_ub,
        const std::size_t &init_n_children,
        const std::size_t &n_children_lb,
        const std::size_t &n_children_ub,
        const double &init_amalgamation_lklh,
        const double &amalgamation_lklh_lb,
        const double &amalgamation_lklh_ub,
        const double &init_min_ad_prob,
        const double &min_ad_prob_lb,
        const double &min_ad_prob_ub,
        const double &init_ad_prob_range,
        const double &ad_prob_range_lb,
        const double &ad_prob_range_ub,
        const double &init_ad_prob_start_percentage,
        const double &init_adapt_ad_prob,
        const double &adapt_ad_prob_lb,
        const double &adapt_ad_prob_ub,
        const double &init_min_sigma,
        const double &min_sigma_lb,
        const double &min_sigma_ub,
        const double &init_sigma_range,
        const double &sigma_range_lb,
        const double &sigma_range_ub,
        const double &init_sigma_range_percentage,
        const double &init_sigma_sigma,
        const double &sigma_sigma_lb,
        const double &sigma_sigma_ub
    ) {
        // We will add parameter types in the same order as the arguments

        // Make sure, p has the correct size
        p->clear();
        p->resize_empty(MOT_NVAR); // Will add empty smart pointers to the collection

        //------------------------------------------------------------
        // n_parents

        // Small number of possible values -- use a flip-adaptor
        std::shared_ptr<gpar::GInt32FlipAdaptor> gifa_ptr(new gpar::GInt32FlipAdaptor());
        gifa_ptr->setAdaptionProbability(1.);

        std::shared_ptr<gpar::GConstrainedInt32Object> npar_ptr(new gpar::GConstrainedInt32Object(
            Gem::Common::narrow<std::int32_t>(init_n_parents),
            Gem::Common::narrow<std::int32_t>(n_parents_lb),
            Gem::Common::narrow<std::int32_t>(n_parents_ub)
        ));
        npar_ptr->addAdaptor(gifa_ptr);
        npar_ptr->setParameterName("n_parents");

        // Add to the individual
        p->at(MOT_NPARENTS) = npar_ptr;

        assert(p->at(MOT_NPARENTS));

        //------------------------------------------------------------
        // n_children

        // Create a default standard gauss adaptor
        std::shared_ptr<gpar::GInt32GaussAdaptor> giga_ptr(new gpar::GInt32GaussAdaptor(
            0.025 // sigma
            ,
            0.2 // sigma_sigma
            ,
            0.001 // min_sigma
            ,
            0.5 // max_sigma
            ,
            1. // ad_prob
        ));

        std::shared_ptr<gpar::GConstrainedInt32Object> nch_ptr(new gpar::GConstrainedInt32Object(
            Gem::Common::narrow<std::int32_t>(init_n_children),
            Gem::Common::narrow<std::int32_t>(n_children_lb),
            Gem::Common::narrow<std::int32_t>(n_children_ub)
        ));
        nch_ptr->addAdaptor(giga_ptr);
        nch_ptr->setParameterName("n_children");

        // Add to the individual
        p->at(MOT_NCHILDREN) = nch_ptr;

        //------------------------------------------------------------
        // amalgamationLklh

        // Create a default standard gauss adaptor
        std::shared_ptr<gpar::GDoubleGaussAdaptor> gdga_ptr(new gpar::GDoubleGaussAdaptor(
            0.025 // sigma
            ,
            0.2 // sigma_sigma
            ,
            0.001 // min_sigma
            ,
            0.5 // max_sigma
            ,
            1. // ad_prob
        ));
        std::shared_ptr<gpar::GConstrainedDoubleObject> amalgamation_lklh_ptr(
            new gpar::GConstrainedDoubleObject(
                init_amalgamation_lklh // initial value
                ,
                amalgamation_lklh_lb // lower boundary
                ,
                amalgamation_lklh_ub // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        amalgamation_lklh_ptr->addAdaptor(gdga_ptr);
        amalgamation_lklh_ptr->setParameterName("amalgamation_likelihood");

        // Add to the individual
        p->at(MOT_AMALGAMATION) = amalgamation_lklh_ptr;

        //------------------------------------------------------------
        // min_ad_prob

        std::shared_ptr<gpar::GConstrainedDoubleObject> min_ad_prob_ptr(new gpar::GConstrainedDoubleObject(
            init_min_ad_prob // initial value
            ,
            min_ad_prob_lb // lower boundary
            ,
            min_ad_prob_ub // upper boundary
        ));
        // Add the gauss adaptor to the parameter
        min_ad_prob_ptr->addAdaptor(gdga_ptr);
        min_ad_prob_ptr->setParameterName("min_ad_prob");

        // Add to the individual
        p->at(MOT_MINADPROB) = min_ad_prob_ptr;

        //------------------------------------------------------------
        // ad_prob_range

        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_range_ptr(new gpar::GConstrainedDoubleObject(
            init_ad_prob_range // initial value
            ,
            ad_prob_range_lb // lower boundary
            ,
            ad_prob_range_ub // upper boundary
        ));
        // Add the gauss adaptor to the parameter
        ad_prob_range_ptr->addAdaptor(gdga_ptr);
        ad_prob_range_ptr->setParameterName("ad_prob_range");

        // Add to the individual
        p->at(MOT_ADPROBRANGE) = ad_prob_range_ptr;

        //------------------------------------------------------------
        // ad_prob_start_percentage

        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_start_percentage_ptr(
            new gpar::GConstrainedDoubleObject(
                init_ad_prob_start_percentage // initial value
                ,
                0. // lower boundary
                ,
                1. // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        ad_prob_start_percentage_ptr->addAdaptor(gdga_ptr);
        ad_prob_start_percentage_ptr->setParameterName("ad_prob_start_percentage");

        // Add to the individual
        p->at(MOT_ADPROBSTARTPERCENTAGE) = ad_prob_start_percentage_ptr;

        //------------------------------------------------------------
        // adapt_ad_prob

        std::shared_ptr<gpar::GConstrainedDoubleObject> adapt_ad_prob_ptr(new gpar::GConstrainedDoubleObject(
            init_adapt_ad_prob // initial value
            ,
            adapt_ad_prob_lb // lower boundary
            ,
            adapt_ad_prob_ub // upper boundary
        ));
        adapt_ad_prob_ptr->addAdaptor(gdga_ptr);
        adapt_ad_prob_ptr->setParameterName("adapt_ad_prob");

        // Add to the individual
        p->at(MOT_ADAPTADPROB) = adapt_ad_prob_ptr;

        //------------------------------------------------------------
        // min_sigma

        std::shared_ptr<gpar::GConstrainedDoubleObject> minsigma_ptr(new gpar::GConstrainedDoubleObject(
            init_min_sigma // initial value
            ,
            min_sigma_lb // lower boundary
            ,
            min_sigma_ub // upper boundary
        ));
        minsigma_ptr->addAdaptor(gdga_ptr);
        minsigma_ptr->setParameterName("min_sigma");

        // Add to the individual
        p->at(MOT_MINSIGMA) = minsigma_ptr;

        //------------------------------------------------------------
        // sigma_range

        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmarange_ptr(new gpar::GConstrainedDoubleObject(
            init_sigma_range // initial value
            ,
            sigma_range_lb // lower boundary
            ,
            sigma_range_ub // upper boundary
        ));
        sigmarange_ptr->addAdaptor(gdga_ptr);
        sigmarange_ptr->setParameterName("sigma_range");

        // Add to the individual
        p->at(MOT_SIGMARANGE) = sigmarange_ptr;

        //------------------------------------------------------------
        // sigma_range_percentage

        std::shared_ptr<gpar::GConstrainedDoubleObject> sigma_range_percentage_ptr(
            new gpar::GConstrainedDoubleObject(
                init_sigma_range_percentage // initial value
                ,
                0. // lower boundary
                ,
                1. // upper boundary
            )
        );
        // Add the gauss adaptor to the parameter
        sigma_range_percentage_ptr->addAdaptor(gdga_ptr);
        sigma_range_percentage_ptr->setParameterName("sigma_range_percentage");

        // Add to the individual
        p->at(MOT_SIGMARANGEPERCENTAGE) = sigma_range_percentage_ptr;

        //------------------------------------------------------------
        // sigma_sigma

        // The sigma adaption strength may change between 0.01 and 1
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmasigma_ptr(new gpar::GConstrainedDoubleObject(
            init_sigma_sigma // initial value
            ,
            sigma_sigma_lb // lower boundary
            ,
            sigma_sigma_ub // upper boundary
        ));
        sigmasigma_ptr->addAdaptor(gdga_ptr);
        sigmasigma_ptr->setParameterName("sigma_sigma");

        // Add to the individual
        p->at(MOT_SIGMASIGMA) = sigmasigma_ptr;

        //------------------------------------------------------------
    }

    /***************************************************************************/
    /**
     * Emit information about this individual
     */
    std::string print(bool with_fitness = true) const {
        std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)
        // Retrieve the parameters
        std::shared_ptr<gpar::GConstrainedInt32Object> npar_ptr =
            this->at<gpar::GConstrainedInt32Object>(MOT_NPARENTS);
        std::shared_ptr<gpar::GConstrainedInt32Object> nch_ptr =
            this->at<gpar::GConstrainedInt32Object>(MOT_NCHILDREN);
        std::shared_ptr<gpar::GConstrainedDoubleObject> amalgamation_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_AMALGAMATION);
        std::shared_ptr<gpar::GConstrainedDoubleObject> min_ad_prob_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_MINADPROB);
        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_range_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADPROBRANGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_start_percentage_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> adapt_adprob_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADAPTADPROB);
        std::shared_ptr<gpar::GConstrainedDoubleObject> minsigma_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_MINSIGMA);
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmarange_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMARANGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigma_range_percentage_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMARANGEPERCENTAGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmasigma_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMASIGMA);

        // Stream the results

        bool unprocessed = (not this->is_processed() || this->has_errors());
        double transformed_primary_fitness =
            unprocessed ? this->getWorstCase() : this->transformed_fitness(0);

        result << "================================================================================"
                  "============"
               << '\n';

        if(with_fitness) {
            result << "Fitness = " << transformed_primary_fitness
                   << (unprocessed ? " // unprocessed or error" : "") << '\n';
        }

        result << "Optimization target: " << getClearTextMOT(mo_target_) << '\n'
               << '\n'
               << "population::population size = " << npar_ptr->value() + nch_ptr->value() << '\n'
               << "population::n_parents = " << npar_ptr->value() << '\n'
               << "population::n_children = " << nch_ptr->value() << '\n'
               << "population::amalgamation_likelihood = " << amalgamation_ptr->value() << '\n'
               << "individual::ad_prob_range = " << ad_prob_range_ptr->value() << '\n'
               << "individual::ad_prob_start_percentage_ptr = "
               << ad_prob_start_percentage_ptr->value() << '\n'
               << "individual::ad_prob = "
               << min_ad_prob_ptr->value() +
                      ad_prob_range_ptr->value() * ad_prob_start_percentage_ptr->value()
               << '\n'
               << "individual::min_ad_prob = " << min_ad_prob_ptr->value() << '\n'
               << "individual::max_ad_prob = "
               << min_ad_prob_ptr->value() + ad_prob_range_ptr->value() << '\n'
               << "individual::adapt_ad_prob = " << adapt_adprob_ptr->value() << '\n'
               << "individual::sigmarange_ptr = " << sigmarange_ptr->value() << '\n'
               << "individual::sigma_range_percentage_ptr = " << sigma_range_percentage_ptr->value()
               << '\n'
               << "individual::sigma1 = "
               << minsigma_ptr->value() +
                      sigmarange_ptr->value() * sigma_range_percentage_ptr->value()
               << '\n'
               << "individual::min_sigma1 = " << minsigma_ptr->value() << '\n'
               << "individual::max_sigma1 = " << minsigma_ptr->value() + sigmarange_ptr->value()
               << '\n'
               << "individual::sigma_sigma1 = " << sigmasigma_ptr->value() << '\n'
               << "================================================================================"
                  "============"
               << '\n'
               << '\n';

        return result.str();
    }

    /***************************************************************************/
    /**
     * Registers a factory class with this object. This function clones the factory,
     * so the individual can be sure to have a unique factory.
     */
    void registerIndividualFactory(std::shared_ptr<typename ind_type::FACTORYTYPE> factory) {
        if(not factory) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GMetaOptimizerIndividualT<T>::registerIndividualFactory(): Error!"
                << '\n'
                << "Individual is empty" << '\n'
            );
        }

        ind_factory_ = Gem::Common::convertSmartPointer<
            Gem::Common::GFactoryT<gpar::GParameterSet>,
            typename ind_type::FACTORYTYPE>(factory->clone());
    }

protected:
    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Call our parent class'es function
        gpar::GParameterSet::addConfigurationOptions_(gpb);

        // Add local data
        gpb.registerFileParameter<std::size_t>(
            "n_runs_per_optimization" // The name of the variable
            ,
            GMETAOPT_DEF_NRUNSPEROPT // The default value
            ,
            [this](std::size_t nrpo) { this->setNRunsPerOptimization(nrpo); }
        ) << "Specifies the number of optimizations performed";

        gpb.registerFileParameter<double>(
            "fitness_target" // The name of the variable
            ,
            GMETAOPT_DEF_FITNESSTARGET // The default value
            ,
            [this](double ft) { this->setFitnessTarget(ft); }
        ) << "The fitness below which optimization should stop";

        gpb.registerFileParameter<std::uint32_t>(
            "iteration_threshold" // The name of the variable
            ,
            GMETAOPT_DEF_ITERATIONTHRESHOLD // The default value
            ,
            [this](std::uint32_t dit) { this->setIterationThreshold(dit); }
        ) << "The maximum number of iterations per sub-optimization";

        gpb.registerFileParameter<metaOptimizationTarget>(
            "meta_optimization_target" // The name of the variable
            ,
            GMETAOPT_DEF_MOTARGET // The default value
            ,
            [this](metaOptimizationTarget mot) { this->setMetaOptimizationTarget(mot); }
        ) << "The target for the meta-optimization: best fitness (0),"
          << '\n'
          << "minimum number of solver calls (1), multi-criterion with best fitness" << '\n'
          << "and smallest number of solver calls as target (2);";

        gpb.registerFileParameter<std::string>(
            "sub_ea_config" // The name of the variable
            ,
            GMETAOPT_DEF_SUBEACONFIG // The default value
            ,
            [this](std::string seac) { this->setSubEAConfig(seac); }
        ) << "Path and name of the configuration file used for the (sub-)evolutionary algorithm";
    }

    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     * Note: ind_factory_ is deliberately NOT listed here -- load_() keeps the
     * object's own factory (see the comment there) and compare_() ignores it, so
     * it is not part of the copy/compare semantics. serialize() handles it
     * separately; do not derive serialize() from localMembers() for this class.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("n_runs_per_optimization_", n_runs_per_optimization_),
            Gem::Common::make_member("fitness_target_", fitness_target_),
            Gem::Common::make_member("iteration_threshold_", iteration_threshold_),
            Gem::Common::make_member("mo_target_", mo_target_),
            Gem::Common::make_member("sub_ea_config_", sub_ea_config_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("n_runs_per_optimization_", n_runs_per_optimization_),
            Gem::Common::make_member("fitness_target_", fitness_target_),
            Gem::Common::make_member("iteration_threshold_", iteration_threshold_),
            Gem::Common::make_member("mo_target_", mo_target_),
            Gem::Common::make_member("sub_ea_config_", sub_ea_config_)
        );
    }

    /***************************************************************************/
    /**
     * Loads the data of another GMetaOptimizerIndividualT<ind_type>
     *
     * @param cp A copy of another GMetaOptimizerIndividualT<ind_type>
     */
    void load_(const gpar::GParameterSet *cp) override {
        // Check that we are dealing with a GMetaOptimizerIndividualT<ind_type> reference independent of this object and convert the pointer
        const GMetaOptimizerIndividualT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<gpar::GParameterSet, GMetaOptimizerIndividualT<ind_type>>(
                cp,
                this
            );

        // Load our parent class'es data ...
        gpar::GParameterSet::load_(cp);

        // ... and then our local data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());

        // We simply keep our local individual factory, as all settings are made inside of fitnessCalculation
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GMetaOptimizerIndividualT<ind_type>>(
        GMetaOptimizerIndividualT<ind_type> const &,
        GMetaOptimizerIndividualT<ind_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GParameterSet object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const gpar::GParameterSet &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const final {
        // Check that we are dealing with a GMetaOptimizerIndividualT<ind_type> reference independent of this object and convert the pointer
        const GMetaOptimizerIndividualT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<gpar::GParameterSet, GMetaOptimizerIndividualT<ind_type>>(
                cp,
                this
            );

        Gem::Common::GToken token("GMetaOptimizerIndividualT<ind_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<gpar::GParameterSet>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

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
        std::shared_ptr<gpar::GConstrainedInt32Object> npar_ptr =
            this->at<gpar::GConstrainedInt32Object>(MOT_NPARENTS);
        std::shared_ptr<gpar::GConstrainedInt32Object> nch_ptr =
            this->at<gpar::GConstrainedInt32Object>(MOT_NCHILDREN);
        std::shared_ptr<gpar::GConstrainedDoubleObject> amalgamation_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_AMALGAMATION);
        std::shared_ptr<gpar::GConstrainedDoubleObject> min_ad_prob_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_MINADPROB);
        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_range_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADPROBRANGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> ad_prob_start_percentage_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> adapt_adprob_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_ADAPTADPROB);
        std::shared_ptr<gpar::GConstrainedDoubleObject> minsigma_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_MINSIGMA);
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmarange_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMARANGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigma_range_percentage_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMARANGEPERCENTAGE);
        std::shared_ptr<gpar::GConstrainedDoubleObject> sigmasigma_ptr =
            this->at<gpar::GConstrainedDoubleObject>(MOT_SIGMASIGMA);

#ifdef DEBUG
        // Check that we have been given a factory
        if(not ind_factory_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GMetaOptimizerIndividualT<T>::fitnessCalculation(): Error!" << '\n'
                << "No factory class for individuals has been registered" << '\n'
            );
        }
#endif

        // Set the parameters
        double min_sigma = minsigma_ptr->value();
        double sigma_range = sigmarange_ptr->value();
        double max_sigma = min_sigma + sigma_range;
        double sigma_range_percentage = sigma_range_percentage_ptr->value();
        double start_sigma = min_sigma + sigma_range_percentage * sigma_range;

        ind_factory_->setSigma1Range(std::tuple<double, double>(min_sigma, max_sigma));
        ind_factory_->setSigma1(start_sigma);
        ind_factory_->setSigmaSigma1(sigmasigma_ptr->value());

        double min_ad_prob = min_ad_prob_ptr->value();
        double ad_prob_range = ad_prob_range_ptr->value();
        double max_ad_prob = min_ad_prob + ad_prob_range;
        double ad_prob_start_percentage = ad_prob_start_percentage_ptr->value();
        double start_ad_prob = min_ad_prob + ad_prob_start_percentage * ad_prob_range;

        double adapt_ad_prob = adapt_adprob_ptr->value();

        ind_factory_->setAdProbRange(min_ad_prob, max_ad_prob);
        ind_factory_->setAdProb(start_ad_prob);
        ind_factory_->setAdaptAdProb(adapt_ad_prob);

        // Set up a population factory for serial execution
        oa::GEvolutionaryAlgorithmFactory ea(sub_ea_config_);

        // Run the required number of optimizations
        std::shared_ptr<oa::GEvolutionaryAlgorithm> ea_ptr;

        std::uint32_t n_children = Gem::Common::narrow<std::uint32_t>(nch_ptr->value());
        std::uint32_t n_parents = Gem::Common::narrow<std::uint32_t>(npar_ptr->value());
        std::uint32_t pop_size = n_parents + n_children;
        std::uint32_t iterations_consumed = 0;
        double amalgamation_likelihood = amalgamation_ptr->value();

        std::vector<double> solver_calls_per_optimization;
        std::vector<double> iterations_per_optimization;
        std::vector<double> best_evaluations;

        for(std::size_t opt = 0; opt < n_runs_per_optimization_; opt++) {
            std::cout << "Starting measurement " << opt + 1 << " / " << n_runs_per_optimization_
                      << '\n';
            ea_ptr = ea.get<oa::GEvolutionaryAlgorithm>();

            // Register an executor
            ea_ptr->registerExecutor(
                execMode::SERIAL,
                "./config/GMetaOptimizerSerialExecutor.json"
            );

            // Set the population parameters
            ea_ptr->setPopulationSizes(pop_size, n_parents);

            // Add the required number of individuals
            for(std::size_t ind = 0; ind < pop_size; ind++) {
                // Retrieve an individual
                std::shared_ptr<gpar::GParameterSet> gi_ptr = ind_factory_->get();

                ea_ptr->push_back(gi_ptr);
            }

            // Set the likelihood for work items to be produced through cross-over rather than mutation alone
            ea_ptr->setAmalgamationLikelihood(amalgamation_likelihood);

            if(metaOptimizationTarget::MINSOLVERCALLS == mo_target_) {
                // Set the stop criteria (either maxIterations_ iterations or falling below the quality threshold
                ea_ptr->setQualityThreshold(fitness_target_, true);
                ea_ptr->setMaxIteration(iteration_threshold_);

                // Make sure the optimization does not emit the termination reason
                ea_ptr->setEmitTerminationReason(false);

                // Make sure the optimization does not stop due to stalls (which is the default in the EA-config
                ea_ptr->setMaxStallIteration(0);
            }
            else { // Optimization of best fitness found or multi-criterion optimization: BESTFITNESS / MC_MINSOLVER_BESTFITNESS
                // Set the stop criterion maxIterations only
                ea_ptr->setMaxIteration(iteration_threshold_);

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
            std::shared_ptr<gpar::GParameterSet> best_individual =
                ea_ptr->getBestGlobalIndividual<gpar::GParameterSet>();

            // Retrieve the number of iterations
            iterations_consumed = ea_ptr->getIteration();

            // Do book-keeping
            solver_calls_per_optimization.push_back(
                static_cast<double>((iterations_consumed + 1) * n_children + n_parents)
            );
            iterations_per_optimization.push_back(static_cast<double>(iterations_consumed + 1));
            best_evaluations.push_back(
                best_individual->transformed_fitness(0)
            ); // We use the transformed fitness to avoid MAX_DOUBLE
        }

        // Calculate the average number of iterations and solver calls
        std::tuple<double, double> sd =
            Gem::Common::GStandardDeviation(solver_calls_per_optimization);
        std::tuple<double, double> itmean =
            Gem::Common::GStandardDeviation(iterations_per_optimization);
        std::tuple<double, double> best_mean = Gem::Common::GStandardDeviation(best_evaluations);

        double evaluation = 0.;
        if(metaOptimizationTarget::MINSOLVERCALLS == mo_target_) {
            evaluation = std::get<0>(sd);
        }
        else if(metaOptimizationTarget::BESTFITNESS == mo_target_) {
            evaluation = std::get<0>(best_mean);
        }
        else if(metaOptimizationTarget::MC_MINSOLVER_BESTFITNESS == mo_target_) {
            evaluation = std::get<0>(best_mean);
            this->setResult(1, std::get<0>(sd)); // The secondary result
        }

        // Emit some information
        std::cout << '\n'
                  << std::get<0>(sd) << " +/- " << std::get<1>(sd) << " solver calls with " << '\n'
                  << std::get<0>(itmean) << " +/- " << std::get<1>(itmean) << " average iterations "
                  << '\n'
                  << "and a best evaluation of " << std::get<0>(best_mean) << " +/- "
                  << std::get<1>(best_mean) << '\n'
                  << "out of " << n_runs_per_optimization_ << " consecutive runs" << '\n'
                  << "fitnessCalculation() will return the value " << evaluation << '\n'
                  << this->print(false)
                  << '\n' // print without fitness -- not defined at this stage
                  << '\n';

        // Let the audience know
        return evaluation;
    }

    /***************************************************************************/
    /**
     * Retrieves a clear-text description of the optimization target
     */
    std::string getClearTextMOT(const metaOptimizationTarget &mot) const {
        switch(mot) {
        case metaOptimizationTarget::BESTFITNESS:
            return std::string("\"best fitness\"");

        case metaOptimizationTarget::MINSOLVERCALLS:
            return std::string("\"minimum number of solver calls\"");

        case metaOptimizationTarget::MC_MINSOLVER_BESTFITNESS:
            return std::string(
                "\"multi-criterion target with best fitness, minimum number of solver calls\""
            );
        }

        // Make the compiler happy
        return {};
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object.
     *
     * @return A boolean indicating whether
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        bool result = false;

        // Call the parent classes' functions
        if(gpar::GParameterSet::modify_GUnitTests_()) {
            result = true;
        }

        // Change the parameter settings
        if(not this->empty()) {
            this->adapt();
            result = true;
        }

        // Let the audience know whether we have changed the content
        return result;

#else  /* GEM_TESTING */
        Gem::Common::condnotset(
            "GMetaOptimizerIndividualT<ind_type>::modify_GUnitTests()",
            "GEM_TESTING"
        );
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

        // Call the parent classes' functions
        gpar::GParameterSet::specificTestsNoFailureExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        {
            /* nothing. Add test cases here that are expected to succeed. */
        }

        //------------------------------------------------------------------------------
#else  /* GEM_TESTING */
        Gem::Common::condnotset(
            "GMetaOptimizerIndividualT<ind_type>::specificTestsNoFailureExpected_GUnitTests()",
            "GEM_TESTING"
        );
#endif /* GEM_TESTING */
    }
    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail.
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        using namespace Gem::Geneva;

        // Call the parent classes' functions
        gpar::GParameterSet::specificTestsFailuresExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        {
            /* Nothing. Add test cases here that are expected to fail.
                Enclose with a BOOST_CHECK_THROW, using the expected
                exception type as an additional argument. See the
                documentation for the Boost.Test library for further
                information */
        }

        //------------------------------------------------------------------------------

#else  /* GEM_TESTING */
        Gem::Common::condnotset(
            "GMetaOptimizerIndividualT<ind_type>::specificTestsNoFailureExpected_GUnitTests()",
            "GEM_TESTING"
        );
#endif /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
     * Creates a deep clone of this object
     *
     * @return A deep clone of this object, camouflaged as a GParameterSet
     */
    gpar::GParameterSet *clone_() const final {
        return new GMetaOptimizerIndividualT<ind_type>(*this);
    }

    /***************************************************************************/

    std::size_t n_runs_per_optimization_; ///< The number of runs performed for each (sub-)optimization
    double fitness_target_;             ///< The quality target to be reached by
    std::uint32_t iteration_threshold_; ///< The maximum allowed number of iterations
    metaOptimizationTarget mo_target_;  ///< The target used for the meta-optimization
    std::string
        individual_config_; ///< Path and name of the configuration file needed for the individual
    std::string
        sub_ea_config_; ///< Path and name of the configuration file needed for (sub-)evolutionary algorithms

    std::shared_ptr<typename ind_type::FACTORYTYPE>
        ind_factory_; ///< Holds a factory for our individuals
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to output a GMetaOptimizerIndividualT<ind_type> or convert it to a string using
 * boost::lexical_cast
 */
template <typename ind_type>
std::ostream &operator<<(std::ostream &stream, const GMetaOptimizerIndividualT<ind_type> &gsi) {
    stream << gsi.print();
    return stream;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GMetaOptimizerIndividualT<ind_type> objects
 */
template <typename ind_type>
class GMetaOptimizerIndividualFactoryT : public Gem::Common::GFactoryT<gpar::GParameterSet> {
public:
    /***************************************************************************/
    /**
     * A constructor with the ability to switch the parallelization mode. It initializes a
     * target item as needed.
     *
     * @param config_file The name of the configuration file
     */
    GMetaOptimizerIndividualFactoryT(std::filesystem::path const &config_file)
      : Gem::Common::GFactoryT<gpar::GParameterSet>(config_file) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GMetaOptimizerIndividualFactoryT() override = default;

    /***************************************************************************/
    /**
     * Registers a factory class with this object. This function clones the factory,
     * so the individual can be sure to have a unique factory.
     */
    void registerIndividualFactory(std::shared_ptr<typename ind_type::FACTORYTYPE> factory) {
        if(not factory) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GMetaOptimizerIndividualFactoryT<T>::registerIndividualFactory(): Error!"
                << '\n'
                << "Individual is empty" << '\n'
            );
        }

        ind_factory_ = Gem::Common::convertSmartPointer<
            Gem::Common::GFactoryT<gpar::GParameterSet>,
            typename ind_type::FACTORYTYPE>(factory->clone());
    }

protected:
    /***************************************************************************/
    /**
     * Allows to describe local configuration options for gradient descents
     */
    void describeLocalOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Describe our own options
        using namespace Gem::Courtier;

        std::string comment; // NOLINT(cppcoreguidelines-init-variables)
        comment = "";
        comment += "The initial number of parents in a population;";
        gpb.registerFileParameter<std::size_t>(
            "init_n_parents",
            init_n_parents_,
            GMETAOPT_DEF_INITNPARENTS,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for variations of the number of parents;";
        gpb.registerFileParameter<std::size_t>(
            "n_parents_lb",
            n_parents_lb_,
            GMETAOPT_DEF_NPARENTS_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for variations of the number of parents;";
        gpb.registerFileParameter<std::size_t>(
            "n_parents_ub",
            n_parents_ub_,
            GMETAOPT_DEF_NPARENTS_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial number of children in a population;";
        gpb.registerFileParameter<std::size_t>(
            "init_n_children",
            init_n_children_,
            GMETAOPT_DEF_INITNCHILDREN,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the number of children;";
        gpb.registerFileParameter<std::size_t>(
            "n_children_lb",
            n_children_lb_,
            GMETAOPT_DEF_NCHILDREN_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the number of children;";
        gpb.registerFileParameter<std::size_t>(
            "n_children_ub",
            n_children_ub_,
            GMETAOPT_DEF_NCHILDREN_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial likelihood for an individual being created from cross-over rather "
                   "than just duplication;";
        gpb.registerFileParameter<double>(
            "init_amalgamation_lklh",
            init_amalgamation_lklh_,
            GMETAOPT_DEF_INITAMALGLKLHOOD,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the amalgamation likelihood ;";
        gpb.registerFileParameter<double>(
            "amalgamation_lklh_lb",
            amalgamation_lklh_lb_,
            GMETAOPT_DEF_AMALGLKLHOOD_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the amalgamation likelihood ;";
        gpb.registerFileParameter<double>(
            "amalgamation_lklh_ub",
            amalgamation_lklh_ub_,
            GMETAOPT_DEF_AMALGLKLHOOD_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial lower boundary for the variation of ad_prob;";
        gpb.registerFileParameter<double>(
            "init_min_ad_prob",
            init_min_ad_prob_,
            GMETAOPT_DEF_INITMINADPROB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for min_ad_prob;";
        gpb.registerFileParameter<double>(
            "min_ad_prob_lb",
            min_ad_prob_lb_,
            GMETAOPT_DEF_MINADPROB_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for min_ad_prob;";
        gpb.registerFileParameter<double>(
            "min_ad_prob_ub",
            min_ad_prob_ub_,
            GMETAOPT_DEF_MINADPROB_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial range for the variation of ad_prob;";
        gpb.registerFileParameter<double>(
            "init_ad_prob_range",
            init_ad_prob_range_,
            GMETAOPT_DEF_INITADPROBRANGE,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for ad_prob_range;";
        gpb.registerFileParameter<double>(
            "ad_prob_range_lb",
            ad_prob_range_lb_,
            GMETAOPT_DEF_ADPROBRANGE_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for ad_prob_range;";
        gpb.registerFileParameter<double>(
            "ad_prob_range_ub",
            ad_prob_range_ub_,
            GMETAOPT_DEF_ADPROBRANGE_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The start value for ad_prob relative to the allowed value range;";
        gpb.registerFileParameter<double>(
            "init_ad_prob_start_percentage",
            init_ad_prob_start_percentage_,
            GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial value of the strength of adProb_ adaption;";
        gpb.registerFileParameter<double>(
            "init_adapt_ad_prob",
            init_adapt_ad_prob_,
            GMETAOPT_DEF_INITADAPTADPROB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the strength of adProb_ adaption;";
        gpb.registerFileParameter<double>(
            "adapt_ad_prob_lb",
            adapt_ad_prob_lb_,
            GMETAOPT_DEF_ADAPTADPROB_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the strength of adProb_ adaption;";
        gpb.registerFileParameter<double>(
            "adapt_ad_prob_ub",
            adapt_ad_prob_ub_,
            GMETAOPT_DEF_ADAPTADPROB_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial minimum sigma for gauss-adaption in ES;";
        gpb.registerFileParameter<double>(
            "init_min_sigma",
            init_min_sigma_,
            GMETAOPT_DEF_INITMINSIGMA,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the lower boundary of sigma;";
        gpb.registerFileParameter<double>(
            "min_sigma_lb",
            min_sigma_lb_,
            GMETAOPT_DEF_MINSIGMA_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the lower boundary of sigma;";
        gpb.registerFileParameter<double>(
            "min_sigma_ub",
            min_sigma_ub_,
            GMETAOPT_DEF_MINSIGMA_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial maximum range for sigma;";
        gpb.registerFileParameter<double>(
            "init_sigma_range",
            init_sigma_range_,
            GMETAOPT_DEF_INITSIGMARANGE,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the maximum range of sigma;";
        gpb.registerFileParameter<double>(
            "sigma_range_lb",
            sigma_range_lb_,
            GMETAOPT_DEF_SIGMARANGE_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the maximum range of sigma;";
        gpb.registerFileParameter<double>(
            "sigma_range_ub",
            sigma_range_ub_,
            GMETAOPT_DEF_SIGMARANGE_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial percentage of the sigma range as a start value;";
        gpb.registerFileParameter<double>(
            "init_sigma_range_percentage",
            init_sigma_range_percentage_,
            GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The initial strength of self-adaption of gauss-mutation in ES;";
        gpb.registerFileParameter<double>(
            "init_sigma_sigma",
            init_sigma_sigma_,
            GMETAOPT_DEF_INITSIGMASIGMA,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The lower boundary for the variation of the strength of sigma adaption;";
        gpb.registerFileParameter<double>(
            "sigma_sigma_lb",
            sigma_sigma_lb_,
            GMETAOPT_DEF_SIGMASIGMA_LB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        comment = "";
        comment += "The upper boundary for the variation of the strength of sigma adaption;";
        gpb.registerFileParameter<double>(
            "sigma_sigma_ub",
            sigma_sigma_ub_,
            GMETAOPT_DEF_SIGMASIGMA_UB,
            Gem::Common::VAR_IS_ESSENTIAL,
            comment
        );

        // Allow our parent class to describe its options
        Gem::Common::GFactoryT<gpar::GParameterSet>::describeLocalOptions_(gpb);
    }

    /***************************************************************************/
    /**
     * Allows to act on the configuration options received from the configuration file. Here
     * we can add the options described in describeLocalOptions to the object. In practice,
     * we will usually add the parameter objects here. Note that a very similar constructor
     * exists for GMetaOptimizerIndividualT<ind_type>, so it may be used independently of the factory.
     *
     * @param p_base A smart-pointer to be acted on during post-processing
     */
    void postProcess_(std::shared_ptr<gpar::GParameterSet> &p_base) override {
        // Convert the base pointer to our local type
        std::shared_ptr<GMetaOptimizerIndividualT<ind_type>> p =
            Gem::Common::convertSmartPointer<gpar::GParameterSet, GMetaOptimizerIndividualT<ind_type>>(
                p_base
            );

        // We simply use a static function defined in GMetaOptimizerIndividualT<ind_type>
        GMetaOptimizerIndividualT<ind_type>::addContent(
            p,
            init_n_parents_,
            n_parents_lb_,
            n_parents_ub_,
            init_n_children_,
            n_children_lb_,
            n_children_ub_,
            init_amalgamation_lklh_,
            amalgamation_lklh_lb_,
            amalgamation_lklh_ub_,
            init_min_ad_prob_,
            min_ad_prob_lb_,
            min_ad_prob_ub_,
            init_ad_prob_range_,
            ad_prob_range_lb_,
            ad_prob_range_ub_,
            init_ad_prob_start_percentage_,
            init_adapt_ad_prob_,
            adapt_ad_prob_lb_,
            adapt_ad_prob_ub_,
            init_min_sigma_,
            min_sigma_lb_,
            min_sigma_ub_,
            init_sigma_range_,
            sigma_range_lb_,
            sigma_range_ub_,
            init_sigma_range_percentage_,
            init_sigma_sigma_,
            sigma_sigma_lb_,
            sigma_sigma_ub_
        );

        // Finally add the individual factory to p
        p->registerIndividualFactory(ind_factory_);
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
    std::shared_ptr<gpar::GParameterSet> getObject_(
        Gem::Common::GParserBuilder &gpb,
        const std::size_t & /*id*/
    ) override {
        // Will hold the result
        std::shared_ptr<GMetaOptimizerIndividualT<ind_type>> target(
            new GMetaOptimizerIndividualT<ind_type>()
        );

        // Make the object's local configuration options known
        target->addConfigurationOptions(gpb);

        return target;
    }

    /***************************************************************************/
    // Data

    // Parameters pertaining to the ea population
    std::size_t init_n_parents_ = GMETAOPT_DEF_INITNPARENTS; ///< The initial number of parents
    std::size_t n_parents_lb_ =
        GMETAOPT_DEF_NPARENTS_LB; ///< The lower boundary for variations of the number of parents
    std::size_t n_parents_ub_ =
        GMETAOPT_DEF_NPARENTS_UB; ///< The upper boundary for variations of the number of parents

    std::size_t init_n_children_ = GMETAOPT_DEF_INITNCHILDREN; ///< The initial number of children
    std::size_t n_children_lb_ =
        GMETAOPT_DEF_NCHILDREN_LB; ///< The lower boundary for the variation of the number of children
    std::size_t n_children_ub_ =
        GMETAOPT_DEF_NCHILDREN_UB; ///< The upper boundary for the variation of the number of children

    double init_amalgamation_lklh_ =
        GMETAOPT_DEF_INITAMALGLKLHOOD; ///< The initial likelihood for an individual being created from cross-over rather than "just" duplication
    double amalgamation_lklh_lb_ =
        GMETAOPT_DEF_AMALGLKLHOOD_LB; ///< The upper boundary for the variation of the amalgamation likelihood
    double amalgamation_lklh_ub_ =
        GMETAOPT_DEF_AMALGLKLHOOD_UB; ///< The upper boundary for the variation of the amalgamation likelihood

    double init_min_ad_prob_ =
        GMETAOPT_DEF_INITMINADPROB; ///< The initial lower boundary for the variation of ad_prob
    double min_ad_prob_lb_ = GMETAOPT_DEF_MINADPROB_LB; ///< The lower boundary for min_ad_prob
    double min_ad_prob_ub_ = GMETAOPT_DEF_MINADPROB_UB; ///< The upper boundary for min_ad_prob

    double init_ad_prob_range_ =
        GMETAOPT_DEF_INITADPROBRANGE; ///< The initial range for the variation of ad_prob
    double ad_prob_range_lb_ = GMETAOPT_DEF_ADPROBRANGE_LB; ///< The lower boundary for ad_prob_range
    double ad_prob_range_ub_ = GMETAOPT_DEF_ADPROBRANGE_UB; ///< The upper boundary for ad_prob_range

    double init_ad_prob_start_percentage_ =
        GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE; ///< The start value for ad_prob relative to the allowed value range

    double init_adapt_ad_prob_ =
        GMETAOPT_DEF_INITADAPTADPROB; ///< The initial value of the strength of adProb_ adaption
    double adapt_ad_prob_lb_ =
        GMETAOPT_DEF_ADAPTADPROB_LB; ///< The lower boundary for the variation of the strength of adProb_ adaption
    double adapt_ad_prob_ub_ =
        GMETAOPT_DEF_ADAPTADPROB_UB; ///< The upper boundary for the variation of the strength of adProb_ adaption

    double init_min_sigma_ = GMETAOPT_DEF_INITMINSIGMA; ///< The initial minimal value of sigma
    double min_sigma_lb_ =
        GMETAOPT_DEF_MINSIGMA_LB; ///< The lower boundary for the variation of the lower boundary of sigma
    double min_sigma_ub_ =
        GMETAOPT_DEF_MINSIGMA_UB; ///< The upper boundary for the variation of the lower boundary of sigma

    double init_sigma_range_ =
        GMETAOPT_DEF_INITSIGMARANGE; ///< The initial range of sigma (beyond min_sigma
    double sigma_range_lb_ =
        GMETAOPT_DEF_SIGMARANGE_LB; ///< The lower boundary for the variation of the maximum range of sigma
    double sigma_range_ub_ =
        GMETAOPT_DEF_SIGMARANGE_UB; ///< The upper boundary for the variation of the maximum range of sigma

    double init_sigma_range_percentage_ =
        GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE; ///< The initial percentage of the sigma range as a start value

    double init_sigma_sigma_ =
        GMETAOPT_DEF_INITSIGMASIGMA; ///< The initial strength of sigma adaption
    double sigma_sigma_lb_ =
        GMETAOPT_DEF_SIGMASIGMA_LB; ///< The lower boundary for the variation of the strength of sigma adaption
    double sigma_sigma_ub_ =
        GMETAOPT_DEF_SIGMASIGMA_UB; ///< The upper boundary for the variation of the strength of sigma adaption

    std::shared_ptr<typename ind_type::FACTORYTYPE>
        ind_factory_; ///< Holds a factory for our individuals. It will be added to the individuals when needed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

constexpr std::size_t P_XDIM = 1200;
constexpr std::size_t P_YDIM = 1400;

/******************************************************************************/
/**
 * This class implements a pluggable optimization monitor for Evolutionary Algorithms. Its main purpose
 * is to find out information about the development of various properties (e.g. sigma, best evaluation)
 * over the course of the optimization for the best individuals. This monitor is thus targeted at a specific
 * type of individual. Note that the class uses ROOT scripts for the output of its results.
 *
 * TODO: templatize this class on executor_type, like is being done for the other optimization monitors
 */
template <typename ind_type>
class GOptOptMonitorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    // Make sure this class can only be instantiated if individual_type is a derivative of GParameterSet
    static_assert(
        std::is_base_of_v<gpar::GParameterSet, ind_type>,
        "GParameterSet is no base class of ind_type"
    );

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(file_name_) &
            BOOST_SERIALIZATION_NVP(gpd_) & BOOST_SERIALIZATION_NVP(progress_plotter_) &
            BOOST_SERIALIZATION_NVP(n_parent_plotter_) &
            BOOST_SERIALIZATION_NVP(n_children_plotter_) & BOOST_SERIALIZATION_NVP(ad_prob_plotter_) &
            BOOST_SERIALIZATION_NVP(min_sigma_plotter_) &
            BOOST_SERIALIZATION_NVP(max_sigma_plotter_) &
            BOOST_SERIALIZATION_NVP(sigma_range_plotter_) &
            BOOST_SERIALIZATION_NVP(sigma_sigma_plotter_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GOptOptMonitorT(const std::string file_name)
      : file_name_(file_name)
      , gpd_("Progress information", 2, 4)
      , progress_plotter_(new Gem::Common::GGraph2D())
      , n_parent_plotter_(new Gem::Common::GGraph2D())
      , n_children_plotter_(new Gem::Common::GGraph2D())
      , ad_prob_plotter_(new Gem::Common::GGraph2D())
      , min_sigma_plotter_(new Gem::Common::GGraph2D())
      , max_sigma_plotter_(new Gem::Common::GGraph2D())
      , sigma_range_plotter_(new Gem::Common::GGraph2D())
      , sigma_sigma_plotter_(new Gem::Common::GGraph2D()) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GOptOptMonitorT object
     */
    GOptOptMonitorT(const GOptOptMonitorT<ind_type> &cp)
      : oa::GBasePluggableOM(cp)
      , file_name_(cp.file_name_)
      , gpd_(
            "Progress information",
            2,
            4
        ) // We do not want to copy progress information of another object
      , progress_plotter_(new Gem::Common::GGraph2D())
      , n_parent_plotter_(new Gem::Common::GGraph2D())
      , n_children_plotter_(new Gem::Common::GGraph2D())
      , ad_prob_plotter_(new Gem::Common::GGraph2D())
      , min_sigma_plotter_(new Gem::Common::GGraph2D())
      , max_sigma_plotter_(new Gem::Common::GGraph2D())
      , sigma_range_plotter_(new Gem::Common::GGraph2D())
      , sigma_sigma_plotter_(new Gem::Common::GGraph2D()) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GOptOptMonitorT() override { /* nothing */
    }

    /***************************************************************************/
    /**
     * Sets the file name
     */
    void setFileName(std::string file_name) {
        file_name_ = file_name;
    }

    /***************************************************************************/
    /**
     * Retrieves the current file name
     */
    std::string getFileName() const {
        return file_name_;
    }

protected:
    /***************************************************************************/
    /**
       * Loads the data of another object
       *
       * @param cp A copy of another GOptOptMonitorT object, camouflaged as a GBasePluggableOM
       */
    void load_(const oa::GBasePluggableOM *cp) override {
        // Check that we are dealing with a GOptOptMonitorT<ind_type> reference independent of this object and convert the pointer
        const GOptOptMonitorT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GOptOptMonitorT<ind_type>>(cp, this);

        // Trigger loading of our parent class'es data
        // Load the parent classes' data ...
        oa::GBasePluggableOM::load_(cp);

        // Load local data
        file_name_ = p_load->file_name_;
        gpd_ = p_load->gpd_;
        Gem::Common::copyCloneableSmartPointer(p_load->progress_plotter_, progress_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->n_parent_plotter_, n_parent_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->n_children_plotter_, n_children_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->ad_prob_plotter_, ad_prob_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->min_sigma_plotter_, min_sigma_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->max_sigma_plotter_, max_sigma_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->sigma_range_plotter_, sigma_range_plotter_);
        Gem::Common::copyCloneableSmartPointer(p_load->sigma_sigma_plotter_, sigma_sigma_plotter_);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptOptMonitorT<ind_type>>(
        GOptOptMonitorT<ind_type> const &,
        GOptOptMonitorT<ind_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GBasePluggableOM object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GOptOptMonitorT<ind_type> reference independent of this object and convert the pointer
        const GOptOptMonitorT<ind_type> *p_load =
            Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GOptOptMonitorT<ind_type>>(cp, this);

        GToken token("GOptOptMonitorT", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(file_name_, p_load->file_name_), token);
        compare_t(IDENTITY(gpd_, p_load->gpd_), token);
        compare_t(IDENTITY(progress_plotter_, p_load->progress_plotter_), token);
        compare_t(IDENTITY(n_parent_plotter_, p_load->n_parent_plotter_), token);
        compare_t(IDENTITY(n_children_plotter_, p_load->n_children_plotter_), token);
        compare_t(IDENTITY(ad_prob_plotter_, p_load->ad_prob_plotter_), token);
        compare_t(IDENTITY(min_sigma_plotter_, p_load->min_sigma_plotter_), token);
        compare_t(IDENTITY(max_sigma_plotter_, p_load->max_sigma_plotter_), token);
        compare_t(IDENTITY(sigma_range_plotter_, p_load->sigma_range_plotter_), token);
        compare_t(IDENTITY(sigma_sigma_plotter_, p_load->sigma_sigma_plotter_), token);

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

        bool result = false;

        // Call the parent classes' functions
        if(oa::GBasePluggableOM::modify_GUnitTests_()) {
            result = true;
        }

        // no local data -- nothing to change

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GOptOptMonitorT<ind_type>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GOptOptMonitorT<ind_type>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GOptOptMonitorT<ind_type>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
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
    oa::GBasePluggableOM *clone_() const override {
        return new GOptOptMonitorT<ind_type>(*this);
    }

    /***************************************************************************/
    /**
     * Allows to emit information in different stages of the information cycle
     * (initialization, during each cycle and during finalization)
     */
    void informationFunction_(infoMode im, oa::GBase const *const goa) override {
        using namespace Gem::Common;

        switch(im) {
        case Gem::Geneva::infoMode::INFOINIT: {
            // Initialize the plots we want to record
            progress_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            progress_plotter_->setPlotLabel("Number of solver calls");
            progress_plotter_->setXAxisLabel("Iteration");
            progress_plotter_->setYAxisLabel("Best Result (lower is better)");

            n_parent_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            n_parent_plotter_->setPlotLabel("Number of parents as a function of the iteration");
            n_parent_plotter_->setXAxisLabel("Iteration");
            n_parent_plotter_->setYAxisLabel("Number of parents");

            n_children_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            n_children_plotter_->setPlotLabel("Number of children as a function of the iteration");
            n_children_plotter_->setXAxisLabel("Iteration");
            n_children_plotter_->setYAxisLabel("Number of children");

            ad_prob_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            ad_prob_plotter_->setPlotLabel("Adaption probability as a function of the iteration");
            ad_prob_plotter_->setXAxisLabel("Iteration");
            ad_prob_plotter_->setYAxisLabel("Adaption probability");

            min_sigma_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            min_sigma_plotter_->setPlotLabel("Lower sigma boundary as a function of the iteration");
            min_sigma_plotter_->setXAxisLabel("Iteration");
            min_sigma_plotter_->setYAxisLabel("Lower sigma boundary");

            max_sigma_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            max_sigma_plotter_->setPlotLabel("Upper sigma boundary as a function of the iteration");
            max_sigma_plotter_->setXAxisLabel("Iteration");
            max_sigma_plotter_->setYAxisLabel("Upper sigma boundary");

            sigma_range_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            sigma_range_plotter_->setPlotLabel(
                "Development of the sigma range as a function of the iteration"
            );
            sigma_range_plotter_->setXAxisLabel("Iteration");
            sigma_range_plotter_->setYAxisLabel("Sigma range");

            sigma_sigma_plotter_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
            sigma_sigma_plotter_->setPlotLabel(
                "Development of the adaption strength as a function of the iteration"
            );
            sigma_sigma_plotter_->setXAxisLabel("Iteration");
            sigma_sigma_plotter_->setYAxisLabel("Sigma-Sigma");

            gpd_.registerPlotter(progress_plotter_);
            gpd_.registerPlotter(n_parent_plotter_);
            gpd_.registerPlotter(n_children_plotter_);
            gpd_.registerPlotter(ad_prob_plotter_);
            gpd_.registerPlotter(min_sigma_plotter_);
            gpd_.registerPlotter(max_sigma_plotter_);
            gpd_.registerPlotter(sigma_range_plotter_);
            gpd_.registerPlotter(sigma_sigma_plotter_);

            gpd_.setCanvasDimensions(P_XDIM, P_YDIM);
        } break;

        case Gem::Geneva::infoMode::INFOPROCESSING: {
            // Convert the base pointer to the target type
            oa::GEvolutionaryAlgorithm const *const ea =
                static_cast<oa::GEvolutionaryAlgorithm const *const>(
                    goa
                ); // NOLINT(cppcoreguidelines-init-variables)

            // Extract the requested data. First retrieve the best individual.
            // It can always be found in the first position with evolutionary algorithms
            std::shared_ptr<GMetaOptimizerIndividualT<ind_type>> p =
                ea->clone_at<GMetaOptimizerIndividualT<ind_type>>(0);

            // Retrieve the best fitness and average sigma value and add it to our local storage
            (*progress_plotter_) &
                std::tuple<double, double>(static_cast<double>(ea->getIteration()), p->raw_fitness(0));
            (*n_parent_plotter_) &
                std::tuple<double, double>(static_cast<double>(ea->getIteration()), static_cast<double>(p->getNParents()));
            (*n_children_plotter_) &
                std::tuple<double, double>(static_cast<double>(ea->getIteration()), static_cast<double>(p->getNChildren()));
            (*ad_prob_plotter_) &
                std::tuple<double, double>(static_cast<double>(ea->getIteration()), p->getAdProb());

            double min_sigma = p->getMinSigma();
            double sigma_range = p->getSigmaRange();
            double max_sigma = min_sigma + sigma_range;

            (*min_sigma_plotter_) & std::tuple<double, double>(static_cast<double>(ea->getIteration()), min_sigma);
            (*max_sigma_plotter_) & std::tuple<double, double>(static_cast<double>(ea->getIteration()), max_sigma);
            (*sigma_range_plotter_) &
                std::tuple<double, double>(static_cast<double>(ea->getIteration()), sigma_range);
            (*sigma_sigma_plotter_) &
                std::tuple<double, double>(static_cast<double>(ea->getIteration()), p->getSigmaSigma());
        } break;

        case Gem::Geneva::infoMode::INFOEND: {
            // Write out the result
            gpd_.writeToFile(file_name_);
        } break;

        default: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptOptMonitorT<ind_type>>: Received invalid infoMode " << im << '\n'
            );
        }
        };
    }

    /***************************************************************************/

    GOptOptMonitorT()
      : gpd_("empty", 1, 1) { /* empty */
      }; ///< Default constructor; Intentionally private (only needed for serialization)

    std::string file_name_; ///< The name of the output file

    Gem::Common::GPlotDesigner gpd_; ///< Ease recording of essential information

    std::shared_ptr<Gem::Common::GGraph2D> progress_plotter_; ///< Records progress information
    std::shared_ptr<Gem::Common::GGraph2D>
        n_parent_plotter_; ///< Records the number of parents in the individual
    std::shared_ptr<Gem::Common::GGraph2D>
        n_children_plotter_; ///< Records the number of children in the individual
    std::shared_ptr<Gem::Common::GGraph2D>
        ad_prob_plotter_; ///< Records the adaption probability for the individual
    std::shared_ptr<Gem::Common::GGraph2D>
        min_sigma_plotter_; ///< Records the development of the lower sigma boundary
    std::shared_ptr<Gem::Common::GGraph2D>
        max_sigma_plotter_; ///< Records the development of the upper sigma boundary
    std::shared_ptr<Gem::Common::GGraph2D>
        sigma_range_plotter_; ///< Records the development of the sigma range
    std::shared_ptr<Gem::Common::GGraph2D>
        sigma_sigma_plotter_; ///< Records the development of the adaption strength
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(
    Gem::Geneva::Individuals::GMetaOptimizerIndividualT<Gem::Geneva::Individuals::GFunctionIndividual>
) // NOLINT

BOOST_CLASS_EXPORT_KEY(
    Gem::Geneva::Individuals::GOptOptMonitorT<Gem::Geneva::Individuals::GFunctionIndividual>
) // NOLINT
