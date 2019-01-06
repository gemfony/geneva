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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort

// Boost header files go here
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_ParameterScan_PersonalityTraits.hpp"
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Tests {

/******************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual1 :public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
			& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GTestIndividual1();
	/** @brief The copy constructor */
	G_API_GENEVA GTestIndividual1(const GTestIndividual1&) = default;
	/** @brief The standard destructor */
	G_API_GENEVA ~GTestIndividual1() override = default;

protected:
	/** @brief Loads the data of another GTestIndividual1 */
	G_API_GENEVA void load_(const GObject*) final;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GTestIndividual1>(
		GTestIndividual1 const &
		, GTestIndividual1 const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	G_API_GENEVA void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const final;

	/** @brief The actual fitness calculation takes place here. */
	G_API_GENEVA double fitnessCalculation() final;

	// Note: The following functions are, in the context of GTestIndividual1,
	// designed to mainly test parent classes

	/** @brief Applies modifications to this object. */
	G_API_GENEVA bool modify_GUnitTests_() override;
	/** @brief Performs self tests that are expected to succeed. */
	G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
	/** @brief Performs self tests that are expected to fail. */
	G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
	/** @brief Creates a deep clone of this object */
	G_API_GENEVA GObject* clone_() const final;

	/** @brief Adds a number of GDoubleObject objects to the individual */
	void G_API_GENEVA addGDoubleObjects_(const std::size_t&);
};

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Tests::GTestIndividual1)

