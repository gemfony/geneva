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

#include "GException.hpp"

namespace GenEvA
{

  /***********************************************************************************/
  /**
   * When GException finds a function with the signature of this function in the stream,
   * it throws itself. This empty function thus triggers an exception. We can very
   * intuitively fill and throw an exception in one go, similar to:<br/>
   * int16_t errorCode = 3;<br/>
   * GException ge;<br/>
   * ge << "An error occured: " << errorCode << raiseException;<br/>
   */
  void raiseException(void)
  {  /* nothing */ }

  /***********************************************************************************/
  /**
   * A standard constructor. The error string is filled with an empty sequence, and
   * the parent classes are initialised.
   */
  GException::GException(void) throw()
    :std::exception(), 
     GLogStreamer(),
     error_(""),
     forceException_(false)
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * A standard copy constructor
   * 
   * \param cp Another GException object
   */
  GException::GException(const GException& cp) throw(){
    (*this) << cp.content();
    error_ = cp.error_;
    forceException_ = cp.forceException_;
  }

  /***********************************************************************************/
  /**
   * A standard destructor
   */
  GException::~GException() throw() 
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * Sets the error message.
   * 
   * \param msg An error message to be stored in the object.
   */
  void GException::setError(string error){
    GLogStreamer::operator<<(error);
    error_ = error;
  }

  /***********************************************************************************/
  /**
   * Retrieves the error message.
   * 
   * \return The error message.
   */
  string GException::getError(void) const throw(){
    return error_;
  }

  /***********************************************************************************/
  /**
   * This is an interface to the C++ exception mechanism.
   * 
   * \return The error message
   */
  const char * GException::what(void) const throw(){ 
    return error_.c_str(); 
  }

  /***********************************************************************************/
  /**
   * Needed for the streaming mechanism 
   */
  GException& GException::operator<< (ostream& ( *val )(ostream&)){
    // Use dynamic cast ??
    *((GLogStreamer*)this) << val; // ugly downcast
    return *this;		
  }

  /***********************************************************************************/
  /**
   *  Needed for the streaming mechanism 
   */
  GException& GException::operator<< (ios& ( *val )(ios&)){
    *((GLogStreamer*)this) << val; // ugly downcast
    return *this;	
  }

  /***********************************************************************************/
  /**
   *  Needed for the streaming mechanism 
   */
  GException& GException::operator<< (ios_base& ( *val )(ios_base&)){
    *((GLogStreamer*)this) << val; // ugly downcast
    return *this;	
  }

  /***********************************************************************************/
  /**
   * If GException finds a function with the signature of "void raiseException(void)" in
   * the stream, it logs the error information with severity level 0 ("EXCEPTION") or
   * 1000000 (EXCEPTIONNOEXIT). In the first case the GLogger will either call
   * an exceptionHandler or terminate the program, if none is found. In the second case
   * the GLogger will log the data. Then the GException throws itself.
   */
  void GException::operator<< (void (*val) (void)){
    error_ = this->content();
	
    if(forceException_){
      *((GLogStreamer *)this) << logLevel(EXCEPTIONNOEXIT);
      throw(*this);	
    }
    else{
      *((GLogStreamer *)this) << logLevel(EXCEPTION);
    }

  }

  /***********************************************************************************/
  /** 
   * Forces GException to throw itself as an exception 
   * 
   * \param fe true if exceptions should be enforced 
   */
  void GException::forceException(bool fe){
    forceException_ = fe;
  }


  /***********************************************************************************/
  /** 
   * Retrieves the value of the forceException_ parameter
   * 
   * \return The value of the forceException_ parameter 
   */
  bool GException::getForceException(void) const throw(){
    return forceException_;
  }

  /***********************************************************************************/

} /* namespace GenEvA */
