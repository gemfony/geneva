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

#include "GLogTargets.hpp"

namespace GenEvA
{

  /***********************************************************************************/
  /**
   * The default constructor. No local data, hence
   * no implementation needed.
   */
  GBaseLogTarget::GBaseLogTarget(void)
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * The standard destructor. No local data, hence
   * no implementation needed.
   */    
  GBaseLogTarget::~GBaseLogTarget()
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * The default constructor. No local data, hence
   * no implementation needed.
   */
  GConsoleLogger::GConsoleLogger(void)
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * The standard destructor. No local data, hence
   * no implementation needed.
   */
  GConsoleLogger::~GConsoleLogger()
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * Logs a message to the console.
   * 
   * \param msg The log message
   */
  void GConsoleLogger::log(const string& msg){
    std::clog << msg;
  }

  /***********************************************************************************/
  /**
   * A default constructor. If no name is specified by the user for the log file, the
   * default name Geneva.log is chosen. Note that logging will be done in append mode,
   * so previous data in this file will not be lost.
   */
  GDiskLogger::GDiskLogger(void){
    fname = "Geneva.log";
  }

  /***********************************************************************************/
  /**
   * A constructor that accepts the name of the log file as argument.
   */
  GDiskLogger::GDiskLogger(string fn){
    fname = fn;
  }

  /***********************************************************************************/
  /**
   * The standard destructor. No local data, hence no implementation needed.
   */
  GDiskLogger::~GDiskLogger()
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * This function logs a message to a file, whose name it takes from the private
   * variable fname. The file is reopened in append mode for every log message. Local
   * caching of messages might be possible at a later stage.
   */
  void GDiskLogger::log(const string& msg){
    ofstream ofstr(fname.c_str(), ios_base::app);
    if(ofstr.good()){
      ofstr << msg;
      ofstr.close();
    }
    else{
      std::cerr << "In GDiskLogger::log() : Error" << endl
		<< "ofstring is in a bad state" << endl;
      exit(1);
    }
  }

  /***********************************************************************************/

} /* namespace GenEvA */
