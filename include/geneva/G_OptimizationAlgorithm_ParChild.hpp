/**
 * @file G_OptimizationAlgorithm_ParChild.hpp
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

#ifndef G_OA_PARCHILDT_HPP_
#define G_OA_PARCHILDT_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <type_traits>
#include <tuple>

// Boost headers go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_ParChildT_PersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The GParChildT class adds the notion of parents and children to
 * the G_OptimizationAlgorithm_Base class. The evolutionary adaptation is realized
 * through the cycle of adaption, evaluation, and sorting, as defined in this
 * class.
 *
 * It forms the base class for either multi populations (i.e. evolutionary algorithms
 * that may act on other optimization algorithms (including themselves), or a hierarchy of
 * algorithms acting on parameter objects.
 *
 * Populations are collections of individuals, which themselves are objects
 * exhibiting at least the GParameterSet class' API, most notably the GParameterSet::fitness()
 * and GParameterSet::adapt() functions.
 *
 * In order to add parents to an instance of this class use the default constructor,
 * then add at least one GParameterSet-derivative to it, and call setPopulationSizes().
 * The population will then be "filled up" with missing individuals as required, before the
 * optimization starts.
 */
class G_OptimizationAlgorithm_ParChild
	: public G_OptimizationAlgorithm_Base
{
	 /////////////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OptimizationAlgorithm_Base", boost::serialization::base_object<G_OptimizationAlgorithm_Base>(*this))
		 & BOOST_SERIALIZATION_NVP(m_n_parents)
		 & BOOST_SERIALIZATION_NVP(m_recombination_method)
		 & BOOST_SERIALIZATION_NVP(m_default_n_children)
		 & BOOST_SERIALIZATION_NVP(m_growth_rate)
		 & BOOST_SERIALIZATION_NVP(m_max_population_size)
		 & BOOST_SERIALIZATION_NVP(m_amalgamationLikelihood);
	 }
	 /////////////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /** @brief The default constructor */
	 G_API_GENEVA G_OptimizationAlgorithm_ParChild();
	 /** @brief A standard copy constructor */
	 G_API_GENEVA G_OptimizationAlgorithm_ParChild(const G_OptimizationAlgorithm_ParChild& cp);
	 /** @brief The standard destructor */
	 virtual G_API_GENEVA ~G_OptimizationAlgorithm_ParChild() = default;

	 /** @broef Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

	 /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
	 G_API_GENEVA void resetToOptimizationStart() override;

	 /** @brief  Specifies the default size of the population plus the number of parents */
	 G_API_GENEVA void setPopulationSizes(std::size_t popSize, std::size_t nParents);

	 /** @brief Retrieve the number of parents as set by the user */
	 G_API_GENEVA std::size_t getNParents() const;
	 /** @brief Calculates the current number of children from the number of parents and the size of the vector. */
	 G_API_GENEVA std::size_t getNChildren() const;
	 /** @brief Retrieves the defaultNChildren_ parameter */
	 G_API_GENEVA std::size_t getDefaultNChildren() const;

	 /** @brief Retrieve the number of processible items in the current iteration. */
	 G_API_GENEVA std::size_t getNProcessableItems() const override;
	 /** @brief Lets the user set the desired recombination method */
	 G_API_GENEVA void setRecombinationMethod(duplicationScheme recombinationMethod);

	 /** @brief Retrieves the value of the recombinationMethod_ variable */
	 G_API_GENEVA duplicationScheme getRecombinationMethod() const;

	 /** @brief Adds the option to increase the population by a given amount per iteration */
	 G_API_GENEVA void setPopulationGrowth(
		 std::size_t growthRate
		 , std::size_t maxPopulationSize
	 );
	 /** @brief Allows to retrieve the growth rate of the population */
	 G_API_GENEVA std::size_t getGrowthRate() const;
	 /** @brief Allows to retrieve the maximum population size when growth is enabled */
	 G_API_GENEVA std::size_t getMaxPopulationSize() const;

	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual G_API_GENEVA void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 )  override;

	 /** @brief Allows to set the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
	 G_API_GENEVA void setAmalgamationLikelihood(double amalgamationLikelihood);
	 /** @brief Allows to retrieve the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
	 G_API_GENEVA double getAmalgamationLikelihood() const;

	 /** @brief This function assigns a new value to each child individual */
	 virtual G_API_GENEVA void doRecombine();

	 /** @brief Gives individuals an opportunity to update their internal structures */
	 G_API_GENEVA void actOnStalls() override;

	 /***************************************************************************/
	 /**
	  * Retrieves a specific parent individual and casts it to the desired type. Note that this
	  * function will only be accessible to the compiler if individual_type is a derivative of GParameterSet,
	  * thanks to the magic of the std::enable_if and type_traits.
	  *
	  * @param parent The id of the parent that should be returned
	  * @return A converted shared_ptr to the parent
	  */
	 template <typename parent_type>
	 std::shared_ptr<parent_type> getParentIndividual(
		 std::size_t parentId
		 , typename std::enable_if<std::is_base_of<GParameterSet, parent_type>::value>::type *dummy = nullptr
	 ){
#ifdef DEBUG
		 // Check that the parent id is in a valid range
		 if(parentId >= this->getNParents()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_OptimizationAlgorithm_ParChild::getParentIndividual<>() : Error" << std::endl
					 << "Requested parent id which does not exist: " << parentId << " / " << this->getNParents() << std::endl
			 );

			 // Make the compiler happy
			 return std::shared_ptr<parent_type>();
		 }
#endif /* DEBUG */

		 // Does error checks on the conversion internally
		 return Gem::Common::convertSmartPointer<GParameterSet, parent_type>(*(this->begin() + parentId));
	 }

	 /***************************************************************************/
	 /** @brief Returns the name of this optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmName() const override = 0;
	 /** @brief Returns information about the type of optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmPersonalityType() const override = 0;

	 /***************************************************************************/
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;

protected:
	 /***************************************************************************/
	 /** @brief Adapts all children of this population */
	 virtual G_API_GENEVA void adaptChildren() = 0;
	 /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	 G_API_GENEVA void runFitnessCalculation() override = 0;
	 /** @brief Choose new parents, based on the selection scheme set by the user */
	 virtual G_API_GENEVA void selectBest() = 0;
	 /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
	 virtual G_API_GENEVA std::tuple<std::size_t,std::size_t> getEvaluationRange() const = 0; // Depends on selection scheme
	 /** @brief Some error checks related to population sizes */
	 virtual G_API_GENEVA void populationSanityChecks() const = 0; // TODO: Take code from old init() function

	 /***************************************************************************/

	 /** @brief Loads the data of another GParChildT object, camouflaged as a GObject. */
	 G_API_GENEVA void load_(const GObject * cp) override;

	 /** @brief This function is called from G_OptimizationAlgorithm_Base::optimize() and performs the actual recombination */
	 virtual G_API_GENEVA void recombine();

	 /** @brief Retrieves the adaption range in a given iteration and sorting scheme. */
	 G_API_GENEVA std::tuple<std::size_t,std::size_t> getAdaptionRange() const;

	 /** @brief This helper function marks parents as parents and children as children. */
	 G_API_GENEVA void markParents();
	 /** @brief This helper function marks children as children */
	 G_API_GENEVA void markChildren();
	 /** @brief This helper function lets all individuals know about their position in the population. */
	 G_API_GENEVA void markIndividualPositions();

	 /** @brief This function implements the logic that constitutes evolutionary algorithms */
	 G_API_GENEVA std::tuple<double, double> cycleLogic() override;

	 /** @brief performs initialization work before the optimization loop starts */
	 G_API_GENEVA void init() override;
	 /** @brief Does any necessary finalization work atfer the optimization loop has ended */
	 G_API_GENEVA void finalize() override;

	 /***************************************************************************/
	 /** @brief The function checks that the population size meets the requirements and resizes the population to the appropriate size, if required. */
	 G_API_GENEVA void adjustPopulation() override;

	 /** @brief Increases the population size if requested by the user */
	 G_API_GENEVA void performScheduledPopulationGrowth();

	 /** @brief This function implements the RANDOMDUPLICATIONSCHEME scheme */
	 G_API_GENEVA void randomRecombine(std::shared_ptr<GParameterSet>& child);
	 /** @brief  This function implements the VALUEDUPLICATIONSCHEME scheme */
	 G_API_GENEVA void valueRecombine(
		 std::shared_ptr<GParameterSet>& p
		 , const std::vector<double>& threshold
	 );

	 /***************************************************************************/

	 std::size_t m_n_parents = DEFPARCHILDNPARENTS; ///< The number of parents
	 duplicationScheme m_recombination_method = duplicationScheme::DEFAULTDUPLICATIONSCHEME; ///< The chosen recombination method
	 std::size_t m_default_n_children = DEFPARCHILDNCHILDREN; ///< Expected number of children
	 std::size_t m_growth_rate = 0; ///< Specifies the amount of individuals added per iteration
	 std::size_t m_max_population_size = 0; ///< Specifies the maximum amount of individuals in the population if growth is enabled
	 double m_amalgamationLikelihood = DEFAULTAMALGAMATIONLIKELIHOOD; ///< Likelihood for children to be created by cross-over rather than "just" duplication (note that they may nevertheless be mutated)

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA GObject *clone_() const override = 0;

	 /***************************************************************************/

	 std::uniform_int_distribution<std::size_t> m_uniform_int_distribution; ///< Access to uniformly distributed random numbers

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::G_OptimizationAlgorithm_ParChild)

/******************************************************************************/

#endif /* G_OA_PARCHILDT_HPP_ */
