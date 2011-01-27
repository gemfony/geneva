/**
 * @file GOpenCLIndividual.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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

#include <geneva-individuals/GOpenCLIndividual.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GOpenCLIndividual)

namespace Gem
{
namespace Geneva
{

/********************************************************************************************/
/**
 * The default constructor
 */
GOpenCLIndividual::GOpenCLIndividual()
: openCLTask_(DEFAULTOPENCLTASK)
{ /* nothing */ }

/********************************************************************************************/
/**
 * Initialization with the name of the OpenCL file
 *
 * @param openCLTask The name of the file holding the OpenCL code
 */
GOpenCLIndividual::GOpenCLIndividual(const std::string& openCLTask)
: openCLTask_(openCLTask)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GOpenCLIndividual object
 */
GOpenCLIndividual::GOpenCLIndividual(const GOpenCLIndividual& cp)
: GParameterSet(cp)
, openCLTask_(cp.openCLTask_)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GOpenCLIndividual::~GOpenCLIndividual()
{ /* nothing */ }

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GOpenCLIndividual
 */
const GOpenCLIndividual::GOpenCLIndividual& operator=(const GOpenCLIndividual& cp){
	GOpenCLIndividual::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GOpenCLIndividual object
 *
 * @param  cp A constant reference to another GOpenCLIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GOpenCLIndividual::operator==(const GOpenCLIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOpenCLIndividual::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GOpenCLIndividual object
 *
 * @param  cp A constant reference to another GOpenCLIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GOpenCLIndividual::operator!=(const GOpenCLIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOpenCLIndividual::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GOpenCLIndividual::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;

	// Check that we are not accidently assigning this object to itself
	GObject::selfAssignmentCheck<GOpenCLIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GOpenCLIndividual", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GOpenCLIndividual", openCLTask_, p_load->openCLTask_, "openCLTask_", "p_load->openCLTask_", e , limit));

	return evaluateDiscrepancies("GOpenCLIndividual", caller, deviations, e);
}

/********************************************************************************************/
/**
 * Allows to set a new OpenCL file
 *
 * @param openCLTask The name of a file holding the OpenCL code
 */
void GOpenCLIndividual::setOpenCLTaskFile(const std::string& openCLTask) {
	openCLTask_ = openCLTask;
}

/********************************************************************************************/
/**
 * Allows to retrieve the name of the file currently used for OpenCL calculations
 */
std::string GOpenCLIndividual::getOpenCLTaskFile() const {
	return openCLTask_;
}

/********************************************************************************************/
/**
 * Loads the data of another GOpenCLIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GOpenCLIndividual, camouflaged as a GObject
 */
void GOpenCLIndividual::load_(const GObject* cp){
	const GOpenCLIndividual *p_load = GObject::conversion_cast<GOpenCLIndividual>(cp);

	// Load our parent class'es data ...
	GParameterSet::load_(cp);

	// ... and then our local data
	openCLTask_ = p_load->openCLTask_;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GOpenCLIndividual::clone_() const {
	return new GOpenCLIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation. It will transmit the calculation to the graphics card
 *
 * @return The value of this object
 */
double GOpenCLIndividual::fitnessCalculation(){
	return 0.; // Make the compiler happy
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
