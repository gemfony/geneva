/**
 * @file GBrokerEA.hpp
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

#ifndef GBROKEREA_HPP_
#define GBROKEREA_HPP_

// Geneva headers go here
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerConnector2T.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GBaseEA.hpp"
#include "geneva/GParameterSet.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadPool.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This population handles evolutionary algorithm-based optimization in environments
 * where communication between client and server is handled through Geneva's
 * broker infrastructure (libcourtier).
 * Note that serialization of this class makes sense only for backup-purposes,
 * in order to allow later, manual recovery. A broker object needs to be registered,
 * and serialization does not help here.
 * Serialization in a network context only happens below the level of this population,
 * it is itself usually not shipped over a network connection.
 */
class GBrokerEA
   : public GBaseEA
   , public Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   G_API void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseEA)
      & make_nvp("GBrokerConnector2T_GParameterSet", boost::serialization::base_object<Gem::Courtier::GBrokerConnector2T<GParameterSet> >(*this))
      & BOOST_SERIALIZATION_NVP(nThreads_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard constructor */
   G_API GBrokerEA();
   /** @brief A standard copy constructor */
   G_API GBrokerEA(const GBrokerEA&);
   /** @brief The standard destructor */
   virtual G_API ~GBrokerEA();

   /** @brief A standard assignment operator */
   G_API const GBrokerEA& operator=(const GBrokerEA&);

   /** @brief Checks for equality with another GBrokerEA object */
   G_API bool operator==(const GBrokerEA&) const;
   /** @brief Checks for inequality with another GBrokerEA object */
   G_API bool operator!=(const GBrokerEA&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual G_API boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Checks whether a given algorithm type likes to communicate via the broker */
   virtual G_API bool usesBroker() const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual G_API void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE;

   /** @brief Sets the maximum number of threads */
   G_API void setNThreads(boost::uint16_t);
   /** @brief Retrieves the maximum number of threads */
   G_API boost::uint16_t getNThreads() const ;

   /** @brief Allows to assign a name to the role of this individual(-derivative) */
   virtual G_API std::string getIndividualCharacteristic() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API std::string name() const OVERRIDE;

protected:
   /** @brief Loads the data of another GTransfer Population */
   virtual G_API void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep copy of this object */
   virtual G_API GObject *clone_() const OVERRIDE;

   /** @brief Adapt children in a serial manner */
   virtual G_API void adaptChildren() OVERRIDE;
   /** @brief Calculates the fitness of all required individuals */
   virtual G_API void runFitnessCalculation() OVERRIDE;
   /** @brief Selects new parents */
   virtual G_API void selectBest() OVERRIDE;

   /** @brief Performs any necessary initialization work before the start of the optimization cycle */
   virtual G_API void init() OVERRIDE;
   /** @brief Performs any necessary finalization work after the end of the optimization cycle */
   virtual G_API void finalize() OVERRIDE;

private:
   /***************************************************************************/

   boost::uint16_t nThreads_; ///< The number of threads
   boost::shared_ptr<Gem::Common::GThreadPool> tp_ptr_; ///< Temporarily holds a thread pool

   std::vector<boost::shared_ptr<GParameterSet> > oldWorkItems_; ///< Temporarily holds old returned work items

   /***************************************************************************/
   /**
    * A simple comparison operator that helps to sort individuals according to their
    * status as parents or children
    */
   class indParentComp {
   public:
      bool operator()(boost::shared_ptr<GParameterSet> x, boost::shared_ptr<GParameterSet> y) {
         return (x->getPersonalityTraits<GEAPersonalityTraits>()->isParent() > y->getPersonalityTraits<GEAPersonalityTraits>()->isParent());
      }
   };

   /***************************************************************************/
   /**
    * This simple operator helps to identify individuals that are parents from an
    * older iteration
    */
   class isOldParent {
   public:
      isOldParent(const boost::uint32_t current_iteration)
      : current_iteration_(current_iteration)
      { /* nothing */ }

      isOldParent(const isOldParent& cp)
      : current_iteration_(cp.current_iteration_)
      { /* nothing */ }

      bool operator()(boost::shared_ptr<GParameterSet> x) {
         if(x->getPersonalityTraits<GEAPersonalityTraits>()->isParent() && x->getAssignedIteration() != current_iteration_) {
            return true;
         } else {
            return false;
         }
      }

   private:
      isOldParent(); // Intentionally private and undefined

      boost::uint32_t current_iteration_;
   };

   /***************************************************************************/
   /**
    * This operator helps to remove individuals whose dirty flag is still set
    * after processing. This may happen in case of an incomplete return.
    */
   class hasDirtyFlagSet {
   public:
      bool operator()(boost::shared_ptr<GParameterSet> x) {
         if(x->isDirty()) {
            return true;
         } else {
            return false;
         }
      }
   };

   /***************************************************************************/

   /** @brief Fixes the population after a job submission */
   void fixAfterJobSubmission();

   /***************************************************************************/

public:
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual G_API bool modify_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual G_API void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual G_API void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerEA)

#endif /* GBROKEREA_HPP_ */
