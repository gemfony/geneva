/**
 * @file GParameterSetMultiConstraint.hpp
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

#include "geneva/GParameterSetMultiConstraint.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSetFormulaConstraint)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GParameterSetConstraint::GParameterSetConstraint()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GParameterSetConstraint::GParameterSetConstraint(const GParameterSetConstraint& cp)
   : GPreEvaluationValidityCheckT<GOptimizableEntity>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSetConstraint::~GParameterSetConstraint()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GParameterSetConstraint& GParameterSetConstraint::operator=(const GParameterSetConstraint& cp)
{
   GPreEvaluationValidityCheckT<GOptimizableEntity>::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GIndividualConstraint object
 */
bool GParameterSetConstraint::operator==(const GParameterSetConstraint& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterSetConstraint::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GIndividualConstraint object
 */
bool GParameterSetConstraint::operator!=(const GParameterSetConstraint& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterSetConstraint::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GParameterSetConstraint::checkRelationshipWith(
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
   const GParameterSetConstraint *p_load = GObject::gobject_conversion<GParameterSetConstraint>(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GPreEvaluationValidityCheckT<GOptimizableEntity>::checkRelationshipWith(cp, e, limit, "GParameterSetConstraint", y_name, withMessages));

   // no local data

   return evaluateDiscrepancies("GParameterSetConstraint", caller, deviations, e);
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GParameterSetConstraint::addConfigurationOptions(
   Gem::Common::GParserBuilder& gpb
   , const bool& showOrigin
) {
   // Call our parent class'es function
   GPreEvaluationValidityCheckT<GOptimizableEntity>::addConfigurationOptions(gpb, showOrigin);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GParameterSetConstraint::check_(
   const GOptimizableEntity * p_raw
) const {
   const GParameterSet * p = Gem::Common::convertSimplePointer<GOptimizableEntity, GParameterSet>(p_raw);
   return this->check_(p);
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetConstraint
 */
void GParameterSetConstraint::load_(const GObject* cp) {
   // Check that we are indeed dealing with an object of the same type and that we are not
   // accidently trying to compare this object with itself.
   const GParameterSetConstraint *p_load = GObject::gobject_conversion<GParameterSetConstraint>(cp);

   // Load our parent class'es data ...
   GPreEvaluationValidityCheckT<GOptimizableEntity>::load_(cp);

   // no local data
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor -- private, only needed for (de-)serialization
 */
GParameterSetFormulaConstraint::GParameterSetFormulaConstraint()
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor that accepts a formula in string form as its argument
 */
GParameterSetFormulaConstraint::GParameterSetFormulaConstraint(std::string rawFormula)
   : rawFormula_(rawFormula)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GParameterSetFormulaConstraint::GParameterSetFormulaConstraint(const GParameterSetFormulaConstraint& cp)
   : GParameterSetConstraint(cp)
   , rawFormula_(cp.rawFormula_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSetFormulaConstraint::~GParameterSetFormulaConstraint()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GParameterSetFormulaConstraint& GParameterSetFormulaConstraint::operator=(const GParameterSetFormulaConstraint& cp)
{
   GPreEvaluationValidityCheckT<GOptimizableEntity>::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GIndividualConstraint object
 */
bool GParameterSetFormulaConstraint::operator==(const GParameterSetFormulaConstraint& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterSetFormulaConstraint::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GIndividualConstraint object
 */
bool GParameterSetFormulaConstraint::operator!=(const GParameterSetFormulaConstraint& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterSetFormulaConstraint::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GParameterSetFormulaConstraint::checkRelationshipWith(
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
   const GParameterSetFormulaConstraint *p_load = GObject::gobject_conversion<GParameterSetFormulaConstraint>(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GParameterSetConstraint::checkRelationshipWith(cp, e, limit, "GParameterSetFormulaConstraint", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GParameterSetFormulaConstraint", rawFormula_, p_load->rawFormula_, "rawFormula_", "p_load->rawFormula_", e , limit));

   return evaluateDiscrepancies("GParameterSetFormulaConstraint", caller, deviations, e);
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GParameterSetFormulaConstraint::addConfigurationOptions(
   Gem::Common::GParserBuilder& gpb
   , const bool& showOrigin
) {
   // Call our parent class'es function
   GParameterSetConstraint::addConfigurationOptions(gpb, showOrigin);
}

/******************************************************************************/
/**
 * This function extracts all double parameter values including their names from the GParameterSet
 * objects. It then initiates replacement of parameter values in the formula string and
 * parsing of the string. If a math error occurs inside of the formula (such as division by 0),
 * the worst possible value will be returned (MAX_DOUBLE, taken from a Boost function).
 *
 * TODO: Make this work for all parameter types
 */
double GParameterSetFormulaConstraint::check_(
   const GParameterSet * p
) const {
   std::map<std::string, std::vector<double> > parameterValues;
   p->streamline(parameterValues); // Extract the parameter values including names
   Gem::Common::GFormulaParserT<double> f(rawFormula_); // Create the parser

   try {
      return f(parameterValues); // Parse the formula. This may throw a Gem::Common::math_logic_error
   } catch(Gem::Common::math_logic_error& m) {
      glogger
      << "In GParameterSetFormulaConstraint::check_(): WARNING" << std::endl
      << "Caught Gem::Common::math_logic_error with message" << std::endl
      << m.what() << std::endl
      << "We will return MAX_DOUBLE" << std::endl
      << GWARNING;

      return boost::numeric::bounds<double>::highest();
   }
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetFormulaConstraint
 */
void GParameterSetFormulaConstraint::load_(const GObject* cp) {
   // Check that we are indeed dealing with an object of the same type and that we are not
   // accidently trying to compare this object with itself.
   const GParameterSetFormulaConstraint *p_load = GObject::gobject_conversion<GParameterSetFormulaConstraint>(cp);

   // Load our parent class'es data ...
   GPreEvaluationValidityCheckT<GOptimizableEntity>::load_(cp);

   // ... and then our local data
   rawFormula_ = p_load->rawFormula_;
}

/******************************************************************************/
/**
 * Returns a deep clone of this object
 */
GObject* GParameterSetFormulaConstraint::clone_() const {
   return new GParameterSetFormulaConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
