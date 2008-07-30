/**
 * \file
 */


/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

#include "GServerSession.hpp"

namespace GenEvA
{

  /*********************************************************************/
  /**
   * The default constructor. We have nothing to initialize, hence this
   * function is empty.
   */
  GServerSession::GServerSession()
  { /* nothing */ }

  /*********************************************************************/
  /**
   * The standard destructor. We have no local, dynamically allocated data,
   * hence it is empty.
   */
  GServerSession::~GServerSession()
  { /* nothing */ }

  /*********************************************************************/
  /**
   * This function processes an individual request from a client.
   */
  void GServerSession::processRequest(void){
    // Every client transmission starts with a command
    string command = getSingleCommand();
	
    // Retrieve an item from the broker and
    // submit it to the client.
    if(command == "getdata"){
      bool timeout=true, success=false;
      string outbound_data;
		
      // Retrieve item
      success = BROKER.get(outbound_data,timeout);
		
      // Check errors
      if(!success) this->sendSingleCommand("nosuccess");
      else if(timeout) this->sendSingleCommand("timeout");
		
      if(!this->submit(outbound_data)){
	GLogStreamer gls;
	gls << "GServerSession::processRequest():" << endl
	    << "Could not submit item to client!" << endl
	    << logLevel(UNCRITICAL);
      }
    }
    // Retrieve an item from the client and
    // submit it to the broker
    else if(command == "result"){
      string dataString;
		
      if(!this->retrieve(dataString)){
	GLogStreamer gls;
	gls << "GServerSession::processRequest():" << endl
	    << "Could not retrieve item from client." << endl
	    << logLevel(UNCRITICAL);
      }
		
      // Give the object back to the broker, so it can be sorted back
      // into the populations.
      if(!BROKER.put(dataString)){
	GLogStreamer gls;
	gls << "GServerSession::processRequest():" << endl
	    << "Could not deliver received item to broker." << endl
	    << logLevel(UNCRITICAL);
      }
    }
    else{
      GLogStreamer gls;
      gls << "GServerSession::processRequest: Warning!" << endl
	  << "Received unkown command." << command << endl
	  << logLevel(UNCRITICAL);

      this->sendSingleCommand("unknown");
    }
  }

  /*********************************************************************/

} /* namespace GenEvA */
