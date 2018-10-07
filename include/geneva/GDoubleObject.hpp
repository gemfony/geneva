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

// Standard headers go here

// Boost headers go here

// Geneva headers go here

#include "geneva/GNumFPT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class encapsulates a double type. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GDoubleCollection class or individual GConstrainedDoubleObject objects
 * instead.
 */
class GDoubleObject
	:public GNumFPT<double>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GNumFPT_double", boost::serialization::base_object<GNumFPT<double>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GDoubleObject();
	 /** @brief The copy constructor */
	 G_API_GENEVA GDoubleObject(const GDoubleObject&);
	 /** @brief Initialization by contained value */
	 explicit G_API_GENEVA GDoubleObject(const double&);
	 /** @brief Random initialization in a given range */
	 G_API_GENEVA GDoubleObject(const double&, const double&);
	 /** @brief Initialization with a fixed value and the initialization range */
	 G_API_GENEVA GDoubleObject(const double&, const double&, const double&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GDoubleObject();

	 /** @brief An assignment operator for the contained value type */
	 G_API_GENEVA double operator=(const double&) override;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;

protected:
	 /** @brief Loads the data of another GObject */
	 G_API_GENEVA void load_(const GObject*) override;

	 /** @brief Attach our local value to the vector. */
	 G_API_GENEVA void doubleStreamline(std::vector<double>&, const activityMode& am) const override;
	 /** @brief Attach boundaries of type double to the vectors */
	 G_API_GENEVA void doubleBoundaries(std::vector<double>&, std::vector<double>&, const activityMode& am) const override;
	 /** @brief Tell the audience that we own a double value */
	 G_API_GENEVA std::size_t countDoubleParameters(const activityMode& am) const override;
	 /** @brief Assigns part of a value vector to the parameter */
	 G_API_GENEVA void assignDoubleValueVector(const std::vector<double>&, std::size_t&, const activityMode& am) override;
	 /** @brief Attach our local value to the map. */
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

private:
	 /** @brief Creates a deep clone of this object. */
	 G_API_GENEVA GObject* clone_() const override;

public:
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};


/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleObject)

