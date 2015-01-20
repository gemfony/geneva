/**
 * @file GMultiConstraintT.hpp
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


// Standard header files go here

// Boost header files go here
#include <boost/mpl/assert.hpp>

#ifndef GMULTICONSTRAINTT_HPP_
#define GMULTICONSTRAINTT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GPODExpectationChecksT.hpp"
#include "common/GMathHelperFunctions.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

namespace Gem {
namespace Geneva {

// Forward declaration
class GOptimizableEntity;

/******************************************************************************/
/**
 * This is the base class of a hierarchy of classes dealing with inter-parameter
 * constraints. Objects representing the template parameter are evaluated for their
 * validity. Note that the classes in this hierarchy are meant to be used PRIOR
 * to the evaluation.
 */
template <typename ind_type>
class GPreEvaluationValidityCheckT : public GObject
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
     & BOOST_SERIALIZATION_NVP(allowNegative_);
   }
   ///////////////////////////////////////////////////////////////////////

   // We only accept validity checks for types derived directly or indirectly from GOptimizableEntity
   BOOST_MPL_ASSERT((boost::is_base_of<Gem::Geneva::GOptimizableEntity , ind_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GPreEvaluationValidityCheckT()
      : allowNegative_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GPreEvaluationValidityCheckT(const GPreEvaluationValidityCheckT<ind_type>& cp)
      : GObject(cp)
      , allowNegative_(cp.allowNegative_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GPreEvaluationValidityCheckT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GPreEvaluationValidityCheckT<ind_type>& operator=(const GPreEvaluationValidityCheckT<ind_type>& cp)  {
      GPreEvaluationValidityCheckT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GIndividualConstraint object
    */
   bool operator==(const GPreEvaluationValidityCheckT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GPreEvaluationValidityCheckT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GIndividualConstraint object
    */
   bool operator!=(const GPreEvaluationValidityCheckT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GPreEvaluationValidityCheckT<ind_type>::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
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
   virtual boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const OVERRIDE {
      using namespace Gem::Common;

      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GPreEvaluationValidityCheckT<ind_type> *p_load = GObject::gobject_conversion<GPreEvaluationValidityCheckT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GPreEvaluationValidityCheckT<ind_type>", y_name, withMessages));

      // ... and then our local data
      deviations.push_back(checkExpectation(withMessages, "GPreEvaluationValidityCheckT<ind_type>", allowNegative_, p_load->allowNegative_, "allowNegative_", "p_load->allowNegative_", e , limit));

      return evaluateDiscrepancies("GPreEvaluationValidityCheckT<ind_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * TODO: Check whether it makes sense to provide custom configuration files -- if so, add allowNegative_ here
    */
   virtual void addConfigurationOptions(
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE {
      // Call our parent class'es function
      GObject::addConfigurationOptions(gpb);
   }

   /***************************************************************************/
   /**
    * Checks whether a given parameter set is valid. The function returns a
    * double value which is expected to be >= 0. Values in the range [0,1]
    * indicate valid parameters (according to this constraint). Values above 1
    * indicate invalid parameters. The size of the return value can thus
    * be used to indicate the extent of the invalidity. Two policies are implemented
    * when check_() returns a value < 0: If allowNevative is set to true, such
    * evaluations are considered to be valid, and the function returns 0. If
    * allowNegative is set to false, am invalidity is calculated, and the return-
    * value will be > 1.
    */
   double check(
      const ind_type *cp
   ) const {
      double result = check_(cp);

      if(allowNegative_) {
         if(result <= 1.) { // valid
            return 0.;
         } else {
            return result;
         }
      } else {
         if(result >= 0. && result <= 1.) { // valid
            return 0.;
         } else { // invalid
            if(result < 0.) { // we need to calculate a replacement value
               // Will be the more invalid the further below 0 "result" is
               return 1. + Gem::Common::gfabs(result);
            } else { // result > 1, we may just return the unmodified value
               return result;
            }
         }
      }

      glogger
      << "In GPreEvaluationValidityCheckT<ind_type>::check(): Error!" << std::endl
      << "Error: This location should never be reached" << std::endl
      << GEXCEPTION;

      // Make the compiler happy -- we return MAX_DOUBLE.
      // Note that this line should never be reached.
      return boost::numeric::bounds<double>::highest();
   }

   /***************************************************************************/
   /**
    * Checks whether the constraint is valid
    *
    * @param cp A pointer to the individual to be checked
    * @param validityLevel Will be filled with the validity level of this individual
    * @return A boolean indicating whether a constraint is valid
    */
   bool isValid(const ind_type *cp, double &validityLevel) const {
      // Set the external validity level
      validityLevel = this->check(cp);

      if(boost::numeric::bounds<double>::highest() == validityLevel || boost::numeric::bounds<double>::lowest() == validityLevel) {
         return false;
      }

      if(allowNegative_) {
         return (validityLevel <= 1.);
      } else {
         return (validityLevel >= 0. && validityLevel <= 1.);
      }
   }

   /***************************************************************************/
   /**
    * Checks whether a constraint it invalid
    *
    * @param cp A pointer to the individual to be checked
    * @param validityLevel Will be filled with the validity level of this individual
    * @return A boolean indicating whether a constraint is invalid
    */
   bool isInvalid(const ind_type *cp, double &validityLevel) const {
      return !this->isValid(cp, validityLevel);
   }

   /***************************************************************************/
   /**
    * Allows to specify whether negative values are considered to be valid
    */
   bool getAllowNegative() const {
      return allowNegative_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether negative values are considered to be valid
    */
   void setAllowNegative(bool allowNegative) {
      allowNegative_ = allowNegative;
   }

protected:
   /***************************************************************************/
   /**
    * Checks whether a given parameter set is valid. The function returns a
    * double value which is expected to be >= 0., giving a level of confidence
    * that this is a valid solution. This function must be overloaded in
    * derived classes.
    */
   virtual double check_(const ind_type *) const = 0;

   /***************************************************************************/
   /**
    * Loads the data of another GPreEvaluationValidityCheckT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GPreEvaluationValidityCheckT<ind_type> *p_load = GObject::gobject_conversion<GPreEvaluationValidityCheckT<ind_type> >(cp);

      // Load our parent class'es data ...
      GObject::load_(cp);

      // ... and then our local data
      allowNegative_ = p_load->allowNegative_;
   }

   /***************************************************************************/
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const = 0;

   /***************************************************************************/

private:
   bool allowNegative_; ///< Set to true if negative values are considered to be valid
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An collection of validity checks with the GPreEvaluationValidityCheckT interface
 */
template <typename ind_type>
class GValidityCheckContainerT : public GPreEvaluationValidityCheckT<ind_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPreEvaluationValidityCheckT<ind_type>);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GValidityCheckContainerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization from a vector of validity checks
    */
   GValidityCheckContainerT(const std::vector<boost::shared_ptr<GPreEvaluationValidityCheckT<ind_type> > >& validityChecks)
   {
      copyGenevaSmartPointerVector(validityChecks, validityChecks_);
   }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GValidityCheckContainerT(const GValidityCheckContainerT<ind_type>& cp)
      : GPreEvaluationValidityCheckT<ind_type>(cp)
   {
      copyGenevaSmartPointerVector(cp.validityChecks_, validityChecks_);
   }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GValidityCheckContainerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GValidityCheckContainerT<ind_type>& operator=(const GValidityCheckContainerT<ind_type>& cp)  {
      GValidityCheckContainerT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GValidityCheckContainerT object
    */
   bool operator==(const GValidityCheckContainerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GValidityCheckContainerT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GValidityCheckContainerT object
    */
   bool operator!=(const GValidityCheckContainerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GValidityCheckContainerT<ind_type>::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
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
   virtual boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const OVERRIDE {
      using namespace Gem::Common;

      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GValidityCheckContainerT<ind_type> *p_load = GObject::gobject_conversion<GValidityCheckContainerT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GPreEvaluationValidityCheckT<ind_type>::checkRelationshipWith(cp, e, limit, "GValidityCheckContainerT<ind_type>", y_name, withMessages));

      // ... and then our local data
      deviations.push_back(checkExpectation(withMessages, "GValidityCheckContainerT<ind_type>", validityChecks_, p_load->validityChecks_, "validityChecks_", "p_load->validityChecks_", e , limit));

      return evaluateDiscrepancies("GValidityCheckContainerT<ind_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Adds a validity check to this object. Note that we clone the check so
    * that it can be used multiple times.
    */
   void addCheck(boost::shared_ptr<GPreEvaluationValidityCheckT<ind_type> > vc_ptr) {
      if(!vc_ptr) {
         glogger
         << "In GValidityCheckContainerT<>::addCheck(): Error!" << std::endl
         << "Got empty check pointer" << std::endl
         << GEXCEPTION;
      }

      validityChecks_.push_back(vc_ptr->GObject::template clone<GPreEvaluationValidityCheckT<ind_type> >());
   }

protected:
   /***************************************************************************/
   /** @brief Checks whether a given parameter set is valid. To be specified in derived classes */
   virtual double check_(const ind_type *) const = 0;

   /***************************************************************************/
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const = 0;

   /***************************************************************************/
   /**
    * Loads the data of another GPreEvaluationValidityCheckT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GValidityCheckContainerT<ind_type> *p_load = GObject::gobject_conversion<GValidityCheckContainerT<ind_type> >(cp);

      // Load our parent class'es data ...
      GPreEvaluationValidityCheckT<ind_type>::load_(cp);

      // and then our local data
      copyGenevaSmartPointerVector(p_load->validityChecks_, validityChecks_);
   }

   /***************************************************************************/
   /** @brief Holds all registered validity checks */
   std::vector<boost::shared_ptr<GPreEvaluationValidityCheckT<ind_type> > > validityChecks_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A class which combines all values (i.e. values > 1) according to a
 * user-defined policy or returns 0, if all checks are valid.
 */
template <typename ind_type>
class GCheckCombinerT
   : public GValidityCheckContainerT<ind_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPreEvaluationValidityCheckT<ind_type>)
     & BOOST_SERIALIZATION_NVP(combinerPolicy_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GCheckCombinerT()
      : combinerPolicy_(Gem::Geneva::MULTIPLYINVALID)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization from a vector of validity checks
    */
   GCheckCombinerT(
      const std::vector<boost::shared_ptr<GPreEvaluationValidityCheckT<ind_type> > >& validityChecks
   )
      : GValidityCheckContainerT<ind_type>(validityChecks)
      , combinerPolicy_(Gem::Geneva::MULTIPLYINVALID)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GCheckCombinerT(const GCheckCombinerT<ind_type>& cp)
      : GValidityCheckContainerT<ind_type>(cp)
      , combinerPolicy_(cp.combinerPolicy_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GCheckCombinerT<ind_type>& operator=(const GCheckCombinerT<ind_type>& cp)  {
      GCheckCombinerT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GCheckCombinerT object
    */
   bool operator==(const GCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GCheckCombinerT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GCheckCombinerT object
    */
   bool operator!=(const GCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GCheckCombinerT<ind_type>::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
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
   virtual boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const OVERRIDE {
      using namespace Gem::Common;

      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GCheckCombinerT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GValidityCheckContainerT<ind_type>::checkRelationshipWith(cp, e, limit, "GCheckCombinerT<ind_type>", y_name, withMessages));

      // ... and then our local data
      deviations.push_back(checkExpectation(withMessages, "GCheckCombinerT<ind_type>", combinerPolicy_, p_load->combinerPolicy_, "combinerPolicy_", "p_load->combinerPolicy_", e , limit));

      return evaluateDiscrepancies("GCheckCombinerT<ind_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Allows to set the combiner policy
    */
   void setCombinerPolicy(validityCheckCombinerPolicy combinerPolicy) {
      combinerPolicy_ = combinerPolicy;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the combiner policy
    */
   validityCheckCombinerPolicy getCombinerPolicy() const {
      return combinerPolicy_;
   }

protected:
   /***************************************************************************/
   /**
    * Combines all parameters according to a user-defined policy. Note that we
    * DO have to take care here of a situation where the invalidity equals
    * MIN- or MAX_DOUBLE.
    */
   virtual double check_(const ind_type *cp) const {
      // First identify invalid checks
      std::vector<double> invalidChecks;
      double validityLevel;
      typename std::vector<boost::shared_ptr<GPreEvaluationValidityCheckT<ind_type> > >::const_iterator cit;
      for(cit=GValidityCheckContainerT<ind_type>::validityChecks_.begin(); cit!=GValidityCheckContainerT<ind_type>::validityChecks_.end(); ++cit) {
         if(!(*cit)->isValid(cp, validityLevel)) {
            invalidChecks.push_back(validityLevel);
         }
      }

      // We can leave now, if no invalid checks were found
      if(invalidChecks.empty()) { // All checks were valid
         return 0.;
      }

      // Now act on the invalid tests
      switch(combinerPolicy_) {
         // --------------------------------------------------------------------
         // Multiply all invalidities
         case Gem::Geneva::MULTIPLYINVALID:
         {
            double result = 1.;
            std::vector<double>::const_iterator d_cit;
            for(d_cit=invalidChecks.begin(); d_cit!=invalidChecks.end(); ++d_cit) {
               // If we encounter an invalidity at the numeric boundaries, we simply
               // return MAX_DOUBLE
               if(boost::numeric::bounds<double>::highest() == *d_cit || boost::numeric::bounds<double>::lowest() == *d_cit) {
                  return boost::numeric::bounds<double>::highest();
               }

               result *= *d_cit;
            }
            return result;
         }
            break;

         // --------------------------------------------------------------------
         // Add all invalidities
         case Gem::Geneva::ADDINVALID:
         {
            double result = 0.;
            std::vector<double>::const_iterator d_cit;
            for(d_cit=invalidChecks.begin(); d_cit!=invalidChecks.end(); ++d_cit) {
               // If we encounter an invalidity at the numeric boundaries, we simply
               // return MAX_DOUBLE
               if(boost::numeric::bounds<double>::highest() == *d_cit || boost::numeric::bounds<double>::lowest() == *d_cit) {
                  return boost::numeric::bounds<double>::highest();
               }

               result += *d_cit;
            }
            return result;
         }
         break;

         // --------------------------------------------------------------------
         default:
         {
            glogger
            << "In GCheckCombinerT<ind_type>::check_(): Error!" << std::endl
            << "Got invalid combinerPolicy_ value: " << combinerPolicy_ << std::endl
            << GEXCEPTION;

            // Make the compiler happy -- we return MAX_DOUBLE.
            // Note that this line should never be reached.
            return boost::numeric::bounds<double>::highest();
         }
            break;
         // --------------------------------------------------------------------
      }

      glogger
      << "In GCheckCombinerT<ind_type>::check_(): Error!" << std::endl
      << "Error: This location should never be reached" << std::endl
      << GEXCEPTION;

      // Make the compiler happy -- we return MAX_DOUBLE.
      // Note that this line should never be reached.
      return boost::numeric::bounds<double>::highest();
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    */
   virtual GObject* clone_() const {
      return new GCheckCombinerT<ind_type>(*this);
   }

   /***************************************************************************/
   /**
    * Loads the data of another GPreEvaluationValidityCheckT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GCheckCombinerT<ind_type> >(cp);

      // Load our parent class'es data ...
      GPreEvaluationValidityCheckT<ind_type>::load_(cp);

      // and then our local data
      combinerPolicy_ = p_load->combinerPolicy_;
   }

private:
   /***************************************************************************/
   // Local data

   validityCheckCombinerPolicy combinerPolicy_; ///< Indicates how validity checks should be combined
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost {
   namespace serialization {
      template<typename ind_type>
      struct is_abstract< Gem::Geneva::GPreEvaluationValidityCheckT<ind_type> > : public boost::true_type {};
      template<typename ind_type>
      struct is_abstract< const Gem::Geneva::GPreEvaluationValidityCheckT<ind_type> > : public boost::true_type {};
   }
}

namespace boost {
   namespace serialization {
      template<typename ind_type>
      struct is_abstract< Gem::Geneva::GValidityCheckContainerT<ind_type> > : public boost::true_type {};
      template<typename ind_type>
      struct is_abstract< const Gem::Geneva::GValidityCheckContainerT<ind_type> > : public boost::true_type {};
   }
}

/******************************************************************************/

#endif /* GMULTICONSTRAINTT_HPP_ */
