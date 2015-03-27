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
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GPODEXPECTATIONCHECKST_HPP_
#define GPODEXPECTATIONCHECKST_HPP_

// Gemfony headers go here
#include "GCommonEnums.hpp"

namespace Gem {
namespace Common {

/*
template <typename T>
void compare(
   const T& a
   , const T& b
   , const Gem::Common::expectation& e
   , const double& limit
   , const bool& withMessages
) {
   if(expectationViolation(a, b, e , limit, withMessages)) {
      throw;
   }
}
*/

/******************************************************************************/
/**
 * This base class implements the interface for expaction checks
 */
class GBaseExpectionCheck {
public:
   /** @brief The default constructor */
   G_API_COMMON GBaseExpectionCheck();
   /** @brief The destructor */
   virtual G_API_COMMON ~GBaseExpectionCheck();

   /** @brief Check for equality / similarity */
   virtual G_API_COMMON boost::optional<std::string> expectationViolation(
      const GBaseExpectionCheck& /* other object */
      , const Gem::Common::expectation& /* the expectation for this object */
      , const double& /* the limit */
      , const bool& /* with messages ? */
   ) = 0;

private:
   std::string className_; ///< The name of the class in which the parameter is defined
   std::string parName_; ///< The name of the parameter stored in this class
};

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
  , const double& limit = pow(10,-10)
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
  , const double& limit = pow(10,-10)
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
/*
 * This define facilitates calls to the checkExpectation() function
 */
#define EXPECTATIONCHECK(x) deviations.push_back(checkExpectation( withMessages , caller , x , p_load->x , #x , "p_load->"#x , e, limit ))

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GPODEXPECTATIONCHECKST_HPP_ */
