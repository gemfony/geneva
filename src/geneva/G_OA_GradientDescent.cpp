/**
 * @file G_OA_GradientDescent.cpp
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

#include "geneva/G_OA_GradientDescent.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GGradientDescent)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GGradientDescent::GGradientDescent()
	: GGradientDescent(DEFAULTGDSTARTINGPOINTS, DEFAULTFINITESTEP, DEFAULTSTEPSIZE)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of starting points and other parameters
 *
 * @param nStartingPoints The number of simultaneous starting points for the gradient descent
 * @param finiteStep The desired size of the incremental adaption process
 * @param stepSize The size of the multiplicative factor of the adaption process
 */
GGradientDescent::GGradientDescent(
	const std::size_t &nStartingPoints
	, const double &finiteStep
	, const double &stepSize
)
	: G_OptimizationAlgorithm_Base()
	, nStartingPoints_(nStartingPoints)
	, finiteStep_(finiteStep)
	, stepSize_(stepSize)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp A copy of another GradientDescent object
 */
GGradientDescent::GGradientDescent(const GGradientDescent &cp)
	: G_OptimizationAlgorithm_Base(cp)
	, nStartingPoints_(cp.nStartingPoints_)
	, nFPParmsFirst_(cp.nFPParmsFirst_)
	, finiteStep_(cp.finiteStep_)
	, stepSize_(cp.stepSize_)
	, stepRatio_(cp.stepRatio_) // Will be recalculated in init()
	, dblLowerParameterBoundaries_(cp.dblLowerParameterBoundaries_) // Will be extracted in init()
	, dblUpperParameterBoundaries_(cp.dblUpperParameterBoundaries_) // Will be extracted in init()
	, adjustedFiniteStep_(cp.adjustedFiniteStep_) // Will be recalculated in init()
{
	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * The destructor
 */
GGradientDescent::~GGradientDescent() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GGradientDescent &GGradientDescent::operator=(const GGradientDescent &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GGradientDescent object
 *
 * @param  cp A constant reference to another GGradientDescent object
 * @return A boolean indicating whether both objects are equal
 */
bool GGradientDescent::operator==(const GGradientDescent &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GGradientDescent object
 *
 * @param  cp A constant reference to another GGradientDescent object
 * @return A boolean indicating whether both objects are unequal
 */
bool GGradientDescent::operator!=(const GGradientDescent &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GGradientDescent::getAlgorithmPersonalityType() const {
	return "PERSONALITY_GD";
}

/******************************************************************************/
/**
 * Retrieves the number of starting points of the algorithm
 *
 * @return The number of simultaneous starting points of the gradient descent
 */
std::size_t GGradientDescent::getNStartingPoints() const {
	return nStartingPoints_;
}

/******************************************************************************/
/**
 * Allows to set the number of starting points for the gradient descent
 *
 * @param nStartingPoints The desired number of starting points for the gradient descent
 */
void GGradientDescent::setNStartingPoints(std::size_t nStartingPoints) {
	// Do some error checking
	if (nStartingPoints == 0) {
		glogger
		<< "In GGradientDescent::setNStartingPoints(const std::size_t&):" << std::endl
		<< "Got invalid number of starting points." << std::endl
		<< GEXCEPTION;
	}

	nStartingPoints_ = nStartingPoints;
}

/******************************************************************************/
/**
 * Set the size of the finite step of the adaption process
 *
 * @param finiteStep The desired size of the adaption
 */
void GGradientDescent::setFiniteStep(double finiteStep) {
	// Check that finiteStep_ has an appropriate value
	if (finiteStep_ <= 0. || finiteStep_ > 1000.) { // Specified in per mill of the allowed or preferred value range
		glogger
		<< "In GGradientDescent::setFiniteStep(double): Error!" << std::endl
		<< "Invalid values of finiteStep_: " << finiteStep_ << std::endl
		<< "Must be in the range ]0.:1000.]" << std::endl
		<< GEXCEPTION;
	}

	finiteStep_ = finiteStep;
}

/******************************************************************************/
/**
 * Retrieve the size of the finite step of the adaption process
 *
 * @return The current finite step size
 */
double GGradientDescent::getFiniteStep() const {
	return finiteStep_;
}

/******************************************************************************/
/**
 * Sets a multiplier for the adaption process
 *
 * @param stepSize A multiplicative factor for the adaption process
 */
void GGradientDescent::setStepSize(double stepSize) {
	// Check that stepSize_ has an appropriate value
	if (stepSize_ <= 0. || stepSize_ > 1000.) { // Specified in per mill of the allowed or preferred value range
		glogger
		<< "In GGradientDescent::setStepSize(double): Error!" << std::endl
		<< "Invalid values of stepSize_: " << stepSize_ << std::endl
		<< "Must be in the range ]0.:1000.]" << std::endl
		<< GEXCEPTION;
	}

	stepSize_ = stepSize;
}

/******************************************************************************/
/**
 * Retrieves the current step size
 *
 * @return The current value of the step size
 */
double GGradientDescent::getStepSize() const {
	return stepSize_;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GGradientDescent::getNProcessableItems() const {
	return this->size(); // Evaluation always needs to be done for the entire population
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GGradientDescent::getAlgorithmName() const {
	return std::string("Gradient Descent");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GGradientDescent::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GGradientDescent reference independent of this object and convert the pointer
	const GGradientDescent *p_load = Gem::Common::g_convert_and_compare<GObject, GGradientDescent>(cp, this);

	GToken token("GGradientDescent", e);

	// Compare our parent data ...
	Gem::Common::compare_base<G_OptimizationAlgorithm_Base>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(nStartingPoints_, p_load->nStartingPoints_), token);
	compare_t(IDENTITY(nFPParmsFirst_, p_load->nFPParmsFirst_), token);
	compare_t(IDENTITY(finiteStep_, p_load->finiteStep_), token);
	compare_t(IDENTITY(stepSize_, p_load->stepSize_), token);
	compare_t(IDENTITY(stepRatio_, p_load->stepRatio_), token);
	compare_t(IDENTITY(dblLowerParameterBoundaries_, p_load->dblLowerParameterBoundaries_), token);
	compare_t(IDENTITY(dblUpperParameterBoundaries_, p_load->dblUpperParameterBoundaries_), token);
	compare_t(IDENTITY(adjustedFiniteStep_, p_load->adjustedFiniteStep_), token);

	// React on deviations from the expectation
	token.evaluate();
}


/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GGradientDescent::resetToOptimizationStart() {
	dblLowerParameterBoundaries_.clear(); // Holds lower boundaries of double parameters; Will be extracted in init()
	dblUpperParameterBoundaries_.clear(); // Holds upper boundaries of double parameters; Will be extracted in init()
	adjustedFiniteStep_.clear(); // A step-size normalized to each parameter range; Will be recalculated in init()

	// There is no more work to be done here, so we simply call the
	// function of the parent class
	G_OptimizationAlgorithm_Base::resetToOptimizationStart();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GGradientDescent::name() const {
	return std::string("GGradientDescent");
}

/******************************************************************************/
/**
 * Loads the data of another population
 *
 * @param cp A pointer to another GGradientDescent object, camouflaged as a GObject
 */
void GGradientDescent::load_(const GObject *cp) {
	// Check that we are dealing with a GGradientDescent reference independent of this object and convert the pointer
	const GGradientDescent *p_load = Gem::Common::g_convert_and_compare<GObject, GGradientDescent>(cp, this);

	// First load the parent class'es data.
	// This will also take care of copying all individuals.
	G_OptimizationAlgorithm_Base::load_(cp);

	// ... and then our own data
	nStartingPoints_ = p_load->nStartingPoints_;
	nFPParmsFirst_ = p_load->nFPParmsFirst_;
	finiteStep_ = p_load->finiteStep_;
	stepSize_ = p_load->stepSize_;
	// stepRatio_ = p_load->stepRatio_; // temporary parameter
	// m_dbl_lower_parameter_boundaries_vec = p_load->m_dbl_lower_parameter_boundaries_vec; // temporary parameter
	// m_dbl_upper_parameter_boundaries_vec = p_load->m_dbl_upper_parameter_boundaries_vec; // temporary parameter
	// adjustedFiniteStep_ = p_load->adjustedFiniteStep_; // temporary parameter
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GGradientDescent::clone_() const {
	return new GGradientDescent(*this);
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * @return The value of the best individual found in this iteration
 */
std::tuple<double, double> GGradientDescent::cycleLogic() {
	if (afterFirstIteration()) {
		// Update the parameters of the parent individuals. This
		// only makes sense once the individuals have been evaluated
		this->updateParentIndividuals();
	}

	// Update the individual parameters in each dimension of the "children"
	this->updateChildParameters();

	// Trigger value calculation for all individuals (including parents)
	runFitnessCalculation();

	// Perform post-evaluation updates (mostly of individuals)
	postEvaluationWork();

	std::tuple<double, double> bestFitness = std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());
	std::tuple<double, double> fitnessCandidate = std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());

	// Retrieve information about the best fitness found and disallow re-evaluation
	GGradientDescent::iterator it;
	for (it = this->begin(); it != this->begin() + this->getNStartingPoints(); ++it) {
		std::get<G_RAW_FITNESS>(fitnessCandidate) = (*it)->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
		std::get<G_TRANSFORMED_FITNESS>(fitnessCandidate) = (*it)->fitness(0, PREVENTREEVALUATION,
																									USETRANSFORMEDFITNESS);

		if (this->at(0)->isBetter(
				std::get<G_TRANSFORMED_FITNESS>(fitnessCandidate)
				, std::get<G_TRANSFORMED_FITNESS>(bestFitness)
			)
		) {
			bestFitness = fitnessCandidate;
		}
	}

	return bestFitness;
}

/******************************************************************************/
/**
 * Updates the individual parameters of children
 */
void GGradientDescent::updateChildParameters() {
	// Loop over all starting points
	for (std::size_t i = 0; i < nStartingPoints_; i++) {
		// Extract the fp vector
		std::vector<double> parmVec;
		this->at(i)->streamline<double>(parmVec, activityMode::ACTIVEONLY); // Only extract active parameters

		// Loop over all directions
		for (std::size_t j = 0; j < nFPParmsFirst_; j++) {
			// Calculate the position of the child
			std::size_t childPos = nStartingPoints_ + i * nFPParmsFirst_ + j;

			// Load the current "parent" into the "child"
			this->at(childPos)->GObject::load(this->at(i));

			// Update the child's position in the population
			this->at(childPos)->getPersonalityTraits<G_OA_GradientDescent_PersonalityTraits>()->setPopulationPosition(childPos);

			// Make a note of the current parameter's value
			double origParmVal = parmVec[j];

			// Add the finite step to the feature vector's current parameter
			parmVec[j] += adjustedFiniteStep_[j];

			// Attach the feature vector to the child individual
			this->at(childPos)->assignValueVector<double>(parmVec, activityMode::ACTIVEONLY);

			// Restore the original value in the feature vector
			parmVec[j] = origParmVal;
		}
	}
}

/**********************************************************************************************************/
/**
 * Performs a step of the parent individuals.
 * TODO: keep going in the same direction as long as there is an improvement
 */
void GGradientDescent::updateParentIndividuals() {
	for (std::size_t i = 0; i < nStartingPoints_; i++) {
		// Extract the fp vector
		std::vector<double> parmVec;
		this->at(i)->streamline<double>(parmVec, activityMode::ACTIVEONLY);

#ifdef DEBUG
		// Make sure the parents are clean
		if(this->at(i)->isDirty()) {
		   glogger
		   << "In GGradientDescent::updateParentIndividuals():" << std::endl
         << "Found individual in position " << i << " with active dirty flag" << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		// Retrieve the fitness of the individual again
		double parentFitness = this->at(i)->minOnly_fitness();

		// Calculate the adaption of each parameter
		// double gradient = 0.;
		for (std::size_t j = 0; j < nFPParmsFirst_; j++) {
			// Calculate the position of the child
			std::size_t childPos = nStartingPoints_ + i * nFPParmsFirst_ + j;

			// Calculate the step to be performed in a given direction and
			// adjust the parameter vector of each parent
			try {
				parmVec[j] -= boost::numeric_cast<double>(
					stepRatio_ *
					((long double) (this->at(childPos)->minOnly_fitness()) - (long double) (parentFitness))
				);
			} catch (boost::bad_numeric_cast &e) {
				glogger
				<< "In GGradientDescent::updateParentIndividuals(): Error!" << std::endl
				<< "Bad conversion with message " << e.what() << std::endl
				<< GEXCEPTION;
			}
		}

		// Load the parameter vector back into the parent
		this->at(i)->assignValueVector<double>(parmVec, activityMode::ACTIVEONLY);
	}
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GGradientDescent::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	G_OptimizationAlgorithm_Base::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<std::size_t>(
		"nStartingPoints" // The name of the variable
		, DEFAULTGDSTARTINGPOINTS // The default value
		, [this](std::size_t nsp) { this->setNStartingPoints(nsp); }
	)
	<< "The number of simultaneous gradient descents";

	gpb.registerFileParameter<double>(
		"finiteStep" // The name of the variable
		, DEFAULTFINITESTEP // The default value
		, [this](double fs) { this->setFiniteStep(fs); }
	)
	<< "The size of the adjustment in the difference quotient," << std::endl
	<< "specified in per mill of the allowed or expected value" << std::endl
	<< "range of a parameter";

	gpb.registerFileParameter<double>(
		"stepSize" // The name of the variable
		, DEFAULTSTEPSIZE // The default value
		, [this](double ss) { this->setStepSize(ss); }
	)
	<< "The size of each step into the" << std::endl
	<< "direction of steepest descent," << std::endl
	<< "specified in per mill of the allowed or expected value" << std::endl
	<< "range of a parameter";
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBaseGD, albeit by delegating work to the broker. Items are evaluated up to the maximum position
 * in the vector. Note that we always start the evaluation with the first item in the vector.
 */
void GGradientDescent::runFitnessCalculation() {
	using namespace Gem::Courtier;

	bool complete = false;

#ifdef DEBUG
	GGradientDescent::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		// Make sure the evaluated individuals have the dirty flag set
		if(this->afterFirstIteration() && !(*it)->isDirty()) {
			glogger
				<< "In GBrokerGD::runFitnessCalculation():" << std::endl
				<< "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag isn't set" << std::endl
				<< GEXCEPTION;
		}
	}
#endif /* DEBUG */

	//--------------------------------------------------------------------------------
	// Submit all work items and wait for their return

	std::vector<bool> workItemPos(this->size(), Gem::Courtier::GBC_UNPROCESSED);
	complete = this->workOn(
		this->data
		, workItemPos
		, true // resubmit unprocessed items
		, "GBrokerGD::runFitnessCalculation()"
	);

	//--------------------------------------------------------------------------------
	// Some error checks

	// Check if all work items have returned
	if (!complete) {
		glogger
			<< "In GBrokerGD::runFitnessCalculation(): Error!" << std::endl
			<< "No complete set of items received" << std::endl
			<< GEXCEPTION;
	}

	// Check if work items exists whose processing function has thrown an exception.
	// This is a severe error, as we need evaluations for all work items in a gradient
	// descent.
	if(auto it = std::find_if(
		this->begin()
		, this->end()
		, [this](std::shared_ptr<GParameterSet> p) -> bool {
			return p->processing_was_unsuccessful();
		}
	) != this->end()) {
		glogger
			<< "In GBrokerGD::runFitnessCalculation(): Error!" << std::endl
			<< "At least one individual could not be processed" << std::endl
			<< "due to errors in the (possibly user-supplied) process() function." << std::endl
			<< "This is a severe error and we cannot continue" << std::endl
			<< GEXCEPTION;
	}

	//--------------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GGradientDescent::init() {
	// To be performed before any other action
	G_OptimizationAlgorithm_Base::init();

	// Extract the boundaries of all parameters
	this->at(0)->boundaries(dblLowerParameterBoundaries_, dblUpperParameterBoundaries_, activityMode::ACTIVEONLY);

#ifdef DEBUG
   // Size matters!
   if(dblLowerParameterBoundaries_.size() != dblUpperParameterBoundaries_.size()) {
      glogger
      << "In GGradientDescent::init(): Error!" << std::endl
      << "Found invalid sizes: "
      << dblLowerParameterBoundaries_.size() << " / " << dblUpperParameterBoundaries_.size() << std::endl
      << GEXCEPTION;
   }

   // Check that stepSize_ has an appropriate value
   if(stepSize_ <= 0. || stepSize_ > 1000.) { // Specified in per mill of the allowed or preferred value range
      glogger
      << "In GGradientDescent::init(): Error!" << std::endl
      << "Invalid values of stepSize_: " << stepSize_ << std::endl
      << "Must be in the range ]0.:1000.]" << std::endl
      << GEXCEPTION;
   }

   // Check that finiteStep_ has an appropriate value
   if(finiteStep_ <= 0. || finiteStep_ > 1000.) { // Specified in per mill of the allowed or preferred value range
      glogger
      << "In GGradientDescent::init(): Error!" << std::endl
      << "Invalid values of finiteStep_: " << finiteStep_ << std::endl
      << "Must be in the range ]0.:1000.]" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	// Set the step ratio. We do the calculation in long double precision to preserve accuracy
	stepRatio_ = ((long double) stepSize_) / ((long double) finiteStep_);

	// Calculate a specific finiteStep_ value for each parameter in long double precision
	try {
		adjustedFiniteStep_.clear();
		long double finiteStepRatio = ((long double) finiteStep_) / ((long double) 1000.);
		for (std::size_t pos = 0; pos < dblLowerParameterBoundaries_.size(); pos++) {
			long double parameterRange =
				(long double) dblUpperParameterBoundaries_[pos] - (long double) dblLowerParameterBoundaries_[pos];
			adjustedFiniteStep_.push_back(boost::numeric_cast<double>(finiteStepRatio * parameterRange));
		}
	} catch (boost::bad_numeric_cast &e) {
		glogger
		<< "In GGradientDescent::init(): Error!" << std::endl
		<< "Bad conversion with message " << e.what() << std::endl
		<< GEXCEPTION;
	}

	// Tell individuals about their position in the population
	markIndividualPositions();
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GGradientDescent::finalize() {
	// Last action
	G_OptimizationAlgorithm_Base::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr <GPersonalityTraits> GGradientDescent::getPersonalityTraits() const {
	return std::shared_ptr<G_OA_GradientDescent_PersonalityTraits>(new G_OA_GradientDescent_PersonalityTraits());
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 */
void GGradientDescent::adjustPopulation() {
	// Check how many individuals we already have
	std::size_t nStart = this->size();

	// Do some error checking ...

	// We need at least one individual
	if (nStart == 0) {
		glogger
		<< "In GGradientDescent::adjustPopulation():" << std::endl
		<< "You didn't add any individuals to the collection. We need at least one." << std::endl
		<< GEXCEPTION;
	}

	// Update the number of active floating point parameters in the individuals
	nFPParmsFirst_ = this->at(0)->countParameters<double>(activityMode::ACTIVEONLY);

	// Check that the first individual has floating point parameters (double for the moment)
	if (nFPParmsFirst_ == 0) {
		glogger
		<< "In GGradientDescent::adjustPopulation():" << std::endl
		<< "No floating point parameters in individual." << std::endl
		<< GEXCEPTION;
	}

	// Check that all individuals currently available have the same amount of parameters
#ifdef DEBUG
	for(std::size_t i=1; i<this->size(); i++) {
		if(this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) != nFPParmsFirst_) {
		   glogger
		   << "In GGradientDescent::adjustPopulation():" << std::endl
         << "Found individual in position " <<  i << " with different" << std::endl
         << "number of floating point parameters than the first one: " << this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) << "/" << nFPParmsFirst_ << std::endl
         << GEXCEPTION;
		}
	}
#endif

	// Set the default size of the population
	G_OptimizationAlgorithm_Base::setDefaultPopulationSize(nStartingPoints_ * (nFPParmsFirst_ + 1));

	// First create a suitable number of start individuals and initialize them as required
	if (nStart < nStartingPoints_) {
		for (std::size_t i = 0; i < (nStartingPoints_ - nStart); i++) {
			// Create a copy of the first individual
			this->push_back(this->at(0)->clone<GParameterSet>());
			// Make sure our start values differ
			this->back()->randomInit(activityMode::ACTIVEONLY);
		}
	} else {
		// Start with a defined size. This will remove surplus items.
		this->resize(nStartingPoints_);
	}

	// Add the required number of clones for each starting point. These will be
	// used for the calculation of the difference quotient for each parameter
	for (std::size_t i = 0; i < nStartingPoints_; i++) {
		for (std::size_t j = 0; j < nFPParmsFirst_; j++) {
			this->push_back(this->at(i)->clone<GParameterSet>());
		}
	}

	// We now should have nStartingPoints_ sets of individuals,
	// each of size nFPParmsFirst_.
#ifdef DEBUG
	if(this->size() != nStartingPoints_*(nFPParmsFirst_ + 1)) {
	   glogger
	   << "In GGradientDescent::adjustPopulation():" << std::endl
      << "Population size is " << this->size() << std::endl
      << "but expected " << nStartingPoints_*(nFPParmsFirst_ + 1) << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * This helper function lets all individuals know about their position in the
 * population.
 */
void GGradientDescent::markIndividualPositions() {
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		this->at(pos)->getPersonalityTraits<G_OA_GradientDescent_PersonalityTraits>()->setPopulationPosition(pos);
	}
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GGradientDescent::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (G_OptimizationAlgorithm_Base::modify_GUnitTests()) result = true;

	return result;
#else /* GEM_TESTING */
   condnotset("GGradientDescent::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GGradientDescent::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GGradientDescent::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GGradientDescent::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GGradientDescent::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


