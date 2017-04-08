/**
 * @file GBaseEA.hpp
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
 * main infrastructure for evolutionary algorithms (except those that deal with
 * multi-populations).
 */
class GBaseEA
	:public GParameterSetParChild
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GParameterSetParChild", boost::serialization::base_object<GParameterSetParChild>(*this))
		& BOOST_SERIALIZATION_NVP(smode_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GBaseEA();
	/** @brief A standard copy constructor */
	G_API_GENEVA GBaseEA(const GBaseEA&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBaseEA();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBaseEA& operator=(const GBaseEA&);

	/** @brief Checks for equality with another GBaseEA object */
	virtual G_API_GENEVA bool operator==(const GBaseEA&) const;
	/** @brief Checks for inequality with another GBaseEA object */
	virtual G_API_GENEVA bool operator!=(const GBaseEA&) const;

	/** @brief Returns information about the type of optimization algorithm */
	virtual G_API_GENEVA std::string getOptimizationAlgorithm() const override;

	/** @brief Set the sorting scheme for this population */
	G_API_GENEVA void setSortingScheme(sortingMode);
	/** @brief Retrieve the current sorting scheme for this population */
	G_API_GENEVA sortingMode getSortingScheme() const;

	/** @brief Extracts all individuals on the pareto front */
	G_API_GENEVA void extractCurrentParetoIndividuals(std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>>&);

	/** @brief Adds the individuals of this iteration to a priority queue */
	virtual G_API_GENEVA void updateGlobalBestsPQ(GParameterSetFixedSizePriorityQueue&) override;
	/** @brief Adds an iteration's individuals to a priority queue (which will be cleared) */
	virtual G_API_GENEVA void updateIterationBestsPQ(GParameterSetFixedSizePriorityQueue&) override;

	/** @brief Returns the name of this optimization algorithm */
	virtual G_API_GENEVA std::string getAlgorithmName() const override;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;
	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

protected:
	/***************************************************************************/
	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) override;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject *clone_() const override = 0;

	/** @brief Some error checks related to population sizes */
	virtual G_API_GENEVA void populationSanityChecks() const override;

	/** @brief Adapts all children of this population */
	virtual G_API_GENEVA void adaptChildren() override = 0;
	/** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	virtual G_API_GENEVA void runFitnessCalculation() override = 0;
	/** @brief Selects the best children of the population */
	virtual G_API_GENEVA void selectBest() override;

	/** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
	virtual G_API_GENEVA std::tuple<std::size_t,std::size_t> getEvaluationRange() const override;

	/** @brief Does some preparatory work before the optimization starts */
	virtual G_API_GENEVA void init() override;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() override;

	/** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	virtual G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

private:
	/***************************************************************************/
	/** @brief Selection according to the pareto tag in MUPLUSNU mode (i.e. taking into account the parents) */
	void sortMuPlusNuParetoMode();
	/** @brief Selection according to the pareto tag in MUCOMMANU mode (i.e. not taking into account the parents) */
	void sortMuCommaNuParetoMode();
	/** @brief Determines whether the first individual dominates the second */
	bool aDominatesB(std::shared_ptr<GParameterSet>, std::shared_ptr<GParameterSet>) const;

	/***************************************************************************/
	// Local data

	sortingMode smode_ = DEFAULTSMODE; ///< The chosen sorting scheme

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Fills the collection with individuals */
	G_API_GENEVA void fillWithObjects(const std::size_t& = 10);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseEA)

#endif /* GBASEEA_HPP_ */
