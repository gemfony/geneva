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

#include "GRange.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GRange)


namespace GenEvA
{

  /***********************************************************************/
  /**
   * The default constructor. It tries to set the lower and upper boundary in 
   * such a way that at least DEFMINDIGITS decimal places are available.
   * Note that the ranges are actually set in GRange::setMinDigits, not in this 
   * constructor. 
   */
  GRange::GRange(void)
    :GObject("GRange")
  {
    setMinDigits(DEFMINDIGITS);
    setIsActive(true);
  }

  /***********************************************************************/
  /**
   * This constructor sets the lower and upper boundary and activates the range.
   * 
   * \param lower The value of the lower boundary
   * \param lwopen A value indicating whether lower is an open (true) or closed (false) boundary
   * \param upper The value of the upper boundary
   * \param upopen A value indicating whether upper is an open (true) or closed (false) boundary
   */
  GRange::GRange(double lower, bool lwopen, double upper, bool upopen)
    :GObject("GRange")
  {
    _lower.setBoundary(lower,ISLOWER, lwopen);
    _upper.setBoundary(upper,ISUPPER, upopen);

    setIsActive(true);
  }

  /***********************************************************************/
  /**
   * A standard copy constructor.
   * 
   * \param cp Another GRange object
   */
  GRange::GRange(const GRange& cp)
    :GObject(cp)
  {
    _lower = cp._lower;
    _upper = cp._upper;

    setIsActive(cp._isactive);
  }

  /***********************************************************************/
  /**
   * The standard destructor. As we have no dynamically allocated data,
   * there is nothing here to do.
   */
  GRange::~GRange(){
    /* nothing */
  }

  /***********************************************************************/
  /**
   * A standard assignment operator
   * 
   * \param cp Another GRange object
   */
  const GRange& GRange::operator=(const GRange& cp){
    GRange::load(&cp);
    return *this;
  }

  /***********************************************************************/
  /**
   * A standard assignment operator for GObject objects
   * 
   * \param cp Another GRange object camouflaged as a GObject
   */
  const GObject& GRange::operator=(const GObject& cp){
    GRange::load(&cp);
    return *this;
  }

  /***********************************************************************/
  /**
   * Resets the range to its initial values
   */
  void GRange::reset(void){
    setMinDigits(DEFMINDIGITS);
    setIsActive(false);
	
    GObject::reset();
  }

  /***********************************************************************/
  /**
   * Loads the data of another GRange object, camouflaged as a GObject.
   * 
   * \param cp Another GRange object, camouflaged as a GObject
   */
  void GRange::load(const GObject * cp){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GRange *>(cp) == this){
      GException ge;
      ge << "In GRange::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GObject::load(cp);

    const GRange *gg = dynamic_cast<const GRange *>(cp);
    if(!gg){
      GException ge;
      ge << "In GRange::load() (1.): Conversion error!" << endl
	 << raiseException;
    }

    _lower = gg->_lower;
    _upper = gg->_upper;

    setIsActive(gg->_isactive);
  }

  /***********************************************************************/
  /**
   * Creates a deep copy of this object
   * 
   * \return A deep copy of this object, camouflaged as a GObject
   */
  GObject *GRange::clone(void){
    return new GRange(*this);
  }

  /***********************************************************************/
  /**
   * Marks the range as active. This implies marking both boundaries as active.
   * 
   * \param A parameter indicating whether the range is active (true) or inactive (false)
   */
  void GRange::setIsActive(bool isactive){
    // First mark the boundaries as active ...	
    _lower.setIsActive(isactive);
    _upper.setIsActive(isactive);
    // ... and then ourself.
    _isactive = isactive;
  }

  /***********************************************************************/
  /**
   * Checks whether the range is active or inactive
   * 
   * \return A value indicating whether the range is active (true) or inactive (false)
   */
  bool GRange::isActive(void) const{
    // Do both boundaries have the same state as the _isactive parameter ?
    // It is a serious error if this is not the case
    if((_lower.isActive()!=_upper.isActive()) || (_lower.isActive()!=_isactive)){
      GException ge;
      ge << "In GRange::isActive() : Bad active values. They should all be the same!" << endl
	 << "lower boundary " << _lower.isActive() << endl
	 << "upper boundary " << _upper.isActive() << endl
	 << "GRange         " << _isactive << endl
	 << raiseException; 
    }
	
    return _isactive;
  }

  /***********************************************************************/
  /**
   * This function tries to limit the range in such a way that likely ndigits decimal
   * places are available. This assumes that the mantissa has MAXDIGITS digits. Note that 
   * this means that part of the usual range of a double ( approx. -10^308:10^308) is
   * unavailable. Also note that the limit imposed is not a strong one. The GRange object
   * will have the same activity status as before the command.
   * 
   * \param ndigits The number of desired decimal places
   */
  void GRange::setMinDigits(int16_t ndigits){
    // We assume MAXDIGITS digits in a double.
    const int16_t exponent = MAXDIGITS - ndigits;
    const double extreme = pow(10.,double(exponent));

    setLowerBoundary(-extreme,false);
    setUpperBoundary(+extreme,false);
  }

  /***********************************************************************/
  /**
   * This function sets the upper and lower boundaries in one go. The GRange
   * object will have the same activity status as before the call.
   * 
   * \param lower The value of the lower boundary
   * \param lwopen Indicates whether this is an open (true) or closed (false) boundary
   * \param upper The value of the upper boundary
   * \param upopen Indicates whether this is an open (true) or closed (false) boundary
   */

  void GRange::setBoundaries(double lower, bool lwopen, double upper, bool upopen){
    setLowerBoundary(lower,lwopen); 
    setUpperBoundary(upper, upopen);
  }

  /***********************************************************************/
  /**
   * Sets the lower boundary to a given value. This function is private, as 
   * it should not be called independently of the upper boundary.
   * 
   * \param lower The value of the lower boundary
   * \param isopen Indicates whether this is an open (true) or closed (false) boundary
   */
  void GRange::setLowerBoundary(double lower, bool isopen){
    _lower.setBoundary(lower,ISLOWER,isopen);
  }

  /***********************************************************************/
  /**
   * Sets the upper boundary to a given value. This function is private, as 
   * it should not be called independently of the lower boundary.
   * 
   * \param upper The value of the upper boundary
   * \param isopen Indicates whether this is an open (true) or closed (false) boundary
   */
  void GRange::setUpperBoundary(double upper, bool isopen){
    _upper.setBoundary(upper,ISUPPER,isopen);
  }

  /***********************************************************************/
  /**
   * Retrieves the (corrected) value of the lower boundary
   * 
   * \return The corrected value of the lower boundary 
   */
  double GRange::lowerBoundary(void){
    return _lower.value();
  }

  /***********************************************************************/
  /**
   * Retrieves the (corrected) value of the upper boundary
   * 
   * \return The corrected value of the lower boundary
   */
  double GRange::upperBoundary(void){
    return _upper.value();
  }

  /***********************************************************************/
  /**
   * Calculates the width of the range
   * 
   * \return The width of the range 
   */
  double GRange::width(void){
    double result;

    result=upperBoundary()-lowerBoundary();

    if(result < 0.)
      {
	GException ge;
	ge << "In GRange::width(): Error!" << endl
	   << "Invalid gap boundaries : " << endl 
	   << "upper boundary: " << upperBoundary() << endl 
	   << "lower boundary: " << lowerBoundary() << endl
	   << "width: " << result << endl
	   << raiseException;
      }

    return result;
  }

  /***********************************************************************/
  /**
   * Checks whether a given value is in this range.
   * 
   * \param val The value to be checked
   * \return A boolean indicating whether or not the value is in the range
   */
  bool GRange::isIn(double val){
    if(val >= lowerBoundary() && val <= upperBoundary()) return true;
    return false;
  }

  /***********************************************************************/
  /**
   *  Checks whether this GRange overlaps with another.
   * 
   *  \param other A GRange object to check for a possible overlap
   *  \return A boolean indicating whether there is an overlap
   */
  bool GRange::overlaps(GRange& other){
    // Check containment
    if(contains(other) || other.contains(*this)) return true;

    // Check partial overlap: one boundary is inside the others
    // boundaries, while the other is not
    if((lowerBoundary() <= other.lowerBoundary() &&
	upperBoundary() > other.lowerBoundary() &&
	upperBoundary() <= other.upperBoundary()) ||
       (other.lowerBoundary() <= lowerBoundary() &&
	other.upperBoundary() > lowerBoundary() &&
	other.upperBoundary() <= upperBoundary()))	return true;

    return false;
  }

  /***********************************************************************/
  /**
   * Checks whether this range contains another range.
   * 
   * \param other A Grange object that is checked for containment
   * \return A boolean indicating whether the other object is contained in this range
   */
  bool GRange::contains(GRange& other){
    if(other.lowerBoundary() >= this->lowerBoundary()
       && other.upperBoundary() <= this->upperBoundary()){
      return true;
    }

    return false;
  }

  /***********************************************************************/
  /**
   * Assembles a reports about the inner state of the object.
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GRange::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GRange: " << this << endl
	<< ws(indention) << "is" << (_isactive?" active":" inactive") << endl
	<< ws(indention) << "-----> Report from lower boundary : " << endl
	<< _lower.assembleReport(indention+NINDENTION) << endl
	<< ws(indention) << "-----> Report from upper boundary : " << endl
	<< _upper.assembleReport(indention+NINDENTION) << endl
	<< ws(indention) << "-----> Report from parent class GObject : " << endl
	<< GObject::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /***********************************************************************/

} /* namespace GenEvA */
