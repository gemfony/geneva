/**
 * @file GParameterSetMultiConstraint.hpp
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

// Standard header files go here

// Boost header files go here

#ifndef GPARAMETERSETMULTICONSTRAINT_HPP_
#define GPARAMETERSETMULTICONSTRAINT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GHelperFunctionsT.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/GIndividualMultiConstraint.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class implements constraint definitions based on multiple parameters
 * coming from GParameterSet-derivatives
 */
class GParameterSetMultiConstraint: GValidityCheckT<GIndividual>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GValidityCheckT<GIndividual>);
   }
   ///////////////////////////////////////////////////////////////////////
public:

   /** @brief The default constructor */
   GParameterSetMultiConstraint();
   /** @brief The copy constructor */
   GParameterSetMultiConstraint(const GParameterSetMultiConstraint&);
   /** @brief The destructor */
   virtual ~GParameterSetMultiConstraint();

   /** @brief A standard assignment operator */
   const GParameterSetMultiConstraint& operator=(const GParameterSetMultiConstraint&);

   /** @brief Checks for equality with another GIndividualConstraint object */
   bool operator==(const GParameterSetMultiConstraint&) const;
   /** @brief Checks for inequality with another GIndividualConstraint object */
   bool operator!=(const GParameterSetMultiConstraint&) const;

   /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject&
         , const Gem::Common::expectation&
         , const double&
         , const std::string&
         , const std::string&
         , const bool&
   ) const;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

protected:
   /** @brief Checks whether a given individual is valid */
   virtual double check_(const GIndividual *, const double&) const;
   /** @brief Checks whether a given GParameterSet object is valid */
   virtual double check_(const GParameterSet *, const double&) const = 0;

   /** @brief Loads the data of another GParameterSetMultiConstraint */
   virtual void load_(const GObject*);
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const = 0;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterSetMultiConstraint)

#endif /* GPARAMETERSETMULTICONSTRAINT_HPP_ */
