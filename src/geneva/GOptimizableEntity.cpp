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
/**
 * The default constructor.
 */
GOptimizableEntity::GOptimizableEntity()
	: GMutableI()
	, GRateableI()
	, GObject()
	, currentFitness_(0.)
   , currentSecondaryFitness_()
   , nFitnessCriteria_(1)
	, bestPastFitness_(0.)
	, bestPastSecondaryFitness_(0)
	, nStalls_(0)
	, dirtyFlag_(true)
	, serverMode_(false)
	, maximize_(false)
	, assignedIteration_(0)
	, validityLevel_(0.) // Always valid by default
   , invalidPolicy_(Gem::Geneva::USEEVALUATION)
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
	, currentFitness_(cp.currentFitness_)
	, currentSecondaryFitness_(cp.currentSecondaryFitness_)
   , nFitnessCriteria_(cp.nFitnessCriteria_)
	, bestPastFitness_(cp.bestPastFitness_)
	, bestPastSecondaryFitness_(cp.bestPastSecondaryFitness_)
	, nStalls_(cp.nStalls_)
	, dirtyFlag_(cp.dirtyFlag_)
	, serverMode_(cp.serverMode_)
	, maximize_(cp.maximize_)
	, assignedIteration_(cp.assignedIteration_)
	, validityLevel_(cp.validityLevel_)
   , invalidPolicy_(cp.invalidPolicy_)
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
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", currentFitness_, p_load->currentFitness_, "currentFitness_", "p_load->currentFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", currentSecondaryFitness_, p_load->currentSecondaryFitness_, "currentSecondaryFitness_", "p_load->currentSecondaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", nFitnessCriteria_, p_load->nFitnessCriteria_, "nFitnessCriteria_", "p_load->nFitnessCriteria_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", bestPastFitness_, p_load->bestPastFitness_, "bestPastFitness_", "p_load->bestPastFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", bestPastSecondaryFitness_, p_load->bestPastSecondaryFitness_, "bestPastSecondaryFitness_", "p_load->bestPastSecondaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", nStalls_, p_load->nStalls_, "nStalls_", "p_load->nStalls_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", dirtyFlag_, p_load->dirtyFlag_, "dirtyFlag_", "p_load->dirtyFlag_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", serverMode_, p_load->serverMode_, "serverMode_", "p_load->serverMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", maximize_, p_load->maximize_, "maximize_", "p_load->maximize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", assignedIteration_, p_load->assignedIteration_, "assignedIteration_", "p_load->assignedIteration_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", validityLevel_, p_load->validityLevel_, "validityLevel_", "p_load->validityLevel_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", invalidPolicy_, p_load->invalidPolicy_, "invalidPolicy_", "p_load->invalidPolicy_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", pt_ptr_, p_load->pt_ptr_, "pt_ptr_", "p_load->pt_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizableEntity", individualConstraint_, p_load->individualConstraint_, "individualConstraint_", "p_load->individualConstraint_", e , limit));

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
 * Allows to register a constraint with this individual
 */
void GOptimizableEntity::registerConstraint(boost::shared_ptr<GValidityCheckT<GOptimizableEntity> > c_ptr) {
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
void GOptimizableEntity::setInvalidPolicy(invalidIndividualPolicy invalidPolicy) {
   invalidPolicy_ = invalidPolicy;
}

/******************************************************************************/
/**
 * Allows to retrieve the current policy in case this individual represents an invalid solution
 */
invalidIndividualPolicy GOptimizableEntity::getInvalidPolicy() const {
   return invalidPolicy_;
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
	currentFitness_ = p_load->currentFitness_;
	currentSecondaryFitness_ = p_load->currentSecondaryFitness_;
	nFitnessCriteria_ = p_load->nFitnessCriteria_;
	bestPastFitness_ = p_load->bestPastFitness_;
	bestPastSecondaryFitness_ = p_load->bestPastSecondaryFitness_;
	nStalls_ = p_load->nStalls_;
	dirtyFlag_ = p_load->dirtyFlag_;
	serverMode_ = p_load->serverMode_;
	maximize_ = p_load->maximize_;
	assignedIteration_ = p_load->assignedIteration_;
	validityLevel_ = p_load->validityLevel_;
	invalidPolicy_ = p_load->invalidPolicy_;

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
 * Returns the last known fitness calculations of this object. Re-calculation
 * of the fitness is triggered, unless this is the server mode. By means of supplying
 * an id it is possible to distinguish between different target functions. 0 denotes
 * the main fitness criterion.
 *
 * @param id The id of the fitness criterion
 * @return The fitness of this individual
 */
double GOptimizableEntity::fitness(const std::size_t& id) {
	// Check whether we need to recalculate the fitness
	if (dirtyFlag_) {
		// Re-evaluation is not allowed on the server
		if (serverMode_) {
		   glogger
		   << "In GOptimizableEntity::fitness():" << std::endl
         << "Tried to perform re-evaluation in server-mode" << std::endl
         << GEXCEPTION;
		}

		this->doFitnessCalculation();
	}

	// Return the desired result
	bool tmpDirtyFlag = false;
	return getCachedFitness(tmpDirtyFlag, id);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Returns the last known fitness calculation of this object, using the fitness function
 * with id 0.
 *
 * @return The fitness of this individual, according to the fitness function with id 0
 */
double GOptimizableEntity::fitness() {
	return fitness(0);
}

/******************************************************************************/
/**
 * Adapts and evaluates the individual in one go
 *
 * @return The main fitness result
 */
double GOptimizableEntity::adaptAndEvaluate() {
	adapt();
	return doFitnessCalculation();
}

/******************************************************************************/
/**
 * Retrieves the cached (not necessarily up-to-date) fitness
 *
 * @param dirtyFlag The value of the dirtyFlag_ variable
 * @param id The id of the primary or secondary fitness value
 * @return The cached fitness value (not necessarily up-to-date) with id id
 */
double GOptimizableEntity::getCachedFitness(bool& dirtyFlag, const std::size_t& id) const  {
	dirtyFlag = dirtyFlag_;

	if(0 == id) {
		return currentFitness_;
	} else {
#ifdef DEBUG
		if(currentSecondaryFitness_.size() < id) {
		   glogger
		   << "In GOptimizableEntity::getCachedFitness(bool&, const std::size_t& id): Error!" << std::endl
         << "Got invalid result id: " << id << std::endl
         << "where maximum allowed id would be " << currentSecondaryFitness_.size()-1 << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		return currentSecondaryFitness_.at(id - 1);
	}

	// Make the compiler happy
	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Enforces re-calculation of the fitness.
 *
 * @return The main result of the fitness calculation
 */
double GOptimizableEntity::doFitnessCalculation() {
   // Make sure the secondary fitness vector is empty
   currentSecondaryFitness_.clear();

   // Check whether this is a valid solution and act accordingly
   if(!this->isValid(validityLevel_)) {
      switch(invalidPolicy_) {
         //---------------------------------------------------------------------
         case USEEVALUATION:
            // Nothing -- we just continue with the evaluation as requested
            break;

         //---------------------------------------------------------------------
         case USEWORSTCASE:
            {
               currentFitness_ = this->getWorstCase();
               for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
                  currentSecondaryFitness_.push_back(this->getWorstCase());
               }
            }
            break;

         //---------------------------------------------------------------------
         // Note: This option usually means that the optimization algorithm
         // will supply information about the worst valid solution found
         case USECONSTRAINTOBJECTPOLICY:
            {
               currentFitness_ = validityLevel_;
               for(std::size_t i=1; i<getNumberOfFitnessCriteria(); i++) {
                  currentSecondaryFitness_.push_back(validityLevel_);
               }
            }
            break;

         //---------------------------------------------------------------------
         default:
            {
               glogger
               << "In GOptimizableEntity::doFitnessCalculation(): Error!" << std::endl
               << "Got invalid invalidPolicy_ parameter: " << invalidPolicy_ << std::endl
               << GEXCEPTION;
            }
            break;

         //---------------------------------------------------------------------
      }
   }

   // Trigger fitness calculation. This will also
   // register secondary fitness values used in multi-criterion
   // optimization.
   currentFitness_ = fitnessCalculation();

   // Clear the dirty flag
   setDirtyFlag(false);

   // Return the main fitness value
	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Registers a new, secondary result value of the custom fitness calculation. This is used in multi-criterion
 * optimization. fitnessCalculation() returns the main fitness value, but may also add further, secondary
 * results. Note that, whether these are used, depends on the optimization algorithm being used.
 *
 * @param secondaryValue The secondary fitness value to be registered
 */
void GOptimizableEntity::registerSecondaryResult(const double& secondaryValue) {
	currentSecondaryFitness_.push_back(secondaryValue);
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
 * Determines the number of fitness criteria present for individual. Secondary fitness values are stored in
 * a std::vector, so we determine its size and add 1 for the main fitness value (which always needs to be present).
 *
 * @return The number of fitness criteria registered with this individual
 */
std::size_t GOptimizableEntity::getNumberOfFitnessCriteria() const {
	return nFitnessCriteria_;
}

/******************************************************************************/
/**
 * Determines the number of secondary itness criteria present for individual.
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

	currentFitness_ = f;
	currentSecondaryFitness_ = sec_f_vec;

	// Clear the dirty flag
	setDirtyFlag(false);
}

/* ----------------------------------------------------------------------------------
 * Throwing and fitness setting is tested in GExternalEvaluatorIndividual
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * (De-)activates the server mode.
 *
 * @param sM The desired new value of the serverMode_ variable
 * @return The previous value of the serverMode_ variable
 */
bool GOptimizableEntity::setServerMode(const bool& sM) {
	bool previous = serverMode_;
	serverMode_ = sM;
	return previous;
}

/* ----------------------------------------------------------------------------------
 * Setting is tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw of fitness() function in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether the server mode is set
 *
 * @return The current value of the serverMode_ variable
 */
bool GOptimizableEntity::serverMode() const {
	return serverMode_;
}

/* ----------------------------------------------------------------------------------
 * Retrieval is tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether the server mode is set
 *
 * @return The current value of the serverMode_ variable
 */
bool GOptimizableEntity::getServerMode() const {
	return serverMode();
}

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
   return (this->getMaxMode()?-DBL_MAX:DBL_MAX);
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
 * Checks whether this solution is valid
 */
bool GOptimizableEntity::isValid(double& validityLevel) const {
   if(individualConstraint_) {
      return individualConstraint_->isValid(this, validityLevel);
   } else { // Always valid, if no constraint object has been registered
      validityLevel = 0.;
      return true;
   }
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
 * Combines secondary evaluation results by adding the individual results
 *
 *  @return The result of the combination
 */
double GOptimizableEntity::sumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=currentSecondaryFitness_.begin(); cit!=currentSecondaryFitness_.end(); ++cit) {
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
	for(cit=currentSecondaryFitness_.begin(); cit!=currentSecondaryFitness_.end(); ++cit) {
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
	for(cit=currentSecondaryFitness_.begin(); cit!=currentSecondaryFitness_.end(); ++cit) {
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

	if(currentSecondaryFitness_.size() != weights.size()) {
	   glogger
	   << "In GOptimizableEntity::weighedSquaredSumCombine(): Error!" << std::endl
      << "Sizes of currentSecondaryFitness_ and the weights vector don't match: " << currentSecondaryFitness_.size() << " / " << weights.size() << std::endl
      << GEXCEPTION;
	}

	for(cit_eval=currentSecondaryFitness_.begin(), cit_weights=weights.begin();
		cit_eval!=currentSecondaryFitness_.end();
		++cit_eval, ++cit_weights
	) {
		result += GSQUARED((*cit_weights)*(*cit_eval));
	}

	return sqrt(result);
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
	// none ...

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
 * functions in derived classes) that does error-checking and sets the dirty flag.
 *
 * @return A boolean indicating whether an update was performed and the individual has changed
 */
bool GOptimizableEntity::updateOnStall() {
	// Do the actual update of the individual's structure
	bool updatePerformed = customUpdateOnStall();
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
 * Updates the object's structure and/or parameters, if the optimization has
 * stalled. The quality of the object is likely to get worse. Hence it will
 * enter a micro-training environment to improve its quality. The function can
 * inform the caller that no action was taken, by returning false. Otherwise, if
 * an update was made that requires micro-training, true should be returned.
 * The actions to be taken for this update depend on the actual structure of the
 * individual and need to be implemented for each particular case individually.
 * Note that, as soon as the structure of this object changes, it should return
 * true, as otherwise no value calculation takes place.
 *
 * @return A boolean indicating whether an update was performed and the object has changed
 */
bool GOptimizableEntity::customUpdateOnStall() {
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
 * Allows to set the globally best known fitness
 *
 * @param bnf The best known fitness so far
 */
void GOptimizableEntity::setBestKnownFitness(const double& bnf) {
	bestPastFitness_ = bnf;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the value of the globally best known fitness
 *
 * @return The best known fitness so far
 */
double GOptimizableEntity::getBestKnownFitness() const {
	return bestPastFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to specify the number of optimization cycles without improvement
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
 * Allows to retrieve the number of optimization cycles without improvement
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

	{ // Test setting and retrieval of the server mode flag
		boost::shared_ptr<GOptimizableEntity> p_test = this->clone<GOptimizableEntity>();

		BOOST_CHECK_NO_THROW(p_test->setServerMode(true));
		BOOST_CHECK(p_test->serverMode() == true);
		BOOST_CHECK_NO_THROW(p_test->setServerMode(false));
		BOOST_CHECK(p_test->serverMode() == false);
	}

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
			BOOST_CHECK_NO_THROW(p_test->setBestKnownFitness(d));
			BOOST_CHECK_MESSAGE(
					p_test->getBestKnownFitness() == d
					,  "\n"
					<< "p_test->getBestKnownFitness() = " << p_test->getBestKnownFitness() << "\n"
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
