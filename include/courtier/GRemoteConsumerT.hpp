/**
 * @file GRemoteConsumerT.hpp
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

#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here


#ifndef GREMOTECONSUMER_HPP_
#define GREMOTECONSUMER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "courtier/GConsumer.hpp"

namespace Gem
{
namespace Courtier
{

/**************************************************************************************/
/**
 * This class implements the framework for all consumers that communicate beyond the
 * boundaries of the (server-)application, most notably consumers that communicate
 * via a network (sockets and MPI).
 */
template <class processable_type>
class GRemoteConsumerT
	: public Gem::Courtier::GConsumer // note: GConsumer is non-copyable
{
public:
	/**********************************************************************************/
	/**
	 * The default constructor
	 */
	GRemoteConsumerT();

	/**********************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GRemoteConsumerT();

	/**********************************************************************************/
	/**
	 * The actual business logic
	 */
	virtual void startProcessing() {

	}

protected:
private:
};

/**************************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GREMOTECONSUMER_HPP_ */
