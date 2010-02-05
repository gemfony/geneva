/**
 * @file GCharFlipAdaptor.hpp
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

#ifndef GCHARFLIPADAPTOR_HPP_
#define GCHARFLIPADAPTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA header files go here

#include "GIntFlipAdaptorT.hpp"


namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * This adaptor increases or decreases a value by 1
 */
class GCharFlipAdaptor
	:public GIntFlipAdaptorT<char>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GIntFlipAdaptorT_char", boost::serialization::base_object<GIntFlipAdaptorT<char> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GCharFlipAdaptor();
	/** @brief The copy constructor */
	GCharFlipAdaptor(const GCharFlipAdaptor&);
	/** @brief Initialization with a mutation probability */
	explicit GCharFlipAdaptor(const double&);

	/** @brief The destructor */
	virtual ~GCharFlipAdaptor();

	/** @brief A standard assignment operator */
	const GCharFlipAdaptor& operator=(const GCharFlipAdaptor&);

	/** @brief Checks for equality with another GCharFlipAdaptor object */
	bool operator==(const GCharFlipAdaptor&) const;
	/** @brief Checks for inequality with another GCharFlipAdaptor object */
	bool operator!=(const GCharFlipAdaptor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Retrieves the id of this adaptor */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;
};

/*************************************************************************/


} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GCHARFLIPADAPTOR_HPP_ */
