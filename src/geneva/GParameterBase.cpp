/**
 * @file GParameterBase.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
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
	, gr_local(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>())
	, gr(gr_local)
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
	, gr_local(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>()) // We do *not* copy cp's random number generator!
	, gr(gr_local)
	, adaptionsActive_(cp.adaptionsActive_)
	, randomInitializationBlocked_(cp.randomInitializationBlocked_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard destructor. No local data, hence nothing to do.
 */
GParameterBase::~GParameterBase()
{
	if(gr_local) delete gr_local;
}

/**********************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterBase object, camouflaged as a GObject
 */
void GParameterBase::load_(const GObject* cp){
	// Convert cp into local format
	const GParameterBase *p_load = gobject_conversion<GParameterBase>(cp);

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

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Switches on adaptions for this object
 */
bool GParameterBase::setAdaptionsActive() {
	bool previous = adaptionsActive_;
	adaptionsActive_ = true;
	return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Disables adaptions for this object
 */
bool GParameterBase::setAdaptionsInactive() {
	bool previous = adaptionsActive_;
	adaptionsActive_ = false;
	return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Determines whether adaptions are performed for this object
 *
 * @return A boolean indicating whether adaptions are performed for this object
 */
bool GParameterBase::adaptionsActive() const {
	return adaptionsActive_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

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
	const GParameterBase *p_load = GObject::gobject_conversion<GParameterBase>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GParameterBase", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", adaptionsActive_, p_load->adaptionsActive_, "adaptionsActive_", "p_load->adaptionsActive_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", randomInitializationBlocked_, p_load->randomInitializationBlocked_, "randomInitializationBlocked_", "p_load->randomInitializationBlocked_", e , limit));

	return evaluateDiscrepancies("GParameterBase", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Assign a random number generator from another object.
 *
 * @param gr_cp A reference to another object's GRandomBase object derivative
 */
void GParameterBase::assignGRandomPointer(Gem::Hap::GRandomBase *gr_cp) {
		if(!gr_cp) {
			raiseException(
					"In GParameterBase::assignGRandomPointer() :" << std::endl
					<< "Tried to assign NULL pointer"
			);
		}

	gr = gr_cp;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***********************************************************************************/
/**
 * Re-connects the local random number generator to gr. Derived collection classes
 * may distribute this call to their sub-objects.
 */
void GParameterBase::resetGRandomPointer() {
	if(gr_local) gr = gr_local;
	else {
		raiseException(
				"In GParameterBase::resetGRandomPointer() :" << std::endl
				<< "Tried to assign NULL pointer"
		);
	}

	gr = gr_local;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***********************************************************************************/
/**
 * Checks whether the local random number generator is used. This is simply done
 * by comparing the two pointers.
 *
 * @bool A boolean indicating whether the local random number generator is used
 */
bool GParameterBase::usesLocalRNG() const {
	return gr == gr_local;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Checks whether the assigned random number generator is used throughout
 *
 * @return A boolean indicating whether the assigned random number generator is used
 */
bool GParameterBase::assignedRNGUsed() const {
	return gr != gr_local;
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

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Triggers random initialization of the parameter(-collection). This is the public
 * version of this function, which only acts if initialization has not been blocked.
 */
void GParameterBase::randomInit() {
	if(!randomInitializationBlocked_) randomInit_();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * This function needs to be overloaded for parameter collections so that it returns the
 * correct value.
 *
 * @return A boolean indicating whether the GParameterBase-derivative is an individual parameter
 */
bool GParameterBase::isIndividualParameter() const {
	return true;
}

/**********************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * As GParameterBase derivates can be either individual parameters or parameter collections,
 * this function just returns the inverse of isIndividualParameter() .
 *
 * @return A boolean indicating whether the GParameterBase-derivative is a collection
 */
bool GParameterBase::isParameterCollection() const {
	return !isIndividualParameter();
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

/* ----------------------------------------------------------------------------------
 * Tested (including overloads for FP numbers) in the most derived classes
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Multiplies double-based parameters with a given value.
 */
void GParameterBase::fpMultiplyBy(const float& val)
{ /* do nothing by default */ }

/* ----------------------------------------------------------------------------------
 * Tested (including overloads for FP numbers) in the most derived classes
 * ----------------------------------------------------------------------------------
 */

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
void GParameterBase::fpMultiplyByRandom(const float& min, const float& max)
{ /* do nothing by default */ }

/* ----------------------------------------------------------------------------------
 * Tested (including overloads for FP numbers) in the most derived classes
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Multiplies with a random floating point number in the range [0, 1[.  The actual
 * functionality needs to be added by derived classes, if they need this.  I.e.,
 * a boolean-based parameter would simply ignore this call, as it does not re-implement
 * this function and thus uses this empty stub. A floating-point-based parameter
 * re-implements this function and takes appropriate action.
 */
void GParameterBase::fpMultiplyByRandom()
{ /* do nothing by default */ }

/* ----------------------------------------------------------------------------------
 * Tested (including overloads for FP numbers) in the most derived classes
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Adds the floating point parameters of another GParameterBase object to this one.
 * The actual actions need to be defined by derived classes.
 *
 * @oaram p A boost::shared_ptr to another GParameterBase object
 */
void GParameterBase::fpAdd(boost::shared_ptr<GParameterBase> p)
{ /* do nothing by default */ }

/* ----------------------------------------------------------------------------------
 * Tested (including overloads for FP numbers) in the most derived classes
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Subtracts the floating point parameters of another GParameterBase object from this one.
 * The actual actions need to be defined by derived classes.
 *
 * @oaram p A boost::shared_ptr to another GParameterBase object
 */
void GParameterBase::fpSubtract(boost::shared_ptr<GParameterBase> p)
{ /* do nothing by default */ }

/* ----------------------------------------------------------------------------------
 * Tested (including overloads for FP numbers) in the most derived classes
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Allows to add all parameters of type double to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
void GParameterBase::streamline<double>(
		std::vector<double>& parVec
) const {
	this->doubleStreamline(parVec);
}

/**********************************************************************************/
/**
 * Allows to add all parameters of type boost::int32_t to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
void GParameterBase::streamline<boost::int32_t>(
		std::vector<boost::int32_t>& parVec
) const {
	this->int32Streamline(parVec);
}

/**********************************************************************************/
/**
 * Allows to add all parameters of type bool to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
void GParameterBase::streamline<bool>(
		std::vector<bool>& parVec
) const {
	this->booleanStreamline(parVec);
}

/**********************************************************************************/
/**
 * Attach parameters of type double to the vector. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::doubleStreamline(
		std::vector<double>& parVec
) const {
	/* do nothing by default */
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Attach parameters of type boost::int32_t to the vector. This function does nothing by
 * default. Parameter types based on boost::int32_t need to overload this function and do
 * the actual work.
 */
void GParameterBase::int32Streamline(
		std::vector<boost::int32_t>& parVec
) const {
	/* do nothing by default */
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Attach parameters of type bool to the vector. This function does nothing by
 * default. Parameter types based on bool need to overload this function and do
 * the actual work.
 */
void GParameterBase::booleanStreamline(
		std::vector<bool>& parVec
) const {
	/* do nothing by default */
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type double
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
template <>
void GParameterBase::boundaries<double>(
		std::vector<double>& lBndVec
		, std::vector<double>& uBndVec
) const {
	this->doubleBoundaries(lBndVec, uBndVec);
}

/**********************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type boost::int32_t
 *
 * @param lBndVec A vector of lower boost::int32_t parameter boundaries
 * @param uBndVec A vector of upper boost::int32_t parameter boundaries
 */
template <>
void GParameterBase::boundaries<boost::int32_t>(
		std::vector<boost::int32_t>& lBndVec
		, std::vector<boost::int32_t>& uBndVec
) const {
	this->int32Boundaries(lBndVec, uBndVec);
}

/**********************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type bool
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
template <>
void GParameterBase::boundaries<bool>(
		std::vector<bool>& lBndVec
		, std::vector<bool>& uBndVec
) const {
	this->booleanBoundaries(lBndVec, uBndVec);
}

/**********************************************************************************/
/**
 * Attach boundaries of type double to the vectors
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GParameterBase::doubleBoundaries(
		std::vector<double>& lBndVec
		, std::vector<double>& uBndVec
) const {
	/* do nothing by default */
}

/**********************************************************************************/
/**
 * Attach boundaries of type boost::int32_t to the vectors
 *
 * @param lBndVec A vector of lower boost::int32_t parameter boundaries
 * @param uBndVec A vector of upper boost::int32_t parameter boundaries
 */
void GParameterBase::int32Boundaries(
		std::vector<boost::int32_t>& lBndVec
		, std::vector<boost::int32_t>& uBndVec
) const {
	/* do nothing by default */
}

/**********************************************************************************/
/**
 * Attach boundaries of type bool to the vectors
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
void GParameterBase::booleanBoundaries(
		std::vector<bool>& lBndVec
		, std::vector<bool>& uBndVec
) const {
	/* do nothing by default */
}

/**********************************************************************************/
/**
 * Allows to count parameters of type double.
 *
 * @return The number of parameters of type double
 */
template <>
std::size_t GParameterBase::countParameters<double>() const {
	return this->countDoubleParameters();
}

/**********************************************************************************/
/**
 * Allows to count parameters of type boost::int32_t.
 *
 * @return The number of parameters of type boost::int32_t
 */
template <>
std::size_t GParameterBase::countParameters<boost::int32_t>() const {
	return this->countInt32Parameters();
}

/**********************************************************************************/
/**
 * Allows to count parameters of type bool.
 *
 * @return The number of parameters of type bool
 */
template <>
std::size_t GParameterBase::countParameters<bool>() const {
	return this->countBoolParameters();
}

/**********************************************************************************/
/**
 * Count the number of double parameters. The actual work needs to be done by
 * derived classes, if they possess double parameters.
 *
 * @return The number of double parameters in this object
 */
std::size_t GParameterBase::countDoubleParameters() const {
	return 0;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Count the number of boost::int32_t parameters. The actual work needs to be done by
 * derived classes, if they possess boost::int32_t parameters.
 *
 * @return The number of boost::int32_t parameters in this object
 */
std::size_t GParameterBase::countInt32Parameters() const {
	return 0;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Count the number of bool parameters. The actual work needs to be done by
 * derived classes, if they possess bool parameters.
 *
 * @return The number of bool parameters in this object
 */
std::size_t GParameterBase::countBoolParameters() const {
	return 0;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
void GParameterBase::assignValueVector<double>(
		const std::vector<double>& parVec
		, std::size_t& pos
) {
	this->assignDoubleValueVector(parVec, pos);
}

/**********************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
void GParameterBase::assignValueVector<boost::int32_t>(
		const std::vector<boost::int32_t>& parVec
		, std::size_t& pos
) {
	this->assignInt32ValueVector(parVec, pos);
}

/**********************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
void GParameterBase::assignValueVector<bool>(
		const std::vector<bool>& parVec
		, std::size_t& pos
) {
	this->assignBooleanValueVector(parVec, pos);
}

/**********************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignDoubleValueVector(const std::vector<double>& parVec, std::size_t& pos) {
	/* Do nothing by default */
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignInt32ValueVector(const std::vector<boost::int32_t>& parVec, std::size_t& pos) {
	/* Do nothing by default */
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignBooleanValueVector(const std::vector<bool>& parVec, std::size_t& pos) {
	/* Do nothing by default */
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::blockRandomInitialization() {
	randomInitializationBlocked_ = true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::allowRandomInitialization() {
	randomInitializationBlocked_ = false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * Checks whether initialization has been blocked
 */
bool GParameterBase::randomInitializationBlocked() const {
	return randomInitializationBlocked_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

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

	//---------------------------------------------------------------------

	{ // Test blocking and unblocking of random initialization
		using namespace Gem::Common;

		// Create two GParameterBase objects as clone of this object for further usage
		boost::shared_ptr<GParameterBase> p_test1 = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test2 = this->clone<GParameterBase>();

		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK_NO_THROW(p_test2->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == true);
		BOOST_CHECK(p_test2->randomInitializationBlocked() == true);

		// Random initialization should leave the object unchanged
		BOOST_CHECK_NO_THROW(p_test1->randomInit());
		BOOST_CHECK(*p_test1 == *p_test2);

		// Unblock random initialization
		BOOST_CHECK_NO_THROW(p_test1->allowRandomInitialization());
		BOOST_CHECK_NO_THROW(p_test2->allowRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == false);
		BOOST_CHECK(p_test2->randomInitializationBlocked() == false);

		// Random initialization should change p_test1
		BOOST_CHECK_NO_THROW(p_test1->randomInit());
		BOOST_CHECK(*p_test1 != *p_test2);
	}

	//---------------------------------------------------------------------

	{ // Check activating and de-activating adaptions. Note that the
	  // effects of these flags can only be tested if an adaptor or
	  // multiple adaptors (in the case of object collections) have
	  // been loaded.

		// Create some clones of this object
		boost::shared_ptr<GParameterBase> p_test = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test_1 = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test_2 = this->clone<GParameterBase>();

		// activate adaptions
		BOOST_CHECK_NO_THROW(p_test_1->setAdaptionsActive());
		BOOST_CHECK(p_test_1->adaptionsActive() == true);

		// de-activate adaptions
		BOOST_CHECK_NO_THROW(p_test_2->setAdaptionsInactive());
		BOOST_CHECK(p_test_2->adaptionsActive() == false);

		if(p_test_1->hasAdaptor() && p_test_2->hasAdaptor()) {
			BOOST_CHECK_NO_THROW(p_test_1->adapt()); // Should change
			BOOST_CHECK_NO_THROW(p_test->setAdaptionsActive()); // Make sure differences do not stem from this flag
			BOOST_CHECK(*p_test_1 != *p_test);

			BOOST_CHECK_NO_THROW(p_test_2->adapt()); // Should stay unchanged
			BOOST_CHECK_NO_THROW(p_test->setAdaptionsInactive()); // Make sure differences do not stem from this flag
			BOOST_CHECK(*p_test_2 == *p_test);
		}
	}

	//---------------------------------------------------------------------

	{ // Check adding and resetting of random number generators, adapt() and (de-)activation of adaptions
		// Create two local clones
		boost::shared_ptr<GParameterBase> p_test1 = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test2 = this->clone<GParameterBase>();

		// Always adapt
		BOOST_CHECK_NO_THROW(p_test1->setAdaptionsActive());
		BOOST_CHECK_NO_THROW(p_test2->setAdaptionsActive());

		// Check that adaptions are indeed active in both objects
		BOOST_CHECK(p_test1->adaptionsActive() == true);
		BOOST_CHECK(p_test2->adaptionsActive() == true);

		// A cloned adaptor should have a local random number generator, as it is default-constructed
		BOOST_CHECK(p_test1->usesLocalRNG() == true);
		BOOST_CHECK(p_test2->usesLocalRNG() == true);


		// Check that we have adaption powers when using a local random number generator
		if(p_test1->hasAdaptor()) {
			for(std::size_t i=0; i<100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt());
				BOOST_CHECK(*p_test1 != *p_test2);
				BOOST_CHECK_NO_THROW(p_test1->load(p_test2));
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}

		// Assign a factory generator
		Gem::Hap::GRandomBase *gr_test = new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>();

		BOOST_CHECK_NO_THROW(p_test1->assignGRandomPointer(gr_test));
		BOOST_CHECK_NO_THROW(p_test2->assignGRandomPointer(gr_test));

		// Has the generator been assigned ?
		BOOST_CHECK(p_test1->usesLocalRNG() == false);
		BOOST_CHECK(p_test2->usesLocalRNG() == false);

		// Check that we have adaption powers when using the new random number generator
		if(p_test1->hasAdaptor()) {
			for(std::size_t i=0; i<100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt());
				BOOST_CHECK(*p_test1 != *p_test2);
				BOOST_CHECK_NO_THROW(p_test1->load(p_test2));
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}

		// Make sure we use the local generator again
		BOOST_CHECK_NO_THROW(p_test1->resetGRandomPointer());
		BOOST_CHECK_NO_THROW(p_test2->resetGRandomPointer());

		// Get rid of the test generator
		delete gr_test;

		// We should now be using a local random number generator again
		BOOST_CHECK(p_test1->usesLocalRNG() == true);
		BOOST_CHECK(p_test2->usesLocalRNG() == true);

		// Check that we have adaption powers when using the local random number generator again
		if(p_test1->hasAdaptor()) {
			for(std::size_t i=0; i<100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt());
				BOOST_CHECK(*p_test1 != *p_test2);
				BOOST_CHECK_NO_THROW(p_test1->load(p_test2));
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}

		// De-activate adaptions in both objects
		BOOST_CHECK_NO_THROW(p_test1->setAdaptionsInactive());
		BOOST_CHECK_NO_THROW(p_test2->setAdaptionsInactive());

		// Check that adaptions do not occur anymore in p_test1
		if(p_test1->hasAdaptor()) {
			for(std::size_t i=0; i<100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt());
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}
	}

	//------------------------------------------------------------------------------
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterBase::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Check that assigning a NULL pointer for the random number generator throws
		boost::shared_ptr<GParameterBase> p_test = this->clone<GParameterBase>();

		// Assigning a NULL pointer should throw
		BOOST_CHECK_THROW(
				p_test->assignGRandomPointer(NULL);
				, Gem::Common::gemfony_error_condition
		);
	}

	//------------------------------------------------------------------------------


	{ // Check that resetting the random number generator throws if gr_local is NULL
		boost::shared_ptr<GParameterBase> p_test = this->clone<GParameterBase>();

		p_test->gr_local = NULL;

		// Resetting the pointer should throw, if gr_local is NULL (which it technically
		// should never be able to become
		BOOST_CHECK_THROW(
				p_test->resetGRandomPointer();
				, Gem::Common::gemfony_error_condition
		);
	}

	//------------------------------------------------------------------------------
}

/**********************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
