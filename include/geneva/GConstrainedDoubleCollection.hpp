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

