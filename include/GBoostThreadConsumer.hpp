/**
 * @file GBoostThreadConsumer.hpp
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

// Standard headers go here

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GBOOSTTHREADCONSUMER_HPP_
#define GBOOSTTHREADCONSUMER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


#include "GBrokerT.hpp"
#include "GConsumer.hpp"
#include "GIndividualBroker.hpp"
#include "GIndividual.hpp"
#include "GThreadGroup.hpp"

namespace Gem {
namespace GenEvA {

const boost::uint16_t DEFAULTGBTCMAXTHREADS = 4;

/***************************************************************/
/**
 * A derivative of GConsumer, that processes items in separate threads.
 * Objects of this class can exist alongside a networked consumer, as the broker
 * accepts more than one consumer. You can thus use this class to aid networked
 * optimization, if the server has spare CPU cores that would otherwise run idle.
 */
class GBoostThreadConsumer
	:public Gem::Util::GConsumer
{
public:
	/** @brief Standard constructor */
	GBoostThreadConsumer();
	/** @brief Standard destructor */
	virtual ~GBoostThreadConsumer();

	/** @brief Sets the maximum number of threads */
	void setMaxThreads(const std::size_t&);
	/** @brief Retrieves the maximum number of threads */
	std::size_t getMaxThreads() const ;

	/** @brief The actual business logic */
	virtual void process();
	/** @brief Called in order to terminate the consumer */
	virtual void shutdown();

private:
	GBoostThreadConsumer(const GBoostThreadConsumer&); ///< Intentionally left undefined
	const GBoostThreadConsumer& operator=(const GBoostThreadConsumer&); ///< Intentionally left undefined

	/** @brief Retrieves, processes and submits items */
	void processItems();

	std::size_t maxThreads_; ///< The maxumum number of allowed threads in the pool
	Gem::Util::GThreadGroup gtg_; ///< Holds the processing threads

	boost::mutex stopMutex_;
	bool stop_; ///< Set to true if we are expected to stop
};

/***************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOSTTHREADCONSUMER_HPP_ */
