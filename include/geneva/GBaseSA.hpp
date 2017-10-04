/**
 * @file GBaseSA.hpp
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
#include <tuple>

// Boost headers go here

#ifndef GBASESA_HPP_
#define GBASESA_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GParameterSetParChild.hpp"
#include "geneva/GSAPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. The class adds
 * an infrastructure for simulated annealing (Geneva-style, i.e. with larger populations).
 */
class GBaseSA
	:public GParameterSetParChild
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GParameterSetParChild", boost::serialization::base_object<GParameterSetParChild >(*this))
		& BOOST_SERIALIZATION_NVP(t0_)
		& BOOST_SERIALIZATION_NVP(t_)
		& BOOST_SERIALIZATION_NVP(alpha_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GBaseSA();
	/** @brief A standard copy constructor */
	G_API_GENEVA GBaseSA(const GBaseSA&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBaseSA();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBaseSA& operator=(const GBaseSA&);


	/** @brief Checks for equality with another GBaseSA object */
	virtual G_API_GENEVA bool operator==(const GBaseSA&) const;
	/** @brief Checks for inequality with another GBaseSA object */
	virtual G_API_GENEVA bool operator!=(const GBaseSA&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Returns information about the type of optimization algorithm */
	virtual G_API_GENEVA std::string getOptimizationAlgorithm() const override;

	/** @brief Returns the name of this optimization algorithm */
	virtual G_API_GENEVA std::string getAlgorithmName() const override;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override;

	/** @brief Determines the strength of the temperature degradation */
	G_API_GENEVA void setTDegradationStrength(double);
	/** @brief Retrieves the temperature degradation strength */
	G_API_GENEVA double getTDegradationStrength() const;
	/** @brief Sets the start temperature */
	G_API_GENEVA void setT0(double);
	/** @brief Retrieves the start temperature */
	G_API_GENEVA double getT0() const;
	/** @brief Retrieves the current temperature */
	G_API_GENEVA double getT() const;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

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
	/** @brief Evaluates all children (and possibly parents) of this population */
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
	/** Performs a simulated annealing style sorting and selection */
	void sortSAMode();
	/** @brief Calculates the Simulated Annealing probability for a child to replace a parent */
	double saProb(const double&, const double&);
	/** @brief Updates the temperature (used for simulated annealing) */
	void updateTemperature();

	double t0_ = SA_T0; ///< The start temperature, used in simulated annealing
	double t_ = t0_; ///< The current temperature, used in simulated annealing
	double alpha_ = SA_ALPHA; ///< A constant used in the cooling schedule in simulated annealing

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseSA)

#endif /* GBASESA_HPP_ */
