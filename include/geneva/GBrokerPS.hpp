/**
 * @file GBrokerPS.hpp
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

#ifndef GBROKERPS_HPP_
#define GBROKERPS_HPP_


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "courtier/GBrokerConnector2T.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GBasePS.hpp"
#include "geneva/GParameterSet.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A networked version of the GBasePS class
 */
class GBrokerPS
   : public GBasePS
   , public Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   G_API_GENEVA void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePS)
         & make_nvp("GBrokerConnector2T_GParameterSet",
               boost::serialization::base_object<Gem::Courtier::GBrokerConnector2T<GParameterSet> >(*this));
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   G_API_GENEVA GBrokerPS();
   /** @brief A standard copy constructor */
   G_API_GENEVA GBrokerPS(const GBrokerPS&);
   /** @brief The destructor */
   virtual G_API_GENEVA ~GBrokerPS();

   /** @brief A standard assignment operator */
   G_API_GENEVA const GBrokerPS& operator=(const GBrokerPS&);

   /** @brief Checks for equality with another GBrokerPS object */
   G_API_GENEVA bool operator==(const GBrokerPS&) const;
   /** @brief Checks for inequality with another GBrokerPS object */
   G_API_GENEVA bool operator!=(const GBrokerPS&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Checks whether a given algorithm type likes to communicate via the broker */
   virtual G_API_GENEVA bool usesBroker() const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual G_API_GENEVA void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE;

   /** @brief Allows to assign a name to the role of this individual(-derivative) */
   virtual G_API_GENEVA std::string getIndividualCharacteristic() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
   /** @brief Loads the data of another population */
   virtual G_API_GENEVA void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual G_API_GENEVA GObject *clone_() const OVERRIDE;

   /** @brief Performs necessary initialization work */
   virtual G_API_GENEVA void init();
   /** @brief Does any necessary finalization work */
   virtual G_API_GENEVA void finalize();

   /** @brief Triggers fitness calculation of a number of individuals */
   virtual void runFitnessCalculation() OVERRIDE;

private:
   std::vector<boost::shared_ptr<GParameterSet> > oldWorkItems_; ///< Temporarily holds old returned work items

public:
   /***************************************************************************/
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerPS)

/******************************************************************************/

#endif /* GBROKERPS_HPP_ */
