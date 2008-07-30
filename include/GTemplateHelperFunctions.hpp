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

#include <sstream>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>


#ifndef GTEMPLATEHELPERFUNCTIONS_H_
#define GTEMPLATEHELPERFUNCTIONS_H_

using namespace std;

#include "GException.hpp"
#include "GHelperFunctions.hpp"

namespace GenEvA
{

  /************************************************************************/
  /**
   * \brief Little helper function needed to find the minimum of two values. Note that this
   * function will return a even if it is equal to b.
   *
   * \param a value used for the comparison
   * \param b value used for the comparison
   */
  template <class T>
  T GMin(T a, T b){
    return a<=b?a:b;
  }

  /************************************************************************/
  /**
   * \brief Little helper function needed to find the maximum of two values. Note that this
   * function will return a only if it is larger than (not equal to) b.
   *
   * \param a value used for the comparison
   * \param b value used for the comparison
   */
  template <class T>
  T GMax(T a, T b){
    return a>b?a:b;
  }

  /************************************************************************/
  /**
   * \brief Converts arbitrary types to a string. Note that this implies
   * that the type carries the Boost.serialization infrastructure. The function
   * also deletes val, if we tell it to,
   */
  template <class T>
  string GToString(T *val, bool del = false){
    // Is the pointer valid ?
    if(!val){
      GException ge;
      ge << "In GToString() : Target pointer invalid!" << endl
	 << raiseException;
    }

    ostringstream ostr;
    assert(ostr.good());

    {
      boost::archive::xml_oarchive oa(ostr);
      oa << make_nvp("top",val);
    } // oa's destructor will be invoked here, putting the end-tag into the stream
	
    // delete val if requested
    if(del) delete val;
	
    return ostr.str();
  }

  /************************************************************************/
  /**
   * \brief Converts arbitrary types from a string to an obect of the same
   * type. Note that this implies that the type carries the Boost.serialization
   * infrastructure. Serialization of these objects must have been done wit
   * GToString() .
   */
  template <class T>
  void GStringToObject(string descr, T* target){
    // Is the pointer valid ?
    if(!target){
      GException ge;
      ge << "In GStringToObject() : Target pointer invalid!" << endl
	 << raiseException;
    }

    istringstream istr(descr);
    assert(istr.good());

    {
      boost::archive::xml_iarchive ia(istr);
      ia >> make_nvp("top",target);
    }
  }

  /************************************************************************/


} /* namespace GenEvA */

#endif /* GTEMPLATEHELPERFUNCTIONS_H_*/
