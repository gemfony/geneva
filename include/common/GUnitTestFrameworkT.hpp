/**
 * @file GUnitTestFrameworkT.hpp
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


/**
 * This file holds function templates that should be specialized for each Geneva class
 * in order to facilitate unit tests.
 */

// Standard heades go here

// Boost headers go here

#ifndef GUNITTESTFRAMEWORKT_HPP_
#define GUNITTESTFRAMEWORKT_HPP_

// Geneva headers go here

/******************************************************************************/
/**
 * This function creates a new T object. It can be specialized by the tested objects e.g. in case
 * they do not have a default constructor.
 *
 * @return A boost::shared_ptr to a newly created T object
 */
template <typename T>
boost::shared_ptr<T> TFactory_GUnitTests() {
	return boost::shared_ptr<T>(new T());
}

/******************************************************************************/

#endif /* GUNITTESTFRAMEWORKT_HPP_ */
