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

#include "GBit.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GBit)

namespace GenEvA
{

  /************************************************************************/
  /**
   * The standard constructor. No local data, hence all work is done
   * by the parent class.
   */
  GBit::GBit(void)
    :GTemplateValue<GenEvA::bit>(GenEvA::TRUE)
  { /* nothing */ }

  /************************************************************************/
  /**
   * A constructor that assigns an initialisation value to the bit.
   * 
   * \param val the desired initial value of the GBit object
   */
  GBit::GBit(GenEvA::bit val)
    :GTemplateValue<GenEvA::bit>(val)
  { /* nothing */ }

  /************************************************************************/
  /**
   * A standard copy constructor. We have no local data. Hence all work is
   * done by the parent class.
   * 
   * \param cp A copy of another GBit object
   */
  GBit::GBit(const GBit& cp)
    :GTemplateValue<GenEvA::bit>(cp)
  { /* nothing */ }

  /************************************************************************/
  /**
   * The standard destructor. No local data, hence nothing to do.
   */
  GBit::~GBit()
  { /* nothing */ }

  /************************************************************************/
  /**
   * A standard assignment operator for GBit object.
   * 
   * \param cp A copy of another GBit object
   * \return A copy of this object
   */
  const GBit& GBit::operator=(const GBit& cp){
    GBit::load(&cp);
    return *this;
  }

  /************************************************************************/
  /**
   * A standard assignment operator for GBit object, camouflaged as
   * a GObject.
   * 
   * \param cp A copy of another GBit object, camouflaged as a GObject
   * \return A copy of this object
   */
  const GObject& GBit::operator=(const GObject& cp){
    GBit::load(&cp);
    return *this;	
  }

  /************************************************************************/
  /**
   * Resets the object to its initial state. As we have no local data,
   * we can rely on the parent class.
   */
  void GBit::reset(void){
    GTemplateValue<GenEvA::bit>::reset();
  }

  /************************************************************************/
  /**
   * Loads the data of another GBit Object.
   * 
   * \param gb A pointer to another GBit object, camouflaged as a GObject
   */
  void GBit::load(const GObject * gb){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBit *>(gb) == this){
      GException ge;
      ge << "In GBit::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    // We can rely on the parent class, as we
    // have no local data ourselves.
    GTemplateValue<GenEvA::bit>::load(gb);
  }

  /************************************************************************/
  /**
   * Creates a deep copy of this object
   * 
   * \return A deep copy of this object
   */
  GObject *GBit::clone(void){
    return new GBit(*this);
  }

  /************************************************************************/
  /**
   * Returns the value of this object. Use this function instead of 
   * GMember::fitness() !
   * 
   * \return The value of this object
   */
  GenEvA::bit GBit::getValue(void) const{
    return GTemplateValue<GenEvA::bit>::getValue();
  }

  /************************************************************************/
  /**
   * Returns the value of this object for use in GMember::fitness(), It is not recommended to 
   * use this function for a GBit - use GBit::getValue() instead.
   * 
   * \return The value of this object
   */
  double GBit::customFitness(void){
    if(GTemplateValue<GenEvA::bit>::getValue()) return 1.;
    return 0.;
  }

  /************************************************************************/
  /**
   * The mutation scheme as defined by the adaptors. 
   */
  void GBit::customMutate(void){
    GenEvA::bit lval = GTemplateValue<GenEvA::bit>::getValue();
    GMutable<GenEvA::bit>::applyAllAdaptors(lval);
    GTemplateValue<GenEvA::bit>::setInternalValue(lval);
  }

  /************************************************************************/
  /**
   * This function assembles a report about the inner state of the object
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GBit::assembleReport(uint16_t indention ) const
  {
    ostringstream oss;
    oss << ws(indention) << "GBit : " << this << endl
	<< ws(indention) << "-----> Report from parent class GTemplateValue<GenEvA::bit> : " << endl
	<< GTemplateValue<GenEvA::bit>::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /************************************************************************/

} /* namespace GenEvA */
