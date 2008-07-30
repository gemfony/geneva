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

#include "GLogStreamer.hpp"

namespace GenEvA
{
	
  /***********************************************************************************/
  /**
   * A standard constructor. As no logger is present yet, we set the pointer to NULL.
   */
  GLogStreamer::GLogStreamer(void) throw() {
    gl = (GLogger *)NULL;
  }

  /***********************************************************************************/
  /**
   * This constructor initialises the GLogger pointer, if possible.
   * 
   * \param g A pointer to a GLogger object
   */
  GLogStreamer::GLogStreamer(GLogger *g) throw() {
    this->setLogger(g);
  }

  /***********************************************************************************/
  /**
   * A standard copy constructor.
   * 
   * \param cp Another GLogStreamer object 
   */
  GLogStreamer::GLogStreamer(const GLogStreamer& cp) throw(){
    gl = cp.gl;
    (*this) << cp.str();
  }

  /***********************************************************************************/
  /**
   * A standard destructor. As we have no local, dynamically allocated data,
   * the function is empty.
   */
  GLogStreamer::~GLogStreamer() throw()
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * Returns the content of the ostringstream object.
   * 
   * \return The content of the ostringstream object
   */
  string GLogStreamer::content(void) const{
    return this->str();
  }

  /***********************************************************************************/
  /**
   * Stores an empty string in the ostringstream object.
   */
  void GLogStreamer::reset(void){
    this->str("");
  }

  /***********************************************************************************/
  /**
   * Stores a GLogger object in this class, if possible.
   * 
   * \param g A pointer to a GLogger object
   */
  void GLogStreamer::setLogger(GLogger *g) throw(){
    if(g) gl = g;
    else{
      std::clog << "In GLogStreamer::setLogger(GLogger *g): WARNING : " << endl
		<< "Invalid GLogger pointer given. No GLogger will be added" << endl;
      gl = (GLogger *)NULL;
    }
  }

  /***********************************************************************************/
  /**
   * Returns a pointer to the internal GLogger object.
   * 
   * \return A pointer to the internal GLogger object
   */
  GLogger *GLogStreamer::logger(void){
    return gl;
  }

  /***********************************************************************************/
  /**
   * Checks whether a logger is present
   * 
   * \return Boolean indicating whether a logger has been registered.
   */
  bool GLogStreamer::hasLogger(void) const{
    if(gl) return true;
    return false;
  }

  /***********************************************************************************/
  /**
   * Needed for ostringstream
   */
  GLogStreamer& GLogStreamer::operator<< (ostream& ( *val )(ostream&)){
    *((std::ostringstream*)this) << val; // ugly downcast
    return *this;		
  }

  /***********************************************************************************/
  /**
   * Needed for ostringstream
   */
  GLogStreamer& GLogStreamer::operator<< (ios& ( *val )(ios&)){
    *((std::ostringstream*)this) << val; // ugly downcast
    return *this;	
  }

  /***********************************************************************************/
  /**
   *  Needed for ostringstream
   */
  GLogStreamer& GLogStreamer::operator<< (ios_base& ( *val )(ios_base&)){
    *((std::ostringstream*)this) << val; // ugly downcast
    return *this;	
  }


  /***********************************************************************************/
  /**
   * Interface to the actual logging mechanism
   * 
   * \param gm A GManipulator object, usually emitted by the logLevel() function.
   */
  void GLogStreamer::operator<<(GManipulator gm){
    gm.man(*this,gm.severity);
  }

  /***********************************************************************************/
  /**
   * This is where the actual logger is called. Either the user has stored a pointer
   * to a logger in the GLogStreamer, or the default logger is used.
   */
  void log_level(GLogStreamer& gss, uint16_t severity){
    GLogger *gl;
    ostringstream logStream;
	
    if(gss.hasLogger()){
      gl=gss.logger();
    }
    else{
      gl=&(LOGGER); // note: LOGGER is a definition ...
    }
	
    // Do the actual logging
    logStream << gss.content();
    gl->log(logStream.str(), severity);
	
    // Finally we need to remove the text from gss
    gss.reset();
  }

  /***********************************************************************************/
  /**
   * This is the user-visible interface to the logging mechanism.
   * 
   * \param severity The log level that should be used for this log message
   * \return A GManipulator object, used for triggering the logging mechanism 
   */
  GManipulator logLevel(uint16_t severity){
    return GManipulator(log_level, severity);
  }

  /***********************************************************************************/
	
} /* namespace GenEvA */
