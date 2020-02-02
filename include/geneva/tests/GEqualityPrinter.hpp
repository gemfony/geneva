/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>

// Geneva headers go here
#include "common/GPODVectorT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GObject.hpp"

/*************************************************************************************************/
/**
 * This is a simple convenience class to facilitate comparisons in Geneva's test framework.
 */
class G_API_GENEVA GEqualityPrinter
	: boost::noncopyable
{
public:
	/*********************************************************************************************/
	/**
	 * The only constructor. The default constructor has been disabled.
	 *
	 * @param caller The name of the calling entity
	 * @param limit Used in floating point comparisons to check similarity
	 * @param emitMessages Determines whether messages should be emitted upon error
	 */
	GEqualityPrinter (
		  const std::string& caller
		, const double& limit
		, const bool& emitMessages
	)
		: caller_(caller)
		, limit_(limit)
		, emitMessages_(emitMessages)
	{ /* nothing */ }

	/*********************************************************************************************/
	/**
	 * Checks for equality of two identical Geneva types, optionally emitting a message.
	 * The compared entities must have the Geneva interface.
	 *
	 * @param x The first parameter to compare
	 * @oaram y The second parameter to compare
	 * @return A boolean indicating whether both parameters are equal
	 */
	template <typename geneva_type>
	bool isEqual(
        const geneva_type& x
      , const geneva_type& y
      , typename std::enable_if<std::is_base_of<Gem::Geneva::GObject, geneva_type>::value>::type *dummy = nullptr
	) const	{
	   using namespace Gem::Common;

	   try {
	      x.compare(y, Gem::Common::expectation::EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
	   } catch(g_expectation_violation& g) {
	      std::cout
	      << "\n=========================================\n"
	      << g
	      << "\n=========================================\n";
         return false;
	   }
	}

	/*********************************************************************************************/
	/**
	 * Checks for equality of a Geneva container type with a std::vector<T> of its base types,
	 * optionally emitting a message.
	 *
	 * @param x The Geneva container type to compare
	 * @oaram y The std::vector used for the comparison
	 * @return A boolean indicating whether both parameters are equal
	 */
	template <typename geneva_simplecontainer_type>
	bool isEqual(
        const geneva_simplecontainer_type& x
      , const std::vector<typename geneva_simplecontainer_type::value_type>& y
      , typename std::enable_if<std::is_base_of<Gem::Common::GPODVectorT<typename geneva_simplecontainer_type::value_type>, geneva_simplecontainer_type>::value>::type *dummy = nullptr
	) const	{
	   using namespace Gem::Common;

	   try{
	     x.Gem::Common::template GPODVectorT<typename geneva_simplecontainer_type::value_type>::compare_base(
	        y, Gem::Common::expectation::EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE
        );

	     return true;
	   } catch(g_expectation_violation& g) {
         std::cout
         << "\n=========================================\n"
         << g
         << "\n=========================================\n";
         return false;
      }
	}

	/*********************************************************************************************/
	/**
	 * Checks for inequality, optionally emitting a message. The compared entities must have the
	 * Geneva interface.
	 *
	 * @param x The first parameter to compare
	 * @oaram y The second parameter to compare
	 * @return A boolean indicating whether both parameters are inequal
	 */
	template <typename geneva_type>
	bool isInEqual(
        const geneva_type& x
      , const geneva_type& y
      , typename std::enable_if<std::is_base_of<Gem::Geneva::GObject, geneva_type>::value>::type *dummy = nullptr
	) const	{
      using namespace Gem::Common;

      try {
         x.compare(y, Gem::Common::expectation::INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation& g) {
         std::cout
         << "\n=========================================\n"
         << g
         << "\n=========================================\n";
         return false;
      }
	}

	/*********************************************************************************************/
	/**
	 * Checks for inequality of a Geneva container type with a std::vector<T> of its base types,
	 * optionally emitting a message.
	 *
	 * @param x The Geneva container type to compare
	 * @oaram y The std::vector used for the comparison
	 * @return A boolean indicating whether both parameters are equal
	 */
	template <typename geneva_simplecontainer_type>
	bool isInEqual(
        const geneva_simplecontainer_type& x
      , const std::vector<typename geneva_simplecontainer_type::value_type>& y
      , typename std::enable_if<std::is_base_of<Gem::Common::GPODVectorT<typename geneva_simplecontainer_type::value_type>, geneva_simplecontainer_type>::value>::type *dummy = nullptr
	) const	{
      using namespace Gem::Common;

      try{
        x.Gem::Common::template GPODVectorT<typename geneva_simplecontainer_type::value_type>::compare_base(
           y, Gem::Common::expectation::INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE
        );
        return true;
      } catch(g_expectation_violation& g) {
         std::cout
         << "\n=========================================\n"
         << g
         << "\n=========================================\n";
         return false;
      }
	}

	/*********************************************************************************************/
	/**
	 * Checks for similarity, optionally emitting a message. The compared entities must have the
	 * Geneva interface.
	 *
	 * @param x The first parameter to compare
	 * @oaram y The second parameter to compare
	 * @param limit A limit used to determine similarity in fp comparisons
	 * @return A boolean indicating whether both parameters are similar
	 */
	template <typename geneva_type>
	bool isSimilar(
        const geneva_type& x
      , const geneva_type& y
      , typename std::enable_if<std::is_base_of<Gem::Geneva::GObject, geneva_type>::value>::type *dummy = nullptr
	) const	{
      using namespace Gem::Common;

      try {
         x.compare(y, Gem::Common::expectation::FP_SIMILARITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation& g) {
         std::cout
         << "\n=========================================\n"
         << g
         << "\n=========================================\n";
         return false;
      }
	}

	/*********************************************************************************************/
	/**
	 * Checks for similarity of a Geneva container type with a std::vector<T> of its base types,
    * possibly emitting a message.
	 *
	 * @param x The first parameter to compare
	 * @oaram y The second parameter to compare
	 * @param limit A limit used to determine similarity in fp comparisons
	 * @return A boolean indicating whether both parameters are similar
	 */
	template <typename geneva_simplecontainer_type>
	bool isSimilar(
        const geneva_simplecontainer_type& x
      , const std::vector<typename geneva_simplecontainer_type::value_type>& y
      , typename std::enable_if<std::is_base_of<Gem::Common::GPODVectorT<typename geneva_simplecontainer_type::value_type>, geneva_simplecontainer_type>::value>::type *dummy = nullptr
	) const	{
      using namespace Gem::Common;

      try {
         x.Gem::Common::template GPODVectorT<typename geneva_simplecontainer_type::value_type>::compare_base(
            y, Gem::Common::expectation::INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE
         );
         return true;
      } catch(g_expectation_violation& g) {
         std::cout
         << "\n=========================================\n"
         << g
         << "\n=========================================\n";
         return false;
      }
	}

	/*********************************************************************************************/

private:
	GEqualityPrinter(); ///< Default constructor intentionally left undefined

	std::string caller_; ///< Holds the name of the calling entity
	double limit_; ///< A limit used to determine similarity in fp comparisons
	bool emitMessages_; ///< Specifies whether messages should be emitted if expectations were not met
};

/*************************************************************************************************/

