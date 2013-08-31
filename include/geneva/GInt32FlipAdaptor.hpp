/**
 * @file GInt32FlipAdaptor.hpp
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


// Standard header files go here

// Boost header files go here

#ifndef GINT32FLIPADAPTOR_HPP_
#define GINT32FLIPADAPTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

#include "GIntFlipAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This adaptor increases or decreases a value by 1
 */
class GInt32FlipAdaptor
	:public GIntFlipAdaptorT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GIntFlipAdaptorT_int32", boost::serialization::base_object<GIntFlipAdaptorT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GInt32FlipAdaptor();
	/** @brief The copy constructor */
	GInt32FlipAdaptor(const GInt32FlipAdaptor&);
	/** @brief Initialization with a adaption probability */
	explicit GInt32FlipAdaptor(const double&);

	/** @brief The destructor */
	virtual ~GInt32FlipAdaptor();

	/** @brief A standard assignment operator */
	const GInt32FlipAdaptor& operator=(const GInt32FlipAdaptor&);

	/** @brief Checks for equality with another GInt32FlipAdaptor object */
	bool operator==(const GInt32FlipAdaptor&) const;
	/** @brief Checks for inequality with another GInt32FlipAdaptor object */
	bool operator!=(const GInt32FlipAdaptor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
	      const GObject&
	      , const Gem::Common::expectation&
	      , const double&
	      , const std::string&
	      , const std::string&
	      , const bool&
	) const OVERRIDE;

	/** @brief Retrieves the id of this adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GInt32FlipAdaptor)

#endif /* GINT32FLIPADAPTOR_HPP_ */
