/**
 * @file GEvolutionaryAlgorithmT.hpp
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

#ifndef GBASEEA_HPP_
#define GBASEEA_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT2.hpp"
#include "geneva/GEAPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/**
 * The default sorting mode
 */
const sortingMode DEFAULTSORTINGMODE = sortingMode::MUCOMMANU_SINGLEEVAL;

/******************************************************************************/
/**
 * This is a specialization of the GBaseParChildT2 class. It provides the
 * main infrastructure for evolutionary algorithms.
 *
 * TODO: Provide specializations for serial execution / threadpool
 * TODO: How to tell the GBrokerExecutor about its return mode ? --> Specializations of the constructor, manually setting the mode if necessary
 */
template <typename executor_type = Gem::Courtier::GMTExecutorT<GParameterSet>>
class GEvolutionaryAlgorithmT :public GBaseParChildT2<executor_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GParameterSetParChild", boost::serialization::base_object<GBaseParChildT2<executor_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_sorting_mode);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GEvolutionaryAlgorithmT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GEvolutionaryAlgorithmT(const GEvolutionaryAlgorithmT<executor_type>& cp)
	 	: GBaseParChildT2<executor_type>(cp)
	   , m_sorting_mode(cp.m_sorting_mode)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GEvolutionaryAlgorithmT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard assignment operator
	  */
	 const GEvolutionaryAlgorithmT<executor_type>& operator=(const GEvolutionaryAlgorithmT<executor_type>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GEvolutionaryAlgorithmT object
	  */
	 virtual bool operator==(const GEvolutionaryAlgorithmT<executor_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GEvolutionaryAlgorithmT object
	  */
	 virtual bool operator!=(const GEvolutionaryAlgorithmT<executor_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns information about the type of optimization algorithm
	  */
	 virtual std::string getOptimizationAlgorithm() const override {
		 return "PERSONALITY_EA";
	 }

	 /***************************************************************************/
	 /**
	  * Set the sorting scheme for this population
	  */
	 void setSortingScheme(sortingMode smode) {
		 m_sorting_mode = smode;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the current sorting scheme for this population
	  */
	 sortingMode getSortingScheme() const {
		 return m_sorting_mode;
	 }

	 /***************************************************************************/
	 /**
	  * Extracts all individuals on the pareto front
	  */
	 void extractCurrentParetoIndividuals(
		 std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>>& paretoInds
	 ) {
		 // Make sure the vector is empty
		 paretoInds.clear();

		 for(auto item: *this) {
			 if(item->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) {
				 paretoInds.push_back(item);
			 }
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Adds the individuals of this iteration to a priority queue
	  */
	 virtual void updateGlobalBestsPQ(
		 GParameterSetFixedSizePriorityQueue& bestIndividuals
	 ) override {
		 const bool REPLACE = true;
		 const bool CLONE = true;

#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GEvolutionaryAlgorithmT<executor_type>::updateGlobalBestsPQ() :" << std::endl
				 << "Tried to retrieve the best individuals even though the population is empty." << std::endl
				 << GEXCEPTION;
		 }
#endif /* DEBUG */

		 switch (m_sorting_mode) {
			 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_SINGLEEVAL:
			 case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
			 case sortingMode::MUCOMMANU_SINGLEEVAL:
				 GOptimizationAlgorithmT2<executor_type>::updateGlobalBestsPQ(bestIndividuals);
				 break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_PARETO:
			 case sortingMode::MUCOMMANU_PARETO: {
				 // Retrieve all individuals on the pareto front
				 std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> paretoInds;
				 this->extractCurrentParetoIndividuals(paretoInds);

				 // We simply add all parent individuals to the queue. As we only want
				 // the individuals on the current pareto front, we replace all members
				 // of the current priority queue
				 bestIndividuals.add(paretoInds, CLONE, REPLACE);
			 }
				 break;

			 //----------------------------------------------------------------------------
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Adds an iteration's individuals to a priority queue (which will be cleared)
	  */
	 virtual void updateIterationBestsPQ(
		 GParameterSetFixedSizePriorityQueue& bestIndividuals
	 ) override {
		 const bool REPLACE = true;
		 const bool CLONE = true;

#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "GEvolutionaryAlgorithmT<executor_type>::updateIterationBestsPQ() :" << std::endl
				 << "Tried to retrieve the best individuals even though the population is empty." << std::endl
				 << GEXCEPTION;
		 }
#endif /* DEBUG */

		 switch (m_sorting_mode) {
			 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_SINGLEEVAL:
			 case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
			 case sortingMode::MUCOMMANU_SINGLEEVAL: {
				 GOptimizationAlgorithmT<executor_type>::updateIterationBestsPQ(bestIndividuals);
			 } break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_PARETO:
			 case sortingMode::MUCOMMANU_PARETO: {
				 // Retrieve all individuals on the pareto front
				 std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> paretoInds;
				 this->extractCurrentParetoIndividuals(paretoInds);

				 // We simply add all parent individuals to the queue. As we only want
				 // the individuals on the current pareto front, we replace all members
				 // of the current priority queue
				 bestIndividuals.add(paretoInds, CLONE, REPLACE);
			 } break;

				 //----------------------------------------------------------------------------
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this optimization algorithm
	  */
	 virtual std::string getAlgorithmName() const override {
		 return std::string("Evolutionary Algorithm");
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override {
		 // Call our parent class'es function
		 GBaseParChildT2<executor_type>::addConfigurationOptions(gpb);

		 gpb.registerFileParameter<sortingMode>(
			 "sortingMethod" // The name of the variable
			 , DEFAULTSORTINGMODE // The default value
			 , [this](sortingMode sm) { this->setSortingScheme(sm); }
		 )
			 << "The sorting scheme. Options" << std::endl
			 << "0: MUPLUSNU mode with a single evaluation criterion" << std::endl
			 << "1: MUCOMMANU mode with a single evaluation criterion" << std::endl
			 << "2: MUCOMMANU mode with single evaluation criterion," << std::endl
			 << "   the best parent of the last iteration is retained" << std::endl
			 << "   unless a better individual has been found" << std::endl
			 << "3: MUPLUSNU mode for multiple evaluation criteria, pareto selection" << std::endl
			 << "4: MUCOMMANU mode for multiple evaluation criteria, pareto selection";
	 }

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 virtual std::string name() const override {
		 return std::string("GEvolutionaryAlgorithmT");
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object of the same type
	  */
	 virtual void compare(
		 const GObject& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GBaseEA reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmT<executor_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GBaseParChildT2<executor_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBaseParChildT2<executor_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(m_sorting_mode, p_load->m_sorting_mode), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another population
	  */
	 virtual void load_(const GObject * cp) override {
		 // Check that we are dealing with a GBaseEA reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmT<executor_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 // First load the parent class'es data ...
		 GBaseParChildT2<executor_type>::load_(cp);

		 // ... and then our own data
		 m_sorting_mode = p_load->m_sorting_mode;
	 }

	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  */
	 virtual GObject *clone_() const override {
		 return new GEvolutionaryAlgorithmT<executor_type>(*this);
	 }

	 /***************************************************************************/
	 /**
	  * Some error checks related to population sizes
	  */
	 virtual void populationSanityChecks() const override {
		 // First check that we have been given a suitable value for the number of parents.
		 // Note that a number of checks (e.g. population size != 0) has already been done
		 // in the parent class.
		 if (m_n_parents == 0) {
			 glogger
				 << "In GEvolutionaryAlgorithmT<executor_type>::populationSanityChecks(): Error!" << std::endl
				 << "Number of parents is set to 0"
				 << GEXCEPTION;
		 }

		 // In MUCOMMANU_SINGLEEVAL mode we want to have at least as many children as parents,
		 // whereas MUPLUSNU_SINGLEEVAL only requires the population size to be larger than the
		 // number of parents. MUNU1PRETAIN has the same requirements as MUCOMMANU_SINGLEEVAL,
		 // as it is theoretically possible that all children are better than the former
		 // parents, so that the first parent individual will be replaced.
		 std::size_t popSize = getPopulationSize();
		 if ( // TODO: Why are PARETO modes missing here ?
			 ((m_sorting_mode == sortingMode::MUCOMMANU_SINGLEEVAL || m_sorting_mode == sortingMode::MUNU1PRETAIN_SINGLEEVAL) && (popSize < 2 * m_n_parents))
			 || (m_sorting_mode == sortingMode::MUPLUSNU_SINGLEEVAL && popSize <= m_n_parents)
		 ) {
			 std::ostringstream error;
			 error
				 << "In GEvolutionaryAlgorithmT<executor_type>::populationSanityChecks() :" << std::endl
				 << "Requested size of population is too small :" << popSize << " " << m_n_parents << std::endl
				 << "Sorting scheme is ";

			 switch (m_sorting_mode) {
				 case sortingMode::MUPLUSNU_SINGLEEVAL:
					 error << "MUPLUSNU_SINGLEEVAL" << std::endl;
					 break;
				 case sortingMode::MUCOMMANU_SINGLEEVAL:
					 error << "MUCOMMANU_SINGLEEVAL" << std::endl;
					 break;
				 case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
					 error << "MUNU1PRETAIN" << std::endl;
					 break;
				 case sortingMode::MUPLUSNU_PARETO:
					 error << "MUPLUSNU_PARETO" << std::endl;
					 break;
				 case sortingMode::MUCOMMANU_PARETO:
					 error << "MUCOMMANU_PARETO" << std::endl;
					 break;
			 };

			 glogger
				 << error.str()
				 << GEXCEPTION;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Adapts all children of this population
	  */
	 virtual void adaptChildren() override {

	 }

	 /***************************************************************************/
	 /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	 virtual G_API_GENEVA void runFitnessCalculation() override = 0;

	 /***************************************************************************/
	 /** @brief Selects the best children of the population */
	 virtual G_API_GENEVA void selectBest() override;

	 /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
	 virtual G_API_GENEVA std::tuple<std::size_t,std::size_t> getEvaluationRange() const override;

	 /***************************************************************************/
	 /** @brief Does some preparatory work before the optimization starts */
	 virtual G_API_GENEVA void init() override;

	 /***************************************************************************/
	 /** @brief Does any necessary finalization work */
	 virtual G_API_GENEVA void finalize() override;

	 /***************************************************************************/
	 /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	 virtual G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

private:
	 /***************************************************************************/
	 /** @brief Selection according to the pareto tag in MUPLUSNU mode (i.e. taking into account the parents) */
	 void sortMuPlusNuParetoMode();

	 /***************************************************************************/
	 /** @brief Selection according to the pareto tag in MUCOMMANU mode (i.e. not taking into account the parents) */
	 void sortMuCommaNuParetoMode();

	 /***************************************************************************/
	 /** @brief Determines whether the first individual dominates the second */
	 bool aDominatesB(std::shared_ptr<GParameterSet>, std::shared_ptr<GParameterSet>) const;

	 /***************************************************************************/
	 // Local data

	 sortingMode m_sorting_mode = DEFAULTSORTINGMODE; ///< The chosen sorting scheme

public:
	 /***************************************************************************/
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 virtual G_API_GENEVA bool modify_GUnitTests() override;

	 /***************************************************************************/
	 /** @brief Fills the collection with individuals */
	 G_API_GENEVA void fillWithObjects(const std::size_t& = 10);

	 /***************************************************************************/
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;

	 /***************************************************************************/
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GEvolutionaryAlgorithmT)

#endif /* GBASEEA_HPP_ */
