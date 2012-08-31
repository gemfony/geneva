/**
 * @file GExternalEvaluatorIndividual.cpp
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

#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GExternalEvaluatorIndividual)

namespace Gem
{
namespace Geneva
{

/********************************************************************************************/
/**
 * The default constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual()
: GParameterSet()
{ /* empty */ }

/********************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GExternalEvaluatorIndividual object
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual& cp)
: Gem::Geneva::GParameterSet(cp)
{	/* nothing */ }

/********************************************************************************************/
/**
 * The standard destructor
 */
GExternalEvaluatorIndividual::~GExternalEvaluatorIndividual()
{ /* nothing */	}

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GExternalEvaluatorIndividual object
 * @return A constant reference to this object
 */
const GExternalEvaluatorIndividual& GExternalEvaluatorIndividual::operator=(const GExternalEvaluatorIndividual& cp){
	GExternalEvaluatorIndividual::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GExternalEvaluatorIndividual::operator==(const GExternalEvaluatorIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GExternalEvaluatorIndividual::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GExternalEvaluatorIndividual::operator!=(const GExternalEvaluatorIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GExternalEvaluatorIndividual::operator!=","cp", CE_SILENT);
}

/********************************************************************************************/
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
boost::optional<std::string> GExternalEvaluatorIndividual::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GExternalEvaluatorIndividual *p_load = gobject_conversion<GExternalEvaluatorIndividual>(&cp);
	GObject::selfAssignmentCheck<GExternalEvaluatorIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(Gem::Geneva::GParameterSet::checkRelationshipWith(cp, e, limit, "GExternalEvaluatorIndividual", y_name, withMessages));

	// ... no local data

	return evaluateDiscrepancies("GExternalEvaluatorIndividual", caller, deviations, e);
}

/********************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GExternalEvaluatorIndividual::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GParameterSet::addConfigurationOptions(gpb, showOrigin);

	// No local data
}

/********************************************************************************************/
/**
 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
 */
void GExternalEvaluatorIndividual::load_(const GObject* cp)
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GExternalEvaluatorIndividual *p_load = gobject_conversion<GExternalEvaluatorIndividual>(cp);
	GObject::selfAssignmentCheck<GExternalEvaluatorIndividual>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// No local data
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GExternalEvaluatorIndividual::clone_() const {
	return new GExternalEvaluatorIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GExternalEvaluatorIndividual::fitnessCalculation(){
	raiseException(
			"In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
			<< "This function is not supposed to be called for this individual." << std::endl
	);

	// Make the exception
	return 0.;
}

#ifdef GEM_TESTING
// Note: The following code is designed to mainly test parent classes

/******************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GExternalEvaluatorIndividual::modify_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

	// Change the parameter settings
	this->adapt();
	result = true;

	return result;
}

/******************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GExternalEvaluatorIndividual::specificTestsNoFailureExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	/* nothing */
}

/******************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GExternalEvaluatorIndividual::specificTestsFailuresExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	/* nothing */
}

/******************************************************************/

#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */
