/**
 * @file GLockVarT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard headers go here

#include <string>
#include <iostream>
#include <fstream>
#include <deque>
#include <list>
#include <algorithm>
#include <stdexcept>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

#ifndef GLOCKVART_HPP_
#define GLOCKVART_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class allows to restrict changes to a variable to defined places,
 * where the class was unlocked. Attempts to change the variable while it
 * is locked will result in an exception being thrown. The object will not
 * change its "locked" state irrespective of interaction with the environment
 * (e.g. copying, copy-construction, etc.). I.e., in order to assign values,
 * the object needs to be explicitly unlocked, and the person doing this
 * needs to take care to lock the object again. There is a default value
 * for the contained variable, that may only be set through the constructor.
 * A default value must always be set, as no public default constructor is
 * provided. Note that, if you want to serialize instances of this class
 * for non-standard types, you will need to supply the necessary BOOST_CLASS_EXPORT_KEY/IMPLEMENT
 * combo yourself.
 */
template <typename T>
class GLockVarT {
   /////////////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;

     ar
     & BOOST_SERIALIZATION_NVP(var_)
     & BOOST_SERIALIZATION_NVP(locked_);
   }
   /////////////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * Initialization with a given value. The default value will be set to "var"
    */
   GLockVarT(const T& var)
      : var_(var)
      , default_(var)
      , locked_(true) // locked by default
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Copy construction
    */
   GLockVarT(const GLockVarT<T>& cp)
      : var_(cp.var_)
      , default_(cp.default_)
      , locked_(true) // locked by default, even if "foreign" object isn't locked
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GLockVarT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Assignment of another object. This function will throw if the object is
    * locked. The default value will remain untouched.
    */
   const GLockVarT<T>& operator=(const GLockVarT<T>& cp) {
      if(locked_) {
         glogger
         << "In GLockVarT<T>& operator=(GLockVarT<T>& cp): Error!" << std::endl
         << "Tried to assign variable while access is locked" << std::endl
         << GEXCEPTION;
      }

      var_ = cp.value();
      // Note that we do not change the "locked" status, irrespective of what the status of cp is

      return *this;
   }

   /***************************************************************************/
   /**
    * Assignment of a given instance of T (not wrapped into a GLockVarT<T> object
    */
   const T& operator=(const T& var) {
      if(locked_) {
         glogger
         << "In GLockVarT<T>& operator=(GLockVarT<T>& cp): Error!" << std::endl
         << "Tried to assign variable while access is locked" << std::endl
         << GEXCEPTION;
      }

      var_ = var;
      return var_;
   }

   /***************************************************************************/
   /**
    * Explicit setting of value (always possible, even if access is locked. Note
    * that a call to this function will not change the "locked" state of this
    * object.
    */
   void setValue(const T& var) {
      var_ = var;
   }

   /***************************************************************************/
   /**
    * Value retrieval
    */
   const T& value() const {
      return var_;
   }

   /***************************************************************************/
   /**
    * Value retrieval
    */
   const T& operator()() const {
      return this->value();
   }

   /***************************************************************************/
   /**
    * Automatic conversion to target type, e.g. for calculations
    */
   operator T() const {
      return var_;
   }

   /***************************************************************************/
   /**
    * Locking
    */
   void lock() {
      locked_ = true;
   }

   /***************************************************************************/
   /**
    * Locking with a specific value
    */
   void lockWithValue(const T& var) {
      locked_ = true;
      var_ = var;
   }

   /***************************************************************************/
   /**
    * Unlocking
    */
   void unlock() {
      locked_ = false;
   }

   /***************************************************************************/
   /**
    * Unlocking with a specific value
    */
   void unlockWithValue(const T& var) {
      locked_ = false;
      var_ = var;
   }

   /***************************************************************************/
   /**
    * Check if the object is locked
    */
   bool isLocked() const {
      return locked_;
   }

   /***************************************************************************/
   /**
    * Returns the object to a locked state with its default value
    */
   void reset() {
      locked_ = true;
      var_ = default_;
   }

protected:
   /***************************************************************************/
   /**
    * The default constructor. The default value will be set to 0. This constructor
    * is only needed for (de-)serialization.
    */
   GLockVarT()
      : var_(T(0))
      , default_(T(0))
      , locked_(true) // locked by default
   { /* nothing */ }

private:
   /***************************************************************************/
   T var_; ///< Holds the actual parameter value
   const T default_; ///< Holds a default value for var_

   bool locked_; ///< Allows to lock access to the variable, so that it cannot be changed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

class GLockVarBool
   : public GLockVarT<bool>
{
   /////////////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;

     ar
     & make_nvp("GLockVarT_bool", boost::serialization::base_object<GLockVarT<bool> >(*this));
   }
   /////////////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard constructor */
   GLockVarBool(const bool&);
   /** @brief The copy constructor */
   GLockVarBool(const GLockVarBool&);
   /** @brief The destructor */
   ~GLockVarBool();

protected:
   /** @brief The default constructor -- intentionally protected */
   GLockVarBool();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
// Some code for serialization purposes
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarBool)

BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<bool>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<float>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<double>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<short>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<int>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<long>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<unsigned short>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<unsigned int>)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GLockVarT<unsigned long>)

/******************************************************************************/
#endif /* GLOCKVART_HPP_ */
