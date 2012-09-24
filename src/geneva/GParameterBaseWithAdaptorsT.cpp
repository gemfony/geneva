/**
 * @file GParameterBaseWithAdaptorsT.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "geneva/GParameterBaseWithAdaptorsT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/////////////////// Specialization for std::vector<bool> ///////////////////////
/******************************************************************************/
/**
 * This function applies the first adaptor of the adaptor sequence to a collection of values.
 * Note that the parameter of this function will get changed. This is a specialization of a
 * generic template function.
 *
 * @param collection A vector of values that shall be adapted
 */
template<>
void GParameterBaseWithAdaptorsT<bool>::applyAdaptor(std::vector<bool>& collection) {
#ifdef DEBUG
		if(!adaptor_) {
			raiseException(
					"In GParameterBaseWithAdaptorsT<T>::applyAdaptor(std::vector<bool>& collection):" << std::endl
					<< "Error: No adaptor was found."
			);
		}
#endif /* DEBUG */

	std::vector<bool>::iterator it;
	for (it = collection.begin(); it != collection.end(); ++it)	{
		bool value = *it;
		adaptor_->adapt(value);
		*it = value;
	}
}

/******************************************************************************/

} /* namespace Geneva  */
} /* namespace Gem */
