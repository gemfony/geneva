/**
 * @file GRateableI.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

#ifndef GRATEABLEI_HPP_
#define GRATEABLEI_HPP_

// Geneva header files go here

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * A simple interface class for objects that can be evaluated.
 */
class GRateableI {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version){
	  using boost::serialization::make_nvp;
	  /* nothing - this is a base class */
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard destructor */
	virtual ~GRateableI(){ /* nothing */ }

	/** @brief Retrieve a value for this class */
	virtual double fitness() = 0;

	/** @brief Retrieve a value for this class and check for exceptions. Useful for threads */
	virtual double checkedFitness() = 0;
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GRateableI)

/**************************************************************************************************/

#endif /* GRATEABLEI_HPP_ */
