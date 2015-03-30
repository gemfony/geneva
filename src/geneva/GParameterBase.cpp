/**
 * @file GParameterBase.cpp
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

#include "geneva/GParameterBase.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Adaptions are switched on by default.
 */
GParameterBase::GParameterBase()
	: GObject()
	, GMutableI()
	, adaptionsActive_(true)
	, randomInitializationBlocked_(false)
	, parameterName_(boost::lexical_cast<std::string>(boost::uuids::random_generator()()))
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor.
 *
 * @param cp A copy of another GParameterBase object
 */
GParameterBase::GParameterBase(const GParameterBase& cp)
	: GObject(cp)
	, GMutableI(cp)
	, adaptionsActive_(cp.adaptionsActive_)
	, randomInitializationBlocked_(cp.randomInitializationBlocked_)
	, parameterName_(cp.parameterName_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. No local data, hence nothing to do.
 */
GParameterBase::~GParameterBase()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GParameterBase& GParameterBase::operator=(const GParameterBase& cp) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterBase object, camouflaged as a GObject
 */
void GParameterBase::load_(const GObject* cp){
	// Convert cp into local format
	const GParameterBase *p_load = gobject_conversion<GParameterBase>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Load local data
	adaptionsActive_ = p_load->adaptionsActive_;
	randomInitializationBlocked_ = p_load->randomInitializationBlocked_;
	parameterName_ = p_load->parameterName_;
}

/******************************************************************************/
/**
 * Calls the function that does the actual adaption (which is in turn implemented
 * by derived classes. Will omit adaption if the adaptionsActive_ parameter is set.
 *
 * @return A boolean which indicates whether a modification was indeed made
 */
std::size_t GParameterBase::adapt() {
	if(adaptionsActive_) {
	   return adaptImpl(); // Will determine whether a modification was made
	} else {
	   return false;
	}
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Switches on adaptions for this object
 */
bool GParameterBase::setAdaptionsActive() {
	bool previous = adaptionsActive_;
	adaptionsActive_ = true;
	return previous;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Disables adaptions for this object
 */
bool GParameterBase::setAdaptionsInactive() {
	bool previous = adaptionsActive_;
	adaptionsActive_ = false;
	return previous;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Determines whether adaptions are performed for this object
 *
 * @return A boolean indicating whether adaptions are performed for this object
 */
bool GParameterBase::adaptionsActive() const {
	return adaptionsActive_;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Determines whether adaptions are inactive for this object
 *
 * @return A boolean indicating whether adaptions are inactive for this object
 */
bool GParameterBase::adaptionsInactive() const {
   return !adaptionsActive_;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameterBase object
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterBase::operator==(const GParameterBase& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterBase::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GParameterBase object
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are inequal
 */
bool GParameterBase::operator!=(const GParameterBase& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterBase::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GParameterBase::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GParameterBase *p_load = GObject::gobject_conversion<GParameterBase>(&cp);

	// Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GParameterBase", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", adaptionsActive_, p_load->adaptionsActive_, "adaptionsActive_", "p_load->adaptionsActive_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", randomInitializationBlocked_, p_load->randomInitializationBlocked_, "randomInitializationBlocked_", "p_load->randomInitializationBlocked_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GParameterBase", parameterName_, p_load->parameterName_, "parameterName_", "p_load->parameterName_", e , limit));

	return evaluateDiscrepancies("GParameterBase", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParameterBase::name() const {
   return std::string("GParameterBase");
}

/***********************************************************************************/
/**
 * Allows to assign a name to this parameter
 */
void GParameterBase::setParameterName(const std::string& pn) {
   parameterName_ = pn;
}

/***********************************************************************************/
/** @brief Allows to retrieve the name of this parameter */
std::string GParameterBase::getParameterName() const {
   return parameterName_;
}

/***********************************************************************************/
/**
 * Checks whether this object matches a given activity mode. This helper function
 * saves us from having to repeat this switch statement in all GParmaterBase-derivatives.
 *
 * @param am The desired activity mode (ACTIVEONLY, ALLPARAMETERS or INACTIVEONLY)
 * @return A boolean indicating whether a given GParameterBase-derivative matches the activity mode
 */
bool GParameterBase::amMatch(const activityMode& am) const {
   switch(am) {
      case ACTIVEONLY:
      {
         if(this->adaptionsActive()) {
            return true;
         } else {
            return false;
         }
      }
      break;

      case ALLPARAMETERS:
      {
         return true;
      }
      break;

      case INACTIVEONLY:
      {
         if(this->adaptionsInactive()) {
            return true;
         } else {
            return false;
         }
      }
      break;

      default:
      {
         glogger
         << "In GParameterBase::amMatch(const activityMode& am): Error!" << std::endl
         << "Got invalid activity mode " << am << std::endl
         << GEXCEPTION;
      }
      break;
   }

   glogger
   << "In GParameterBase::amMatch(const activityMode& am): Error!" << std::endl
   << "This line should never be reached" << std::endl
   << GEXCEPTION;

   // Make the compiler happy
   return false;
}

/***********************************************************************************/
/**
 * Returns true on the case of an activity mode mismatch
 */
bool GParameterBase::amMismatch(const activityMode& am) const {
   return !amMatch(am);
}

/******************************************************************************/
/**
 * Checks whether this object matches a given activity mode and is modifiable
 */
bool GParameterBase::modifiableAmMatchOrHandover(const activityMode& am) const {
   return ((this->isLeaf() && this->amMatch(am)) || !this->isLeaf());
}

/***********************************************************************************/
/**
 * Lets the audience know whether this is a leaf or a branch object
 */
bool GParameterBase::isLeaf() const {
   return false;
}

/******************************************************************************/
/**
 * Convenience function so we do not need to always cast  derived classes.
 * See GParameterBaseWithAdaptors::hasAdaptors() for the "real"
 * function.
 */
bool GParameterBase::hasAdaptor() const {
	return false;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter(-collection). This is the public
 * version of this function, which only acts if initialization has not been blocked.
 */
void GParameterBase::randomInit(const activityMode& am) {
	if(
      !randomInitializationBlocked_
      && this->modifiableAmMatchOrHandover(am)
   ) {
	   randomInit_(am);
	}
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * This function needs to be overloaded for parameter collections so that it returns the
 * correct value.
 *
 * @return A boolean indicating whether the GParameterBase-derivative is an individual parameter
 */
bool GParameterBase::isIndividualParameter() const {
	return true;
}

/******************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * As GParameterBase derivates can be either individual parameters or parameter collections,
 * this function just returns the inverse of isIndividualParameter() .
 *
 * @return A boolean indicating whether the GParameterBase-derivative is a collection
 */
bool GParameterBase::isParameterCollection() const {
	return !isIndividualParameter();
}

/******************************************************************************/
/**
 * Attach parameters of type double to the vector. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::floatStreamline(
   std::vector<float>& parVec
   , const activityMode&
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type double to the vector. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::doubleStreamline(
   std::vector<double>& parVec
   , const activityMode&
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type boost::int32_t to the vector. This function does nothing by
 * default. Parameter types based on boost::int32_t need to overload this function and do
 * the actual work.
 */
void GParameterBase::int32Streamline(
   std::vector<boost::int32_t>& parVec
   , const activityMode&
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type bool to the vector. This function does nothing by
 * default. Parameter types based on bool need to overload this function and do
 * the actual work.
 */
void GParameterBase::booleanStreamline(
   std::vector<bool>& parVec
   , const activityMode&
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type double to the map. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::floatStreamline(
   std::map<std::string
   , std::vector<float> >& parVec
   , const activityMode&
) const {
   /* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type double to the map. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::doubleStreamline(
   std::map<std::string
   , std::vector<double> >& parVec
   , const activityMode&
) const {
   /* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type boost::int32_t to the map. This function does nothing by
 * default. Parameter types based on boost::int32_t need to overload this function and do
 * the actual work.
 */
void GParameterBase::int32Streamline(
   std::map<std::string
   , std::vector<boost::int32_t> >& parVec
   , const activityMode&
) const {
   /* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type bool to the map. This function does nothing by
 * default. Parameter types based on bool need to overload this function and do
 * the actual work.
 */
void GParameterBase::booleanStreamline(
   std::map<std::string
   , std::vector<bool> >& parVec
   , const activityMode&
) const {
   /* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors
 *
 * @param lBndVec A vector of lower float parameter boundaries
 * @param uBndVec A vector of upper float parameter boundaries
 */
void GParameterBase::floatBoundaries(
		std::vector<float>& lBndVec
		, std::vector<float>& uBndVec
		, const activityMode&
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GParameterBase::doubleBoundaries(
		std::vector<double>& lBndVec
		, std::vector<double>& uBndVec
		, const activityMode&
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Attach boundaries of type boost::int32_t to the vectors
 *
 * @param lBndVec A vector of lower boost::int32_t parameter boundaries
 * @param uBndVec A vector of upper boost::int32_t parameter boundaries
 */
void GParameterBase::int32Boundaries(
		std::vector<boost::int32_t>& lBndVec
		, std::vector<boost::int32_t>& uBndVec
		, const activityMode&
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
void GParameterBase::booleanBoundaries(
		std::vector<bool>& lBndVec
		, std::vector<bool>& uBndVec
		, const activityMode&
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Count the number of float parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of float parameters in this object
 */
std::size_t GParameterBase::countFloatParameters(
   const activityMode& am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Count the number of double parameters. The actual work needs to be done by
 * derived classes, if they possess double parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of double parameters in this object
 */
std::size_t GParameterBase::countDoubleParameters(
   const activityMode& am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Count the number of boost::int32_t parameters. The actual work needs to be done by
 * derived classes, if they possess boost::int32_t parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of boost::int32_t parameters in this object
 */
std::size_t GParameterBase::countInt32Parameters(
   const activityMode& am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Count the number of bool parameters. The actual work needs to be done by
 * derived classes, if they possess bool parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of bool parameters in this object
 */
std::size_t GParameterBase::countBoolParameters(
   const activityMode& am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignFloatValueVector(
   const std::vector<float>& parVec
   , std::size_t& pos
   , const activityMode&
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignDoubleValueVector(
   const std::vector<double>& parVec
   , std::size_t& pos
   , const activityMode&
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignInt32ValueVector(
   const std::vector<boost::int32_t>& parVec
   , std::size_t& pos
   , const activityMode&
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignBooleanValueVector(
   const std::vector<bool>& parVec
   , std::size_t& pos
   , const activityMode&
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignFloatValueVectors(
   const std::map<std::string, std::vector<float> >& parMap
   , const activityMode&
) {
   /* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignDoubleValueVectors(
   const std::map<std::string, std::vector<double> >& parMap
   , const activityMode&
) {
   /* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignInt32ValueVectors(
   const std::map<std::string, std::vector<boost::int32_t> >& parMap
   , const activityMode&
) {
   /* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignBooleanValueVectors(
   const std::map<std::string, std::vector<bool> >& parMap
   , const activityMode&
) {
   /* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::floatMultiplyByRandom(
   const float& min
   , const float& max
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::doubleMultiplyByRandom(
   const double& min
   , const double& max
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::int32MultiplyByRandom(
   const boost::int32_t& min
   , const boost::int32_t& max
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::booleanMultiplyByRandom(
   const bool& min
   , const bool& max
   , const activityMode& am
) {
   // Complain: This function should not be called for boolean values
   glogger
   << "In GParameterBase::booleanMultiplyByRandom(min,max): Error!" << std::endl
   << "This function should not be called for boolean parameters" << std::endl
   << GEXCEPTION;
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::floatMultiplyByRandom(const activityMode& am) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::doubleMultiplyByRandom(const activityMode& am) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::int32MultiplyByRandom(const activityMode& am) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::booleanMultiplyByRandom(const activityMode& am) {
   // Complain: This function should not be called for boolean values
   glogger
   << "In GParameterBase::booleanMultiplyByRandom(): Error!" << std::endl
   << "This function should not be called for boolean parameters" << std::endl
   << GEXCEPTION;
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GParameterBase::floatMultiplyBy(
   const float& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GParameterBase::doubleMultiplyBy(
   const double& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GParameterBase::int32MultiplyBy(
   const boost::int32_t& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::booleanMultiplyBy(
   const bool& value
   , const activityMode& am
) {
   // Complain: This function should not be called for boolean values
   glogger
   << "In GParameterBase::booleanMultiplyBy(): Error!" << std::endl
   << "This function should not be called for boolean parameters" << std::endl
   << GEXCEPTION;
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::floatFixedValueInit(
   const float& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::doubleFixedValueInit(
   const double& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::int32FixedValueInit(
   const boost::int32_t& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::booleanFixedValueInit(
   const bool& value
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::floatAdd(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::doubleAdd(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::int32Add(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::booleanAdd(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   // Complain: This function should not be called for boolean values
   glogger
   << "In GParameterBase::booleanAdd(): Error!" << std::endl
   << "This function should not be called for boolean parameters" << std::endl
   << GEXCEPTION;
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::floatSubtract(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::doubleSubtract(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::int32Subtract(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   /* Do nothing by default */
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::booleanSubtract(
   boost::shared_ptr<GParameterBase>
   , const activityMode& am
) {
   // Complain: This function should not be called for boolean values
   glogger
   << "In GParameterBase::booleanSubtract(): Error!" << std::endl
   << "This function should not be called for boolean parameters" << std::endl
   << GEXCEPTION;
}

/******************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::blockRandomInitialization() {
	randomInitializationBlocked_ = true;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::allowRandomInitialization() {
	randomInitializationBlocked_ = false;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether initialization has been blocked
 */
bool GParameterBase::randomInitializationBlocked() const {
	return randomInitializationBlocked_;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterBase::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterBase::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterBase::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------

	{ // Test blocking and unblocking of random initialization
		using namespace Gem::Common;

		// Create two GParameterBase objects as clone of this object for further usage
		boost::shared_ptr<GParameterBase> p_test1 = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test2 = this->clone<GParameterBase>();

		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK_NO_THROW(p_test2->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == true);
		BOOST_CHECK(p_test2->randomInitializationBlocked() == true);

		// Random initialization should leave the object unchanged
		BOOST_CHECK_NO_THROW(p_test1->randomInit(ALLPARAMETERS));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Unblock random initialization
		BOOST_CHECK_NO_THROW(p_test1->allowRandomInitialization());
		BOOST_CHECK_NO_THROW(p_test2->allowRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == false);
		BOOST_CHECK(p_test2->randomInitializationBlocked() == false);

		// Random initialization should change p_test1
		BOOST_CHECK_NO_THROW(p_test1->randomInit(ALLPARAMETERS));
		BOOST_CHECK(*p_test1 != *p_test2);
	}

	//---------------------------------------------------------------------

	{ // Check activating and de-activating adaptions. Note that the
	  // effects of these flags can only be tested if an adaptor or
	  // multiple adaptors (in the case of object collections) have
	  // been loaded.

		// Create some clones of this object
		boost::shared_ptr<GParameterBase> p_test = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test_1 = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test_2 = this->clone<GParameterBase>();

		// activate adaptions
		BOOST_CHECK_NO_THROW(p_test_1->setAdaptionsActive());
		BOOST_CHECK(p_test_1->adaptionsActive() == true);

		// de-activate adaptions
		BOOST_CHECK_NO_THROW(p_test_2->setAdaptionsInactive());
		BOOST_CHECK(p_test_2->adaptionsActive() == false);

		if(p_test_1->hasAdaptor() && p_test_2->hasAdaptor()) {
			BOOST_CHECK_NO_THROW(p_test_1->adapt()); // Should change
			BOOST_CHECK_NO_THROW(p_test->setAdaptionsActive()); // Make sure differences do not stem from this flag
			BOOST_CHECK(*p_test_1 != *p_test);

			BOOST_CHECK_NO_THROW(p_test_2->adapt()); // Should stay unchanged
			BOOST_CHECK_NO_THROW(p_test->setAdaptionsInactive()); // Make sure differences do not stem from this flag
			BOOST_CHECK(*p_test_2 == *p_test);
		}
	}

	//---------------------------------------------------------------------

	{ // Check adapt() and (de-)activation of adaptions
		// Create two local clones
		boost::shared_ptr<GParameterBase> p_test1 = this->clone<GParameterBase>();
		boost::shared_ptr<GParameterBase> p_test2 = this->clone<GParameterBase>();

		// Always adapt
		BOOST_CHECK_NO_THROW(p_test1->setAdaptionsActive());
		BOOST_CHECK_NO_THROW(p_test2->setAdaptionsActive());

		// Check that adaptions are indeed active in both objects
		BOOST_CHECK(p_test1->adaptionsActive() == true);
		BOOST_CHECK(p_test2->adaptionsActive() == true);

		// Check that we have adaption powers when using the random number generator
		if(p_test1->hasAdaptor()) {
			for(std::size_t i=0; i<100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt());
				BOOST_CHECK(*p_test1 != *p_test2);
				BOOST_CHECK_NO_THROW(p_test1->load(p_test2));
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}

		// De-activate adaptions in both objects
		BOOST_CHECK_NO_THROW(p_test1->setAdaptionsInactive());
		BOOST_CHECK_NO_THROW(p_test2->setAdaptionsInactive());

      // Check that adaptions are indeed inactive in both objects
      BOOST_CHECK(p_test1->adaptionsActive() == false);
      BOOST_CHECK(p_test2->adaptionsActive() == false);

		// Check that adaptions do not occur anymore in p_test1
		if(p_test1->hasAdaptor()) {
			for(std::size_t i=0; i<100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt());
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}
	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterBase::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterBase::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();

	//---------------------------------------------------------------------------

	{ // Some test

	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterBase::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
