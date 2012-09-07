/**
 * @file GBrokerEA.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard headers go here

// Boost headers go here

#ifndef GBROKEREA_HPP_
#define GBROKEREA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerConnectorT.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GBaseEA.hpp"
#include "geneva/GIndividual.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadPool.hpp"

namespace Gem
{
namespace Geneva
{
  /**********************************************************************************/
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
    , public Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseEA)
         & make_nvp("GBrokerConnectorT_GIndividual", boost::serialization::base_object<Gem::Courtier::GBrokerConnectorT<GIndividual> >(*this))
         & BOOST_SERIALIZATION_NVP(nThreads_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
	/** @brief The standard constructor */
    GBrokerEA();
    /** @brief A standard copy constructor */
    GBrokerEA(const GBrokerEA&);
    /** @brief The standard destructor */
    virtual ~GBrokerEA();

    /** @brief A standard assignment operator */
    const GBrokerEA& operator=(const GBrokerEA&);

	/** @brief Checks for equality with another GBrokerEA object */
	bool operator==(const GBrokerEA&) const;
	/** @brief Checks for inequality with another GBrokerEA object */
	bool operator!=(const GBrokerEA&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Checks whether a given algorithm type likes to communicate via the broker */
	virtual bool usesBroker() const;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
		, const bool& showOrigin
	);

	/** @brief Sets the maximum number of threads */
	void setNThreads(boost::uint16_t);
	/** @brief Retrieves the maximum number of threads */
	uint16_t getNThreads() const ;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual std::string getIndividualCharacteristic() const;

  protected:
    /** @brief Loads the data of another GTransfer Population */
    virtual void load_(const GObject *);
    /** @brief Creates a deep copy of this object */
    virtual GObject *clone_() const;

	/** @brief Adapt children in a serial manner */
	virtual void adaptChildren();
	/** @brief Evaluates all children (and possibly parents) of this population */
	virtual void evaluateChildren();
    /** @brief Selects new parents */
    virtual void selectBest();

	/** @brief Performs any necessary initialization work before the start of the optimization cycle */
	virtual void init();
	/** @brief Performs any necessary finalization work after the end of the optimization cycle */
	virtual void finalize();

  private:
    /*********************************************************************************/

	boost::uint16_t nThreads_; ///< The number of threads
	bool storedServerMode_; ///< Indicates whether an individual runs in server mode

	boost::shared_ptr<Gem::Common::GThreadPool> tp_; ///< Temporarily holds a thread pool

    /*********************************************************************************/
    /**
     * A simple comparison operator that helps to sort individuals according to their
     * status as parents or children
     */
    class indParentComp {
    public:
    	bool operator()(boost::shared_ptr<GIndividual> x, boost::shared_ptr<GIndividual> y) {
    		return x->getPersonalityTraits<GEAPersonalityTraits>()->isParent() > y->getPersonalityTraits<GEAPersonalityTraits>()->isParent();
    	}
    };

    /*********************************************************************************/
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

    	bool operator()(boost::shared_ptr<GIndividual> x) {
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

    /*********************************************************************************/

    /** @brief Fixes the population after a job submission */
    void fixAfterJobSubmission();

    /*********************************************************************************/

#ifdef GEM_TESTING
  public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GEM_TESTING */
  };

  /**********************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerEA)

#endif /* GBROKEREA_HPP_ */
