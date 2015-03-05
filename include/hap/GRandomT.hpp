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

// Boost headers go here

#ifndef GRANDOMT_HPP_
#define GRANDOMT_HPP_

// Hap headers go here
#include "hap/GRandomBase.hpp"
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
template <Gem::Hap::RANDFLAVOURS s = Gem::Hap::RANDOMPROXY>
class GRandomT
 : public Gem::Hap::GRandomBase
{
public:
	/** @brief The default constructor */
   G_API_HAP GRandomT();
	/** @brief The destructor */
	virtual G_API_HAP ~GRandomT();

protected:
	 /** @brief Uniformly distributed double random numbers in the range [0,1[ */
	virtual G_API_HAP double dbl_random01();
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
template <>
class GRandomT<Gem::Hap::RANDOMPROXY>
	: public Gem::Hap::GRandomBase
{
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
   G_API_HAP GRandomT()
		: Gem::Hap::GRandomBase()
		, current01_(1) // position 0 holds the array size
		, grf_(GRANDOMFACTORY) // Make sure we have a local pointer to the factory
	{
		// Make sure we have a first random number package available
		getNewP01();
	}

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual G_API_HAP ~GRandomT()
	{
		p01_.reset();
		grf_.reset();
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
	virtual G_API_HAP double dbl_random01() {
		if (current01_ >= DEFAULTARRAYSIZE) {
		   getNewP01();
		}
		return p_raw_[current01_++];
	}

private:
	/***************************************************************************/
	/**
	 * (Re-)Initialization of p01_. Checks that a valid GRandomFactory still
	 * exists, then retrieves a new container.
	 */
	void getNewP01() {
		// Make sure we get rid of the old container
		p01_.reset();

#ifdef DEBUG
		if(!grf_) {
		   glogger
		   << "In GRandomT<RANDOMPROXY>::getNewP01(): Error!" << std::endl
         << "No connection to GRandomFactory object." << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

#ifdef DEBUG
		boost::uint32_t nRetries = 0;
#endif /* DEBUG */

		// Try until a valid container has been received
		while (!(p01_ = grf_->new01Container())) {
#ifdef DEBUG
		   nRetries++;
#endif /* DEBUG */
		}

#ifdef DEBUG
		if(nRetries>0) {
		   std::cout << "Info: Had to try " << nRetries << " times to retrieve a valid random number container." << std::endl;
		}
#endif /* DEBUG */

		current01_ = 0; // Position 0 is the array size

		// We should now have a valid p01_ in any case
		p_raw_ = p01_.get();
	}


	/***************************************************************************/
	/** @brief Holds the container of uniform random numbers  Size is 16 bytes on a 64 bit system */
	boost::shared_array<double> p01_;
	/** @brief A pointer to the content of p01_ for faster access.  Size is 8 byte on a 64 bit system */
	double *p_raw_;
	/** @brief The current position in p01_.  Size is 8 byte on a 64 bit system */
	std::size_t current01_;

	/** @brief A local copy of the global GRandomFactory.  Size is 16 byte on a 64 bit system */
	boost::shared_ptr<Gem::Hap::GRandomFactory> grf_;
};

/** @brief Convenience typedef */
typedef GRandomT<Gem::Hap::RANDOMPROXY> GRandom;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class produces random numbers
 * locally. The functions provided by GRandomBaseT<> then produce different types
 * of random numbers from this raw material. A seed can be provided either to
 * the constructor, or is taken from the global seed manager (recommended) in
 * case the default constructor is used.
 */
template <>
class GRandomT<Gem::Hap::RANDOMLOCAL>
	: public Gem::Hap::GRandomBase
{
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
   G_API_HAP GRandomT()
		: Gem::Hap::GRandomBase()
		, linCongr_(boost::numeric_cast<boost::random::rand48::result_type>(GRANDOMFACTORY->getSeed()))
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual G_API_HAP ~GRandomT()
	{ /* nothing */ }

protected:
	/***************************************************************************/
	/**
	 * This function produces uniform random numbers locally.
	 */
	virtual G_API_HAP double dbl_random01() {
		boost::rand48::result_type enumerator  = linCongr_() - (linCongr_.min)();
		boost::rand48::result_type denominator = (linCongr_.max)() - (linCongr_.min)();

		enumerator>0?enumerator-=1:enumerator=0;

#ifdef DEBUG
		double value =  boost::numeric_cast<double>(enumerator)/boost::numeric_cast<double>(denominator);
		assert(value>=double(0.) && value<double(1.));
		return value;
#else
		return static_cast<double>(enumerator)/static_cast<double>(denominator);
#endif
	}

private:
	/***************************************************************************/
	/** @brief The actual generator for local random number creation */
	boost::rand48 linCongr_;
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMT_HPP_ */
