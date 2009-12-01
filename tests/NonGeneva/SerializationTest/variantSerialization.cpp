/**
 * @file variantSerialization.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

/*
 * This code illustrates how to build a property map based on boost::variant
 * that is serializable.
 *
 * Compile with a command similar to
 * g++ -o variantSerialization -I/opt/boost141/include -L/opt/boost141/lib -lboost_serialization variantSerialization.cpp
 */

#include <typeinfo>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

#include <boost/variant.hpp>
#include <boost/cstdint.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

class attributeTester {
  ///////////////////////////////////////////////////////////////////////
  friend class boost::serialization::access;
  
  template<typename Archive>
  void serialize(Archive & ar, const unsigned int){
    using boost::serialization::make_nvp;
    ar & make_nvp("attributeTable_", attributeTable_);
  }
  ///////////////////////////////////////////////////////////////////////
  
public:
  /*******************************************************************/
  /** @brief Adds an attribute to the individual */
  template <typename T>
  T setAttribute(const std::string& key, const T& value) {
    // Do some preparatory checks
    if(typeid(T) != typeid(std::string) &&
       typeid(T) != typeid(boost::int32_t) &&
       typeid(T) != typeid(double) &&
       typeid(T) != typeid(bool)) {
      std::cout << "In attributeTester::setAttribute: Error: bad type given for attribute: " << typeid(T).name() << std::endl;
      exit(1);
    }
    
    // Add a suitable variant to the map
    boost::variant<std::string, boost::int32_t, double, bool> attribute(value);
    attributeTable_[key] = attribute;
  
    return value;
  }

  /*******************************************************************/
  /** @brief Retrieves an attribute from the individual */
  template <typename T>
  bool getAttribute(const std::string& key, T& value) {
    if(this->hasAttribute(key)) {
      value = boost::get<T>(attributeTable_[key]); // will throw if T is not a valid type
      return true; // found key
    }
    else 
      return false; // key not found
  }

  /*******************************************************************/
  /** @brief Removes an attribute from the individual */
  bool delAttribute(const std::string& key) {
    if(this->hasAttribute(key)) {
      attributeTable_.erase(key);
      return true;
    }
    else
      return false;
  }

  /*******************************************************************/
  /** @brief Checks whether a given attribute is present */
  bool hasAttribute(const std::string& key) {
    if(attributeTable_.find(key) != attributeTable_.end()) return true;
    return false;
  }

  /*******************************************************************/
  /** @brief Clears the attribute table */
  void clearAttributes() {
    attributeTable_.clear();
  }

private:
  /*******************************************************************/
  std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> > attributeTable_;
};

int main() {
  std::string eins;
  boost::int32_t zwei;
  double drei;
  bool vier;

  attributeTester aT, aT2;

  aT.setAttribute("eins",std::string("eins"));
  aT.setAttribute("zwei", 2);
  aT.setAttribute("drei", 3.);
  aT.setAttribute("vier", true);

  std::ofstream serialAttributesOut("serialAttributes.xml");
  {
    boost::archive::xml_oarchive oa(serialAttributesOut);
    oa << boost::serialization::make_nvp("aT" , aT);
  }
  serialAttributesOut.close();

  std::ifstream serialAttributesIn("serialAttributes.xml");
  {
    boost::archive::xml_iarchive ia(serialAttributesIn);
    ia >> boost::serialization::make_nvp("aT" , aT2);
  }
  serialAttributesIn.close();

  aT2.getAttribute("eins", eins);
  aT2.getAttribute("zwei", zwei);
  aT2.getAttribute("drei", drei);
  aT2.getAttribute("vier", vier);

  std::cout << eins << " " << zwei << " " << drei << " " << vier << std::endl;
}
