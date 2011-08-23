/**
 * @file GEvaluator.cpp
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

#include "geneva/GEvaluator.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvaluator)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************************/
/**
 * The default constructor
 */
GEvaluator::GEvaluator()
	:eval_(0.)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GEvaluator object
 */
GEvaluator::GEvaluator(const GEvaluator& cp)
	:eval_(cp.eval_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor
 */
GEvaluator::~GEvaluator()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Checks for equality with another GEvaluator object
 *
 * @param cp A copy of another GEvaluator object
 */
bool GEvaluator::operator==(const GEvaluator& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GEvaluator::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GEvaluator object
 *
 * @param cp A copy of another GEvaluator object
 */
bool GEvaluator::operator!=(const GEvaluator& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GEvaluator::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GEvaluator::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEvaluator *p_load = GObject::gobject_conversion<GEvaluator>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GEvaluator", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GEvaluator", eval_, p_load->eval_, "eval_", "p_load->eval_", e , limit));

	return evaluateDiscrepancies("GEvaluator", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Triggers the fitness calculation, stores and returns the result
 *
 * @param gps A constant pointer to the surrounding GParameterSet object
 * @return The fitness of the current object
 */
double GEvaluator::fitness(GParameterSet * const gps) {
	eval_ = this->fitnessCalculation(gps);
	return eval_;
}

/************************************************************************************************************/
/**
 * Returns the cached (i.e. last known) result of the fitness calculation
 */
double GEvaluator::getCachedFitness() const {
	return eval_;
}

/************************************************************************************************************/
/**
 * Loads the data of another GEvaluator, camouflaged as a GObject
 *
 * @param cp A copy of another GEvaluator object, camouflaged as a GObject
 */
void GEvaluator::load_(const GObject* cp) {
	const GEvaluator *p_load = gobject_conversion<GEvaluator>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Then load our local data
	eval_ = p_load->eval_;
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject* GEvaluator::clone_() const {
	return new GEvaluator(*this);
}

/************************************************************************************************************/
/**
 * The actual fitness calculation
 *
 * @param gps A constant pointer to the surrounding GParameterSet object
 * @return The fitness of the current object
 */
double GEvaluator::fitnessCalculation(GParameterSet * const gps) const {
	raiseException(
			"In GEvaluator::fitnessCalculation()" << std::endl
			<< "Function called directly which should not happen"
	);
}

/************************************************************************************************************/

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GEvaluator::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvaluator::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvaluator::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
