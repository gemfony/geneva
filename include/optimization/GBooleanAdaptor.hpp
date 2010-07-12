/**
 * @file GBooleanAdaptor.hpp
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
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GBOOLEANADAPTOR_HPP_
#define GBOOLEANADAPTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GBoundedDouble.hpp"
#include "GOptimizationEnums.hpp"
#include "GExceptions.hpp"
#include "GIntFlipAdaptorT.hpp"
#include "GObjectExpectationChecksT.hpp"

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * The GBooleanAdaptor represents an adaptor used for the adaption of
 * bool values by flipping its value. See the documentation of GAdaptorT<T> for
 * further information on adaptors in the GenEvA context. Most functionality
 * (with the notable exception of the actual adaption logic) is currently
 * implemented in the GIntFlipAdaptorT class.
 */
class GBooleanAdaptor
	:public GIntFlipAdaptorT<bool>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GIntFlipAdaptorT_bool", boost::serialization::base_object<GIntFlipAdaptorT<bool> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBooleanAdaptor();
	/** @brief The copy constructor */
	GBooleanAdaptor(const GBooleanAdaptor&);
	/** @brief Initialization with a adaption probability */
	explicit GBooleanAdaptor(const double&);

	/** @brief The destructor */
	virtual ~GBooleanAdaptor();

	/** @brief A standard assignment operator */
	const GBooleanAdaptor& operator=(const GBooleanAdaptor&);

	/** @brief Checks for equality with another GBooleanAdaptor object */
	bool operator==(const GBooleanAdaptor&) const;
	/** @brief Checks for inequality with another GBooleanAdaptor object */
	bool operator!=(const GBooleanAdaptor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Retrieves the id of this adaptor */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const;

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

	/** The actual adaption logic */
	virtual void customAdaptions(bool&);
};

/***********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEANADAPTOR_HPP_ */
