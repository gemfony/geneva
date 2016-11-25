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
#include <type_traits>

// Boost headers go here

#ifndef GEVOLUTIONARYALGORITHMT_HPP_
#define GEVOLUTIONARYALGORITHMT_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GParameterSetParChild.hpp"
#include "geneva/GEAPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/**
 * The default sorting mode
 */
const sortingMode DEFAULTSMODE = sortingMode::MUCOMMANU_SINGLEEVAL;

/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. It provides the
 * main infrastructure for evolutionary algorithms. You may enable different
 * parallelization modes by choosing a different executor as template parameter.
 */
template <
	typename executor_type = Gem::Courtier::GBrokerExecutorT<GParameterSet>
>
class GEvolutionaryAlgorithmT : public GParameterSetParChild
{

	 static_assert(
		 std::enable_if<std::is_base_of<Gem::Courtier::GBaseExecutorT<GParameterSet>,executor_type>::value>::type
		 , "Error: executor_type is not derived from GBaseExecutorT<GParameterSet>"
	 );

	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GParameterSetParChild", boost::serialization::base_object<GParameterSetParChild>(*this))
		 & BOOST_SERIALIZATION_NVP(m_executor)
		 & BOOST_SERIALIZATION_NVP(m_smode)
	    & BOOST_SERIALIZATION_NVP(m_nThreads);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /**************************************************************************/
	 /**
	  * The default constructor, As we do not have any individuals yet, we set the population
	  * size, and number of parents to 0. It is the philosophy of this class not
	  * to provide constructors for each and every use case. Instead, you should set
	  * vital parameters, such as the population size or the parent individuals by hand.
	  */
	 GEvolutionaryAlgorithmT()
		 : GParameterSetParChild()
		 , nThreads_(boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
	 {
		 // Make sure we start with a valid population size if the user does not supply these values
		 this->setPopulationSizes(100, 1);
	 }

	 /**************************************************************************/
	 /**
	  * A standard copy constructor. Note that the generation number is reset to 0 and
	  * is not copied from the other object. We assume that a new optimization run will
	  * be started.
	  *
	  * @param cp Another GEvolutionaryAlgorithmT<executor_type> object
	  */
	 GEvolutionaryAlgorithmT(const GEvolutionaryAlgorithmT<executor_type>& cp)
		 : GParameterSetParChild(cp)
			, m_smode(cp.m_smode)
	 {
		 // Copying / setting of the optimization algorithm id is done by the parent class. The same
		 // applies to the copying of the optimization monitor.
	 }

	 /**************************************************************************/
	 /**
 	  * The standard destructor. All work is done in the parent class.
 	  */
	 virtual ~GEvolutionaryAlgorithmT()
	 { /* nothing */ }

	 /**************************************************************************/
	 /**
	  * The standard assignment operator
	  */
	 const GEvolutionaryAlgorithmT<executor_type>& operator=(const GEvolutionaryAlgorithmT<executor_type>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /**************************************************************************/
	 /**
	  * Checks for equality with another GEvolutionaryAlgorithmT object
	  *
	  * @param  cp A constant reference to another GEvolutionaryAlgorithmT object
	  * @return A boolean indicating whether both objects are equal
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

	 /**************************************************************************/
	 /**
	  * Checks for inequality with another GEvolutionaryAlgorithmT object
	  *
	  * @param  cp A constant reference to another GEvolutionaryAlgorithmT object
	  * @return A boolean indicating whether both objects are inequal
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

	 /**************************************************************************/
	 /**
	  * Returns information about the type of optimization algorithm. This function needs
	  * to be overloaded by the actual algorithms to return the correct type.
	  *
	  * @return The type of optimization algorithm
	  */
	 virtual std::string getOptimizationAlgorithm() const override {
		 return "PERSONALITY_EA";
	 }

	 /**************************************************************************/
	 /**
	  * Sets the sorting scheme. In MUPLUSNU_SINGLEEVAL, new parents will be selected from the entire
	  * population, including the old parents. In MUCOMMANU_SINGLEEVAL new parents will be selected
	  * from children only. MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation
	  * will also become a new parent (unless a better child was found). All other parents are
	  * selected from children only.
	  *
	  * @param smode The desired sorting scheme
	  */
	 void setSortingScheme(sortingMode smode) {
		 m_smode = smode;
	 }

	 /**************************************************************************/
	 /**
	  * Retrieves information about the current sorting scheme (see
	  * GEvolutionaryAlgorithmT<executor_type>::setSortingScheme() for further information).
	  *
	  * @return The current sorting scheme
	  */
	 sortingMode getSortingScheme() const {
		 return m_smode;
	 }

	 /**************************************************************************/
	 /**
	  * Extracts all individuals on the pareto front
	  */
	 void extractCurrentParetoIndividuals(
		 std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>>& paretoInds
	 ) {
		 // Make sure the vector is empty
		 paretoInds.clear();

		 // Copy the pareto individuals over
		 for(auto p_ind_ptr: *this) {
			 if(p_ind_ptr->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) {
				 paretoInds.push_back(p_ind_ptr);
			 }
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Adds the individuals of this iteration to a priority queue. The
	  * queue will be sorted by the first evaluation criterion of the individuals
	  * and may either have a limited or unlimited size, depending on user-
	  * settings. The procedure is different for pareto optimization, as we only
	  * want the individuals on the current pareto front to be added.
	  */
	 virtual void updateGlobalBestsPQ(
		 GParameterSetFixedSizePriorityQueue& bestIndividuals
	 ) override {
		 const bool REPLACE = true;
		 const bool CLONE = true;

#ifdef DEBUG
		 if(this->empty()) {
			glogger
			<< "In GEvolutionaryAlgorithmT::updateGlobalBestsPQ() :" << std::endl
			<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
			<< GEXCEPTION;
		}
#endif /* DEBUG */

		 switch (m_smode) {
			 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_SINGLEEVAL:
			 case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
			 case sortingMode::MUCOMMANU_SINGLEEVAL:
				 GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::updateGlobalBestsPQ(bestIndividuals);
				 break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_PARETO:
			 case sortingMode::MUCOMMANU_PARETO: {
				 // Retrieve all individuals on the pareto front
				 std::vector<std::shared_ptr < Gem::Geneva::GParameterSet>> paretoInds;
				 this->extractCurrentParetoIndividuals(paretoInds);

				 // We simply add all parent individuals to the queue. As we only want
				 // the individuals on the current pareto front, we replace all members
				 // of the current priority queue
				 bestIndividuals.add(paretoInds, CLONE, REPLACE);
			 }
				 break;

				 //----------------------------------------------------------------------------
			 default: {
				 glogger
					 << "In GEvolutionaryAlgorithmT::updateGlobalBestsPQ(): Error" << std::endl
					 << "Incorrect sorting scheme requested: " << m_smode << std::endl
					 << GEXCEPTION;
			 }
				 break;
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Adds the individuals of this iteration to a priority queue. The
	  * queue will be sorted by the first evaluation criterion of the individuals
	  * and will be cleared prior to adding the new individuals. This results in
	  * the best individuals of the current iteration.
	  */
	 virtual void updateIterationBestsPQ(
		 GParameterSetFixedSizePriorityQueue& bestIndividuals
	 ) override {
		 const bool CLONE = true;
		 const bool REPLACE = true;

#ifdef DEBUG
		 if(this->empty()) {
			glogger
			<< "GEvolutionaryAlgorithmT::updateIterationBestsPQ() :" << std::endl
			<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
			<< GEXCEPTION;
		}
#endif /* DEBUG */

		 switch (m_smode) {
			 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_SINGLEEVAL:
			 case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
			 case sortingMode::MUCOMMANU_SINGLEEVAL: {
				 GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::updateIterationBestsPQ(bestIndividuals);
			 } break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_PARETO:
			 case sortingMode::MUCOMMANU_PARETO: {
				 // Retrieve all individuals on the pareto front
				 std::vector<std::shared_ptr < Gem::Geneva::GParameterSet>> paretoInds;
				 this->extractCurrentParetoIndividuals(paretoInds);

				 // We simply add all parent individuals to the queue. As we only want
				 // the individuals on the current pareto front, we replace all members
				 // of the current priority queue
				 bestIndividuals.add(paretoInds, CLONE, REPLACE);
			 } break;

				 //----------------------------------------------------------------------------
			 default: {
				 glogger
					 << "In GEvolutionaryAlgorithmT::updateGlobalBestsPQ(): Error" << std::endl
					 << "Incorrect sorting scheme requested: " << m_smode << std::endl
					 << GEXCEPTION;
			 } break;
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Returns the name of this optimization algorithm
	  *
	  * @return The name assigned to this optimization algorithm
	  */
	 virtual std::string getAlgorithmName() const override {
		 return std::string("Evolutionary Algorithm");
	 }

	 /**************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override {
		 // Call our parent class'es function
		 GParameterSetParChild::addConfigurationOptions(gpb);
		 Gem::Courtier::GBrokerExecutorT<GParameterSet>::addConfigurationOptions(gpb);

		 // Add local data
		 gpb.registerFileParameter<std::uint16_t>(
			 "nEvaluationThreads" // The name of the variable
			 , 0 // The default value
			 , [this](std::uint16_t nt) { this->setNThreads(nt); }
		 )
		 << "The number of threads used to simultaneously adapt individuals." << std::endl
		 << "The value will also be used for the parallel evaluation, should the" << std::endl
	  	 << "multithreaded personality be used for this class. A value of 0 will" << std::endl
	    << "result in a number of threads equal to the number of processing cores" << std::endl
		 << "(possibly including virtual cores in the case of hyperthreading).";

		 gpb.registerFileParameter<sortingMode>(
			 "sortingMethod" // The name of the variable
			 , DEFAULTSMODE // The default value
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

	 /**************************************************************************/
	 /**
	  * Sets the number of threads this population uses for adaption and possibly
	  * evaluation of objects (depending on the executor type used for the instantiation
	  * of this class). If nThreads is set to 0, an attempt will be made to set the
	  * number of threads to the number of hardware threading units (e.g. number of
	  * cores or hyper-threading units).
	  *
	  * @param nThreads The number of threads this class uses for adaption and/or evaluation
	  */
	 void setNThreads(std::uint16_t nThreads) {
		 if (nThreads == 0) {
			 m_nThreads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
		 }
		 else {
			 m_nThreads = nThreads;
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Retrieves the number of threads this population uses for adaption and
	  * possibly evaluation (depending on the executor type used for the
	  * instantiation of this class)
	  *
	  * @return The maximum number of allowed threads
	  */
	 std::uint16_t getNThreads() const {
		 return m_nThreads;
	 }

	 /**************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 virtual std::string name() const override {
		 return std::string("GEvolutionaryAlgorithmT");
	 }

	 /**************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GObject object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GObject &cp
		 , const Gem::Common::expectation &e
		 , const double &limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GEvolutionaryAlgorithmT<executor_type> reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmT<executor_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GEvolutionaryAlgorithmT", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GParameterSetParChild>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(m_smode, p_load->m_smode), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

protected:
	 /**************************************************************************/
	 /**
	  * Loads the data of another GEvolutionaryAlgorithmT<executor_type> object, camouflaged as a GObject.
	  *
	  * @param cp A pointer to another GEvolutionaryAlgorithmT<executor_type> object, camouflaged as a GObject
	  */
	 virtual void load_(const GObject *cp) override {
		 // Check that we are dealing with a GEvolutionaryAlgorithmT<executor_type> reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmT<executor_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithmT<executor_type>>(cp, this);

		 // First load the parent class'es data ...
		 GParameterSetParChild::load_(cp);

		 // ... and then our own data
		 m_smode = p_load->m_smode;
	 }

	 /**************************************************************************/
	 /**
	  * Creates a deep copy of this object
	  *
	  * @return A deep copy of this object
	  */
	 virtual GObject *clone_() const override {
		 return new GEvolutionaryAlgorithmT<executor_type>(*this);
	 }

	 /**************************************************************************/
	 /**
	  * Some error checks related to population sizes, run during the init() phase.
	  *
	  * TODO: Why are PARETO modes missing here ?
	  */
	 virtual void populationSanityChecks() const override {
		 // First check that we have been given a suitable value for the number of parents.
		 // Note that a number of checks (e.g. population size != 0) has already been done
		 // in the parent class.
		 if (nParents_ == 0) {
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
		 if (
			 ((m_smode == sortingMode::MUCOMMANU_SINGLEEVAL || m_smode == sortingMode::MUNU1PRETAIN_SINGLEEVAL) && (popSize < 2 * nParents_)) ||
			 (m_smode == sortingMode::MUPLUSNU_SINGLEEVAL && popSize <= nParents_)
		 ) {
			 std::ostringstream error;
			 error
				 << "In GEvolutionaryAlgorithmT<executor_type>::populationSanityChecks() :" << std::endl
				 << "Requested size of population is too small :" << popSize << " " << nParents_ << std::endl
				 << "Sorting scheme is ";

			 switch (m_smode) {
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

	 /**************************************************************************/
	 /**
	  * Adapt all children in parallel. Evaluation is done in a seperate function (runFitnessCalculation).
	  */
	 // TODO: Add m_tp_ptr tp class
	 virtual void adaptChildren() override {
		 std::tuple<std::size_t, std::size_t> range = getAdaptionRange();
		 std::vector<std::shared_ptr < GParameterSet>> ::iterator
		 it;

		 for (it = data.begin() + std::get<0>(range); it != data.begin() + std::get<1>(range); ++it) {
			 m_tp_ptr->async_schedule([it]() { (*it)->adapt(); });
		 }

		 // Wait for all threads in the pool to complete their work
		 m_tp_ptr->wait();
	 }

	 /**************************************************************************/
	 /**
	  * We submit individuals to the executor and wait for processed items.
	  */
	 virtual void runFitnessCalculation() override {
		 //--------------------------------------------------------------------------------
		 // Start by marking the work to be done in the individuals.
		 // "range" will hold the start- and end-points of the range
		 // to be worked on
		 std::tuple<std::size_t, std::size_t> range = getEvaluationRange();


#ifdef DEBUG
		 // There should be no situation in which a "clean" child is submitted
		 // through this function. There MAY be situations, where in the first iteration
		 // parents are clean, e.g. when they were extracted from another optimization.
		 for(std::size_t i=this->getNParents(); i<this->size(); i++) {
			if(!this->at(i)->isDirty()) {
				glogger
				<< "In GEvolutionaryAlgorithmT<executor_type>::runFitnessCalculation(): Error!" << std::endl
				<< "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
				<< "but found \"clean\" individual in position " << i << std::endl
				<< GEXCEPTION;
			}
		}
#endif

		 //--------------------------------------------------------------------------------
		 // Now submit work items and wait for results.
		 Gem::Courtier::GBrokerExecutorT<GParameterSet>::workOn(
			 data
			 , range
			 , m_oldWorkItems
			 , true // Remove unprocessed items
		 );

		 //--------------------------------------------------------------------------------
		 // Now fix the population -- it may be smaller than its nominal size
		 fixAfterJobSubmission();
	 }

	 /**************************************************************************/
	 /**
	  * Choose new parents, based on the selection scheme set by the user.
	  */
	 virtual void selectBest() override {
#ifdef DEBUG
   // We require at this stage that at least the default number of
	// children is present. If individuals can get lost in your setting,
	// you must add mechanisms to "repair" the population before this
	// function is called
	if((data.size()-nParents_) < defaultNChildren_){
	   glogger
	   << "In GEvolutionaryAlgorithmT<executor_type>::select():" << std::endl
      << "Too few children. Got " << data.size()-getNParents() << "," << std::endl
      << "but was expecting at least " << getDefaultNChildren() << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

		 switch (m_smode) {
			 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_SINGLEEVAL: {
				 sortMuPlusNuMode();
			 }
				 break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUNU1PRETAIN_SINGLEEVAL: {
				 if (this->inFirstIteration()) {
					 sortMuPlusNuMode();
				 } else {
					 sortMunu1pretainMode();
				 }
			 }
				 break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUCOMMANU_SINGLEEVAL: {
				 if (this->inFirstIteration()) {
					 sortMuPlusNuMode();
				 } else {
					 sortMuCommaNuMode();
				 }
			 }
				 break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUPLUSNU_PARETO:
				 sortMuPlusNuParetoMode();
				 break;

				 //----------------------------------------------------------------------------
			 case sortingMode::MUCOMMANU_PARETO: {
				 if (this->inFirstIteration()) {
					 sortMuPlusNuParetoMode();
				 } else {
					 sortMuCommaNuParetoMode();
				 }
			 }
				 break;

				 //----------------------------------------------------------------------------
			 default: {
				 glogger
					 << "In GEvolutionaryAlgorithmT<executor_type>::selectBest(): Error" << std::endl
					 << "Incorrect sorting scheme requested: " << m_smode << std::endl
					 << GEXCEPTION;
			 }
				 break;
		 }

		 // Let parents know they are parents
		 markParents();

#ifdef DEBUG
		 // Make sure our population is not smaller than its nominal size -- this
		 // should have been taken care of in fixAfterJobSubmission() .
		 if(data.size() < getDefaultPopulationSize()) {
	   	glogger
			<< "In GBrokerEA::selectBest(): Error!" << std::endl
			<< "Size of population is smaller than expected: " << data.size() << " / " << getDefaultPopulationSize() << std::endl
			<< GEXCEPTION;
	 	 }
#endif /* DEBUG */

		 ////////////////////////////////////////////////////////////
		 // At this point we have a sorted list of individuals and can take care of
		 // too many members, so the next iteration finds a "standard" population. This
		 // function will remove the last items.
		 data.resize(getNParents() + getDefaultNChildren());

		 // Everything should be back to normal ...
	 }

	 /**************************************************************************/
	 /**
	  * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
	  * iteration and sorting scheme, the start point will be different. The end-point is not meant
	  * to be inclusive.
	  *
	  * @return The range inside which evaluation should take place
	  */
	 virtual std::tuple<std::size_t,std::size_t> getEvaluationRange() const override {
		 // We evaluate all individuals in the first iteration This happens so pluggable
		 // optimization monitors do not need to distinguish between algorithms
		 return std::tuple<std::size_t, std::size_t>(
			 inFirstIteration() ? 0 : getNParents(), data.size()
		 );
	 };

	 /**************************************************************************/
	 /**
	  * The function checks that the population size meets the requirements and does some
	  * tagging. It is called from within GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::optimize(), before the
	  * actual optimization cycle starts.
	  */
	 virtual void init() override {
		 // To be performed before any other action
		 GParameterSetParChild::init();
	 }

	 /**************************************************************************/
	 /**
	  * Does any necessary finalization work
	  */
	 virtual void finalize() override {
		 // Last action
		 GParameterSetParChild::finalize();
	 }

	 /**************************************************************************/
	 /**
	  * Retrieve a GPersonalityTraits object belonging to this algorithm
	  */
	 virtual std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override {
		 return std::shared_ptr<GEAPersonalityTraits>(new GEAPersonalityTraits());
	 }

private:
	 /***************************************************************************/
	 /**
	  * Selection according to the pareto tag, also taking into account the parents of a population (i.e. in MUPLUSNU
	  * mode). This is used in conjunction with multi-criterion optimization. See e.g.
	  * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
	  */
	 void sortMuPlusNuParetoMode() {
		 GEvolutionaryAlgorithmT<executor_type>::iterator it, it_cmp;

		 // We fall back to the single-eval MUPLUSNU mode if there is just one evaluation criterion
		 it = this->begin();
		 if (!(*it)->hasMultipleFitnessCriteria()) {
			 sortMuPlusNuMode();
		 }

		 // Mark all individuals as being on the pareto front initially
		 for (it = this->begin(); it != this->end(); ++it) {
			 (*it)->getPersonalityTraits<GEAPersonalityTraits>()->resetParetoTag();
		 }

		 // Compare all parameters
		 for (it = this->begin(); it != this->end(); ++it) {
			 for (it_cmp = it + 1; it_cmp != this->end(); ++it_cmp) {
				 // If we already know that this individual is *not*
				 // on the front we do not have to do any tests
				 if (!(*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) continue;

				 // Check if it dominates it_cmp. If so, mark it accordingly
				 if (aDominatesB(*it, *it_cmp)) {
					 (*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
				 }

				 // If a it dominated by it_cmp, we mark it accordingly and break the loop
				 if (aDominatesB(*it_cmp, *it)) {
					 (*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
					 break;
				 }
			 }
		 }

		 // At this point we have tagged all individuals according to whether or not they are
		 // on the pareto front. Lets sort them accordingly, bringing individuals with the
		 // pareto tag to the front of the collection.
		 sort(
			 data.begin()
			 , data.end()
			 , [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) {
				 return x->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront() > y->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront();
			 }
		 );

		 // Count the number of individuals on the pareto front
		 std::size_t nIndividualsOnParetoFront = 0;
		 for (it = this->begin(); it != this->end(); ++it) {
			 if ((*it)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) nIndividualsOnParetoFront++;
		 }

		 // If the number of individuals on the pareto front exceeds the number of parents, we
		 // do not want to introduce a bias by selecting only the first nParent individuals. Hence
		 // we randomly shuffle them. Note that not all individuals on the pareto front might survive,
		 // as subsequent iterations will only take into account parents for the reproduction step.
		 // If fewer individuals are on the pareto front than there are parents, then we want the
		 // remaining parent positions to be filled up with the non-pareto-front individuals with
		 // the best minOnly_fitness(0), i.e. with the best "master" fitness (transformed to take into
		 // account minimization and maximization).
		 if (nIndividualsOnParetoFront > getNParents()) {
			 // randomly shuffle pareto-front individuals to avoid a bias
			 std::random_shuffle(this->begin(), this->begin() + nIndividualsOnParetoFront);
		 } else if (nIndividualsOnParetoFront < getNParents()) {
			 // Sort the non-pareto-front individuals according to their master fitness
			 std::partial_sort(
				 data.begin() + nIndividualsOnParetoFront, data.begin() + nParents_, data.end(),
				 [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
					 return x->minOnly_fitness() < y->minOnly_fitness();
				 }
			 );
		 }

		 // Finally, we sort the parents only according to their master fitness. This is meant
		 // to give some sense to the value recombination scheme. It won't change much in case of the
		 // random recombination scheme.
		 std::sort(
			 data.begin(), data.begin() + nParents_,
			 [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				 return x->minOnly_fitness() < y->minOnly_fitness();
			 }
		 );
	 }

	 /**************************************************************************/
	 /**
	  * Selection according to the pareto tag, not taking into account the parents of a population (i.e. in MUCOMMANU
	  * mode). This is used in conjunction with multi-criterion optimization. See e.g.
	  * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
	  */
	 void sortMuCommaNuParetoMode() {
		 GEvolutionaryAlgorithmT<executor_type>::iterator it, it_cmp;

		 // We fall back to the single-eval MUCOMMANU mode if there is just one evaluation criterion
		 it = this->begin();
		 if (!(*it)->hasMultipleFitnessCriteria()) {
			 sortMuCommaNuMode();
			 return;
		 }

		 // Mark the last iterations parents as not being on the pareto front
		 for (it = this->begin(); it != this->begin() + nParents_; ++it) {
			 (*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
		 }

		 // Mark all children as being on the pareto front initially
		 for (it = this->begin() + nParents_; it != this->end(); ++it) {
			 (*it)->getPersonalityTraits<GEAPersonalityTraits>()->resetParetoTag();
		 }

		 // Compare all parameters of all children
		 for (it = this->begin() + nParents_; it != this->end(); ++it) {
			 for (it_cmp = it + 1; it_cmp != this->end(); ++it_cmp) {
				 // If we already know that this individual is *not*
				 // on the front we do not have to do any tests
				 if (!(*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) continue;

				 // Check if it dominates it_cmp. If so, mark it accordingly
				 if (aDominatesB(*it, *it_cmp)) {
					 (*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
				 }

				 // If a it dominated by it_cmp, we mark it accordingly and break the loop
				 if (aDominatesB(*it_cmp, *it)) {
					 (*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
					 break;
				 }
			 }
		 }

		 // At this point we have tagged all children according to whether or not they are
		 // on the pareto front. Lets sort them accordingly, bringing individuals with the
		 // pareto tag to the front of the population. Note that parents have been manually
		 // tagged as not being on the pareto front in the beginning of this function, so
		 // sorting the individuals according to the pareto tag will move former parents out
		 // of the parents section.
		 sort(
			 data.begin()
			 , data.end()
			 , [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) {
				 return x->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront() > y->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront();
			 }
		 );

		 // Count the number of individuals on the pareto front
		 std::size_t nIndividualsOnParetoFront = 0;
		 for (it = this->begin(); it != this->end(); ++it) {
			 if ((*it)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) nIndividualsOnParetoFront++;
		 }

		 // If the number of individuals on the pareto front exceeds the number of parents, we
		 // do not want to introduce a bias by selecting only the first nParent individuals. Hence
		 // we randomly shuffle them. Note that not all individuals on the pareto front might survive,
		 // as subsequent iterations will only take into account parents for the reproduction step.
		 // If fewer individuals are on the pareto front than there are parents, then we want the
		 // remaining parent positions to be filled up with the non-pareto-front individuals with
		 // the best minOnly_fitness(0), i.e. with the best "master" fitness, transformed to take into account
		 // minimization and maximization. Note that, unlike MUCOMMANU_SINGLEEVAL
		 // this implies the possibility that former parents are "elected" as new parents again. This
		 // might be changed in subsequent versions of Geneva (TODO).
		 if (nIndividualsOnParetoFront > getNParents()) {
			 // randomly shuffle pareto-front individuals to avoid a bias
			 std::random_shuffle(this->begin(), this->begin() + nIndividualsOnParetoFront);
		 } else if (nIndividualsOnParetoFront < getNParents()) {
			 // Sort the non-pareto-front individuals according to their master fitness
			 std::partial_sort(
				 data.begin() + nIndividualsOnParetoFront, data.begin() + nParents_, data.end(),
				 [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
					 return x->minOnly_fitness() < y->minOnly_fitness();
				 }
			 );
		 }

		 // Finally, we sort the parents only according to their master fitness. This is meant
		 // to give some sense to the value recombination scheme. It won't change much in case of the
		 // random recombination scheme.
		 std::sort(
			 data.begin(), data.begin() + nParents_,
			 [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				 return x->minOnly_fitness() < y->minOnly_fitness();
			 }
		 );
	 }

	 /**************************************************************************/
	 /**
	  * Determines whether the first individual dominates the second.
	  *
	  * @param a The individual that is assumed to dominate
	  * @param b The individual that is assumed to be dominated
	  * @return A boolean indicating whether the first individual dominates the second
	  */
	 bool aDominatesB(
		 std::shared_ptr < GParameterSet > a
		 , std::shared_ptr < GParameterSet > b
	 ) const {
		 std::size_t nCriteriaA = a->getNumberOfFitnessCriteria();

#ifdef DEBUG
		 std::size_t nCriteriaB = b->getNumberOfFitnessCriteria();
   if(nCriteriaA != nCriteriaB) {
      glogger
      << "In GEvolutionaryAlgorithmT<executor_type>::aDominatesB(): Error!" << std::endl
      << "Number of fitness criteria differ: " << nCriteriaA << " / " << nCriteriaB << std::endl
      << GEXCEPTION;
   }
#endif

		 for (std::size_t i = 0; i < nCriteriaA; i++) {
			 if (this->isWorse(a->transformedFitness(i), b->transformedFitness(i))) return false;
		 }

		 return true;
	 }

	 /***************************************************************************/
	 /**
 	  * Fixes the population after a job submission
     */
	 void fixAfterJobSubmission() {
		 std::size_t np = getNParents();
		 std::uint32_t iteration = getIteration();

		 // Remove parents from older iterations from old work items -- we do not want them.
		 // Note that "remove_if" simply moves items not satisfying the predicate to the end of the list.
		 // We thus need to explicitly erase these items. remove_if returns the iterator position right after
		 // the last item not satisfying the predicate.
		 m_oldWorkItems.erase(
			 std::remove_if(
				 m_oldWorkItems.begin()
				 , m_oldWorkItems.end()
				 , [&iteration](std::shared_ptr<GParameterSet> x) -> bool {
					 if(x->getPersonalityTraits<GEAPersonalityTraits>()->isParent() && x->getAssignedIteration() != iteration) {
						 return true;
					 } else {
						 return false;
					 }
				 }
			 )
			 , m_oldWorkItems.end()
		 );

		 // Make it known to remaining old individuals that they are now part of a new iteration
		 std::for_each(
			 m_oldWorkItems.begin()
			 , m_oldWorkItems.end()
			 , [iteration](std::shared_ptr <GParameterSet> p) { p->setAssignedIteration(iteration); }
		 );

		 // Make sure that parents are at the beginning of the array.
		 sort(
			 data.begin()
			 , data.end()
			 , [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) -> bool {
				 return (x->getPersonalityTraits<GEAPersonalityTraits>()->isParent() > y->getPersonalityTraits<GEAPersonalityTraits>()->isParent());
			 }
		 );

		 // Attach all old work items to the end of the current population and clear the array of old items
		 for(auto item: m_oldWorkItems) {
			 data.push_back(item);
		 }
		 m_oldWorkItems.clear();

		 // Add missing individuals, as clones of the last item
		 if (data.size() < getDefaultPopulationSize()) {
			 std::size_t fixSize = getDefaultPopulationSize() - data.size();
			 for (std::size_t i = 0; i < fixSize; i++) {
				 // This function will create a clone of its argument
				 this->push_back_clone(data.back());
			 }
		 }

		 // Mark the first nParents_ individuals as parents in the first iteration. We want to have a "sane" population.
		 if (inFirstIteration()) {
			 for (auto it = this->begin(); it != this->begin() + np; ++it) {
				 (*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsParent();
			 }
		 }

		 // We care for too many returned individuals in the selectBest() function. Older
		 // individuals might nevertheless have a better quality. We do not want to loose them.
	 }

	 /***************************************************************************/
	 // Local data

	 executor_type m_executor; ///< Takes care of the evaluation of objects

	 sortingMode m_smode = DEFAULTSMODE; ///< The chosen sorting scheme
	 std::uint16_t m_nThreads = DEFAULTNBOOSTTHREADS; ///< The number of threads, e.g. for the parallel adaption

	 std::shared_ptr<Gem::Common::GThreadPool> m_tp_ptr; ///< Temporarily holds a thread pool
	 std::vector<std::shared_ptr<GParameterSet>> m_oldWorkItems; ///< Temporarily holds old returned work items

public:
	 /**************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 // TODO: add GBrokerEA tests
	 virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING

		 bool result = false;

		 // Call the parent class'es function
		 if (GParameterSetParChild::modify_GUnitTests()) result = true;

		 if(sortingMode::MUPLUSNU_SINGLEEVAL == this->getSortingScheme()) {
		 	 this->setSortingScheme(sortingMode::MUCOMMANU_SINGLEEVAL);
		 } else {
			 this->setSortingScheme(sortingMode::MUPLUSNU_SINGLEEVAL);
		 }
		 result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GEvolutionaryAlgorithmT<executor_type>::modify_GUnitTests", "GEM_TESTING");
		 return false;
#endif /* GEM_TESTING */
	 }

	 /**************************************************************************/
	 /**
	  * Fills the collection with individuals.
	  *
	  * @param nIndividuals The number of individuals that should be added to the collection
	  */
	 void fillWithObjects(const std::size_t &nIndividuals = 10) {
#ifdef GEM_TESTING
		 // Clear the collection, so we can start fresh
		 BOOST_CHECK_NO_THROW(this->clear());

		 // Add some some
		 for (std::size_t i = 0; i < nIndividuals; i++) {
			this->push_back(std::shared_ptr<Gem::Tests::GTestIndividual1>(new Gem::Tests::GTestIndividual1()));
		 }

		 // Make sure we have unique data items
		 this->randomInit(activityMode::ALLPARAMETERS);

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GEvolutionaryAlgorithmT<executor_type>::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /**************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 GParameterSetParChild::specificTestsNoFailureExpected_GUnitTests();

		 //------------------------------------------------------------------------------

		 { // Call the parent class'es function
			 std::shared_ptr <GEvolutionaryAlgorithmT<executor_type>> p_test = this->clone<GEvolutionaryAlgorithmT<executor_type>>();

			 // Fill p_test with individuals
			 p_test->fillWithObjects();

			 // Run the parent class'es tests
			 p_test->GParameterSetParChild::specificTestsNoFailureExpected_GUnitTests();
		 }

		 //------------------------------------------------------------------------------

		 { // Check setting and retrieval of the population size and number of parents/childs
			 std::shared_ptr <GEvolutionaryAlgorithmT<executor_type>> p_test = this->clone<GEvolutionaryAlgorithmT<executor_type>>();

			 // Set the default population size and number of children to different numbers
			 for (std::size_t nChildren = 5; nChildren < 10; nChildren++) {
			 	 for (std::size_t nParents = 1; nParents < nChildren; nParents++) {
					 // Clear the collection
					 BOOST_CHECK_NO_THROW(p_test->clear());

					 // Add the required number of individuals
					 p_test->fillWithObjects(nParents + nChildren);

					 BOOST_CHECK_NO_THROW(p_test->setPopulationSizes(nParents + nChildren, nParents));

					 // Check that the number of parents is as expected
					 BOOST_CHECK_MESSAGE(p_test->getNParents() == nParents,
											  "p_test->getNParents() == " << p_test->getNParents()
											  << ", nParents = " << nParents
											  << ", size = " << p_test->size());

					 // Check that the actual number of children has the same value
					 BOOST_CHECK_MESSAGE(p_test->getNChildren() == nChildren,
											  "p_test->getNChildren() = " << p_test->getNChildren()
											  << ", nChildren = " << nChildren);
				 }
			}
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GEvolutionaryAlgorithmT<executor_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /**************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 GParameterSetParChild::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
		 condnotset("GEvolutionaryAlgorithmT<executor_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /**************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

// TODO Add KEY/IMPLEMENT for all executors, no longer abstract
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GEvolutionaryAlgorithmT)

#endif /* GEVOLUTIONARYALGORITHMT_HPP_ */
