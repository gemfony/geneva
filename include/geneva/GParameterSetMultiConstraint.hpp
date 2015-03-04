/**
 * @file GParameterSetConstraint.hpp
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

#ifndef GPARAMETERSETMULTICONSTRAINT_HPP_
#define GPARAMETERSETMULTICONSTRAINT_HPP_

// Geneva header files go here
#include "common/GHelperFunctionsT.hpp"
#include "common/GFormulaParserT.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/GIndividualMultiConstraint.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements constraint definitions based on GParameterSet-derivatives.
 * It is meant to be added to a constraint collection. The main purpose of this
 * class is to "translate" GOptimizableEntity-based constraints into constraints
 * based on GParameterSets
 */
class GParameterSetConstraint: public GPreEvaluationValidityCheckT<GOptimizableEntity>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   G_API void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPreEvaluationValidityCheckT<GOptimizableEntity>);
   }
   ///////////////////////////////////////////////////////////////////////
public:

   /** @brief The default constructor */
   G_API GParameterSetConstraint();
   /** @brief The copy constructor */
   G_API GParameterSetConstraint(const GParameterSetConstraint&);
   /** @brief The destructor */
   virtual G_API ~GParameterSetConstraint();

   /** @brief A standard assignment operator */
   G_API const GParameterSetConstraint& operator=(const GParameterSetConstraint&);

   /** @brief Checks for equality with another GIndividualConstraint object */
   G_API bool operator==(const GParameterSetConstraint&) const;
   /** @brief Checks for inequality with another GIndividualConstraint object */
   G_API bool operator!=(const GParameterSetConstraint&) const;

   /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
   virtual G_API boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual G_API void addConfigurationOptions(Gem::Common::GParserBuilder&) OVERRIDE;

protected:
   /** @brief Checks whether a given individual is valid */
   virtual G_API double check_(const GOptimizableEntity *) const OVERRIDE;
   /** @brief Checks whether a given GParameterSet object is valid */
   virtual G_API double check_(const GParameterSet *) const = 0;

   /** @brief Loads the data of another GParameterSetConstraint */
   virtual G_API void load_(const GObject*) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual G_API GObject* clone_() const = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a string as input, which describes a formula. It then
 * inserts parameter values into the string, parses the formula and returns the
 * value represented by the formula as the "check"-value. Note that this class
 * currently only deals with double values.
 */
class GParameterSetFormulaConstraint: public GParameterSetConstraint
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   G_API void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetConstraint)
     & BOOST_SERIALIZATION_NVP(rawFormula_);
   }
   ///////////////////////////////////////////////////////////////////////
public:
   /** @brief The default constructor */
   G_API GParameterSetFormulaConstraint(std::string);
   /** @brief The copy constructor */
   G_API GParameterSetFormulaConstraint(const GParameterSetFormulaConstraint&);
   /** @brief The destructor */
   virtual G_API ~GParameterSetFormulaConstraint();

   /** @brief A standard assignment operator */
   G_API const GParameterSetFormulaConstraint& operator=(const GParameterSetFormulaConstraint&);

   /** @brief Checks for equality with another GIndividualConstraint object */
   G_API bool operator==(const GParameterSetFormulaConstraint&) const;
   /** @brief Checks for inequality with another GIndividualConstraint object */
   G_API bool operator!=(const GParameterSetFormulaConstraint&) const;

   /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
   virtual G_API boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual G_API void addConfigurationOptions(Gem::Common::GParserBuilder&) OVERRIDE;

protected:
   /** @brief Checks whether a given GParameterSet object is valid */
   virtual G_API double check_(const GParameterSet *) const OVERRIDE;

   /** @brief Loads the data of another GParameterSetConstraint */
   virtual G_API void load_(const GObject*) OVERRIDE;
   /** @brief Creates a deep clone of this object */
   virtual G_API GObject* clone_() const;

private:
   /** @brief The default constructor -- intentionally private*/
   G_API GParameterSetFormulaConstraint();

   std::string rawFormula_; ///< Holds the raw formula, in which values haven't been replaced yet
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterSetConstraint)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSetFormulaConstraint)

#endif /* GPARAMETERSETMULTICONSTRAINT_HPP_ */
