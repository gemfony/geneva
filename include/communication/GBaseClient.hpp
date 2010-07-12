/**
 * @file GBaseClient.hpp
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

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
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

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GEnums.hpp"
#include "GIndividual.hpp"
#include "GSerializationHelperFunctions.hpp"
#include "GSerializationHelperFunctionsT.hpp"

namespace Gem
{
namespace GenEvA
{

/*********************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes designed for client-side
 * network communication. Their task is to retrieve GMember descriptions from
 * the server over a given protocol (implemented in derived classes), to instantiate
 * the corresponding GMember object, to process it and to deliver the results to
 * the server.
 */
class GBaseClient
	:private boost::noncopyable
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
	void setProcessMax(const boost::uint32_t& processMax);
	/** @brief Retrieves the value of the _processMax variable */
	boost::uint32_t getProcessMax() const;

	/** @brief Sets the maximum allowed processing time */
	void setMaxTime(const boost::posix_time::time_duration&);
	/** @brief Retrieves the maximum allowed processing time */
	boost::posix_time::time_duration getMaxTime();

	/** @brief Specifies whether results should be returned to the server regardless of their success */
	void returnResultIfUnsuccessful(const bool&);

protected:
	/** @brief One-time data retrieval, processing and result submission */
	bool process();

	/** @brief Performs initialization work */
	virtual bool init() { return true; }
	/** @brief Perform necessary finalization activities */
	virtual bool finally() { return true; }

	/** @brief Retrieve work items from the server. To be defined by derived classes. */
	virtual bool retrieve(std::string&, std::string&, std::string&) = 0;

	/** @brief Submit processed items to the server. To be defined by derived classes. */
	virtual bool submit(const std::string&, const std::string&) = 0;

	/** @brief Custom halt condition for processing */
	virtual bool customHalt(){return false;};

private:
	/** @brief halt condition for further processing */
	bool halt();

	GBaseClient(const GBaseClient&); ///< Intentionally left undefined
	const GBaseClient& operator=(const GBaseClient&);  ///< Intentionally left undefined

	boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization

	boost::uint32_t processed_; ///< The number of processed items so far
	boost::uint32_t processMax_; ///< The maximum number of items to process

	bool returnRegardless_; ///< Specifies whether unsuccessful processing attempts should be returned to the server
};

/*********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBASECLIENT_HPP_ */
