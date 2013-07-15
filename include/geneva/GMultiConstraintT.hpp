/**
 * @file GMultiConstraintT.hpp
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
#include <boost/mpl/assert.hpp>

#ifndef GMULTICONSTRAINTT_HPP_
#define GMULTICONSTRAINTT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GPODExpectationChecksT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

namespace Gem {
namespace Geneva {

// Forward declaration
class GOptimizableEntity;

/******************************************************************************/
/**
 * This is the base class of a hierarchy of classes dealing with inter-parameter
 * constraints. Objects representing the template parameter are evaluated for their
 * validity.
 */
template <typename ind_type>
class GValidityCheckT : public GObject
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject);
   }
   ///////////////////////////////////////////////////////////////////////

   // We only accept validity checks for types derived directly or indirectly from GOptimizableEntity
   BOOST_MPL_ASSERT((boost::is_base_of<Gem::Geneva::GOptimizableEntity , ind_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GValidityCheckT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GValidityCheckT(const GValidityCheckT<ind_type>& cp)
      : GObject(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GValidityCheckT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GValidityCheckT<ind_type>& operator=(const GValidityCheckT<ind_type>& cp)  {
      GValidityCheckT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GIndividualConstraint object
    */
   bool operator==(const GValidityCheckT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GValidityCheckT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GIndividualConstraint object
    */
   bool operator!=(const GValidityCheckT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GValidityCheckT<ind_type>::operator!=","cp", CE_SILENT);
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
      const GValidityCheckT<ind_type> *p_load = GObject::gobject_conversion<GValidityCheckT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GValidityCheckT<ind_type>", y_name, withMessages));

      // no local data

      return evaluateDiscrepancies("GValidityCheckT<ind_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    */
   virtual void addConfigurationOptions(
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) OVERRIDE {
      // Call our parent class'es function
      GObject::addConfigurationOptions(gpb, showOrigin);
   }

   /***************************************************************************/
   /**
    * Checks whether a given parameter set is valid. The function returns a
    * double value which is expected to be in the range of [0,1], giving a
    * level of confidence that this is a valid solution.
    */
   double check(
      const ind_type *cp
      , const double& validityThreshold
   ) const {
      double result = check_(cp, validityThreshold);

#ifdef DEBUG
      if(result < 0. || result > 1.) {
         glogger
         << "In GValidityCheckT<>::check(): Error!" << std::endl
         << "Validity check yielded a result outside of the allowed value range [0:1]: " << result << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

      return result;
   }

protected:
   /***************************************************************************/
   /**
    * Checks whether a given parameter set is valid. The function returns a
    * double value which is expected to be in the range of [0,1], giving a
    * level of confidence that this is a valid solution. This function must
    * be overloaded in derived classes.
    */
   virtual double check_(const ind_type *, const double&) const = 0;

   /***************************************************************************/
   /**
    * Loads the data of another GValidityCheckT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GValidityCheckT<ind_type> *p_load = GObject::gobject_conversion<GValidityCheckT<ind_type> >(cp);

      // Load our parent class'es data ...
      GObject::load_(cp);

      // no local data
   }

   /***************************************************************************/
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const = 0;

   /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class takes a GValidityCheckT<ind_type> and inverts its verdict. It
 * acts like a negation operator.
 */
template <typename ind_type>
class GInvalidityCheckT : public GValidityCheckT<ind_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GValidityCheckT<ind_type>);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * Construction with a pointer to a validity check
    */
   GInvalidityCheckT(boost::shared_ptr<GValidityCheckT<ind_type> > vc_ptr) {
      copyGenevaSmartPointer(vc_ptr, vc_ptr_);
   }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GInvalidityCheckT(const GInvalidityCheckT<ind_type>& cp) : GValidityCheckT<ind_type>(cp) {
      copyGenevaSmartPointer(cp.vc_ptr_, vc_ptr_);
   }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GInvalidityCheckT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GInvalidityCheckT<ind_type>& operator=(const GInvalidityCheckT<ind_type>& cp)  {
      GInvalidityCheckT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GIndividualConstraint object
    */
   bool operator==(const GInvalidityCheckT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInvalidityCheckT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GIndividualConstraint object
    */
   bool operator!=(const GInvalidityCheckT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInvalidityCheckT<ind_type>::operator!=","cp", CE_SILENT);
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
      const GInvalidityCheckT<ind_type> *p_load = GObject::gobject_conversion<GInvalidityCheckT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GValidityCheckT<ind_type>::checkRelationshipWith(cp, e, limit, "GInvalidityCheckT<ind_type>", y_name, withMessages));

      // ... and then our local data
      deviations.push_back(checkExpectation(withMessages, "GInvalidityCheckT<ind_type>", vc_ptr_, p_load->vc_ptr_, "vc_ptr_", "p_load->vc_ptr_", e , limit));

      return evaluateDiscrepancies("GInvalidityCheckT<ind_type>", caller, deviations, e);
   }

protected:
   /***************************************************************************/
   /**
    * Check whether a given parameter set is valid
    */
   virtual double check_(
      const ind_type * cp
      , const double& validityThreshold
   ) const OVERRIDE {
      return !vc_ptr_->check(cp, validityThreshold);
   }

   /***************************************************************************/
   /**
    * Loads the data of another GValidityCheckT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GInvalidityCheckT<ind_type> *p_load = GObject::gobject_conversion<GInvalidityCheckT<ind_type> >(cp);

      // Load our parent class'es data ...
      GValidityCheckT<ind_type>::load_(cp);

      // and then our local data
      copyGenevaSmartPointer(p_load->vc_ptr_, vc_ptr_);
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    */
   virtual GObject* clone_() const OVERRIDE {
      return new GInvalidityCheckT<ind_type>(*this);
   }

private:
   /***************************************************************************/
   /** @brief The default constructor. Intentionally private -- only needed for de-serialization */
   GInvalidityCheckT() { /* nothing */ }

   boost::shared_ptr<GValidityCheckT<ind_type> > vc_ptr_; ///< Holds the validity check whose verdict should be inverted
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A abstract class capable of combining several GValidityCheckT<ind_type>-derivatives.
 * This class just holds the logic needed to add validity checks. Ways of
 * combining the checks (such as logical AND, OR and XOR) need to be specified
 * in derived classes.
 */
template <typename ind_type>
class GValidityCheckCombinerT : public GValidityCheckT<ind_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GValidityCheckT<ind_type>);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GValidityCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization from a vector of validity checks
    */
   GValidityCheckCombinerT(const std::vector<boost::shared_ptr<GValidityCheckT<ind_type> > >& validityChecks)
   {
      copyGenevaSmartPointerVector(validityChecks, validityChecks_);
   }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GValidityCheckCombinerT(const GValidityCheckCombinerT<ind_type>& cp)
      : GValidityCheckT<ind_type>(cp)
   {
      copyGenevaSmartPointerVector(cp.validityChecks_, validityChecks_);
   }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GValidityCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GValidityCheckCombinerT<ind_type>& operator=(const GValidityCheckCombinerT<ind_type>& cp)  {
      GValidityCheckCombinerT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GValidityCheckCombinerT object
    */
   bool operator==(const GValidityCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GValidityCheckCombinerT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GValidityCheckCombinerT object
    */
   bool operator!=(const GValidityCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GValidityCheckCombinerT<ind_type>::operator!=","cp", CE_SILENT);
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
      const GValidityCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GValidityCheckCombinerT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GValidityCheckT<ind_type>::checkRelationshipWith(cp, e, limit, "GValidityCheckCombinerT<ind_type>", y_name, withMessages));

      // ... and then our local data
      deviations.push_back(checkExpectation(withMessages, "GValidityCheckCombinerT<ind_type>", validityChecks_, p_load->validityChecks_, "validityChecks_", "p_load->validityChecks_", e , limit));

      return evaluateDiscrepancies("GValidityCheckCombinerT<ind_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Adds a validity check to this object. Note that we clone the check so
    * that it can be used multiple times.
    */
   void addCheck(boost::shared_ptr<GValidityCheckT<ind_type> > vc_ptr) {
      if(!vc_ptr) {
         glogger
         << "In GValidityCheckCombinerT<>::addCheck(): Error!" << std::endl
         << "Got empty check pointer" << std::endl
         << GEXCEPTION;
      }

      validityChecks_.push_back(vc_ptr->GObject::clone<GValidityCheckT<ind_type> >());
   }

protected:
   /***************************************************************************/
   /** @brief Check whether a given parameter set is valid. To be specified in derived classes */
   virtual double check_(const ind_type *, const double&) const = 0;

   /***************************************************************************/
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const = 0;

   /***************************************************************************/
   /**
    * Loads the data of another GValidityCheckT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GValidityCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GValidityCheckCombinerT<ind_type> >(cp);

      // Load our parent class'es data ...
      GValidityCheckT<ind_type>::load_(cp);

      // and then our local data
      copyGenevaSmartPointerVector(p_load->validityChecks_, validityChecks_);
   }

   /***************************************************************************/
   /** @brief Holds all registered validity checks */
   std::vector<boost::shared_ptr<GValidityCheckT<ind_type> > > validityChecks_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class applies a logical "AND" to all checks contained in the vector. In
 * other words, the check will return true if all checks in the check vector return
 * true.
 */
template <typename ind_type>
class GANDCheckCombinerT : public GValidityCheckCombinerT<ind_type> {
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GValidityCheckCombinerT<ind_type>);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GANDCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization from a vector of validity checks
    */
   GANDCheckCombinerT(const std::vector<boost::shared_ptr<GValidityCheckCombinerT<ind_type> > >& validityChecks)
      : GValidityCheckCombinerT<ind_type>(validityChecks)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GANDCheckCombinerT(const GANDCheckCombinerT<ind_type>& cp)
      : GValidityCheckCombinerT<ind_type>(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GANDCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GANDCheckCombinerT<ind_type>& operator=(const GANDCheckCombinerT<ind_type>& cp)  {
      GANDCheckCombinerT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GANDCheckCombinerT object
    */
   bool operator==(const GANDCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GANDCheckCombinerT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GANDCheckCombinerT object
    */
   bool operator!=(const GANDCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GANDCheckCombinerT<ind_type>::operator!=","cp", CE_SILENT);
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
      const GANDCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GANDCheckCombinerT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GValidityCheckCombinerT<ind_type>::checkRelationshipWith(cp, e, limit, "GANDCheckCombinerT<ind_type>", y_name, withMessages));

      // ... no local data

      return evaluateDiscrepancies("GANDCheckCombinerT<ind_type>", caller, deviations, e);
   }

protected:
   /***************************************************************************/
   /**
    * Check whether all checks in the check vector return true. If so, it
    * returns 1.0, otherwise 0.0
    */
   virtual double check_(
         const ind_type *p
         , const double& validityThreshold
   ) const OVERRIDE {
      double result = 1.0;

      typename std::vector<boost::shared_ptr<GValidityCheckT<ind_type> > >::const_iterator cit;
      for(cit=GValidityCheckCombinerT<ind_type>::validityChecks_.begin(); cit!=GValidityCheckCombinerT<ind_type>::validityChecks_.end(); ++cit) {
         if((*cit)->GValidityCheckT<ind_type>::check(p, validityThreshold) < validityThreshold) {
            result = 0.0;
            break;
         }
      }

      return result;
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    */
   virtual GObject* clone_() const OVERRIDE {
      return new GANDCheckCombinerT<ind_type>(*this);
   }

   /***************************************************************************/
   /**
    * Loads the data of another GValidityCheckCombinerT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GANDCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GANDCheckCombinerT<ind_type> >(cp);

      // Load our parent class'es data ...
      GValidityCheckCombinerT<ind_type>::load_(cp);

      // no local data
   }

   /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class applies a logical "OR" to all checks contained in the vector. In
 * other words, the check will return true if at least one check in the check vector
 * returns true.
 */
template <typename ind_type>
class GORCheckCombinerT : public GValidityCheckCombinerT<ind_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GValidityCheckCombinerT<ind_type>);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GORCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization from a vector of validity checks
    */
   GORCheckCombinerT(const std::vector<boost::shared_ptr<GValidityCheckCombinerT<ind_type> > >& validityChecks)
      : GValidityCheckCombinerT<ind_type>(validityChecks)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GORCheckCombinerT(const GORCheckCombinerT<ind_type>& cp)
      : GValidityCheckCombinerT<ind_type>(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GORCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GORCheckCombinerT<ind_type>& operator=(const GORCheckCombinerT<ind_type>& cp)  {
      GORCheckCombinerT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GORCheckCombinerT object
    */
   bool operator==(const GORCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GORCheckCombinerT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GORCheckCombinerT object
    */
   bool operator!=(const GORCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GORCheckCombinerT<ind_type>::operator!=","cp", CE_SILENT);
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
      const GORCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GORCheckCombinerT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GValidityCheckCombinerT<ind_type>::checkRelationshipWith(cp, e, limit, "GORCheckCombinerT<ind_type>", y_name, withMessages));

      // ... no local data

      return evaluateDiscrepancies("GORCheckCombinerT<ind_type>", caller, deviations, e);
   }

protected:
   /***************************************************************************/
   /**
    * Check whether at least one check in the check vector returns true. If so,
    * it returns 1.0, otherwise 0.0
    */
   virtual double check_(
      const ind_type *p
      , const double& validityThreshold
   ) const OVERRIDE {
      double result = 0.0;

      typename std::vector<boost::shared_ptr<GValidityCheckT<ind_type> > >::const_iterator cit;
      for(cit=GValidityCheckCombinerT<ind_type>::validityChecks_.begin(); cit!=GValidityCheckCombinerT<ind_type>::validityChecks_.end(); ++cit) {
         if((*cit)->GValidityCheckT<ind_type>::check(p, validityThreshold) >= validityThreshold) {
            result = 1.0;
            break;
         }
      }

      return result;
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    */
   virtual GObject* clone_() const OVERRIDE {
      return new GORCheckCombinerT<ind_type>(*this);
   }

   /***************************************************************************/
   /**
    * Loads the data of another GValidityCheckCombinerT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GORCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GORCheckCombinerT<ind_type> >(cp);

      // Load our parent class'es data ...
      GValidityCheckCombinerT<ind_type>::load_(cp);

      // no local data
   }

   /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class applies a logical "XOR" to all checks contained in the vector. In
 * other words, the check will return true if exactly one check in the check vector
 * returns true
 */
template <typename ind_type>
class GXORCheckCombinerT : public GValidityCheckCombinerT<ind_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;
     ar
     & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GValidityCheckCombinerT<ind_type>);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GXORCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization from a vector of validity checks
    */
   GXORCheckCombinerT(const std::vector<boost::shared_ptr<GValidityCheckCombinerT<ind_type> > >& validityChecks)
      : GValidityCheckCombinerT<ind_type>(validityChecks)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GXORCheckCombinerT(const GXORCheckCombinerT<ind_type>& cp)
      : GValidityCheckCombinerT<ind_type>(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GXORCheckCombinerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator
    */
   const GXORCheckCombinerT<ind_type>& operator=(const GXORCheckCombinerT<ind_type>& cp)  {
      GXORCheckCombinerT<ind_type>::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GXORCheckCombinerT object
    */
   bool operator==(const GXORCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GXORCheckCombinerT<ind_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GXORCheckCombinerT object
    */
   bool operator!=(const GXORCheckCombinerT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GXORCheckCombinerT<ind_type>::operator!=","cp", CE_SILENT);
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
      const GXORCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GXORCheckCombinerT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GValidityCheckCombinerT<ind_type>::checkRelationshipWith(cp, e, limit, "GXORCheckCombinerT<ind_type>", y_name, withMessages));

      // ... no local data

      return evaluateDiscrepancies("GXORCheckCombinerT<ind_type>", caller, deviations, e);
   }

protected:
   /***************************************************************************/
   /**
    * Check whether exactly one check in the check vector returns true. If so,
    * it returns 1.0, otherwise 0.0
    */
   virtual double check_(
      const ind_type *p
      , const double& validityThreshold
   ) const OVERRIDE {
      std::size_t nTrue = 0;
      typename std::vector<boost::shared_ptr<GValidityCheckT<ind_type> > >::const_iterator cit;
      for(cit=GValidityCheckCombinerT<ind_type>::validityChecks_.begin(); cit!=GValidityCheckCombinerT<ind_type>::validityChecks_.end(); ++cit) {
         if((*cit)->GValidityCheckT<ind_type>::check(p, validityThreshold) >= validityThreshold) {
            nTrue++;
         }
      }

      if(nTrue != 1) {
         return 0.;
      } else {
         return 1.;
      }
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    */
   virtual GObject* clone_() const OVERRIDE {
      return new GXORCheckCombinerT<ind_type>(*this);
   }

   /***************************************************************************/
   /**
    * Loads the data of another GValidityCheckCombinerT<ind_type>
    */
   virtual void load_(const GObject* cp) OVERRIDE {
      // Check that we are indeed dealing with an object of the same type and that we are not
      // accidently trying to compare this object with itself.
      const GXORCheckCombinerT<ind_type> *p_load = GObject::gobject_conversion<GXORCheckCombinerT<ind_type> >(cp);

      // Load our parent class'es data ...
      GValidityCheckCombinerT<ind_type>::load_(cp);

      // no local data
   }

   /***************************************************************************/
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
      struct is_abstract< Gem::Geneva::GValidityCheckT<ind_type> > : public boost::true_type {};
      template<typename ind_type>
      struct is_abstract< const Gem::Geneva::GValidityCheckT<ind_type> > : public boost::true_type {};
   }
}

/******************************************************************************/

#endif /* GMULTICONSTRAINTT_HPP_ */
