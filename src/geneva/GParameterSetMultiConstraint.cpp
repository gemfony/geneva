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

#include "geneva/GParameterSetMultiConstraint.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GParameterSetMultiConstraint::GParameterSetMultiConstraint()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GParameterSetMultiConstraint::GParameterSetMultiConstraint(const GParameterSetMultiConstraint& cp)
   : GValidityCheckT<GOptimizableEntity>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSetMultiConstraint::~GParameterSetMultiConstraint()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GParameterSetMultiConstraint& GParameterSetMultiConstraint::operator=(const GParameterSetMultiConstraint& cp)
{
   GValidityCheckT<GOptimizableEntity>::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GIndividualConstraint object
 */
bool GParameterSetMultiConstraint::operator==(const GParameterSetMultiConstraint& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterSetMultiConstraint::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GIndividualConstraint object
 */
bool GParameterSetMultiConstraint::operator!=(const GParameterSetMultiConstraint& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterSetMultiConstraint::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GParameterSetMultiConstraint::checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with an object of the same type and that we are not
   // accidently trying to compare this object with itself.
   const GParameterSetMultiConstraint *p_load = GObject::gobject_conversion<GParameterSetMultiConstraint>(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GValidityCheckT<GOptimizableEntity>::checkRelationshipWith(cp, e, limit, "GParameterSetMultiConstraint", y_name, withMessages));

   // no local data

   return evaluateDiscrepancies("GParameterSetMultiConstraint", caller, deviations, e);
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GParameterSetMultiConstraint::addConfigurationOptions(
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
) {
   // Call our parent class'es function
   GValidityCheckT<GOptimizableEntity>::addConfigurationOptions(gpb, showOrigin);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GParameterSetMultiConstraint::check_(
      const GOptimizableEntity * p_raw
      , const double& validityThreshold
) const {
   const GParameterSet * p = Gem::Common::convertSimplePointer<GOptimizableEntity, GParameterSet>(p_raw);
   return this->check_(p, validityThreshold);
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetMultiConstraint
 */
void GParameterSetMultiConstraint::load_(const GObject* cp) {
   // Check that we are indeed dealing with an object of the same type and that we are not
   // accidently trying to compare this object with itself.
   const GParameterSetMultiConstraint *p_load = GObject::gobject_conversion<GParameterSetMultiConstraint>(cp);

   // Load our parent class'es data ...
   GValidityCheckT<GOptimizableEntity>::load_(cp);

   // no local data
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
