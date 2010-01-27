/**
 * @file GInt32Collection.hpp
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

// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>

#ifndef GINT32COLLECTION_HPP_
#define GINT32COLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA header files go here

#include "GNumCollectionT.hpp"


namespace Gem {
namespace GenEvA {

/*****************************************************************************************/
/**
 * A collection of boost::int32_t objects without boundaries
 */
class GInt32Collection
	:public GNumCollectionT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumCollectionT_int32", boost::serialization::base_object<GNumCollectionT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GInt32Collection();
	/** @brief The copy constructor */
	GInt32Collection(const GInt32Collection&);
	/** @brief Initialization with a number of random values */
	explicit GInt32Collection(const std::size_t& nval, const boost::int32_t& min, const boost::int32_t& max);
	/** @brief The destructor */
	virtual ~GInt32Collection();

	/** @brief A standard assignment operator */
	const GInt32Collection& operator=(const GInt32Collection&);

	/** @brief Checks for equality with another GInt32Collection object */
	bool operator==(const GInt32Collection&) const;
	/** @brief Checks for inequality with another GInt32Collection object */
	bool operator!=(const GInt32Collection&) const;

	/** @brief Checks for equality with another GInt32Collection object. */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GInt32Collection object. */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Loads the data of another GObject */
	virtual void load(const GObject* cp);

protected:
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;
};

/*****************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINT32COLLECTION_HPP_ */
