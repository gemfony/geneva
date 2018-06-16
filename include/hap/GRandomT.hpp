/**
 * @file GRandomT.hpp
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>
#include <thread>

// Boost headers go here

// Geneva headers go here
#include "hap/GRandomBase.hpp"
#include "hap/GRandomDefines.hpp"
#include "common/GLogger.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Access to different random number distributions, whose "raw" material is
 * produced in different ways. We only define the interface here. The actual
 * implementation can be found in the (partial) specializations of this class.
 */
template<Gem::Hap::RANDFLAVOURS s = Gem::Hap::RANDFLAVOURS::RANDOMPROXY>
class GRandomT
	: public Gem::Hap::GRandomBase {
public:
	 /** @brief The default constructor */
	 GRandomT();

	 /** @brief The destructor */
	 virtual ~GRandomT();

protected:
	 /** @brief Uniformly distributed integer random numbers */
	 virtual GRandomBase::result_type int_random();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class retrieves random numbers
 * in batches from a global random number factory. The functions provided by
 * GRandomBase then produce different types of random numbers from this raw material.
 * As the class derives from boost::noncopyable, it is not possible to assign other
 * objects or use copy constructors.
 */
template<>
class GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>
	: public Gem::Hap::GRandomBase
{
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
	GRandomT()
		: Gem::Hap::GRandomBase()
	  	, m_p( /* empty */ )
	  	, m_grf(GRANDOMFACTORY) // Make sure we have a local pointer to the factory
	{
		// Make sure we have a first random number package available
		getNewRandomContainer();
	}

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRandomT() {
		if (m_p) {
			m_grf->returnUsedPackage(std::move(m_p));
		}
		m_grf.reset();
	}

	/***************************************************************************/
	/**
	 * Retrieves the id of the currently running thread. This function exists
	 * mostly for debugging purposes
	 */
	decltype(std::this_thread::get_id()) getThreadId() const {
		return std::this_thread::get_id();
	}

protected:
	/***************************************************************************/
	/**
	 * This function retrieves random number packages from a global
	 * factory and emits them one by one. Once a package has been fully
	 * used, it is discarded and a new package is obtained from the factory.
	 * Essentially this class thus acts as a random number proxy -- to the
	 * caller it appears as if random numbers are created locally. This function
	 * assumes that a valid container is already available.
	 */
	virtual GRandomBase::result_type int_random() {
		if (m_p->empty()) {
			// Get rid of the old container ...
			m_grf->returnUsedPackage(std::move(m_p));
			// ... then get a new one
			getNewRandomContainer();
		}
		return m_p->next();
	}

private:
	/***************************************************************************/
	/**
	 * (Re-)Initialization of p_. Checks that a valid GRandomFactory still
	 * exists, then retrieves a new container.
	 */
	void getNewRandomContainer() {
		// Make sure we get rid of the old container
		// m_p.reset(); No longer needed with std::unique_ptr

#ifdef DEBUG
		if(!m_grf) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GRandomT<RANDOMPROXY>::getNewRandomContainer(): Error!" << std::endl
					<< "No connection to GRandomFactory object." << std::endl
			);
		}
#endif /* DEBUG */

#ifdef DEBUG
		std::uint32_t nRetries = 0;
#endif /* DEBUG */

		// Try until a valid container has been received. new01Container has
		// a timeout of DEFAULTFACTORYGETWAIT internally.
		while (!(m_p = m_grf->getNewRandomContainer())) {
#ifdef DEBUG
		   nRetries++;
#endif /* DEBUG */
		}

#ifdef DEBUG
		if(nRetries>1) {
		   std::cout << "Info: Had to try " << nRetries << " times to retrieve a valid random number container." << std::endl;
		}
#endif /* DEBUG */
	}


	/***************************************************************************/
	/** @brief Holds the container of uniform random numbers */
	std::unique_ptr<random_container> m_p;
	/** @brief A local copy of the global GRandomFactory */
	std::shared_ptr<Gem::Hap::GRandomFactory> m_grf;
};

/** @brief Convenience typedef */
using GRandom = GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class produces random numbers
 * locally. The functions provided by GRandomBase<> then produce different types
 * of random numbers from this raw material. A seed can be provided either to
 * the constructor, or is taken from the global seed manager (recommended) in
 * case the default constructor is used.
 */
template<>
class GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL>
	: public Gem::Hap::GRandomBase
{
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
	GRandomT()
		: Gem::Hap::GRandomBase()
	  	, m_rng(GRANDOMFACTORY->getSeed())
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRandomT()
	{ /* nothing */ }

protected:
	/***************************************************************************/
	/**
	 * This function produces uniform random numbers locally.
	 */
	virtual GRandomBase::result_type int_random() {
		return m_rng();
	}

private:
	/***************************************************************************/
	/** @brief The actual generator for local random number creation */
	G_BASE_GENERATOR m_rng;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

// Access to a thread_local copy of a random proxy object
// extern thread_local GRandomT<RANDFLAVOURS::RANDOMPROXY> GRANDOM_TLS;

// /** @brief Gives access to a thread-local copy of the GRandomT proxy */
// G_API_HAP GRandomT<RANDFLAVOURS::RANDOMPROXY>& randomProxy();

} /* namespace Hap */
} /* namespace Gem */

