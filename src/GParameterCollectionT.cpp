/**
 * @file GParameterCollectionT.cpp
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

#include "GParameterCollectionT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 *  Checks for similarity with another GParameterCollectionT<double> object. A specialization
 *  for typeof(T)==typeof(double) is needed, as particularly during text I/O FP accuracy can lead
 *  to undesirable results. This function is mainly needed for testing purposes.
 *
 * @param  cp A constant reference to another GParameterCollectionT<T> object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
template <> bool GParameterCollectionT<double>::isSimilarTo(const GParameterCollectionT<double>& cp, const double& limit) const {
	// Check similarity of the parent class
	if(!GParameterBaseWithAdaptorsT<double>::isSimilarTo(cp, limit)) return false;

	// Check whether there are any differences higher than the allowed limit
	std::vector<double>::const_iterator it;
	std::vector<double>::const_iterator cp_it;
	for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it) {
		if(fabs(*it - *cp_it) > fabs(limit)) return false;
	}

	return true;
}

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
