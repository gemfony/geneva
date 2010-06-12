/**
 * @file GDoubleCollection.hpp
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

#ifndef GDOUBLECOLLECTION_HPP_
#define GDOUBLECOLLECTION_HPP_

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
 * A collection of double objects without boundaries
 */
class GDoubleCollection
	:public GNumCollectionT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumCollectionT_double", boost::serialization::base_object<GNumCollectionT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDoubleCollection();
	/** @brief The copy constructor */
	GDoubleCollection(const GDoubleCollection&);
	/** @brief Initialization with a number of random values */
	explicit GDoubleCollection(const std::size_t& nval, const double& min, const double& max);
	/** @brief The destructor */
	virtual ~GDoubleCollection();

	/** @brief A standard assignment operator */
	const GDoubleCollection& operator=(const GDoubleCollection&);

	/** @brief Checks for equality with another GDoubleCollection object */
	bool operator==(const GDoubleCollection&) const;
	/** @brief Checks for inequality with another GDoubleCollection object */
	bool operator!=(const GDoubleCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

#ifdef GENEVATESTING
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

	/** @brief Triggers random initialization of the parameter collection */
	virtual void randomInit_();
};

/*****************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLECOLLECTION_HPP_ */
