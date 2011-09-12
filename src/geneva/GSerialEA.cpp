/**
 * @file GSerialEA.cpp
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
#include "geneva/GSerialEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialEA)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GSerialEA::GSerialEA()
   : GBaseEA()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp Reference to another GSerialEA object
 */
GSerialEA::GSerialEA(const GSerialEA& cp)
   : GBaseEA(cp)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GSerialEA::~GSerialEA()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard assignment operator for GSerialEA objects.
 *
 * @param cp Reference to another GSerialEA object
 * @return A constant reference to this object
 */
const GSerialEA& GSerialEA::operator=(const GSerialEA& cp) {
	GSerialEA::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data from another GSerialEA object.
 *
 * @param vp Pointer to another GSerialEA object, camouflaged as a GObject
 */
void GSerialEA::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	const GSerialEA *p_load = this->gobject_conversion<GSerialEA>(cp);

	// First load our parent class'es data ...
	GBaseEA::load_(cp);

	// no local data ...
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialEA::clone_() const  {
	return new GSerialEA(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GSerialEA object
 *
 * @param  cp A constant reference to another GSerialEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialEA::operator==(const GSerialEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSerialEA::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GSerialEA object
 *
 * @param  cp A constant reference to another GSerialEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialEA::operator!=(const GSerialEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSerialEA::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
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
boost::optional<std::string> GSerialEA::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSerialEA *p_load = GObject::gobject_conversion<GSerialEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseEA::checkRelationshipWith(cp, e, limit, "GSerialEA", y_name, withMessages));

	// ... no local data

	return evaluateDiscrepancies("GSerialEA", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialEA::init() {
	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::init();

	// Put own initialization code here
}

/************************************************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialEA::finalize() {
    // Put own finalization code here

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::finalize();
}

/************************************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GSerialEA::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	// Call our parent class'es function
	GBaseEA::addConfigurationOptions(gpb, showOrigin);

	// No local data
}

/************************************************************************************************************/
/**
 * Adapt all children in sequence. Note that this also triggers their value
 * calculation, so this function needs to be overloaded for optimization in a
 * network context.
 */
void GSerialEA::adaptChildren()
{
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We start with the parents, if this is the first iteration. Their
	// initial fitness needs to be determined, if this is the MUPLUSNU_SINGLEEVAL
	// or MUNU1PRETAIN selection model.
	// Make sure we also evaluate the parents in the first iteration, if needed.
	// This is only applicable to the MUPLUSNU_SINGLEEVAL and MUNU1PRETAIN modes.
	if(inFirstIteration()) {
		switch(getSortingScheme()) {
		//--------------------------------------------------------------
		case SA:
		case MUPLUSNU_PARETO:
		case MUCOMMANU_PARETO: // The current setup will still allow some old parents to become new parents
		case MUPLUSNU_SINGLEEVAL:
		case MUNU1PRETAIN: // same procedure. We do not know which parent is best
			for(it=data.begin(); it!=data.begin() + getNParents(); ++it) {
				(*it)->fitness(0);
			}
			break;

		case MUCOMMANU_SINGLEEVAL:
			break; // nothing
		}
	}

	// Next we perform the adaption of each child individual in
	// sequence. Note that this can also trigger fitness calculation.
	for(it=data.begin()+getNParents(); it!=data.end(); ++it) {
		(*it)->adapt();
	}
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSerialEA::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GBaseEA::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialEA::specificTestsNoFailureExpected_GUnitTests() {

	//------------------------------------------------------------------------------

	{ // Call the parent class'es function
		boost::shared_ptr<GSerialEA> p_test = this->clone<GSerialEA>();

		// Fill p_test with individuals
		p_test->fillWithObjects();

		// Run the parent class'es tests
		p_test->GBaseEA::specificTestsNoFailureExpected_GUnitTests();
	}

	//------------------------------------------------------------------------------
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialEA::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseEA::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
