/**
 * @file GDoubleCollection.hpp
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
#include <sstream>
#include <vector>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GDOUBLECOLLECTION_HPP_
#define GDOUBLECOLLECTION_HPP_

// GenEvA header files go here

#include "GNumCollectionT.hpp"


namespace Gem {
namespace GenEvA {

/** All "real" functionality is implemented in GNumCollectionT and its parent classes */
typedef GNumCollectionT<double> GDoubleCollection;

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLECOLLECTION_HPP_ */
