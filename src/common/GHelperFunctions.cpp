/**
 * @file GHelperFunctions.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GHelperFunctions.hpp"

namespace Gem {
namespace Common {

unsigned int getNHardwareThreads(const unsigned int& defaultNThreads) {
	unsigned int nHardwareThreads = boost::thread::hardware_concurrency();
	if(nHardwareThreads > 0) {
#ifdef DEBUG
		std::cout << "Determined " << (int)nHardwareThreads << " as a suitable number of processing threads for this architecture." << std::endl;
#endif
		return nHardwareThreads;
	}
	else {
#ifdef DEBUG
		std::cout << "Could not get information regarding suitable number of threads." << std::endl
				  << "Using the default value " << defaultNThreads << " instead." << std::endl;
#endif

		return defaultNThreads;
	}
}

} /* namespace Common */
} /* namespace Gem */
