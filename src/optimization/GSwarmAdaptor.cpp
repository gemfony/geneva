/**
 * @file GSwarmAdaptor.cpp
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

#include "optimization/GSwarmAdaptor.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GSwarmAdaptor)

namespace Gem {
namespace GenEvA {

/************************************************************************************************************/
/**
 * The standard constructor.
 */
GSwarmAdaptor::GSwarmAdaptor()
	: GAdaptorT<double> ()
	, cDelta_(DEFAULTCDELTA)
	, cLocal_(DEFAULTCLOCAL)
	, cGlobal_(DEFAULTCGLOBAL)
 {
	// We want to always perform adaptions when this adaptor is called.
	GAdaptorT<double>::setAdaptionMode(true);
 }

/************************************************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GSwarmAdaptor object
 */
GSwarmAdaptor::GSwarmAdaptor(const GSwarmAdaptor& cp)
	: GAdaptorT<double>(cp)
	, cDelta_(cp.cDelta_)
	, cLocal_(cp.cLocal_)
	, cGlobal_(cp.cGlobal_)
 {
	// We want to always perform adaptions when this adaptor is called.
	GAdaptorT<double>::setAdaptionMode(true);
 }

/************************************************************************************************************/
/**
 * The standard destructor. Empty, as we have no local, dynamically
 * allocated data.
 */
GSwarmAdaptor::~GSwarmAdaptor()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard assignment operator for GSwarmAdaptor objects,
 *
 * @param cp A copy of another GSwarmAdaptor object
 */
const GSwarmAdaptor& GSwarmAdaptor::operator=(const GSwarmAdaptor& cp)
{
	GSwarmAdaptor::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Checks for equality with another GSwarmAdaptor object
 *
 * @param  cp A constant reference to another GSwarmAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarmAdaptor::operator==(const GSwarmAdaptor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarmAdaptor::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GSwarmAdaptor object
 *
 * @param  cp A constant reference to another GSwarmAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarmAdaptor::operator!=(const GSwarmAdaptor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarmAdaptor::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
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
boost::optional<std::string> GSwarmAdaptor::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GSwarmAdaptor reference
	const GSwarmAdaptor *p_load = conversion_cast<GSwarmAdaptor>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GAdaptorT<double>::checkRelationshipWith(cp, e, limit, "GSwarmAdaptor", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GSwarmAdaptor", cDelta_, p_load->cDelta_, "cDelta_", "p_load->cDelta_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmAdaptor", cLocal_, p_load->cLocal_, "cLocal_", "p_load->cLocal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmAdaptor", cGlobal_, p_load->cGlobal_, "cGlobal_", "p_load->cGlobal_", e , limit));

	return evaluateDiscrepancies("GSwarmAdaptor", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Retrieves the id of the adaptor.
 *
 * @return The id of the adaptor
 */
Gem::GenEvA::adaptorId GSwarmAdaptor::getAdaptorId() const {
	return GSWARMADAPTOR;
}

/************************************************************************************************************/
/**
 * Prevents the adaption mode to be reset. This function is a trap.
 *
 * @param adaptionMode The desired mode (always/never/with a given probability)
 */
void GSwarmAdaptor::setAdaptionMode(boost::logic::tribool adaptionMode) {
	std::ostringstream error;
	error << "In GSwarmAdaptor::setAdaptionMode(): Error!" << std::endl
		  << "This function should not have been called for this adaptor." << std::endl;
	throw(Gem::Common::gemfony_error_condition(error.str()));
}

/************************************************************************************************************/
/**
 * Sets the cDelta_ parameter used to multiply velocities with. Compare the general
 * swarm algorithm shown e.g. in \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
 *
 * @param cDelta A parameter multiplied with velocity terms
 */
void GSwarmAdaptor::setCDelta(const double& cDelta) {
	cDelta_ = cDelta;
}

/************************************************************************************************************/
/**
 * Retrieves the cDelta_ parameter used to multiply velocities with. Compare the general
 * swarm algorithm shown e.g. in \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
 *
 * @return The cdelta_ parameter multiplied with velocity terms
 */
double GSwarmAdaptor::getCDelta() const {
	return cDelta_;
}

/************************************************************************************************************/
/**
 * Sets the cLocal_ parameter used as a multiplier for the direction to the local best.
 * Compare the general swarm algorithm shown e.g. in
 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
 *
 * @param cLocal A  multiplier for the direction to the local best
 */
void GSwarmAdaptor::setCLocal(const double& cLocal) {
	cLocal_ = cLocal;
}

/************************************************************************************************************/
/**
 * Retrieves the cLocal parameter used as a multiplier for the direction to the local best.
 * Compare the general swarm algorithm shown e.g. in
 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
 *
 * @return The cLocal multiplier for the direction to the local best
 */
double GSwarmAdaptor::getCLocal() const {
	return cLocal_;
}

/************************************************************************************************************/
/**
 * Sets the cGlobal_ parameter used as a multiplier for the direction to the global best.
 * Compare the general swarm algorithm shown e.g. in
 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
 *
 * @param cGlobal A  multiplier for the direction to the global best
 */
void GSwarmAdaptor::setCGlobal(const double& cGlobal) {
	cGlobal_ = cGlobal;
}

/************************************************************************************************************/
/**
 * Retrieves the cGlobal parameter used as a multiplier for the direction to the global best.
 * Compare the general swarm algorithm shown e.g. in
 * \url{http://en.wikipedia.org/wiki/Particle_Swarm_Optimization} .
 *
 * @return The cGlobal multiplier for the direction to the global best
 */
double GSwarmAdaptor::getCGlobal() const {
	return cGlobal_;
}

/************************************************************************************************************/
/**
 * This function loads the data of another GSwarmAdaptor, camouflaged as a GObject.
 *
 * @param A copy of another GSwarmAdaptor, camouflaged as a GObject
 */
void GSwarmAdaptor::load_(const GObject *cp)
{
	// Convert GObject pointer to local format
	// (also checks for self-assignments in DEBUG mode)
	const GSwarmAdaptor *p_load = conversion_cast<GSwarmAdaptor>(cp);

	// Load the data of our parent class ...
	GAdaptorT<double>::load_(cp);

	// ... and then our local data
	cDelta_ = p_load->cDelta_;
	cLocal_ = p_load->cLocal_;
	cGlobal_ = p_load->cGlobal_;
}

/************************************************************************************************************/
/**
 * This function creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GSwarmAdaptor::clone_() const {
	return new GSwarmAdaptor(*this);
}

/************************************************************************************************************/
/**
 * The actual adaption
 *
 * @param value The value to be adapted
 */
void GSwarmAdaptor::customAdaptions(double& value) {
	// nothing yet
}


#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarmAdaptor::modify_GUnitTests() {
	bool result;

	// Call the parent classes' functions
	if(GAdaptorT<double>::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmAdaptor::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent classes' functions
	GAdaptorT<double>::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmAdaptor::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent classes' functions
	GAdaptorT<double>::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace GenEvA */
} /* namespace Gem */
