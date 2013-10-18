/**
 * @file GConstrainedInt32Object.cpp
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

#include "geneva/GConstrainedInt32Object.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedInt32Object)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GConstrainedInt32Object::GConstrainedInt32Object()
	: GConstrainedIntT<boost::int32_t>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with boundaries only. A random value inside of the allowed ranges will
 * be assigned to the object.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedInt32Object::GConstrainedInt32Object(
		const boost::int32_t& lowerBoundary
		, const boost::int32_t& upperBoundary
)
	: GConstrainedIntT<boost::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedInt32Object::GConstrainedInt32Object (
		  const boost::int32_t& val
		, const boost::int32_t& lowerBoundary
		, const boost::int32_t& upperBoundary
)
	: GConstrainedIntT<boost::int32_t>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GConstrainedInt32Object object
 */
GConstrainedInt32Object::GConstrainedInt32Object(const GConstrainedInt32Object& cp)
	: GConstrainedIntT<boost::int32_t>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedInt32Object::GConstrainedInt32Object(const boost::int32_t& val)
	: GConstrainedIntT<boost::int32_t>(val)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GConstrainedInt32Object::~GConstrainedInt32Object()
{ /* nothing */ }

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
boost::int32_t GConstrainedInt32Object::operator=(const boost::int32_t& val) {
	return GConstrainedIntT<boost::int32_t>::operator=(val);
}

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GConstrainedInt32Object object
 * @return A constant reference to this object
 */
const GConstrainedInt32Object& GConstrainedInt32Object::operator=(const GConstrainedInt32Object& cp){
	GConstrainedInt32Object::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedInt32Object::clone_() const {
	return new GConstrainedInt32Object(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GConstrainedInt32Object object
 *
 * @param  cp A constant reference to another GConstrainedInt32Object object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedInt32Object::operator==(const GConstrainedInt32Object& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedInt32Object::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GConstrainedInt32Object object
 *
 * @param  cp A constant reference to another GConstrainedInt32Object object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedInt32Object::operator!=(const GConstrainedInt32Object& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedInt32Object::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedInt32Object::checkRelationshipWith (
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedInt32Object>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GConstrainedIntT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GConstrainedInt32Object", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedInt32Object", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedInt32Object::name() const {
   return std::string("GConstrainedInt32Object");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedInt32Object::int32Streamline(std::vector<boost::int32_t>& parVec) const {
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedInt32Object::int32Streamline(std::map<std::string, boost::int32_t>& parVec) const {
   parVec[this->getParameterName()] = this->value();
}

/******************************************************************************/
/**
 * Attach boundaries of type boost::int32_t to the vectors.
 *
 * @param lBndVec A vector of lower boost::int32_t parameter boundaries
 * @param uBndVec A vector of upper boost::int32_t parameter boundaries
 */
void GConstrainedInt32Object::int32Boundaries(
		std::vector<boost::int32_t>& lBndVec
		, std::vector<boost::int32_t>& uBndVec
) const {
	lBndVec.push_back(this->getLowerBoundary());
	uBndVec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a boost::int32_t value
 *
 * @return The number 1, as we own a single boost::int32_t parameter
 */
std::size_t GConstrainedInt32Object::countInt32Parameters() const {
	return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation
 * to the assigned value, so that it lies inside of the allowed value range.
 */
void GConstrainedInt32Object::assignInt32ValueVector(const std::vector<boost::int32_t>& parVec, std::size_t& pos) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
	   glogger
	   << "In GConstrainedInt32Object::assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&):" << std::endl
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
 * @param cp A copy of another GConstrainedInt32Object object, camouflaged as a GObject
 */
void GConstrainedInt32Object::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedInt32Object>(cp);

	// Load our parent class'es data ...
	GConstrainedIntT<boost::int32_t>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
void GConstrainedInt32Object::randomInit_() {
	GConstrainedIntT<boost::int32_t>::randomInit_();
}

/* ----------------------------------------------------------------------------------
 * Tested in GConstrainedIntT<int_type>::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedInt32Object::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GConstrainedIntT<boost::int32_t>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedInt32Object::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedInt32Object::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
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
	GConstrainedIntT<boost::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedInt32Object::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedInt32Object::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
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
	GConstrainedIntT<boost::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedInt32Object::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
