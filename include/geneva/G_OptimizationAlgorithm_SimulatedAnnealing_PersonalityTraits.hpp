/**
 * @file G_OA_SimulatedAnnealing_PersonalityTraits.hpp
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
#ifndef GENEVA_LIBRARY_COLLECTION_G_OA_SIMULATEDANNEALING_PT_HPP
#define GENEVA_LIBRARY_COLLECTION_G_OA_SIMULATEDANNEALING_PT_HPP

// Geneva headers go here
#include "geneva/G_OA_ParChildT_PersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to add variables and functions to GPersonalityTraits that
 * are specific to simulated annealing. Note that at the current time this class
 * adds no additional data. Since each optimization algorithm needs its own
 * personality type, though, we provide this default implementation. The base
 * class is the same as for evolutionary algorithms, as in Geneva Simulated Annealing
 * uses the same framework.
 */
class GSimulatedAnnealing_PersonalityTraits
	: public GBaseParChildPersonalityTraits
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;
		 ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseParChildPersonalityTraits);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief An easy identifier for the class */
	 static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

	 /** @brief The default constructor */
	 G_API_GENEVA GSimulatedAnnealing_PersonalityTraits();
	 /** @brief The copy contructor */
	 G_API_GENEVA GSimulatedAnnealing_PersonalityTraits(const GSimulatedAnnealing_PersonalityTraits&);
	 /** @brief The standard destructor */
	 virtual G_API_GENEVA ~GSimulatedAnnealing_PersonalityTraits();

	 /** @brief The standard assignment operator */
	 G_API_GENEVA const GSimulatedAnnealing_PersonalityTraits& operator=(const GSimulatedAnnealing_PersonalityTraits&);

	 /** @brief Checks for equality with another GSAPersonalityTraits object */
	 G_API_GENEVA bool operator==(const GSimulatedAnnealing_PersonalityTraits&) const;
	 /** @brief Checks for inequality with another GSAPersonalityTraits object */
	 G_API_GENEVA bool operator!=(const GSimulatedAnnealing_PersonalityTraits&) const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Emits a name for this class / object */
	 virtual G_API_GENEVA std::string name() const override;
	 /** @brief Retrieves the mnemonic of the optimization algorithm */
	 virtual G_API_GENEVA std::string getMnemonic() const override;

protected:
	 /** @brief Loads the data of another GSAPersonalityTraits object */
	 virtual G_API_GENEVA void load_(const GObject*) override;
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_GENEVA GObject* clone_() const override;

public:
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 virtual G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSimulatedAnnealing_PersonalityTraits)

#endif //GENEVA_LIBRARY_COLLECTION_G_OA_SIMULATEDANNEALING_PT_HPP
