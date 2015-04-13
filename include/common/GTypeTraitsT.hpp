/**
 * @file GTypeTraitsT.hpp
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


// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <deque>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>

// Boost headers go here
#include <boost/shared_ptr.hpp>
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>


#ifndef INCLUDE_COMMON_GTYPETRAITST_HPP_
#define INCLUDE_COMMON_GTYPETRAITST_HPP_


// Geneva headers go here
#include "common/GExpectationChecksT.hpp"

namespace Gem {
namespace Common {

#ifndef _MSC_VER

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a compare function. This is
 * used to distinguish compare functions that act on types through their own
 * compare() from compare-variants that act through operator==/!= .
 */
template <typename T>
class has_compare_member
{
    typedef char yes;
    typedef long no;

    // TODO: Replace BOOST_TYPEOF with decltype when switch to C++11 is complete
    template <typename C> static yes test( BOOST_TYPEOF(&C::compare) ) ;
    template <typename C> static no  test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* _MSC_VER */

} /* namespace Common */
} /* namespace Gem */

#endif /* INCLUDE_COMMON_GTYPETRAITST_HPP_ */
