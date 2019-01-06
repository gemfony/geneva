/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort
#include <tuple>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GSingletonT.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GCommonMathHelperFunctions.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"

namespace Gem {
namespace Tests {

/******************************************************************************/
/**
 * This individual tests different access methods for parameter objects inside
 * of the individual.
 */
class GTestIndividual3
	: public Gem::Geneva::GParameterSet
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 using namespace Gem::Geneva;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_INDIVIDUALS GTestIndividual3();
	 /** @brief The copy constructor */
	 G_API_INDIVIDUALS GTestIndividual3(const GTestIndividual3 &);

	 /** @brief The destructor */
	 virtual G_API_INDIVIDUALS ~GTestIndividual3();

	 /** @brief Get all data members of this class as a plain array */
	 G_API_INDIVIDUALS std::shared_ptr<float> getPlainData() const;

protected:
	 /** @brief Loads the data of another GTestIndividual3 */
	 virtual G_API_INDIVIDUALS void load_(const GObject *) final;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GTestIndividual3>(
		GTestIndividual3 const &
		, GTestIndividual3 const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_INDIVIDUALS void compare_(
		const GObject & // the other object
		, const Gem::Common::expectation & // the expectation for this object, e.g. equality
		, const double & // the limit for allowed deviations of floating point types
	) const final;


	/** @brief The actual fitness calculation takes place here. */
	virtual G_API_INDIVIDUALS double fitnessCalculation() final;

	/** @brief Applies modifications to this object. */
	virtual G_API_INDIVIDUALS bool modify_GUnitTests_();
	/** @brief Performs self tests that are expected to succeed. */
	virtual G_API_INDIVIDUALS void specificTestsNoFailureExpected_GUnitTests_();
	/** @brief Performs self tests that are expected to fail. */
	virtual G_API_INDIVIDUALS void specificTestsFailuresExpected_GUnitTests_();

private:
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_INDIVIDUALS GObject *clone_() const final;
};

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Tests::GTestIndividual3)
