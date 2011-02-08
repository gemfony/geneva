/**
 * @file GMainLoop.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include <common/GMainLoop.hpp>

namespace Gem {
namespace Common {

/***************************************************************************************/
/**
 * The default constructor. No local data -- hence nothing to do.
 */
GMainLoop::GMainLoop()
{ /* nothing */ }

/***************************************************************************************/
/**
 * The copy constructor. No local data -- hence nothing to do.
 */
GMainLoop::GMainLoop(const GMainLoop& cp)
{ /* nothing */ }

/***************************************************************************************/
/**
 * The destructor
 */
GMainLoop::~GMainLoop()
{ /* nothing */ }

/***************************************************************************************/
/**
 * The event loop.
 *
 * @return A boolean indicating whether processing was successful
 */
bool GMainLoop::run() {
	// Perform any preparatory work
	init();

	do {
		// This is the main logic, to be run in every cycle
		cycleLogic();
	} while(!halt()); // A custom halt criterion

	// Perform any necessary clean-up work
	return finalize();
}

/***************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
