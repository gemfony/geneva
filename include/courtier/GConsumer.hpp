/**
 * @file GConsumer.hpp
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
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread/mutex.hpp>

#ifndef GCONSUMER_HPP_
#define GCONSUMER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here

namespace Gem
{
namespace Courtier
{

/**************************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes that take
 * objects from GBrokerT and process them, either locally or remotely.
 * Derived classes such as the GAsioTCPConsumerT form the single point
 * of contact for remote clients. We do not want this class and its
 * derivatives to be copyable, hence we derive it from the
 * boost::noncopyable class. GConsumer::process() is started in a separate
 * thread by the broker. GConsumer::shutdown() is called by the broker
 * when the consumer is supposed to shut down.
 */
class GConsumer
	:private boost::noncopyable
{
public:
	/** @brief The default constructor */
	GConsumer();
	/** @brief The standard destructor */
	virtual ~GConsumer();

	/** @brief The actual business logic */
	virtual void async_startProcessing() = 0;
	/** @brief To be called from GConsumer::process() */
	virtual void shutdown() = 0;

	/** @brief A unique identifier for a given consumer */
	virtual std::string getConsumerName() const = 0;

private:
	GConsumer(const GConsumer&); ///< Intentionally left undefined
	const GConsumer& operator=(const GConsumer&); ///< Intentionally left undefined
};

/**************************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /*GCONSUMER_HPP_*/
