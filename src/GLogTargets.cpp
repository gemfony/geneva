/**
 * @file
 */

/* GLogTargets.cpp
 *
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

namespace Gem {
namespace GLogFramework {

  /***********************************************************************************/
  /**
   * The default constructor. No local data, hence
   * no implementation needed.
   */
  GBaseLogTarget::GBaseLogTarget()
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
  GConsoleLogger::GConsoleLogger()
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
   * @param msg The log message
   */
  void GConsoleLogger::log(const std::string& msg){
    std::cerr << msg;
  }

  /***********************************************************************************/
  /**
   * A default constructor. If no name is specified by the user for the log file, the
   * default name Geneva.log is chosen. Note that logging will be done in append mode,
   * so previous data in this file will not be lost.
   */
  GDiskLogger::GDiskLogger()
	  :fname_("Geneva.log")
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * A standard copy constructor
   */
  GDiskLogger::GDiskLogger(const GDiskLogger& cp)
	  :fname_(cp.fname_)
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * A constructor that accepts the name of the log file as argument.
   *
   * @param fname The name of the file log messages should be written to
   */
  GDiskLogger::GDiskLogger(const std::string& fname)
	  :fname_(fname)
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * The standard destructor. No local data, hence no implementation needed.
   */
  GDiskLogger::~GDiskLogger()
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * Standard assignment operator
   */
  const GDiskLogger& GDiskLogger::operator=(const GDiskLogger& cp){
	  fname_ = cp.fname_;
  }

  /***********************************************************************************/
  /**
   * This function logs a message to a file, whose name it takes from the private
   * variable fname_. The file is reopened in append mode for every log message. Local
   * caching of messages might be possible at a later stage.
   *
   * @param msg The log message
   */
  void GDiskLogger::log(const std::string& msg){
	if(fname_.empty()){
		std::cerr << "In GDiskLogger::log() : Error" << std::endl
			      << "Filename fname_ is empty." << std::endl;

		std::terminate();
	}

    std::ofstream ofstr(fname_.c_str(), std::ios_base::app);

    if(!ofstr.good()){
        std::cerr << "In GDiskLogger::log() : Error" << std::endl
  		<< "ofstring is in a bad state" << std::endl;

        std::terminate();
    }

    ofstr << msg;
    ofstr.close();
  }

  /***********************************************************************************/

}} /* namespace Gem::GLogFramework */
