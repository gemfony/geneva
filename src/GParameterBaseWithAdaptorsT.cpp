/**
 * @file GParameterBaseWithAdaptorsT.cpp
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

#include "GParameterBaseWithAdaptorsT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/////////////////// Specialization for std::vector<bool> ////////////////////////////////////////
/***********************************************************************************************/
/**
 * This function applies the first adaptor of the adaptor sequence to a collection of values.
 * Note that the parameter of this function will get changed. The check for adaptors happens
 * inside applyFirstAdaptor. This is a specialization of a generic template function.
 *
 * @param collection A vector of values that shall be mutated
 */
template<>
void GParameterBaseWithAdaptorsT<bool>::applyFirstAdaptor(std::vector<bool> &collection) {
	std::vector<bool>::iterator it;
	for (it = collection.begin(); it != collection.end(); ++it)	{
		bool value = *it;
		applyFirstAdaptor(value);
		*it = value;
	}
}

/***********************************************************************************************/
/**
 * This function applies all adaptors of the adaptor sequence to a collection of values. Note that
 * the parameter of this function will get changed. This is a specialization of a generic template
 * function.
 *
 * @param collection A vector of values that shall be mutated
 */
template<>
void GParameterBaseWithAdaptorsT<bool>::applyAllAdaptors(std::vector<bool> &collection) {
	std::vector<bool>::iterator it;
	for (it = collection.begin(); it != collection.end(); ++it) {
		bool value = *it;
		applyAllAdaptors(value);
		*it = value;
	}
}

/***********************************************************************************************/
/**
 * This function applies a named adaptor to a collection of values. Note that the parameter of
 * this function will get changed. This is a specialization of a generic template function.
 *
 * @param adName The name of the adaptor to apply to the collection
 * @param collection The vector of values that shall be mutated
 */
template<>
void GParameterBaseWithAdaptorsT<bool>::applyNamedAdaptor(const std::string& adName, std::vector<bool> &collection) {
	GATvec::iterator pos;
	std::vector<bool>::iterator it;

	// Search for the adaptor
	if (findAdaptor(adName, pos)) {
		// Now apply the adaptor to all values of the collection in sequence
		for (it = collection.begin(); it != collection.end(); ++it) {
			bool value = *it;
			(*pos)->mutate(value);
			*it = value;
		}
	} else { // Error - bad adaptor called
		std::ostringstream error;
		error << "In GParameterBaseWithAdaptorsT<bool>::applyNamedAdaptor(string adName, std::vector<bool>& value):" << std::endl
			  << "Error: Named adaptor " << adName << " was not found." << std::endl;

		throw geneva_error_condition(error.str());
	}
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
