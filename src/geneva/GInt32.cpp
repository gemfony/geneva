/**
 * @file GInt32.cpp
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

#include "geneva/GInt32.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GInt32)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GInt32::GInt32()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32 object
 */
GInt32::GInt32(const GInt32& cp)
	: GNumT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GInt32::GInt32(const boost::int32_t& val)
	: GNumT<boost::int32_t>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GInt32::GInt32(const boost::int32_t& lowerBoundary, const boost::int32_t& upperBoundary)
	: GNumT<boost::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GInt32::~GInt32()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
boost::int32_t GInt32::operator=(const boost::int32_t& val) {
	return GNumT<boost::int32_t>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32 object
 * @return A constant reference to this object
 */
const GInt32& GInt32::operator=(const GInt32& cp){
	GInt32::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32::clone_() const {
	return new GInt32(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32 object
 *
 * @param  cp A constant reference to another GInt32 object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32::operator==(const GInt32& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInt32::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GInt32 object
 *
 * @param  cp A constant reference to another GInt32 object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32::operator!=(const GInt32& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInt32::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GInt32::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GNumT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GInt32", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32 object, camouflaged as a GObject
 */
void GInt32::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32>(cp);

	// Load our parent class'es data ...
	GNumT<boost::int32_t>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter.
 */
void GInt32::randomInit_() {
	boost::int32_t lowerBoundary = getLowerInitBoundary();
	boost::int32_t upperBoundary = getUpperInitBoundary()+1;

	setValue(gr->uniform_int(lowerBoundary, upperBoundary));
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumT<boost::int32_t>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32::specificTestsNoFailureExpected_GUnitTests() {
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
	GNumT<boost::int32_t>::specificTestsNoFailureExpected_GUnitTests();

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
void GInt32::specificTestsFailuresExpected_GUnitTests() {
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
	GNumT<boost::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
