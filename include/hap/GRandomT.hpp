/**
 * @file GRandom.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// Standard headers go here

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"


// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/function.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/concept_check.hpp>

#ifndef GRANDOMT_HPP_
#define GRANDOMT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Hap headers go here
#include "GRandomBaseT.hpp"

/****************************************************************************/

namespace Gem {
namespace Hap {

/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
/****************************************************************************/
/**
 * Access to different random number distributions, whose "raw" material is
 * produced in different ways. We only define the interface here. The actual
 * implementation can be found in the (partial) specializations of this class.
 */
template <
	Gem::Hap::gRandomTSpecialization s = Gem::Hap::RANDOMPROXY
  , typename fp_type = double
  , typename int_type = boost::int32_t
>
class GRandomT
	: public Gem::Hap::GRandomBaseT<fp_type, int_type>
{
	BOOST_CONCEPT_ASSERT((boost::SignedInteger<int_type>));

public:
	/** @brief The default constructor */
	GRandomT();
	/** @brief Initialization by seed */
	explicit GRandomT(const seed_type&);
	/** @brief The destructor */
	~GRandomT();

	/** @brief Production of uniformly distributed floating point numbers in [0,1[ */
	virtual fp_type uniform_01();
};

/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
/****************************************************************************/
/**
 * This specialization of the general GRandomT<> class retrieves random numbers
 * in batches from a global random number factory. The functions provided by
 * GRandomBase then produce different types of random numbers from this raw material.
 * As the class derives from boost::noncopyable, it is not possible to assign other
 * objects or use copy constructors.
 */
template <
	typename fp_type
  , typename int_type
>
class GRandomT<Gem::Hap::RANDOMPROXY, fp_type, int_type>
	: public Gem::Hap::GRandomBaseT<fp_type, int_type>
{
public:
	/************************************************************************/
	/**
	 * The standard constructor
	 */
	GRandomT()
		: Gem::Hap::GRandomBaseT<fp_type, int_type>()
		, currentPackageSize_(DEFAULTARRAYSIZE)
		, current01_(1) // position 0 holds the array size
		, grf_(GRANDOMFACTORY) // Make sure we have a local pointer to the factory
	{
		// Make sure we have a first random number package available
		getNewP01();
	}

	/************************************************************************/
	/**
	 * Initialization by seed. In the case of RANDOMPROXY this seed is not
	 * used, so this function behaves identical to the default constructor.
	 */
	explicit GRandomT(const seed_type& seed)
		: Gem::Hap::GRandomBaseT<fp_type, int_type>(seed)
		, currentPackageSize_(DEFAULTARRAYSIZE)
		, current01_(1) // position 0 holds the array size
		, grf_(GRANDOMFACTORY) // Make sure we have a local pointer to the factory
	{
		// Make sure we have a first random number package available
		getNewP01();
	}

	/************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRandomT()
	{
		p01_.reset();
		grf_.reset();
	}

	/************************************************************************/
	/**
	 * This function retrieves random number packages from a global
	 * factory and emits them one by one. Once a package has been fully
	 * used, it is discarded and a new package is obtained from the factory.
	 * Essentially this class thus acts as a random number proxy -- to the
	 * caller it appears as if random numbers are created locally. This function
	 * assumes that a valid container is already available.
	 */
	virtual fp_type uniform_01() {
		if (current01_ > currentPackageSize_) getNewP01();
		return p_raw_[current01_++];
	}

private:
	/************************************************************************/
	/**
	 * (Re-)Initialization of p01_. Checks that a valid GRandomFactory still
	 * exists, then retrieves a new container.
	 */
	void getNewP01() {
		if(grf_) {
#ifdef DEBUG
			boost::uint32_t nRetries = 0;
#endif /* DEBUG */

			// Try until a valid container has been received
			while (!(p01_ = grf_->new01Container())) {
#ifdef DEBUG
				nRetries++;
#endif /* DEBUG */

				// TODO: Configurable sleep here ?
			}

#ifdef DEBUG
			if(nRetries) {
				std::cout << "Info: Had to try " << nRetries+1 << " times to" << std::endl
						  << "retrieve a valid random number container." << std::endl;
			}
#endif /* DEBUG */

			current01_ = 1; // Position 0 is the array size
		}
		else {
			std::ostringstream error;
			error << "In GRandomT<RANDOMPROXY, fp_type, int_type>::getNewP01(): Error!" << std::endl
				  << "No connection to GRandomFactory object." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// We should now have a valid p01_ in any case
		p_raw_ = p01_.get();

		// Extract the array size for later use
		currentPackageSize_ = static_cast<std::size_t>(p_raw_[0]);
	}


	/************************************************************************/
	/** @brief Holds the container of uniform random numbers  Size is 16 bytes on a 64 bit system */
	boost::shared_array<fp_type> p01_;
	/** @brief A pointer to the content of p01_ for faster access.  Size is 8 byte on a 64 bit system */
	fp_type *p_raw_;
	/** @brief The package size, as obtained from the factory.  Size is 8 byte on a 64 bit system */
	std::size_t currentPackageSize_;
	/** @brief The current position in p01_.  Size is 8 byte on a 64 bit system */
	std::size_t current01_;

	/** @brief A local copy of the global GRandomFactory.  Size is 16 byte on a 64 bit system */
	boost::shared_ptr<Gem::Hap::GRandomFactory> grf_;
};

/** @brief Convenience typedef */
typedef GRandomT<Gem::Hap::RANDOMPROXY, double, boost::uint32_t> GRandom;

/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
/****************************************************************************/
/**
 * This specialization of the general GRandomT<> class produces random numbers
 * locally. The functions provided by GRandomBaseT<> then produce different types
 * of random numbers from this raw material. A seed can be provided either to
 * the constructor, or is taken from the global seed manager (recommended) in
 * case the default constructor is used.
 */
template <
	typename fp_type
  , typename int_type
>
class GRandomT<Gem::Hap::RANDOMLOCAL, fp_type, int_type>
	: public Gem::Hap::GRandomBaseT<fp_type, int_type>
{
public:
	/************************************************************************/
	/**
	 * The standard constructor
	 */
	GRandomT()
		: Gem::Hap::GRandomBaseT<fp_type, int_type>()
		, linCongr_(GRANDOMFACTORY->getSeed())
	{ /* nothing */ }

	/************************************************************************/
	/**
	 * Initialization by seed. Note that you need to ensure that two
	 * GRandomT<RANDOMLOCAL> objects do not use the same seed, when using
	 * this constructor.
	 */
	explicit GRandomT(const seed_type& seed)
		: Gem::Hap::GRandomBaseT<fp_type, int_type>(seed)
		, linCongr_(seed)
	{ /* nothing */ }

	/************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRandomT()
	{ /* nothing */ }

	/************************************************************************/
	/**
	 * This function produces uniform random numbers locally.
	 */
	virtual fp_type uniform_01() {
#ifdef DEBUG
		fp_type value =  boost::numeric_cast<fp_type>(linCongr_() - linCongr_.min()) / boost::numeric_cast<fp_type>(linCongr_.max() - linCongr_.min());
		assert(value>=fp_type(0.) && value<fp_type(1.));
		return value;
#else
		return static_cast<fp_type>(linCongr_() - linCongr_.min()) / static_cast<fp_type>(linCongr_.max() - linCongr_.min());
#endif
	}

private:
	/************************************************************************/
	/** @brief The actual generator for local random number creation */
	boost::rand48 linCongr_;
};

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMT_HPP_ */
