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

#include "GBitCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GBitCollection)


namespace GenEvA
{

  /**********************************************************************/
  /**
   * The standard constructor. As we have no local data, all work is done
   * by the parent class.
   */
  GBitCollection::GBitCollection(void)
    :GTemplateValueCollection<bit>()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Initializes the class with a set of nval random bits.
   * 
   * \param nval The size of the collection
   */
  GBitCollection::GBitCollection(int16_t nval)
    :GTemplateValueCollection<bit>()
  {
    for(int16_t i= 0; i<nval; i++) this->push_back(gr.bitRandom());
  }

  /**********************************************************************/
  /**
   * No local data, hence we can rely on the parent class.
   * 
   * \param cp A copy of another GBitCollection object
   */
  GBitCollection::GBitCollection(const GBitCollection& cp)
    :GTemplateValueCollection<GenEvA::bit>(cp)
  { /* nothing */ }
	
  /**********************************************************************/
  /**
   * The standard destructor. No local data, hence it is empty.
   */
  GBitCollection::~GBitCollection()
  { /* nothing */ }

  /**********************************************************************/
  /** 
   * A standard assignment operator for  GBitCollection objects.
   * 
   * \param cp A copy of another GBitCollection object
   */
  const GBitCollection& GBitCollection::operator=(const GBitCollection& cp){
    GBitCollection::load(&cp);
    return *this;
  }

  /**********************************************************************/
  /** 
   * A standard assignment operator for  GBitCollection objects,
   * camouflaged as GObject objects.
   * 
   * \param cp A copy of another GBitCollection object
   */
  const GObject& GBitCollection::operator=(const GObject& cp){
    GBitCollection::load(&cp);
    return *this;	
  }

  /**********************************************************************/
  /**
   * Resets the class to its initial state. As we have no local data,
   * all work is done by the parent class.
   */
  void GBitCollection::reset(void){
    GTemplateValueCollection<bit>::reset();
  }

  /**********************************************************************/
  /**
   * Loads the data of another GBitCollection object, camouflaged as 
   * a GObject.
   * 
   * \param gb A pointer to another GBitCollection object, camouflaged as a GObject
   */
  void GBitCollection::load(const GObject * gb){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBitCollection *>(gb) == this){
      GException ge;
      ge << "In GBitCollection::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GTemplateValueCollection<bit>::load(gb);
  }

  /**********************************************************************/
  /**
   * Creates a report about the inner state of the object.
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GBitCollection::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GBitCollection : " << this << endl
	<< ws(indention) << "-----> Report from parent class GTemplateValueCollection<bit> : " << endl
	<< GTemplateValueCollection<bit>::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /**********************************************************************/

} /* namespace GenEvA */
