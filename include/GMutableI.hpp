/**
 * @file
 */

/* GMutableI.hpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#ifndef GMUTABLEI_HPP_
#define GMUTABLEI_HPP_

// Geneva header files go here

namespace Gem {
namespace GenEvA {

/**
 * This is a simple interface class for mutable objects.
 */
class GMutableI {
public:
	/** @brief The standard destructor */
	virtual ~GMutableI(){ /* nothing */ }

	/** @brief Allows derivatives to be mutated */
	virtual void mutate(void) = 0;
};

}} /* namespace Gem::GenEvA */

#endif /* GMUTABLEI_HPP_ */
