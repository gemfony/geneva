/**
 * @file GSerializableI.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard header files go here
#include <string>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#ifndef GSERIALIZABLEI_HPP_
#define GSERIALIZABLEI_HPP_

// Geneva header files go here
#include "GCommonEnums.hpp" // For the serialization mode

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This is an interface class that specifies the operations available
 * for "serializable" classes.
 */
class GSerializableI {
public:
	/** @brief The standard destructor */
	virtual ~GSerializableI() { /* nothing */ }

	/** @brief Create a text representation from this class */
	virtual std::string toString(const serializationMode&) const = 0;

	/** @brief Initialize this class from a text representation */
	virtual void fromString(const std::string&, const serializationMode&) = 0;
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GSERIALIZABLEI_HPP_ */
