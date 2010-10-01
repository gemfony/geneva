/**
 * @file GOptimizationMonitor.cpp
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

#include "geneva/GOptimizationMonitor.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************************/
/**
 * The default constructor
 */
GOptimizationMonitor::GOptimizationMonitor()
	: quiet_(false)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GOptimizationMonitor object
 */
GOptimizationMonitor::GOptimizationMonitor(const GOptimizationMonitor& cp)
	: GObject(cp)
	, quiet_(cp.quiet_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The destructor
 */
GOptimizationMonitor::~GOptimizationMonitor()
{ /* nothing */ }

/**********************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GOptimizationMonitor::operator==(const GOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizationMonitor::operator==","cp", CE_SILENT);
}

/**********************************************************************************/
/**
 * Checks for inequality with another GOptimizationMonitor object
 *
 * @param  cp A constant reference to another GOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GOptimizationMonitor::operator!=(const GOptimizationMonitor&) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizationMonitor::operator!=","cp", CE_SILENT);
}

/**********************************************************************************/
/**
 * Checks whether this object fulfills a given expectation in relation to another object
 */
boost::optional<std::string> GOptimizationMonitor::checkRelationshipWith(
		const GObject&
		, const Gem::Common::expectation&
		, const double&
		, const std::string&
		, const std::string&
		, const bool&
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GOptimizationMonitor *p_load = GObject::conversion_cast<GOptimizationMonitor>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GOptimizationMonitor", y_name, withMessages));

	// ... there is no local data

	return evaluateDiscrepancies("GOptimizationMonitor", caller, deviations, e);
}

/**********************************************************************************/
/**
 * The actual information function
 */
void GOptimizationMonitor::informationFunction(const infoMode& im, GOptimizationAlgorithm * const) {
	if(quiet_) return;

	switch(im) {
	case Gem::Geneva::INFOINIT:
		firstInformation();
		break;
	case Gem::Geneva::INFOPROCESSING:
		cycleInformation();
		break;
	case Gem::Geneva::INFOEND:
		lastInformation();
		break;
	};
}

/**********************************************************************************/
/**
 * A function that is called once before the optimization starts
 */
void GOptimizationMonitor::firstInformation(GOptimizationAlgorithm * const)
{ /* nothing */ }

/**********************************************************************************/
/**
 * A function that is called during each optimization cycle
 */
void GOptimizationMonitor::cycleInformation(GOptimizationAlgorithm * const goa) {

}

/**********************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 */
void GOptimizationMonitor::lastInformation(GOptimizationAlgorithm * const goa)
{ /* nothing */ }

/**********************************************************************************/
/**
 * Loads the data of another GObject
 */
void GOptimizationMonitor::load_(const GObject* cp) {

}

/**********************************************************************************/
/**
 * Prevents any information from being emitted by this object
 */
void preventInformationEmission() {
	quiet_ = true;
}

/**********************************************************************************/
/**
 * Allows this object to emit information
 */
void allowInformationEmission() {
	quiet_ = false;
}

/**********************************************************************************/
/**
 * Allows to check whether the emission of information is prevented
 *
 * @return A boolean which indicates whether information emission is prevented
 */
bool informationEmissionPrevented() const {
	return quiet_;
}

#ifdef GENEVATESTING
/**********************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GOptimizationMonitor::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	return result;
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();
}

/**********************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
