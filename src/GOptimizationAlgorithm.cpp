/**
 * @file GOptimizationAlgorithm.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "GOptimizationAlgorithm.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GOptimizationAlgorithm)

namespace Gem
{
namespace GenEvA
{
/******************************************************************/
/**
 * The default constructor
 */
GOptimizationAlgorithm::GOptimizationAlgorithm()
	:GMutableSetT<Gem::GenEvA::GIndividual>()
{
	gr.setRnrGenerationMode(Gem::Util::DEFAULTRNRGENMODE);
}

/******************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GOptimizationAlgorithm object
 */
GOptimizationAlgorithm::GOptimizationAlgorithm(const GOptimizationAlgorithm& cp)
:GMutableSetT<Gem::GenEvA::GIndividual>(cp),
 gr(cp.gr)
{ /* nothing */ }

/******************************************************************/
/**
 * The destructor
 */
GOptimizationAlgorithm::~GOptimizationAlgorithm()
{ /* nothing */ }

/******************************************************************/
/**
 * Checks for equality with another GOptimizationAlgorithm object
 *
 * @param  cp A constant reference to another GOptimizationAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GOptimizationAlgorithm::operator==(const GOptimizationAlgorithm& cp) const {
	return GOptimizationAlgorithm::isEqualTo(cp, boost::logic::indeterminate);
}

/******************************************************************/
/**
 * Checks for inequality with another GOptimizationAlgorithm object
 *
 * @param  cp A constant reference to another GOptimizationAlgorithm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GOptimizationAlgorithm::operator!=(const GOptimizationAlgorithm& cp) const {
	return !GOptimizationAlgorithm::isEqualTo(cp, boost::logic::indeterminate);
}

/******************************************************************/
/**
 * Checks for equality with another GOptimizationAlgorithm object. As we have no
 * local data, we just check for equality of the parent class-
 *
 * @param  cp A constant reference to another GOptimizationAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GOptimizationAlgorithm::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
	using namespace Gem::Util;

	// Check that we are indeed dealing with a GIndividual reference
	const GOptimizationAlgorithm *gis_load = GObject::conversion_cast(&cp,  this);

	// Check our parent class
	if(!GMutableSetT<Gem::GenEvA::GIndividual>::isEqualTo(*gis_load, expected)) return  false;

	// And then our local data
	if(!gr.isEqualTo(gis_load->gr, expected)) return false;

	return true;
}

/******************************************************************/
/**
 * Checks for similarity with another GOptimizationAlgorithm object. As we have
 * no local data, we just check for similarity of the parent class.
 *
 * @param  cp A constant reference to another GOptimizationAlgorithm object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GOptimizationAlgorithm::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
	using namespace Gem::Util;

	// Check that we are indeed dealing with a GIndividual reference
	const GOptimizationAlgorithm *gis_load = GObject::conversion_cast(&cp,  this);

	// Check our parent class
	if(!GMutableSetT<Gem::GenEvA::GIndividual>::isSimilarTo(*gis_load, limit, expected)) return  false;

	// And then our local data
	if(!gr.isSimilarTo(gis_load->gr, limit, expected)) return false;

	return true;
}

/******************************************************************/
/**
 * Determines whether production of random numbers should happen remotely
 * (RNRFACTORY) or locally (RNRLOCAL)
 *
 * @param rnrGenMode A parameter which indicates where random numbers should be produced
 */
void GOptimizationAlgorithm::setRnrGenerationMode(const Gem::Util::rnrGenerationMode& rnrGenMode) {
	gr.setRnrGenerationMode(rnrGenMode);
}

/******************************************************************/
/**
 * Retrieves the random number generators current generation mode.
 *
 * @return The current random number generation mode of the local generator
 */
Gem::Util::rnrGenerationMode GOptimizationAlgorithm::getRnrGenerationMode() const {
	return gr.getRnrGenerationMode();
}

/******************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
 */
void GOptimizationAlgorithm::load(const GObject* cp)
{
	const GOptimizationAlgorithm *gis_load = static_cast<const GOptimizationAlgorithm *> (cp);

	// Check that this object is not accidentally assigned to itself.
	if (gis_load == this) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::load(): Error!" << std::endl
		<< "Tried to assign an object to itself." << std::endl;

		throw geneva_error_condition(error.str());
	}

	// Load the parent class'es data
	GMutableSetT<Gem::GenEvA::GIndividual>::load(cp);

	// and then our local data
	gr.load(&(gis_load->gr));
}

/******************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
