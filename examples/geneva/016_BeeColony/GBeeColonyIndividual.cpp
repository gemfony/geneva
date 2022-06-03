/**
 * @file GBeeColonyIndividual.hpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "GBeeColonyIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBeeColonyIndividual)

namespace Gem {
namespace Geneva {

/********************************************************************************************/
/**
 * The default constructor. This function will add two double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
GBeeColonyIndividual::GBeeColonyIndividual()
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
 * @param cp A copy of another GBeeColonyIndividual
 */
GBeeColonyIndividual::GBeeColonyIndividual(const GBeeColonyIndividual& cp)
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
GBeeColonyIndividual::~GBeeColonyIndividual()
{ /* nothing */	}

/********************************************************************************************/
/**
 * Loads the data of another GBeeColonyIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GBeeColonyIndividual, camouflaged as a GObject
 */
void GBeeColonyIndividual::load_(const GObject* cp)
{
	// Check that we are dealing with a GBeeColonyIndividual reference independent of this object and convert the pointer
	const GBeeColonyIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GBeeColonyIndividual>(cp, this);

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
GObject* GBeeColonyIndividual::clone_() const {
	return new GBeeColonyIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GBeeColonyIndividual::fitnessCalculation() {
	double result = 0.; // Will hold the result
	std::vector<double> parVec; // Will hold the parameters

	this->streamline(parVec); // Retrieve the parameters

	// Do the actual calculation
	for(auto const &d: parVec) {
		result += (d)*(d);
	}

	return result;
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
