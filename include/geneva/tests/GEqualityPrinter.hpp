 /**
 * @file GEqualityPrinter.hpp
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
#include <boost/test/test_case_template.hpp>
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

