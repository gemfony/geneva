/**
 * @file G_OA_ParChildT_PersonalityTraits.hpp
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

#ifndef GENEVA_LIBRARY_COLLECTION_G_OA_PARCHILDT_PT_HPP
#define GENEVA_LIBRARY_COLLECTION_G_OA_PARCHILDT_PT_HPP

// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to populations comprising parents and children
 */
class GBaseParChildPersonalityTraits
	:public GPersonalityTraits
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
		 & BOOST_SERIALIZATION_NVP(parentCounter_)
		 & BOOST_SERIALIZATION_NVP(popPos_)
		 & BOOST_SERIALIZATION_NVP(parentId_);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GBaseParChildPersonalityTraits();
	 /** @brief The copy contructor */
	 G_API_GENEVA GBaseParChildPersonalityTraits(const GBaseParChildPersonalityTraits&);
	 /** @brief The standard destructor */
	 virtual G_API_GENEVA ~GBaseParChildPersonalityTraits();

	 /** @brief The standard assignment operator */
	 G_API_GENEVA  GBaseParChildPersonalityTraits& operator=(const GBaseParChildPersonalityTraits&);

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Marks an individual as a parent*/
	 G_API_GENEVA bool setIsParent();
	 /** @brief Marks an individual as a child */
	 G_API_GENEVA bool setIsChild();

	 /** @brief Checks whether this is a parent individual */
	 G_API_GENEVA bool isParent() const;
	 /** @brief Retrieves the current value of the parentCounter_ variable */
	 G_API_GENEVA std::uint32_t getParentCounter() const;

	 /** @brief Sets the position of the individual in the population */
	 G_API_GENEVA void setPopulationPosition(const std::size_t&);
	 /** @brief Retrieves the position of the individual in the population */
	 G_API_GENEVA std::size_t getPopulationPosition(void) const;

	 /** @brief Stores the parent's id with this object */
	 G_API_GENEVA void setParentId(const std::size_t&);
	 /** @brief Retrieves the parent id's value */
	 G_API_GENEVA std::size_t getParentId() const;
	 /** @brief Checks whether a parent id has been set */
	 G_API_GENEVA bool parentIdSet() const;
	 /** @brief Marks the parent id as unset */
	 G_API_GENEVA void unsetParentId();

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;
	 /** @brief Retrieves the mnemonic of the optimization algorithm */
	 G_API_GENEVA std::string getMnemonic() const override;

protected:
	 /** @brief Loads the data of another GBaseParChildPersonalityTraits object */
	 G_API_GENEVA void load_(const GObject*) override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA GObject* clone_() const override;

private:
	 /** @brief Allows populations to record how often an individual has been reelected as parent (0 if it is a child) */
	 std::uint32_t parentCounter_ = 0;
	 /** @brief Stores the current position in the population */
	 std::size_t popPos_ = 0;
	 /** @brief The id of the old parent individual. This is intentionally a signed value. A negative value refers to an unset parent id */
	 std::int16_t parentId_ = -1;

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseParChildPersonalityTraits)

#endif // GENEVA_LIBRARY_COLLECTION_G_OA_PARCHILDT_PT_HPP
