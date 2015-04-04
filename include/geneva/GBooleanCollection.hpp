/**
 * @file GBooleanCollection.hpp
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

#ifndef GBOOLEANCOLLECTION_HPP_
#define GBOOLEANCOLLECTION_HPP_


// Geneva headers go here
#include "common/GExceptions.hpp"
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

      ar & make_nvp("GParameterCollectionT_bool",
            boost::serialization::base_object<GParameterCollectionT<bool> >(*this));
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   G_API_GENEVA GBooleanCollection();
   /** @brief Random initialization with a given number of values */
   explicit G_API_GENEVA GBooleanCollection(const std::size_t&);
   /** @brief Initialization with a given number of items of defined value */
   G_API_GENEVA GBooleanCollection(const std::size_t&, const bool&);
   /** @brief Random initialization with a given number of values of
    * a certain probability structure */
   G_API_GENEVA GBooleanCollection(const std::size_t&, const double&);
   /** @brief A standard copy constructor */
   G_API_GENEVA GBooleanCollection(const GBooleanCollection&);
   /** @brief The standard destructor */
   virtual G_API_GENEVA ~GBooleanCollection();

   /** @brief The standard assignment operator */
   G_API_GENEVA const GBooleanCollection& operator=(const GBooleanCollection&);

   /** @brief Checks for equality with another GBooleanCollection object */
   G_API_GENEVA bool operator==(const GBooleanCollection&) const;
   /** @brief Checks for inequality with another GBooleanCollection object */
   G_API_GENEVA bool operator!=(const GBooleanCollection&) const;

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const OVERRIDE;

   /** @brief Random initialization */
   virtual G_API_GENEVA void randomInit(const activityMode&) OVERRIDE;
   /** @brief Random initialization with a given probability structure */
   G_API_GENEVA void randomInit(const double&, const activityMode&);

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
   /** @brief Loads the data of another GBooleanCollection class */
   virtual G_API_GENEVA void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep copy of this object */
   virtual G_API_GENEVA GObject *clone_() const OVERRIDE;

   /** @brief Triggers random initialization of the parameter collection */
   virtual G_API_GENEVA void randomInit_(const activityMode&) OVERRIDE;
   /** @brief Triggers random initialization of the parameter collection, with a given likelihood structure */
   G_API_GENEVA void randomInit_(const double&, const activityMode&);

   /** @brief Returns a "comparative range" for this type */
   virtual G_API_GENEVA bool range() const OVERRIDE;

   /** @brief Tell the audience that we own a number of boolean values */
   virtual G_API_GENEVA std::size_t countBoolParameters(const activityMode& am) const OVERRIDE;
   /** @brief Attach boundaries of type bool to the vectors */
   virtual G_API_GENEVA void booleanBoundaries(std::vector<bool>&, std::vector<bool>&, const activityMode& am) const OVERRIDE;
   /** @brief Attach our local values to the vector. */
   virtual G_API_GENEVA void booleanStreamline(std::vector<bool>&, const activityMode& am) const OVERRIDE;
   /** @brief Attach our local values to the map */
   virtual G_API_GENEVA void booleanStreamline(std::map<std::string, std::vector<bool> >&, const activityMode& am) const OVERRIDE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignBooleanValueVector(const std::vector<bool>&, std::size_t&, const activityMode& am) OVERRIDE;
   /** @brief Assigns part of a value map to the parameter */
   virtual G_API_GENEVA void assignBooleanValueVectors(const std::map<std::string, std::vector<bool> >&, const activityMode& am) OVERRIDE;

public:
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanCollection)

#endif /* GBOOLEANCOLLECTION_HPP_ */
