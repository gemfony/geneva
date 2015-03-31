/**
 * @file GExpectationChecksT.hpp
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
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GPODEXPECTATIONCHECKST_HPP_
#define GPODEXPECTATIONCHECKST_HPP_

// Gemfony headers go here
#include "common/GCommonEnums.hpp"
#include "common/GMathHelperFunctions.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An exception to be thrown in case of an expectation violation.
 */
class g_expectation_violation
   : public gemfony_error_condition
{
public:
   /** @brief The standard constructor */
   G_API_COMMON g_expectation_violation(const std::string&) throw();
   /** @brief The destructor */
   virtual G_API_COMMON ~g_expectation_violation() throw();
private:
   /** @brief The default constructor: Intentionally private and undefined */
   g_expectation_violation();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two "basic" types fulfill a given expectation.
 * It assumes that x and y understand the == and != operators and may be streamed.
 * If they do not fulfill this requirement, you need to provide a specialization
 * of these functions. A check for similarity is treated the same as a check for
 * equality. A specialization of this function is provided for floating point values.
 * The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename basic_type>
void compare(
   const basic_type& x
   , const basic_type& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = 0.
   , typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
      if(x==y) {
         expectationMet = true;
      }
      break;

   case Gem::Common::CE_INEQUALITY:
      expectation_str = "CE_INEQUALITY";
      if(x!=y) {
         expectationMet = true;
      }
      break;

   default:
      {
         glogger
         << "In compare(/* 1 */): Got invalid expectation " << e << std::endl
         << GEXCEPTION;
      }
      break;
   };

   if(!expectationMet) {
      std::ostringstream error;
      error
      << "Expectation of " << expectation_str << " was violated for parameters " << std::endl
      << "[" << std::endl
      <<    x_name << " = " << x << std::endl
      <<    y_name << " = " << y << std::endl
      << "]" << std::endl;
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
/**
 * This function checks whether two floating point types meet a given expectation.
 * The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename fp_type>
void compare(
   const fp_type& x
   , const fp_type& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = std::pow(10,-6)
   , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
      expectation_str = "CE_FP_SIMILARITY";
      if(gfabs(x-y) < boost::numeric_cast<fp_type>(limit)) {
         expectationMet = true;
      }
      break;
   case Gem::Common::CE_EQUALITY:
      expectation_str = "CE_EQUALITY";
      if(x==y) {
         expectationMet = true;
      }
      break;
   case Gem::Common::CE_INEQUALITY:
      expectation_str = "CE_INEQUALITY";
      if(x!=y) {
         expectationMet = true;
      }
      break;

   default:
      {
         glogger
         << "In compare(/* 2 */): Got invalid expectation " << e << std::endl
         << GEXCEPTION;
      }
      break;
   };

   if(!expectationMet) {
      std::ostringstream error;

      error
      << "Expectation of " << expectation_str << " was violated for parameters " << std::endl
      << "[" << std::endl
      <<    x_name << " = " << x << std::endl
      <<    y_name << " = " << y << std::endl
      << "]" << std::endl;
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
/**
 * This function checks whether two containers of "basic" types meet a given expectation. It assumes that
 * these types understand the == and != operators. If they do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A specialization of this function is provided for floating point values. The function will throw a
 * g_expectation_violation exception if the expectation was violated.
 *
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename basic_type>
void compare(
   const std::vector<basic_type>& x
   , const std::vector<basic_type>& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = 0.
   , typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
      if(x==y) {
         expectationMet = true;
      }
      break;

   case Gem::Common::CE_INEQUALITY:
      expectation_str = "CE_INEQUALITY";
      if(x!=y) {
         expectationMet = true;
      }
      break;

   default:
      {
         glogger
         << "In compare(/* 3 */): Got invalid expectation " << e << std::endl
         << GEXCEPTION;
      }
      break;
   };

   if(!expectationMet) {
      std::ostringstream error;
      error
      << "Expectation of " << expectation_str << " was violated for parameters "
      << x_name << " and " << y_name << "!" << std::endl;

      if(Gem::Common::CE_FP_SIMILARITY == e || Gem::Common::CE_EQUALITY == e) {
         if(x.size() != y.size()) {
            error
            << "Sizes of vectors differ:" << std::endl
            << "x_name.size() = " << x.size() << " / " << "y_name.size() = " << y.size() << std::endl;
         } else { // Some data member differs
            // Find out about the first entry that differs
            typename std::vector<basic_type>::const_iterator x_it, y_it;
            std::size_t failedIndex = 0;
            for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
               if(*x_it != *y_it) {
                  error
                  << "Found inequality at index " << failedIndex << ": "
                  << x_name << "[" << failedIndex << "] = " << *x_it << "; "
                  << y_name << "[" << failedIndex << "] = " << *y_it;
                  break; // break the loop
               }
            }
         }
      } else { // Gem::Common::CE_INEQUALITY == e
         error
         << "The two vectors " << x_name << " and " << y_name << " are equal "
         << "even though differences were expected"
         << std::endl;
      }

      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
/**
 * This function checks whether two vectors of floating point types meet a given expectation.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename fp_type>
void compare(
   const std::vector<fp_type>& x
   , const std::vector<fp_type>& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = std::pow(10,-6)
   , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::size_t deviation_pos = 0;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      if(Gem::Common::CE_FP_SIMILARITY == e) {
         expectation_str = "CE_FP_SIMILARITY";
      } else if(Gem::Common::CE_EQUALITY == e) {
         expectation_str = "CE_EQUALITY";
      }

      if(x.size() != y.size()) {
         error
         << "Different vector-sizes found : "
         << x_name << ".size() = " << x.size() << std::endl
         << y_name << ".size() = " << y.size() << std::endl;
         break; // expectationMet is false here
      }

      // Do a per-position comparison
      bool foundDeviation = false;
      typename std::vector<fp_type>::const_iterator x_it, y_it;
      if(Gem::Common::CE_FP_SIMILARITY == e) {
         for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
            if(gfabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
               foundDeviation = true;
               deviation_pos = std::distance(x.begin(), x_it);
               error
               << "Found deviation between vectors:" << std::endl
               << x_name << "[" << deviation_pos << "] = " << *x_it << "; " << std::endl
               << y_name << "[" << deviation_pos << "] = " << *y_it << "; " << std::endl
               << "limit = " << boost::numeric_cast<fp_type>(limit) << "; " << std::endl
               << "deviation = " << gfabs(*x_it - *y_it) << std::endl;
               break; // break the loop
            }
         }
      } else if(Gem::Common::CE_EQUALITY == e) {
         for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
            if(*x_it != *y_it) {
               foundDeviation = true;
               deviation_pos = std::distance(x.begin(), x_it);
               error
               << "Found deviation between vectors:" << std::endl
               << x_name << "[" << deviation_pos << "] = " << *x_it << "; " << std::endl
               << y_name << "[" << deviation_pos << "] = " << *y_it << "; " << std::endl;
               break; // break the loop
            }
         }
      }

      if(!foundDeviation) {
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
      expectation_str = "CE_INEQUALITY";
      if(x!=y) {
         expectationMet = true;
      } else {
         error
         << "The vectors " << x_name << " and " << y_name << std::endl
         << "do not differ even though they should" << std::endl;
      }
      break;

   default:
      {
         glogger
         << "In compare(/* 4 */): Got invalid expectation " << e << std::endl
         << GEXCEPTION;
      }
      break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}


/******************************************************************************/
/** @brief This function checks whether two objects of type boost::logic::tribool meet a given expectation. */
G_API_COMMON
void compare(
   const boost::logic::tribool&
   , const boost::logic::tribool&
   , const std::string&
   , const std::string&
   , const Gem::Common::expectation&
   , const double& limit = std::pow(10,-6)
);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This define facilitates calls to the checkExpectation() function
 */
#define COMPARE(x,y,e,l) Gem::Common::compare(x,y,std::string(#x),std::string(#y),e,l)

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two "basic" types meet a given expectation. It assumes that x and y
 * understand the == and != operators. If x and y do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A specialization of this function is provided for floating point values. If "withMessages" is set to
 * true, a descriptive string will be returned in case of deviations from the expected outcome.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename basic_type>
boost::optional<std::string> checkExpectation (
	  const bool& withMessages
	, const std::string& caller
	, const basic_type& x
	, const basic_type& y
	, const std::string& x_name
	, const std::string& y_name
	, const Gem::Common::expectation& e
	, const double& limit = 0.
	, typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	case Gem::Common::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;
	case Gem::Common::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		message << "In expectation check initiated by \"" << caller << "\" : "
				<< x_name << " and " << y_name << " where not " <<
				((e==Gem::Common::CE_EQUALITY || e==Gem::Common::CE_FP_SIMILARITY)?"equal/similar":"inequal")
				<< " as expected. Further analysis: " << x_name << " = " << x << " " << y_name << " = " << y;
	}

	if(expectationMet) {
		return boost::optional<std::string>();
	}
	else {
		return boost::optional<std::string>(message.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two floating point types meet a given expectation.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename fp_type>
boost::optional<std::string> checkExpectation (
  const bool& withMessages
  , const std::string& caller
  , const fp_type& x
  , const fp_type& y
  , const std::string& x_name
  , const std::string& y_name
  , const Gem::Common::expectation& e
  , const double& limit = std::pow(10,-10)
  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
)
{
   bool expectationMet = false;
   std::ostringstream message;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
      if(fabs(x-y) < boost::numeric_cast<fp_type>(limit)) expectationMet = true;
      break;
   case Gem::Common::CE_EQUALITY:
      if(x==y) expectationMet = true;
      break;
   case Gem::Common::CE_INEQUALITY:
      if(x!=y) expectationMet = true;
      break;
   };

   if(withMessages && !expectationMet) {
      std::streamsize originalPrecision = std::cerr.precision ( );
      std::cerr.precision(15);

      switch(e) {
      case Gem::Common::CE_FP_SIMILARITY:
         message << "In expectation check initiated by \"" << caller << "\" : "
                 << "floating point values " << x_name << " and " << y_name << " where not similar as expected. "
                 << "x = " << x << "; "
                 << "y = " << y << "; "
                 << "limit = " << boost::numeric_cast<fp_type>(limit) << "; "
                 << "deviation = " << fabs(x-y);
         break;

      case Gem::Common::CE_EQUALITY:
         message << "In expectation check initiated by \"" << caller << "\" : "
               << "floating point values " << x_name << " and " << y_name << " where not equal as expected. "
               << "x = " << x << "; "
               << "y = " << y << "; "
               << "limit = " << boost::numeric_cast<fp_type>(limit) << "; "
               << "deviation = " << fabs(x-y);
         break;

      case Gem::Common::CE_INEQUALITY:
         message << "In expectation check initiated by \"" << caller << "\" : "
               << "floating point values " << x_name << " and " << y_name << " where equal contrary to expectation. "
               << "x = " << x << "; "
               << "y = " << y;
         break;
      };

      std::cerr.precision(originalPrecision);
   }

   if(expectationMet) {
      return boost::optional<std::string>();
   }
   else {
      return boost::optional<std::string>(message.str());
   }
}

/******************************************************************************/
/**
 * This function checks whether two containers of "basic" types meet a given expectation. It assumes that
 * these types understand the == and != operators. If they do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A  specialization of this function is provided for floating point values.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename basic_type>
boost::optional<std::string> checkExpectation (
  const bool& withMessages
  , const std::string& caller
  , const std::vector<basic_type>& x
  , const std::vector<basic_type>& y
  , const std::string& x_name
  , const std::string& y_name
  , const Gem::Common::expectation& e
  , const double& limit = 0.
  , typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
){
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	case Gem::Common::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;
	case Gem::Common::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		switch(e) {
		// Similarity and equality are treated the same for non-floating point types
		case Gem::Common::CE_FP_SIMILARITY:
		case Gem::Common::CE_EQUALITY:
			{
				message
				<< "In expectation check initiated by \"" << caller << "\" : "
				<< "The two vectors " << x_name << " and " << y_name << " differ "
				<< "while equality was expected. Further analysis: ";

				std::size_t x_size = x.size(), y_size = y.size();
				if(x_size != y_size) {
					message
					<< "Different vector-sizes found : "
					<< x_name << ".size() = " << x_size << "; "
					<< y_name << ".size() = " << y_size;
				}
				else {
					// Find out about the first entry that differs
					typename std::vector<basic_type>::const_iterator x_it, y_it;
					std::size_t failedIndex = 0;
					for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
						if(*x_it != *y_it) {
							message << "Found inequality at index " << failedIndex << ": "
									<< x_name << "[" << failedIndex << "] = " << *x_it << "; "
									<< y_name << "[" << failedIndex << "] = " << *y_it;
							break;
						}
					}
				}
			}
			break;

		case Gem::Common::CE_INEQUALITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
			        << "The two vectors " << x_name << " and " << y_name << " are equal "
			        << "even though differences were expected";
			break;
		}
	}

	if(expectationMet) {
		return boost::optional<std::string>();
	}
	else {
		return boost::optional<std::string>(message.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two containers of "basic" types meet a given expectation. It assumes that
 * these types understand the == and != operators. If they do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A  specialization of this function is provided for floating point values.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename basic_type>

boost::optional<std::string> checkExpectation (
  const bool& withMessages
  , const std::string& caller
  , const std::deque<basic_type>& x
  , const std::deque<basic_type>& y
  , const std::string& x_name
  , const std::string& y_name
  , const Gem::Common::expectation& e
  , const double& limit = 0.
  , typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
){
   bool expectationMet = false;
   std::ostringstream message;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
      if(x==y) expectationMet = true;
      break;
   case Gem::Common::CE_INEQUALITY:
      if(x!=y) expectationMet = true;
      break;
   };

   if(withMessages && !expectationMet) {
      switch(e) {
      // Similarity and equality are treated the same for non-floating point types
      case Gem::Common::CE_FP_SIMILARITY:
      case Gem::Common::CE_EQUALITY:
         {
            message
            << "In expectation check initiated by \"" << caller << "\" : "
            << "The two vectors " << x_name << " and " << y_name << " differ "
            << "while equality was expected. Further analysis: ";

            std::size_t x_size = x.size(), y_size = y.size();
            if(x_size != y_size) {
               message
               << "Different vector-sizes found : "
               << x_name << ".size() = " << x_size << "; "
               << y_name << ".size() = " << y_size;
            }
            else {
               // Find out about the first entry that differs
               typename std::deque<basic_type>::const_iterator x_it, y_it;
               std::size_t failedIndex = 0;
               for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
                  if(*x_it != *y_it) {
                     message << "Found inequality at index " << failedIndex << ": "
                           << x_name << "[" << failedIndex << "] = " << *x_it << "; "
                           << y_name << "[" << failedIndex << "] = " << *y_it;
                     break;
                  }
               }
            }
         }
         break;

      case Gem::Common::CE_INEQUALITY:
         message << "In expectation check initiated by \"" << caller << "\" : "
                 << "The two vectors " << x_name << " and " << y_name << " are equal "
                 << "even though differences were expected";
         break;
      }
   }

   if(expectationMet) {
      return boost::optional<std::string>();
   }
   else {
      return boost::optional<std::string>(message.str());
   }
}

/******************************************************************************/
/**
 * This function checks whether two vectors of floating point types meet a given expectation.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename fp_type>

boost::optional<std::string> checkExpectation (
  const bool& withMessages
  , const std::string& caller
  , const std::vector<fp_type>& x
  , const std::vector<fp_type>& y
  , const std::string& x_name
  , const std::string& y_name
  , const Gem::Common::expectation& e
  , const double& limit = std::pow(10,-10)
  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	    {
	    	// Leave switch statement if sizes differ
	    	if(x.size() != y.size()) break;

	    	// Loop over all entries
			typename std::vector<fp_type>::const_iterator x_it, y_it;
			bool foundDeviation = false;

			for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
				if(fabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
					foundDeviation = true;
					break;
				}
			}

			if(!foundDeviation) expectationMet = true;
	    }
		break;

	case Gem::Common::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;

	case Gem::Common::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		std::streamsize originalPrecision = std::cerr.precision ( );
		std::cerr.precision(15);

		switch(e) {
		case Gem::Common::CE_FP_SIMILARITY:
			{
				message << "In expectation check initiated by \"" << caller << "\" : "
						 << "The two vector<fp_type> objects " << x_name << " and " << y_name << " show deviations "
						 << "while similarity was expected. Further analysis: ";

				std::size_t x_size = x.size(), y_size = y.size();
				if(x_size != y_size) {
					message << "Different vector-sizes found : "
							 << x_name << ".size() = " << x_size << "; "
							 << y_name << ".size() = " << y_size;
				}
				else {
					// Find out about the first entry that differs
					typename std::vector<fp_type>::const_iterator x_it, y_it;
					std::size_t failedIndex = 0;
					for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
						if(fabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
							message << "Found deviation at index " << failedIndex << ": "
									 << x_name << "[" << failedIndex << "] = " << *x_it << "; "
									 << y_name << "[" << failedIndex << "] = " << *y_it << "; "
									 << "limit = " << boost::numeric_cast<fp_type>(limit) << "; "
									 << "deviation = " << fabs(*x_it - *y_it);
							break;
						}
					}
				}
			}
			break;

		case Gem::Common::CE_EQUALITY:
			{
				message << "In expectation check initiated by \"" << caller << "\" : "
						 << "The two vector<fp_type> objects " << x_name << " and " << y_name << " differ "
						 << "while equality was expected. Further analysis: ";

				std::size_t x_size = x.size(), y_size = y.size();
				if(x_size != y_size) {
					message << "Different vector-sizes found : "
							 << x_name << ".size() = " << x_size << "; "
							 << y_name << ".size() = " << y_size;
				}
				else {
					// Find out about the first entry that differs
					typename std::vector<fp_type>::const_iterator x_it, y_it;
					std::size_t failedIndex = 0;
					for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
						if(*x_it != *y_it) {
							message << "Found inequality at index " << failedIndex << ": "
									 << x_name << "[" << failedIndex << "] = " << *x_it << "; "
									 << y_name << "[" << failedIndex << "] = " << *y_it;
							break;
						}
					}
				}
			}
			break;

		case Gem::Common::CE_INEQUALITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
				     << "The two vectors " << x_name << " and " << y_name << " are equal "
				     << "even though differences were expected";
			break;
		}

		std::cerr.precision(originalPrecision);
	}

	if(expectationMet) {
		return boost::optional<std::string>();
	}
	else {
		return boost::optional<std::string>(message.str());
	}
}

/******************************************************************************/
/**
 * @brief This function checks whether two objects of type boost::logic::tribool meet a given expectation.
 */
G_API_COMMON
boost::optional<std::string> checkExpectation (
  const bool& withMessages
  , const std::string& caller
  , const boost::logic::tribool& x
  , const boost::logic::tribool& y
  , const std::string& x_name
  , const std::string& y_name
  , const Gem::Common::expectation& e
  , const double& limit = 0
);

/******************************************************************************/
/**
 * Helps to evaluate possible discrepancies between expectations in relationship tests
 */
G_API_COMMON
boost::optional<std::string> evaluateDiscrepancies(
   const std::string&
   , const std::string&
   , const std::vector<boost::optional<std::string> >&
   , const Gem::Common::expectation&
);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This define facilitates calls to the checkExpectation() function
 */
#define EXPECTATIONCHECK(x) deviations.push_back(checkExpectation( withMessages , caller , x , p_load->x , #x , "p_load->"#x , e, limit ))

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GPODEXPECTATIONCHECKST_HPP_ */
