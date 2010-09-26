/**
 * @file GDoubleObject.cpp
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
#include "geneva/GDoubleObject.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GDoubleObject)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GDoubleObject::GDoubleObject()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleObject object
 */
GDoubleObject::GDoubleObject(const GDoubleObject& cp)
	: GNumFPT<double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GDoubleObject::GDoubleObject(const double& val)
	: GNumFPT<double>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GDoubleObject::GDoubleObject(const double& lowerBoundary, const double& upperBoundary)
	: GNumFPT<double>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GDoubleObject::~GDoubleObject()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
double GDoubleObject::operator=(const double& val) {
	return GNumFPT<double>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GDoubleObject object
 * @return A constant reference to this object
 */
const GDoubleObject& GDoubleObject::operator=(const GDoubleObject& cp){
	GDoubleObject::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GDoubleObject::clone_() const {
	return new GDoubleObject(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDoubleObject object
 *
 * @param  cp A constant reference to another GDoubleObject object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleObject::operator==(const GDoubleObject& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDoubleObject::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDoubleObject object
 *
 * @param  cp A constant reference to another GDoubleObject object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleObject::operator!=(const GDoubleObject& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDoubleObject::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GDoubleObject::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDoubleObject>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GNumFPT<double>::checkRelationshipWith(cp, e, limit, "GDoubleObject", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GDoubleObject", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GDoubleObject::doubleStreamline(std::vector<double>& parVec) const {
	parVec.push_back(this->value());
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleObject object, camouflaged as a GObject
 */
void GDoubleObject::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDoubleObject>(cp);

	// Load our parent class'es data ...
	GNumFPT<double>::load_(cp);

	// ... no local data
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleObject::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumFPT<double>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleObject::specificTestsNoFailureExpected_GUnitTests() {
	// A few settings
	const std::size_t nTests = 10000;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<double> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GNumFPT<double>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test of GParameterT<T>'s methods for setting and retrieval of values
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		for(double d=0.; d<10; d+=0.01) {
			BOOST_CHECK_NO_THROW((*p_test) = d); // Setting using operator=()
			BOOST_CHECK(p_test->value() == d); // Retrieval through the value() function
			BOOST_CHECK_NO_THROW(p_test->setValue(d)); // Setting using the setValue() function
			BOOST_CHECK(p_test->value() == d); // Retrieval through the value() function
			BOOST_CHECK_NO_THROW(p_test->setValue_(d)); // Setting using the protected constant setValue_() function
			BOOST_CHECK(p_test->value() == d); // Retrieval through the value() function
		}
	}

	//------------------------------------------------------------------------------

	{ // Test automatic conversion to the target type, using GParameterT<T>'s operator T()
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		double target = -1.;
		for(double d=0.; d<10; d+=0.01) {
			BOOST_CHECK_NO_THROW(p_test->setValue(d)); // Setting using the setValue() function
			BOOST_CHECK_NO_THROW(target = *p_test); // Automatic conversion
			BOOST_CHECK(target == d); // Cross-check
		}
	}

	//------------------------------------------------------------------------------

	{ // Test the GParameterT<T>::adaptImpl() implementation
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		if(p_test->hasAdaptor()) {
			BOOST_CHECK_NO_THROW(*p_test = 1.);
			double origVal = *p_test;
			BOOST_CHECK(*p_test == 1.);
			BOOST_CHECK(origVal == 1.);

			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(p_test->adaptImpl());
				BOOST_CHECK(origVal != *p_test); // Should be different
				BOOST_CHECK_NO_THROW(origVal = *p_test);
			}
		}
	}

	//------------------------------------------------------------------------------

	{ // Test resetting, adding and retrieval of adaptors in GParameterBaseWithAdaptorsT<T>
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		// Remove any available local adaptor and cross-check
		BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
		BOOST_CHECK(p_test->hasAdaptor() == false);

		// Add a new adaptor. This should clone the adaptor
		BOOST_CHECK_NO_THROW(p_test->addAdaptor(gdga_ptr));

		// Check that we again have an adaptor
		BOOST_CHECK(p_test->hasAdaptor() == true);

		// Retrieve a pointer to the adaptor
		boost::shared_ptr<GAdaptorT<double> > p_adaptor_base;
		BOOST_CHECK(!p_adaptor_base);
		BOOST_CHECK_NO_THROW(p_adaptor_base = p_test->getAdaptor());

		// Check that we have indeed received an adaptor
		BOOST_CHECK(p_adaptor_base);

		// Retrieve another, converted pointer to the adaptor
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_clone_ptr;
		BOOST_CHECK(!gdga_clone_ptr);
		BOOST_CHECK_NO_THROW(gdga_clone_ptr = p_test->getAdaptor<GDoubleGaussAdaptor>());

		// Check that we have indeed received an adaptor
		BOOST_CHECK(gdga_clone_ptr);

		// The address of the original adaptor and of this one should differ
		BOOST_CHECK(gdga_clone_ptr.get() != gdga_ptr.get());

		// The adaptors should otherwise be identical
		BOOST_CHECK(*gdga_clone_ptr == *gdga_ptr);
	}

	//------------------------------------------------------------------------------

	// Remove the test adaptor
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
void GDoubleObject::specificTestsFailuresExpected_GUnitTests() {
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<double> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GNumFPT<double>::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test of GParameterBaseWithAdaptorsT<T>::addAdaptor() in case of an empty adaptor pointer
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		// Make sure no adaptor is present
		BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
		BOOST_CHECK(p_test->hasAdaptor() == false);

		// Add an empty boost::shared_ptr<GDoubleGaussAdaptor>. This should throw
		BOOST_CHECK_THROW(p_test->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>()), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an empty adaptor throws in GParameterBaseWithAdaptorsT<T>::getAdaptor() (Note: This is the non-templated version of the function)
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		// Make sure no adaptor is present
		BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
		BOOST_CHECK(p_test->hasAdaptor() == false);

		BOOST_CHECK_THROW(p_test->getAdaptor(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an empty adaptor throws in GParameterBaseWithAdaptorsT<T>::getAdaptor<>() (Note: This is the templated version of the function)
		boost::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

		// Make sure no adaptor is present
		BOOST_CHECK_NO_THROW(p_test->resetAdaptor());
		BOOST_CHECK(p_test->hasAdaptor() == false);

		BOOST_CHECK_THROW(p_test->getAdaptor<GDoubleGaussAdaptor>(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	// Note: Test for invalid conversion is done in GInt32Object

	//------------------------------------------------------------------------------

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
