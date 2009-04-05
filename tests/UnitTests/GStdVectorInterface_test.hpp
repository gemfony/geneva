/**
 * @file GStdVectorInterface_test.cpp
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

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#ifndef GSTDVECTORINTERFACE_TEST_HPP_
#define GSTDVECTORINTERFACE_TEST_HPP_

const std::size_t NITEMS = 100;

/*************************************************************************************************/
/**
 * Tests whether an object of type "vi" has the std::vector<items> interface. Items are
 * assumed to have the Geneva interface. vectorObject is assumed to store boost::shared_ptr
 * wrappers around pointers to the items. Hence the SP in the name, It stands for smart
 * pointers.
 */
template <typename vi, typename item>
void stdvectorinterfacetestSP(vi& vectorObject, const item& templItem, const item& findItem) {
	// Make sure the object is empty
	BOOST_CHECK_NO_THROW(vectorObject.clear());

	// Attach items
	for(std::size_t i=0; i<NITEMS; i++) {
		BOOST_CHECK_NO_THROW(vectorObject.push_back(boost::shared_ptr<item>(new item(templItem))));
	}

	BOOST_CHECK(vectorObject.size() == NITEMS);
	BOOST_CHECK(!vectorObject.empty());
	BOOST_CHECK(vectorObject.max_size() != 0);
	BOOST_CHECK(vectorObject.capacity() != 0);
	BOOST_CHECK_NO_THROW(vectorObject.reserve(1000));

	// Count objects in the collection
	BOOST_CHECK_NO_THROW((*vectorObject.at(2)) = findItem);
	BOOST_CHECK(vectorObject.count(findItem)==1);

	// Try the same with a shared_ptr<T> as search object
	boost::shared_ptr<item> findItem_ptr(new item(findItem));
	BOOST_CHECK(vectorObject.count(findItem_ptr)==1);

	// Find items in the collection
	typename vi::const_iterator begin_cit = vectorObject.begin(), find_cit;
	find_cit = vectorObject.find(findItem);
	BOOST_CHECK(find_cit - begin_cit == 2);

	// Try the same with a shared_ptr<T> as search object
	find_cit = vectorObject.find(findItem_ptr);
	BOOST_CHECK(find_cit - begin_cit == 2);
}

/*************************************************************************************************/

#endif /* GSTDVECTORINTERFACE_TEST_HPP_ */
