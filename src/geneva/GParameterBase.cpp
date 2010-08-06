/**
 * @file GParameterBase.cpp
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

#include "geneva/GParameterBase.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************************/
/**
 * The default constructor. Adaptions are switched on by default.
 */
GParameterBase::GParameterBase()
	: GMutableI()
	, GObject()
	, adaptionsActive_(true)
	, randomInitializationBlocked_(false)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard copy constructor.
 *
 * @param cp A copy of another GParameterBase object
 */
GParameterBase::GParameterBase(const GParameterBase& cp)
	: GMutableI(cp)
	, GObject(cp)
	, adaptionsActive_(cp.adaptionsActive_)
	, randomInitializationBlocked_(cp.randomInitializationBlocked_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard destructor. No local data, hence nothing to do.
 */
GParameterBase::~GParameterBase()
{ /* nothing */ }

/**********************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterBase object, camouflaged as a GObject
 */
void GParameterBase::load_(const GObject* cp){
	// Convert cp into local format
	const GParameterBase *p_load = conversion_cast<GParameterBase>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Load local data
	adaptionsActive_ = p_load->adaptionsActive_;
	randomInitializationBlocked_ = p_load->randomInitializationBlocked_;
}

/**********************************************************************************/
/**
 * Calls the function that does the actual adaption (which is in turn implemented
 * by derived classes. Will omit adaption if the adaptionsActive_ parameter is set.
 */
void GParameterBase::adapt() {
	if(adaptionsActive_) adaptImpl();
}

/**********************************************************************************/
/**
 * Switches on adaptions for this object
 */
void GParameterBase::setAdaptionsActive() {
	adaptionsActive_ = true;
}

/**********************************************************************************/
/**
 * Disables adaptions for this object
 */
void GParameterBase::setAdaptionsInactive() {
	adaptionsActive_ = false;
}

/**********************************************************************************/
/**
 * Determines whether adaptions are performed for this object
 *
 * @return A boolean indicating whether adaptions are performed for this object
 */
bool GParameterBase::adaptionsActive() const {
	return adaptionsActive_;
}

/**********************************************************************************/
/**
 * Checks for equality with another GParameterBase object
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterBase::operator==(const GParameterBase& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterBase::operator==","cp", CE_SILENT);
}

/**********************************************************************************/
/**
 * Checks for inequality with another GParameterBase object
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are inequal
 */
bool GParameterBase::operator!=(const GParameterBase& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterBase::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GParameterBase::checkRelationshipWith(const GObject& cp,
														           const Gem::Common::expectation& e,
														           const double& limit,
														           const std::string& caller,
														           const std::string& y_name,
														           const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GParameterBase *p_load = GObject::conversion_cast<GParameterBase>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GParameterBase", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", adaptionsActive_, p_load->adaptionsActive_, "adaptionsActive_", "p_load->adaptionsActive_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", randomInitializationBlocked_, p_load->randomInitializationBlocked_, "randomInitializationBlocked_", "p_load->randomInitializationBlocked_", e , limit));

	return evaluateDiscrepancies("GParameterBase", caller, deviations, e);
}

/**********************************************************************************/
/**
 * Convenience function so we do not need to always cast  derived classes.
 * See GParameterBaseWithAdaptors::hasAdaptors() for the "real"
 * function.
 */
bool GParameterBase::hasAdaptor() const {
	return false;
}

/**********************************************************************************/
/**
 * Triggers random initialization of the parameter(-collection). This is the public
 * version of this function, which only acts if initialization has not been blocked.
 */
void GParameterBase::randomInit() {
	if(!randomInitializationBlocked_) randomInit_();
}

/**********************************************************************************/
/**
 * Initializes double-based parameters with a given value. Allows e.g. to set all
 * floating point parameters to 0.
 *
 * @param val The value to be assigned to the parameters
 */
void GParameterBase::fpFixedValueInit(const float& val)
{ /* do nothing by default */ }

/**********************************************************************************/
/**
 * Multiplies double-based parameters with a given value.
 */
void GParameterBase::fpMultiplyBy(const float& val)
{ /* do nothing by default */ }

/**********************************************************************************/
/**
 * Multiplies with a random floating point number in a given range. The actual
 * functionality needs to be added by derived classes, if they need this. I.e.,
 * a boolean-based parameter would simply ignore this call, as it does not re-implement
 * this function and thus uses this empty stub. A floating-point-based parameter
 * re-implements this function and takes appropriate action.
 *
 * @param min The lower boundary for random number generation
 * @param max The upper boundary for random number generation
 */
void GParameterBase::fpRandomMultiplyBy(const float& min, const float& max)
{ /* do nothing by default */ }

/**********************************************************************************/
/**
 * Multiplies with a random floating point number in the range [0, 1[.  The actual
 * functionality needs to be added by derived classes, if they need this.  I.e.,
 * a boolean-based parameter would simply ignore this call, as it does not re-implement
 * this function and thus uses this empty stub. A floating-point-based parameter
 * re-implements this function and takes appropriate action.
 */
void GParameterBase::fpRandomMultiplyBy()
{ /* do nothing by default */ }

/**********************************************************************************/
/**
 * Adds the floating point parameters of another GParameterBase object to this one.
 * The actual actions need to be defined by derived classes.
 *
 * @oaram p A boost::shared_ptr to another GParameterBase object
 */
void GParameterBase::fpAdd(boost::shared_ptr<GParameterBase> p)
{ /* do nothing by default */ }

/**********************************************************************************/
/**
 * Subtracts the floating point parameters of another GParameterBase object from this one.
 * The actual actions need to be defined by derived classes.
 *
 * @oaram p A boost::shared_ptr to another GParameterBase object
 */
void GParameterBase::fpSubtract(boost::shared_ptr<GParameterBase> p)
{ /* do nothing by default */ }


/**********************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::blockRandomInitialization() {
	randomInitializationBlocked_ = true;
}

/**********************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::allowRandomInitialization() {
	randomInitializationBlocked_ = false;
}

/**********************************************************************************/
/**
 * Checks whether initialization has been blocked
 */
bool GParameterBase::randomInitializationBlocked() const {
	return randomInitializationBlocked_;
}

#ifdef GENEVATESTING
/**********************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterBase::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	return result;
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterBase::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterBase::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();
}

/**********************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
