/**
 * @file GenevaHelperFunctions.hpp
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

#include "geneva/GenevaHelperFunctions.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Sets the DO_PROCESS flag in a given range, the IGNORE flag in the rest of the vector
 *
 * @param workItems The items for which the processing flag should be set
 * @param range A tuple with the half-open range inside of the vector, where the flags should be set
 */
void setProcessingFlag(
	std::vector<std::shared_ptr<GParameterSet>>& workItems
	, const std::tuple<std::size_t, std::size_t>& range
) {
	std::size_t start = std::get<0>(range);
	std::size_t end   = std::get<1>(range);

	for(auto p_it = workItems.begin(); p_it != workItems.begin() + start; ++p_it) {
		(*p_it)->reset_processing_status(Gem::Courtier::processingStatus::IGNORE);
	}
	for(auto p_it = workItems.begin() + start; p_it != workItems.begin() + end; ++p_it) {
		(*p_it)->reset_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
	}
	for(auto p_it = workItems.begin() + end; p_it != workItems.end(); ++p_it) {
		(*p_it)->reset_processing_status(Gem::Courtier::processingStatus::IGNORE);
	}
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
