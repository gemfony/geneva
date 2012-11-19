/**
 * @file GDoubleGaussAdaptor.hpp
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
 * http://www.gemfony.com .
 */

// Standard headers go here

// Boost headers go here

#ifndef GDOUBLEGAUSSADAPTOR_HPP_
#define GDOUBLEGAUSSADAPTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GFPGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GDoubleGaussAdaptor represents an adaptor used for the adaption of
 * double values through the addition of gaussian-distributed random numbers.
 * See the documentation of GNumGaussAdaptorT<T> for further information on adaptors
 * in the Geneva context. This class is at the core of evolutionary strategies,
 * as implemented by this library. It is now implemented through a generic
 * base class that can also be used to adapt other numeric types.
 */
class GDoubleGaussAdaptor
	:public GFPGaussAdaptorT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GFPGaussAdaptorT_double", boost::serialization::base_object<GFPGaussAdaptorT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDoubleGaussAdaptor();
	/** @brief The copy constructor */
	GDoubleGaussAdaptor(const GDoubleGaussAdaptor&);
	/** @brief Initialization with a adaption probability */
	explicit GDoubleGaussAdaptor(const double&);
	/** @brief Initialization with a number of values belonging to the width of the gaussian */
	GDoubleGaussAdaptor(
			const double&
			, const double&
			, const double&
			, const double&
	);
	/** @brief Initialization with a number of values belonging to the width of the gaussian and the adaption probability */
	GDoubleGaussAdaptor(
			const double&
			, const double&
			, const double&
			, const double&
			, const double&
	);
	/** @brief The destructor */
	virtual ~GDoubleGaussAdaptor();

	/** @brief A standard assignment operator */
	const GDoubleGaussAdaptor& operator=(const GDoubleGaussAdaptor&);

	/** @brief Checks for equality with another GDoubleGaussAdaptor object */
	bool operator==(const GDoubleGaussAdaptor&) const;
	/** @brief Checks for inequality with another GDoubleGaussAdaptor object */
	bool operator!=(const GDoubleGaussAdaptor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Retrieves the id of this adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleGaussAdaptor)

#endif /* GDOUBLEGAUSSADAPTOR_HPP_ */
