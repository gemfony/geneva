/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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


#ifndef GBASECLIENT_H_
#define GBASECLIENT_H_

using namespace std;

#include "GMemberCarrier.hpp"


namespace GenEvA
{

/**
 * The serialization mode - XML or text
 */
const uint8_t CLIENTXMLMODE=0;
const uint8_t CLIENTTEXTMODE=1;

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
	/** \brief Constructor passes command line arguments 
	 * to network implementation */
	GBaseClient(void);
	/** \brief Standard destructor */
	virtual ~GBaseClient();
	
	/** \brief The main loop */
	void run(void);
	
	/** \brief Allows to set a maximum number of processing steps */
	void setProcessMax(uint16_t processMax);
	/** \brief Retrieves the value of the _processMax variable */
	uint16_t getProcessMax(void) const;
	
	/** \brief Initializes the networking implementation */
	virtual void init(void){}; // To be called from derived classes' constructor
	/** \brief Shuts down the network implementation */
	virtual void finally(void){}; // To be called from derived classes' destructor
	
protected:
	/** \brief One-time data retrieval, processing and result submission */
	bool process(void);
	
	/** \brief Retrieve work items from the server. To be defined by derived classes. */
	virtual bool retrieve(string&) = 0;

	/** \brief Submit processed items to the server. To be defined by derived classes. */
	virtual bool submit(const string&) = 0;
	
private:
	GBaseClient(const GBaseClient&); ///< Intentionally left undefined
	const GBaseClient& operator=(const GBaseClient&);  ///< Intentionally left undefined
	
	uint16_t processMax_; ///< The maximum number of items to process
};

/*********************************************************************************/

} /* namespace GenEvA */

#endif /* GBASECLIENT_H_ */
