/**
 * @file GStdSimpleVectorInterfaceT.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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

#include "GStdSimpleVectorInterfaceT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * Specialization fo typeof(T) == typof(double).
 */
template <> bool GStdSimpleVectorInterfaceT<double>::isSimilarTo(const std::vector<double>& cp_data, const double& limit) const {
	// Check sizes
	if(data.size() != cp_data.size()) return false;

	std::vector<double>::const_iterator cp_it;
	std::vector<double>::const_iterator it;

	for(it=data.begin(), cp_it=cp_data.begin(); it!=data.end(), cp_it!=cp_data.end(); ++it, ++cp_it)	if(fabs(*it-*cp_it) > fabs(limit)) return false;

	return true;
}

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
