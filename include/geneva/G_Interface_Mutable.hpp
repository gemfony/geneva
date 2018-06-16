/**
 * @file G_Interface_Mutable.hpp
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>

// Boost header files go here
#include <boost/exception/all.hpp>

// Geneva header files go here

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a interface class for mutable objects, in particular individuals
 */
class G_Interface_Mutable {
public:
	 /** @brief The standard constructor */
	 G_API_GENEVA G_Interface_Mutable() = default;
	 /** @brief Allows derivatives to be adapted */
	 virtual G_API_GENEVA std::size_t adapt() BASE = 0;

protected:
	 /**
 	  * The standard destructor. Making this destructor protected follows this
 	  * discussion: http://www.gotw.ca/publications/mill18.htm
 	  */
	 G_API_GENEVA ~G_Interface_Mutable() = default;

};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/

