/**
 * @file GExternalSetterIndividual.cpp
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

#include "geneva-individuals/GExternalSetterIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GExternalSetterIndividual)

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/**
 * The default constructor.
 */
GExternalSetterIndividual::GExternalSetterIndividual()
: GParameterSet()
{ /* empty */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GExternalSetterIndividual object
 */
GExternalSetterIndividual::GExternalSetterIndividual(const GExternalSetterIndividual& cp)
: Gem::Geneva::GParameterSet(cp)
{	/* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GExternalSetterIndividual::~GExternalSetterIndividual()
{ /* nothing */	}

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GExternalSetterIndividual object
 * @return A constant reference to this object
 */
const GExternalSetterIndividual& GExternalSetterIndividual::operator=(const GExternalSetterIndividual& cp){
	GExternalSetterIndividual::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GExternalSetterIndividual object
 *
 * @param  cp A constant reference to another GExternalSetterIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GExternalSetterIndividual::operator==(const GExternalSetterIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GExternalEvaluatorIndividual::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GExternalSetterIndividual object
 *
 * @param  cp A constant reference to another GExternalSetterIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GExternalSetterIndividual::operator!=(const GExternalSetterIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GExternalEvaluatorIndividual::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GExternalSetterIndividual::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GExternalSetterIndividual *p_load = gobject_conversion<GExternalSetterIndividual>(&cp);
	GObject::selfAssignmentCheck<GExternalSetterIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(Gem::Geneva::GParameterSet::checkRelationshipWith(cp, e, limit, "GExternalEvaluatorIndividual", y_name, withMessages));

	// ... no local data

	return evaluateDiscrepancies("GExternalEvaluatorIndividual", caller, deviations, e);
}

/******************************************************************************/
/**
 * Sets the fitness to a given set of values and clears the dirty flag. This is meant for external
 * methods of performing the actual evaluation.
 *
 * @param f The primary fitness value
 * @param sec_f_vec A vector of secondary fitness values
 */
void GExternalSetterIndividual::setFitness(
		const double& f
		, const std::vector<double>& sec_f_vec
) {
	GIndividual::setFitness_(f, sec_f_vec);
}


/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GExternalSetterIndividual::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GParameterSet::addConfigurationOptions(gpb, showOrigin);

	// No local data
}

/******************************************************************************/
/**
 * Loads the data of another GExternalSetterIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GExternalSetterIndividual, camouflaged as a GObject
 */
void GExternalSetterIndividual::load_(const GObject* cp)
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GExternalSetterIndividual *p_load = gobject_conversion<GExternalSetterIndividual>(cp);
	GObject::selfAssignmentCheck<GExternalSetterIndividual>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// No local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GExternalSetterIndividual::clone_() const {
	return new GExternalSetterIndividual(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GExternalSetterIndividual::fitnessCalculation(){
   glogger
   << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
   << "This function is not supposed to be called for this individual." << std::endl
   << GEXCEPTION;

	// Make the exception
	return 0.;
}

#ifdef GEM_TESTING
// Note: The following code is designed to mainly test parent classes

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GExternalSetterIndividual::modify_GUnitTests() {
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

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GExternalSetterIndividual::specificTestsNoFailureExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------------

	{ // Check that we can set the value of this object and that it isn't dirty afterwards
		double f = 0.; // For the fitness value
		const double FITNESS = 3.0;
		boost::shared_ptr<GExternalSetterIndividual> p_test = this->GObject::clone<GExternalSetterIndividual>();
		boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(1.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.,0.6,0.,2.));
		gdo_ptr->addAdaptor(gdga_ptr);
		p_test->push_back(gdo_ptr);

		BOOST_CHECK_NO_THROW(p_test->adapt());
		BOOST_CHECK(p_test->isDirty()); // The dirty flag should be set after mutation

		BOOST_CHECK_NO_THROW(p_test->setFitness(FITNESS,std::vector<double>()));
		BOOST_CHECK(!p_test->isDirty());
		BOOST_CHECK_NO_THROW(f = p_test->fitness());
		BOOST_CHECK(f == FITNESS);
	}

	//---------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GExternalSetterIndividual::specificTestsFailuresExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//---------------------------------------------------------------------------

	{ // Check that calling the fitness function throws
		boost::shared_ptr<GExternalSetterIndividual> p_test = this->GObject::clone<GExternalSetterIndividual>();
		boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(1.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.,0.6,0.,2.));
		gdo_ptr->addAdaptor(gdga_ptr);
		p_test->push_back(gdo_ptr);

		BOOST_CHECK_NO_THROW(p_test->adapt());
		BOOST_CHECK(p_test->isDirty()); // The dirty flag should be set after mutation
		BOOST_CHECK_THROW(p_test->fitness(), Gem::Common::gemfony_error_condition); // No direct evaluation is allowed for this object
	}

	//---------------------------------------------------------------------------

	{ // Check that supplying secondary fitness values when no corresponding variables have been registered throws
		const double FITNESS = 3.0;
		boost::shared_ptr<GExternalSetterIndividual> p_test = this->GObject::clone<GExternalSetterIndividual>();
		std::vector<double> x_vec;
		x_vec.push_back(1.);
		x_vec.push_back(2.);
		x_vec.push_back(3.);
		BOOST_CHECK_THROW(p_test->setFitness(FITNESS, x_vec), Gem::Common::gemfony_error_condition);
	}

	//---------------------------------------------------------------------------
}

/******************************************************************************/

#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */
