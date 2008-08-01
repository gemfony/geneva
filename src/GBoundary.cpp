/**
 * @file
 */

/* GBoundary.cpp
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

#include "GBoundary.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundary)


namespace Gem
{
namespace GenEvA
{

  /***************************************************************************/
  /**
   * The default construcor. Marks the boundary as an upper closed boundary
   * with value 0.
   */
  GBoundary::GBoundary():
	  GObject(),
	  boundary_(0),
	  isactive_(false),
	  isupper_(true),
	  isopen_(BNDISCLOSED)
  { /* nothing */ }

  /***************************************************************************/
  /**
   * This constructor sets the internal representation of the boundary and
   * lets us specify whether this is an upper or lower, open or closed
   * boundary.
   *
   * @param boundary The boundary value
   * @param isupper Specifies whether this is an upper (true) or lower (false) boundary
   * @param isopen Specifies whether this is an open (true) or closed (false) boundary
   */
  GBoundary::GBoundary(double boundary,  bool isupper, bool isopen):
	  GObject(),
	  boundary_(boundary),
	  isactive_(false),
	  isupper_(isupper),
	  isopen_(isopen)
  { /* nothing */ }

  /***************************************************************************/
  /**
   * A standard copy constructor. We first initialise our parent class,
   * then we copy all data over.
   *
   * @param cp A copy of another GBoundary object
   */
  GBoundary::GBoundary(const GBoundary& cp)
    :GObject(cp),
    boundary_(cp.boundary_),
    isactive_(cp.isactive_),
    isupper_(cp.isupper_),
    isopen_(cp.isopen_)
  { /* nothing */ }

  /***************************************************************************/
  /**
   * The standard constructor. As we have no local, dynamically allocated
   * data, there is nothing to do here.
   */
  GBoundary::~GBoundary()
  { /* nothing */ }

  /***************************************************************************/
  /**
   * A standard assignment operator for GBoundary objects,
   *
   * @param cp A copy of another GBoundary object
   */
  const GBoundary& GBoundary::operator=(const GBoundary& cp){
    GBoundary::load(&cp);
    return *this;
  }

  /***************************************************************************/
  /**
   * Resets the object to its initial state.
   */
  void GBoundary::reset(){
    // First we reset our own data
    setBoundary(0.);
    setIsActive(false);
    setIsUpper(true);
    setIsOpen(BNDISCLOSED);

    // then we reset the parent class
    GObject::reset();
  }

  /***************************************************************************/
  /**
   * Loads the data of another GBoundary object, camouflaged as a GObject .
   *
   *  @param cp A copy of another GBoundary object, camouflaged as a GObject
   */
  void GBoundary::load(const GObject *cp){
	// Convert GObject pointer to local format
    const GBoundary *gb_load = this->checkedConversion(cp,this);

	// Load the parent class'es data
	GObject::load(cp);

    boundary_=gb_load->boundary_;
    isactive_=gb_load->isactive_;
    isupper_=gb_load->isupper_;
    isopen_=gb_load->isopen_;
  }

  /***************************************************************************/
  /**
   * Creates a deep copy of this object.
   *
   * @return A deep copy of this object, camouflaged as a GObject
   */
  GObject *GBoundary::clone(){
    return new GBoundary(*this);
  }

  /***************************************************************************/
  /**
   * Specifies whether this boundary is active.
   *
   * @param isactive A parameter indicating whether or not this is an active boundary
   */
  void GBoundary::setIsActive(bool isactive) throw(){
    isactive_ = isactive;
  }

  /***************************************************************************/
  /**
   * Allows to check whether this boundary is active or not.
   *
   * @return A value indicating whether or not this is an active boundary
   */
  bool GBoundary::isActive() const throw(){
    return isactive_;
  }

  /***************************************************************************/
  /**
   * Checks whether this is an upper (true) or lower (false) boundary.
   *
   * @return A value indicating whether or not this is an upper boundary
   */
  bool GBoundary::isUpper() const throw(){
    return isupper_;
  }

  /***************************************************************************/
  /**
   * Retrieves the value of the boundary_ variable.
   *
   * @return The current value of the boundary
   */
  double GBoundary::getBoundary() const throw(){
    return boundary_;
  }

  /***************************************************************************/
  /**
   * Private function that sets the boundary to a given value.
   *
   * @param boundary A new value for the boundary
   */
  void GBoundary::setBoundary(double boundary) throw(){
    boundary_ = boundary;
  }

  /***************************************************************************/
  /**
   * This function sets the values of the boundary and of the isupper_ and
   * isopen_ parameters. Internally it calculates a new value for the boundary,
   * as this depends on the position of the boundary and on whether or not
   * it is open.
   *
   * Note that after this function the boundary will have been activated.
   * You need to deactivate it again, if you do not want this.
   *
   * @param boundary A new value for the boundary
   * @param isupper A value indicating whether this is an upper (true) or lower (false) boundary
   * @param isopen A value indicating whether this is an open (true) or closed (false) boundary
   *
   * @return The new value of the boundary, after application of correction factors
   */
  double GBoundary::setBoundary(double boundary, bool isupper, bool isopen){
    double result = 0.;

    if(isopen){
    	// Add/substract the smallest distinguishable double value
    	if(isupper){
    		result=boundary-getMinDouble(boundary);
    	}
    	else{
    		result=boundary+getMinDouble(boundary);
    	}

    	// Check that the result makes sense
    	if((result >= boundary && isupper) || (result <= boundary && !isupper)){
    		std::ostringstream error;
			error << "In GBoundary::setBoundary : Error!" << std::endl
				  << "Result not inside allowed range:" << std::endl
				  << "result = " << result << std::endl
				  << "boundary = " << boundary << std::endl
				  << "isupper = " << isupper << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_result_not_in_range() << error_string(error.str());
    	}
    }

    isupper_ = isupper;
    isopen_ = isopen;

    boundary_ = result;

    isactive_ = true;

    return result;
  }

  /***************************************************************************/
  /**
   * Finds the smallest double x for which x+getMinDouble(x) > x
   * Attention : This implementation seems to be close to the smallest
   * distinguishable double, but apparently still returns slightly too
   * large values.
   *
   * \param val double value for which to search for smallest larger neighbour
   * \return Smallest distinguishable double (for "val").
   */

  double GBoundary::getMinDouble(double val) const {
    double result;
    int exponent; // we cannot use int16_t here - frexp complains ...

    std::frexp(val,&exponent);
    result=std::pow(2.,exponent-DBL_MANT_DIG);

    return result;
  }


  /***************************************************************************/
  /**
   * Private function that sets the isupper variable.
   *
   * @param isupper A value indicating whether this is an upper (true) or lower (false) boundary
   */
  void GBoundary::setIsUpper(bool isupper) throw(){
    isupper_ = isupper;
  }

  /***************************************************************************/
  /**
   * Checks whether this is an open (true) or closed (false) boundary
   *
   * @return A value indicating whether this is an open (true) or closed (false) boundary
   */
  bool GBoundary::isOpen() const throw(){
    return isopen_;
  }

  /***************************************************************************/
  /**
   * Private function that sets the isopen_ variable.
   *
   * @param isopen A variable indicating whether this is an open (true) or closed (false) boundary
   */
  void GBoundary::setIsOpen(bool isopen) throw(){
    isopen_ = isopen;
  }

  /***************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
