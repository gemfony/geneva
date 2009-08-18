/**
 * @file GParameterTCollectionT.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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

#include "GParameterTCollectionT.hpp"

#include "GDouble.hpp"
#include "GChar.hpp"
#include "GInt32.hpp"
#include "GBoolean.hpp"
#include "GBoundedDouble.hpp"
#include "GBoundedInt32.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterTCollectionT<Gem::GenEvA::GDouble>)
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterTCollectionT<Gem::GenEvA::GChar>)
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterTCollectionT<Gem::GenEvA::GInt32>)
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterTCollectionT<Gem::GenEvA::GBoolean>)
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterTCollectionT<Gem::GenEvA::GBoundedDouble>)
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterTCollectionT<Gem::GenEvA::GBoundedInt32>)
