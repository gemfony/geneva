/**
 * @file GInt32Collection.hpp
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

// Standard header files go here

// Boost header files go here

#ifndef GINT32COLLECTION_HPP_
#define GINT32COLLECTION_HPP_

// Geneva header files go here

#include "geneva/GIntNumCollectionT.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of std::int32_t objects without boundaries
 */
class GInt32Collection
	:public GIntNumCollectionT<std::int32_t>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GIntNumCollectionT_int32", boost::serialization::base_object<GIntNumCollectionT<std::int32_t>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GInt32Collection();
	 /** @brief Initialization with a number of random values in a given range */
	 G_API_GENEVA GInt32Collection(
		 const std::size_t&
		 , const std::int32_t&
		 , const std::int32_t&
	 );
	 /** @brief Initialization with a number of predefined values and the initialization range */
	 G_API_GENEVA GInt32Collection(
		 const std::size_t&
		 , const std::int32_t&
		 , const std::int32_t&
		 , const std::int32_t&
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA GInt32Collection(const GInt32Collection&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GInt32Collection();

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
	 G_API_GENEVA void load_(const GObject* cp) override;

	 /** @brief Attach our local values to the vector. */
	 G_API_GENEVA void int32Streamline(std::vector<std::int32_t>&, const activityMode& am) const override;
	 /** @brief Attach boundaries of type std::int32_t to the vectors */
	 G_API_GENEVA void int32Boundaries(std::vector<std::int32_t>&, std::vector<std::int32_t>&, const activityMode& am) const override;
	 /** @brief Tell the audience that we own a number of std::int32_t values */
	 G_API_GENEVA std::size_t countInt32Parameters(const activityMode& am) const override;
	 /** @brief Assigns part of a value vector to the parameter */
	 G_API_GENEVA void assignInt32ValueVector(const std::vector<std::int32_t>&, std::size_t&, const activityMode& am) override;
	 /** @brief Attach our local values to the vector. */
	 G_API_GENEVA void int32Streamline(std::map<std::string, std::vector<std::int32_t>>&, const activityMode& am) const override;
	 /** @brief Assigns part of a value vector to the parameter */
	 G_API_GENEVA void assignInt32ValueVectors(const std::map<std::string, std::vector<std::int32_t>>&, const activityMode& am) override;

	 /** @brief Multiplication with a random value in a given range */
	 virtual G_API_GENEVA void int32MultiplyByRandom(
		 const std::int32_t& min
		 , const std::int32_t& max
		 , const activityMode& am
		 , Gem::Hap::GRandomBase&
	 ) override;
	 /** @brief Multiplication with a random value in the range [0,1[ */
	 G_API_GENEVA void int32MultiplyByRandom(const activityMode& am, Gem::Hap::GRandomBase&) override;
	 /** @brief Multiplication with a constant value */
	 G_API_GENEVA void int32MultiplyBy(const std::int32_t& value, const activityMode& am) override;
	 /** @brief Initialization with a constant value */
	 G_API_GENEVA void int32FixedValueInit(const std::int32_t& value, const activityMode& am) override;
	 /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
	 G_API_GENEVA void int32Add(std::shared_ptr<GParameterBase>, const activityMode& am) override;
	 /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
	 G_API_GENEVA void int32Subtract(std::shared_ptr<GParameterBase>, const activityMode& am) override;

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GInt32Collection)

#endif /* GINT32COLLECTION_HPP_ */
