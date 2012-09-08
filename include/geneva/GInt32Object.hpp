/**
 * @file GInt32Object.hpp
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

#ifndef GINT32OBJECT_HPP_
#define GINT32OBJECT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here

#include "geneva/GNumIntT.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class encapsulates a single integer value. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GInt32Collection instead.
 *
 * Integers are adapted by the GInt32FlipAdaptor or the GInt32GaussAdaptor in Geneva.
 * The reason for this class is that there might be applications where one might want different
 * adaptor characteristics for different values. This cannot be done with a GInt32Collection.
 * Plus, having a separate integer class adds some consistency to Geneva, as other values
 * (most notably doubles) have their own class as well (GConstrainedDoubleObject, GDoubleObject).
 */
class GInt32Object
	:public GNumIntT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumIntT", boost::serialization::base_object<GNumIntT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GInt32Object();
	/** @brief The copy constructor */
	GInt32Object(const GInt32Object&);
	/** @brief Initialization by contained value */
	explicit GInt32Object(const boost::int32_t&);
	/** @brief Initialization by random number in a given range */
	GInt32Object(
			const boost::int32_t&
			, const boost::int32_t&
	);
	/** @brief Initialization with a fixed value and a range for random initialization */
	GInt32Object(
			const boost::int32_t&
			, const boost::int32_t&
			, const boost::int32_t&
	);
	/** @brief The destructor */
	virtual ~GInt32Object();

	/** @brief An assignment operator for the contained value type */
	virtual boost::int32_t operator=(const boost::int32_t&);

	/** @brief A standard assignment operator */
	const GInt32Object& operator=(const GInt32Object&);

	/** @brief Checks for equality with another GInt32Object object */
	bool operator==(const GInt32Object&) const;
	/** @brief Checks for inequality with another GInt32Object object */
	bool operator!=(const GInt32Object&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
	      const GObject&
	      , const Gem::Common::expectation&
	      , const double&
	      , const std::string&
	      , const std::string&
	      , const bool&
	) const;

	/** @brief Attach our local value to the vector. */
	virtual void int32Streamline(std::vector<boost::int32_t>&) const;
	/** @brief Attach boundaries of type boost::int32_t to the vectors */
	virtual void int32Boundaries(std::vector<boost::int32_t>&, std::vector<boost::int32_t>&) const;
	/** @brief Tell the audience that we own a boost::int32_t value */
	virtual std::size_t countInt32Parameters() const;
	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&);

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

#ifdef GEM_TESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GEM_TESTING */
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GInt32Object)

#endif /* GINT32OBJECT_HPP_ */
