/**
 * @file GTestIndividual2.cpp
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

#include "geneva-individuals/GTestIndividual2.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual2)

namespace Gem
{
namespace Tests
{

/********************************************************************************************/
/**
 * The default constructor -- private, as it is only needed for (de-)serialization purposes
 */
GTestIndividual2::GTestIndividual2()
: GParameterSet()
{ /* nothing */ }

/********************************************************************************************/
/**
 * The standard constructor
 *
 * @param nObjects The number of parameters to be added to this individual
 */
GTestIndividual2::GTestIndividual2(const std::size_t& nObjects, const PERFOBJECTTYPE& otype)
: GParameterSet()
{
	using namespace Gem::Geneva;

	// Fill with the requested amount of data of the requested type
	switch(otype) {
	case PERFGDOUBLEOBJECT:
	{
		for(std::size_t i=0; i<nObjects; i++) {
			boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(0.));
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.0, 0.8, 0.0001, 1.,1.));
			gdo_ptr->addAdaptor(gdga_ptr);
			this->push_back(gdo_ptr);
		}
		break;
	}

	case PERFGCONSTRDOUBLEOBJECT:
	{
		for(std::size_t i=0; i<nObjects; i++) {
			boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(0.,-10.,10.));
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.0, 0.8, 0.0001, 1.,1.));
			gcdo_ptr->addAdaptor(gdga_ptr);
			this->push_back(gcdo_ptr);
		}
		break;
	}

	case PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION:
	{
		boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(0.,-10.,10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.0, 0.8, 0.0001, 1.,1.));
		gcdo_ptr->addAdaptor(gdga_ptr);
		boost::shared_ptr<GConstrainedDoubleObjectCollection> gcdc_ptr(new GConstrainedDoubleObjectCollection(nObjects,gcdo_ptr));
		break;
	}

	case PERFGDOUBLECOLLECTION:
	{
		boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(nObjects,0.,-10.,10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.0, 0.8, 0.0001, 1.,1.));
		gdc_ptr->addAdaptor(gdga_ptr);
		this->push_back(gdc_ptr);
		break;
	}

	case PERFGCONSTRAINEDDOUBLECOLLECTION:
	{
		boost::shared_ptr<GConstrainedDoubleCollection> gcdc_ptr(new GConstrainedDoubleCollection(nObjects,0.,-10.,10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(1.0, 0.8, 0.0001, 1.,1.));
		gcdc_ptr->addAdaptor(gdga_ptr);
		this->push_back(gcdc_ptr);
		break;
	}

	default:
	{
		std::cerr
		<< "In GTestIndividual2::GTestIndividual2(): Error!" << std::endl
		<< "Invalid object type requested: " << otype << std::endl;
		exit(1);
		break;
	}
	}
}

/********************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual2 object
 */
GTestIndividual2::GTestIndividual2(const GTestIndividual2& cp)
: Gem::Geneva::GParameterSet(cp)
{	/* nothing */ }

/********************************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual2::~GTestIndividual2()
{ /* nothing */	}

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GTestIndividual2 object
 * @return A constant reference to this object
 */
const GTestIndividual2& GTestIndividual2::operator=(const GTestIndividual2& cp){
	GTestIndividual2::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GTestIndividual2 object
 *
 * @param  cp A constant reference to another GTestIndividual2 object
 * @return A boolean indicating whether both objects are equal
 */
bool GTestIndividual2::operator==(const GTestIndividual2& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GTestIndividual2::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GTestIndividual2 object
 *
 * @param  cp A constant reference to another GTestIndividual2 object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GTestIndividual2::operator!=(const GTestIndividual2& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GTestIndividual2::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GTestIndividual2::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	const GTestIndividual2 *p_load = gobject_conversion<GTestIndividual2>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(Gem::Geneva::GParameterSet::checkRelationshipWith(cp, e, limit, "GTestIndividual2", y_name, withMessages));

	// no local data

	return evaluateDiscrepancies("GTestIndividual2", caller, deviations, e);
}

/********************************************************************************************/
/**
 * Loads the data of another GTestIndividual2, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual2, camouflaged as a GObject
 */
void GTestIndividual2::load_(const GObject* cp)
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	const GTestIndividual2 *p_load = gobject_conversion<GTestIndividual2>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// no local data
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GTestIndividual2::clone_() const {
	return new GTestIndividual2(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual2::fitnessCalculation(){
	double result = 0.;

	// We just calculate the square of all double values
	std::vector<double> parVec;
	this->streamline(parVec);

	// Calculate the value of the parabola
	for(std::size_t i=0; i<parVec.size(); i++) {
		result += GSQUARED(parVec[i]);
	}

	return result;
}

#ifdef GEM_TESTING

/******************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual2::modify_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

	// Change the parameter settings
	result = true;

	return result;
}

/******************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual2::specificTestsNoFailureExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
}

/******************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual2::specificTestsFailuresExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
}

/******************************************************************/

#endif /* GEM_TESTING */

} /* namespace Tests */
} /* namespace Gem */
