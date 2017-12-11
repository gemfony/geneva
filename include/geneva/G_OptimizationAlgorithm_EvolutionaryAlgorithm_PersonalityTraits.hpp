/**
 * @file G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.hpp
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

// Boost headers go here

#ifndef GENEVA_LIBRARY_COLLECTION_G_OA_EVOLUTIONARYALGORITHM_PT_HPP
#define GENEVA_LIBRARY_COLLECTION_G_OA_EVOLUTIONARYALGORITHM_PT_HPP

// Geneva headers go here
#include "geneva/G_OptimizationAlgorithm_ParChildT_PersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to evolutionary algorithms.
 */
class GEvolutionaryAlgorithm_PersonalityTraits
	: public GBaseParChildPersonalityTraits
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseParChildPersonalityTraits)
		 & BOOST_SERIALIZATION_NVP(isOnParetoFront_);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief An easy identifier for the class */
	 static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

	 /** @brief The default constructor */
	 G_API_GENEVA GEvolutionaryAlgorithm_PersonalityTraits();
	 /** @brief The copy contructor */
	 G_API_GENEVA GEvolutionaryAlgorithm_PersonalityTraits(const GEvolutionaryAlgorithm_PersonalityTraits&);
	 /** @brief The standard destructor */
	 virtual G_API_GENEVA ~GEvolutionaryAlgorithm_PersonalityTraits();

	 /** @brief The standard assignment operator */
	 G_API_GENEVA  GEvolutionaryAlgorithm_PersonalityTraits& operator=(const GEvolutionaryAlgorithm_PersonalityTraits&);

	 /** @brief Checks for equality with another GEAPersonalityTraits object */
	 G_API_GENEVA bool operator==(const GEvolutionaryAlgorithm_PersonalityTraits&) const;
	 /** @brief Checks for inequality with another GEAPersonalityTraits object */
	 G_API_GENEVA bool operator!=(const GEvolutionaryAlgorithm_PersonalityTraits&) const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Allows to check whether this individual lies on the pareto front (only yields useful results after pareto-sorting in EA) */
	 G_API_GENEVA bool isOnParetoFront() const;
	 /** @brief Allows to reset the pareto tag to "true" */
	 G_API_GENEVA void resetParetoTag();
	 /** @brief Allows to specify that this individual does not lie on the pareto front of the current iteration */
	 G_API_GENEVA void setIsNotOnParetoFront();

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;
	 /** @brief Retrieves the mnemonic of the optimization algorithm */
	 G_API_GENEVA std::string getMnemonic() const override;

protected:
	 /** @brief Loads the data of another GEAPersonalityTraits object */
	 G_API_GENEVA void load_(const GObject*) override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA GObject* clone_() const override;

private:
	 /** @brief Determines whether the individual lies on the pareto front */
	 bool isOnParetoFront_;

public:
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithm_PersonalityTraits)

#endif //GENEVA_LIBRARY_COLLECTION_G_OA_EVOLUTIONARYALGORITHM_PT_HPP

