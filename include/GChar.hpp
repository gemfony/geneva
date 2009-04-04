/**
 * @file GChar.hpp
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

// Standard headers go here

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GCHAR_HPP_
#define GCHAR_HPP_

// GenEvA headers go here

#include "GParameterT.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a char type. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GString class instead. char types are mutated by the GCharAdaptor
 * in GenEvA.
 *
 * As the GParameterT template class holds a suitable specialization for char,
 * this class can be implemented as a simple typedef.
 */
typedef GParameterT<char> GChar;

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GCHAR_HPP_ */
