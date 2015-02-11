/**
 * @file GMPEAPersonalityTraits.hpp
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

#ifndef GMPEAPERSONALITYTRAITS_HPP_
#define GMPEAPERSONALITYTRAITS_HPP_


// Geneva headers go here
#include "geneva/GBaseParChildPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class allows to add variables and functions to GPersonalityTraits that
 * are specific to evolutionary algorithms in the multi-population form. Note that
 * at the current time this class adds no additional data. Since each optimization
 * algorithm needs its own personality type, though, we provide this default
 * implementation.
 */
class G_API GMPEAPersonalityTraits
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
   /** @brief The default constructor */
   GMPEAPersonalityTraits();
   /** @brief The copy contructor */
   GMPEAPersonalityTraits(const GMPEAPersonalityTraits&);
   /** @brief The standard destructor */
   virtual ~GMPEAPersonalityTraits();

   /** @brief A standard assignment operator */
   const GMPEAPersonalityTraits& operator=(const GMPEAPersonalityTraits&);

   /** @brief Checks for equality with another GMPEAPersonalityTraits object */
   bool operator==(const GMPEAPersonalityTraits&) const;
   /** @brief Checks for inequality with another GMPEAPersonalityTraits object */
   bool operator!=(const GMPEAPersonalityTraits&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
   /** @brief Loads the data of another GMPEAPersonalityTraits object */
   virtual void load_(const GObject*) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const OVERRIDE;

private:

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMPEAPersonalityTraits)

#endif /* GMPEAPERSONALITYTRAITS_HPP_ */




