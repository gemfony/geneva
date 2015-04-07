/**
 * @file GObjectExpectationChecksT.hpp
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

#ifndef GOBJECTEXPECTATIONCHECKST_HPP_
#define GOBJECTEXPECTATIONCHECKST_HPP_

// Geneva headers go here
#include "common/GExpectationChecksT.hpp"
#include "geneva/GObject.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two omplex types meet a given expectation. It is assumed that
 * these types have the standard Geneva interface with corresponding "compare" functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
   const geneva_type& x
   , const geneva_type& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
   , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

      // If we reach this line, then both pointers have content

      { // Check whether the content differs
         try {
            x.compare(y,e,limit);
         } catch(g_expectation_violation& g) {
            error
            << "Content of " << x_name << " and " << y_name << " differ." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
            << g.what() << std::endl;
            break; // Terminate the switch statement
         }

         // If we reach this line, the expectation was met
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
   {
      expectation_str = "CE_INEQUALITY";

      // Check whether the content differs
      try {
         x.compare(y,e,limit);
      } catch(g_expectation_violation& g) {
         // If we catch an expectation violation for expectation "inequality",
         // we simply break the switch statement so that expectationMet remains to be false
         error
         << "Content of " << x_name << " and " << y_name << " are equal/similar." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
         << g.what() << std::endl;
         break;
      }
      expectationMet = true;
   }
   break;

   default:
   {
      glogger
      << "In compare(/* 6 */): Got invalid expectation " << e << std::endl
      << GEXCEPTION;
   }
   break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
/**
 * This function checks whether two smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
   const boost::shared_ptr<geneva_type>& x
   , const boost::shared_ptr<geneva_type>& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
   , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

      // Check whether the pointers hold content
      if(x && !y) {
         error
         << "Smart pointer " << x_name << " holds content while " << y_name << " does not." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
         break; //
      } else if(!x && y) {
         error
         << "Smart pointer " << x_name << " doesn't hold content while " << y_name << " does." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
         break;  // The expectation was clearly not met
      } else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
         expectationMet = true;
         break;
      }

      // If we reach this line, then both pointers have content

      { // Check whether the content differs
         try {
            x->compare(*y,e,limit);
         } catch(g_expectation_violation& g) {
            error
            << "Content of " << x_name << " and " << y_name << " differ." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
            << g.what() << std::endl;
            break; // Terminate the switch statement
         }

         // If we reach this line, the expectation was met
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
   {
      expectation_str = "CE_INEQUALITY";

      // Check whether the pointers hold content
      if((x && !y) || (!x && y)) {
         expectationMet = true;
         break;
      } else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
         break; // The expectation was not met
      }

      // Check whether the content differs
      try {
         x->compare(*y,e,limit);
      } catch(g_expectation_violation& g) {
         // If we catch an expectation violation for expectation "inequality",
         // we simply break the switch statement so that expectationMet remains to be false
         error
         << "Content of " << x_name << " and " << y_name << " are equal/similar." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
         << g.what() << std::endl;
         break;
      }
      expectationMet = true;
   }
   break;

   default:
   {
      glogger
      << "In compare(/* 7 */): Got invalid expectation " << e << std::endl
      << GEXCEPTION;
   }
   break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two vectors of smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
   const std::vector<boost::shared_ptr<geneva_type> >& x
   , const std::vector<boost::shared_ptr<geneva_type> >& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
   , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

      // First check sizes
      if(x.size() != y.size()) {
         error
         << "Vectors " << x_name << " and " << y_name << " have different sizes " << x.size() << " / " << y.size() << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
         // Terminate the switch statement. expectationMet will be false then
         break;
      }

      // Now loop over all members of the vectors
      bool foundDeviation = false;
      typename std::vector<boost::shared_ptr<geneva_type> >::const_iterator x_it, y_it;
      std::size_t index = 0;
      for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++index) {
         // First check that both pointers have content
         // Check whether the pointers hold content
         if(*x_it && !*y_it) {
            error
            << "Smart pointer " << x_name << "[" << index << "] holds content while " << y_name << "[" << index << "]  does not." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
            foundDeviation = true;
            break; // terminate the loop
         } else if(!*x_it && *y_it) {
            error
            << "Smart pointer " << x_name << "[" << index << "] doesn't hold content while " << y_name << "[" << index << "]  does." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
            foundDeviation = true;
            break;  // terminate the loop
         } else if(!*x_it && !*y_it) { // No content to check. Both smart pointers can be considered equal
            continue; // Go on with next iteration in the loop
         }

         // At this point we know that both pointers have content. We can now check the content
         // which is assumed to have the compare() function
         try {
          (*x_it)->compare(**y_it,e,limit);
         } catch(g_expectation_violation& g) {
            error
            << "Content of " << x_name << "[" << index << "] and " << y_name << "[" << index << "] differs." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
            << g.what() << std::endl;
            foundDeviation = true;
            break; // Terminate the loop
         }
      }

      if(!foundDeviation) {
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
   {
      expectation_str = "CE_INEQUALITY";

      // First check sizes. The expectation of inequality will be met if they differ
      if(x.size() != y.size()) {
         expectationMet = true;
         break; // Terminate the switch statement
      }

      // Now loop over all members of the vectors
      bool foundInequality = false;
      typename std::vector<boost::shared_ptr<geneva_type> >::const_iterator x_it, y_it;
      for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
         // First check that both pointers have content
         // Check whether the pointers hold content
         if((*x_it && !*y_it) || (!*x_it && *y_it)) {
            foundInequality = true;
            break; // terminate the loop
         } else if(!*x_it && !*y_it) { // No content to check. Both smart pointers can be considered equal
            continue; // Go on with next iteration in the loop - there is nothing to check here
         }

         // At this point we know that both pointers have content. We can now check this content
         // which is assumed to have the compare() function
         try {
          (*x_it)->compare(**y_it,e,limit);
         } catch(g_expectation_violation& g) {
            // Go on with the next item in the vector -- the content is equal or similar
            continue;
         }
      }

      if(foundInequality) {
         expectationMet = true;
      } else {
         error
         << "The two vectors " << x_name << " and " << y_name << " are equal." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
      }
   }
   break;

   default:
   {
      glogger
      << "In compare(/* 8 */): Got invalid expectation " << e << std::endl
      << GEXCEPTION;
   }
   break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GOBJECTEXPECTATIONCHECKST_HPP_ */
