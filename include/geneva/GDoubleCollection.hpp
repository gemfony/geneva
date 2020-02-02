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

// Boost header files go here

// Geneva header files go here
#include "geneva/GFPNumCollectionT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of double objects without boundaries
 */
class GDoubleCollection
	:public GFPNumCollectionT<double>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GFPNumCollectionT_double", boost::serialization::base_object<GFPNumCollectionT<double>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GDoubleCollection() = default;
	 /** @brief The copy constructor */
	 G_API_GENEVA GDoubleCollection(const GDoubleCollection&) = default;
	 /** @brief Initialization with a number of random values in a given range */
	 G_API_GENEVA GDoubleCollection(
		 const std::size_t&
		 , const double&
		 , const double&
	 );
	 /** @brief Initialization with a number of predefined values in all positions */
	 G_API_GENEVA GDoubleCollection(
		 const std::size_t&
		 , const double&
		 , const double&
		 , const double&
	 );
	 /** @brief The destructor */
	 G_API_GENEVA ~GDoubleCollection() override = default;

protected:
	 /** @brief Loads the data of another GObject */
	 G_API_GENEVA void load_(const GObject*) override;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GDoubleCollection>(
		GDoubleCollection const &
		, GDoubleCollection const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	G_API_GENEVA void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	 /** @brief Attach our local values to the vector. */
	 G_API_GENEVA void doubleStreamline(std::vector<double>&, const activityMode& am) const override;
	 /** @brief Attach boundaries of type double to the vectors */
	 G_API_GENEVA void doubleBoundaries(std::vector<double>&, std::vector<double>&, const activityMode& am) const override;
	 /** @brief Tell the audience that we own a number of double values */
	 G_API_GENEVA std::size_t countDoubleParameters(const activityMode& am) const override;
	 /** @brief Assigns part of a value vector to the parameter */
	 G_API_GENEVA void assignDoubleValueVector(const std::vector<double>&, std::size_t&, const activityMode& am) override;
	 /** @brief Attach our local values to the vector. */
	 G_API_GENEVA void doubleStreamline(std::map<std::string, std::vector<double>>&, const activityMode& am) const override;
	 /** @brief Assigns part of a value map to the parameter */
	 G_API_GENEVA void assignDoubleValueVectors(const std::map<std::string, std::vector<double>>&, const activityMode& am) override;

	 /** @brief Multiplication with a random value in a given range */
	 G_API_GENEVA void doubleMultiplyByRandom(const double& min, const double& max, const activityMode& am, Gem::Hap::GRandomBase&) override;
	 /** @brief Multiplication with a random value in the range [0,1[ */
	 G_API_GENEVA void doubleMultiplyByRandom(const activityMode& am, Gem::Hap::GRandomBase&) override;
	 /** @brief Multiplication with a constant value */
	 G_API_GENEVA void doubleMultiplyBy(const double& value, const activityMode& am) override;
	 /** @brief Initialization with a constant value */
	 G_API_GENEVA void doubleFixedValueInit(const double& value, const activityMode& am) override;
	 /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
	 G_API_GENEVA void doubleAdd(std::shared_ptr<GParameterBase>, const activityMode& am) override;
	 /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
	 G_API_GENEVA void doubleSubtract(std::shared_ptr<GParameterBase>, const activityMode& am) override;

	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA bool modify_GUnitTests_() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name_() const override;
	 /** @brief Creates a deep clone of this object. */
	 G_API_GENEVA GObject* clone_() const override;

	 /** @brief Fills the collection with some random data */
	 G_API_GENEVA void fillWithData_(const std::size_t&);
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleCollection)

