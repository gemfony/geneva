/**
 * @file
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

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

// Boost header files go here
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp"
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Tests {

/**
 * The types of objects to be tested in this class
 */
enum class PERFOBJECTTYPE : Gem::Common::ENUMBASETYPE {
	 PERFGDOUBLEOBJECT = 0,
	 PERFGCONSTRDOUBLEOBJECT = 1,
	 PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION = 2,
	 PERFGDOUBLECOLLECTION = 3,
	 PERFGCONSTRAINEDDOUBLECOLLECTION = 4
};

const PERFOBJECTTYPE POTMIN = PERFOBJECTTYPE::PERFGDOUBLEOBJECT;
const PERFOBJECTTYPE POTMAX = PERFOBJECTTYPE::PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION;
const std::size_t NPERFOBJECTTYPES = 5;

/******************************************************************************/
/** @brief Puts a Gem::Common::logType into a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, const Gem::Tests::PERFOBJECTTYPE&);
/** @brief Reads a Gem::Common::logType from a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::istream &operator>>(std::istream &, Gem::Tests::PERFOBJECTTYPE&);

/******************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual2 : public Gem::Geneva::GParameterSet {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_INDIVIDUALS GTestIndividual2(const std::size_t &, const PERFOBJECTTYPE &);
	 /** @brief The copy constructor */
	 G_API_INDIVIDUALS GTestIndividual2(const GTestIndividual2 &);

	 /** @brief The standard destructor */
	 virtual G_API_INDIVIDUALS ~GTestIndividual2();

protected:
	 /** @brief Loads the data of another GTestIndividual2 */
	 virtual G_API_INDIVIDUALS void load_(const GObject *) final;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GTestIndividual2>(
		GTestIndividual2 const &
		, GTestIndividual2 const &
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

	 /** @brief The default constructor -- protected, as it is only needed for (de-)serialization purposes */
	 GTestIndividual2();
};

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Tests::GTestIndividual2)

