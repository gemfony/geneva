/**
 * @file GCommonInterfaceT.hpp
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

// Standard header files go here
#include <string>
#include <memory>

// Boost header files go here

#ifndef GCOMMONINTERFACET_HPP_
#define GCOMMONINTERFACET_HPP_

// Geneva header files go here
#include "GCommonEnums.hpp" // For the serialization mode

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This is an interface class that specifies common operations that must be
 * available for the majority of classes in the Gemfony scientific library.
 * As one example, (de-)serialization is simplified by some of the functions
 * in this class, as is the task of conversion to the derived types. The
 * GCommonInterfaceT::load_(const T *) and  GCommonInterfaceT::clone_() member
 * functions must be re-implemented for each derived class. Further common
 * functionality of many classes in the Gemfony library collection will be
 * implemented here over time.
 */
template <typename T>
class GCommonInterfaceT {
public:
	/** @brief The default constructor */
	G_API_COMMON GCommonInterfaceT() { /* nothing */ }

	/** @brief The standard destructor */
	virtual G_API_COMMON ~GCommonInterfaceT() { /* nothing */ }

	/** @brief Create a text representation from this class */
	virtual G_API_COMMON std::string toString(const serializationMode &) const = 0;

	/** @brief Initialize this class from a text representation */
	virtual G_API_COMMON void fromString(const std::string &, const serializationMode &) = 0;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_COMMON void compare(
		const T& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const BASE = 0;

	/** @brief Creates a clone of this object, storing it in a std::shared_ptr<T> */
	std::shared_ptr<T> clone() const {
		return std::shared_ptr<T>(clone_());
	}

	/***************************************************************************/
	/**
	 * The function creates a clone of the T pointer, converts it to a pointer to a derived class
	 * and emits it as a std::shared_ptr<> . Note that this template will only be accessible to the
	 * compiler if T is a base type of clone_type.
	 *
	 * @return A converted clone of this object, wrapped into a std::shared_ptr
	 */
	template <typename clone_type>
	std::shared_ptr<clone_type> clone(
		typename boost::enable_if<boost::is_base_of<T, clone_type>>::type* dummy = 0
	) const {
		return Gem::Common::convertSmartPointer<T, clone_type>(std::shared_ptr<T>(this->clone_()));
	}

	/* ----------------------------------------------------------------------------------
	 * cloning is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loads the data of another T(-derivative), wrapped in a shared pointer. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of T.
	 *
	 * @param cp A copy of another T-derivative, wrapped into a std::shared_ptr<>
	 */
	template <typename load_type>
	inline void load(
		const std::shared_ptr<load_type>& cp
		, typename boost::enable_if<boost::is_base_of<T, load_type>>::type* dummy = 0
	) {
		load_(cp.get());
	}

	/* ----------------------------------------------------------------------------------
	 * loading is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loads the data of another T(-derivative), presented as a constant reference. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of T.
	 *
	 * @param cp A copy of another T-derivative, wrapped into a std::shared_ptr<>
	 */
	template <typename load_type>
	inline void load(
		const load_type& cp
		, typename boost::enable_if<boost::is_base_of<T, load_type>>::type* dummy = 0
	) {
		load_(&cp);
	}

	/* ----------------------------------------------------------------------------------
	 * loading is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual G_API_COMMON void load_(const T*) BASE = 0;
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON T* clone_() const BASE = 0;
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GCOMMONINTERFACET_HPP_ */
