/**
 * @file GOptimizationMonitorT.hpp
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
#include "geneva/GObject.hpp"
#include "geneva/GObjectExpectationChecksT.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class defines the interface of optimization monitors, as used
 * in the Geneva library. It also provides users with some basic information.
 * The template parameter will usually either be of the type "GParameterSet"
 * or "GIndividual".
 *
 * TODO: Add MPL-Assert that T is a derivative of GIndividual
 */
template <typename T>
class GOptimizationMonitorT
	: public GObject
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
      	 & BOOST_SERIALIZATION_NVP(quiet_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**********************************************************************************/
    /**
     * The default constructor
     */
    GOptimizationMonitorT()
    	: quiet_(false)
    { /* nothing */ }

    /**********************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GOptimizationMonitorT object
     */
    GOptimizationMonitorT(const GOptimizationMonitorT<T>& cp)
    	: GObject(cp)
    	, quiet_(cp.quiet_)
    { /* nothing */ }

    /**********************************************************************************/
    /**
     * The destructor
     */
    virtual ~GOptimizationMonitorT()
    { /* nothing */ }

    /**********************************************************************************/
    /**
     * Checks for equality with another GParameter Base object
     *
     * @param  cp A constant reference to another GOptimizationMonitorT object
     * @return A boolean indicating whether both objects are equal
     */
    virtual bool operator==(const GOptimizationMonitorT<T>& cp) const {
    	using namespace Gem::Common;
    	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
    	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizationMonitorT::operator==","cp", CE_SILENT);
    }

    /**********************************************************************************/
    /**
     * Checks for inequality with another GOptimizationMonitorT object
     *
     * @param  cp A constant reference to another GOptimizationMonitorT object
     * @return A boolean indicating whether both objects are inequal
     */
    virtual bool operator!=(const GOptimizationMonitorT<T>& cp) const {
    	using namespace Gem::Common;
    	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
    	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizationMonitorT::operator!=","cp", CE_SILENT);
    }

    /**********************************************************************************/
    /**
     * Checks whether a given expectation for the relationship between this object and another object
     * is fulfilled.
     *
     * @param cp A constant reference to another object, camouflaged as a GObject
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     * @param caller An identifier for the calling entity
     * @param y_name An identifier for the object that should be compared to this one
     * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
     * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
     */
    virtual boost::optional<std::string> checkRelationshipWith(
    		const GObject& cp
    		, const Gem::Common::expectation& e
    		, const double& limit
    		, const std::string& caller
    		, const std::string& y_name
    		, const bool& withMessages
    ) const {
        using namespace Gem::Common;

    	// Check that we are indeed dealing with a GParamterBase reference
    	const GOptimizationMonitorT<T> *p_load = GObject::conversion_cast<GOptimizationMonitorT<T> >(&cp);

    	// Will hold possible deviations from the expectation, including explanations
        std::vector<boost::optional<std::string> > deviations;

    	// Check our parent class'es data ...
    	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GOptimizationMonitorT", y_name, withMessages));

    	// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GOptimizationMonitorT<T>", quiet_, p_load->quiet_, "quiet_", "p_load->quiet_", e , limit));

    	return evaluateDiscrepancies("GOptimizationMonitorT", caller, deviations, e);
    }

    /**********************************************************************************/
    /**
     * The actual information function
     *
     * @param im The mode in which the information function is called
     * @param goa A pointer to the current optimization algorithm for which information should be emitted
     */
    virtual void informationFunction(const infoMode& im, GOptimizationAlgorithmT<T> * const goa) {
    	if(quiet_) return;

    	switch(im) {
    	case Gem::Geneva::INFOINIT:
    		firstInformation(goa);
    		break;
    	case Gem::Geneva::INFOPROCESSING:
    		cycleInformation(goa);
    		break;
    	case Gem::Geneva::INFOEND:
    		lastInformation(goa);
    		break;
    	};
    }

    /**********************************************************************************/
    /**
     * Prevents any information from being emitted by this object
     */
    void preventInformationEmission() {
    	quiet_ = true;
    }

    /**********************************************************************************/
    /**
     * Allows this object to emit information
     */
    void allowInformationEmission() {
    	quiet_ = false;
    }

    /**********************************************************************************/
    /**
     * Allows to check whether the emission of information is prevented
     *
     * @return A boolean which indicates whether information emission is prevented
     */
    bool informationEmissionPrevented() const {
    	return quiet_;
    }

protected:
    /**********************************************************************************/
    /**
     * A function that is called once before the optimization starts
     *
     * @param goa A pointer to the current optimization algorithm for which information should be emitted
     */
    virtual void firstInformation(GOptimizationAlgorithmT<T> * const goa) {
    	std::cout << "Starting the optimization run" << std::endl;
    }

    /**********************************************************************************/
    /**
     * A function that is called during each optimization cycle. It is possible to
     * extract quite comprehensive information in each iteration. For examples, see
     * the standard overloads provided for the various optimization algorithms.
     *
     * @param goa A pointer to the current optimization algorithm for which information should be emitted
     */
    virtual void cycleInformation(GOptimizationAlgorithmT<T> * const goa) {
    	std::cout << "Fitness in iteration " << goa->getIteration() << ": " << goa->getBestFitness() << std::endl;
    }

    /**********************************************************************************/
    /**
     * A function that is called once at the end of the optimization cycle
     *
     * @param goa A pointer to the current optimization algorithm for which information should be emitted
     */
    virtual void lastInformation(GOptimizationAlgorithmT<T> * const goa) {
    	std::cout << "End of optimization reached" << std::endl;
    }

    /**********************************************************************************/
    /**
     * Loads the data of another object
     *
     * cp A pointer to anither GOptimizationMonitorT object, camouflaged as a GObject
     */
    virtual void load_(const GObject* cp) {
    	const GOptimizationMonitorT<T> *p_load = GObject::conversion_cast<GOptimizationMonitorT<T> >(cp);

    	// Load the parent classes' data ...
    	GObject::load_(cp);

    	// ... and then our local data
    	quiet_ = p_load->quiet_;
    }

    /**********************************************************************************/
    /** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

private:
	/**********************************************************************************/
	bool quiet_; ///< Specifies whether any information should be emitted at all

#ifdef GENEVATESTING
public:
	/**********************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent class'es function
		if(GObject::modify_GUnitTests()) result = true;

		return result;
	}

	/**********************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent class'es function
		GObject::specificTestsNoFailureExpected_GUnitTests();
	}

	/**********************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent class'es function
		GObject::specificTestsFailuresExpected_GUnitTests();
	}

	/**********************************************************************************/
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GOptimizationMonitorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GOptimizationMonitorT<T> > : public boost::true_type {};
	}
}

/**************************************************************************************/

#endif /* GOPTIMIZATIONMONITOR_HPP_ */
