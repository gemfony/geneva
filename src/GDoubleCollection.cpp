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

#include "GDoubleCollection.hpp"

/** 
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GDoubleCollection)


namespace GenEvA
{

  /**********************************************************************/
  /**
   * The default constructor.
   */
  GDoubleCollection::GDoubleCollection()
    :GTemplateValueCollection<double>()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Fills the class with nval random double values
   * 
   * \param nval Number of values to put into the vector
   */
  GDoubleCollection::GDoubleCollection(int16_t nval)
    :GTemplateValueCollection<double>()
  {
    for(int16_t i= 0; i<nval; i++){
      this->push_back(gr.evenRandom(-DEFINIT,DEFINIT));
    }
  }

  /**********************************************************************/
  /**
   * Fills the class with nval random double values between min and max.
   * 
   * \param nval Number of values to put into the vector
   * \param min The lower boundary for random entries
   * \param max The upper boundary for random entries 
   */
  GDoubleCollection::GDoubleCollection(int16_t nval, double min, double max)
    :GTemplateValueCollection<double>()
  {
    for(int16_t i= 0; i<nval; i++){
      this->push_back(gr.evenRandom(min,max));
    }
  }

  /**********************************************************************/
  /**
   * The standard copy constructor. All work is done by the parent class.
   * 
   * \param cp A copy of another GDoubleCollection object
   */
  GDoubleCollection::GDoubleCollection(const GDoubleCollection& cp)
    :GTemplateValueCollection<double>(cp)
  { /* nothing */ }

  /**********************************************************************/
  /**
   * The standard destructor. All work will be done by the parent class. 
   */
  GDoubleCollection::~GDoubleCollection()
  { /* nothing */ }

  /**********************************************************************/
  /** 
   * A standard assignment operator. We have no local data, so all work
   * is done by the parent class.
   * 
   * \param cp A copy of another GDoubleCollection object
   */
  const GDoubleCollection& GDoubleCollection::operator=(const GDoubleCollection& cp){
    GDoubleCollection::load(&cp);
    return *this;
  }

  /**********************************************************************/	
  /**
   * Creates a deep copy of this object
   */
  GObject *GDoubleCollection::clone(void){
    return new GDoubleCollection(*this);
  }

  /**********************************************************************/	
  /** 
   * An assignment operator for the base class GObject. This is needed, as
   * we are working in a highly polymorphic environment. 
   */
  const GObject& GDoubleCollection::operator=(const GObject& cp){
    GDoubleCollection::load(&cp);
    return *this;
  }

  /**********************************************************************/
  /**
   * Resets the class to its original values. As we have no local, dynamically
   * allocated data, all work is done by the parent class.
   */
  void GDoubleCollection::reset(void){
    GTemplateValueCollection<double>::reset();
  }

  /**********************************************************************/
  /**
   * Loads the data of another GDoubleCollection object,
   * camouflaged as a GObject. We have no local data, so 
   * all we need to do is to the standard identity check,
   * preventing that an object is assigned to itself.
   * 
   * \param gm A copy of another GDoubleCollection object, camouflaged as a GObject
   */
  void GDoubleCollection::load(const GObject * gm){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GDoubleCollection *>(gm) == this){
      GException ge;
      ge << "In GDoubleCollection::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GTemplateValueCollection<double>::load(gm);
  }

  /**********************************************************************/
  /**
   * Appends nval random values between min and max to this class.
   * 
   * \param nval Number of values to put into the vector
   * \param min The lower boundary for random entries
   * \param max The upper boundary for random entries 
   */
  void GDoubleCollection::addData(int16_t nval, double min, double max){
    for(int16_t i= 0; i<nval; i++){
      this->push_back(gr.evenRandom(min,max));
    }
  }

  /************************************************************************/
  /**
   * This function assembles a report about the class'es internal state and then calls the 
   * parent classes' assembleReport() function. As we do not know what values are stored in
   * our vector and whether they are suitable for a stream, we do not emit information about them.
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GDoubleCollection::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GDoubleCollection: " << this << endl
	<< ws(indention) << "Stored values:" << endl
        << ws(indention);

    vector<double>::const_iterator it;
    uint16_t counter=0;
    for(it=this->begin(); it!=this->end(); ++it){
      oss << *it << " ";
      if(counter++ == 5){
	oss << endl;
	counter=0;
      }
    }

    oss	<< ws(indention) << "-----> Report from parent class GTemplateValueCollection<double> : " << endl
	<< GTemplateValueCollection<double>::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /**********************************************************************/

} /* namespace GenEvA */
