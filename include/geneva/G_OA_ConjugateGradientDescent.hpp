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

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GConjugateGradientDescent class implements a steepest descent algorithm. It is possible
 * to search for optima starting from several positions simultaneously. This class limits
 * itself to evaluation through the Courtier framework, i.e. all evaluation of individuals
 * is delegated to the Broker (which may in turn use other means, such as threads or
 * networked execution for the evaluation step).
 */
class GConjugateGradientDescent : public G_OptimizationAlgorithm_Base
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OptimizationAlgorithm_Base",
			 boost::serialization::base_object<G_OptimizationAlgorithm_Base>(*this));
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 // Defaulted functions

	 G_API_GENEVA GConjugateGradientDescent() = default;
	 G_API_GENEVA GConjugateGradientDescent(GConjugateGradientDescent const &) = default;
	 G_API_GENEVA GConjugateGradientDescent(GConjugateGradientDescent &&) = default;

	 G_API_GENEVA ~GConjugateGradientDescent() override = default;

	 G_API_GENEVA GConjugateGradientDescent& operator=(GConjugateGradientDescent const&) = delete;
	 G_API_GENEVA GConjugateGradientDescent& operator=(GConjugateGradientDescent &&) = delete;

	 /***************************************************************************/

protected:
	 /***************************************************************************/
	 // Virtual or overridden protected functions

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_GENEVA void addConfigurationOptions_ (
		 Gem::Common::GParserBuilder& gpb
	 ) override;
	 /** @brief Loads the data of another population */
	 G_API_GENEVA void load_(const GObject *) override;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GConjugateGradientDescent>(
		GConjugateGradientDescent const &
		, GConjugateGradientDescent const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	G_API_GENEVA void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Does some preparatory work before the optimization starts */
	G_API_GENEVA void init() override;
	/** @brief Does any necessary finalization work */
	G_API_GENEVA void finalize() override;

	/** @brief Applies modifications to this object. This is needed for testing purposes */
	G_API_GENEVA bool modify_GUnitTests_() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

	/***************************************************************************/

private:
	 /***************************************************************************/
	 // Virtual or overridden private functions

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA GObject *clone_() const override;

	 /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	 G_API_GENEVA std::tuple<double, double> cycleLogic_() override;
	 /** @brief Triggers fitness calculation of a number of individuals */
	 G_API_GENEVA void runFitnessCalculation_() override;

	 /** @brief Returns information about the type of optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmPersonalityType_() const override;
	 /** @brief Returns the name of this optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmName_() const override;

	 /** @brief Retrieves the number of processable items for the current iteration */
	 G_API_GENEVA std::size_t getNProcessableItems_() const override;

	 /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	 G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;
	 /** @brief Gives individuals an opportunity to update their internal structures */
	 G_API_GENEVA void actOnStalls_() override;

 	 /** @brief Resizes the population to the desired level and does some error checks */
	 G_API_GENEVA void adjustPopulation_() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GConjugateGradientDescent)

