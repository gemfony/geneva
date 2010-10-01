/**
 * @file GOptimizationMonitor.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

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
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>


#ifndef GOPTIMIZATIONMONITOR_HPP_
#define GOPTIMIZATIONMONITOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "GObject.hpp"
#include "GObjectExpectationChecksT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************************/
/**
 * This class defines the interface of optimization monitors, as used
 * in the Geneva library. It also provides users with some basic information.
 */
class GOptimizationMonitor {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
      	 & BOOS_SERIALIZATION_NVP(quiet_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GOptimizationMonitor();
	/** @brief The copy constructor */
	GOptimizationMonitor(const GOptimizationMonitor&);
	/** @brief The destructor */
	~GOptimizationMonitor();

	/** @brief Checks for equality with another GParameter Base object */
	bool operator==(const GOptimizationMonitor&) const;
	/** @brief Checks for inequality with another GOptimizationMonitor object */
	bool operator!=(const GOptimizationMonitor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief The actual information function */
	void informationFunction(const infoMode& im, GOptimizationAlgorithm * const);

	/** @brief Prevents any information from being emitted by this object */
	void preventInformationEmission();
	/** @brief Allows this object to emit information */
	void allowInformationEmission();
	/** @brief Allows to check whether the emission of information is prevented */
	bool informationEmissionPrevented() const;

protected:
	/** @brief A function that is called once before the optimization starts */
	virtual void firstInfo(GOptimizationAlgorithm * const);
	/** @brief A function that is called during each optimization cycle */
	virtual void cycleInfo(GOptimizationAlgorithm * const);
	/** @brief A function that is called once at the end of the optimization cycle */
	virtual void lastInfo(GOptimizationAlgorithm * const);

	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

private:
	bool quiet_; ///< Specifies whether any information should be emitted at all

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

/******************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GOptimizationMonitor)

/******************************************************************************************/

#endif /* GOPTIMIZATIONMONITOR_HPP_ */
