/**
 * @file GPersonalityTraits.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard headers go here

// Boost headers go here

#ifndef GPERSONALITYTRAITS_HPP_
#define GPERSONALITYTRAITS_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GObjectExpectationChecksT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is the base class for a small hierarchy that encapsulates information
 * relevant to particular optimization algorithms. The information is stored in
 * individuals (i.e. the parameter sets which are subject to a given optimization
 * problem). In this sense, individuals can take on more than one role or
 * personality. Note that this class is purely virtual. It can only be used in
 * conjunction with a derived personality.
 */
class GPersonalityTraits :public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GPersonalityTraits();
	/** @brief The copy constructor */
	GPersonalityTraits(const GPersonalityTraits&);
	/** @brief The standard destructor */
	virtual ~GPersonalityTraits();

	/** @brief Checks for equality with another GPersonalityTraits object */
	bool operator==(const GPersonalityTraits&) const;
	/** @brief Checks for inequality with another GPersonalityTraits object */
	bool operator!=(const GPersonalityTraits&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GPersonalityTraits object */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

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

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GPersonalityTraits)
/******************************************************************************/

#endif /* GPERSONALITYTRAITS_HPP_ */
