/**
 * @file G_OA_EvolutionaryAlgorithm.hpp
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
#include <tuple>

// Boost headers go here

#ifndef G_OA_EVOLUTIONARYALGORITHM_HPP_
#define G_OA_EVOLUTIONARYALGORITHM_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_ParChild.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/**
 * The default sorting mode
 */
const sortingMode DEFAULTEASORTINGMODE = sortingMode::MUCOMMANU_SINGLEEVAL;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is a specialization of the GParChildT<executor_type> class. The class adds
 * an infrastructure for evolutionary algorithms.
 */
class GEvolutionaryAlgorithm
	: public G_OptimizationAlgorithm_ParChild
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OptimizationAlgorithm_ParChild", boost::serialization::base_object<G_OptimizationAlgorithm_ParChild>(*this))
		 & BOOST_SERIALIZATION_NVP(m_sorting_mode)
		 & BOOST_SERIALIZATION_NVP(m_n_threads);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /** @brief The default constructor */
	 G_API_GENEVA GEvolutionaryAlgorithm();
	 /** @brief A standard copy constructor */
	 G_API_GENEVA GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm& cp);
	 /** @brief The standard destructor */
	 G_API_GENEVA ~GEvolutionaryAlgorithm() override = default;

	 /** @brief The standard assignment operator */
	 G_API_GENEVA  GEvolutionaryAlgorithm& operator=(const GEvolutionaryAlgorithm& cp);

	 /** @brief Checks for equality with another GEvolutionaryAlgorithm object */
	 virtual G_API_GENEVA bool operator==(const GEvolutionaryAlgorithm& cp) const;
	 /** @brief Checks for inequality with another GEvolutionaryAlgorithm object */
	 virtual G_API_GENEVA bool operator!=(const GEvolutionaryAlgorithm& cp) const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 G_API_GENEVA void compare(
		 const GObject& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit// the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
	 G_API_GENEVA void resetToOptimizationStart() override;

	 /** @brief Returns information about the type of optimization algorithm. */
	 G_API_GENEVA std::string getAlgorithmPersonalityType() const override;

	 /** @brief Returns the name of this optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmName() const override;

	 /** @brief Sets the sorting scheme */
	 G_API_GENEVA void setSortingScheme(sortingMode smode);
	 /** @brief Retrieves information about the current sorting scheme */
	 G_API_GENEVA sortingMode getSortingScheme() const;

	 /** @brief Extracts all individuals on the pareto front */
	 G_API_GENEVA void extractCurrentParetoIndividuals(std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>>& paretoInds);

	 /** @brief Adds the individuals of this iteration to a priority queue */
	 G_API_GENEVA void updateGlobalBestsPQ(
		 GParameterSetFixedSizePriorityQueue & bestIndividuals
	 ) override;
	 /** @brief Adds the individuals of this iteration to a priority queue */
	 G_API_GENEVA void updateIterationBestsPQ(
		 GParameterSetFixedSizePriorityQueue& bestIndividuals
	 ) override;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_GENEVA void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override;

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;

	 /** @brief Sets the number of threads this population uses for adaption */
	 G_API_GENEVA void setNThreads(std::uint16_t nThreads);
	 /** @brief Retrieves the number of threads this population uses for adaption */
	 G_API_GENEVA std::uint16_t getNThreads() const;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GEvolutionaryAlgorithm object, camouflaged as a GObject */
	 G_API_GENEVA void load_(const GObject *cp) override;

	 /** @brief Some error checks related to population sizes */
	 G_API_GENEVA void populationSanityChecks() const override;

	 /** @brief Adapt all children in parallel */
	 G_API_GENEVA void adaptChildren() override;

	 /** @brief We submit individuals to the broker connector and wait for processed items */
	 G_API_GENEVA void runFitnessCalculation() override;

	 /** @brief Fixes the population after a job submission */
	 G_API_GENEVA void fixAfterJobSubmission();

	 /** @brief Choose new parents, based on the selection scheme set by the user */
	 G_API_GENEVA void selectBest() override;

	 /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
	 G_API_GENEVA std::tuple<std::size_t,std::size_t> getEvaluationRange() const override;

	 /** @brief Does any necessary initialization work before the optimization cycle starts */
	 G_API_GENEVA void init() override;
	 /** @brief Does any necessary finalization work */
	 G_API_GENEVA void finalize() override;

	 /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	 G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

private:
	 /** @brief Creates a deep copy of this object */
	 G_API_GENEVA GObject *clone_() const override;

	 /** @brief Selection, MUPLUSNU_SINGLEEVAL style */
	 G_API_GENEVA void sortMuPlusNuMode();
	 /** @brief Selection, MUCOMMANU_SINGLEEVAL style */
	 G_API_GENEVA void sortMuCommaNuMode();
	 /** @brief Selection, MUNU1PRETAIN_SINGLEEVAL style */
	 G_API_GENEVA void sortMunu1pretainMode();

	 /** @brief Selection according to the pareto tag, also taking into account the parents of a population (i.e. in MUPLUSNU mode). */
	 G_API_GENEVA void sortMuPlusNuParetoMode();
	 /** @brief Selection according to the pareto tag, not taking into account the parents of a population (i.e. in MUCOMMANU mode). */
	 G_API_GENEVA void sortMuCommaNuParetoMode();
	 /** @brief Determines whether the first individual dominates the second */
	 G_API_GENEVA bool aDominatesB(
		 std::shared_ptr < GParameterSet > a
		 , std::shared_ptr < GParameterSet > b
	 ) const;

	 /***************************************************************************/
	 // Local data

	 sortingMode m_sorting_mode = DEFAULTEASORTINGMODE; ///< The chosen sorting scheme
	 std::uint16_t m_n_threads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(
		 Gem::Common::DEFAULTNHARDWARETHREADS
		 , Gem::Common::DEFAULTMAXNHARDWARETHREADS
	 )); ///< The number of threads
	 std::shared_ptr<Gem::Common::GThreadPool> m_tp_ptr; ///< Temporarily holds a thread pool

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Fills the collection with individuals */
	 G_API_GENEVA void fillWithObjects(const std::size_t &nIndividuals);
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithm)

#endif /* G_OA_EVOLUTIONARYALGORITHM_HPP_ */
