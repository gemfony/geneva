/**
 * @file GSimpleContainer.cpp
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

#include "GSimpleContainer.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::Tests::GSimpleContainer)

namespace Gem {
namespace Courtier {
namespace Tests {

/********************************************************************************************/
/**
 * The default constructor -- only needed for de-serialization purposes.
 */
GSimpleContainer::GSimpleContainer()
	: m_stored_number(0)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The standard constructor -- Initialization with a single number (can e.g. be used as an id).
 *
 * @param snr The number to be stored in the object
 */
GSimpleContainer::GSimpleContainer(const std::size_t& snr)
	: m_stored_number(snr)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSimpleContainer object
 */
GSimpleContainer::GSimpleContainer(const GSimpleContainer& cp)
	: Gem::Courtier::GProcessingContainerT<GSimpleContainer, bool>(cp)
	, m_stored_number(cp.m_stored_number)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GSimpleContainer::~GSimpleContainer()
{ /* nothing */ }

/********************************************************************************************/
/**
 * Allows to specify the tasks to be performed for this object. We simply do nothing.
 *
 * @return A boolean which indicates whether a useful result was obtained
 */
void GSimpleContainer::process_() {
	/* nothing */
}

/********************************************************************************************/
/**
 * Allows to give an indication of the processing result; may not throw.
 */
bool GSimpleContainer::get_processing_result() const noexcept {
	return true;
}

/********************************************************************************************/
/**
 * Prints out this functions stored number
 */
void GSimpleContainer::print() {
	std::cout << "storedNumber_ = " << m_stored_number << std::endl;
}

/********************************************************************************************/

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */
