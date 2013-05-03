/**
 * @file GBaseConsumer.hpp
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
#include <boost/thread.hpp>
#include <boost/type_traits.hpp>

#ifndef GCONSUMER_HPP_
#define GCONSUMER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GParserBuilder.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes that take
 * objects from GBrokerT and process them, either locally or remotely.
 * Derived classes such as the GAsioTCPConsumerT form the single point
 * of contact for remote clients. We do not want this class and its
 * derivatives to be copyable, hence we derive it from the
 * boost::noncopyable class. GBaseConsumer::process() is started in a separate
 * thread by the broker. GBaseConsumer::shutdown() is called by the broker
 * when the consumer is supposed to shut down.
 */
class GBaseConsumer
	:private boost::noncopyable
{
public:
	/** @brief The default constructor */
	GBaseConsumer();
	/** @brief The standard destructor */
	virtual ~GBaseConsumer();

	/** @brief The actual business logic */
	virtual void async_startProcessing() = 0;

	/** @brief Stop execution */
	virtual void shutdown();
	/** @brief Check whether the stop flag has been set */
	bool stopped() const;

	/** @brief A unique identifier for a given consumer */
	virtual std::string getConsumerName() const = 0;

	/** @brief Returns an indication whether full return can be expected from the consumer */
	virtual bool capableOfFullReturn() const;

   /** @brief Parses a given configuration file */
   void parseConfigFile(const std::string&);

protected:
   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

private:
	GBaseConsumer(const GBaseConsumer&); ///< Intentionally left undefined
	const GBaseConsumer& operator=(const GBaseConsumer&); ///< Intentionally left undefined

   mutable boost::shared_mutex stopMutex_; ///< Regulate access to the stop_ variable
   mutable bool stop_; ///< Set to true if we are expected to stop
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /*GCONSUMER_HPP_*/
