/**
 * @file GBoundedBufferWithIdT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

// Standard header files go here
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/date_time.hpp>

#ifndef GBOUNDEDBUFFERWITHIDT_HPP_
#define GBOUNDEDBUFFERWITHIDT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA header files go here
#include "GBoundedBufferT.hpp"

namespace Gem {
namespace Util {

/**
 * These typedefs are needed as it is not clear whether we're running on a 64 or 32 bit
 * machine. We do want, however, to be able to count as high as is possible for a given machine
 * for the port id. This is a poor replacement for a guid, which is not officially part of
 * Boost yet.
 */
#ifdef BOOST_HAS_LONG_LONG
typedef boost::uint64_t PORTIDTYPE;
#else
typedef boost::uint32_t PORTIDTYPE;
#endif

/*****************************************************************************/
/**
 * A small helper class that adds a unique id to the GBoundedBufferT class. Note
 * that, once it has been set, it may not be modified anymore.
 */
template<typename T>
class GBoundedBufferWithIdT
	:public GBoundedBufferT<T>
{
public:
	/***************************************************************/
	/**
	 * The default constructor.
	 */
	GBoundedBufferWithIdT()
		:GBoundedBufferT<T>(),
		 id_(0),
		 idSet_(false)
	{ /* nothing */}

	/***************************************************************/
	/**
	 * A constructor that creates a buffer with custom size "capacity".
	 * It enforces a minimum buffer size of 1.
	 *
	 * @param capacity The desired size of the buffer
	 */
	explicit GBoundedBufferWithIdT(const std::size_t& capacity)
		:GBoundedBufferT<T>(capacity),
		 id_(0),
		 idSet_(false)
	{ /* nothing */}

	/***************************************************************/
	/**
	 * A standard destructor.
	 */
	virtual ~GBoundedBufferWithIdT()
	{ /* nothing */ }

	/***************************************************************/
	/*
	 * Retrieves the id_ variable.
	 *
	 * @return The value of the id_ variable
	 */
	PORTIDTYPE getId() const {
		return id_;
	}

	/***************************************************************/
	/*
	 * Allows to set the id_ once. Any subsequent calls to this
	 * function will have no effect.
	 *
	 * @param id The desired value of the id_ variable
	 */
	void setId(const PORTIDTYPE& id) {
		if(!idSet_){
			id_ = id;
			idSet_ = true;
		}
	}

private:
	/***************************************************************/
	volatile PORTIDTYPE id_; ///< An id that allows to identify this class
	volatile bool idSet_; ///< Allows control over whether the id has been set before
};


} /* namespace Util */
} /* namespace Gem */

#endif /* GBOUNDEDBUFFERWITHIDT_HPP_ */
