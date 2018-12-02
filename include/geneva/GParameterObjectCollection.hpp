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

// Boost header files go here
#include <boost/cast.hpp>

// Geneva header files go here
#include "geneva/GParameterBase.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GBooleanObject.hpp"
#include "geneva/GInt32Object.hpp"
#include "geneva/GDoubleObject.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GParameterBase objects, ready for use in a
 * GParameterSet derivative.
 */
class GParameterObjectCollection
	:public GParameterTCollectionT<GParameterBase>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar & make_nvp("GParameterTCollectionT_gbd",
			 boost::serialization::base_object<GParameterTCollectionT<GParameterBase>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GParameterObjectCollection() = default;
	 /** @brief Initialization with a number of GParameterBase objects */
	 G_API_GENEVA GParameterObjectCollection(const std::size_t&, std::shared_ptr<GParameterBase>);
	 /** @brief The copy constructor */
	 G_API_GENEVA GParameterObjectCollection(const GParameterObjectCollection&) = default;
	 /** @brief The destructor */
	 G_API_GENEVA ~GParameterObjectCollection() override = default;

	 /** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
	 G_API_GENEVA std::shared_ptr<Gem::Geneva::GParameterBase> at(const std::size_t& pos);

	 /***************************************************************************/
	 /**
	  * This function returns a parameter item at a given position of the data set.
		* Note that this function will only be accessible to the compiler if parameter_type
		* is a derivative of GParameterBase, thanks to the magic of std::enable_if
		* and type_traits
	  *
	  * @param pos The position in our data array that shall be converted
	  * @return A converted version of the GParameterBase object, as required by the user
	  */
	 template <typename parameter_type>
	 const std::shared_ptr<parameter_type> at(
		 const std::size_t& pos
		 , typename std::enable_if<std::is_base_of<GParameterBase, parameter_type>::value>::type *dummy = nullptr
	 )  const {
#ifdef DEBUG
		 if(this->empty() || pos >= this->size()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObjectCollection::at<>(): Error!" << std::endl
					 << "Tried to access position " << pos << " while size is " << this->size() << std::endl
			 );

			 // Make the compiler happy
			 return std::shared_ptr<parameter_type>();
		 }
#endif /* DEBUG */

		 // Does error checks on the conversion internally
		 return Gem::Common::convertSmartPointer<GParameterBase, parameter_type>(data.at(pos));
	 }

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GObject */
	 G_API_GENEVA void load_(const GObject*) override;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GParameterObjectCollection>(
		GParameterObjectCollection const &
		, GParameterObjectCollection const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	G_API_GENEVA void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

private:
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name_() const override;
	 /** @brief Creates a deep clone of this object. */
	 G_API_GENEVA GObject* clone_() const override;

public:
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Fills the collection with GParameterBase objects */
	 G_API_GENEVA void fillWithObjects();
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterObjectCollection)
