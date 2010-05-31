/**
 * @file GPersonalityTraits.hpp
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


// Standard headers go here
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>


#ifndef GPERSONALITYTRAITS_HPP_
#define GPERSONALITYTRAITS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GObject.hpp"
#include "GObjectExpectationChecksT.hpp"
#include "GExceptions.hpp"

namespace Gem {
namespace GenEvA {

/*********************************************************************************/
/**
 * This is the base class for a small hierarchy that encapsulates information
 * relevant to particular optimization algorithms. The information is stored in
 * individuals (i.e. the parameter sets which are subject to a given optimization
 * problem). In this sense, individuals can take on more than one role or
 * personality. Note that this class is purely virtual. It can only be used in
 * conjunction with a derived personality.
 */
class GPersonalityTraits :public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GPersonalityTraits();
	/** @brief The copy constructor */
	GPersonalityTraits(const GPersonalityTraits&);
	/** @brief The standard destructor */
	virtual ~GPersonalityTraits();

	/** @brief Checks for equality with another GPersonalityTraits object */
	bool operator==(const GPersonalityTraits&) const;
	/** @brief Checks for inequality with another GPersonalityTraits object */
	bool operator!=(const GPersonalityTraits&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Sets a command to be performed by a remote client. */
	virtual void setCommand(const std::string&) = 0;
	/** @brief Retrieves the command to be performed by a remote client. */
	virtual std::string getCommand() const = 0;

	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

protected:
	/** @brief Loads the data of another GPersonalityTraits object */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;
};

/*********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/*********************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GPersonalityTraits)
/*********************************************************************************/

#endif /* GPERSONALITYTRAITS_HPP_ */
