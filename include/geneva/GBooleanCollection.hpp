/**
 * @file GBooleanCollection.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard headers go here

// Boost headers go here

#ifndef GBOOLEANCOLLECTION_HPP_
#define GBOOLEANCOLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterCollectionT.hpp"
#include "geneva/GBooleanAdaptor.hpp"

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/**
 * This class represents collections of bits. They are usually adapted by
 * the GBooleanAdaptor, which has a mutable flip probability. One adaptor
 * is applied to all bits. If you want individual flip probabilities for
 * all bits, use GBool objects instead.
 */
class GBooleanCollection
:public GParameterCollectionT<bool>
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
   GBooleanCollection();
   /** @brief Random initialization with a given number of values */
   explicit GBooleanCollection(const std::size_t&);
   /** @brief Initialization with a given number of items of defined value */
   GBooleanCollection(const std::size_t&, const bool&);
   /** @brief Random initialization with a given number of values of
    * a certain probability structure */
   GBooleanCollection(const std::size_t&, const double&);
   /** @brief A standard copy constructor */
   GBooleanCollection(const GBooleanCollection&);
   /** @brief The standard destructor */
   virtual ~GBooleanCollection();

   /** @brief A standard assignment operator */
   const GBooleanCollection& operator=(const GBooleanCollection&);

   /** @brief Checks for equality with another GBooleanCollection object */
   bool operator==(const GBooleanCollection&) const;
   /** @brief Checks for inequality with another GBooleanCollection object */
   bool operator!=(const GBooleanCollection&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const;

   /** @brief Random initialization */
   virtual void randomInit();
   /** @brief Random initialization with a given probability structure */
   virtual void randomInit(const double&);

   /** @brief Attach our local values to the vector. */
   virtual void booleanStreamline(std::vector<bool>&) const;
   /** @brief Attach boundaries of type bool to the vectors */
   virtual void booleanBoundaries(std::vector<bool>&, std::vector<bool>&) const;
   /** @brief Tell the audience that we own a number of boolean values */
   virtual std::size_t countBoolParameters() const;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignBooleanValueVector(const std::vector<bool>&, std::size_t&);

   /** @brief Emits a name for this class / object */
   virtual std::string name() const;

protected:
   /** @brief Loads the data of another GBooleanCollection class */
   virtual void load_(const GObject *);
   /** @brief Creates a deep copy of this object */
   virtual GObject *clone_() const;

   /** @brief Triggers random initialization of the parameter collection */
   virtual void randomInit_();
   /** @brief Triggers random initialization of the parameter collection, with a given likelihood structure */
   void randomInit_(const double&);

public:
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual bool modify_GUnitTests();
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual void specificTestsNoFailureExpected_GUnitTests();
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual void specificTestsFailuresExpected_GUnitTests();
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanCollection)

#endif /* GBOOLEANCOLLECTION_HPP_ */
