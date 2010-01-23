/**
 * @file GIntFlipAdaptorT.hpp
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


// Standard headers go here
#include <cmath>
#include <string>
#include <sstream>
#include <utility> // For std::pair

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/cast.hpp>

#ifndef GINTFLIPADAPTORT_HPP_
#define GINTFLIPADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GBoundedDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
/**
 * GIntFlipAdaptorT represents an adaptor used for the mutation of integer
 * types, by flipping an integer number to the next larger or smaller number.
 * The integer type used needs to be specified as a template parameter. Note
 * that a specialization of this class, as defined in GIntFlipAdaptorT.cpp,
 * allows to deal with booleans instead of "standard" integer types.
 */
template<typename T>
class GIntFlipAdaptorT:
	public GAdaptorT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor.
	 */
	GIntFlipAdaptorT()
		: GAdaptorT<T> (DEFAULTBITMUTPROB)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * This constructor takes an argument, that specifies the (initial) probability
	 * for the mutation of an integer or bit value
	 *
	 * @param prob The probability for a flip
	 */
	explicit GIntFlipAdaptorT(const double& prob)
		: GAdaptorT<T>(prob)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GIntFlipAdaptorT object
	 */
	GIntFlipAdaptorT(const GIntFlipAdaptorT<T>& cp)
		: GAdaptorT<T>(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GIntFlipAdaptorT()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GIntFlipAdaptorT objects,
	 *
	 * @param cp A copy of another GIntFlipAdaptorT object
	 */
	const GIntFlipAdaptorT<T>& operator=(const GIntFlipAdaptorT<T>& cp)
	{
		GIntFlipAdaptorT<T>::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * This function loads the data of another GIntFlipAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GIntFlipAdaptorT, camouflaged as a GObject
	 */
	void load(const GObject *cp)
	{
		// Convert GObject pointer to local format
		// (also checks for self-assignments in DEBUG mode)
		const GIntFlipAdaptorT<T> *p_load = this->conversion_cast(cp, this);

		// Load the data of our parent class ...
		GAdaptorT<T>::load(cp);

		// no local data
	}


	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object. The function is purely virtual, as we
	 * do not want this class to be instantiated. Use the derived classes instead.
	 *
	 * @return A deep copy of this object
	 */
	virtual GObject *clone() const = 0;

	/********************************************************************************************/
	/**
	 * Checks for equality with another GIntFlipAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIntFlipAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GIntFlipAdaptorT<T>& cp) const {
		return GIntFlipAdaptorT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GIntFlipAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIntFlipAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GIntFlipAdaptorT<T>& cp) const {
		return !GIntFlipAdaptorT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GIntFlipAdaptorT<T> object Equality means
	 * that all individual sub-values are equal and that the parent class is equal.
	 *
	 * @param  cp A constant reference to another GIntFlipAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GIntFlipAdaptorT reference
		const GIntFlipAdaptorT<T> *p_load = GObject::conversion_cast(&cp,  this);

		// Check our parent class
		if(!GAdaptorT<T>::isEqualTo(*p_load, expected)) return false;

		// no local data

		return true;
	}

	/********************************************************************************************/
	/**
	 * Checks for similarity with another GIntFlipAdaptorT<T> object. Similarity means
	 * that all double values are similar to each other within a given limit and that all other
	 * values are equal. Also, parent classes must be similar to each other.
	 *
	 * @param  cp A constant reference to another GIntFlipAdaptorT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GIntFlipAdaptorT reference
		const GIntFlipAdaptorT<T> *p_load = GObject::conversion_cast(&cp,  this);

		// First check our parent class
		if(!GAdaptorT<T>::isSimilarTo(*p_load, limit, expected))  return false;

		// no local data

		return true;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Purely virtual, as we do not want this class to be
	 * instantiated.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const = 0;

protected:
	/********************************************************************************************/
	/**
	 * We want to flip the value only in a given percentage of cases. Thus
	 * we calculate a probability between 0 and 1 and compare it with the desired
	 * mutation probability. Please note that evenRandom returns a value in the
	 * range of [0,1[, so we make a tiny error here. This function assumes
	 * an integer type. It hence flips the value up or down. A specialization
	 * for booleans is provided in GIntFlipAdaptorT.cpp .
	 *
	 * @param value The bit value to be mutated
	 */
	virtual void customMutations(T& value) {
		bool up = this->gr.boolRandom();
		if(up){
#if defined (CHECKOVERFLOWS)
			if(std::numeric_limits<T>::max() == value) {
#ifdef DEBUG
				std::cout << "Warning: Had to change mutation due to overflow in GIntFlipAdaptorT<>::customMutations()" << std::endl;
#endif
				value -= 1;
			}
			else value += 1;
#else
			value += 1;
#endif
		}
		else {
#if defined (CHECKOVERFLOWS)
			if(std::numeric_limits<T>::min() == value) {
#ifdef DEBUG
				std::cout << "Warning: Had to change mutation due to underflow in GIntFlipAdaptorT<>::customMutations()" << std::endl;
#endif
				value += 1;
			}
			else value -= 1;
#else
			value -= 1;
#endif
		}
	}
};

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GIntFlipAdaptorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GIntFlipAdaptorT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GINTFLIPADAPTORT_HPP_ */
