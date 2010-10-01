/**
 * @file GEAOptimizationMonitor.hpp
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


#ifndef GEAOPTIMIZATIONMONITOR_HPP_
#define GEAOPTIMIZATIONMONITOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GOptimizationMonitorT.hpp"
#include "geneva/GIndividual.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class defines the interface of optimization monitors, as used
 * by default in the Geneva library for evolutionary algorithms.
 */
class GEAOptimizationMonitor
	: public GOptimizationMonitorT<GIndividual>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & make_nvp("GOptimizationMonitorT_GIndividual", boost::serialization::base_object<GOptimizationMonitorT<GIndividual> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GEAOptimizationMonitor();
    /** @brief The copy constructor */
    GEAOptimizationMonitor(const GEAOptimizationMonitor&);
    /** @brief The destructor */
    virtual ~GEAOptimizationMonitor();

    /** @brief A standard assignment operator */
    const GEAOptimizationMonitor& operator=(const GEAOptimizationMonitor&);
    /** @brief Checks for equality with another GParameter Base object */
    virtual bool operator==(const GEAOptimizationMonitor&) const;
    /** @brief Checks for inequality with another GEAOptimizationMonitor object */
    virtual bool operator!=(const GEAOptimizationMonitor&) const;

    /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
    virtual boost::optional<std::string> checkRelationshipWith(
    		const GObject&
    		, const Gem::Common::expectation&
    		, const double&
    		, const std::string&
    		, const std::string&
    		, const bool&
    ) const;

protected:
    /** @brief A function that is called once before the optimization starts */
    virtual void firstInformation(GOptimizationAlgorithmT<GIndividual> * const);
    /** @brief A function that is called during each optimization cycle */
    virtual void cycleInformation(GOptimizationAlgorithmT<GIndividual> * const);
    /** @brief A function that is called once at the end of the optimization cycle */
    virtual void lastInformation(GOptimizationAlgorithmT<GIndividual> * const);

    /** @brief Loads the data of another object */
    virtual void load_(const GObject*);
    /** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

	/**********************************************************************************/
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************/

#endif /* GEAOPTIMIZATIONMONITOR_HPP_ */
