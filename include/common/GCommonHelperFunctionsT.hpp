/**
 * @file GCommonHelperFunctionsT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>
#include <tuple>
#include <limits>
#include <thread>
#include <chrono>
#include <type_traits>

// Boost headers go here
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/math/special_functions/next.hpp>
#include <boost/variant.hpp>
#include <boost/checked_delete.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>

#ifndef GCOMMONHELPERFUNCTIONST_HPP_
#define GCOMMONHELPERFUNCTIONST_HPP_

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This functions uses boosts checked_delete to delete a pointer, then assigns
 * nullptr to the pointer. As a consequence to the usage of checked_delete, T must
 * be
 * a "complete" type. The assignment of a nullptr is not done automatically
 * by C++ and helps to detect "empty" pointers.
 */
template <typename T>
void g_delete(T *&p) {
	if(p) {
		boost::checked_delete(p);
		p=nullptr;
	}
}

/******************************************************************************/
/**
 * This functions uses boosts checked_delete to delete a pointer, then assigns
 * nullptr to the pointer. As a consequence to the usage of checked_delete, T must
 * be
 * a "complete" type. The assignment of a nullptr is not done automatically
 * by C++ and helps to detect "empty" pointers.
 */
template <typename T>
void g_array_delete(T *&p) {
	if(p) {
		boost::checked_array_delete(p);
		p = nullptr;
	}
}

/******************************************************************************/
/**
 * This function checks in DEBUG mode whether two pointers point to the
 * same object. The function will throw if this is the case. This is needed
 * in order to prevent assignment of a pointer's content to itself. Both pointers
 * must be of the same type or must be convertible to each other. The function
 * will not throw in case p1 is a nullptr.
 *
 * @param p1 The first pointer to be checked
 * @param p2 The second pointer to be checked
 */
template <typename T>
void ptrDifferenceCheck (
	const T *p1
	, const T *p2
) {
#ifdef DEBUG
	// Check that the two pointers point to different objects
	if (nullptr!=p1 && p1==p2) {
		glogger
		<< "In Gem::Common::ptrEqualityCheck<T>() :" << std::endl
		<< "p1 and p2 point to the same object!" << std::endl
		<< GEXCEPTION;
	}
#endif
}

/******************************************************************************/
/**
 * This function checks in DEBUG mode whether two pointers point to the
 * same object. The function will throw if this is the case. This is needed
 * in order to prevent assignment of a pointer's content to itself. Both pointers
 * must be of the same type or must be convertible to each other. The function
 * will not throw in case p1 is a nullptr.
 *
 * @param p1 The first pointer to be checked
 * @param p2 The second pointer to be checked
 */
template <typename T>
void ptrDifferenceCheck (
	std::shared_ptr<T> p1
	, std::shared_ptr<T> p2
) {
#ifdef DEBUG
	// Check that the two pointers point to different objects
	if (p1 && p1.get()==p2.get()) {
		glogger
		<< "In Gem::Common::ptrEqualityCheck<T>() :" << std::endl
		<< "Smart pointers p1 and p2 point to the same object!" << std::endl
		<< GEXCEPTION;
	}
#endif
}


/******************************************************************************/
/**
 * This function converts the "convert_ptr" pointer to the target type.  Note that this template will
 * only be accessible to the compiler if base_type is a base type of target_type.  As a consequence, the
 * function allows up-casts, but no downcasts. The function will do nothing, if convert_ptr points to
 * nullptr.
 */
template <typename base_type, typename target_type>
const target_type * g_ptr_conversion (
	const base_type *convert_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
#ifdef DEBUG
	const target_type *p = dynamic_cast<const target_type *>(convert_ptr);

	if(nullptr==convert_ptr || p) {
		return p;
	} else {
		glogger
		<< "In const target_type* g_ptr_conversion<target_type, base_type>() :" << std::endl
		<< "Invalid conversion from type with name " << typeid(base_type).name() << std::endl
		<< "to type with name " << typeid(target_type).name() << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return nullptr;
	}
#else
	return static_cast<const target_type *>(convert_ptr);
#endif
}

/******************************************************************************/
/**
 * This function converts the "convert_ptr" pointer to the target type.  Note that this template will
 * only be accessible to the compiler if base_type is a base type of target_type. As a consequence, the
 * function allows up-casts, but no downcasts. The function will do nothing, if convert_ptr points to
 * nullptr.
 */
template <typename base_type, typename target_type>
std::shared_ptr<target_type> g_ptr_conversion (
	std::shared_ptr<base_type> convert_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
#ifdef DEBUG
	std::shared_ptr<target_type> p = std::dynamic_pointer_cast<target_type>(convert_ptr);

	if(nullptr==convert_ptr.get() || p) {
		return p;
	} else {
		glogger
		<< "In std::shared_ptr<target_type> g_ptr_conversion<target_type, base_type>() :" << std::endl
		<< "Invalid conversion from type with name " << typeid(base_type).name() << std::endl
		<< "to type with name " << typeid(target_type).name() << std::endl
		<< GEXCEPTION;
	}
#else
	return std::static_pointer_cast<target_type>(convert_ptr);
#endif
}

/******************************************************************************/
/**
 * This function will convert a "convert_ptr" to a given target type and will
 * check whether it points to the same object as another pointer supplied
 * as a function argument.  Note that this function will only be accessible to
 * the compiler if base_type is a base type of target_type. As a consequence, the
 + function allows up-casts, but no downcasts. The function will not throw for
 * nullptr-values.
 *
 * @param convert_ptr A base pointer to be converted to the target type
 * @param compare_ptr A pointer to be compared to convert_ptr
 */
template <typename base_type, typename target_type>
std::shared_ptr<target_type> g_convert_and_compare (
	std::shared_ptr<base_type> convert_ptr
	, std::shared_ptr<target_type> compare_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
	// Convert the base pointer -- this call will throw, if conversion cannot be done
	std::shared_ptr<target_type> p =  g_ptr_conversion<base_type, target_type>(convert_ptr);

	// Then compare the two pointers (will throw in case of equality)
	ptrDifferenceCheck(p, compare_ptr);

	// Return the converted pointer
	return p;
}

/******************************************************************************/
/**
 * This function will convert a "convert_ptr" to a given target type and will
 * check whether it points to the same object as another pointer supplied
 * as a function argument.  Note that this function will only be accessible to
 * the compiler if base_type is a base type of target_type. As a consequence, the
 + function allows up-casts, but no downcasts. The function will not throw for
 * nullptr-values.
 *
 * @param convert_ptr A base pointer to be converted to the target type
 * @param compare_ptr A pointer to be compared to convert_ptr
 */
template <typename base_type, typename target_type>
const target_type* g_convert_and_compare (
	const base_type * convert_ptr
	, const target_type * compare_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
	// Convert the base pointer -- this call will throw, if conversion cannot be done
	const target_type * p =  g_ptr_conversion<base_type, target_type>(convert_ptr);

	// Then compare the two pointers (will throw in case of equality)
	ptrDifferenceCheck(p, compare_ptr);

	// Return the converted pointer
	return p;
}

/******************************************************************************/
/**
 * This function will convert a "convert_ref" to a given target type and will
 * check whether it points to the same object as another pointer supplied
 * as a function argument.  Note that this function will only be accessible to
 * the compiler if base_type is a base type of target_type. As a consequence, the
 + function allows up-casts, but no downcasts. The function will not throw for
 * nullptr-values.
 *
 * @param convert_ref A reference to an object to be converted to the target type as a pointer
 * @param compare_ptr A pointer to be compared to convert_ptr
 */
template <typename base_type, typename target_type>
const target_type* g_convert_and_compare (
	const base_type& convert_ref
	, const target_type * compare_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
	// Convert the base pointer -- this call will throw, if conversion cannot be done
	const target_type * p =  g_ptr_conversion<base_type, target_type>(&convert_ref);

	// Then compare the two pointers (will throw in case of equality)
	ptrDifferenceCheck(p, compare_ptr);

	// Return the converted pointer
	return p;
}

/******************************************************************************/
/**
 * This function takes a std::vector and transforms its contents to a std::string.
 * Note that this function assumes, that the template type of the vector can
 * be streamed.
 *
 * @param vec The vector to be printed
 * @return A string-representation of the vector
 */
template<typename vecType>
std::string vecToString(const std::vector<vecType> &vec) {
	std::ostringstream result;
	typename std::vector<vecType>::const_iterator cit;
	for (cit = vec.begin(); cit != vec.end(); ++cit) {
		result << *cit << " ";
	}
	return result.str();
}

/******************************************************************************/
/**
 * This function takes two smart pointers and copies their contents (if any). Note that this
 * function might yield bad results for virtual types and will not work for purely virtual types.
 *
 * @param from The source smart pointer
 * @param to The target smart pointer
 */
template<typename T>
void copySmartPointer(
	const std::shared_ptr <T> &from, std::shared_ptr <T> &to
) {
	// Make sure to is empty when from is empty
	if (!from) {
		to.reset();
	} else {
		if (!to) {
			to.reset(new T(*from));
		} else {
			*to = *from;
		}
	}
}

/******************************************************************************/
/**
 * This function takes two vectors of std::shared_ptr smart pointers and copies
 * one into the other. As we want to make a deep copy of the smart pointers' contents
 * this can be quite complicated. Note that we assume here that the objects pointed to
 * can be copied using an operator=(). The function also assumes the existence of
 * a valid copy constructor.  Note that this function might yield bad results for
 * virtual types, when handled through a base class.
 *
 * @param from The vector used as the source of the copying
 * @param to The vector used as the target of the copying
 */
template<typename T>
void copySmartPointerVector(
	const std::vector<std::shared_ptr <T>>& from
	, std::vector<std::shared_ptr <T>>& to
) {
	typename std::vector<std::shared_ptr <T>>::const_iterator it_from;
	typename std::vector<std::shared_ptr <T>>::iterator it_to;

	std::size_t size_from = from.size();
	std::size_t size_to = to.size();

	if(size_from==size_to) { // The most likely case
		for(it_from = from.begin(), it_to = to.begin(); it_to!=to.end(); ++it_from, ++it_to) {
			**it_to = **it_from; // Uses T::operator=()
		}
	} else if(size_from > size_to) {
		// First copy the data of the first size_to items
		for(it_from = from.begin(), it_to = to.begin(); it_to!=to.end(); ++it_from, ++it_to) {
			**it_to = **it_from;
		}

		// Then attach copies of the remaining items
		for(it_from = from.begin() + size_to; it_from!=from.end(); ++it_from) {
			std::shared_ptr <T> p(new T(**it_from));
			to.push_back(p);
		}
	} else if(size_from<size_to) {
		// First copy the initial size_foreight items over
		for(it_from = from.begin(), it_to = to.begin(); it_from!=from.end(); ++it_from, ++it_to) {
			**it_to = **it_from;
		}

		// Then resize the local vector. Surplus items will vanish
		to.resize(size_from);
	}
}

/******************************************************************************/
/**
 * This function takes two smart pointers to cloneable objects and copies their contents (if any)
 * with the load / clone functions. std::enable_if makes sure that this function can only be called
 * if the object pointed to has a clone and load function
 *
 * @param from The source smart pointer
 * @param to The target smart pointer
 */
template <typename T>
void copyCloneableSmartPointer (
	const std::shared_ptr<T>& from
	, std::shared_ptr<T>& to
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<T>::value>::type *dummy = nullptr
) {
	// Make sure to is empty when from is empty
	if(!from) {
		to.reset();
	} else {
		if(!to) {
			to = from->T::template clone<T>();
		} else {
			to->T::load(from);
		}
	}
}

/******************************************************************************/
/**
 * This function copies a container of smart pointers to cloneable objects to another container.
 * It assumes the availability of a load- and clone-call.
 *
 * @param from The container used as the source of the copying
 * @param to The container used as the target of the copying
 */
template <typename T, template <typename, typename> class c_type>
void copyCloneableSmartPointerContainer(
	const c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>& from
	, c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>& to
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<T>::value>::type *dummy = nullptr
) {
	typename c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>::const_iterator it_from;
	typename c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>::iterator it_to;

	std::size_t size_from = from.size();
	std::size_t size_to   = to.size();

	if(size_from==size_to) { // The most likely case
		for(it_from=from.begin(), it_to=to.begin(); it_from!=from.end(); ++it_from, ++it_to) {
			copyCloneableSmartPointer(*it_from, *it_to);
		}
	} else if(size_from > size_to) {
		// First copy the data of the first size_to items
		for(it_from=from.begin(), it_to=to.begin(); it_to!=to.end(); ++it_from, ++it_to) {
			copyCloneableSmartPointer(*it_from, *it_to);
		}

		// Then attach copies of the remaining items
		for(it_from=from.begin()+size_to; it_from!=from.end(); ++it_from) {
			to.push_back((*it_from)->T::template clone<T>());
		}
	} else if(size_from < size_to) {
		// First copy the initial size_for items over
		for(it_from=from.begin(), it_to=to.begin(); it_from!=from.end(); ++it_from, ++it_to) {
			copyCloneableSmartPointer(*it_from, *it_to);
		}

		// Then resize the local vector. Surplus items will vanish
		to.resize(size_from);
	}
}

/******************************************************************************/
/**
 * This function copies a container of cloneable / loadable objects to another container
 * holding objects of the same type.
 *
 * @param from The container used as the source of the copying
 * @param to The container used as the target of the copying
 */
template <typename T, template <typename, typename> class c_type>
void copyCloneableObjectsContainer(
	const c_type<T, std::allocator<T>>& from
	, c_type<T, std::allocator<T>>& to
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<T>::value>::type *dummy = nullptr
) {
	typename c_type<T, std::allocator<T>>::const_iterator it_from;
	typename c_type<T, std::allocator<T>>::iterator it_to;

	std::size_t size_from = from.size();
	std::size_t size_to = to.size();

	if(size_from==size_to) { // The most likely case
		for(it_from=from.begin(), it_to=to.begin(); it_from!=from.end(); ++it_from, ++it_to) {
			it_to->T::load(*it_from);
		}
	} else if(size_from > size_to) {
		// First copy the data of the first size_to items
		for(it_from=from.begin(), it_to=to.begin(); it_to!=to.end(); ++it_from, ++it_to) {
			it_to->T::load(*it_from);
		}

		// Then attach copies of the remaining items
		for(it_from=from.begin()+size_to; it_from!=from.end(); ++it_from) {
			to.push_back(T(*it_from));
		}
	} else if(size_from < size_to) {
		// First copy the initial size_for items over
		for(it_from=from.begin(), it_to=to.begin(); it_from!=from.end(); ++it_from, ++it_to) {
			it_to->T::load(*it_from);
		}

		// Then resize the local vector. Surplus items will vanish
		to.resize(size_from);
	}
}

/******************************************************************************/
/**
 * This function takes two arrays and copies their contents. It assumes that
 * uninitialized arrays point to nullptr and that the number of entries given
 * is exact. The from-array may be empty, in which case the to array will also
 * be empty after the call to this function. The function assumes that for
 * this operation, T::operator= makes sense. The function may modify all of its
 * "to"-arguments
 */
template<typename T>
void copyArrays(
	T const *const from
	, T *&to
	, const std::size_t &nFrom
	, std::size_t &nTo
) {
	//--------------------------------------------------------------------------
	// Do some error checks
	if (nullptr == from && 0 != nFrom) {
		glogger
		<< "In copyArrays(): Error: from-array is empty, but nFrom isn\'t:" << nFrom << std::endl
		<< GEXCEPTION;
	}

	if (nullptr != from && 0 == nFrom) {
		glogger
		<< "In copyArrays(): Error: from-array isn't empty, but nFrom is:" << std::endl
		<< GEXCEPTION;
	}

	if (nullptr == to && 0 != nTo) {
		glogger
		<< "In copyArrays(): Error: to-array is empty, but nTo isn\'t:" << nTo << std::endl
		<< GEXCEPTION;
	}

	if (nullptr != to && 0 == nTo) {
		glogger
		<< "In copyArrays(): Error: to-array isn't empty, but nTo is" << std::endl
		<< GEXCEPTION;
	}

	//--------------------------------------------------------------------------

	// If from is empty, make sure all other arguments are empty
	if (nullptr == from) {
		nTo = 0;
		if (to) {
			g_array_delete(to);
		}

		return;
	}

	// From here in we assume that nFrom contains entries

	// Make sure from and to have the same size. If not, adapt "to" accordingly
	if (nFrom != nTo) {
		if (to) {
			g_array_delete(to);
		}
		to = new T[nFrom];
		nTo = nFrom;
	}

	// Copy all elements over
	for (std::size_t i = 0; i < nFrom; i++) {
		to[i] = from[i];
	}
}

/******************************************************************************/
/**
 * This function takes two arrays of std::shared_ptr smart pointers and copies
 * one into the other. We want to make a deep copy of the smart pointers' contents.
 * Note that we assume here that the objects pointed to can be copied using an
 * operator=(). The function also assumes the existence of a valid copy constructor.
 * Note that this function might yield bad results for virtual types, when handled
 * through a base class.
 *
 * @param from The array used as the source of the copying
 * @param to The array used as the target of the copying
 * @param size_from The number of entries in the first array
 * @param size_to The number of entries in the second array before and after copying (will be modified)
 */
template<typename T>
void copySmartPointerArrays(
	std::shared_ptr <T> const *const from, std::shared_ptr <T> *&to, const std::size_t &size_from, std::size_t &size_to
) {
	//--------------------------------------------------------------------------
	// Do some error checks
	if (nullptr == from && 0 != size_from) {
		glogger
		<< "In copySmartPointerArrays(): Error: from-array is empty, but size_from isn\'t:" << size_from << std::endl
		<< GEXCEPTION;
	}

	if (nullptr != from && 0 == size_from) {
		glogger
		<< "In copySmartPointerArrays(): Error: from-array isn't empty, but size_from is:" << std::endl
		<< GEXCEPTION;
	}

	if (nullptr == to && 0 != size_to) {
		glogger
		<< "In copySmartPointerArrays(): Error: to-array is empty, but size_to isn\'t:" << size_to << std::endl
		<< GEXCEPTION;
	}

	if (nullptr != to && 0 == size_to) {
		glogger
		<< "In copySmartPointerArrays(): Error: to-array isn't empty, but size_to is" << std::endl
		<< GEXCEPTION;
	}

	//--------------------------------------------------------------------------
	// From here on we assume that from and to have valid content

	if (size_from != size_to) { // Get rid of all content in "to"
		for (std::size_t i = 0; i < size_to; i++) {
			to[i].reset();
		}
		g_array_delete(to);
		to = new std::shared_ptr <T>[size_from];
		size_to = size_from;
	}

	for (std::size_t i = 0; i < size_to; i++) {
		to[i] = std::shared_ptr<T>(new T(*(from[i])));
	}
}

/******************************************************************************/
/**
 * This function converts a smart pointer to a target type, throwing an exception
 * if the conversion cannot be done.
 */
template<typename source_type, typename target_type>
std::shared_ptr <target_type> convertSmartPointer(std::shared_ptr <source_type> p_raw) {
#ifdef DEBUG
      // Check that we have indeed been given an item and that the pointer isn't empty
      if(!p_raw) {
         glogger
         << "In std::shared_ptr<target_type> convertSmartPointer(std::shared_ptr<source_type> p_raw) :" << std::endl
         << "Error: Pointer is empty." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return std::shared_ptr<target_type>();
      }

      // Do the actual conversion
      std::shared_ptr<target_type> p = std::dynamic_pointer_cast<target_type>(p_raw);
      if(p) return p;
      else {
         glogger
         << "In std::shared_ptr<target_type> convertSmartPointer(std::shared_ptr<source_type> p_raw) :" << std::endl
         << "Error: Invalid conversion to type " << typeid(target_type).name() << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return std::shared_ptr<target_type>();
      }
#else
	return std::static_pointer_cast<target_type>(p_raw);
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * This function converts a simple pointer to a target type, throwing an exception
 * if the conversion cannot be done.
 */
template<typename source_type, typename target_type>
target_type *convertSimplePointer(source_type *p_raw) {
#ifdef DEBUG
      // Check that we have indeed been given an item and that the pointer isn't empty
      if(!p_raw) {
         glogger
         << "In target_type * convertSimplePointer(source_type *p_raw) :" << std::endl
         << "Error: Pointer is empty." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return nullptr;
      }

      // Do the actual conversion
      target_type  *p = dynamic_cast<target_type>(p_raw);
      if(p) return p;
      else {
         glogger
         << "In target_type * convertSimplePointer(source_type * p_raw) :" << std::endl
         << "Error: Invalid conversion to type " << typeid(target_type).name() << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return nullptr;
      }
#else
	return static_cast<target_type>(p_raw);
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * This function converts a simple pointer to a target type, throwing an exception
 * if the conversion cannot be done.
 */
template<typename source_type, typename target_type>
const target_type *convertSimplePointer(const source_type *p_raw) {
#ifdef DEBUG
      // Check that we have indeed been given an item and that the pointer isn't empty
      if(!p_raw) {
         glogger
         << "In const target_type * convertSimplePointer(const source_type *p_raw) :" << std::endl
         << "Error: Pointer is empty." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return nullptr;
      }

      // Do the actual conversion
      const target_type  *p = dynamic_cast<const target_type *>(p_raw);
      if(p) return p;
      else {
         glogger
         << "In target_type * convertSimplePointer(source_type * p_raw) :" << std::endl
         << "Error: Invalid conversion to type " << typeid(target_type).name() << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return nullptr;
      }
#else
	return static_cast<const target_type *>(p_raw);
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * Splits a string into a vector of user-defined types, according to a seperator character.
 * The only precondition is that the target type is known to boost::lexical_cast, which can
 * be achieved simply by providing related operator<< and operator>> .
 */
template<typename split_type>
std::vector<split_type> splitStringT(const std::string &raw, const char *sep) {
	std::vector<std::string> fragments = Gem::Common::splitString(raw, sep);
	std::vector<split_type> result;
	std::vector<std::string>::iterator it;
	for (it = fragments.begin(); it != fragments.end(); ++it) {
		result.push_back(boost::lexical_cast<split_type>(*it));
	}
	return result;
}

/******************************************************************************/
/**
 * Splits a string into a vector of user-defined type-pairs, according to seperator characters.
 * The only precondition is that the target types are known to boost::lexical_cast, which can
 * be achieved simply by providing related operator<< and operator>> . A possible usage is a
 * split of a string "0/0 0/1 1/0" into tuples of integers.
 */
template<typename split_type1, typename split_type2>
std::vector<std::tuple<split_type1, split_type2>> splitStringT(
	const std::string &raw, const char *sep1, const char *sep2
) {
	// Check that sep1 and sep2 differ
	if (std::string(sep1) == std::string(sep2)) {
		glogger
		<< "In splitStringT(std::string, const char*, const char*): Error!" << std::endl
		<< "sep1 and sep2 are identical: \"" << sep1 << "\" / \"" << sep2 << "\"" << std::endl
		<< GEXCEPTION;
	}

	std::vector<std::string> fragments = Gem::Common::splitString(raw, sep1);
	std::vector<std::tuple<split_type1, split_type2>> result;
	std::vector<std::string>::iterator it;
	for (it = fragments.begin(); it != fragments.end(); ++it) {
		std::vector<std::string> sub_fragments = Gem::Common::splitString(*it, sep2);

#ifdef DEBUG
      if(2 != sub_fragments.size()) {
         glogger
         << "In splitStringT(std::string, const char*, const char*): Error!" << std::endl
         << "Incorrect number of sub-fragments: " << sub_fragments.size()
         << GEXCEPTION;
      }
#endif /* DEBUG */

		result.push_back(
			std::tuple<split_type1, split_type2>(
				boost::lexical_cast<split_type1>(sub_fragments[0]), boost::lexical_cast<split_type2>(sub_fragments[1])
			)
		);
	}

	return result;
}

/******************************************************************************/
/**
 * Retrieves an item from a std::map and throws, if the corresponding key
 * isn't found
 */
template<typename item_type>
item_type &getMapItem(std::map<std::string, item_type> &m, const std::string &key) {
	if (m.empty()) {
		glogger
		<< "In item_type& getMapItem(std::map<std::string, item_type>& m, const std::string& key): Error!" << std::endl
		<< "Map is empty" << std::endl
		<< GEXCEPTION;
	}

	typename std::map<std::string, item_type>::iterator it = m.find(key);
	if (it != m.end()) {
		return it->second;
	} else {
		glogger
		<< "In \"item_type& getMapItem(std::map<std::string, item_type>& m, const std::string& key)\": Error!" <<
		std::endl
		<< "key " << key << " is not in the map." << std::endl
		<< GEXCEPTION;
	}

	// Make the compiler happy
	return m.begin()->second;
}

/******************************************************************************/
/**
 * Retrieves an item from a std::map and throws, if the corresponding key
 * isn't found
 */
template<typename item_type>
const item_type &getMapItem(const std::map<std::string, item_type> &m, const std::string &key) {
	if (m.empty()) {
		glogger
		<< "In const item_type& getMapItem(const std::map<std::string, item_type>& m, const std::string& key): Error!" <<
		std::endl
		<< "Map is empty" << std::endl
		<< GEXCEPTION;
	}

	typename std::map<std::string, item_type>::const_iterator cit = m.find(key);
	if (cit != m.end()) {
		return cit->second;
	} else {
		glogger
		<<
		"In \"const item_type& getMapItem(const std::map<std::string, item_type>& m, const std::string& key)\": Error!" <<
		std::endl
		<< "key " << key << " is not in the map." << std::endl
		<< GEXCEPTION;
	}

	return m.begin()->second;
}


/******************************************************************************/
/**
 * Checks whether start- and end-ids match a given container type. "start"
 * is inclusive, "end" is exclusive.
 */
template<typename container_type>
void assert_sizes_match_container(
	const container_type& container
	, std::size_t start
	, std::size_t end
	, const std::string& caller
){
	if (end <= start) {
		glogger
			<< "In assert_sizes_match_container() (caller " << caller << "): Error!" << std::endl
			<< "Invalid start or end-values: " << start << " / " << end << std::endl
			<< GEXCEPTION;
	}

	if (end > container.size()) {
		glogger
			<< "In assert_sizes_match_container() (caller " << caller << "): Error!" << std::endl
			<< "Last id " << end << " exceeds size of vector " << container.size() << std::endl
			<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Checks that the sizes of two container types match
 */
template<typename container_type1, typename container_type2>
void assert_container_sizes_match(
	const container_type1& container1
	, const container_type2& container2
	, const std::string& caller
) {
	if(container1.size() != container2.size()) {
		glogger
			<< "In assert_container_sizes_match() (caller " << caller << "): Error!" << std::endl
			<< "Invalid container sizes: " << container1.size() << " / " << container2.size() << std::endl
			<< GEXCEPTION;
	}
};

/******************************************************************************/
/**
 * Erases items from a standard container that comply with a specific condition
 */
template<typename container_type, typename predicate_type >
void erase_if(
	container_type& container
	, const predicate_type& predicate
) {
	for(auto it = container.begin(); it != container.end();) {
		if(predicate(*it)) {
			it = container.erase(it);
		}
		else {
			++it;
		}
	}
};

/******************************************************************************/
/**
 * Erases items from a standard container according to a collection of flags
 * in a std::vector of the same size. Erasure may happen in a given range only.
 * The items held by the container must be copyable. A flag equal to "flag" means
 * that the associated container entry will be erased.
 */
template<typename container_type>
void erase_according_to_flags(
	container_type& container
	, const std::vector<bool>& flags
	, bool flag
	, std::size_t start
	, std::size_t end
) {
	typename container_type::iterator item_it;
	std::vector<bool>::const_iterator pos_it;

	// Make sure the start/stop positions match the container
	assert_sizes_match_container(container, start, end, "erase_according_to_flags");

	// Make sure the flag vector has the same size as the container
	assert_container_sizes_match(container, flags, "erase_according_to_flags");

	// Copy items over that do not need to be erased
	container_type container_tmp;
	for (
		item_it = container.begin() + start, pos_it = flags.begin() + start;
		item_it != container.begin() + end; ++item_it, ++pos_it
	) {
		// Attach processed items to the tmp vector
		if (flag != *pos_it) {
			container_tmp.push_back(*item_it);
		}
	}

	// Remove all other items in the range [start:end[
	container.erase(container.begin() + start, container.begin() + end);

	// Insert the items from the tmp vector in position "start"
	container.insert(container.begin() + start, container_tmp.begin(), container_tmp.end());
}

/******************************************************************************/
/**
 * Forces submission to a boost::lockfree queue (either queue or spsc_queue)
 *
 * TODO: use enable_if and is_same to make this bullet-proof
 */
template <typename item_type, template <typename, typename...> class queue_type, typename... Options>
// Template syntax loosely follows http://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
bool forcedSubmissionToBoostLockfree(
	queue_type<item_type, Options...>& queue
	, item_type item
	, const std::chrono::duration<double> &sleepTime = std::chrono::duration<double>(std::chrono::milliseconds(1))
) {
	while(!queue.push(item)){
		std::this_thread::sleep_for(sleepTime);
	}

	return true;
}

/******************************************************************************/
/**
 * Submits an item to a boost::lockfree queue (either queue or spsc_queue),
 * observing a timeout.
 */
template <typename item_type, template <typename, typename...> class queue_type, typename... Options>
// Template syntax loosely follows http://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
bool timedSubmissionToBoostLockfree(
	queue_type<item_type, Options...>& queue
	, item_type item
	, const std::chrono::duration<double> &timeout
	, const std::chrono::duration<double> &sleepTime = std::chrono::duration<double>(std::chrono::milliseconds(1))
) {
	bool submitted = true;
	auto startTime = std::chrono::system_clock::now();
	while(!queue.push(item)) {
		if(std::chrono::system_clock::now()-startTime > timeout) {
			submitted = false;
			break; // Terminate the loop
		}

		std::this_thread::sleep_for(sleepTime);
	}
	return submitted;
}

/******************************************************************************/
/**
 * Forces retrieval from a boost::lockfree queue (either queue or spsc_queue)
 */
template <typename item_type, template <typename, typename...> class queue_type, typename... Options>
// Template syntax loosely follows http://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
bool forcedRetrievalFromBoostLockfree(
	queue_type<item_type, Options...>& queue
	, item_type& item
	, const std::chrono::duration<double> &sleepTime = std::chrono::duration<double>(std::chrono::milliseconds(1))
) {
	while(!queue.pop(item)){
		std::this_thread::sleep_for(sleepTime);
	}

	return true;
}

/******************************************************************************/
/**
 * Retrieves an item from a boost::lockfree queue (either queue or spsc_queue),
 * observing a timeout
 */
template <typename item_type, template <typename, typename...> class queue_type, typename... Options>
// Template syntax loosely follows http://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
bool timedRetrievalFromBoostLockfree(
	queue_type<item_type, Options...>& queue
	, item_type& item
	, const std::chrono::duration<double> &timeout
	, const std::chrono::duration<double> &sleepTime = std::chrono::duration<double>(std::chrono::milliseconds(1))
) {
	bool retrieved = true;
	auto startTime = std::chrono::system_clock::now();
	while(!queue.pop(item)) {
		if(std::chrono::system_clock::now()-startTime > timeout) {
			retrieved = false;
			break; // Terminate the loop
		}

		std::this_thread::sleep_for(sleepTime);
	}
	return retrieved;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GCOMMONHELPERFUNCTIONST_HPP_ */
