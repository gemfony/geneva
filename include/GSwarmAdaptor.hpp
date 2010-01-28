/**
 * @file GSwarmAdaptor.hpp
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
#include <cmath>
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/cast.hpp>

#ifndef GSWARMADAPTOR_HPP_
#define GSWARMADAPTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GAdaptorT.hpp"
#include "GBoundedDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
/*
 * Default settings for a number of local variables
 */

/** @brief The default multiplier for velocities */
const double DEFAULTOMEGA = 0.95;
/** @brief The default multiplier for the difference between individual and local best */
const double DEFAULTC1 = 2.;
/** @brief The default multiplier for the difference between individual and global best */
const double DEFAULTC2 = 2.;

/************************************************************************************************/
/**
 * This adaptor implements the mutations performed by swarm algorithms. Just like swarm algorithms
 * it is specific to double values.
 */
class GSwarmAdaptor
	: public GAdaptorT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;
	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT_double", boost::serialization::base_object<GAdaptorT<double> >(*this))
		   & BOOST_SERIALIZATION_NVP(omega_)
		   & BOOST_SERIALIZATION_NVP(c1_)
		   & BOOST_SERIALIZATION_NVP(c2_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor. We want to always perform mutations when this adaptor is called.
	 */
	GSwarmAdaptor()
		: GAdaptorT<double> ()
		, omega_(DEFAULTOMEGA)
		, c1_(DEFAULTC1)
		, c2_(DEFAULTC2)
	 {
		GAdaptorT<double>::setMutationMode(true);
	 }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GSwarmAdaptor object
	 */
	GSwarmAdaptor(const GSwarmAdaptor& cp)
		: GAdaptorT<double>(cp)
		, omega_(cp.omega_)
		, c1_(cp.c1_)
		, c2_(cp.c2_)
	 {
		GAdaptorT<double>::setMutationMode(true);
	 }

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GSwarmAdaptor()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GSwarmAdaptor objects,
	 *
	 * @param cp A copy of another GSwarmAdaptor object
	 */
	const GSwarmAdaptor& operator=(const GSwarmAdaptor& cp)
	{
		GSwarmAdaptor::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * This function loads the data of another GSwarmAdaptor, camouflaged as a GObject.
	 *
	 * @param A copy of another GSwarmAdaptor, camouflaged as a GObject
	 */
	void load(const GObject *cp)
	{
		// Convert GObject pointer to local format
		// (also checks for self-assignments in DEBUG mode)
		const GSwarmAdaptor *p_load = this->conversion_cast(cp, this);

		// Load the data of our parent class ...
		GAdaptorT<double>::load(cp);

		// ... and then our local data
		omega_ = p_load->omega_;
		c1_ = p_load->c1_;
		c2_ = p_load->c2_;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GSwarmAdaptor object
	 *
	 * @param  cp A constant reference to another GSwarmAdaptor object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GSwarmAdaptor& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarmAdaptor::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GSwarmAdaptor object
	 *
	 * @param  cp A constant reference to another GSwarmAdaptor object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GSwarmAdaptor& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarmAdaptor::operator!=","cp", CE_SILENT);
	}

	/***********************************************************************************/
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
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GSwarmAdaptor reference
		const GSwarmAdaptor *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<double>::checkRelationshipWith(cp, e, limit, "GSwarmAdaptor", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GSwarmAdaptor", omega_, p_load->omega_, "omega_", "p_load->omega_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GSwarmAdaptor", c1_, p_load->c1_, "c1_", "p_load->c1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GSwarmAdaptor", c2_, p_load->c2_, "c2_", "p_load->c2_", e , limit));

		return evaluateDiscrepancies("GSwarmAdaptor", caller, deviations, e);
	}


	/***********************************************************************************/
	/**
	 * Retrieves the id of the adaptor.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const {
		return GSWARMADAPTOR;
	}

	/***********************************************************************************/
	/**
	 * Prevents the mutation mode to be reset. This function is a trap.
	 *
	 * @param mutationMode The desired mode (always/never/with a given probability)
	 */
	virtual void setMutationMode(boost::logic::tribool mutationMode) {
		std::ostringstream error;
		error << "In GSwarmAdaptor::setMutationMode(): Error!" << std::endl
			  << "This function should not have been called for this adaptor." << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}

	/***********************************************************************************/
	/**
	 * Sets the \omega parameter used to multiply velocities with. Compare the general
	 * swarm algorithm shown e.g. in \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
	 *
	 * @param omega A parameter multiplied with velocity terms
	 */
	void setOmega(double omega) {
		omega_ = omega;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the \omega parameter used to multiply velocities with. Compare the general
	 * swarm algorithm shown e.g. in \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
	 *
	 * @return The \omega parameter multiplied with velocity terms
	 */
	double getOmega() const {
		return omega_;
	}

	/***********************************************************************************/
	/**
	 * Sets the c1 parameter used as a multiplier for the direction to the local best.
	 * Compare the general swarm algorithm shown e.g. in
	 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
	 *
	 * @param c1 A  multiplier for the direction to the local best
	 */
	void setC1(double c1) {
		c1_ = c1;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the c1 parameter used as a multiplier for the direction to the local best.
	 * Compare the general swarm algorithm shown e.g. in
	 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
	 *
	 * @return The c1 multiplier for the direction to the local best
	 */
	double getC1() const {
		return c1_;
	}

	/***********************************************************************************/
	/**
	 * Sets the c2 parameter used as a multiplier for the direction to the global best.
	 * Compare the general swarm algorithm shown e.g. in
	 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
	 *
	 * @param c2 A  multiplier for the direction to the global best
	 */
	void setC2(double c2) {
		c2_ = c2;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the c2 parameter used as a multiplier for the direction to the global best.
	 * Compare the general swarm algorithm shown e.g. in
	 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
	 *
	 * @return The c2 multiplier for the direction to the global best
	 */
	double getC2() const {
		return c2_;
	}

protected:
	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object
	 *
	 * @return A deep copy of this object
	 */
	GObject *clone_() const {
		return new GSwarmAdaptor(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual mutation
	 *
	 * @param value The value to be mutated
	 */
	virtual void customMutations(double& value) {
		// nothing yet
	}

private:
	double omega_; ///< The \omega parameter used as a multiplicator to velocities in swarm algorithms
	double c1_; ///< A multiplier for the directions to the local best individual
	double c2_; ///< A multiplier for the directions to the global best individual

	std::vector<double> velocity_; ///< The velocity term used in swarm algorithms
	std::vector<double> localBest_; ///< The locally best solution(s)
	std::vector<double> globalBest_; ///< The globally best solution(s)
};

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GSWARMADAPTOR_HPP_ */
