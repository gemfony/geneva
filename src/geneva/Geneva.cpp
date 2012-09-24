/**
 * @file Geneva.cpp
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

#include <geneva/Geneva.hpp>

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A termination handler
 */
void GTerminate() {
	std::cout
	<< "***********************************************************" << std::endl
	<< "* Note that there seems to be a bug in some Boost         *" << std::endl
	<< "* versions that prevents proper termination of Geneva.    *" << std::endl
	<< "* If you see this message it means that you are using     *" << std::endl
	<< "* one of the affected releases, so we have to force       *" << std::endl
	<< "* termination. Since this happens when all work has       *" << std::endl
	<< "* already been done, this will very likely have no effect *" << std::endl
	<< "* on your results. So you can safely ignore this message. *" << std::endl
	<< "***********************************************************" << std::endl;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
