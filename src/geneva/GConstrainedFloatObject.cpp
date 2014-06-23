/**
 * @file GConstrainedFloatObject.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#include "geneva/GConstrainedFloatObject.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedFloatObject)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GConstrainedFloatObject::GConstrainedFloatObject()
	: GConstrainedFPT<float>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedFloatObject::GConstrainedFloatObject(
		const float& lowerBoundary
		, const float& upperBoundary
)
	: GConstrainedFPT<float>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedFloatObject::GConstrainedFloatObject (
		  const float& val
		, const float& lowerBoundary
		, const float& upperBoundary
)
	: GConstrainedFPT<float>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GConstrainedFloatObject object
 */
GConstrainedFloatObject::GConstrainedFloatObject(const GConstrainedFloatObject& cp)
	: GConstrainedFPT<float>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedFloatObject::GConstrainedFloatObject(const float& val)
	: GConstrainedFPT<float>(val)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GConstrainedFloatObject::~GConstrainedFloatObject()
{ /* nothing */ }

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
float GConstrainedFloatObject::operator=(const float& val) {
	return GConstrainedFPT<float>::operator=(val);
}

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GConstrainedFloatObject object
 * @return A constant reference to this object
 */
const GConstrainedFloatObject& GConstrainedFloatObject::operator=(const GConstrainedFloatObject& cp){
	GConstrainedFloatObject::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedFloatObject::clone_() const {
	return new GConstrainedFloatObject(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GConstrainedFloatObject object
 *
 * @param  cp A constant reference to another GConstrainedFloatObject object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedFloatObject::operator==(const GConstrainedFloatObject& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedFloatObject::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GConstrainedFloatObject object
 *
 * @param  cp A constant reference to another GConstrainedFloatObject object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedFloatObject::operator!=(const GConstrainedFloatObject& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedFloatObject::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GConstrainedFloatObject::checkRelationshipWith (
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedFloatObject>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GConstrainedFPT<float>::checkRelationshipWith(cp, e, limit, "GConstrainedFloatObject", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedFloatObject", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedFloatObject::name() const {
   return std::string("GConstrainedFloatObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedFloatObject::floatStreamline(std::vector<float>& parVec) const {
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 *
 * @param parVec The map to which the local value should be attached
 */
void GConstrainedFloatObject::floatStreamline(std::map<std::string, std::vector<float> >& parVec) const {
   std::vector<float> parameters;
   parameters.push_back(this->value());
   parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GConstrainedFloatObject::assignFloatValueVectors(const std::map<std::string, std::vector<float> >& parMap) {
   this->setValue(
      this->transfer(
         Gem::Common::getMapItem(
            parMap
            ,this->getParameterName()
         ).at(0)
      )
   );
}

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors.
 *
 * @param lBndVec A vector of lower float parameter boundaries
 * @param uBndVec A vector of upper float parameter boundaries
 */
void GConstrainedFloatObject::floatBoundaries(
		std::vector<float>& lBndVec
		, std::vector<float>& uBndVec
) const {
	lBndVec.push_back(this->getLowerBoundary());
	uBndVec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a float value. The function will act
 * differently depending on the argument. E.g., if the ACTIVEONLY parameter is
 * given, it will return 0 unless this parameter is active. Likewise, you may pass
 * INACTIVEONLY and will retrieve a 0 unless this parameter type is active.
 * Passing ALLPARAMETERS will just return the number of parameters stored in this
 * object.
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of active, incactive or all float parameters
 */
std::size_t GConstrainedFloatObject::countFloatParameters(
   const activityMode& am
) const {
   return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation to
 * the parameter value, so that it lies inside of the allowed value range.
 */
void GConstrainedFloatObject::assignFloatValueVector(const std::vector<float>& parVec, std::size_t& pos) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
	   glogger
	   << "In GConstrainedFloatObject::assignFloatValueVector(const std::vector<float>&, std::size_t&):" << std::endl
      << "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
      << GEXCEPTION;
	}
#endif

	this->setValue(this->transfer(parVec[pos]));
	pos++;
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedFloatObject object, camouflaged as a GObject
 */
void GConstrainedFloatObject::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedFloatObject>(cp);

	// Load our parent class'es data ...
	GConstrainedFPT<float>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedFloatObject::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if(GConstrainedFPT<float>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFloatObject::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedFloatObject::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	// Some general settings
	const float testVal = 42.;
	const float testVal2 = 17.;
	const float lowerBoundary = 0.;
	const float upperBoundary = 100.;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<float> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GFloatGaussAdaptor> gdga_ptr(new GFloatGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GConstrainedFPT<float>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Check that assignment of a value with operator= works both for set and unset boundaries
		boost::shared_ptr<GConstrainedFloatObject> p_test = this->GObject::clone<GConstrainedFloatObject>();

		// Reset the boundaries so we are free to do what we want
		BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

		// Assign a value with operator=
		BOOST_CHECK_NO_THROW(*p_test = testVal2);

		// Check the value
		BOOST_CHECK(p_test->value() == testVal2);

		// Assign boundaries and values
		BOOST_CHECK_NO_THROW(p_test->setValue(testVal2, lowerBoundary, upperBoundary));

		// Check the value again
		BOOST_CHECK(p_test->value() == testVal2);

		// Assign a value with operator=
		BOOST_CHECK_NO_THROW(*p_test = testVal);

		// Check the value again, should have changed
		BOOST_CHECK(p_test->value() == testVal);
	}

	//------------------------------------------------------------------------------

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFloatObject::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedFloatObject::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING

   //------------------------------------------------------------------------------
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<float> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GFloatGaussAdaptor> gdga_ptr(new GFloatGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GConstrainedFPT<float>::specificTestsFailuresExpected_GUnitTests();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

   //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFloatObject::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
