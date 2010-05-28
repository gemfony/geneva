/**
 * @file GParameterBase.hpp
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
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/optional.hpp>

#ifndef GPARAMETERBASE_HPP_
#define GPARAMETERBASE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here

#include "GMutableI.hpp"
#include "GObject.hpp"
#include "GExceptions.hpp"
#include "GPODExpectationChecksT.hpp"

namespace Gem {
namespace GenEvA {

/**
 * The purpose of this class is to provide a common base for all parameter classes, so
 * that a GParameterSet can be built from different parameter types. The class also
 * defines the interface that needs to be implemented by parameter classes.
 *
 * Note: It is required that derived classes make sure that a useful operator=() is available!
 */
class GParameterBase
	: public GMutableI
	, public GObject
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
         & BOOST_SERIALIZATION_NVP(adaptionsActive_);
    }
    ///////////////////////////////////////////////////////////////////////
public:
	/** @brief The standard constructor */
	GParameterBase();
	/** @brief The copy constructor */
	GParameterBase(const GParameterBase&);
	/** @brief The standard destructor */
	virtual ~GParameterBase();

	/** @brief The adaption interface */
	void adapt();
	/** @brief The actual adaption logic */
	virtual void adaptImpl() = 0;

	/** @brief Switches on adaptions for this object */
	void setAdaptionsActive();
	/** @brief Disables adaptions for this object */
	void setAdaptionsInactive();
	/** @brief Determines whether adaptions are performed for this object */
	bool adaptionsActive() const;

	/** @brief Checks for equality with another GParameterBase object */
	bool operator==(const GParameterBase&) const;
	/** @brief Checks for inequality with another GParameterBase object */
	bool operator!=(const GParameterBase&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Convenience function so we do not need to always cast derived classes */
	virtual bool hasAdaptor() const;

	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

private:
	bool adaptionsActive_; ///< Specifies whether adaptions of this object should be carried out
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GParameterBase)

/**************************************************************************************************/

#endif /* GPARAMETERBASE_HPP_ */
