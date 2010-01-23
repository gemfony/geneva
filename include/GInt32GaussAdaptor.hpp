/**
 * @file GInt32GaussAdaptor.hpp
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
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#ifndef GINT32GAUSSADAPTOR_HPP_
#define GINT32GAUSSADAPTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here

#include "GGaussAdaptorT.hpp"

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * The GInt32GaussAdaptor represents an adaptor used for the mutation of
 * boost::int32_t values through the addition of gaussian-distributed random numbers.
 * See the documentation of GAdaptorT<T> for further information on adaptors
 * in the GenEvA context. Most functionality is currently implemented in the
 * GGaussAdaptorT parent-class. Note that, for the purpose of mutating integer
 * values, it is generally not useful to choose very small sigma values. A value of
 * 1 might be a good choice. Similarly, the minSigma parameter should be set
 * accordingly, so sigma cannot get too small when being adapted.
 */
class GInt32GaussAdaptor
	:public GGaussAdaptorT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GGaussAdaptorT_int", boost::serialization::base_object<GGaussAdaptorT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GInt32GaussAdaptor();
	/** @brief The copy constructor */
	GInt32GaussAdaptor(const GInt32GaussAdaptor&);
	/** @brief Initialization with a mutation probability */
	explicit GInt32GaussAdaptor(const double&);
	/** @brief Initialization with a number of values belonging to the width of the gaussian */
	GInt32GaussAdaptor(const double&, const double&, const double&, const double&);
	/** @brief Initialization with a number of values belonging to the width of the gaussian and the mutation probability */
	GInt32GaussAdaptor(const double&, const double&, const double&, const double&, const double&);
	/** @brief The destructor */
	virtual ~GInt32GaussAdaptor();

	/** @brief A standard assignment operator */
	const GInt32GaussAdaptor& operator=(const GInt32GaussAdaptor&);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone() const;

	/** @brief Checks for equality with another GInt32GaussAdaptor object */
	bool operator==(const GInt32GaussAdaptor&) const;
	/** @brief Checks for inequality with another GInt32GaussAdaptor object */
	bool operator!=(const GInt32GaussAdaptor&) const;

	/** @brief Checks for equality with another GInt32GaussAdaptor object. */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GInt32GaussAdaptor object. */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Loads the data of another GObject */
	virtual void load(const GObject* cp);

	/** @brief Retrieves the id of this adaptor */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const;

protected:
	/** The actual mutation performed on the value type */
	virtual void customMutations(boost::int32_t&);
};


} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINT32GAUSSADAPTOR_HPP_ */
