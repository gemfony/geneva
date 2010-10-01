/**
 * @file GGradientDescent.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva/GGradientDescent.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GGradientDescent)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor
 */
GGradientDescent::GGradientDescent()
	: GOptimizationAlgorithmT<GParameterSet>()
	, infoFunction_(&GGradientDescent::simpleInfoFunction)
	, nStartingPoints_(DEFAULTGDSTARTINGPOINTS)
	, nFPParmsFirst_(0)
	, finiteStep_(DEFAULTFINITESTEP)
	, stepSize_(DEFAULTSTEPSIZE)
{
	GOptimizationAlgorithmT<GParameterSet>::setOptimizationAlgorithm(GD);
}

/************************************************************************************************************/
/**
 * Initialization with the number of starting points and other parameters
 *
 * @param nStartingPoints The number of simultaneous starting points for the gradient descent
 * @param finiteStep The desired size of the incremental adaption process
 * @param stepSize The size of the multiplicative factor of the adaption process
 */
GGradientDescent::GGradientDescent(
		const std::size_t& nStartingPoints
		, const float& finiteStep
		, const float& stepSize
)
	: GOptimizationAlgorithmT<GParameterSet>()
	, infoFunction_(&GGradientDescent::simpleInfoFunction)
	, nStartingPoints_(nStartingPoints)
	, nFPParmsFirst_(0)
	, finiteStep_(finiteStep)
	, stepSize_(stepSize)
{
	GOptimizationAlgorithmT<GParameterSet>::setOptimizationAlgorithm(GD);
}

/************************************************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp A copy of another GRadientDescent object
 */
GGradientDescent::GGradientDescent(const GGradientDescent& cp)
	: GOptimizationAlgorithmT<GParameterSet>(cp)
	, infoFunction_(&GGradientDescent::simpleInfoFunction)
	, nStartingPoints_(cp.nStartingPoints_)
	, nFPParmsFirst_(cp.nFPParmsFirst_)
	, finiteStep_(cp.finiteStep_)
	, stepSize_(cp.stepSize_)
{
	// Copying / setting of the optimization algorithm id is done by the parent class
}

/************************************************************************************************************/
/**
 * The destructor
 */
GGradientDescent::~GGradientDescent()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Retrieves the number of starting points of the algorithm
 *
 * @return The number of simultaneous starting points of the gradient descent
 */
std::size_t GGradientDescent::getNStartingPoints() const {
	return nStartingPoints_;
}

/************************************************************************************************************/
/**
 * Allows to set the number of starting points for the gradient descent
 *
 * @param nStartingPoints The desired number of starting points for the gradient descent
 */
void GGradientDescent::setNStartingPoints(const std::size_t& nStartingPoints) {
	// Do some error checking
	if(nStartingPoints == 0) {
		std::ostringstream error;
		error << "In GGradientDescent::setNStartingPoints(const std::size_t&): Error!" << std::endl
			  << "Got invalid number of starting points." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	nStartingPoints_ = nStartingPoints;
}

/************************************************************************************************************/
/**
 * Set the size of the finite step of the adaption process
 *
 * @param finiteStep The desired size of the adaption
 */
void GGradientDescent::setFiniteStep(const float& finiteStep) {
	// Do some error checking
	if(finiteStep <= 0.) {
		std::ostringstream error;
		error << "In GGradientDescent::setFiniteStep(const float&): Error!" << std::endl
			  << "Got invalid finite step size: " << finiteStep << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	finiteStep_ = finiteStep;
}

/************************************************************************************************************/
/**
 * Retrieve the size of the finite step of the adaption process
 *
 * @return The current finite step size
 */
float GGradientDescent::getFiniteStep() const {
	return finiteStep_;
}

/************************************************************************************************************/
/**
 * Sets a multiplier for the adaption process
 *
 * @param stepSize A multiplicative factor for the adaption process
 */
void GGradientDescent::setStepSize(const float& stepSize) {
	// Do some error checking
	if(stepSize <= 0.) {
		std::ostringstream error;
		error << "In GGradientDescent::setStepSize(const float&): Error!" << std::endl
			  << "Got invalid step size: " << stepSize << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	stepSize_ = stepSize;
}

/************************************************************************************************************/
/**
 * Retrieves the current step size
 *
 * @return The current valze of the step size
 */
float GGradientDescent::getStepSize() const {
	return stepSize_;
}

/************************************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GRadientDescent object
 */
const GGradientDescent& GGradientDescent::operator=(const GGradientDescent& cp) {
	GGradientDescent::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Checks for equality with another GGradientDescent object
 *
 * @param cp A copy of another GRadientDescent object
 */
bool GGradientDescent::operator==(const GGradientDescent& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GGradientDescent::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GGradientDescent object
 *
 * @param cp A copy of another GRadientDescent object
 */
bool GGradientDescent::operator!=(const GGradientDescent& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GGradientDescent::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks whether this object fulfills a given expectation in relation to another object
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GGradientDescent::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GGradientDescent *p_load = GObject::conversion_cast<GGradientDescent>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
    deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GOptimizationAlgorithmT<GParameterSet>", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GGradientDescent", nStartingPoints_, p_load->nStartingPoints_, "nStartingPoints_", "p_load->nStartingPoints_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GGradientDescent", nFPParmsFirst_, p_load->nFPParmsFirst_, "nFPParmsFirst_", "p_load->nFPParmsFirst_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GGradientDescent", finiteStep_, p_load->finiteStep_, "finiteStep_", "p_load->finiteStep_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GGradientDescent", stepSize_, p_load->stepSize_, "stepSize_", "p_load->stepSize_", e , limit));

	return evaluateDiscrepancies("GGradientDescent", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Emits information specific to this population
 *
 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
 */
void GGradientDescent::doInfo(const infoMode& im) {
	if(!infoFunction_.empty()) infoFunction_(im, this);
}

/************************************************************************************************************/
/**
 * Registers a function to be called when emitting information from doInfo
 *
 * @param infoFunction A Boost.function object allowing the emission of information
 */
void GGradientDescent::registerInfoFunction(boost::function<void (const infoMode&, GGradientDescent * const)> infoFunction) {
	infoFunction_ = infoFunction;
}

/************************************************************************************************************/
/**
 * Loads a checkpoint from disk
 *
 * @param cpFile The name of the file the checkpoint should be loaded from
 */
void GGradientDescent::loadCheckpoint(const std::string& cpFile) {
	// Create a vector to hold the best individuals
	std::vector<boost::shared_ptr<Gem::Geneva::GParameterSet> > bestIndividuals;

	// Check that the file indeed exists
	if(!boost::filesystem::exists(cpFile)) {
		std::ostringstream error;
		error << "In GGradientDescent::loadCheckpoint(const std::string&)" << std::endl
			  << "Got invalid checkpoint file name " << cpFile << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Create the input stream and check that it is in good order
	std::ifstream checkpointStream(cpFile.c_str());
	if(!checkpointStream) {
		std::ostringstream error;
		error << "In GGradientDescent::loadCheckpoint(const std::string&)" << std::endl
			  << "Error: Could not open input file";
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	switch(getCheckpointSerializationMode()) {
	case Gem::Common::SERIALIZATIONMODE_TEXT:
		// Load the data from disc in text mode
		{
			boost::archive::text_iarchive ia(checkpointStream);
		    ia >> boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
		} // note: explicit scope here is essential so the ia-destructor gets called
		break;

	case Gem::Common::SERIALIZATIONMODE_XML:
		// Load the data from disc in xml mode
		{
			boost::archive::xml_iarchive ia(checkpointStream);
		    ia >> boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
		} // note: explicit scope here is essential so the ia-destructor gets called
		break;

	case Gem::Common::SERIALIZATIONMODE_BINARY:
		// Load the data from disc in binary mode
		{
			boost::archive::binary_iarchive ia(checkpointStream);
		    ia >> boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
		} // note: explicit scope here is essential so the ia-destructor gets called
		break;
	}

	// Make sure the stream is closed again
	checkpointStream.close();

	// Load the individuals into this class
	std::size_t thisSize = this->size();
	std::size_t biSize = bestIndividuals.size();
	if(thisSize >= biSize) { // The most likely case
		for(std::size_t ic=0; ic<biSize; ic++) {
			(*this)[ic]->GObject::load(bestIndividuals[ic]);
		}
	}
	else if(thisSize < biSize) {
		for(std::size_t ic=0; ic<thisSize; ic++) {
			(*this)[ic]->GObject::load(bestIndividuals[ic]);
		}
		for(std::size_t ic=thisSize; ic<biSize; ic++) {
			this->push_back(bestIndividuals[ic]);
		}
	}
}

/************************************************************************************************************/
/**
 * Loads the data of another population
 *
 * @param cp A pointer to another GGradientDescent object, camouflaged as a GObject
 */
void GGradientDescent::load_(const GObject *cp) {
	// Make a note of the current iteration (needed for a check below).
	// The information would otherwise be lost after the load call below
	boost::uint32_t currentIteration = this->getIteration();

	const GGradientDescent *p_load = this->conversion_cast<GGradientDescent>(cp);

	// First load the parent class'es data.
	// This will also take care of copying all individuals.
	GOptimizationAlgorithmT<GParameterSet>::load_(cp);

	// ... and then our own data
	nStartingPoints_ = p_load->nStartingPoints_;
	nFPParmsFirst_ = p_load->nFPParmsFirst_;
	finiteStep_ = p_load->finiteStep_;
	stepSize_ = p_load->stepSize_;

	// Note that we do not copy the info function
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a pointer to a GObject
 */
GObject *GGradientDescent::clone_() const {
	return new GGradientDescent(*this);
}

/************************************************************************************************************/
/**
 * Sets the individual's personality types to GradientDescent
 */
void GGradientDescent::setIndividualPersonalities() {
	for(GGradientDescent::iterator it=this->begin(); it!=this->end(); ++it) {
		(*it)->setPersonality(GD);
	}
}

/************************************************************************************************************/
/**
 * The actual business logic to be performed during each iteration. Returns the best achieved fitness
 *
 * @return The value of the best individual found
 */
double GGradientDescent::cycleLogic() {
	// Update the individual parameters in each dimension of the "children"
	this->updateChildParameters();

	// Trigger value calculation for all individuals (including parents)
	double bestFitness = doFitnessCalculation(this->size());

	// Update the parameters of the parent individuals
	this->updateParentIndividuals();

	return bestFitness;
}

/************************************************************************************************************/
/**
 * Updates the individual parameters of children
 */
void GGradientDescent::updateChildParameters() {
	// Loop over all starting points
	for(std::size_t i=0; i<nStartingPoints_; i++) {
		// Extract the fp vector
		std::vector<double> parmVec;
		this->at(i)->streamline(parmVec);

		// Loop over all directions
		for(std::size_t j=0; j<nFPParmsFirst_; j++) {
			// Load the current "parent" into the "child"
			this->at((i+1)*nStartingPoints_ + j)->load(this->at(i));

			// Make a note of the current parameter's value
			double origParmVal = parmVec[j];

			// Add the finite step to the feature vector's current parameter
			parmVec[j] += finiteStep_;

			// Attach the feature vector to the child individual
			this->at((i+1)*nStartingPoints_ + j)->assignValueVector(parmVec);

			// Restore the original value in the feature vector
			parmVec[j] = origParmVal;
		}
	}
}

/**********************************************************************************************************/
/**
 * Performs a step of the parent individuals
 */
void GGradientDescent::updateParentIndividuals() {
	for(std::size_t i=0; i<nStartingPoints_; i++) {
		// Extract the fp vector
		std::vector<double> parmVec;
		this->at(i)->streamline(parmVec);

#ifdef DEBUG
		// Make sure the parents are clean
		if(this->at(i)->isDirty()) {
			std::ostringstream error;
			error << "In GGradientDescent::updateParentIndividuals(): Error!" << std::endl
				  << "Found individual in position " << i << " with active dirty flag" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif /* DEBUG*/

		// Retrieve the fitness of the individual again
		double parentFitness = this->at(i)->fitness();

		// Calculate the adaption of each parameter
		double step = 0.;
		for(std::size_t j=0; j<nFPParmsFirst_; j++) {
			step = (1./finiteStep_) * (this->at((i+1)*nStartingPoints_ + j)->fitness() - parentFitness);

			if(getMaximize()) {
				parmVec[j] += stepSize_*step;
			}
			else { // Minimization
				parmVec[j] -= stepSize_*step;
			}
		}

		// Load the parameter vector back into the parent
		this->at(i)->assignValueVector(parmVec);
	}
}

/************************************************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GGradientDescent::init() {
	// To be performed before any other action
	GOptimizationAlgorithmT<GParameterSet>::init();

	// Tell individuals about their position in the population
	markIndividualPositions();
}

/************************************************************************************************************/
/**
 * Does any necessary finalization work
 */
void GGradientDescent::finalize() {
	// Make sure the fitness of the parent individuals is calculated in the final iteration
	checkpoint(ifProgress(doFitnessCalculation(nStartingPoints_)));

	// Last action
	GOptimizationAlgorithmT<GParameterSet>::finalize();
}

/************************************************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function can be overloaded to perform
 * fitness calculation in parallel, using threads or network communication.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
double GGradientDescent::doFitnessCalculation(const std::size_t& finalPos) {
	double bestFitness = getWorstCase(); // Holds the best fitness found so far

#ifdef DEBUG
	if(finalPos > this->size()) {
		std::ostringstream error;
		error << "In GGradientDescent::doFitnessCalculation(const std::size_t&): Error!" << std::endl
			  << "Got invalid final position: " << finalPos << "/" << this->size() << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	if(finalPos < nStartingPoints_) {
		std::ostringstream error;
		error << "In GGradientDescent::doFitnessCalculation(const std::size_t&): Error!" << std::endl
			  << "We require finalPos to be at least " << nStartingPoints_ << ", but got " << finalPos << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	// Trigger value calculation for all individuals (including parents)
	double fitnessFound = 0.;
	for(std::size_t i=0; i<finalPos; i++) {
#ifdef DEBUG
		// Make sure the evaluated individuals have the dirty flag set
		if(!this->at(i)->isDirty()) {
			std::ostringstream error;
			error << "In GGradientDescent::doFitnessCalculation(const std::size_t&): Error!" << std::endl
				  << "Found individual in position " << i << " whose dirty flag isn't set" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif /* DEBUG*/

		fitnessFound = this->at(i)->fitness();

		// Update the best fitness value found
		if(i<nStartingPoints_) {
			if(isBetter(fitnessFound, bestFitness)) {
				bestFitness = fitnessFound;
			}
		}
	}

	return bestFitness;
}

/************************************************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 */
void GGradientDescent::adjustPopulation() {
	// Check how man individuals we already have
	std::size_t nStart = this->size();

	// Do some error checking ...

	// We need at least one individual
	if(nStart == 0) {
		std::ostringstream error;
		error << "In GGradientDescent::adjustPopulation(): Error!" << std::endl
			  << "You didn't add any individuals to the collection. We need at least one." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// Update the number of floating point parameters in the individuals
	nFPParmsFirst_ = this->at(0)->countParameters<double>();

	// Check that the first individual has floating point parameters (double for the moment)
	if(nFPParmsFirst_ == 0) {
		std::ostringstream error;
		error << "In GGradientDescent::adjustPopulation(): Error!" << std::endl
			  << "No floating point parameters in individual." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	// Check that all individuals currently available have the same amount of parameters
	for(std::size_t i=1; i<this->size(); i++) {
		if(this->at(i)->countParameters<double>() != nFPParmsFirst_) {
			std::ostringstream error;
			error << "In GGradientDescent::adjustPopulation(): Error!" << std::endl
				  << "Found individual in position " <<  i << " with different" << std::endl
				  << "number of floating point parameters than the first one: " << this->at(i)->countParameters<double>() << "/" << nFPParmsFirst_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
	}

	// Set the default size of the population
	GOptimizationAlgorithmT<GParameterSet>::setDefaultPopulationSize((nStartingPoints_+1)*nFPParmsFirst_);

	// First create a suitable number of start individuals and initialize them as required
	if(nStart < nStartingPoints_) {
		for(std::size_t i=0; i<(nStartingPoints_-nStart); i++) {
			// Create a copy of the first individual
			this->push_back(this->at(0)->clone<GParameterSet>());
			// Make sure our start values differ
			this->back()->randomInit();
		}
	}

	// Start with a defined size. This will remove surplus items.
	this->resize(nStartingPoints_);

	// Add the required number of clones for each starting point
	for(std::size_t i=0; i<nStartingPoints_; i++) {
		for(std::size_t j=0; j<nFPParmsFirst_; j++) {
			this->push_back(this->at(i)->clone<GParameterSet>());
		}
	}

	// We now should have nStartingPoints_ sets of identical individuals,
	// each of size nFPParmsFirst_.
}

/************************************************************************************************************/
/**
 * Saves the state of the class to disc.
 */
void GGradientDescent::saveCheckpoint() const {
	// Copy the parent individuals to a vector
	std::vector<boost::shared_ptr<Gem::Geneva::GParameterSet> > bestIndividuals;
	GGradientDescent::const_iterator it;
	for(it=this->begin(); it!=this->begin() + nStartingPoints_; ++it) {
		bestIndividuals.push_back(*it);
	}

	// Determine a suitable name for the output file
	std::string outputFile =
			getCheckpointDirectory() +
			(this->halted()?"final":boost::lexical_cast<std::string>(getIteration())) +
			"_"	+
			boost::lexical_cast<std::string>(getBestFitness()) +
			"_"	+
			getCheckpointBaseName();

	// Create the output stream and check that it is in good order
	std::ofstream checkpointStream(outputFile.c_str());
	if(!checkpointStream) {
		std::ostringstream error;
		error << "In GGradientDescent::saveCheckpoint()" << std::endl
			  << "Error: Could not open output file " << outputFile.c_str() << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	switch(getCheckpointSerializationMode()) {
	case Gem::Common::SERIALIZATIONMODE_TEXT:
		// Write the individuals' data to disc in text mode
		{
			boost::archive::text_oarchive oa(checkpointStream);
			oa << boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
		} // note: explicit scope here is essential so the oa-destructor gets called
		break;

	case Gem::Common::SERIALIZATIONMODE_XML:
		// Write the individuals' data to disc in XML mode
		{
			boost::archive::xml_oarchive oa(checkpointStream);
			oa << boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
		} // note: explicit scope here is essential so the oa-destructor gets called
		break;

	case Gem::Common::SERIALIZATIONMODE_BINARY:
		// Write the individuals' data to disc in binary mode
		{
			boost::archive::binary_oarchive oa(checkpointStream);
			oa << boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
		} // note: explicit scope here is essential so the oa-destructor gets called
		break;
	}

	// Make sure the stream is closed again
	checkpointStream.close();
}

/************************************************************************************************************/
/**
 * This helper function lets all individuals know about their position in the
 * population.
 */
void GGradientDescent::markIndividualPositions() {
	std::size_t pos = 0;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->getGDPersonalityTraits()->setPopulationPosition(pos++);
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GGradientDescent::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GParameterSet>::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GGradientDescent::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GGradientDescent::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::specificTestsFailuresExpected_GUnitTests();
}
#endif /* GENEVATESTING */

/************************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


/************************************************************************************************************/
#ifdef GENEVATESTING
/**
 * A factory function that emits a GGradientDescent object
 */
template <> boost::shared_ptr<Gem::Geneva::GGradientDescent> TFactory_GUnitTests<Gem::Geneva::GGradientDescent>() {
	using namespace Gem::Tests;
	boost::shared_ptr<Gem::Geneva::GGradientDescent> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GGradientDescent>(new Gem::Geneva::GGradientDescent()));
	p->push_back(boost::shared_ptr<GTestIndividual1>(new GTestIndividual1()));
	return p;
}
#endif /* GENEVATESTING */

