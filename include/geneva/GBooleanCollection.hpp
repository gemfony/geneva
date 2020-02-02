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

// Standard headers go here
#include <random>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterCollectionT.hpp"
#include "geneva/GBooleanAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents collections of bits. They are usually adapted by
 * the GBooleanAdaptor, which has a mutable flip probability. One adaptor
 * is applied to all bits. If you want individual flip probabilities for
 * all bits, use GBool objects instead.
 */
class GBooleanCollection :public GParameterCollectionT<bool>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar & make_nvp("GParameterCollectionT_bool", boost::serialization::base_object<GParameterCollectionT<bool>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GBooleanCollection() = default;
	 /** @brief Random initialization with a given number of values */
	 explicit G_API_GENEVA GBooleanCollection(const std::size_t&);
	 /** @brief Initialization with a given number of items of defined value */
	 G_API_GENEVA GBooleanCollection(const std::size_t&, const bool&);
	 /** @brief Random initialization with a given number of values of
	  * a certain probability structure */
	 G_API_GENEVA GBooleanCollection(const std::size_t&, const double&);
	 /** @brief A standard copy constructor */
	 G_API_GENEVA GBooleanCollection(const GBooleanCollection&) = default;
	 /** @brief The standard destructor */
	 G_API_GENEVA ~GBooleanCollection() override = default;

	 /** @brief FLips the value at a given position */
	 G_API_GENEVA void flip(const std::size_t&);

	 /** @brief Random initialization */
	 G_API_GENEVA bool randomInit(
		 const activityMode&
		 , Gem::Hap::GRandomBase&
	 ) override;
	 /** @brief Random initialization with a given probability structure */
	 G_API_GENEVA bool randomInit(
		 const double&
		 , const activityMode&
		 , Gem::Hap::GRandomBase&
	 );

protected:
	 /** @brief Loads the data of another GBooleanCollection class */
	 G_API_GENEVA void load_(const GObject *) override;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GBooleanCollection>(
		GBooleanCollection const &
		, GBooleanCollection const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	G_API_GENEVA void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Triggers random initialization of the parameter collection */
	G_API_GENEVA bool randomInit_(
		 const activityMode&
		 , Gem::Hap::GRandomBase&
	 ) override;
	 /** @brief Triggers random initialization of the parameter collection, with a given likelihood structure */
	 G_API_GENEVA bool randomInit_(
		 const double&
		 , const activityMode&
		 , Gem::Hap::GRandomBase&
	 );

	 /** @brief Returns a "comparative range" for this type */
	 G_API_GENEVA bool range() const override;

	 /** @brief Tell the audience that we own a number of boolean values */
	 G_API_GENEVA std::size_t countBoolParameters(const activityMode& am) const override;
	 /** @brief Attach boundaries of type bool to the vectors */
	 G_API_GENEVA void booleanBoundaries(std::vector<bool>&, std::vector<bool>&, const activityMode& am) const override;
	 /** @brief Attach our local values to the vector. */
	 G_API_GENEVA void booleanStreamline(std::vector<bool>&, const activityMode& am) const override;
	 /** @brief Attach our local values to the map */
	 G_API_GENEVA void booleanStreamline(std::map<std::string, std::vector<bool>>&, const activityMode& am) const override;
	 /** @brief Assigns part of a value vector to the parameter */
	 G_API_GENEVA void assignBooleanValueVector(const std::vector<bool>&, std::size_t&, const activityMode& am) override;
	 /** @brief Assigns part of a value map to the parameter */
	 G_API_GENEVA void assignBooleanValueVectors(const std::map<std::string, std::vector<bool>>&, const activityMode& am) override;

	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA bool modify_GUnitTests_() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name_() const override;
	 /** @brief Creates a deep copy of this object */
	 G_API_GENEVA GObject *clone_() const override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanCollection)

