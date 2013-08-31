/**
 * @file GGDPersonalityTraits.hpp
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


// Standard headers go here

// Boost headers go here


#ifndef GGDPERSONALITYTRAITS_HPP_
#define GGDPERSONALITYTRAITS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to gradient descents.
 */
class GGDPersonalityTraits :public GPersonalityTraits
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
	     & BOOST_SERIALIZATION_NVP(popPos_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GGDPersonalityTraits();
	/** @brief The copy contructor */
	GGDPersonalityTraits(const GGDPersonalityTraits&);
	/** @brief The standard destructor */
	virtual ~GGDPersonalityTraits();

	/** @brief A standard assignment operator */
	const GGDPersonalityTraits& operator=(const GGDPersonalityTraits&);

	/** @brief Checks for equality with another GGDPersonalityTraits object */
	bool operator==(const GGDPersonalityTraits&) const;
	/** @brief Checks for inequality with another GGDPersonalityTraits object */
	bool operator!=(const GGDPersonalityTraits&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const OVERRIDE;

	/** @brief Sets the position of the individual in the population */
	void setPopulationPosition(const std::size_t&) ;
	/** @brief Retrieves the position of the individual in the population */
	std::size_t getPopulationPosition(void) const ;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GGDPersonalityTraits object */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const OVERRIDE;

private:
	/** @brief Stores the current position in the population */
	std::size_t popPos_;

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GGDPersonalityTraits)

#endif /* GGDPERSONALITYTRAITS_HPP_ */

