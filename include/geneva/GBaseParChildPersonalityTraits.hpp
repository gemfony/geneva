/**
 * @file GBaseParChildPersonalityTraits.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard headers go here

// Boost headers go here

#ifndef GBASEPARCHILDPERSONALITYTRAITS_HPP_
#define GBASEPARCHILDPERSONALITYTRAITS_HPP_

// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

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
     ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
        & BOOST_SERIALIZATION_NVP(parentCounter_)
        & BOOST_SERIALIZATION_NVP(popPos_)
        & BOOST_SERIALIZATION_NVP(parentId_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GBaseParChildPersonalityTraits();
   /** @brief The copy contructor */
   GBaseParChildPersonalityTraits(const GBaseParChildPersonalityTraits&);
   /** @brief The standard destructor */
   virtual ~GBaseParChildPersonalityTraits();

   /** @brief A standard assignment operator */
   const GBaseParChildPersonalityTraits& operator=(const GBaseParChildPersonalityTraits&);

   /** @brief Checks for equality with another GBaseParChildPersonalityTraits object */
   bool operator==(const GBaseParChildPersonalityTraits&) const;
   /** @brief Checks for inequality with another GBaseParChildPersonalityTraits object */
   bool operator!=(const GBaseParChildPersonalityTraits&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const OVERRIDE;

   /** @brief Marks an individual as a parent*/
   bool setIsParent();
   /** @brief Marks an individual as a child */
   bool setIsChild();

   /** @brief Checks whether this is a parent individual */
   bool isParent() const;
   /** @brief Retrieves the current value of the parentCounter_ variable */
   boost::uint32_t getParentCounter() const;

   /** @brief Sets the position of the individual in the population */
   void setPopulationPosition(const std::size_t&);
   /** @brief Retrieves the position of the individual in the population */
   std::size_t getPopulationPosition(void) const;

   /** @brief Stores the parent's id with this object */
   void setParentId(const std::size_t&);
   /** @brief Retrieves the parent id's value */
   std::size_t getParentId() const;
   /** @brief Checks whether a parent id has been set */
   bool parentIdSet() const;
   /** @brief Marks the parent id as unset */
   void unsetParentId();

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
   /** @brief Loads the data of another GBaseParChildPersonalityTraits object */
   virtual void load_(const GObject*) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const OVERRIDE;

private:
   /** @brief Allows populations to record how often an individual has been reelected as parent (0 if it is a child) */
   boost::uint32_t parentCounter_;
   /** @brief Stores the current position in the population */
   std::size_t popPos_;
   /** @brief The id of the old parent individual. This is intentionally a signed value. A negative value refers to an unset parent id */
   boost::int16_t parentId_;

public:
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual bool modify_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseParChildPersonalityTraits)

#endif /* GBASEPARCHILDPERSONALITYTRAITS_HPP_ */
