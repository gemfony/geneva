/**
 * @file GHelperFunctionsT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General
 * Public License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS DUAL-LICENSING OPTION DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * version 3 and of version 2 of the GNU General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
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
#include <boost/limits.hpp>
#include <boost/type_traits.hpp>

#ifndef GHELPERFUNCTIONST_HPP_
#define GHELPERFUNCTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Our own headers go here

namespace Gem
{
namespace Common
{

/**************************************************************************************************/
/**
 * This function takes two smart pointers and copies their contents (if any). Note that this
 * function might yield bad results for virtual types and will not work for purely virtual types.
 *
 * @param from The source smart pointer
 * @param to The target smart pointer
 */
template <typename T>
void copySmartPointer (
	const boost::shared_ptr<T>& from
	, boost::shared_ptr<T>& to
) {
	// Make sure to is empty when from is empty
	if(!from) {
		to.reset();
	} else {
		if(!to) {
			to.reset(new T(*from));
		} else {
			*to = *from;
		}
	}
}

/**************************************************************************************************/
/**
 * This function takes two vectors of boost::shared_ptr smart pointers and copies
 * one into the other. As we want to make a deep copy of the smart pointers' contents
 * this can be quite complicated. Note that we assume here that the objects pointed to
 * can be copied using an operator=(). The function also assumes the existence of
 * a valid copy constructor.  Note that this function might yield bad results for
 * virtual types.
 *
 * @param from The vector used as the source of the copying
 * @param to The vector used as the target of the copying
 */
template <typename T>
void copySmartPointerVector(
		const std::vector<boost::shared_ptr<T> >& from
		, std::vector<boost::shared_ptr<T> >& to
) {
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
/**
 * Takes two std::vector<> and subtracts each position of the second vector from the
 * corresponding position of the first vector. Note that we assume here that T understands
 * the operator-= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector from whose elements numbers will be subtracted
 * @param b The vector whose elements will be subtracted from the elements of a
 */
template <typename T>
void subtractVec (
		std::vector<T>& a
		, const std::vector<T>& b
) {
#ifdef DEBUG
	// Do some error checking
	if(a.size() != b.size()) {
		raiseException(
				"In subtractVec(std::vector<T>, const std::vector<T>&): Error!" << std::endl
				<< "Found invalid sizes: " << a.size() << " / " << b.size() << std::endl
		);
	}
#endif /* DEBUG */

	typename std::vector<T>::iterator it_a;
	typename std::vector<T>::const_iterator cit_b;

	// Subtract the elements
	for(it_a=a.begin(), cit_b=b.begin(); it_a != a.end(); ++it_a, ++cit_b) {
		(*it_a) -= (*cit_b);
	}
}

/**************************************************************************************************/
/**
 * Takes two std::vector<> and adds each position of the second vector to the
 * corresponding position of the first vector. Note that we assume here that T understands
 * the operator+= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector to whose elements numbers will be added
 * @param b The vector whose elements will be added to the elements of a
 */
template <typename T>
void addVec (
		std::vector<T>& a
		, const std::vector<T>& b
) {
#ifdef DEBUG
	// Do some error checking
	if(a.size() != b.size()) {
		raiseException(
				"In addVec(std::vector<T>, const std::vector<T>&): Error!" << std::endl
				<< "Found invalid sizes: " << a.size() << " / " << b.size() << std::endl
		);
	}
#endif /* DEBUG */

	typename std::vector<T>::iterator it_a;
	typename std::vector<T>::const_iterator cit_b;

	// Subtract the elements
	for(it_a=a.begin(), cit_b=b.begin(); it_a != a.end(); ++it_a, ++cit_b) {
		(*it_a) += (*cit_b);
	}
}

/**************************************************************************************************/
/**
 * Multiplies each position of a std::vector<> with a constant. Note that we assume here that
 * T understands the operator*= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector to whose elements numbers will be added
 * @param c The constant which will be multiplied with each position of a
 */
template <typename T>
void multVecConst (
		std::vector<T>& a
		, const T& c
) {
	typename std::vector<T>::iterator it_a;

	// Subtract the elements
	for(it_a=a.begin(); it_a != a.end(); ++it_a) {
		(*it_a) *= c;
	}
}

/**************************************************************************************************/
/**
 * Assigns a constant value to each position of the vector. Note that we assume here that
 * T understands the operator*= . Note that after this function has been called, a will have changed.
 *
 * @param a The vector to whose elements c will be assigned
 * @param c The constant which will be assigned each position of a
 */
template <typename T>
void assignVecConst (
		std::vector<T>& a
		, const T& c
) {
	typename std::vector<T>::iterator it_a;

	// Assign c to each element
	for(it_a=a.begin(); it_a != a.end(); ++it_a) {
		(*it_a) = c;
	}
}

/**************************************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GHELPERFUNCTIONST_HPP_ */
