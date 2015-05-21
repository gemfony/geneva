/**
 * @file GTSSAccessT.hpp
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


// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here
#include <boost/thread/tss.hpp>

#ifndef INCLUDE_COMMON_GTSSACCESST_HPP_
#define INCLUDE_COMMON_GTSSACCESST_HPP_

// Common headers go here

namespace Gem {
namespace Common {

/***********************************************************************************/
/**
 * This function instantiates and gives access to an arbitrary, user-defined
 * variable or object to be stored in thread-local storage.
 */
template<typename tss_type>
inline boost::thread_specific_ptr<tss_type> &tss_ptr() {
	static boost::thread_specific_ptr<tss_type> instance;
	if (!instance.get()) {
		instance.reset(new tss_type());
	}
	return instance;
}

/**
 * Tested through examples/hap/05_GHapUsagePatterns
 */

/***********************************************************************************/

} /* Common */
} /* Gem */


#endif /* INCLUDE_COMMON_GTSSACCESST_HPP_ */
