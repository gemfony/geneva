/**
 * @file GSwarmPersonalityTraits.cpp
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

#include "GSwarmPersonalityTraits.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GSwarmPersonalityTraits)

namespace Gem {
namespace GenEvA {

/*****************************************************************************/
/**
 * The default constructor
 */
GSwarmPersonalityTraits::GSwarmPersonalityTraits()
	: GPersonalityTraits()
	, neighborhood_(0)
	, command_("")
	, c_local_(DEFAULTCLOCAL)
	, c_local_range_(-1.)
	, c_global_(DEFAULTCGLOBAL)
	, c_global_range_(-1.)
	, c_delta_(DEFAULTCDELTA)
	, c_delta_range_(-1.)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GSwarmPersonalityTraits object
 */
GSwarmPersonalityTraits::GSwarmPersonalityTraits(const GSwarmPersonalityTraits& cp)
	: GPersonalityTraits(cp)
	, neighborhood_(cp.neighborhood_)
	, command_(cp.command_)
	, c_local_(cp.c_local_)
	, c_local_range_(cp.c_local_range_)
	, c_global_(cp.c_global_)
	, c_global_range_(cp.c_global_range_)
	, c_delta_(cp.c_delta_)
	, c_delta_range_(cp.c_delta_range_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor
 */
GSwarmPersonalityTraits::~GSwarmPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * Checks for equality with another GSwarmPersonalityTraits object
 *
 * @param  cp A constant reference to another GSwarmPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarmPersonalityTraits::operator==(const GSwarmPersonalityTraits& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarmPersonalityTraits::operator==","cp", CE_SILENT);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GSwarmPersonalityTraits object
 *
 * @param  cp A constant reference to another GSwarmPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarmPersonalityTraits::operator!=(const GSwarmPersonalityTraits& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarmPersonalityTraits::operator!=","cp", CE_SILENT);
}

/*****************************************************************************/
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
boost::optional<std::string> GSwarmPersonalityTraits::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSwarmPersonalityTraits *p_load = GObject::conversion_cast<GSwarmPersonalityTraits>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GSwarmPersonalityTraits", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", neighborhood_, p_load->neighborhood_, "neighborhood_", "p_load->neighborhood_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", command_, p_load->command_, "command_", "p_load->command_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", c_local_, p_load->c_local_, "c_local_", "p_load->c_local_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", c_local_range_, p_load->c_local_range_, "c_local_range_", "p_load->c_local_range_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", c_global_, p_load->c_global_, "c_global_", "p_load->c_global_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", c_global_range_, p_load->c_global_range_, "c_global_range_", "p_load->c_global_range_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", c_delta_, p_load->c_delta_, "c_delta_", "p_load->c_delta_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", c_delta_range_, p_load->c_delta_range_, "c_delta_range_", "p_load->c_delta_range_", e , limit));

	return evaluateDiscrepancies("GEAPersonalityTraits", caller, deviations, e);
}

/*****************************************************************************/
/**
 * Allows to set a static multiplier for local distances.
 */
void GSwarmPersonalityTraits::setCLocal(const double& c_local) {
	c_local_range_ = CLOCALRANGEDISABLED;
	c_local_ = c_local;
}

/*****************************************************************************/
/**
 * Allows to set the lower and upper boundary for random multiplier range for local distances
 */
void GSwarmPersonalityTraits::setCLocal(const double& cl_lower, const double& cl_upper) {
	c_local_ = cl_lower;

#ifdef DEBUG
	if(cl_upper <= cl_lower) {
		std::ostringstream error;
		error << "In GSwarmPersonalityTraits::setCLocal(const double&, const double&): Error!" << std::endl
			  << "cl_upper = " << cl_upper << " is <= cl_lower = " << cl_lower << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	c_local_range_ = cl_upper - cl_lower;
}

/*****************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances or the lower boundary of a random range
 */
double GSwarmPersonalityTraits::getCLocal() const {
	return c_local_;
}

/*****************************************************************************/
/**
 * Allows to retrieve the random multiplier range for local distances (-1 if unset)
 */
double GSwarmPersonalityTraits::getCLocalRange() const {
	return c_local_range_;
}

/*****************************************************************************/
/**
 * Allows to set a static multiplier for global distances
 */
void GSwarmPersonalityTraits::setCGlobal(const double& c_global) {
	c_global_range_ = CGLOBALRANGEDISABLED;
	c_global_ = c_global;
}

/*****************************************************************************/
/**
 * Allows to set the lower and upper boundary for random multiplier range for global distances
 */
void GSwarmPersonalityTraits::setCGlobal(const double& cg_lower, const double& cg_upper) {
	c_global_ = cg_lower;

#ifdef DEBUG
	if(cg_upper <= cg_lower) {
		std::ostringstream error;
		error << "In GSwarmPersonalityTraits::setCGlobal(const double&, const double&): Error!" << std::endl
			  << "cg_upper = " << cg_upper << " which is <= cg_lower = " << cg_lower << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	c_global_range_ = cg_upper - cg_lower;
}

/*****************************************************************************/
/**
 * Allows to retrieve the static multiplier for local distances or the lower boundary of a random range
 */
double GSwarmPersonalityTraits::getCGlobal() const {
	return c_global_;
}

/*****************************************************************************/
/**
 * Allows to retrieve the random multiplier range for local distances (-1 if unset)
 */
double GSwarmPersonalityTraits::getCGlobalRange() const {
	return c_global_range_;
}

/*****************************************************************************/
/**
 * Allows to set a static multiplier for deltas
 */
void GSwarmPersonalityTraits::setCDelta(const double& c_delta) {
	c_delta_range_ = CDELTARANGEDISABLED;
	c_delta_ = c_delta;
}

/*****************************************************************************/
/**
 * Allows to set the lower and upper boundary for random multiplier range for deltas
 */
void GSwarmPersonalityTraits::setCDelta(const double& cd_lower, const double& cd_upper) {
	c_delta_ = cd_lower;

#ifdef DEBUG
	if(cd_upper <= cd_lower) {
		std::ostringstream error;
		error << "In GSwarmPersonalityTraits::setCGlobal(const double&, const double&): Error!" << std::endl
			  << "cd_upper = " << cd_upper << " which is <= cd_lower = " << cd_lower << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	c_delta_range_ = cd_upper - cd_lower;
}

/*****************************************************************************/
/**
 * Allows to retrieve the static multiplier for deltas or the lower boundary of a random range
 */
double GSwarmPersonalityTraits::getCDelta() const {
	return c_delta_;
}

/*****************************************************************************/
/**
 * Allows to retrieve the random multiplier range for deltas (-1 if unset)
 */
double GSwarmPersonalityTraits::getCDeltaRange() const {
	return c_delta_range_;
}

/*****************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GSwarmPersonalityTraits::clone_() const {
	return new GSwarmPersonalityTraits(*this);
}

/*****************************************************************************/
/**
 * Loads the data of another GSwarmPersonalityTraits object
 *
 * @param cp A copy of another GSwarmPersonalityTraits object, camouflaged as a GObject
 */
void GSwarmPersonalityTraits::load_(const GObject* cp) {
	const GSwarmPersonalityTraits *p_load = this->conversion_cast<GSwarmPersonalityTraits>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// and then the local data
	neighborhood_ = p_load->neighborhood_;
	command_ = p_load->command_;
	c_local_ = p_load->c_local_;
	c_local_range_ = p_load->c_local_range_;
	c_global_ = p_load->c_global_;
	c_global_range_ = p_load->c_global_range_;
	c_delta_ = p_load->c_delta_;
	c_delta_range_ = p_load->c_delta_range_;
}

/*****************************************************************************/
/**
 * Specifies in which of the populations neighborhood the individual lives
 *
 * @param neighborhood The current neighborhood of this individual
 */
void GSwarmPersonalityTraits::setNeighborhood(const std::size_t& neighborhood) {
	neighborhood_ = neighborhood;
}

/*****************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GSwarmPersonalityTraits::getNeighborhood(void) const {
	return neighborhood_;
}

/*****************************************************************************/
/**
 * Sets a command to be performed by a remote client.
 *
 * @param command The command to be performed by a remote client
 */
void GSwarmPersonalityTraits::setCommand(const std::string& command) {
	if(command != "evaluate") { // The allowed "grammar"
		std::ostringstream error;
		error << "In GSwarmPersonalityTraits::setCommand(): Got invalid command " << command << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	command_ = command;
}

/*****************************************************************************/
/**
 * Retrieves the command to be performed by a remote client.
 *
 * @return The command to be performed by a remote client.
 */
std::string GSwarmPersonalityTraits::getCommand() const {
	return command_;
}

/*****************************************************************************/
/**
 * Makes the globally best individual known to this object
 */
void GSwarmPersonalityTraits::registerGlobalBest(boost::shared_ptr<Gem::GenEvA::GIndividual> gb_cp) {
	global_best_ = gb_cp;
}

/*****************************************************************************/
/**
 * Makes the locally best individual known to this object
 */
void GSwarmPersonalityTraits::registerLocalBest(boost::shared_ptr<Gem::GenEvA::GIndividual> lb_cp) {
	local_best_ = lb_cp;
}

/*****************************************************************************/
/**
 * Updates the parameters of the individual
 */
void GSwarmPersonalityTraits::updateParameters() {
	/* nothing yet */
}

/*****************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarmPersonalityTraits::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GPersonalityTraits::modify_GUnitTests()) result = true;

	return result;
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests();
}

/*****************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
