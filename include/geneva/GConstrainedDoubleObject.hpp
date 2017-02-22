/**
 * @file GConstrainedDoubleObject.hpp
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

#ifndef GCONSTRAINEDDOUBLEOBJECT_HPP_
#define GCONSTRAINEDDOUBLEOBJECT_HPP_

// Geneva headers go here
#include "geneva/GConstrainedFPT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GConstrainedDoubleObject class allows to limit the value range of a double value,
 * while applying adaptions to a continuous range. This is done by means of a
 * mapping from an internal representation to an externally visible value.
 */
class GConstrainedDoubleObject
	: public GConstrainedFPT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar & make_nvp("GConstrainedFPT_double",
						  boost::serialization::base_object<GConstrainedFPT<double>>(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GConstrainedDoubleObject();
	/** @brief Initialization with boundaries only */
	G_API_GENEVA GConstrainedDoubleObject(const double&, const double&);
	/** @brief Initialization with value and boundaries */
	G_API_GENEVA GConstrainedDoubleObject(const double&, const double&, const double&);
	/** @brief The copy constructor */
	G_API_GENEVA GConstrainedDoubleObject(const GConstrainedDoubleObject&);
	/** @brief Initialization by contained value */
	explicit G_API_GENEVA GConstrainedDoubleObject(const double&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GConstrainedDoubleObject();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GConstrainedDoubleObject& operator=(const GConstrainedDoubleObject&);
	/** @brief An assignment operator for the contained value type */
	virtual G_API_GENEVA double operator=(const double&) override;

	/** @brief Checks for equality with another GConstrainedDoubleObject object */
	G_API_GENEVA bool operator==(const GConstrainedDoubleObject&) const;
	/** @brief Checks for inequality with another GConstrainedDoubleObject object */
	G_API_GENEVA bool operator!=(const GConstrainedDoubleObject&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

protected:
	/** @brief Loads the data of another GObject */
	virtual G_API_GENEVA void load_(const GObject*) override;
	/** @brief Creates a deep clone of this object. */
	virtual G_API_GENEVA GObject* clone_() const override;

	/** @brief Attach our local value to the vector. */
	virtual G_API_GENEVA void doubleStreamline(std::vector<double>&, const activityMode& am) const override;
	/** @brief Attach boundaries of type double to the vectors */
	virtual G_API_GENEVA void doubleBoundaries(std::vector<double>&, std::vector<double>&, const activityMode& am) const override;
	/** @brief Tell the audience that we own a double value */
	virtual G_API_GENEVA std::size_t countDoubleParameters(const activityMode& am) const override;
	/** @brief Assigns part of a value vector to the parameter */
	virtual G_API_GENEVA void assignDoubleValueVector(const std::vector<double>&, std::size_t&, const activityMode& am) override;
	/** @brief Attach our local value to the vector. */
	virtual G_API_GENEVA void doubleStreamline(std::map<std::string, std::vector<double>>&, const activityMode& am) const override;
	/** @brief Assigns part of a value map to the parameter */
	virtual G_API_GENEVA void assignDoubleValueVectors(const std::map<std::string, std::vector<double>>&, const activityMode& am) override;

	/** @brief Multiplication with a random value in a given range */
	virtual G_API_GENEVA void doubleMultiplyByRandom(const double& min, const double& max, const activityMode& am, Gem::Hap::GRandomBase&) override;
	/** @brief Multiplication with a random value in the range [0,1[ */
	virtual G_API_GENEVA void doubleMultiplyByRandom(const activityMode& am, Gem::Hap::GRandomBase&) override;
	/** @brief Multiplication with a constant value */
	virtual G_API_GENEVA void doubleMultiplyBy(const double& value, const activityMode& am) override;
	/** @brief Initialization with a constant value */
	virtual G_API_GENEVA void doubleFixedValueInit(const double& value, const activityMode& am) override;
	/** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
	virtual G_API_GENEVA void doubleAdd(std::shared_ptr<GParameterBase>, const activityMode& am) override;
	/** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
	virtual G_API_GENEVA void doubleSubtract(std::shared_ptr<GParameterBase>, const activityMode& am) override;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GConstrainedDoubleObject)

#endif /* GCONSTRAINEDDOUBLEOBJECT_HPP_ */
