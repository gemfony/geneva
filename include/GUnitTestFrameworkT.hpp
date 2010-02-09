/**
 * @file GUnitTestFrameworkT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

/**
 * This file holds function templates that should be specialized for each Geneva class
 * in order to facilitate unit tests.
 */

// Standard heades go here

// Boost headers go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifndef GUNITTESTFRAMEWORKT_HPP_
#define GUNITTESTFRAMEWORKT_HPP_

// Geneva headers go here

/*************************************************************************************************/
/**
 * This function creates a new T object. It can be specialized by the tested objects e.g. in case
 * they do not have a default constructor.
 *
 * @return A pointer to a newly created T object
 */
template <typename T>
boost::shared_ptr<T> TFactory_GUnitTests() {
	return boost::shared_ptr<T>(new T());
}

/*************************************************************************************************/

#endif /* GUNITTESTFRAMEWORKT_HPP_ */
