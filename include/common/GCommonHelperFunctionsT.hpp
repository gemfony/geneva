/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <map>
#include <optional>
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
#include <mutex>
#include <chrono>
#include <type_traits>
#include <memory>

// Boost headers go here
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/math/special_functions/next.hpp>
#include <boost/checked_delete.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Reads a given environment variable and converts it to a target type. The
 * function assumes that a suitable boost::lexical_cast exists for this type.
 *
 * @param var The name of the environment variable to be read
 * @return The converted environment variable, or an empty optional
 */
template <typename target_type>
std::optional<target_type> environmentVariableAs(std::string const& var) {
	std::string result_str;

	{
		// std::getenv is not thread-safe; serialise access with a local mutex.
		static std::mutex read_env_mutex;
		std::unique_lock<std::mutex> lk(read_env_mutex);

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
		char* env_ptr = 0;
		size_t sz = 0;
		if (0 == _dupenv_s(&env_ptr, &sz, var.c_str()) && nullptr != env_ptr) {
			result_str = std::string(env_ptr);
			free(env_ptr);
		} else {
			return {};
		}
#else
		const char *env_ptr = std::getenv(var.c_str());
		if (env_ptr) {
			result_str = std::string(env_ptr);
		} else {
			return {};
		}
#endif
	} // releases the lock

	boost::trim(result_str);
	return { boost::lexical_cast<target_type>(result_str) };
}

/******************************************************************************/
/**
 * Null-safe delete: uses boost::checked_delete (compile-time check for
 * complete type), then sets the pointer to nullptr.
 */
template <typename T>
void g_delete(T *&p) {
	if(p) {
		boost::checked_delete(p);
		p = nullptr;
	}
}

/******************************************************************************/
/**
 * Null-safe array delete: uses boost::checked_array_delete, then sets the
 * pointer to nullptr.
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
 * In debug builds, throws if two raw pointers alias the same object.
 * No-op for nullptr p1.
 */
template <typename T>
void ptrDifferenceCheck(const T *p1, const T *p2) {
#ifdef DEBUG
	if (nullptr != p1 && p1 == p2) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In Gem::Common::ptrDifferenceCheck<T>(): "
				<< "p1 and p2 point to the same object!" << std::endl
		);
	}
#endif
}

/******************************************************************************/
/**
 * Shared-pointer overload: in debug builds, throws if both non-null shared
 * pointers alias the same object.
 */
template <typename T>
void ptrDifferenceCheck(std::shared_ptr<T> p1, std::shared_ptr<T> p2) {
#ifdef DEBUG
	if (p1 && p1.get() == p2.get()) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In Gem::Common::ptrDifferenceCheck<T>(): "
				<< "Smart pointers p1 and p2 point to the same object!" << std::endl
		);
	}
#endif
}

/******************************************************************************/
/**
 * Converts a raw base pointer to target_type*. Only accessible when
 * base_type is a base of target_type (upcasts only). Returns nullptr
 * unchanged. In debug builds uses dynamic_cast and throws on failure;
 * in release builds uses static_cast.
 */
template <typename base_type, typename target_type>
const target_type * g_ptr_conversion(
	const base_type *convert_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
#ifdef DEBUG
	const auto *p = dynamic_cast<const target_type *>(convert_ptr);
	if (nullptr == convert_ptr || p) {
		return p;
	}
	throw geneva_exception(
		g_error_streamer(DO_LOG, time_and_place)
			<< "In g_ptr_conversion(): invalid conversion from "
			<< typeid(base_type).name() << " to " << typeid(target_type).name() << std::endl
	);
	return nullptr;
#else
	return static_cast<const target_type *>(convert_ptr);
#endif
}

/******************************************************************************/
/**
 * Shared-pointer overload of g_ptr_conversion.
 */
template <typename base_type, typename target_type>
std::shared_ptr<target_type> g_ptr_conversion(
	std::shared_ptr<base_type> convert_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
#ifdef DEBUG
	auto p = std::dynamic_pointer_cast<target_type>(convert_ptr);
	if (nullptr == convert_ptr.get() || p) {
		return p;
	}
	throw geneva_exception(
		g_error_streamer(DO_LOG, time_and_place)
			<< "In g_ptr_conversion(): invalid conversion from "
			<< typeid(base_type).name() << " to " << typeid(target_type).name() << std::endl
	);
#else
	return std::static_pointer_cast<target_type>(convert_ptr);
#endif
}

/******************************************************************************/
/**
 * Converts convert_ptr to target_type and checks it does not alias
 * compare_ptr. Only accessible when base_type is a base of target_type.
 */
template <typename base_type, typename target_type>
std::shared_ptr<target_type> g_convert_and_compare(
	std::shared_ptr<base_type> convert_ptr
	, std::shared_ptr<target_type> compare_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
	auto p = g_ptr_conversion<base_type, target_type>(convert_ptr);
	ptrDifferenceCheck(p, compare_ptr);
	return p;
}

/******************************************************************************/
/**
 * Raw-pointer overload of g_convert_and_compare.
 */
template <typename base_type, typename target_type>
const target_type* g_convert_and_compare(
	const base_type *convert_ptr
	, const target_type *compare_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
	const target_type *p = g_ptr_conversion<base_type, target_type>(convert_ptr);
	ptrDifferenceCheck(p, compare_ptr);
	return p;
}

/******************************************************************************/
/**
 * Reference overload of g_convert_and_compare.
 */
template <typename base_type, typename target_type>
const target_type* g_convert_and_compare(
	const base_type& convert_ref
	, const target_type *compare_ptr
	, typename std::enable_if<std::is_base_of<base_type, target_type>::value>::type *dummy = nullptr
) {
	const auto *p = g_ptr_conversion<base_type, target_type>(&convert_ref);
	ptrDifferenceCheck(p, compare_ptr);
	return p;
}

/******************************************************************************/
/**
 * Returns a space-separated string representation of a std::vector.
 * T must be streamable.
 */
template<typename T>
std::string vecToString(const std::vector<T> &vec) {
	std::ostringstream result;
	for (const auto& item : vec) {
		result << item << " ";
	}
	return result.str();
}

/******************************************************************************/
/**
 * Deep-copies a shared_ptr to a cloneable/loadable object using clone()/load().
 */
template <typename T>
void copyCloneableSmartPointer(
	const std::shared_ptr<T>& from
	, std::shared_ptr<T>& to
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<T>::value>::type *dummy = nullptr
) {
	if (not from) {
		to.reset();
	} else if (not to) {
		to = from->T::template clone<T>();
	} else {
		to->T::load(from);
	}
}

/******************************************************************************/
/**
 * Deep-copies a container of shared_ptrs to cloneable objects using
 * clone()/load(). Resizes the target container as needed.
 */
template <typename T, template <typename, typename> class c_type>
void copyCloneableSmartPointerContainer(
	const c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>& from
	, c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>& to
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<T>::value>::type *dummy = nullptr
) {
	using iter_t       = typename c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>::iterator;
	using const_iter_t = typename c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>::const_iterator;

	const std::size_t size_from = from.size();
	const std::size_t size_to   = to.size();

	if (size_from == size_to) {
		const_iter_t it_from = from.begin();
		for (iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
			copyCloneableSmartPointer(*it_from, *it_to);
		}
	} else if (size_from > size_to) {
		const_iter_t it_from = from.begin();
		for (iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
			copyCloneableSmartPointer(*it_from, *it_to);
		}
		for (const_iter_t it = from.begin() + size_to; it != from.end(); ++it) {
			to.push_back((*it)->T::template clone<T>());
		}
	} else { // size_from < size_to
		const_iter_t it_from = from.begin();
		for (iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
			copyCloneableSmartPointer(*it_from, *it_to);
		}
		to.resize(size_from);
	}
}

/******************************************************************************/
/**
 * Deep-copies a container of cloneable objects using load(). Resizes the
 * target container as needed.
 */
template <typename T, template <typename, typename> class c_type>
void copyCloneableObjectsContainer(
	const c_type<T, std::allocator<T>>& from
	, c_type<T, std::allocator<T>>& to
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<T>::value>::type *dummy = nullptr
) {
	using iter_t       = typename c_type<T, std::allocator<T>>::iterator;
	using const_iter_t = typename c_type<T, std::allocator<T>>::const_iterator;

	const std::size_t size_from = from.size();
	const std::size_t size_to   = to.size();

	if (size_from == size_to) {
		const_iter_t it_from = from.begin();
		for (iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
			it_to->T::load(*it_from);
		}
	} else if (size_from > size_to) {
		const_iter_t it_from = from.begin();
		for (iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
			it_to->T::load(*it_from);
		}
		for (const_iter_t it = from.begin() + size_to; it != from.end(); ++it) {
			to.push_back(T(*it));
		}
	} else { // size_from < size_to
		const_iter_t it_from = from.begin();
		for (iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
			it_to->T::load(*it_from);
		}
		to.resize(size_from);
	}
}

/******************************************************************************/
/**
 * Copies a raw array into another raw array, allocating or reallocating the
 * destination as needed. Both size parameters are kept consistent.
 */
template<typename T>
void copyArrays(
	T const *const from
	, T *&to
	, const std::size_t &nFrom
	, std::size_t &nTo
) {
	if (nullptr == from && 0 != nFrom) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copyArrays(): from is null but nFrom=" << nFrom << std::endl
		);
	}
	if (nullptr != from && 0 == nFrom) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copyArrays(): from is non-null but nFrom=0" << std::endl
		);
	}
	if (nullptr == to && 0 != nTo) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copyArrays(): to is null but nTo=" << nTo << std::endl
		);
	}
	if (nullptr != to && 0 == nTo) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copyArrays(): to is non-null but nTo=0" << std::endl
		);
	}

	if (nullptr == from) {
		nTo = 0;
		if (to) { g_array_delete(to); }
		return;
	}

	if (nFrom != nTo) {
		if (to) { g_array_delete(to); }
		to = new T[nFrom];
		nTo = nFrom;
	}

	for (std::size_t i = 0; i < nFrom; i++) {
		to[i] = from[i];
	}
}

/******************************************************************************/
/**
 * Deep-copies a raw array of shared_ptrs into another, allocating or
 * reallocating the destination as needed.
 */
template<typename T>
void copySmartPointerArrays(
	std::shared_ptr<T> const *const from
	, std::shared_ptr<T> *&to
	, const std::size_t &size_from
	, std::size_t &size_to
) {
	if (nullptr == from && 0 != size_from) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copySmartPointerArrays(): from is null but size_from=" << size_from << std::endl
		);
	}
	if (nullptr != from && 0 == size_from) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copySmartPointerArrays(): from is non-null but size_from=0" << std::endl
		);
	}
	if (nullptr == to && 0 != size_to) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copySmartPointerArrays(): to is null but size_to=" << size_to << std::endl
		);
	}
	if (nullptr != to && 0 == size_to) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In copySmartPointerArrays(): to is non-null but size_to=0" << std::endl
		);
	}

	if (size_from != size_to) {
		for (std::size_t i = 0; i < size_to; i++) { to[i].reset(); }
		g_array_delete(to);
		to = new std::shared_ptr<T>[size_from];
		size_to = size_from;
	}

	for (std::size_t i = 0; i < size_to; i++) {
		to[i] = std::make_shared<T>(*(from[i]));
	}
}

/******************************************************************************/
/**
 * Converts a shared_ptr to target_type. In debug builds uses dynamic_pointer_cast
 * and throws on failure or null input; in release builds uses static_pointer_cast.
 */
template<typename source_type, typename target_type>
std::shared_ptr<target_type> convertSmartPointer(std::shared_ptr<source_type> p_raw) {
#ifdef DEBUG
	if (not p_raw) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In convertSmartPointer(): pointer is empty." << std::endl
		);
	}
	auto p = std::dynamic_pointer_cast<target_type>(p_raw);
	if (p) return p;
	throw geneva_exception(
		g_error_streamer(DO_LOG, time_and_place)
			<< "In convertSmartPointer(): invalid conversion to "
			<< typeid(target_type).name() << std::endl
	);
#else
	return std::static_pointer_cast<target_type>(p_raw);
#endif
}

/******************************************************************************/
/**
 * Splits a string into a vector of target_type values using a single separator.
 * target_type must be known to boost::lexical_cast.
 */
template<typename split_type>
std::vector<split_type> splitStringT(const std::string &raw, const char *sep) {
	std::vector<split_type> result;
	for (const auto& fragment : Gem::Common::splitString(raw, sep)) {
		result.push_back(boost::lexical_cast<split_type>(fragment));
	}
	return result;
}

/******************************************************************************/
/**
 * Splits a string into a vector of (split_type1, split_type2) pairs using
 * two different separators. A possible usage: "0/0 0/1 1/0" → tuples of ints.
 */
template<typename split_type1, typename split_type2>
std::vector<std::tuple<split_type1, split_type2>> splitStringT(
	const std::string &raw, const char *sep1, const char *sep2
) {
	if (std::string(sep1) == std::string(sep2)) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In splitStringT(): sep1 and sep2 are identical: \""
				<< sep1 << "\" / \"" << sep2 << "\"" << std::endl
		);
	}

	std::vector<std::tuple<split_type1, split_type2>> result;
	for (const auto& fragment : Gem::Common::splitString(raw, sep1)) {
		const auto sub = Gem::Common::splitString(fragment, sep2);
#ifdef DEBUG
		if (2 != sub.size()) {
			throw geneva_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In splitStringT(): expected 2 sub-fragments, got "
					<< sub.size() << std::endl
			);
		}
#endif
		result.emplace_back(
			boost::lexical_cast<split_type1>(sub[0])
			, boost::lexical_cast<split_type2>(sub[1])
		);
	}
	return result;
}

/******************************************************************************/
/**
 * Returns a reference to the value at key in m; throws if the map is empty
 * or the key is absent.
 */
template<typename item_type>
item_type &getMapItem(std::map<std::string, item_type> &m, const std::string &key) {
	if (m.empty()) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In getMapItem(): map is empty" << std::endl
		);
	}
	auto it = m.find(key);
	if (it != m.end()) return it->second;
	throw geneva_exception(
		g_error_streamer(DO_LOG, time_and_place)
			<< "In getMapItem(): key \"" << key << "\" not found" << std::endl
	);
}

/******************************************************************************/
/**
 * Const overload of getMapItem.
 */
template<typename item_type>
const item_type &getMapItem(const std::map<std::string, item_type> &m, const std::string &key) {
	if (m.empty()) {
		throw geneva_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In getMapItem(): map is empty" << std::endl
		);
	}
	auto cit = m.find(key);
	if (cit != m.end()) return cit->second;
	throw geneva_exception(
		g_error_streamer(DO_LOG, time_and_place)
			<< "In getMapItem(): key \"" << key << "\" not found" << std::endl
	);
}

/******************************************************************************/
/**
 * Adds an operator== to every object with a Gemfony-common interface
 */
template<
	class gemfony_common_type
	, class = typename std::enable_if<Gem::Common::has_gemfony_common_interface<gemfony_common_type>::value>::type
>
bool operator==(
	const gemfony_common_type& x
	, const gemfony_common_type& y
) {
	try {
		x.compare(y, Gem::Common::expectation::EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Adds an operator!= to every object with a Gemfony-common interface
 */
template<
	class gemfony_common_type
	, class = typename std::enable_if<Gem::Common::has_gemfony_common_interface<gemfony_common_type>::value>::type
>
bool operator!=(
	const gemfony_common_type& x
	, const gemfony_common_type& y
) {
	try {
		x.compare(y, Gem::Common::expectation::INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Converts integral types (except scoped enums) to std::string.
 */
template <typename integral_type>
std::string to_string(
	integral_type val
	, typename std::enable_if<
		std::is_integral<integral_type>::value
		|| (std::is_enum<integral_type>::value && std::is_convertible<integral_type, int>::value)
	>::type* = 0
) {
	return std::to_string(val);
}

/******************************************************************************/
/**
 * Converts floating-point values to std::string. Uses boost::lexical_cast to
 * avoid locale-dependent decimal separators (. vs ,).
 */
template <typename fp_type>
std::string to_string(
	fp_type val
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type* = 0
) {
	return boost::lexical_cast<std::string>(val);
}

/******************************************************************************/
/**
 * Converts a scoped enum (enum class) to std::string via uint32_t cast.
 */
template <typename enum_type>
std::string to_string(
	enum_type val
	, typename std::enable_if<
		std::is_enum<enum_type>::value && not std::is_convertible<enum_type, int>::value
	>::type* = 0
) {
	return std::to_string(static_cast<std::uint32_t>(val));
}

/******************************************************************************/
/**
 * Converts any remaining streamable type to std::string via boost::lexical_cast.
 */
template <typename default_type>
std::string to_string(
	default_type val
	, typename std::enable_if<
		not std::is_enum<default_type>::value && not std::is_arithmetic<default_type>::value
	>::type* = 0
) {
	return boost::lexical_cast<std::string>(val);
}

/******************************************************************************/
/**
 * Erases elements from a standard container matching a predicate. Equivalent
 * to C++20 std::erase_if, kept here for CUDA nvcc compatibility (nvcc does not
 * expose the C++20 standard-library additions). Returns the number of erased
 * elements.
 */
template<typename container_type, typename predicate_type>
std::size_t erase_if(container_type& container, const predicate_type& predicate) {
	std::size_t n_erased = 0;
	for (auto it = container.begin(); it != container.end();) {
		if (predicate(*it)) {
			it = container.erase(it);
			++n_erased;
		} else {
			++it;
		}
	}
	return n_erased;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
