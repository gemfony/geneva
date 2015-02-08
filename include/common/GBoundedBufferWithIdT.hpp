/**
 * @file GBoundedBufferWithIdT.hpp
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

// Standard header files go here
#include <sstream>

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/date_time.hpp>

#ifndef GBOUNDEDBUFFERWITHIDT_HPP_
#define GBOUNDEDBUFFERWITHIDT_HPP_

// Geneva header files go here
#include "common/GBoundedBufferT.hpp"

namespace Gem {
namespace Common {

/**
 * These typedefs are needed as it is not clear whether we're running on a 64 or 32 bit
 * machine. We do want, however, to be able to count as high as is possible for a given machine
 * for the port id. This is a poor replacement for a guid, which was not yet officially part of
 * Boost, when this class was created.
 */
#ifdef BOOST_HAS_LONG_LONG
typedef boost::uint64_t PORTIDTYPE;
#else
typedef boost::uint32_t PORTIDTYPE;
#endif

/******************************************************************************/
/**
 * A small helper class that adds a unique id to the GBoundedBufferT class. Note
 * that, once it has been set, it may not be modified anymore.
 *
 * TODO: Move to Boost's unique identifier ?
 */
template<typename T>
class G_API GBoundedBufferWithIdT
	:public Gem::Common::GBoundedBufferT<T>
{
public:
	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GBoundedBufferWithIdT()
		: Gem::Common::GBoundedBufferT<T>()
		, id_(0)
		, idSet_(false)
	{ /* nothing */}

	/***************************************************************************/
	/**
	 * A constructor that creates a buffer with custom size "capacity".
	 * It enforces a minimum buffer size of 1.
	 *
	 * @param capacity The desired size of the buffer
	 */
	explicit GBoundedBufferWithIdT(const std::size_t& capacity)
		: Gem::Common::GBoundedBufferT<T>(capacity)
		, id_(0)
		, idSet_(false)
	{ /* nothing */}

	/***************************************************************************/
	/**
	 * A standard destructor.
	 */
	virtual ~GBoundedBufferWithIdT()
	{ /* nothing */ }

	/***************************************************************************/
	/*
	 * Retrieves the id_ variable.
	 *
	 * @return The value of the id_ variable
	 */
	PORTIDTYPE getId() const {
		return id_;
	}

	/***************************************************************************/
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
	/***************************************************************************/
	volatile PORTIDTYPE id_; ///< An id that allows to identify this class
	volatile bool idSet_; ///< Allows control over whether the id has been set before
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GBOUNDEDBUFFERWITHIDT_HPP_ */
