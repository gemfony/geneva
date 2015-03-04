/**
 * @file GSerialEA.hpp
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

// Standard headers go here

// Boost headers go here

#ifndef GSERIALEA_HPP_
#define GSERIALEA_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GBaseEA.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class adds a simple, serial adaptChildren() call to the GBaseEA class.
 */
class GSerialEA
	: public GBaseEA
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	G_API void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseEA);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API GSerialEA();
	/** @brief A standard copy constructor */
	G_API GSerialEA(const GSerialEA&);
	/** @brief The standard destructor */
	virtual G_API ~GSerialEA();

	/** @brief Assignment operator */
	G_API const GSerialEA& operator=(const GSerialEA&);

	/** @brief Checks for equality with another GSerialEA object */
	G_API bool operator==(const GSerialEA&) const;
	/** @brief Checks for inequality with another GSerialEA object */
	G_API bool operator!=(const GSerialEA&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	virtual G_API void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) OVERRIDE;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual G_API std::string getIndividualCharacteristic() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API std::string name() const OVERRIDE;

protected:
	/** @brief Loads data from another object */
	virtual G_API void load_(const GObject *) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual G_API GObject *clone_() const OVERRIDE;

	/** @brief Adapt children in a serial manner */
	virtual G_API void adaptChildren() OVERRIDE;
	/** @brief Evaluates all children (and possibly parents) of this population */
	virtual G_API void runFitnessCalculation() OVERRIDE;

	/** @brief Necessary initialization work before the start of the optimization */
	virtual G_API void init() OVERRIDE;
	/** @brief Necessary clean-up work after the optimization has finished */
	virtual G_API void finalize() OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSerialEA)

#endif /* GSERIALEA_HPP_ */
