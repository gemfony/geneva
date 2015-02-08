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

#include <boost/type_traits.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>

#ifndef GEQUALITYPRINTER_HPP_
#define GEQUALITYPRINTER_HPP_

// Geneva headers go here
#include "geneva/GObject.hpp"
#include "geneva/GStdSimpleVectorInterfaceT.hpp"

/*************************************************************************************************/
/**
 * This is a simple convenience class to facilitate comparisons in Geneva's test framework.
 */
class GEqualityPrinter
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
			, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
	) const	{
		using namespace Gem::Hap;

		boost::optional<std::string> o =
				x.checkRelationshipWith(
						y
					  , Gem::Common::CE_EQUALITY
					  , 0.
					  , caller_
					  , "y"
					  , emitMessages_?Gem::Common::CE_WITH_MESSAGES:Gem::Common::CE_SILENT);

		if(o) { // The expectation was not met
			std::cout
			<< "\n=========================================\n"
			<< *o
			<< "\n=========================================\n";
			return false;
		}
		else {
			return true;
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
			, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GStdSimpleVectorInterfaceT<typename geneva_simplecontainer_type::value_type>, geneva_simplecontainer_type> >::type* dummy = 0
	) const	{
		using namespace Gem::Hap;

		boost::optional<std::string> o =
				x.GStdSimpleVectorInterfaceT<typename geneva_simplecontainer_type::value_type>::checkRelationshipWith(
						y
					  , Gem::Common::CE_EQUALITY
					  , 0.
					  , caller_
					  , "y"
					  , emitMessages_?Gem::Common::CE_WITH_MESSAGES:Gem::Common::CE_SILENT);

		if(o) { // The expectation was not met
			std::cout
			<< "\n=========================================\n"
			<< *o
			<< "\n=========================================\n";
			return false;
		}
		else {
			return true;
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
			, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
	) const	{
		using namespace Gem::Hap;

		boost::optional<std::string> o =
				x.checkRelationshipWith(
						y
					  , Gem::Common::CE_INEQUALITY
					  , 0.
					  , caller_
					  , "y"
					  , emitMessages_?Gem::Common::CE_WITH_MESSAGES:Gem::Common::CE_SILENT);

		if(o) { // The expectation was not met
			std::cout
			<< "\n=========================================\n"
			<< *o
			<< "\n=========================================\n";
			return false;
		}
		else {
			return true;
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
			, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GStdSimpleVectorInterfaceT<typename geneva_simplecontainer_type::value_type>, geneva_simplecontainer_type> >::type* dummy = 0
	) const	{
		using namespace Gem::Hap;

		boost::optional<std::string> o =
				x.GStdSimpleVectorInterfaceT<typename geneva_simplecontainer_type::value_type>::checkRelationshipWith(
						y
					  , Gem::Common::CE_INEQUALITY
					  , 0.
					  , caller_
					  , "y"
					  , emitMessages_?Gem::Common::CE_WITH_MESSAGES:Gem::Common::CE_SILENT);

		if(o) { // The expectation was not met
			std::cout
			<< "\n=========================================\n"
			<< *o
			<< "\n=========================================\n";
			return false;
		}
		else {
			return true;
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
			, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
	) const	{
		using namespace Gem::Hap;

		boost::optional<std::string> o =
				x.checkRelationshipWith(
						y
					  , Gem::Common::CE_FP_SIMILARITY
					  , limit_
					  , caller_
					  , "y"
					  , emitMessages_?Gem::Common::CE_WITH_MESSAGES:Gem::Common::CE_SILENT);

		if(o) { // The expectation was not met
			std::cout
			<< "\n=========================================\n"
			<< *o
			<< "\n=========================================\n";
			return false;
		}
		else {
			return true;
		}
	}

	/*********************************************************************************************/
	/**
	 * Checks for similarity, optionally emitting a message. The compared entities must have the
	 * Geneva interface. This is an overloaded version of isSimilar which allows to specify an
	 * individual limit, which will be used instead of the class-wide limit.
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
			, double limit
			, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
	) const	{
		using namespace Gem::Hap;

		boost::optional<std::string> o =
				x.checkRelationshipWith(
						y
					  , Gem::Common::CE_FP_SIMILARITY
					  , limit
					  , caller_
					  , "y"
					  , emitMessages_?Gem::Common::CE_WITH_MESSAGES:Gem::Common::CE_SILENT);

		if(o) { // The expectation was not met
			std::cout
			<< "\n=========================================\n"
			<< *o
			<< "\n=========================================\n";
			return false;
		}
		else {
			return true;
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

#endif /* GEQUALITYPRINTER_HPP_ */
