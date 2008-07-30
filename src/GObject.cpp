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

#include "GObject.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GObject)


using namespace std;

namespace GenEvA
{

  /**************************************************************************************************/
  /**
   * The default constructor initializes the internal values of this class.
   * In particular, it sets the name of the Geneva object to "GObject"
   */
  GObject::GObject(void) throw()
  {
    this->_geneva_object_name = "GObject";
  }

  /**************************************************************************************************/
  /**
   * We need a copy constructor, even though this class is purely virtual. It is called by child
   * classes, when their copy constructor is executed. Note that this function is not implemented
   * with the load() function. (REASON: A copy constructor needs to call the constructor
   * of its parent class. Using a load function in the copy constructor might replicate some
   * copying. Hence, throughout the Geneva library, we do not use the load function for copying
   * in the copy constructor).
   * 
   * \param cp A copy of another GObject object
   */
  GObject::GObject(const GObject& cp) throw()
  {
    this->_geneva_object_name = cp._geneva_object_name;
  }

  /**************************************************************************************************/
  /**
   * If a particular name is desired for an object of this class, it can be
   * set upon initialization with this function.
   *
   * \param geneva_object_name The name which is assigned to a Geneva object
   */
  GObject::GObject(const string& geneva_object_name) throw()
  {
    this->_geneva_object_name = geneva_object_name;
  }

  /**************************************************************************************************/
  /**
   * As no memory is dynamically allocated in GObject, no work has to
   * be done in the destructor at the moment.
   */
  GObject::~GObject()
  { /* nothing */ }

  /**************************************************************************************************/
  /** 
   * A standard assignment operator for GObject objects,
   * 
   * \param cp A copy of another GObject object
   * \return A constant reference to this object
   */	
  const GObject& GObject::operator=(const GObject& cp){
    GObject::load(&cp);
    return *this;
  }

  /**************************************************************************************************/
  /**
   * It is the purpose of the reset() function to reset a GObject object to
   * exactly the state it had upon creation with the default constructor.
   * <b>Important: This function needs to be re-implemented in each derived
   * class for Geneva to work properly.<\b> There is unfortunately no way
   * to enforce this in C++.
   *
   * Remark: The object will actually not be in exactly the same state as 
   * after its creation, as the random number and log streamer objects are
   * not being reset.
   */
  void GObject::reset(void)
  {
    this->_geneva_object_name = "GObject";
  }

  /**************************************************************************************************/
  /** 
   * This function loads the internal data of another GObject object. <b>Important: 
   * This function needs to be re-implemented in each derived class for Geneva to work 
   * properly.<\b> There is unfortunately no way this can be enforced in C++.
   *
   * Please note that this function takes a pointer to a GObject object rather than
   * a constant reference.
   *
   * \param gb A pointer to a GObject object
   */
  void GObject::load(const GObject * gb)
  {
    // Check that this object is not accidentally assigned to itself.
    // Note that we terminate the application if this happens - it 
    // shouldn't and we do not want to be tolerant towards problems
    // in the library.
    if(gb == this){
      gls << "In GObject::load(): Error!" << endl
	  << "Tried to assign an object to itself." << endl
	  << logLevel(CRITICAL);

      exit(1);
    }
	
    this->_geneva_object_name = gb->_geneva_object_name;
  }

  /**************************************************************************************************/
  /**
   * Emits information about this class. It should not be necessary to reimplement this function
   * in derived classes.
   */
  void GObject::report(void) throw(){
    try{
      gls  << "***********************************************************" << endl
	   << "Report from class " << typeid(*this).name() << endl
	   << "with address " << this << " :" << endl
	   << "+++++++++++++++++++++++++++++++++++++++" << endl  
	   << this->assembleReport()
	   << "***********************************************************" << endl
	   << logLevel(REPORT);
    }
    catch(bad_typeid& b){
      gls << "In GObject::report(): Error!" << endl
	  << "Caught \"bad_typeid\" exception with message" << endl
	  << b.what() << endl
	  << logLevel(CRITICAL);

      exit(1);
    }
    catch(...){
      gls << "In GObject::report(): Error!" << endl
	  << "Caught unknown exception" << endl
	  << logLevel(CRITICAL);
      
      exit(1);
    }
  }

  /**************************************************************************************************/
  /**
   * Every class in the Geneva library should implement the assembleReport function and recursivley
   * call the parent class'es assembleReport function. This way, where needed, we can get a full view
   * of the libraries status in short time.
   *
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GObject::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GObject: " << this << endl
	<< ws(indention) << "name = " << name() << endl;

    return oss.str(); 
  }

  /**************************************************************************************************/
  /**
   * A name is assigned to every object derived directly or indirectly from GObject. It can be
   * retrieved with this function.
   *
   * \return The name of a GObject object or an object derived from it
   */
  string GObject::name(void) const throw()
  {
    return _geneva_object_name;
  }

  /**************************************************************************************************/
  /**
   * This function allows you to set the name of a GObject object or an object derived from it.
   *
   * \param geneva_object_name The name of a GObject object or an object derived from it.
   */
  void GObject::setName(const string& geneva_object_name) throw()
  {
    _geneva_object_name = geneva_object_name;
  }


  /**************************************************************************************************/

} /* namespace GenEvA */
