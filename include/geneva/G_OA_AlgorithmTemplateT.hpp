/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <memory>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GAlgorithmTemplateT class template demonstrates which functions need to or
 * may be overloaded for a new optimization algorithm. Not all functions are needed
 * -- refer to the comments for each function below. Please note that you
 * do not need to implement this class as a template. For most users deriving
 * from G_OptimizationAlgorithm_Base<Gem::Courtier::GBrokerExecutor<GParameterSet>>
 * will suffice. This will simplify some of the code, as you do not need to
 * care for some of the C++ template oddities, such as having to preface iterators
 * over individuals with the "template" keyword. When splitting header and implementation
 * and compiling on Microsoft Windows, preface each header function with the G_API_GENEVA
 * macro.
 */
class G_OA_AlgorithmTemplateT
	: public G_OptimizationAlgorithm_Base
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OptimizationAlgorithm_Base_GBrokerExecutorT",
			 boost::serialization::base_object<G_OptimizationAlgorithm_Base>(*this));

		 // Add local variables to this function, if they need to be saved / loaded
		 // when dealing with checkpoint files of this algorithm.
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor.
	  *
	  * Add additional actions here, where needed. Note that, for serialization
	  * to work, a default constructor needs to be made available.
	  */
	 G_OA_AlgorithmTemplateT() = default;

	 /***************************************************************************/
	 /**
	  * A standard copy constructor.
	  *
	  * Add local data as needed.
	  *
	  * @param cp A copy of another G_OA_AlgorithmTemplateT object
	  */
	 G_OA_AlgorithmTemplateT(const G_OA_AlgorithmTemplateT& cp)
		 : G_OptimizationAlgorithm_Base(cp)
	 // copy local data here or fill out the function body, as needed
	 {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * The destructor.
	  *
	  * Add any necessary clean-up work.
	  */
	 virtual ~G_OA_AlgorithmTemplateT() {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * Loads the state of the class from disc.
	  *
	  * Unless you need to do anything special after loading a checkpoint,
	  * this function may be removed or kept as is. If you leave it in place
	  * and alter it, make sure that this function corresponds to the
	  * saveCheckpoint() function.
	  */
	 void loadCheckpoint(const bf::path& cpFile) override {
		 G_OptimizationAlgorithm_Base::loadCheckpoint(cpFile);
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another
	  * object of the same type.
	  *
	  * Leave the structure of this function intact, but add compare_t-calls for local
	  * data needing to be compared in tests. You can add POD-data directly, or objects
	  * which directly or indirectly derive from Gem::Common::GCommonInterfaceT<> (this
	  * class specifies the common interface for the majority of classes in the Geneva
	  * framework).
	  *
	  * @param cp A copy of another G_OA_AlgorithmTemplateT object, camouflaged as a GObject
	  * @param e The expectation for the comparison (see e.g. operator ==)
	  * @param limit Determines, below which difference the comparison of two doubles is considered as equality
	  */
	 virtual void compare(
		 const GObject& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmT *p_load
			 = Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithmT>(cp, this);

		 GToken token("GEvolutionaryAlgorithmT", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<G_OptimizationAlgorithm_Base>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 // compare_t(IDENTITY(some_local_pod_or_gci_derivative, p_load->some_local_pod_or_gci_derivative), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
 	  * Resets the class to the state before the optimize call. This will in
 	  * particular erase all individuals stored in this class and clear the list
 	  * of best individuals. Please note that a subsequent call to optimize will
 	  * result in an error, unless you add new individuals. The purpose of this
 	  * function is allow repeated optimization with the same settings, but different
 	  * starting points. Actual implementations of optimization algorithms derived
 	  * from this class may have to perform additional work by overloading (and
 	  * calling) this function.
 	  *
 	  * Add and reset any data that was changed during an optimize() call and
 	  * that you wish to be reset before calling optimize() again. Make sure the
 	  * call to the parent class'es reset function remains in place.
 	  */
	 virtual void resetToOptimizationStart() {
		 // Call our parent class'es function
		 G_OptimizationAlgorithm_Base::resetToOptimizationStart();
	 }

	 /***************************************************************************/
	 /**
	  * Returns information about the type of optimization algorithm.
	  *
	  * Change this to a unique string prefaced by "PERSONALITY_" and
	  * ending in a descriptive mnemonic for your algorithm
	  *
	  * @return The type of optimization algorithm
	  */
	 std::string getOptimizationAlgorithm() const override {
		 return std::string("PERSONALITY_TMPL");
	 }
	 ‚
	 /***************************************************************************/
	 /**
 	  * Returns the name of this optimization algorithm
 	  *
 	  * Change this to a descriptive string for your algorithm not used elsewhere
 	  *
 	  * @return The name assigned to this optimization algorithm
 	  */
	 std::string getAlgorithmName() const override {
		 return std::string("Optimization Algorithm Template");
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of processable items for the current iteration
	  *
	  * Alter this function to return the number of processible items in this class
	  */
	 std::size_t getNProcessableItems() const override {
		 return 0;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * Add local configuration options as needd (compare one of
	  * the other optimization algorithms for the syntax), but make sure
	  * to keep the call to our parent function in place.
	  */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override {
		 // Call our parent class'es function first
		 G_OptimizationAlgorithm_Base<Gem::Courtier::GBrokerExecutorT<GParameterSet>>::addConfigurationOptions(gpb);
	 }

	 /***************************************************************************/
	 /**
	  * Adds the individuals of this iteration to a priority queue. The
 	  * queue will be sorted by the first evaluation criterion of the individuals
 	  * and may either have a limited or unlimited size, depending on user-
 	  * settings.
 	  *
 	  * You can either remove this function or add some specific settings. Removing
 	  * it will result in the parent class'es function being used. This is the
 	  * most likely case, as this is for specialist usage only.
	  */
	 void updateGlobalBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) override {
		 G_OptimizationAlgorithm_Base<Gem::Courtier::GBrokerExecutorT<GParameterSet>>::updateGlobalBestsPQ(bestIndividuals);
	 }

	 /***************************************************************************/
	 /**
	  * Adds the individuals of this iteration to a priority queue. The
	  * queue will be sorted by the first evaluation criterion of the individuals
	  * and may either have a limited or unlimited size, depending on user-
	  * settings.
	  *
	  * You can either remove this function or add some specific settings. Removing
 	  * it will result in the parent class'es function being used. This is the
 	  * most likely case, as this is for specialist usage only.
	  */
	 void updateIterationBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) override {
		 G_OptimizationAlgorithm_Base<Gem::Courtier::GBrokerExecutorT<GParameterSet>>::updateIterationBestsPQ(bestIndividuals);
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another population
	  *
	  * Add any local data as needed, but leave the conversion and the call to the parent class'es
	  * function in place.
	  */
	 void load_(const GObject *) override {
		 // Check that we are dealing with a G_OA_AlgorithmTemplateT reference independent of this object and convert the pointer
		 const G_OA_AlgorithmTemplateT *p_load = Gem::Common::g_convert_and_compare<GObject, G_OA_AlgorithmTemplateT >(cp, this);

		 // First load the parent class'es data.
		 // This will also take care of copying all individuals.
		 G_OptimizationAlgorithm_Base<Gem::Courtier::GBrokerExecutorT<GParameterSet>>::load_(cp);

		 // ... and then our own data
		 // some_var = p_load->some_var;
	 }

	 /***************************************************************************/
	 /**
	  * Saves the state of the class to disc
	  *
	  * Unless you need to do anything special when saving a checkpoint,
	  * this function may be removed or kept as is. If you leave it in place
	  * and alter it, make sure that this function corresponds to the
	  * loadCheckpoint() function.
	  */
	 void saveCheckpoint(bf::path outputFile) const override {
		 G_OptimizationAlgorithm_Base::saveCheckpoint(outputFile);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found up to now (which is usually the best individual
	  * in the priority queue).
	  *
	  * You will not likely have to overload this function. Erase, leave in place as is, or modify
	  * (specialist's setting!), but make sure that the parent class'es function is called.
	  */
	 std::shared_ptr<GParameterSet> customGetBestGlobalIndividual() const override {
		 return G_OptimizationAlgorithm_Base::customGetBestGlobalIndividual();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found (equal to the content of
	  * the priority queue)
	  *
	  * You will not likely have to overload this function. Erase, leave in place as is, or modify
	  * (specialist's setting!), but make sure that the parent class'es function is called.
	  */
	 std::vector<std::shared_ptr<GParameterSet>> customGetBestGlobalIndividuals() const override {
		 return G_OptimizationAlgorithm_Base::customGetBestGlobalIndividuals();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found in the iteration (which is the best individual
	  * in the priority queue).
	  *
	  * You will not likely have to overload this function. Erase, leave in place as is, or modify
	  * (specialist's setting!), but make sure that the parent class'es function is called.
	  */
	 std::shared_ptr<GParameterSet> customGetBestIterationIndividual() override {
		 return G_OptimizationAlgorithm_Base::customGetBestIterationIndividual();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found in the iteration (equal to the content of
	  * the priority queue)
	  *
	  * You will not likely have to overload this function. Erase, leave in place as is, or modify
	  * (specialist's setting!), but make sure that the parent class'es function is called.
	  */
	 std::vector<std::shared_ptr<GParameterSet>> customGetBestIterationIndividuals() override {
		 return G_OptimizationAlgorithm_Base::customGetBestIterationIndividuals();
	 }

	 /***************************************************************************/
	 /**
	  * The actual business logic to be performed during each iteration. Returns the best achieved fitness.
	  *
	  * It is here that most of your optimization algorithm needs to be specified.
	  */
	 std::tuple<double, double> cycleLogic() override {
		 // nothing -- fill out as needed
	 };

	 /***************************************************************************/
	 /**
	  * It is possible for derived classes to specify in overloaded versions of this
	  * function under which conditions the optimization should be stopped. The
	  * function is called from G_OptimizationAlgorithm_Base::halt .
	  *
	  * @return boolean indicating that a stop condition was reached
	  */
	 virtual bool customHalt() const BASE {
		 // nothing, unless you have any stop criteria specific to this algorithm
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * Does some preparatory work right before the optimization starts.
	  *
	  * Add any custom initialization work here, but make sure the parent classes
	  * function is called first.
	  */
	 void init() override {
		 // Call the parent classes function
		 G_OptimizationAlgorithm_Base::init();
	 }

	 /***************************************************************************/
	 /**
	  * Does any necessary finalization work right after the optimization has ended.
	  *
	  * Add any custom finalization work here, but make sure the parent classes
	  * function is called last.
	  */
	 void finalize() override {
		 // Call the parent classes function
		 G_OptimizationAlgorithm_Base::finalize();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve a GPersonalityTraits object belonging to this algorithm
	  */
	 std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override {

	 }

	 /***************************************************************************/
	 /**
	  * Gives derived classes an opportunity to update their internal structures.
	  * NOTE that no action may be taken here that affects the "dirty" state
	  * of individuals. A typical usage scenario would be the update of the adaptor
	  * settings in evolutionary algorithms.
	  */
	 void actOnStalls() override {

	 }

	 /***************************************************************************/
	 /**
	  * Resizes the population to the desired level and does some error checks
	  */
	 void adjustPopulation() override {

	 }

	 /***************************************************************************/
	 /**
	  * Triggers fitness calculation of a number of individuals
	  */
	 void runFitnessCalculation() override {

	 }

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  *
	  * Add some clear-text description of this optimization algorithm
	  */
	 std::string name_() const override {
		 return std::string("algorithm template");
	 }

	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  *
	  * This function may remain unchanged. Do not remove!
	  */
	 GObject *clone_() const override {
		 return new G_OA_AlgorithmTemplateT(*this);
	 }

	 /***************************************************************************/
	 // Add any private data here

	 /***************************************************************************/

public
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  */
	 bool modify_GUnitTests() override {

	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {

	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {

	 }

	 /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


// TODO: Add specializations for different executors
// BOOST_CLASS_EXPORT_KEY(Gem::Geneva::G_OA_AlgorithmTemplateT)

