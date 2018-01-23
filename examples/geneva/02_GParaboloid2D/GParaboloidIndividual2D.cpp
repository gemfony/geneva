/**
 * @file GParaboloidIndividual2D.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#include "GParaboloidIndividual2D.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParaboloidIndividual2D)

namespace Gem {
namespace Geneva {

/********************************************************************************************/
/**
 * The default constructor. This function will add two double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
GParaboloidIndividual2D::GParaboloidIndividual2D()
	: GParameterSet()
	, M_PAR_MIN(-10.)
	, M_PAR_MAX(10.)
{
	for(std::size_t npar=0; npar<2; npar++) {
		// GConstrainedDoubleObject is constrained to [M_PAR_MIN:M_PAR_MAX[
		std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(M_PAR_MIN, M_PAR_MAX));
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
	, M_PAR_MIN(-10.)
	, M_PAR_MAX(10)
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
 * Loads the data of another GParaboloidIndividual2D, camouflaged as a GObject.
 *
 * @param cp A copy of another GParaboloidIndividual2D, camouflaged as a GObject
 */
void GParaboloidIndividual2D::load_(const GObject* cp)
{
	// Check that we are dealing with a GParaboloidIndividual2D reference independent of this object and convert the pointer
	const GParaboloidIndividual2D *p_load = Gem::Common::g_convert_and_compare<GObject, GParaboloidIndividual2D>(cp, this);

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
double GParaboloidIndividual2D::fitnessCalculation() {
	double result = 0.; // Will hold the result
	std::vector<double> parVec; // Will hold the parameters

	this->streamline(parVec); // Retrieve the parameters

	// Do the actual calculation
	for(auto const &d: parVec) {
		result += d*d;
	}

	return result;
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
