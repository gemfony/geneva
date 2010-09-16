/**
 * @file GParameterObjectCollection.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <sstream>
#include <vector>
#include <algorithm>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GPARAMETROBJECTCOLLECTION_HPP_
#define GPARAMETROBJECTCOLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "GParameterBase.hpp"
#include "GParameterTCollectionT.hpp"
#include "GBooleanAdaptor.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GInt32GaussAdaptor.hpp"
#include "GBooleanObject.hpp"
#include "GInt32Object.hpp"
#include "GDoubleObject.hpp"

namespace Gem {
namespace Geneva {

/*************************************************************************/
/**
 * A collection of GParameterBase objects, ready for use in a
 * GParameterSet derivative.
 */
class GParameterObjectCollection
	:public GParameterTCollectionT<GParameterBase>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterTCollectionT_gbd",
			  boost::serialization::base_object<GParameterTCollectionT<GParameterBase> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GParameterObjectCollection();
	/** @brief The copy constructor */
	GParameterObjectCollection(const GParameterObjectCollection&);
	/** @brief The destructor */
	virtual ~GParameterObjectCollection();

	/** @brief A standard assignment operator */
	const GParameterObjectCollection& operator=(const GParameterObjectCollection&);

	/** @brief Checks for equality with another GParameterObjectCollection object */
	bool operator==(const GParameterObjectCollection&) const;
	/** @brief Checks for inequality with another GParameterObjectCollection object */
	bool operator!=(const GParameterObjectCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
	boost::shared_ptr<Gem::Geneva::GParameterBase> at(const std::size_t& pos);

	/**********************************************************************/
	/**
	 * This function returns a parameter item at a given position of the data set.
     * Note that this function will only be accessible to the compiler if parameter_type
     * is a derivative of GParameterBase, thanks to the magic of Boost's enable_if
     * and Type Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <typename parameter_type>
	inline const boost::shared_ptr<parameter_type> at(
			  const std::size_t& pos
			, typename boost::enable_if<boost::is_base_of<GParameterBase, parameter_type> >::type* dummy = 0
	)  const {
#ifdef DEBUG
		boost::shared_ptr<parameter_type> p = boost::static_pointer_cast<parameter_type>(data.at(pos));

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GParameterSet::at<>() : Conversion error" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#else
		return boost::static_pointer_cast<parameter_type>(data[pos]);
#endif /* DEBUG */
	}

protected:
	/**********************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Fills the collection with GParameterBase objects */
	void fillWithObjects();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

/*************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GPARAMETROBJECTCOLLECTION_HPP_ */
