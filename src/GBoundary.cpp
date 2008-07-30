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

#include "GBoundary.hpp"

/** 
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GBoundary)


namespace GenEvA
{

  /***************************************************************************/
  /**
   * The default construcor. Marks the boundary as an upper closed boundary
   * with value 0.
   */
  GBoundary::GBoundary(void){
    setBoundary(0.,true,false);
    setIsActive(false); // standard value
  }

  /***************************************************************************/
  /**
   * This constructor sets the internal representation of the boundary and
   * lets us specify whether this is an upper or lower, open or closed 
   * boundary.
   * 
   * \param boundary The boundary value
   * \param isupper Specifies whether this is an upper (true) or lower (false) boundary
   * \param isopen Specifies whether this is an open (true) or closed (false) boundary
   */
  GBoundary::GBoundary(double boundary,  bool isupper, bool isopen)
  {
    setBoundary(boundary,isupper,isopen);
    setIsActive(false); // standard value
  }

  /***************************************************************************/
  /**
   * A standard copy constructor. We first initialise our parent class,
   * then we copy all data over.
   *
   * \param cp A copy of another GBoundary object
   */
  GBoundary::GBoundary(const GBoundary& cp)
    :GObject(cp)
  {
    setIsActive(cp._isactive);
    setBoundary(cp._boundary);
    setIsUpper(cp._isupper);
    setIsOpen(cp._isopen);
  }

  /***************************************************************************/
  /**
   * The standard constructor. As we have no local, dynamically allocated
   * data, there is nothing to do here.
   */
  GBoundary::~GBoundary(){
    /* nothing */
  }

  /***************************************************************************/
  /** 
   * A standard assignment operator for GBoundary objects,
   * 
   * \param cp A copy of another GBoundary object
   */
  const GBoundary& GBoundary::operator=(const GBoundary& cp){
    GBoundary::load(&cp);
    return *this;
  }

  /***************************************************************************/	
  /** 
   * A standard assignment operator for GBoundary objects,
   * 
   * \param cp A copy of another GBoundary object, camouflaged as a GObject
   */	
  const GObject& GBoundary::operator=(const GObject& cp){
    GBoundary::load(&cp);
    return *this;	
  }

  /***************************************************************************/
  /**
   * Resets the object to its initial state.
   */
  void GBoundary::reset(void){
    // First we reset our own data
    setBoundary(0.);
    setIsActive(false);
    setIsUpper(true);
    setIsOpen(false);

    // then we reset the parent class
    GObject::reset();
  }

  /***************************************************************************/
  /**
   * Loads the data of another GBoundary object, camouflaged as a GObject .
   * 
   *  \param cp A copy of another GBoundary object
   */
  void GBoundary::load(const GObject *cp){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBoundary *>(cp) == this){
      GException ge;
      ge << "In GBoundary::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GObject::load(cp);

    const GBoundary *gbptr = dynamic_cast<const GBoundary *>(cp);
    if(!gbptr){
      GException ge;
      ge << "In GBoundary::load() : Conversion error!" << endl
	 << raiseException;
    }

    setIsActive(gbptr->_isactive);
    setBoundary(gbptr->_boundary);
    setIsUpper(gbptr->_isupper);
    setIsOpen(gbptr->_isopen);
  }

  /***************************************************************************/
  /**
   * Creates a deep copy of this object.
   * 
   * \return A deep copy of this object, camouflaged as a GObject
   */
  GObject *GBoundary::clone(void){
    return new GBoundary(*this);
  }

  /***************************************************************************/
  /**
   * Returns the value of this object. <em>Note that retrieving the value of an
   * inactive boundary will result in an exception being thrown</em>. Use getBoundary()
   * to retrieve the boundary value of an inactive GBoundary object.
   * 
   * \return The current boundary value
   */
  double GBoundary::value(void) const{
    if(!this->isActive()){
      GException ge;
      ge << "In GBoundary::value(): Error!" << endl
	 << "Tried to retrieve the value of an inactive boundary" << endl
	 << raiseException;
    }
	
    return _boundary;
  }

  /***************************************************************************/
  /**
   * Marks the boundary as active. A runtime error will be thrown from
   * GBoundary::value(), if this variable is set to false.
   * 
   * \param isactive A parameter indicating whether or not this is an active boundary
   */
  void GBoundary::setIsActive(bool isactive){
    _isactive = isactive;
  }

  /***************************************************************************/
  /**
   * Allows to check whether this boundary is active or not.
   * 
   * \return A value indicating whether or not this is an active boundary
   */
  bool GBoundary::isActive(void) const {
    return _isactive;
  }

  /***************************************************************************/
  /**
   * Checks whether this is an upper (true) or lower (false) boundary.
   * 
   * \return A value indicating whether or not this is an upper boundary
   */
  bool GBoundary::isUpper(void) const {
    return _isupper;
  }

  /***************************************************************************/
  /**
   * Retrieves the value of the boundary regardless of whether this boundary
   * is active. It is recommended not to use this function in calculations that
   * depend on the boundary being active. Use GBoundary::value() instead
   * 
   * \return The current value of the boundary, regardless of activity status.
   */ 
  double GBoundary::getBoundary(void) const {
    return _boundary;
  }

  /***************************************************************************/
  /**
   * Private function that sets the boundary to a given value.
   * 
   * \param boundary A new value for the boundary
   */
  void GBoundary::setBoundary(double boundary){
    _boundary = boundary;
  }

  /***************************************************************************/
  /**
   * This function sets the values of the boundary and of the _isupper and
   * _isopen parameters. Internally it calculates a new value for the boundary,
   * as this depends on the position of the boundary and on whether or not
   * it is open.
   * 
   * Note that after this function the boundary will have been activated.
   * You need to deactivate it again, if you do not want this.
   * 
   * \param boundary A new value for the boundary
   * \param isupper A value indicating whether this is an upper (true) or lower (false) boundary
   * \param isopen A value indicating whether this is an open (true) or closed (false) boundary
   * 
   * \return The new value of the boundary, after application of correction factors
   */
  double GBoundary::setBoundary(double boundary, bool isupper, bool isopen){
    double result = 0., factor = 0.;

    // Make sure we have got the right factor
    if(isupper && isopen) factor = -1.;
    else if(!isupper && isopen) factor = 1.;

    // Apply smallest distinguishable double, if necessary
    result=boundary+factor*getMinDouble(boundary);

    if(isopen && ((result >= boundary && isupper) || (result <= boundary && !isupper))){ // open ? upper&result>=bdn ? lower&result<=boundary?
      GException ge;
      ge << "In GBoundary::setBoundary(" << boundary << isupper << "," << isopen << "): Bad result !" << endl
	 << "Result = " << result << ", Boundary = " << boundary << endl
	 << "getMinDouble(boundary) = " << getMinDouble(boundary) << endl
	 << raiseException;
    }

    setIsUpper(isupper);
    setIsOpen(isopen);
	
    setBoundary(result);
   	
    setIsActive(true);
   	
    return result;
  }

  /***************************************************************************/
  /**
   * Private function that sets the isupper variable.
   * 
   * \param isupper A value indicating whether this is an upper (true) or lower (false) boundary
   */
  void GBoundary::setIsUpper(bool isupper){
    _isupper = isupper;
  }

  /***************************************************************************/
  /**
   * Checks whether thos os an open (true) or closed (false) boundary
   * 
   * \return A value indicating whether this is an open (true) or closed (false) boundary
   */
  bool GBoundary::isOpen(void) const {
    return _isopen;
  }

  /***************************************************************************/
  /**
   * Private function that sets the _isopen variable.
   * 
   * \param isopen A variable indicating whether this is an open (true) or closed (false) boundary
   */
  void GBoundary::setIsOpen(bool isopen){
    _isopen = isopen;
  }

  /***************************************************************************/
  /**
   * Assembles a report about the inner state of this object.
   *
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of this object
   */
  string GBoundary::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GBoundary : " << this << endl
	<< ws(indention) << "boundary value is " << _boundary << " ." << endl
	<< ws(indention) << "This is an" << (_isactive?" active":"inactive") << (_isupper?" upper,":" lower,") << (_isopen?"open ":"closed ") << "boundary" << endl
	<< ws(indention) << "-----> Report from parent class GTemplateValue<GenEvA::bit> : " << endl
	<< GObject::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /***************************************************************************/

} /* namespace GenEvA */
