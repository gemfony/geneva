/**
 * @file GSerializableI.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here

#include <string>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#ifndef GSERIALIZABLEI_HPP_
#define GSERIALIZABLEI_HPP_

// Geneva header files go here

#include "GEnums.hpp" // For the serialization mode

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * This is an interface class that specifies the operations available
 * for "serializable" classes.
 */
class GSerializableI {
public:
	/** @brief The standard destructor */
	virtual ~GSerializableI() { /* nothing */ }

	/** @brief Create a text representation from this class */
	virtual std::string toString(const serializationMode&) = 0;

	/** @brief Initialize this class from a text representation */
	virtual void fromString(const std::string&,const serializationMode&) = 0;
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GSERIALIZABLEI_HPP_ */
