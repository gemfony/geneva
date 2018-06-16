/**
 * @file G_OA_LineSearchT.hpp
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
class GLineSearchT
	:public G_OptimizationAlgorithm_Base<Gem::Courtier::GBrokerExecutorT<GParameterSet>>
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
	 /** @brief The default constructor */
	 G_API_GENEVA GLineSearchT();
	 /** @brief A standard copy constructor */
	 G_API_GENEVA GLineSearchT(const GLineSearchT&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GLineSearchT();

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Returns information about the type of optimization algorithm */
	 G_API_GENEVA std::string getOptimizationAlgorithm() const override;

	 /** @brief Retrieves the number of processable items for the current iteration */
	 G_API_GENEVA std::size_t getNProcessableItems() const override;

	 /** @brief Returns the name of this optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmName() const override;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 virtual G_API_GENEVA void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override;

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another population */
	 G_API_GENEVA void load_(const GObject *) override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA GObject *clone_() const override;

	 /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	 G_API_GENEVA std::tuple<double, double> cycleLogic() override;
	 /** @brief Does some preparatory work before the optimization starts */
	 G_API_GENEVA void init() override;
	 /** @brief Does any necessary finalization work */
	 G_API_GENEVA void finalize() override;

	 /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	 G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

	 /** @brief Resizes the population to the desired level and does some error checks */
	 G_API_GENEVA void adjustPopulation() override;

	 /** @brief Triggers fitness calculation of a number of individuals */
	 G_API_GENEVA void runFitnessCalculation() override;

private:
	 /***************************************************************************/
	 // ...

public
	 /***************************************************************************/:
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GLineSearchT)
