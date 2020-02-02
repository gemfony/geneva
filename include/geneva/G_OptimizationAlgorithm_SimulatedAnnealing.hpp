/**
 * @file
 */

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

// Standard headers go here
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_ParChild.hpp"
#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing_PersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. The class adds
 * an infrastructure for simulated annealing (Geneva-style, i.e. with larger populations).
 */
class GSimulatedAnnealing :
    public G_OptimizationAlgorithm_ParChild
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "G_OptimizationAlgorithm_ParChild"
            , boost::serialization::base_object<G_OptimizationAlgorithm_ParChild>(*this))
        & BOOST_SERIALIZATION_NVP(m_t0)
        & BOOST_SERIALIZATION_NVP(m_t)
        & BOOST_SERIALIZATION_NVP(m_alpha)
        & BOOST_SERIALIZATION_NVP(m_n_threads);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    G_API_GENEVA GSimulatedAnnealing();
    /** @brief A standard copy constructor */
    G_API_GENEVA GSimulatedAnnealing(const GSimulatedAnnealing &) = default;
    /** @brief The standard destructor */
    G_API_GENEVA ~GSimulatedAnnealing() override = default;

    /** @brief Sets the number of threads this population uses for adaption */
    G_API_GENEVA void setNThreads(std::uint16_t nThreads);
    /** @brief Retrieves the number of threads this population uses for adaption */
    G_API_GENEVA std::uint16_t getNThreads() const;

    /** @brief Determines the strength of the temperature degradation */
    G_API_GENEVA void setTDegradationStrength(double alpha);
    /** @brief Retrieves the temperature degradation strength. This function is used for simulated annealing */
    G_API_GENEVA double getTDegradationStrength() const;

    /** @brief Sets the start temperature. This function is used for simulated annealing */
    G_API_GENEVA void setT0(double t0);
    /** @brief Retrieves the start temperature. This function is used for simulated annealing */
    G_API_GENEVA double getT0() const;
    /** @brief Retrieves the current temperature. This function is used for simulated annealing */
    G_API_GENEVA double getT() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(
        Gem::Common::GParserBuilder &gpb
    ) override;

    /** @brief Loads the data of another GSimulatedAnnealingT object, camouflaged as a GObject */
    G_API_GENEVA void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSimulatedAnnealing>(
        GSimulatedAnnealing const &
        , GSimulatedAnnealing const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject &cp // the other object
        , const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        , const double &limit// the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    G_API_GENEVA void resetToOptimizationStart_() override;

    /** @brief Does any necessary initialization work before the optimization loop starts */
    G_API_GENEVA void init() override;
    /** @brief Does any necessary finalization work after the optimization loop has ended */
    G_API_GENEVA void finalize() override;

    /** @brief Applies modifications to this object */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

    /** @brief Fixes the population after a job submission */
    G_API_GENEVA void fixAfterJobSubmission();

private:
    /***************************************************************************/
    // Virtual or overridden private functions

    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep copy of this object */
    G_API_GENEVA GObject *clone_() const override;

    /** @brief  We submit individuals to the broker connector and wait for processed items. */
    G_API_GENEVA void runFitnessCalculation_() override;

    /** @brief Returns information about the type of optimization algorithm */
    G_API_GENEVA std::string getAlgorithmPersonalityType_() const override;
    /** @brief Returns the name of this optimization algorithm */
    G_API_GENEVA std::string getAlgorithmName_() const override;

    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    /** @brief Adapt all children in parallel */
    G_API_GENEVA void adaptChildren_() override;
    /** @brief Choose new parents, based on the SA selection scheme. */
    G_API_GENEVA void selectBest_() override;

    /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
    G_API_GENEVA std::tuple<std::size_t, std::size_t> getEvaluationRange_() const override;
    /** @brief Some error checks related to population sizes */
    G_API_GENEVA void populationSanityChecks_() const override;

    /***************************************************************************/

    /** @brief Performs a simulated annealing style sorting and selection */
    void sortSAMode();

    /** @brief Calculates the simulated annealing probability for a child to replace a parent */
    double saProb(const double &qParent, const double &qChild);

    /** @brief Updates the temperature. This function is used for simulated annealing. */
    void updateTemperature();

    /***************************************************************************/
    // Data

    double m_t0 = SA_T0; ///< The start temperature, used in simulated annealing
    double m_t = m_t0; ///< The current temperature, used in simulated annealing
    double m_alpha = SA_ALPHA; ///< A constant used in the cooling schedule in simulated annealing

    std::uint16_t m_n_threads = Gem::Common::DEFAULTNHARDWARETHREADS; ///< The number of threads

    std::shared_ptr<Gem::Common::GThreadPool> m_tp_ptr; ///< Temporarily holds a thread pool
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSimulatedAnnealing)

