/**
 * @file GNumCollectionT.hpp
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
#include <string>
#include <sstream>
#include <vector>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GNUMCOLLECTIONT_HPP_
#define GNUMCOLLECTIONT_HPP_

// GenEvA header files go here

#include "GObject.hpp"
#include "GParameterCollectionT.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/**********************************************************************/
/**
 * This class represents a collection of numeric values, all modified
 * using the same algorithm. The most likely types to be stored in this
 * class are double and boost::uint32_t . By using the framework provided
 * by GParameterCollectionT, this class becomes rather simple.
 */
template <typename num_type>
class GNumCollectionT
	: public GParameterCollectionT<num_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<num_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/******************************************************************/
	/**
	 * The default constructor.
	 */
	GNumCollectionT():
		GParameterCollectionT<num_type> ()
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * Initialize with a number of random values within given boundaries
	 *
	 * @param nval Number of values to put into the vector
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumCollectionT(const std::size_t& nval, const num_type& min, const num_type& max){
		this->addRandomData(nval, min, max);
	}

	/******************************************************************/
	/**
	 * The standard copy constructor
	 */
	GNumCollectionT(const GNumCollectionT& cp)
		:GParameterCollectionT<num_type> (cp)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GNumCollectionT()
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard assignment operator.
	 *
	 * @param cp A copy of another GDoubleCollection object
	 * @return A constant reference to this object
	 */
	const GNumCollectionT& operator=(const GNumCollectionT& cp){
		GNumCollectionT<num_type>::load(&cp);
		return *this;
	}

	/******************************************************************/
	/**
	 * Creates a deep copy of this object.
	 *
	 * @return A pointer to a deep clone of this object
	 */
	virtual GObject *clone(){
		return new GNumCollectionT<num_type>(*this);
	}

	/******************************************************************/
	/**
	 * Loads the data of another GNumCollectionT<num_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GNumCollectionT<num_type> object, camouflaged as a GObject
	 */
	virtual void load(const GObject *cp){
		// Check that this object is not accidently assigned to itself.
		// As we do not actually do any calls with this pointer, we
		// can use the faster static_cast<>
		if(static_cast<const GNumCollectionT<num_type> *>(cp) == this) {
			std::ostringstream error;

			error << "In GNumCollectionT<num_type>::load(): Error!" << std::endl
				  << "Tried to assign an object to itself." << std::endl;

			throw geneva_error_condition(error.str());
		}

		GParameterCollectionT<num_type>::load(cp);
	}

	/******************************************************************/
	/**
	 * In derived classes, this function appends nval random values
	 * between min and max to this class. This function is a trap to
	 * early detect improper usage of this class.
	 *
	 * @param nval Number of values to put into the vector
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	void addRandomData(const std::size_t& nval, const num_type& min, const num_type& max){
		std::ostringstream error;

		error << "In GNumCollectionT<num_type>::addRandomData(): Error!" << std::endl
			  << "This function should never have been called direclty." << std::endl;

		throw geneva_error_condition(error.str());
	}

	/******************************************************************/
};

/**********************************************************************/
// Declaration of some specializations
template<> void GNumCollectionT<double>::addRandomData(const std::size_t&, const double&, const double&);
template<> void GNumCollectionT<boost::uint32_t>::addRandomData(const std::size_t&, const boost::uint32_t&, const boost::uint32_t&);

/**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GNUMCOLLECTIONT_HPP_ */
