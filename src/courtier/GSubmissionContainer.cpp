/**
 * @file GSubmissionContainer.cpp
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

#include "courtier/GSubmissionContainer.hpp"

namespace Gem
{
namespace Courtier
{

/**********************************************************************************/
/**
 * The default constructor
 */
GSubmissionContainer::GSubmissionContainer()
{ /* nothing */ }

/**********************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSubmissionContainer object
 */
GSubmissionContainer::GSubmissionContainer(const GSubmissionContainer& cp)
	: id_(cp.id_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The destructor
 */
GSubmissionContainer::~GSubmissionContainer()
{ /* nothing */ }

/**********************************************************************************/
/**
 * Allows the courtier library to associate an id with the container
 *
 * @param id An id that allows the broker connector to identify this object
 */
void GSubmissionContainer::setCourtierId(const std::pair<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2>& id) {
	id_ = id;
}

/**********************************************************************************/
/**
 * Allows to retrieve the courtier-id associated with this container
 *
 * @return An id that allows the broker connector to identify this object
 */
std::pair<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> GSubmissionContainer::getCourtierId() const {
	return id_;
}

/**********************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */
