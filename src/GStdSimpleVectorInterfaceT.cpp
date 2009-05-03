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
template <> bool GStdSimpleVectorInterfaceT<double>::checkIsSimilarTo(const std::vector<double>& cp_data, const double& limit) const {
#ifdef GENEVATESTING
		// Check sizes
		if(data.size() != cp_data.size()) {
			std::cout << "//-----------------------------------------------------------------" << std::endl
				            << "Found dissimilarity in object of type GStdSimpleVectorInterfaceT<double>:" << std::endl
				            <<  "data (type std::vector<double>): Size = " << data.size() << std::endl
				            << "cp_data (type std::vector<double>): Size = " << cp_data.size() << std::endl;

			return false;
		}

		// Loop over all entries and find out which is wrong
		for(std::size_t i=0; i<data.size(); i++) {
			if(fabs(data.at(i) - cp_data.at(i)) > fabs(limit)) {
				std::cout << "//-----------------------------------------------------------------" << std::endl
					            << "Found dissimilarity in object of type GStdSimpleVectorInterfaceT<double>:" << std::endl
					            <<"data[" << i << "] (type std::vector<double>) " << data.at(i) << std::endl
					            << "cp_data[" << i << "] (type std::vector<double>) " << cp_data.at(i) << std::endl;

				return false;
			}
		}
#else
		if(	data != cp_data) return false;
#endif  /* GENEVATESTING  */

		return true;
}

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
