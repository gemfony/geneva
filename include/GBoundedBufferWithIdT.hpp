/**
 * @file GBoundedBufferWithIdT.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/date_time.hpp>

#ifndef GBOUNDEDBUFFERWITHIDT_HPP_
#define GBOUNDEDBUFFERWITHIDT_HPP_

// GenEvA header files go here

#include "GBoundedBufferT.hpp"

namespace Gem {
namespace Util {

/**
 * These typedefs are needed as it is not clear whether we're running on a 64 or 32 bit
 * machine. We do, however, be able to count as high as is possible for a given machine for
 * the port id.
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
	GBoundedBufferWithIdT() throw()
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
	explicit GBoundedBufferWithIdT(const std::size_t& capacity) throw()
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
	PORTIDTYPE getId() const throw(){
		return id_;
	}

	/***************************************************************/
	/*
	 * Allows to set the id_ once. Any subsequent calls to this
	 * function will have no effect.
	 *
	 * @param id The desired value of the id_ variable
	 */
	void setId(const PORTIDTYPE& id) throw(){
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
