/**
 * @file GEAPersonalityTraits.hpp
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



// Standard headers go here
#include <string>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

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


#ifndef GEAPERSONALITYTRAITS_HPP_
#define GEAPERSONALITYTRAITS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "GPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/*********************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to evolutionary algorithms.
 */
class GEAPersonalityTraits :public GPersonalityTraits
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
	     & BOOST_SERIALIZATION_NVP(parentCounter_)
	     & BOOST_SERIALIZATION_NVP(popPos_)
	     & BOOST_SERIALIZATION_NVP(command_)
	     & BOOST_SERIALIZATION_NVP(parentId_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GEAPersonalityTraits();
	/** @brief The copy contructor */
	GEAPersonalityTraits(const GEAPersonalityTraits&);
	/** @brief The standard destructor */
	virtual ~GEAPersonalityTraits();

	/** @brief Checks for equality with another GEAPersonalityTraits object */
	bool operator==(const GEAPersonalityTraits&) const;
	/** @brief Checks for inequality with another GEAPersonalityTraits object */
	bool operator!=(const GEAPersonalityTraits&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Marks an individual as a parent*/
	bool setIsParent();
	/** @brief Marks an individual as a child */
	bool setIsChild();

	/** @brief Checks whether this is a parent individual */
	bool isParent() const ;
	/** @brief Retrieves the current value of the parentCounter_ variable */
	boost::uint32_t getParentCounter() const ;

	/** @brief Sets the position of the individual in the population */
	void setPopulationPosition(std::size_t) ;
	/** @brief Retrieves the position of the individual in the population */
	std::size_t getPopulationPosition(void) const ;

	/** @brief Sets a command to be performed by a remote client. */
	virtual void setCommand(const std::string&);
	/** @brief Retrieves the command to be performed by a remote client. */
	virtual std::string getCommand() const;
	/** @brief Resets the command string */
	virtual void resetCommand();

	/** @brief Stores the parent's id with this object */
	void setParentId(const std::size_t&);
	/** @brief Retrieves the parent id's value */
	std::size_t getParentId() const;
	/** @brief Checks whether a parent id has been set */
	bool parentIdSet() const;
	/** @brief Marks the parent id as unset */
	void unsetParentId();

protected:
	/** @brief Loads the data of another GEAPersonalityTraits object */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

private:
	/** @brief Allows populations to record how often an individual has been reelected as parent (0 if it is a child) */
	boost::uint32_t parentCounter_;
	/** @brief Stores the current position in the population */
	std::size_t popPos_;
	/** @brief The command to be performed by remote clients */
	std::string command_;
	/** @brief The id of the old parent individual. This is intentionally a signed value. A negative value refers to an unset parent id */
	boost::int16_t parentId_;

#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */


#endif /* GEAPERSONALITYTRAITS_HPP_ */

