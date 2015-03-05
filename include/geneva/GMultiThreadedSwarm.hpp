/**
 * @file GMultiThreadedSwarm.hpp
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

#ifndef GMULTITHREADEDSWARM_HPP_
#define GMULTITHREADEDSWARM_HPP_


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GThreadWrapper.hpp"
#include "common/GThreadPool.hpp"
#include "geneva/GBaseSwarm.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GObject.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A multi-threaded swarm based on GBaseSwarm. This version uses the
 * Boost.Threads library and a thread-pool library from http://threadpool.sf.net .
 */
class GMultiThreadedSwarm
	: public GBaseSwarm
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	G_API_GENEVA void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseSwarm)
		& BOOST_SERIALIZATION_NVP(nThreads_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor. Intentionally empty, as it is only needed for de-serialization purposes */
	G_API_GENEVA GMultiThreadedSwarm();
	/** @brief The default constructor */
	G_API_GENEVA GMultiThreadedSwarm(const std::size_t&, const std::size_t&);
	/** @brief A standard copy constructor */
	G_API_GENEVA GMultiThreadedSwarm(const GMultiThreadedSwarm&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GMultiThreadedSwarm();

	/** @brief A standard assignment operator */
	G_API_GENEVA const GMultiThreadedSwarm& operator=(const GMultiThreadedSwarm&);

	/** @brief Checks for equality with another GMultiThreadedSwarm object */
	G_API_GENEVA bool operator==(const GMultiThreadedSwarm&) const;
	/** @brief Checks for inequality with another GMultiThreadedSwarm object */
	G_API_GENEVA bool operator!=(const GMultiThreadedSwarm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&, const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Sets the maximum number of threads */
	G_API_GENEVA void setNThreads(boost::uint16_t);
	/** @brief Retrieves the maximum number of threads */
	G_API_GENEVA boost::uint16_t getNThreads() const ;

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

	/** @brief Does some preparatory work before the optimization starts */
	virtual G_API_GENEVA void init() OVERRIDE;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() OVERRIDE;

	/** @brief Updates the fitness of all individuals */
	virtual G_API_GENEVA void runFitnessCalculation() OVERRIDE;

	/***************************************************************************/
private:
	boost::uint16_t nThreads_; ///< The number of threads
	boost::shared_ptr<Gem::Common::GThreadPool> tp_ptr_; ///< Temporarily holds a thread pool

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING
// Tests of this class (and parent classes)
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GMultiThreadedSwarm has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
inline G_API_GENEVA boost::shared_ptr<Gem::Geneva::GMultiThreadedSwarm> TFactory_GUnitTests<Gem::Geneva::GMultiThreadedSwarm>() {
   boost::shared_ptr<Gem::Geneva::GMultiThreadedSwarm> p;
   BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GMultiThreadedSwarm>(new Gem::Geneva::GMultiThreadedSwarm(5,10)));
   return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMultiThreadedSwarm)

#endif /* GMULTITHREADEDSWARM_HPP_ */
