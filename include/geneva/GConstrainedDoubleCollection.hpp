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
#include <string>
#include <sstream>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "geneva/GConstrainedFPNumCollectionT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents a collection of double values with common
 * boundaries, all modified using the same algorithm. Note: If you want
 * to access or set the transformed value, use the value() and setValue()
 * functions. Using the subscript operator or at() function, or the
 * native iterator, will give you the "raw" data only.
 */
class GConstrainedDoubleCollection
	: public GConstrainedFPNumCollectionT<double>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & make_nvp("GConstrainedFPNumCollectionT", boost::serialization::base_object<GConstrainedFPNumCollectionT<double>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /** @brief Initialize the lower and upper boundaries for data members of this class */
	 G_API_GENEVA GConstrainedDoubleCollection (
		 const std::size_t&
		 , const double&
		 , const double&
	 );
	 /** @brief Assign a fixed value to all positions of the vector and initialize the allowed value range */
	 G_API_GENEVA GConstrainedDoubleCollection (
		 const std::size_t&
		 , const double&
		 , const double&
		 , const double&
	 );
	 /** @brief The standard copy constructor */
	 G_API_GENEVA GConstrainedDoubleCollection(const GConstrainedDoubleCollection&) = default;
	 /** @brief The standard destructor */
	 G_API_GENEVA ~GConstrainedDoubleCollection() override = default;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GConstrainedDoubleCollection object */
	 G_API_GENEVA void load_(const GObject *) override;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GConstrainedDoubleCollection>(
		GConstrainedDoubleCollection const &
		, GConstrainedDoubleCollection const &
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

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA bool modify_GUnitTests_() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
	 /** @brief Performs self tests that are expected to fail */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

	 /***************************************************************************/
	 /** @brief The default constructor. Intentionally protected	 */
	 G_API_GENEVA GConstrainedDoubleCollection() = default;

private:
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA GObject* clone_() const override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GConstrainedDoubleCollection has a private default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
inline std::shared_ptr<Gem::Geneva::GConstrainedDoubleCollection> TFactory_GUnitTests<Gem::Geneva::GConstrainedDoubleCollection>() {
	const std::size_t NPARAMETERS = 100;
	double LOWERBOUNDARY = -10.;
	double UPPERBOUNDARY =  10.;
	std::shared_ptr<Gem::Geneva::GConstrainedDoubleCollection> p;
	BOOST_CHECK_NO_THROW(
		p= std::shared_ptr<Gem::Geneva::GConstrainedDoubleCollection>(
			new Gem::Geneva::GConstrainedDoubleCollection(NPARAMETERS, LOWERBOUNDARY, UPPERBOUNDARY)
		)
	);
	return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GConstrainedDoubleCollection)

/******************************************************************************/

