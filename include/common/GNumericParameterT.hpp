/**
 * @file GNumericParameterT.hpp
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
 * the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General
 * Public License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS DUAL-LICENSING OPTION DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * version 3 and of version 2 of the GNU General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Standard headers go here
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cast.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/noncopyable.hpp>

#ifndef GNUMERICPARAMETERT_HPP_
#define GNUMERICPARAMETERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GLogger.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class allows to store a numeric parameter plus possible boundaries (both
 * included in the allowed value range). If the upper- and lower boundary are both
 * set to equal values, then no boundaries are assumed to be present. The class is
 * used in conjunction with the communication with external programs used for
 * evaluation.
 */
template <typename T>
class GNumericParameterT
   :boost::noncopyable
{
   // Make sure T has a suitable type
   BOOST_MPL_ASSERT((boost::is_arithmetic<T>)); // This includes bool

public:
   /***************************************************************************/
    /**
     * The default constructor.
     */
    GNumericParameterT()
       : param_(T(0))
       , lowerBoundary_(T(0))
       , upperBoundary_(T(0))
       , name_("none")
       , randomInit_(false)
    { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
    virtual ~GNumericParameterT()
    { /* nothing */ }

   /***************************************************************************/
   /**
    * Allows to set the name of this parameter
    */
   void setName(const std::string& name) {
      name_ = name;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the name of the parameter
    */
   std::string getName() const {
      return name_;
   }

   /***************************************************************************/
   /**
    * Marks the parameter as having to be ramdomly initialized (or not)
    */
   void setRandomInit(const bool& randomInit) {
      randomInit_ = randomInit;
   }

   /***************************************************************************/
   /**
    * Allows to check whether the parameter is marked for random initialization
    */
   bool getRandomInit() const {
      return randomInit_;
   }

   /***************************************************************************/
   /**
    * Sets the parameter to a user-defined value. This function requires that
    * either the new value is inside existing boundaries or that boundaries
    * have not been set.
    *
    * @param param The value of the parameter
    */
   void setParameter(const T& param){
      // Check the validity of the new variable
      if(lowerBoundary_ != upperBoundary_ && (param_ < lowerBoundary_ || param_ > upperBoundary_)) {
         glogger
         << "In GNumericParameterT<T>::setParameter(const T&): Error" << std::endl
         << "Invalid value received:" << std::endl
         << "param_ = " << param_ << std::endl
         << "lowerBoundary_ = " << lowerBoundary_ << std::endl
         << "upperBoundary_ = " << upperBoundary_ << std::endl
         << "Leaving ..." << std::endl
         << GEXCEPTION;
      }

      // Set the parameter- and boundary values;
      param_ = param;
   }

   /***************************************************************************/
   /**
    * Sets the parameter and boundaries to a user-defined value
    *
    * @param param The value of the parameter
    * @param lower The value of the lower boundary
    * @param upper The value of the upper boundary
    */
   void setParameter(
         const T& param
         , const T& lower
         , const T& upper
   ){
      // Check the validity of the boundaries
      if(lowerBoundary_ != upperBoundary_ && (param_ < lowerBoundary_ || param_ > upperBoundary_ || lowerBoundary_ >= upperBoundary_)) {
         glogger
         << "In GNumericParameterT<T>::setParameter(param, lower, upper): Error!" << std::endl
         << "Invalid boundary and/or parameter values:" << std:: endl
         << "param_ = " << param_ << std::endl
         << "lowerBoundary_ = " << lowerBoundary_ << std::endl
         << "upperBoundary_ = " << upperBoundary_ << std::endl
         << "Leaving ... " << std::endl
         << GEXCEPTION;
      }

      // Set the parameter- and boundary values;
      param_ = param;
      lowerBoundary_ = lower;
      upperBoundary_ = upper;
   }

   /***************************************************************************/
   /**
    * Sets all class variables in one go
    */
   void setAll(
         const T& param
         , const T& lower
         , const T& upper
         , const std::string& name
   ) {
      this->setParameter(param, lower, upper);
      this->setName(name);
   }

   /***************************************************************************/
   /**
    * Retrieve the parameter value
    *
    * @return The current value of the param_ variable
    */
   T value() const {
      return param_;
   }

   /***************************************************************************/
   /**
    * Retrieves the lower boundary assigned to this parameter
    *
    * @return The current value of the lower boundary
    */
   T getLowerBoundary() const {
      return lowerBoundary_;
   }

   /***************************************************************************/
   /**
    * Retrieves the upper boundary assigned to this parameter
    *
    * @return The current value of the upper boundary
    */
   T getUpperBoundary() const {
      return upperBoundary_;
   }

   /***************************************************************************/
   /**
    * Retrieves both boundaries in one go
    */
   boost::tuple<T,T> getBoundaries() const {
      return boost::tuple<T,T>(lowerBoundary_, upperBoundary_);
   }

   /***************************************************************************/
   /**
    * Checks if the parameter has boundaries defined
    *
    * @return A boolean indicating whether boundaries have been defined
    */
   bool hasBoundaries() {
      if(lowerBoundary_ == upperBoundary_) return false;
      return true;
   }

   /***************************************************************************/
   /**
    * Retrieves the parameter type name as a string. This function needs to
    * be overloaded for different parameter types.
    */
   virtual std::string getParameterType() const {
      glogger
      << "In GNumericParameterT::getParameterType(): Function called for unknown type."
      << GEXCEPTION;

      // Make the compiler happy
      return std::string();
   }

   /***************************************************************************/
   /**
    * Writes the class'es data to a property tree
    */
   void toPropertyTree(
         const std::string& baseName
        , boost::property_tree::ptree& pt
   ) const {
      pt.put((baseName + ".type").c_str(), this->getParameterType());
      pt.put((baseName + ".value").c_str(), this->value());
      pt.put((baseName + ".lowerBoundary").c_str(), lowerBoundary_);
      pt.put((baseName + ".upperBoundary").c_str(), upperBoundary_);
      pt.put((baseName + ".name").c_str(), name_);
      pt.put((baseName + ".randomInit").c_str(), randomInit_);
   }

   /***************************************************************************/
   /**
    * Reads the class'es data from a property (sub-)tree
    *
    * @param str The external input string holding the data
    */
   void fromPropertyTree(
         const std::string& baseName
         , const boost::property_tree::ptree& pt
   ) {
      param_ = pt.get<T>(baseName + ".value");
      lowerBoundary_ = pt.get<T>(baseName + ".lowerBoundary");
      upperBoundary_ = pt.get<T>(baseName + ".upperBoundary");
      name_ = pt.get<std::string>(baseName + ".name");
      randomInit_ = pt.get<bool>(baseName + ".randomInit");

#ifdef DEBUG
      // Check that the parsed parameter type is identical to our local type
      std::string storedType = pt.get<std::string>(baseName + ".type");
      if(storedType != this->getParameterType()) {
         glogger
         << "In GNumericParameterT<T>::fromPropertyTree(): Error!" << std::endl
         << "Encountered invalid parameter type: \"" << storedType << "\"," << std::endl
         << "Expected \"" << this->getParameterType() << "\"" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */
   }

private:
   /***************************************************************************/
   T param_; ///< The actual parameter value
   T lowerBoundary_; ///< The lower boundary allowed for param_
   T upperBoundary_; ///< The upper boundary allowed for param_
   std::string name_; ///< A name assigned to this parameter
   bool randomInit_; ///< Determines whether a random value should be used instead of the stored value
};

// Declaration of specializations for various allowed types
template <> GNumericParameterT<bool>::GNumericParameterT();
template <> void GNumericParameterT<bool>::setParameter(const bool&, const bool&, const bool&);

template <> std::string GNumericParameterT<double>::getParameterType() const;
template <> std::string GNumericParameterT<boost::int32_t>::getParameterType() const;
template <> std::string GNumericParameterT<bool>::getParameterType() const;

} /* namespace Common */
} /* namespace Gem */

#endif /* GNUMERICPARAMETERT_HPP_ */
