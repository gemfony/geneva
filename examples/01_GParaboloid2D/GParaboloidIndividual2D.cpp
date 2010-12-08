/**
 * @file GParaboloidIndividual2D.hpp
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

#include "GParaboloidIndividual2D.hpp"

/* Included here so no conflicts occur */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GParaboloidIndividual2D)

namespace Gem
{
namespace Geneva
{

/********************************************************************************************/
/**
 * The default constructor. This function will add two double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
GParaboloidIndividual2D::GParaboloidIndividual2D()
	: GParameterSet()
	, PAR_MIN_(-10.)
	, PAR_MAX_(10.)
{
	for(std::size_t npar=0; npar<2; npar++) {
		// GConstrainedDoubleObject cannot assume value below or above PAR_MIN_/MAX_
		boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(PAR_MIN_, PAR_MAX_));
		// Assign a random value in the expected range
		gcdo_ptr->setValue(gr.uniform_real(PAR_MIN_, PAR_MAX_));
		// Add the parameters to this individual
		this->push_back(gcdo_ptr);
	}
}

/********************************************************************************************/
/**
 * A standard copy constructor. All real work is done by the parent class.
 *
 * @param cp A copy of another GParaboloidIndividual2D
 */
GParaboloidIndividual2D::GParaboloidIndividual2D(const GParaboloidIndividual2D& cp)
	: GParameterSet(cp)
	, PAR_MIN_(-10.)
	, PAR_MAX_(10)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The standard destructor. Note that you do not need to care for the parameter objects
 * added in the constructor. Upon destruction, they will take care of releasing the allocated
 * memory.
 */
GParaboloidIndividual2D::~GParaboloidIndividual2D()
{ /* nothing */	}

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GParaboloidIndividual2D object
 * @return A constant reference to this object
 */
const GParaboloidIndividual2D& GParaboloidIndividual2D::operator=(const GParaboloidIndividual2D& cp){
	GParaboloidIndividual2D::load_(&cp);
	return *this;
}

/********************************************************************************************/
/**
 * Loads the data of another GParaboloidIndividual2D, camouflaged as a GObject.
 *
 * @param cp A copy of another GParaboloidIndividual2D, camouflaged as a GObject
 */
void GParaboloidIndividual2D::load_(const GObject* cp)
{
	const GParaboloidIndividual2D *p_load = GObject::conversion_cast<GParaboloidIndividual2D>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// No local data
	// sampleVariable = p_load->sampleVariable;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GParaboloidIndividual2D::clone_() const {
	return new GParaboloidIndividual2D(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GParaboloidIndividual2D::fitnessCalculation(){
	double result = 0.; // Will hold the result
	std::vector<double> parVec; // Will hold the parameters

	this->streamline(parVec); // Retrieve the parameters

	// Do the actual calculation
	for(std::size_t i=0; i<parVec.size(); i++) {
		result += parVec[i]*parVec[i];
	}

	return result;
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
