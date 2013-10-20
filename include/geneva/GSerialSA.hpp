/**
 * @file GSerialSA.hpp
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


// Standard headers go here

// Boost headers go here

#ifndef GSERIALSA_HPP_
#define GSERIALSA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GBaseSA.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class adds a simple, serial adaptChildren() call to the GBaseSA class.
 */
class GSerialSA
   : public GBaseSA
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseSA);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GSerialSA();
   /** @brief A standard copy constructor */
   GSerialSA(const GSerialSA&);
   /** @brief The standard destructor */
   virtual ~GSerialSA();

   /** @brief Assignment operator */
   const GSerialSA& operator=(const GSerialSA&);

   /** @brief Checks for equality with another GSerialSA object */
   bool operator==(const GSerialSA&) const;
   /** @brief Checks for inequality with another GSerialSA object */
   bool operator!=(const GSerialSA&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const OVERRIDE;

   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) OVERRIDE;

   /** @brief Allows to assign a name to the role of this individual(-derivative) */
   virtual std::string getIndividualCharacteristic() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
   /** @brief Loads data from another object */
   virtual void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual GObject *clone_() const OVERRIDE;

   /** @brief Adapt children in a serial manner */
   virtual void adaptChildren() OVERRIDE;
   /** @brief Evaluates all children (and possibly parents) of this population */
   virtual void runFitnessCalculation() OVERRIDE;

   /** @brief Necessary initialization work before the start of the optimization */
   virtual void init() OVERRIDE;
   /** @brief Necessary clean-up work after the optimization has finished */
   virtual void finalize() OVERRIDE;

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSerialSA)

#endif /* GSERIALSA_HPP_ */
