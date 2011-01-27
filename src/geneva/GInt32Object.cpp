/**
 * @file GInt32Object.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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

#include "geneva/GInt32Object.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32Object)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GInt32Object::GInt32Object()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32Object object
 */
GInt32Object::GInt32Object(const GInt32Object& cp)
	: GNumIntT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GInt32Object::GInt32Object(const boost::int32_t& val)
	: GNumIntT<boost::int32_t>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GInt32Object::GInt32Object(const boost::int32_t& lowerBoundary, const boost::int32_t& upperBoundary)
	: GNumIntT<boost::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GInt32Object::~GInt32Object()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
boost::int32_t GInt32Object::operator=(const boost::int32_t& val) {
	return GNumIntT<boost::int32_t>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32Object object
 * @return A constant reference to this object
 */
const GInt32Object& GInt32Object::operator=(const GInt32Object& cp){
	GInt32Object::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32Object::clone_() const {
	return new GInt32Object(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32Object object
 *
 * @param  cp A constant reference to another GInt32Object object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32Object::operator==(const GInt32Object& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInt32Object::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GInt32Object object
 *
 * @param  cp A constant reference to another GInt32Object object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32Object::operator!=(const GInt32Object& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInt32Object::operator!=","cp", CE_SILENT);
}

/*******************************************************************************************/
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
boost::optional<std::string> GInt32Object::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Object>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GNumIntT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GInt32Object", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32Object", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GInt32Object::int32Streamline(std::vector<boost::int32_t>& parVec) const {
	parVec.push_back(this->value());
}

/*******************************************************************************************/
/**
 * Tell the audience that we own a boost::int32_t value
 *
 * @return The number 1, as we own a single boost::int32_t parameter
 */
std::size_t GInt32Object::countInt32Parameters() const {
	return 1;
}

/*******************************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GInt32Object::assignInt32ValueVector(const std::vector<boost::int32_t>& parVec, std::size_t& pos) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
		raiseException(
				"In GBooleanObject::assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&):" << std::endl
				<< "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos
		);
	}
#endif

	this->setValue(parVec[pos]);
	pos++;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Object object, camouflaged as a GObject
 */
void GInt32Object::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Object>(cp);

	// Load our parent class'es data ...
	GNumIntT<boost::int32_t>::load_(cp);

	// ... no local data
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Object::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumIntT<boost::int32_t>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Object::specificTestsNoFailureExpected_GUnitTests() {
	// A few settings
	const std::size_t nTests = 10000;
	const boost::int32_t LOWERINITBOUNDARY = -10;
	const boost::int32_t UPPERINITBOUNDARY =  10;
	const boost::int32_t FIXEDVALUEINIT = 1;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<boost::int32_t> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GNumIntT<boost::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test different ways of adding an adaptor (Test of GParameterBaseWithAdaptorsT<T> functions)
		boost::shared_ptr<GInt32Object> p_test = this->clone<GInt32Object>();

		// Make sure we start in pristine condition. This will add a GInt32FlipAdaptor.
		BOOST_CHECK_NO_THROW(p_test->resetAdaptor());

		//********************************
		// Adding an adaptor of different type present should clone the adaptor
		BOOST_CHECK_NO_THROW(p_test->addAdaptor(giga_ptr));

		// Check that the addresses of both adaptors differ
		boost::shared_ptr<GInt32GaussAdaptor> giga_clone_ptr;
		BOOST_CHECK_NO_THROW(giga_clone_ptr = p_test->getAdaptor<GInt32GaussAdaptor>());
		BOOST_CHECK(giga_clone_ptr.get() != giga_ptr.get());

		//********************************
		// Adding an adaptor when an adaptor of the same type is present should leave the original address intact

		// Make a note of the stored adaptor's address
		GInt32GaussAdaptor *ptr_store = giga_clone_ptr.get();

		// Add the "global" adaptor again, should be load()-ed
		BOOST_CHECK_NO_THROW(p_test->addAdaptor(giga_ptr));

		// Retrieve the adaptor again
		boost::shared_ptr<GInt32GaussAdaptor> giga_clone2_ptr;
		BOOST_CHECK_NO_THROW(giga_clone2_ptr = p_test->getAdaptor<GInt32GaussAdaptor>());

		// Check that the address hasn't changed
		BOOST_CHECK(ptr_store == giga_clone2_ptr.get());

		//********************************
	}

	//------------------------------------------------------------------------------

	// Reset to the original state
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32Object::specificTestsFailuresExpected_GUnitTests() {
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<boost::int32_t> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GNumIntT<boost::int32_t>::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Check that retrieval of the adaptor with simultaneous conversion to an incorrect target type throws (Test of GParameterBaseWithAdaptorsT<T> functions)
		boost::shared_ptr<GInt32Object> p_test = this->clone<GInt32Object>();

		// Make sure an adaptor is present
		BOOST_REQUIRE(p_test->hasAdaptor() == true);

		// Make sure the local adaptor has the type we expect
		BOOST_CHECK(p_test->getAdaptor()->getAdaptorId() == GINT32GAUSSADAPTOR);

		// Attempted conversion to an invalid target type should throw
		BOOST_CHECK_THROW(p_test->getAdaptor<GInt32FlipAdaptor>(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
