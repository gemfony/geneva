/**
 * @file GOptimizableEntity.cpp
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

#include "geneva/GOptimizableEntity.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor.
 */
GOptimizableEntity::GOptimizableEntity()
	: GMutableI()
	, GRateableI()
	, GObject()
	, transformedCurrentFitness_(0.)
   , transformedCurrentSecondaryFitness_()
   , nFitnessCriteria_(1)
   , rawCurrentFitness_(0.)
   , rawCurrentSecondaryFitness_()
	, bestPastPrimaryFitness_(0.)
	, nStalls_(0)
	, dirtyFlag_(true)
	, maximize_(false)
	, assignedIteration_(0)
	, validityLevel_(0.) // Always valid by default
   , evalPolicy_(Gem::Geneva::USESIMPLEEVALUATION)
   , steepness_(Gem::Geneva::FITNESSSIGMOIDSTEEPNESS)
   , barrier_(Gem::Geneva::WORSTALLOWEDVALIDFITNESS)
   , worstKnownValids_()
   , markedAsInvalidExternally_(false)
   , changesAllowedTo_markedAsInvalidExternally_(false)
   , evaluationID_("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor.
 *
 * @param cp A copy of another GOptimizableEntity object
 */
GOptimizableEntity::GOptimizableEntity(const GOptimizableEntity& cp)
	: GMutableI(cp)
	, GRateableI(cp)
	, GObject(cp)
	, transformedCurrentFitness_(cp.transformedCurrentFitness_)
	, transformedCurrentSecondaryFitness_(cp.transformedCurrentSecondaryFitness_)
   , nFitnessCriteria_(cp.nFitnessCriteria_)
   , rawCurrentFitness_(cp.rawCurrentFitness_)
   , rawCurrentSecondaryFitness_(cp.rawCurrentSecondaryFitness_)
	, bestPastPrimaryFitness_(cp.bestPastPrimaryFitness_)
	, nStalls_(cp.nStalls_)
	, dirtyFlag_(cp.dirtyFlag_)
	, maximize_(cp.maximize_)
	, assignedIteration_(cp.assignedIteration_)
	, validityLevel_(cp.validityLevel_)
   , evalPolicy_(cp.evalPolicy_)
   , steepness_(cp.steepness_)
   , barrier_(cp.barrier_)
   , worstKnownValids_(cp.worstKnownValids_)
   , markedAsInvalidExternally_(cp.markedAsInvalidExternally_)
   , changesAllowedTo_markedAsInvalidExternally_(cp.changesAllowedTo_markedAsInvalidExternally_)
   , evaluationID_(cp.evaluationID_)
{
	// Copy the personality pointer over
	copyGenevaSmartPointer(cp.pt_ptr_, pt_ptr_);

	// Make sure any constraints are copied over
	copyGenevaSmartPointer(cp.individualConstraint_, individualConstraint_);
}

/******************************************************************************/
/**
 * The standard destructor.
 */
GOptimizableEntity::~GOptimizableEntity() { /* nothing */ }

/******************************************************************************/
/**
 * Checks for equality with another GOptimizableEntity object
 *
 * @param  cp A constant reference to another GOptimizableEntity object
 * @return A boolean indicating whether both objects are equal
 */
bool GOptimizableEntity::operator==(const GOptimizableEntity& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizableEntity::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GOptimizableEntity object
 *
 * @param  cp A constant reference to another GOptimizableEntity object
 * @return A boolean indicating whether both objects are inequal
 */
bool GOptimizableEntity::operator!=(const GOptimizableEntity& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizableEntity::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GOptimizableEntity::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GOptimizableEntity *p_load = GObject::gobject_conversion<GOptimizableEntity>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GOptimizableEntity", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", transformedCurrentFitness_, p_load->transformedCurrentFitness_, "transformedCurrentFitness_", "p_load->transformedCurrentFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", transformedCurrentSecondaryFitness_, p_load->transformedCurrentSecondaryFitness_, "transformedCurrentSecondaryFitness_", "p_load->transformedCurrentSecondaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", nFitnessCriteria_, p_load->nFitnessCriteria_, "nFitnessCriteria_", "p_load->nFitnessCriteria_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", rawCurrentFitness_, p_load->rawCurrentFitness_, "rawCurrentFitness_", "p_load->rawCurrentFitness_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", rawCurrentSecondaryFitness_, p_load->rawCurrentSecondaryFitness_, "rawCurrentSecondaryFitness_", "p_load->rawCurrentSecondaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", bestPastPrimaryFitness_, p_load->bestPastPrimaryFitness_, "bestPastPrimaryFitness_", "p_load->bestPastPrimaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", nStalls_, p_load->nStalls_, "nStalls_", "p_load->nStalls_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", dirtyFlag_, p_load->dirtyFlag_, "dirtyFlag_", "p_load->dirtyFlag_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", maximize_, p_load->maximize_, "maximize_", "p_load->maximize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", assignedIteration_, p_load->assignedIteration_, "assignedIteration_", "p_load->assignedIteration_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", validityLevel_, p_load->validityLevel_, "validityLevel_", "p_load->validityLevel_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", evalPolicy_, p_load->evalPolicy_, "evalPolicy_", "p_load->evalPolicy_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", pt_ptr_, p_load->pt_ptr_, "pt_ptr_", "p_load->pt_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", individualConstraint_, p_load->individualConstraint_, "individualConstraint_", "p_load->individualConstraint_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", steepness_, p_load->steepness_, "steepness_", "p_load->steepness_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", barrier_, p_load->barrier_, "barrier_", "p_load->barrier_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", worstKnownValids_, p_load->worstKnownValids_, "worstKnownValids_", "p_load->worstKnownValids_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", markedAsInvalidExternally_, p_load->markedAsInvalidExternally_, "markedAsInvalidExternally_", "p_load->markedAsInvalidExternally_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", changesAllowedTo_markedAsInvalidExternally_, p_load->changesAllowedTo_markedAsInvalidExternally_, "changesAllowedTo_markedAsInvalidExternally_", "p_load->changesAllowedTo_markedAsInvalidExternally_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", evaluationID_, p_load->evaluationID_, "evaluationID_", "p_load->evaluationID_", e , limit));

   return evaluateDiscrepancies("GOptimizableEntity", caller, deviations, e);
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GOptimizableEntity::name() const {
   return std::string("GOptimizableEntity");
}

/******************************************************************************/
/**
 * Allows to register a constraint with this individual. Note that the constraint
 * object will be cloned.
 */
void GOptimizableEntity::registerConstraint(
   boost::shared_ptr<GValidityCheckT<GOptimizableEntity> > c_ptr
) {
   if(!c_ptr) {
      glogger
      << "In GOptimizableEntity::registerConstraint(): Error!" << std::endl
      << "Tried to register empty constraint object" << std::endl
      << GEXCEPTION;
   }

   // We store clones, so individual objects do not share the same object
   individualConstraint_ = c_ptr->GObject::clone<GValidityCheckT<GOptimizableEntity> >();
}

/******************************************************************************/
/**
 * Allows to set the policy to use in case this individual represents an invalid solution
 */
void GOptimizableEntity::setEvaluationPolicy(evaluationPolicy evalPolicy) {
   evalPolicy_ = evalPolicy;
}

/******************************************************************************/
/**
 * Allows to retrieve the current policy in case this individual represents an invalid solution
 */
evaluationPolicy GOptimizableEntity::getEvaluationPolicy() const {
   return evalPolicy_;
}

/******************************************************************************/
/**
 * Loads the data of another GOptimizableEntity, camouflaged as a GObject
 *
 * @param cp A copy of another GOptimizableEntity object, camouflaged as a GObject
 */
void GOptimizableEntity::load_(const GObject* cp) {
	const GOptimizableEntity *p_load = gobject_conversion<GOptimizableEntity>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Then load our local data
	transformedCurrentFitness_ = p_load->transformedCurrentFitness_;
	transformedCurrentSecondaryFitness_ = p_load->transformedCurrentSecondaryFitness_;
	nFitnessCriteria_ = p_load->nFitnessCriteria_;
	rawCurrentFitness_ = p_load->rawCurrentFitness_;
	rawCurrentSecondaryFitness_ = p_load->rawCurrentSecondaryFitness_;
	bestPastPrimaryFitness_ = p_load->bestPastPrimaryFitness_;
	nStalls_ = p_load->nStalls_;
	dirtyFlag_ = p_load->dirtyFlag_;
	maximize_ = p_load->maximize_;
	assignedIteration_ = p_load->assignedIteration_;
	validityLevel_ = p_load->validityLevel_;
	evalPolicy_ = p_load->evalPolicy_;
	steepness_ = p_load->steepness_;
	barrier_ = p_load->barrier_;
	worstKnownValids_ = p_load->worstKnownValids_;
	markedAsInvalidExternally_ = p_load->markedAsInvalidExternally_;
	changesAllowedTo_markedAsInvalidExternally_ = p_load->changesAllowedTo_markedAsInvalidExternally_;
	evaluationID_ = p_load->evaluationID_;

	copyGenevaSmartPointer(p_load->pt_ptr_, pt_ptr_);
   copyGenevaSmartPointer(p_load->individualConstraint_, individualConstraint_);
}

/******************************************************************************/
/**
 * The adaption interface. Triggers adaption of the individual, using each parameter object's adaptor.
 * Sets the dirty flag, as the parameters have been changed.
 */
void GOptimizableEntity::adapt() {
	this->customAdaptions(); // The actual mutation and adaption process
	GOptimizableEntity::setDirtyFlag(); // Make sure the individual is re-evaluated when fitness() is called next time
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Returns the cached result of the fitness function with id 0. This function
 * will always return the transformed fitness. This is the const version.
 */
double GOptimizableEntity::fitness() const {
   return fitness(0, Gem::Geneva::PREVENTREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS);
}

/******************************************************************************/
/**
 * Calculate or returns the result of a fitness function with a given id.
 * This function will always return the transformed fitness. This is the const version.
 */
double GOptimizableEntity::fitness(const std::size_t& id) const {
   return fitness(id, Gem::Geneva::PREVENTREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS);
}

/******************************************************************************/
/**
 * Returns the last known fitness calculations of this object. Re-calculation
 * of the fitness is triggered, unless this is the server mode. By means of supplying
 * an id it is possible to distinguish between different target functions. 0 denotes
 * the main fitness criterion. The user can specify whether he/she is interested
 * in the transformed or the raw fitness value.
 *
 * @param id The id of the fitness criterion
 * @param reevaluationAllowed Explicit permission to re-evaluate the individual
 * @param useTransformedFitness Whether the transformed or the raw fitness should be returned
 * @return The fitness of this individual
 */
double GOptimizableEntity::fitness(
   const std::size_t& id
   , bool reevaluationAllowed
   , bool useTransformedFitness
) {
	// Check whether we need to recalculate the fitness
	if (dirtyFlag_) {
	   if(reevaluationAllowed) {
         this->enforceFitnessUpdate();

#ifdef DEBUG
         // Check if the dirty flag is still set. This should only happen in special cases
         if(dirtyFlag_ && USEWORSTKNOWNVALIDFORINVALID != evalPolicy_) { // evaluation is delayed for USEWORSTKNOWNVALIDFORINVALID
            glogger
            << "In GOptimizableEntity::fitness(...): Error!" << std::endl
            << "Dirty flag is still set in a location where it shouldn't be" << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */
	   } else {
         glogger
         << "In GOptimizableEntity::fitness():" << std::endl
         << "Tried to perform re-evaluation in server-mode" << std::endl
         << GEXCEPTION;
	   }
	}

	// Return the desired result -- there should be no situation where the dirtyFlag is still set
   return getCachedFitness(id, useTransformedFitness);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Returns the last known fitness calculations of this object. This is the
 * const version of the general fitness() function, which consequently cannot
 * trigger re-evaluation, if the individual is dirty. Hence the function will
 * throw, when it is called on a dirty individual (unless we use the
 * USEWORSTKNOWNVALIDFORINVALID policy)
 *
 * @param id The id of the fitness criterion
 * @param reevaluationAllowed Explicit permission to re-evaluate the individual
 * @param useTransformedFitness Whether the transformed or the raw fitness should be returned
 * @return The fitness of this individual
 */
double GOptimizableEntity::fitness(
   const std::size_t& id
   , bool reevaluationAllowed
   , bool useTransformedFitness
) const {
#ifdef DEBUG
   if(dirtyFlag_ && USEWORSTKNOWNVALIDFORINVALID != evalPolicy_) { // evaluation is delayed for USEWORSTKNOWNVALIDFORINVALID
      glogger
      << "In GOptimizableEntity::fitness(...) const: Error!" << std::endl
      << "Dirty flag is still set in a location where it shouldn't be" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // Return the desired result -- there should be no situation where the dirtyFlag is still set
   return getCachedFitness(id, useTransformedFitness);
}

/******************************************************************************/
/**
 * A wrapper for the non-const fitness function, so we can bind to it. It is
 * needed as boost::bind cannot distinguish between the non-const and const
 * overload of the fitness() function.
 */
double GOptimizableEntity::nonConstFitness(
   const std::size_t& id
   , bool reevaluationAllowed
   , bool useTransformedFitness
) {
   return this->fitness(id, reevaluationAllowed, useTransformedFitness);
}

/******************************************************************************/
/**
 * A wrapper for the const fitness function, so we can bind to it. It is
 * needed as boost::bind cannot distinguish between the non-const and const
 * overload of the fitness() function.
 */
double GOptimizableEntity::constFitness(
   const std::size_t& id
   , bool reevaluationAllowed
   , bool useTransformedFitness
) const {
   return this->fitness(id, reevaluationAllowed, useTransformedFitness);
}

/******************************************************************************/
/**
 * Adapts and evaluates the individual in one go
 *
 * @return The main fitness result
 */
double GOptimizableEntity::adaptAndEvaluate() {
	adapt();
	return enforceFitnessUpdate();
}

/******************************************************************************/
/**
 * Retrieve the current (not necessarily up-to-date) fitness
 */
double GOptimizableEntity::getCachedFitness(
   const std::size_t& id
   , const bool& useTransformedFitness
) const {
   if(useTransformedFitness) {
      if(0 == id) {
         return transformedCurrentFitness_;
      } else {
         return transformedCurrentSecondaryFitness_.at(id - 1);
      }
   } else {
      if(0 == id) {
         return rawCurrentFitness_;
      } else {
         return rawCurrentSecondaryFitness_.at(id - 1);
      }
   }
}

/******************************************************************************/
/**
 * Enforces re-calculation of the fitness values.
 *
 * TODO: Check that all secondary fitnesses have been dealt with appropriately
 *
 * @return The main result of the fitness calculation
 */
double GOptimizableEntity::enforceFitnessUpdate() {
   // Assign a new evaluation id
   evaluationID_ = std::string("eval_") + boost::lexical_cast<std::string>(boost::uuids::random_generator()());

   // We start a new evaluation. Marking individuals as invalid
   // happens inside of the user-supplied fitness calculation (if at all)
   // so we want to reset the corresponding "invalid" flag
   markedAsInvalidExternally_ = false;
   // Make it clear that changes to the above flag are allowed
   changesAllowedTo_markedAsInvalidExternally_ = true;

   // Find out, whether this is a valid solution
   if(
      this->parameterSetFulfillsConstraints_(validityLevel_) // Needs to be called first, or else the validityLevel_ will not be filled
      || USESIMPLEEVALUATION==evalPolicy_
   ) {
      // Make sure the secondary fitness vector is empty
      rawCurrentSecondaryFitness_.clear();
      transformedCurrentSecondaryFitness_.clear();

      // Trigger actual fitness calculation using the user-supplied function. This will
      // also register secondary fitness values used in multi-criterion optimization.
      rawCurrentFitness_ = fitnessCalculation();
      transformedCurrentFitness_ = this->getWorstCase();

      if(markedAsInvalidExternally_ || this->allRawResultsAtWorst()) { // Is this an invalid result ?
         // Make sure the secondary fitness vector is empty again
         rawCurrentSecondaryFitness_.clear();
         transformedCurrentSecondaryFitness_.clear();

         rawCurrentFitness_ = this->getWorstCase();
         transformedCurrentFitness_ = this->getWorstCase();
         for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
            rawCurrentSecondaryFitness_.push_back(this->getWorstCase());
            transformedCurrentSecondaryFitness_.push_back(this->getWorstCase());
         }
      } else { // This is a valid solution nonetheless
         // Make a note of the true measurement, in case we want to transform them later
         transformedCurrentFitness_ = rawCurrentFitness_;

#ifdef DEBUG
         // Check that the correct number of secondary evaluation criteria has been registered
         if(rawCurrentSecondaryFitness_.size() != getNumberOfSecondaryFitnessCriteria()) {
            glogger
            << "In GOptimizableEntity::enforceFitnessUpdate(): Error!" << std::endl
            << "Invalid number of secondary fitness values. Got " << rawCurrentSecondaryFitness_.size() << std::endl
            << "but expected " << getNumberOfSecondaryFitnessCriteria() << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */

         // Make a note of the true secondary measurements, in case we want to transform them later
         transformedCurrentSecondaryFitness_ = rawCurrentSecondaryFitness_;

         if(USESIGMOID == evalPolicy_) { // Update the fitness value to use sigmoidal values
            transformedCurrentFitness_ = Gem::Common::gsigmoid(transformedCurrentFitness_, barrier_, steepness_);
            for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
               transformedCurrentSecondaryFitness_.at(i-1) = Gem::Common::gsigmoid(transformedCurrentSecondaryFitness_.at(i-1), barrier_, steepness_);
            }
         }
      }

      // Clear the dirty flag -- all possible evaluation work was done
      setDirtyFlag(false);

   } else { // Some constraints were violated. Act on the chosen policy
      // Clear the secondary arrays so we can fill them as needed
      rawCurrentSecondaryFitness_.clear();
      transformedCurrentSecondaryFitness_.clear();

      if(USEWORSTCASEFORINVALID==evalPolicy_) {
         rawCurrentFitness_ = this->getWorstCase();
         transformedCurrentFitness_ = this->getWorstCase();

         for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
            rawCurrentSecondaryFitness_.push_back(this->getWorstCase());
            transformedCurrentSecondaryFitness_.push_back(this->getWorstCase());
         }

         // Clear the dirty flag -- all possible evaluation work was done
         setDirtyFlag(false);

      } else if(USESIGMOID == evalPolicy_) {
         rawCurrentFitness_ = this->getWorstCase();
         if(true == this->getMaxMode()) { // maximize
            transformedCurrentFitness_ = -validityLevel_*barrier_;
         } else { // minimize
            transformedCurrentFitness_ =  validityLevel_*barrier_;
         }

         for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
            rawCurrentSecondaryFitness_.push_back(this->getWorstCase());
            transformedCurrentSecondaryFitness_.push_back(transformedCurrentFitness_);
         }

         // Clear the dirty flag -- all possible evaluation work was done
         setDirtyFlag(false);

      } else if(USEWORSTKNOWNVALIDFORINVALID == evalPolicy_) {
         // Some of this will be reset later, in  GOptimizableEntity::evaluationUpdate().
         // The caller needs to tell us about the worst solution known up to now. It is only
         // known once all individuals of this iteration have been evaluated.
         rawCurrentFitness_ = this->getWorstCase();
         transformedCurrentFitness_ = this->getWorstCase();

         for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
            rawCurrentSecondaryFitness_.push_back(this->getWorstCase());
            transformedCurrentSecondaryFitness_.push_back(this->getWorstCase());
         }

         // As only place-holders have been stored in the fitness criteria, the individual is still "dirty"
         //*****************
         setDirtyFlag(true);
         //*****************
      }
   }

   // Prevent further changes to markedAsInvalidExternallyAllowed_
   changesAllowedTo_markedAsInvalidExternally_ = false;

   // Return the main fitness value
   return transformedCurrentFitness_;
}

/* ----------------------------------------------------------------------------------
 * untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Registers a new, "raw" secondary result value of the custom fitness calculation. This is used in
 * multi-criterion optimization. fitnessCalculation() returns the main fitness value, but may also add further,
 * secondary results. Note that, whether these are used, depends on the optimization algorithm being used.
 *
 * @param secondaryValue The secondary fitness value to be registered
 */
void GOptimizableEntity::registerSecondaryResult(const double& secondaryValue) {
	rawCurrentSecondaryFitness_.push_back(secondaryValue);
}

/******************************************************************************/
/**
 * Sets the number of fitness criteria to be used with this object
 */
void GOptimizableEntity::setNumberOfFitnessCriteria(std::size_t nFitnessCriteria) {
   if(0 == nFitnessCriteria) {
      glogger
      << "In GOptimizableEntity::setNumberOfFitnessCriteria(): Error!" << std::endl
      << "Number of fitness criteria is empty" << std::endl
      << GEXCEPTION;
   }

   nFitnessCriteria_ = nFitnessCriteria;
}

/******************************************************************************/
/**
 * Determines the number of fitness criteria present for individual.
 *
 * @return The number of fitness criteria registered with this individual
 */
std::size_t GOptimizableEntity::getNumberOfFitnessCriteria() const {
	return nFitnessCriteria_;
}

/******************************************************************************/
/**
 * Determines the number of secondary fitness criteria present for individual.
 *
 * @return The number of secondary fitness criteria registered with this individual
 */
std::size_t GOptimizableEntity::getNumberOfSecondaryFitnessCriteria() const {
   if(0 == nFitnessCriteria_) {
      glogger
      << "In GOptimizableEntity::getNumberOfSecondaryFitnessCriteria(): Error!" << std::endl
      << "Number of fitness criteria is empty" << std::endl
      << GEXCEPTION;
   }

	return nFitnessCriteria_ - 1;
}

/******************************************************************************/
/**
 * Determines whether more than one fitness criterion is present for this individual
 *
 * @return A boolean indicating whether more than one target function is present
 */
bool GOptimizableEntity::hasMultipleFitnessCriteria() const {
	return (getNumberOfFitnessCriteria()>1?true:false);
}

/******************************************************************************/
/**
 * Checks the worst valid fitness and updates it when needed
 */
void GOptimizableEntity::challengeWorstValidFitness(
   boost::tuple<double, double>& worstCandidate
   , const std::size_t& id
) {
#ifdef DEBUG
   if(id >= this->getNumberOfFitnessCriteria()) {
      glogger
      << "In GOptimizableEntity::challengeWorstValidFitness(): Error!" << std::endl
      << "Requested fitness id " << id << " exceeds allowed range " << this->getNumberOfFitnessCriteria() << std::endl
      << GEXCEPTION;
   }

   if(this->isDirty()) {
      glogger
      << "In GOptimizableEntity::challengeWorstValidFitness(): Error!" << std::endl
      << "Function called for dirty individual" << std::endl
      << GEXCEPTION;
   }

   if(!this->isValid()) {
      glogger
      << "In GOptimizableEntity::challengeWorstValidFitness(): Error!" << std::endl
      << "Function called for invalid individual" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   double rawFitness         = this->fitness(id, PREVENTREEVALUATION, USERAWFITNESS);
   double transformedFitness = this->fitness(id, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);

   boost::tuple<double, double> eval(transformedFitness, rawFitness);

   if(isWorse(boost::get<0>(worstCandidate), boost::get<0>(eval))) { // TODO: Beim vertauschen von raw und transformed <0> --> <1>
      worstCandidate = eval;
   }
}

/******************************************************************************/
/**
 * Retrieve the fitness tuple at a given evaluation position
 */
boost::tuple<double,double> GOptimizableEntity::getFitnessTuple(const boost::uint32_t& id) {
   return boost::tuple<double, double>(
      this->fitness(id, PREVENTREEVALUATION, USETRANSFORMEDFITNESS)
      , this->fitness(id, PREVENTREEVALUATION, USERAWFITNESS)
   );
}

/******************************************************************************/
/**
 * Sets the fitness to a given set of values and clears the dirty flag. This is meant for external
 * methods of performing the actual evaluation.
 *
 * @param f The primary fitness value
 * @param sec_f_vec A vector of secondary fitness values
 */
void GOptimizableEntity::setFitness_(const double& f, const std::vector<double>& sec_f_vec) {
	if(sec_f_vec.size() != GOptimizableEntity::getNumberOfSecondaryFitnessCriteria()) {
	   glogger
	   << "In GOptimizableEntity::setFitness_(...): Error!" << std::endl
      << "Invalid size of secondary fitness vector: " << std::endl
      << sec_f_vec.size() << " / " << GOptimizableEntity::getNumberOfSecondaryFitnessCriteria() << std::endl
      << GEXCEPTION;
	}

	transformedCurrentFitness_ = f;
	transformedCurrentSecondaryFitness_ = sec_f_vec;

	// Clear the dirty flag
	setDirtyFlag(false);
}

/* ----------------------------------------------------------------------------------
 * Throwing and fitness setting is tested in GExternalEvaluatorIndividual
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether the dirty flag is set
 *
 * @return The value of the dirtyFlag_ variable
 */
bool GOptimizableEntity::isDirty() const  {
	return dirtyFlag_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Specify whether we want to work in maximization (true) or minimization
 * (false) mode. This function is protected. The idea is that GParameterSet provides a public
 * wrapper for this function, so that a user can specify whether he wants to maximize or
 * minimize a given evaluation function. Optimization algorithms, in turn, only check the
 * maximization-mode of the individuals stored in them and set their own maximization mode
 * internally accordingly, using the protected, overloaded function.
 *
 * @param mode A boolean which indicates whether we want to work in maximization or minimization mode
 */
void GOptimizableEntity::setMaxMode_(const bool& mode) {
	maximize_ = mode;
}

/* ----------------------------------------------------------------------------------
 * Setting is tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to retrieve the maximize_ parameter
 *
 * @return The current value of the maximize_ parameter
 */
bool GOptimizableEntity::getMaxMode() const {
	return maximize_;
}

/* ----------------------------------------------------------------------------------
 * Retrieval is tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***************************************************************************/
/**
 * Helper function that emits the worst case value depending on whether maximization
 * or minimization is performed.
 *
 * @return The worst case value, depending on maximization or minimization
 */
double GOptimizableEntity::getWorstCase() const {
   return (this->getMaxMode()?boost::numeric::bounds<double>::lowest():boost::numeric::bounds<double>::highest());
}

/******************************************************************************/
/**
 * Retrieves the best possible evaluation result, depending on whether we are in
 * maximization or minimization mode
 */
double GOptimizableEntity::getBestCase() const {
   return (this->getMaxMode()?boost::numeric::bounds<double>::highest():boost::numeric::bounds<double>::lowest());
}

/******************************************************************************/
/**
 * Retrieves the steepness_ variable (used for the sigmoid transformation)
 */
double GOptimizableEntity::getSteepness() const {
   return steepness_;
}

/******************************************************************************/
/**
 * Sets the steepness variable (used for the sigmoid transformation)
 */
void GOptimizableEntity::setSteepness(double steepness) {
   if(steepness <= 0.) {
      glogger
      << "In GOptimizableEntity::setSteepness(double steepness): Error!" << std::endl
      << "Invalid value of steepness parameter: " << steepness << std::endl
      << GEXCEPTION;
   }

   steepness_ = steepness;
}

/******************************************************************************/
/**
 * Retrieves the barrier_ variable (used for the sigmoid transformation)
 */
double GOptimizableEntity::getBarrier() const {
   return barrier_;
}

/******************************************************************************/
/**
 * Sets the barrier variable (used for the sigmoid transformation)
 */
void GOptimizableEntity::setBarrier(double barrier) {
   if(barrier <= 0.) {
      glogger
      << "In GOptimizableEntity::setBarrier(double barrier): Error!" << std::endl
      << "Invalid value of barrier parameter: " << barrier << std::endl
      << GEXCEPTION;
   }

   barrier_ = barrier;
}

/******************************************************************************/
/**
 * Sets the dirtyFlag_. This is a "one way" function, accessible to derived classes. Once the dirty flag
 * has been set, the only way to reset it is to calculate the fitness of this object.
 */
void GOptimizableEntity::setDirtyFlag()  {
	dirtyFlag_ = true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Sets the dirtyFlag_ to any desired value
 *
 * @param dirtyFlag The new value for the dirtyFlag_ variable
 * @return The previous value of the dirtyFlag_ variable
 */
bool GOptimizableEntity::setDirtyFlag(const bool& dirtyFlag)  {
	bool previous = dirtyFlag_;
	dirtyFlag_ = dirtyFlag;
	return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether this solution is valid. This function is meant to be called
 * for "clean" individuals only and will throw when called for individuals, whose
 * dirty flag is set
 */
bool GOptimizableEntity::isValid() const {
   if(this->isDirty()) {
      glogger
      << "In GOptimizableEntity::isValid(): Error!" << std::endl
      << "Function was called while dirty flag was set" << std::endl
      << GEXCEPTION;
   }

   if(validityLevel_ <= 1. && !markedAsInvalidExternally_ && !this->allRawResultsAtWorst()) {
      return true;
   } else {
      return false;
   }
}

/******************************************************************************/
/**
 * Checks whether this solution is invalid
 */
 bool GOptimizableEntity::isInValid() const {
    if(validityLevel_ > 1. || markedAsInvalidExternally_ || this->allRawResultsAtWorst()) {
       return true;
    } else {
       return false;
    }
 }

/******************************************************************************/
/**
 * Checks whether this solution is valid
 */
bool GOptimizableEntity::parameterSetFulfillsConstraints_(double& validityLevel) const {
   if(individualConstraint_) {
      return individualConstraint_->isValid(this, validityLevel);
   } else { // Always valid, if no constraint object has been registered
      validityLevel = 0.;
      return true;
   }

   // Make the compiler happy
   return false;
}

/******************************************************************************/
/**
 * Checks whether all results are at the worst possible value
 */
bool GOptimizableEntity::allRawResultsAtWorst() const {
   if(this->getWorstCase() != rawCurrentFitness_) return false;
   for(std::size_t i=0; i<getNumberOfSecondaryFitnessCriteria(); i++) {
      if(this->getWorstCase() != rawCurrentSecondaryFitness_.at(i)) return false;
   }

   // O.k., so all results are at their worst value
   return true;
}

/******************************************************************************/
/**
 * Check how valid a given solution is
 */
double GOptimizableEntity::getValidityLevel() const {
   return validityLevel_;
}

/******************************************************************************/
/**
 * Checks whether all constraints were fulfilled
 */
bool GOptimizableEntity::constraintsFulfilled() const {
   if(validityLevel_ <= 1.) return true;
   else return false;
}

/******************************************************************************/
/**
 * Allows to set the globally best known primary fitness so far
 *
 * @param bnf The best known primary fitness so far
 */
void GOptimizableEntity::setBestKnownPrimaryFitness(
   const double& bnf
) {
   bestPastPrimaryFitness_ = bnf;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the value of the globally best known primary fitness so far
 *
 * @return The best known primary fitness so far
 */
double GOptimizableEntity::getBestKnownPrimaryFitness() const {
   return bestPastPrimaryFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieve the id assigned to the current evaluation
 */
std::string GOptimizableEntity::getCurrentEvaluationID() const {
   return evaluationID_;
}

/******************************************************************************/
/**
 * Allows an optimization algorithm to set the worst known valid (primary and secondary
 * evaluation up to the current iteration. Note that these are not the best evaluations
 * for a single evaluation criterion, but the worst evaluations for all individuals that
 * were visited so far. Of the boost::tuple, the first value signifies the (possibly
 * transformed) evaluation, the second value the untransformed value.
 */
void GOptimizableEntity::setWorstKnownValid(
   const std::vector<boost::tuple<double, double> >& worstKnownValid
) {
   worstKnownValids_ = worstKnownValid;
}

/******************************************************************************/
/**
 * Allows to retrieve the worst known valid evaluation up to the current iteration,
 * as set by an external optimization algorithm, at a given position.
 */
boost::tuple<double, double> GOptimizableEntity::getWorstKnownValid(
   const boost::uint32_t& id
) const {
#ifdef DEBUG
   glogger
   << "In GOptimizableEntity::getWorstKnownValid(" << id << "): Error!" << std::endl
   << "Expected id of max " << worstKnownValids_.size() - 1 << std::endl
   << GEXCEPTION;
#endif /* DEBUG */

   return worstKnownValids_.at(id);
}

/******************************************************************************/
/**
 * Allows to retrieve all worst known valid evaluations up to the
 * current iteration, as set by an external optimization algorithm
 */
std::vector<boost::tuple<double, double> > GOptimizableEntity::getWorstKnownValids() const {
   return worstKnownValids_;
}

/******************************************************************************/
/**
 * Fills the worstKnownValid-vector with best values. This function assumes all
 * fitness criteria have been made known already.
 */
void GOptimizableEntity::populateWorstKnownValid() {
   // Make sure the worstKnownValids_ vector is empty
   worstKnownValids_.clear();

   for(std::size_t i=0; i<nFitnessCriteria_; i++) {
      worstKnownValids_.push_back(boost::tuple<double, double>(this->getBestCase(), this->getBestCase()));
   }
}

/******************************************************************************/
/**
 * Triggers an update of the internal evaluation, if necessary
 */
void GOptimizableEntity::evaluationUpdate() {
#ifdef DEBUG
   if((nFitnessCriteria_-1) != transformedCurrentSecondaryFitness_.size()) {
      glogger
      << "In GOptimizableEntity::evaluationUpdate(): Error!" << std::endl
      << "Number of expected fitness criteria " << (nFitnessCriteria_-1) << " does not match actual number " << transformedCurrentSecondaryFitness_.size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   if(USEWORSTKNOWNVALIDFORINVALID == evalPolicy_ && this->isInValid()) {
      if(true == maximize_) {
         transformedCurrentFitness_
               = -std::max(boost::get<0>(worstKnownValids_.at(0)),std::max(barrier_,1.))*validityLevel_;
         for(std::size_t i=1; i<nFitnessCriteria_; i++) {
            transformedCurrentSecondaryFitness_.at(i-1)
                 = -std::max(boost::get<0>(worstKnownValids_.at(i)), std::max(barrier_,1.))*validityLevel_;
         }
      } else {
         transformedCurrentFitness_
            =  std::max(boost::get<0>(worstKnownValids_.at(0)),std::max(barrier_,1.))*validityLevel_;
         for(std::size_t i=1; i<nFitnessCriteria_; i++) {
            transformedCurrentSecondaryFitness_.at(i-1)
                 = std::max(boost::get<0>(worstKnownValids_.at(i)), std::max(barrier_,1.))*validityLevel_;
         }
      }

      // Note: the "rawFitness" values have already been set in enforceFitnessUpdate() to the worst known evaluation

      // Make sure the dirty flag is set to false, so our results do not get overwritten
      this->setDirtyFlag(false);
   }
}

/******************************************************************************/
/**
 * Combines secondary evaluation results by adding the individual results
 *
 *  @return The result of the combination
 */
double GOptimizableEntity::sumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=transformedCurrentSecondaryFitness_.begin(); cit!=transformedCurrentSecondaryFitness_.end(); ++cit) {
		result += *cit;
	}
	return result;
}

/******************************************************************************/
/**
 * Combines secondary evaluation results by adding the absolute values of individual results
 *
 *  @return The result of the combination
 */
double GOptimizableEntity::fabsSumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=transformedCurrentSecondaryFitness_.begin(); cit!=transformedCurrentSecondaryFitness_.end(); ++cit) {
		result += fabs(*cit);
	}
	return result;
}

/******************************************************************************/
/**
 * Combines secondary evaluation results by calculation the square root of the squared sum. Note that we
 * only evaluate the secondary results here. It is assumed that the result of this function is returned as
 * the main result of the fitnessCalculation() function.
 *
 * @return The result of the combination
 */
double GOptimizableEntity::squaredSumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=transformedCurrentSecondaryFitness_.begin(); cit!=transformedCurrentSecondaryFitness_.end(); ++cit) {
		result += GSQUARED(*cit);
	}
	return sqrt(result);
}

/******************************************************************************/
/**
 * Combines secondary evaluation results by calculation the square root of the weighed squared sum. Note that we
 * only evaluate the secondary results here. It is assumed that the result of this function is returned as
 * the main result of the fitnessCalculation() function.
 *
 * @param weights The weights to be multiplied with the cached results
 * @return The result of the combination
 */
double GOptimizableEntity::weighedSquaredSumCombiner(const std::vector<double>& weights) const {
	double result = 0.;
	std::vector<double>::const_iterator cit_eval, cit_weights;

	if(transformedCurrentSecondaryFitness_.size() != weights.size()) {
	   glogger
	   << "In GOptimizableEntity::weighedSquaredSumCombine(): Error!" << std::endl
      << "Sizes of transformedCurrentSecondaryFitness_ and the weights vector don't match: " << transformedCurrentSecondaryFitness_.size() << " / " << weights.size() << std::endl
      << GEXCEPTION;
	}

	for(cit_eval=transformedCurrentSecondaryFitness_.begin(), cit_weights=weights.begin();
		cit_eval!=transformedCurrentSecondaryFitness_.end();
		++cit_eval, ++cit_weights
	) {
		result += GSQUARED((*cit_weights)*(*cit_eval));
	}

	return sqrt(result);
}

/******************************************************************************/
/**
 * Allows users to mark this solution as invalid in derived classes (usually
 * from within the evaluation function)
 */
void GOptimizableEntity::userMarkAsInvalid() {
   if(changesAllowedTo_markedAsInvalidExternally_) {
      markedAsInvalidExternally_ = true;
   } else {
      glogger
      << "In GOptimizableEntity::userMarkAsInvalid(): Error!" << std::endl
      << "Tried to mark individual as invalid while changes to this property" << std::endl
      << "were not allowed. This function may only be called from inside the" << std::endl
      << "fitness evaluation!" << std::endl
      << GEXCEPTION;
   }
}

/******************************************************************************/
/**
 * Helps to determine whether a given value is strictly better (i.e. better than equal)
 * than another one. As "better" means something different for maximization and minimization,
 * this function helps to make the code easier to understand.
 *
 * @param newValue The new value
 * @param oldValue The old value
 * @return true if newValue is better than oldValue, otherwise false.
 */
bool GOptimizableEntity::isBetter(double newValue, const double& oldValue) const {
   if(this->getMaxMode()) {
      if(newValue >= oldValue) return true;
      else return false;
   } else { // minimization
      if(newValue <= oldValue) return true;
      else return false;
   }
}

/******************************************************************************/
/**
 * Helps to determine whether a given value is strictly worse (i.e. worse than equal)
 * than another one. As "worse" means something different for maximization and minimization,
 * this function helps to make the code easier to understand.
 *
 * @param newValue The new value
 * @param oldValue The old value
 * @return true of newValue is worse than oldValue, otherwise false.
 */
bool GOptimizableEntity::isWorse(double newValue, const double& oldValue) const {
   if(this->getMaxMode()) {
      if(newValue < oldValue) return true;
      else return false;
   } else { // minimization
      if(newValue > oldValue) return true;
      else return false;
   }
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GOptimizableEntity::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
   std::string comment;

	// Call our parent class'es function
	GObject::addConfigurationOptions(gpb, showOrigin);

	// Add local data
   comment = ""; // Reset the comment string
   comment += "Specifies which strategy should be used to calculate the evaluation:;";
   comment += "0 (a.k.a. USESIMPLEEVALUATION): Always call the evaluation function, even for invalid solutions;";
   comment += "1 (a.k.a. USEWORSTCASEFORINVALID) : Assign the worst possible value to our fitness and evaluate only valid solutions;";
   comment += "2 (a.k.a. USESIGMOID): Assign a multiple of validityLevel_ and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations;";
   comment += "3 (a.k.a. USEWORSTKNOWNVALIDFORINVALID): Assign \"invalidityLevel*worstKnownValid\" to invalid individuals, using normal evaluation otherwise;";
   if(showOrigin) comment += "[GOptimizableEntity];";
   gpb.registerFileParameter<evaluationPolicy>(
      "evalPolicy" // The name of the variable
      , Gem::Geneva::USESIMPLEEVALUATION // The default value
      , boost::bind(
         &GOptimizableEntity::setEvaluationPolicy
         , this
         , _1
      )
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = ""; // Reset the comment string
   comment += "When using a sigmoid function to transform the individual's fitness,;";
   comment += "this parameter influences the steepness of the function at the center of the sigmoid.;";
   comment += "The parameter must have a value > 0.;";
   if(showOrigin) comment += "[GOptimizableEntity];";
   gpb.registerFileParameter<double>(
      "steepness" // The name of the variable
      , Gem::Geneva::FITNESSSIGMOIDSTEEPNESS // The default value
      , boost::bind(
         &GOptimizableEntity::setSteepness
         , this
         , _1
      )
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = ""; // Reset the comment string
   comment += "When using a sigmoid function to transform the individual's fitness,;";
   comment += "this parameter sets the upper/lower boundary of the sigmoid.;";
   comment += "The parameter must have a value > 0.;";
   if(showOrigin) comment += "[GOptimizableEntity];";
   gpb.registerFileParameter<double>(
      "barrier" // The name of the variable
      , Gem::Geneva::WORSTALLOWEDVALIDFITNESS // The default value
      , boost::bind(
         &GOptimizableEntity::setBarrier
         , this
         , _1
      )
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	// maximize_ will be set in GParameterSet, as it has a different
	// meaning for optimization algorithms that also derive indirectly
	// from this class.
}

/******************************************************************************/
/**
 * Sets the current personality of this individual
 *
 * @param gpt A pointer to an object representing the new personality of this object
 */
void GOptimizableEntity::setPersonality(
      boost::shared_ptr<GPersonalityTraits> gpt
) {
   // Make sure we haven't been given an empty pointer
   if(!gpt) {
      glogger
      << "In GOptimizableEntity::setPersonality(): Error!" << std::endl
      << "Received empty personality traits pointer" << std::endl
      << GEXCEPTION;
   }

	// Add the personality traits object to our local pointer
	pt_ptr_ = gpt;
}


/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Resets the current personality to PERSONALITY_NONE
 */
void GOptimizableEntity::resetPersonality() {
   pt_ptr_.reset();
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the current personality of this individual
 *
 * @return An identifier for the current personality of this object
 */
std::string GOptimizableEntity::getPersonality() const {
	if(pt_ptr_) {
	   return pt_ptr_->name();
	} else {
	   return std::string("PERSONALITY_NONE");
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * This function returns the current personality traits base pointer. Note that there
 * is another version of the same command that does on-the-fly conversion of the
 * personality traits to the derived class.
 *
 * @return A shared pointer to the personality traits base class
 */
boost::shared_ptr<GPersonalityTraits> GOptimizableEntity::getPersonalityTraits() {
#ifdef DEBUG
	// Do some error checking
	if(!pt_ptr_) {
	   glogger
	   << "In GOptimizableEntity::getPersonalityTraits():" << std::endl
      << "Pointer to personality traits object is empty." << std::endl
      << GEXCEPTION;
	}
#endif

	return pt_ptr_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GOptimizableEntity::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * A wrapper for GOptimizableEntity::customUpdateOnStall() (or the corresponding overloaded
 * functions in derived classes) that does error-checking and sets the dirty flag. The function
 * is called by GOptimizationAlgorithmT<>, whenever the optimization procedure is stalled.
 *
 * @param nStalls The number of stalls since the last improvement
 * @return A boolean indicating whether an update was performed and the individual has changed
 */
bool GOptimizableEntity::updateOnStall(const std::size_t& nStalls) {
	// Do the actual update of the individual's structure
	bool updatePerformed = customUpdateOnStall(nStalls);
	if(updatePerformed) {
		setDirtyFlag();
	}

	return updatePerformed;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows users to update the structure of the individual when a stall has occurred
 *
 * @return A boolean indicating whether an update was performed and the object has changed
 */
bool GOptimizableEntity::customUpdateOnStall(const std::size_t& nStalls) {
	return false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Actions to be performed when adapting this object. This function will be overloaded particularly
 * for the GParameterSet class.
 */
void GOptimizableEntity::customAdaptions()
{ /* nothing */}

/******************************************************************************/
/**
 * Allows to set the current iteration of the parent optimization algorithm.
 *
 * @param parentAlgIteration The current iteration of the optimization algorithm
 */
void GOptimizableEntity::setAssignedIteration(const boost::uint32_t& parentAlgIteration) {
	assignedIteration_ = parentAlgIteration;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Gives access to the parent optimization algorithm's iteration
 *
 * @return The parent optimization algorithm's current iteration
 */
boost::uint32_t GOptimizableEntity::getAssignedIteration() const {
	return assignedIteration_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to specify the number of optimization cycles without improvement of the primary fitness criterion
 *
 * @param nStalls The number of optimization cycles without improvement in the parent algorithm
 */
void GOptimizableEntity::setNStalls(const boost::uint32_t& nStalls) {
	nStalls_ = nStalls;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion
 *
 * @return The number of optimization cycles without improvement in the parent algorithm
 */
boost::uint32_t GOptimizableEntity::getNStalls() const {
	return nStalls_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GOptimizableEntity::modify_GUnitTests() {
#ifdef GEM_TESTING

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	// Adaption of parameters through the adapt() call is done in GTestIndividual1.
	// We do not know what is stored in this individual, hence we cannot modify
	// parameters directly here in this class.

	// A relatively harmless change
	nStalls_++;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GOptimizableEntity::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the maximization mode flag
		boost::shared_ptr<GOptimizableEntity> p_test = this->clone<GOptimizableEntity>();

		BOOST_CHECK_NO_THROW(p_test->setMaxMode_(true));
		BOOST_CHECK(p_test->getMaxMode() == true);
		BOOST_CHECK_NO_THROW(p_test->setMaxMode_(false));
		BOOST_CHECK(p_test->getMaxMode() == false);
	}

	// --------------------------------------------------------------------------

	{ // Check setting of the dirty flag
		boost::shared_ptr<GOptimizableEntity> p_test = this->clone<GOptimizableEntity>();

		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(true));
		BOOST_CHECK(p_test->isDirty() == true);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(false));
		BOOST_CHECK(p_test->isDirty() == false);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
		BOOST_CHECK(p_test->isDirty() == true); // Note the missing argument -- this is a different function
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(false));
		BOOST_CHECK(p_test->isDirty() == false);
	}

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the surrounding optimization algorithm's current iteration
		boost::shared_ptr<GOptimizableEntity> p_test = this->clone<GOptimizableEntity>();

		for(boost::uint32_t i=1; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setAssignedIteration(i));
			BOOST_CHECK_MESSAGE(
					p_test->getAssignedIteration() == i
					,  "\n"
					<< "p_test->getAssignedIteration() = " << p_test->getAssignedIteration() << "\n"
					<< "i = " << i << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the best known fitness so far
		boost::shared_ptr<GOptimizableEntity> p_test = this->clone<GOptimizableEntity>();

		for(double d=0.; d<1.; d+=0.1) {
			BOOST_CHECK_NO_THROW(p_test->setBestKnownPrimaryFitness(d));
			BOOST_CHECK_MESSAGE(
					p_test->getBestKnownPrimaryFitness() == d
					,  "\n"
					<< "p_test->getBestKnownPrimaryFitness() = " << p_test->getBestKnownPrimaryFitness() << "\n"
					<< "d = " << d << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the number of consecutive stalls
		boost::shared_ptr<GOptimizableEntity> p_test = this->clone<GOptimizableEntity>();

		for(boost::uint32_t i=1; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setNStalls(i));
			BOOST_CHECK_MESSAGE(
					p_test->getNStalls() == i
					,  "\n"
					<< "p_test->getNStalls() = " << p_test->getNStalls() << "\n"
					<< "i = " << i << "\n"
			);
		}
	}

   // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GOptimizableEntity::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();

   // --------------------------------------------------------------------------
   // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GOptimizableEntity::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
