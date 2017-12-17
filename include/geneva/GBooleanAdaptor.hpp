/**
 * @file GBooleanAdaptor.hpp
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

#ifndef GBOOLEANADAPTOR_HPP_
#define GBOOLEANADAPTOR_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GAdaptorT.hpp"
#include "GConstrainedDoubleObject.hpp"
#include "GObject.hpp"
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GBooleanAdaptor represents an adaptor used for the adaption of
 * boolean variables by flipping their values. See the documentation of GAdaptorT<T> for
 * further information on adaptors in the Geneva context. Most functionality
 * (with the notable exception of the actual adaption logic) is currently
 * implemented in the GAdaptorT class.
 */
class GBooleanAdaptor
	:public GAdaptorT<bool>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GAdaptorT_bool",
			 boost::serialization::base_object<GAdaptorT<bool>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GBooleanAdaptor();
	 /** @brief The copy constructor */
	 G_API_GENEVA GBooleanAdaptor(const GBooleanAdaptor&);
	 /** @brief Initialization with a adaption probability */
	 explicit G_API_GENEVA GBooleanAdaptor(const double&);

	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GBooleanAdaptor();

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Retrieves the id of this adaptor */
	 G_API_GENEVA Gem::Geneva::adaptorId getAdaptorId() const override;

	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA std::string name() const override;
	 /** @brief Random initialization of the adaptor */
	 G_API_GENEVA bool randomInit(Gem::Hap::GRandomBase&) override;

protected:
	 /** @brief Loads the data of another GObject */
	 G_API_GENEVA void load_(const GObject*) override;

	 /** @brief Flip the value up or down by 1, depending on a random number */
	 virtual G_API_GENEVA void customAdaptions(
		 bool&
		 , const bool&
		 , Gem::Hap::GRandomBase&
	 ) override;

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanAdaptor)

#endif /* GBOOLEANADAPTOR_HPP_ */
