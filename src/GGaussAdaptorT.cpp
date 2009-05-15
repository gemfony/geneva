/**
 * @file GGaussAdaptorT.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

#include "GGaussAdaptorT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
//////////////////////// Specialization for various numeric types ///////////////////////////////
/***********************************************************************************************/
/**
 * Specialization for typeof(num_type) == typeof(double). Main difference to the generic version:
 * The potentially time-consuming "boost::numeric_cast" is missing -- gaussRandom emits doubles.
 *
 * @param value The value to be mutated
 */
template<>
void GGaussAdaptorT<double>::customMutations(double& value) {
		// adapt the value in situ. Note that this changes
		// the argument of this function
#if defined (CHECKOVERFLOWS) || defined (DEBUG)
		// Prevent over- and underflows. Note that we currently do not check the
		// size of "addition" in comparison to "value".
		double addition = this->gr.gaussRandom(0.,sigma_);

		if(value >= 0){
			if(addition >= 0. && (std::numeric_limits<double>::max()-value < addition)) addition *= -1.;
		}
		else { // < 0
			if(addition < 0. && (std::numeric_limits<double>::min()-value > addition)) addition *= -1.;
		}

		value += addition;
#else
		// We do not check for over- or underflows for performance reasons.
		value += this->gr.gaussRandom(0.,sigma_);
#endif /* CHECKOVERFLOWS || DEBUG */
}

/***********************************************************************************************/
/**
 * Specialization for typeof(num_type) == typeof(char). We want to prevent the usage of
 * GGaussAdaptor for this type as its value range is so limited. There should be a
 * GIntFlipAdaptor in the Geneva library, that is better suited for the mutation of char
 * variables.
 *
 * @param value The value to be mutated
 */
template<>
void GGaussAdaptorT<char>::customMutations(char& value) {
	std::ostringstream error;
	error << "In GGaussAdaptorT<char>(): Error!" << std::endl
		  << "This adaptor should not be used for this type" << std::endl;

	throw geneva_error_condition(error.str());
}

/***********************************************************************************************/
/**
 * Specialization for typeof(num_type) == typeof(short). We want to prevent the usage of
 * GGaussAdaptor for this type as its value range is so limited. There should be a
 * GIntFlipAdaptor in the Geneva library, that is better suited for the mutation of short
 * variables.
 *
 * @param value The value to be mutated
 */
template<>
void GGaussAdaptorT<short>::customMutations(short& value) {
	std::ostringstream error;
	error << "In GGaussAdaptorT<short>(): Error!" << std::endl
		  << "This adaptor should not be used for this type" << std::endl;

	throw geneva_error_condition(error.str());
}

/***********************************************************************************************/
/**
 * Returns the id of this object
 *
 * @return The id of a GDoubleGaussAdaptor
 */
template<> Gem::GenEvA::adaptorId GGaussAdaptorT<double>::getAdaptorId() const {
	return Gem::GenEvA::GDOUBLEGAUSSADAPTOR;
}

/***********************************************************************************************/
/**
 * Returns the id of this object
 *
 * @return The id of a GInt32GaussAdaptor
 */
template<> Gem::GenEvA::adaptorId GGaussAdaptorT<boost::int32_t>::getAdaptorId() const {
	return Gem::GenEvA::GINT32GAUSSADAPTOR;
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
