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

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/strong_typedef.hpp>

#ifndef GOBJECT_H_
#define GOBJECT_H_

using namespace std;
using namespace boost;
using namespace boost::serialization;
using boost::lexical_cast;
using boost::bad_lexical_cast;

#include "GRandom.hpp"
#include "GTemplateHelperFunctions.hpp"
#include "GHelperFunctions.hpp"
#include "GException.hpp"
#include "GLogStreamer.hpp"


namespace GenEvA
{

  /**
   * These two values are likely useful for many scenarios, hence they are
   * defined in the base class'es header file.
   */
  const bool EVALUATE = false;
  const bool MUTATE = true;

  /**
   * This variable controls the indention depth, e.g. in the 
   * assembleReport() functions.
   */
  const uint16_t NINDENTION = 2;

  /**************************************************************************************************/
  /**
   * GObject is the parent class for the majority of GenEvA classes. Fundamentally, GObject gives a 
   * GenEvA class the ability to carry a name and defines a number of interface functions.
   * The GObject::reset(void) , GObject::load(const GObject *) and  GObject::clone(void)
   * member functions should be reimplemented for each derived class. Unfortunately,
   * there is no way to enforce this in C++. Further common functionality of all GenEvA classes will 
   * be implemented here over time.
   */
  class GObject
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("_geneva_object_name",GObject::_geneva_object_name);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GObject(void) throw();
    /** \brief A standard copy constructor */
    GObject(const GObject& cp) throw();
    /** \brief Initialization by name */
    explicit GObject(const string& geneva_object_name) throw();
    /** \brief The destructor */
    virtual ~GObject();

    /** \brief The standard assignment operator */
    virtual const GObject& operator=(const GObject&);

    /** \brief Reset the GObject class to its initial state upon initialization */
    virtual void reset(void);
    /** \brief Reload internal values from another GObject class */
    virtual void load(const GObject *gb);
    /** \brief Create a clone of this class. */
    virtual GObject *clone(void) = 0;

    /** \brief Emit information about this class */
    void report(void) throw();

    /** \brief Returns the name of a GenEvA object */
    string name(void) const throw();
    /** \brief Sets the name of a GenEvA object */
    void setName(const string& geneva_object_name) throw();

    /** \brief Assemble information about this class */
    virtual string assembleReport(uint16_t indention = 0) const;
  protected:
    /**
     * \brief A random number generator. Each GenEvA object has
     * its own instance with a separate seed. Note that the actual
     * calculation is done in a random number server. Note that the 
     * GRandom object will not be serialized. This means that objects
     * created from serialization data will re-initialize the random
     * number generator.
     */
    GRandom gr;
	
    /**
     * \brief Every derived class should be able to log events. Note 
     * that the log streamer object is not part of the serialization 
     * process.
     */
    GLogStreamer gls;

  private:
    /** \brief The internal representation of a GenEvA object's name */
    string _geneva_object_name;
  };

  /**************************************************************************************************/

} /* namespace GenEvA */


/**************************************************************************************************/
/**
 * \brief Needed for Boost.Serialization
 */

#if BOOST_VERSION <= 103500

BOOST_IS_ABSTRACT(GenEvA::GObject)

#else

BOOST_SERIALIZATION_ASSUME_ABSTRACT(GenEvA::GObject)

#endif /* Serialization traits */

/**************************************************************************************************/

#endif /*GOBJECT_H_*/
