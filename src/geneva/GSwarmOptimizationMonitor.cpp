/**
 * @file GSwarmOptimizationMonitor.cpp
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

#include "geneva/GSwarmOptimizationMonitor.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GSwarmOptimizationMonitor)

namespace Gem {
namespace Geneva {

/**********************************************************************************/
/**
 * The default constructor
 */
GSwarmOptimizationMonitor::GSwarmOptimizationMonitor()
    		{ /* nothing */ }

/**********************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSwarmOptimizationMonitor object
 */
GSwarmOptimizationMonitor::GSwarmOptimizationMonitor(const GSwarmOptimizationMonitor& cp)
	: GOptimizationMonitorT<GParameterSet>(cp)
  { /* nothing */ }

/**********************************************************************************/
/**
 * The destructor
 */
GSwarmOptimizationMonitor::~GSwarmOptimizationMonitor()
{ /* nothing */ }

/**********************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GSwarmOptimizationMonitor object
 * @return A constant reference to this object
 */
const GSwarmOptimizationMonitor& GSwarmOptimizationMonitor::operator=(const GSwarmOptimizationMonitor& cp){
	GSwarmOptimizationMonitor::load_(&cp);
	return *this;
}

/**********************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GSwarmOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarmOptimizationMonitor::operator==(const GSwarmOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarmOptimizationMonitor::operator==","cp", CE_SILENT);
}

/**********************************************************************************/
/**
 * Checks for inequality with another GSwarmOptimizationMonitor object
 *
 * @param  cp A constant reference to another GSwarmOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarmOptimizationMonitor::operator!=(const GSwarmOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarmOptimizationMonitor::operator!=","cp", CE_SILENT);
}

/**********************************************************************************/
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
boost::optional<std::string> GSwarmOptimizationMonitor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSwarmOptimizationMonitor *p_load = GObject::conversion_cast<GSwarmOptimizationMonitor >(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationMonitorT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GSwarmOptimizationMonitor", y_name, withMessages));

	// ... there is no local data

	return evaluateDiscrepancies("GSwarmOptimizationMonitor", caller, deviations, e);
}

/**********************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GSwarmOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	// This should always be the first statement in a custom optimization monitor
	GOptimizationMonitorT<GParameterSet>::firstInformation(goa);


}

/**********************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GSwarmOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	GOptimizationMonitorT<GParameterSet>::cycleInformation(goa);
}

/**********************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GSwarmOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {

	// This should always be the last statement in a custom optimization monitor
	GOptimizationMonitorT<GParameterSet>::lastInformation(goa);
}

/**********************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to anither GSwarmOptimizationMonitor object, camouflaged as a GObject
 */
void GSwarmOptimizationMonitor::load_(const GObject* cp) {
	const GSwarmOptimizationMonitor *p_load = conversion_cast<GSwarmOptimizationMonitor>(cp);

	// Load the parent classes' data ...
	GObject::load_(cp);

	// no local data
}

/**********************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GSwarmOptimizationMonitor::clone_() const {
	return new GSwarmOptimizationMonitor(*this);
}

#ifdef GENEVATESTING
/**********************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GSwarmOptimizationMonitor::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationMonitorT<GParameterSet>::modify_GUnitTests()) result = true;

	return result;
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationMonitorT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests();
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationMonitorT<GParameterSet>::specificTestsFailuresExpected_GUnitTests();
}

/**********************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************/
