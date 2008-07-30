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

#include "GSimpleMemberAdaptor.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GSimpleMemberAdaptor)


namespace GenEvA
{

  /********************************************************************************************/
  /**
   * The default constructor. As every adaptor is required to have a name, we label it private.
   * This way the GSimpleMemberAdaptor::GSimpleMemberAdaptor(string) constructor has to be called.
   */
  GSimpleMemberAdaptor::GSimpleMemberAdaptor() throw()
    :GTemplateAdaptor<GMember_ptr>("GSimpleMemberAdaptor")
  { /* nothing */ }

  /********************************************************************************************/
  /**
   * The standard constructor. Every adaptor needs a name - it is supplied as an argument to
   * this function and passed to the GTemplateAdaptor<GMember_ptr> constructor.
   * 
   * \param name The name of the adaptor
   */
  GSimpleMemberAdaptor::GSimpleMemberAdaptor(string name) throw()
    :GTemplateAdaptor<GMember_ptr>(name)
  { /* nothing */ }

  /********************************************************************************************/
  /**
   * A standard copy constructor. No local data, so we can just call the parent class'es 
   * copy constructor. 
   * 
   * \param cp Another GSimpleMemberAdaptor object
   */
  GSimpleMemberAdaptor::GSimpleMemberAdaptor(const GSimpleMemberAdaptor& cp) throw()
    :GTemplateAdaptor<GMember_ptr>(cp)
  { /* nothing */ }

  /********************************************************************************************/
  /**
   * A standard destructor. As we have no local, dynamically allocated data, it is just empty.
   */
  GSimpleMemberAdaptor::~GSimpleMemberAdaptor()
  { /* nothing */ }

  /*******************************************************************************************/
  /** 
   * A standard assignment operator for GSimpleMemberAdaptor objects,
   * 
   * \param cp A copy of another GSimpleMemberAdaptor object
   */
  const GSimpleMemberAdaptor& GSimpleMemberAdaptor::operator=(const GSimpleMemberAdaptor& cp){
    GSimpleMemberAdaptor::load(&cp);
    return *this;
  }
	
  /*******************************************************************************************/	
  /** 
   * A standard assignment operator for GSimpleMemberAdaptor objects, camouflaged as a GObject
   * 
   * \param cp A copy of another GSimpleMemberAdaptor object, camouflaged as a GObject
   */	
  const GObject& GSimpleMemberAdaptor::operator=(const GObject& cp){
    GSimpleMemberAdaptor::load(&cp);
    return *this;	
  }

  /********************************************************************************************/
  /**
   * Resets the object to its initial state. As we have no local data, all work is done in
   * the parent class'es reset function.
   */
  void GSimpleMemberAdaptor::reset(void)
  {
    GTemplateAdaptor<GMember_ptr>::reset();
  }

  /********************************************************************************************/
  /**
   * Loads the data of another GSimpleMemberAdaptor (camouflaged as a GObject) into this class. 
   * As we have no local data, all work is done in the parent class'es load() function. Note
   * that, unlike most other GObject-derivatives, we only use a static_cast here. This is because
   * there is no local data, so we do not need a safe conversion (the other object's data isn't
   * touched in this function.   
   * 
   * \param gb A pointer to another GSimpleMemberAdaptor object, camouflaged as a GObject
   */
  void GSimpleMemberAdaptor::load(const GObject *gb)
  {
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GSimpleMemberAdaptor *>(gb) == this){
      gls << "In GSimpleMemberAdaptor::load(): Error!" << endl
	  << "Tried to assign an object to itself." << endl
	  << logLevel(CRITICAL);

      exit(1);
    }
	
    GTemplateAdaptor<GMember_ptr>::load(gb);
  }

  /********************************************************************************************/
  /**
   * Creates a deep copy of this class.
   * 
   * \return A deep copy of this GSimpleMemberAdaptor object
   */
  GObject *GSimpleMemberAdaptor::clone(void)
  {
    return new GSimpleMemberAdaptor(*this);
  }

  /********************************************************************************************/
  /**
   * Assembles a report about the inner state of this object. As we have no local data, only 
   * the parent class'es assembleReport() function gets called.
   *
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GSimpleMemberAdaptor::assembleReport(uint16_t indention ) const
  {
    ostringstream oss;

    oss << ws(indention) << "GSimpleMemberAdaptor<T>: " << this << endl
	<< ws(indention) << "-----> Report from parent class GTemplateAdaptor<GMember_ptr> : " << endl
	<< GTemplateAdaptor<GMember_ptr>::assembleReport(indention+NINDENTION) << endl;

    return oss.str();
  }

  /********************************************************************************************/
  /**
   * Mutation of a GMember can be achieved by using its own mutate() function (which in turn
   * will call the customMutate() function supplied by the user for that GMember object. This
   * is a generic way of dealing with the mutation of GMember objects. This function is only
   * called in a context where exception handling is already implemented. So we do not replicate
   * this here.
   * 
   * \param value The GMember object that should be mutated (wrapped into a GMember_ptr object).
   */
  void GSimpleMemberAdaptor::customMutate(GMember_ptr& value){
    value->mutate();
  }

  /********************************************************************************************/

} /* namespace GenEvA */
