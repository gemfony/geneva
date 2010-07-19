/**
 * @file GHelperFunctionsT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/shared_ptr.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/date_time.hpp>
#include <boost/variant.hpp>

#ifndef GHELPERFUNCTIONST_HPP_
#define GHELPERFUNCTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Our own headers go here

namespace Gem
{
namespace Util
{


/**************************************************************************************************/
/**
 * This function takes two vectors of boost::shared_ptr smart pointers and copies
 * one into the other. As we want to make a deep copy of the smart pointers' contents
 * this can be quite complicated. Note that we assume here that the objects pointed to
 * can be copied using an operator=(). The function also assumes the existence of
 * a valid copy constructor.
 *
 * @param from The vector used as the source of the copying
 * @param to The vector used as the target of the copying
 */
template <typename T>
void copySmartPointerVector(const std::vector<boost::shared_ptr<T> >& from,
		                            std::vector<boost::shared_ptr<T> >& to) {
	typename std::vector<boost::shared_ptr<T> >::const_iterator it_from;
	typename std::vector<boost::shared_ptr<T> >::iterator it_to;

	std::size_t size_from = from.size();
	std::size_t size_to = to.size();

	if(size_from==size_to) { // The most likely case
		for(it_from=from.begin(), it_to=to.begin();
		     it_from!=from.end(), it_to!=to.end();
		     ++it_from, ++it_to) {
			**it_to=**it_from; // Uses T::operator=()
		}
	}
	else if(size_from > size_to) {
		// First copy the data of the first size_to items
		for(it_from=from.begin(), it_to=to.begin();
		     it_to!=to.end(); ++it_from, ++it_to) {
			**it_to=**it_from;
		}

		// Then attach copies of the remaining items
		for(it_from=from.begin()+size_to; it_from!=from.end(); ++it_from) {
			boost::shared_ptr<T> p(new T(**it_from));
			to.push_back(p);
		}
	}
	else if(size_from < size_to) {
		// First copy the initial size_foreight items over
		for(it_from=from.begin(), it_to=to.begin();
		     it_from!=from.end(); ++it_from, ++it_to) {
			**it_to=**it_from;
		}

		// Then resize the local vector. Surplus items will vanish
		to.resize(size_from);
	}
}

/**************************************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GHELPERFUNCTIONST_HPP_ */
