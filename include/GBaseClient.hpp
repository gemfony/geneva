/**
 * @file GBaseClient.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the Geneva library.
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

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GBASECLIENT_HPP_
#define GBASECLIENT_HPP_

// GenEvA headers go here

#include "GIndividual.hpp"
#include "GLogger.hpp"

namespace Gem
{
namespace GenEvA
{

/**
 * The serialization mode - XML or text
 */
const boost::uint8_t CLIENTXMLMODE=0;
const boost::uint8_t CLIENTTEXTMODE=1;

/*********************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes designed for client-side
 * network communication. Their task is to retrieve GMember descriptions from
 * the server over a given protocol (implemented in derived classes), to instantiate
 * the corresponding GMember object, to process it and to deliver the results to
 * the server.
 */
class GBaseClient
	:boost::noncopyable
{
public:
	/** @brief Constructor passes command line arguments
	 * to network implementation */
	GBaseClient();
	/** @brief Standard destructor */
	virtual ~GBaseClient();

	/** @brief The main loop */
	void run(void);

	/** @brief Allows to set a maximum number of processing steps */
	void setProcessMax(boost::uint32_t processMax);
	/** @brief Retrieves the value of the _processMax variable */
	boost::uint32_t getProcessMax() const;

	/** @brief Sets the maximum allowed processing time */
	void setMaxTime(boost::posix_time::time_duration);
	/** @brief Retrieves the maximum allowed processing time */
	boost::posix_time::time_duration getMaxTime();

	/** @brief Initializes the networking implementation */
	virtual void init(){}; // To be called from derived classes' constructor
	/** @brief Shuts down the network implementation */
	virtual void finally(){}; // To be called from derived classes' destructor

protected:
	/** @brief One-time data retrieval, processing and result submission */
	bool process();

	/** @brief Retrieve work items from the server. To be defined by derived classes. */
	virtual bool retrieve(std::string&) = 0;

	/** @brief Submit processed items to the server. To be defined by derived classes. */
	virtual bool submit(const std::string&, const std::string&, const std::string&, const std::string&) = 0;

	/** @brief Custom halt condition for processing */
	virtual bool customHalt(){return false;};

private:
	/** @brief halt condition for further processing */
	bool halt();

	GBaseClient(const GBaseClient&); ///< Intentionally left undefined
	const GBaseClient& operator=(const GBaseClient&);  ///< Intentionally left undefined

	boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization

	boost::uint32_t processed_;
	boost::uint32_t processMax_; ///< The maximum number of items to process
};

/*********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBASECLIENT_HPP_ */
