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

#include "GBaseClient.hpp"

namespace GenEvA
{

/*********************************************************************************/
/**
 * Some networking implementations will need to know about command line arguments.
 * As this class has been designed so the main function of the client only has to
 * call the run() function, we need to pass the command line arguments to the
 * constructor.
 */
GBaseClient::GBaseClient(void)
	:processMax_(0)
{ /* nothing*/ }

/*********************************************************************************/
/**
 * A standard destructor. We have no local, dynamically allocated data, hence it is empty.
 */
GBaseClient::~GBaseClient()
{ /* nothing */ }

/*********************************************************************************/
/**
 * This is the main loop of the client. It will continue to call the process()
 * function (defined by derived classes), until it returns false or the maximum 
 * number of processing steps has been reached. All network connectivity is done 
 * in process().
 */
void GBaseClient::run(void){
	uint16_t processed = 0;
	while(this->process()  &&  (processMax_?processed++<processMax_:true));
}

/*********************************************************************************/
/** 
 * Allows to set a maximum number of processing steps. If set to 0 or left unset,
 * processing will be done until process() returns false.
 * 
 * \param processMax Desired value for the processMax_ variable 
 */
void GBaseClient::setProcessMax(uint16_t processMax){
	processMax_ = processMax;
}

/*********************************************************************************/
/** 
 * Retrieves the value of the processMax_ variable.
 * 
 * \return The value of the processMax_ variable 
 */
uint16_t GBaseClient::getProcessMax(void) const{
	return processMax_;
}

/*********************************************************************************/
/**
 * In order to allow derived classes to concentrate on network issues, all
 * marshalling, calculation, and unmarshallung is done in the GBaseClient class,
 * which in turn makes use of the facilities provided by GMemberCarrier.
 */
bool GBaseClient::process(void){
	// Get an item from the server
	string data;
	if(!this->retrieve(data)) return false;
	
	// There is a possibility that we have received an unknown command
	// or a timeout command. In this case we want to try again until retrieve
	// returns "false". If we return true here, the next "process" command will
	// be executed.
	if(data == "empty") return true;
	
	// unmarshall the data and do the calculation
	GMemberCarrier target(data);
			
	// This one line is all it takes to do the processing required for this GMemberCarrier.
	// GMemberCarrier has all required functions on board. This class does not need to understand 
	// what is being done during the processing.
	target.process();
							
	// transform target back into a string and submit to the server
	if(!this->submit(target.toString())) return false;
	
	return true;
}

/*********************************************************************************/

} /* namespace GenEvA */
