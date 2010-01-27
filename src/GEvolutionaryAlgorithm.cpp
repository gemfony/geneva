/**
 * @file GEvolutionaryAlgorithm.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "GEvolutionaryAlgorithm.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GEvolutionaryAlgorithm)

namespace Gem {
namespace GenEvA {

/***********************************************************************************/
/**
 * The default constructor, As we do not have any individuals yet, we set the population
 * size, and number of parents to 0. It is the philosophy of this class not
 * to provide constructors for each and every use case. Instead, you should set
 * vital parameters, such as the population size or the parent individuals by hand.
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm()
	: GOptimizationAlgorithm()
	, nParents_(0)
	, microTrainingInterval_(DEFAULTMICROTRAININGINTERVAL)
	, recombinationMethod_(DEFAULTRECOMBINE)
	, smode_(DEFAULTSMODE)
	, defaultNChildren_(0)
	, oneTimeMuCommaNu_(false)
	, infoFunction_(&GEvolutionaryAlgorithm::simpleInfoFunction)
{ /* nothing */ }

/***********************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GEvolutionaryAlgorithm object
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm& cp)
	: GOptimizationAlgorithm(cp)
	, nParents_(cp.nParents_)
	, microTrainingInterval_(cp.microTrainingInterval_)
	, recombinationMethod_(cp.recombinationMethod_)
	, smode_(cp.smode_)
	, defaultNChildren_(cp.defaultNChildren_)
	, oneTimeMuCommaNu_(cp.oneTimeMuCommaNu_)
	, infoFunction_(cp.infoFunction_)
{ /* nothing */ }

/***********************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GEvolutionaryAlgorithm::~GEvolutionaryAlgorithm()
{ /* nothing */ }

/***********************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp Another GEvolutionaryAlgorithm object
 * @return A constant reference to this object
 */
const GEvolutionaryAlgorithm& GEvolutionaryAlgorithm::operator=(const GEvolutionaryAlgorithm& cp) {
	GEvolutionaryAlgorithm::load(&cp);
	return *this;
}

/***********************************************************************************/
/**
 * Loads the data of another GEvolutionaryAlgorithm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GEvolutionaryAlgorithm object, camouflaged as a GObject
 */
void GEvolutionaryAlgorithm::load(const GObject * cp)
{
	const GEvolutionaryAlgorithm *p_load = this->conversion_cast(cp,this);

	// First load the parent class'es data ...
	GOptimizationAlgorithm::load(cp);

	// ... and then our own data
	nParents_ = p_load->nParents_;
	microTrainingInterval_ = p_load->microTrainingInterval_;
	recombinationMethod_ = p_load->recombinationMethod_;
	smode_ = p_load->smode_;
	defaultNChildren_ = p_load->defaultNChildren_;
	oneTimeMuCommaNu_=p_load->oneTimeMuCommaNu_;
	infoFunction_ = p_load->infoFunction_;
}

/***********************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object, camouflaged as a pointer to a GObject
 */
GObject *GEvolutionaryAlgorithm::clone_() const  {
	return new GEvolutionaryAlgorithm(*this);
}

/***********************************************************************************/
/**
 * Checks for equality with another GEvolutionaryAlgorithm object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithm::operator==(const GEvolutionaryAlgorithm& cp) const {
	return GEvolutionaryAlgorithm::isEqualTo(cp,  boost::logic::indeterminate);
}

/***********************************************************************************/
/**
 * Checks for inequality with another GEvolutionaryAlgorithm object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEvolutionaryAlgorithm::operator!=(const GEvolutionaryAlgorithm& cp) const {
	return !GEvolutionaryAlgorithm::isEqualTo(cp,  boost::logic::indeterminate);
}

/***********************************************************************************/
/**
 * Checks for equality with another GEvolutionaryAlgorithm object.
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithm::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
	using namespace Gem::Util;

	// Check that we are indeed dealing with a GIndividual reference
	const GEvolutionaryAlgorithm *p_load = GObject::conversion_cast(&cp,  this);

	// First take care of our parent class
	if(!GOptimizationAlgorithm::isEqualTo( *p_load, expected)) return  false;

	// Then we take care of the local data
	if(checkForInequality("GEvolutionaryAlgorithm", nParents_, p_load->nParents_,"nParents_", "p_load->nParents_", expected)) return false;
	if(checkForInequality("GEvolutionaryAlgorithm", microTrainingInterval_, p_load->microTrainingInterval_,"microTrainingInterval_", "p_load->microTrainingInterval_", expected)) return false;
	if(checkForInequality("GEvolutionaryAlgorithm", recombinationMethod_, p_load->recombinationMethod_,"recombinationMethod_", "p_load->recombinationMethod_", expected)) return false;
	if(checkForInequality("GEvolutionaryAlgorithm", smode_, p_load->smode_,"smode_", "p_load->smode_", expected)) return false;
	if(checkForInequality("GEvolutionaryAlgorithm", defaultNChildren_, p_load->defaultNChildren_,"defaultNChildren_", "p_load->defaultNChildren_", expected)) return false;
	if(checkForInequality("GEvolutionaryAlgorithm", oneTimeMuCommaNu_, p_load->oneTimeMuCommaNu_,"oneTimeMuCommaNu_", "p_load->oneTimeMuCommaNu_", expected)) return false;

	return true;
}

/***********************************************************************************/
/**
 * Checks for similarity with another GEvolutionaryAlgorithm object.
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GEvolutionaryAlgorithm::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
	using namespace Gem::Util;

	// Check that we are indeed dealing with a GIndividual reference
	const GEvolutionaryAlgorithm *p_load = GObject::conversion_cast(&cp,  this);

	// First take care of our parent class
	if(!GOptimizationAlgorithm::isSimilarTo(*p_load, limit, expected)) return  false;

	// Then we take care of the local data
	if(checkForDissimilarity("GEvolutionaryAlgorithm", nParents_, p_load->nParents_, limit, "nParents_", "p_load->nParents_", expected)) return false;
	if(checkForDissimilarity("GEvolutionaryAlgorithm", microTrainingInterval_, p_load->microTrainingInterval_, limit, "microTrainingInterval_", "p_load->microTrainingInterval_", expected)) return false;
	if(checkForDissimilarity("GEvolutionaryAlgorithm", recombinationMethod_, p_load->recombinationMethod_, limit, "recombinationMethod_", "p_load->recombinationMethod_", expected)) return false;
	if(checkForDissimilarity("GEvolutionaryAlgorithm", smode_, p_load->smode_, limit, "smode_", "p_load->smode_", expected)) return false;
	if(checkForDissimilarity("GEvolutionaryAlgorithm", defaultNChildren_, p_load->defaultNChildren_, limit, "defaultNChildren_", "p_load->defaultNChildren_", expected)) return false;
	if(checkForDissimilarity("GEvolutionaryAlgorithm", oneTimeMuCommaNu_, p_load->oneTimeMuCommaNu_, limit, "oneTimeMuCommaNu_", "p_load->oneTimeMuCommaNu_", expected)) return false;

	return true;
}

/***********************************************************************************/
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
boost::optional<std::string> GEvolutionaryAlgorithm::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEvolutionaryAlgorithm *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithm::checkRelationshipWith(cp, e, limit, "GEvolutionaryAlgorithm", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", nParents_, p_load->nParents_, "nParents_", "p_load->nParents_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", microTrainingInterval_, p_load->microTrainingInterval_, "microTrainingInterval_", "p_load->microTrainingInterval_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", recombinationMethod_, p_load->recombinationMethod_, "recombinationMethod_", "p_load->recombinationMethod_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", smode_, p_load->smode_, "smode_", "p_load->smode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", defaultNChildren_, p_load->defaultNChildren_, "defaultNChildren_", "p_load->defaultNChildren_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", oneTimeMuCommaNu_, p_load->oneTimeMuCommaNu_, "oneTimeMuCommaNu_", "p_load->oneTimeMuCommaNu_", e , limit));

	return evaluateDiscrepancies("GEvolutionaryAlgorithm", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Sets the individual's personality types to EA
 */
void GEvolutionaryAlgorithm::setIndividualPersonalities() {
	GEvolutionaryAlgorithm::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) (*it)->setPersonality(EA);
}

/***********************************************************************************/
/**
 * Enforces a one-time selection policy of MUCOMMANU. This is used for updates of
 * the parents' structure in the optimize() function. As the quality of updated
 * parents may decrease, it is important to ensure that the next generation's parents
 * are chosen from children with new structure.
 */
void GEvolutionaryAlgorithm::setOneTimeMuCommaNu() {
	oneTimeMuCommaNu_ = true;
}

/***********************************************************************************/
/**
 * Updates the parents' structure, using their updateOnStall function.
 *
 * @return A boolean indicating whether an update was performed
 */
bool GEvolutionaryAlgorithm::updateParentStructure() {
	bool updatePerformed=false;

	GEvolutionaryAlgorithm::iterator it;
	for(it=this->begin(); it!=this->begin() + nParents_; ++it) {
		if((*it)->updateOnStall()) updatePerformed = true;
	}

	return updatePerformed;
}

/***********************************************************************************/
/**
 * Saves the state of the class to disc. The function adds the current generation
 * and the fitness to the base name. We do not save the entire population, but only
 * the best individuals, as these contain the "real" information. Note that no real
 * copying of the individual's data takes place here, as we are dealing with
 * boost::shared_ptr objects.
 */
void GEvolutionaryAlgorithm::saveCheckpoint() const {
	// Copy the nParents best individuals to a vector
	std::vector<boost::shared_ptr<Gem::GenEvA::GIndividual> > bestIndividuals;
	GEvolutionaryAlgorithm::const_iterator it;
	for(it=this->begin(); it!=this->begin() + this->getNParents(); ++it)
		bestIndividuals.push_back(*it);

#ifdef DEBUG // Cross check so we do not accidently trigger value calculation
	if(this->at(0)->isDirty()) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::saveCheckpoint():" << std::endl
			  << "Error: class member has the dirty flag set" << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}
#endif /* DEBUG */
	double newValue = this->at(0)->fitness();

	// Determine a suitable name for the output file
	std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(this->getIteration()) + "_"
		+ boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

	// Create the output stream and check that it is in good order
	std::ofstream checkpointStream(outputFile.c_str());
	if(!checkpointStream) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::saveCheckpoint(const std::string&)" << std::endl
			  << "Error: Could not open output file";
		throw geneva_error_condition(error.str());
	}

	// Write the individuals' data to disc in binary mode
	{
		boost::archive::binary_oarchive oa(checkpointStream);
		oa << boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
	} // note: explicit scope here is essential so the oa-destructor gets called

	// Make sure the stream is closed again
	checkpointStream.close();
}

/***********************************************************************************/
/**
 * Loads the state of the class from disc. We do not load the entire population,
 * but only the best individuals of a former optimization run, as these contain the
 * "real" information.
 */
void GEvolutionaryAlgorithm::loadCheckpoint(const std::string& cpFile) {
	// Create a vector to hold the best individuals
	std::vector<boost::shared_ptr<Gem::GenEvA::GIndividual> > bestIndividuals;

	// Check that the file indeed exists
	if(!boost::filesystem::exists(cpFile)) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::loadCheckpoint(const std::string&)" << std::endl
			  << "Got invalid checkpoint file name " << cpFile << std::endl;
		throw geneva_error_condition(error.str());
	}

	// Create the input stream and check that it is in good order
	std::ifstream checkpointStream(cpFile.c_str());
	if(!checkpointStream) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::loadCheckpoint(const std::string&)" << std::endl
			  << "Error: Could not open input file";
		throw geneva_error_condition(error.str());
	}

	// Load the data from disc in binary mode
	{
		boost::archive::binary_iarchive ia(checkpointStream);
	    ia >> boost::serialization::make_nvp("bestIndividuals", bestIndividuals);
	} // note: explicit scope here is essential so the ia-destructor gets called

	// Make sure the stream is closed again
	checkpointStream.close();

	// Load the individuals into this class
	std::size_t thisSize = this->size();
	std::size_t biSize = bestIndividuals.size();
	if(thisSize >= biSize) { // The most likely case
		for(std::size_t ic=0; ic<biSize; ic++) {
			(*this)[ic]->load((bestIndividuals[ic]).get());
		}
	}
	else if(thisSize < biSize) {
		for(std::size_t ic=0; ic<thisSize; ic++) {
			(*this)[ic]->load((bestIndividuals[ic]).get());
		}
		for(std::size_t ic=thisSize; ic<biSize; ic++) {
			this->push_back(bestIndividuals[ic]);
		}
	}
}

/***********************************************************************************/
/**
 * Emits information specific to this population. The function can be overloaded
 * in derived classes. By default we allow the user to register a call-back function
 * using GEvolutionaryAlgorithm::registerInfoFunction() . Please note that it is not
 * possible to serialize this function, so it will only be active on the host were
 * it was registered, but not on remote systems.
 *
 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
 */
void GEvolutionaryAlgorithm::doInfo(const infoMode& im) {
	if(!infoFunction_.empty()) infoFunction_(im, this);
}

/***********************************************************************************/
/**
 * The user can specify what information should be emitted in a call-back function
 * that is registered in the setup phase. This functionality is based on boost::function .
 *
 * @param infoFunction A Boost.function object allowing the emission of information
 */
void GEvolutionaryAlgorithm::registerInfoFunction(boost::function<void (const infoMode&, GEvolutionaryAlgorithm * const)> infoFunction) {
	infoFunction_ = infoFunction;
}

/***********************************************************************************/
/**
 * Specifies the initial size of the population plus the number of parents.
 * The population will be filled with additional individuals later, as required --
 * see GEvolutionaryAlgorithm::adjustPopulation() . Also, all error checking is done in
 * that function.
 *
 * @param popSize The desired size of the population
 * @param nParents The desired number of parents
 */
void GEvolutionaryAlgorithm::setPopulationSize(const std::size_t& popSize, const std::size_t& nParents) {
	GOptimizationAlgorithm::setPopulationSize(popSize);
	nParents_ = nParents;
}

/***********************************************************************************/
/**
 * This function implements the logic that constitutes evolutionary algorithms. The
 * function is called by GOptimizationAlgorithm for each cycle of the optimization,
 *
 * @return The value of the best individual found
 */
double GEvolutionaryAlgorithm::cycleLogic() {
	this->recombine(); // create new children from parents
	this->markIndividualPositions();
	this->mutateChildren(); // mutate children and calculate their value
	this->select(); // find out the best individuals of the population

	boost::uint32_t stallCounter = getStallCounter();
	if(microTrainingInterval_ && stallCounter && stallCounter%microTrainingInterval_ == 0) {
#ifdef DEBUG
		std::cout << "Updating parents ..." << std::endl;
#endif /* DEBUG */

		if(this->updateParentStructure()) {
			this->setOneTimeMuCommaNu();
		}
	}

	// Retrieve the fitness of the best individual in the collection
	bool isDirty = false;
	double bestFitness = this->at(0)->getCurrentFitness(isDirty);

#ifdef DEBUG
	if(isDirty) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::cycleLogic(): Found dirty individual when it should not be" << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}
#endif /* DEBUG */

	return bestFitness;
}


/***********************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithm::optimize(), before the
 * actual optimization cycle starts.
 */
void GEvolutionaryAlgorithm::init() {
	// To be performed before any other action
	GOptimizationAlgorithm::init();

	// First check that we have been given a suitable value for the number of parents.
	// Note that a number of checks (e.g. population size != 0) has already been done
	// in the parent class.
	if(nParents_ == 0) {
		nParents_ = 1;
	}

	// In MUCOMMANU mode we want to have at least as many children as parents,
	// whereas MUPLUSNU only requires the population size to be larger than the
	// number of parents. NUNU1PRETAIN has the same requirements as MUCOMMANU,
	// as it is theoretically possible that all children are better than the former
	// parents, so that the first parent individual will be replaced.
	std::size_t popSize = getPopulationSize();
	if(((smode_==MUCOMMANU || smode_==MUNU1PRETAIN) && (popSize < 2*nParents_)) || (smode_==MUPLUSNU && popSize<=nParents_))
	{
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::adjustPopulation() : Error!" << std::endl
			  << "Requested size of population is too small :" << popSize << " " << nParents_ << std::endl
		      << "Sorting scheme is ";

		switch(smode_) {
		case MUPLUSNU:
			error << "MUPLUSNU" << std::endl;
			break;
		case MUCOMMANU:
			error << "MUCOMMANU" << std::endl;
			break;
		case MUNU1PRETAIN:
			error << "MUNU1PRETAIN" << std::endl;
			break;
		}

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_error_condition(error.str());
	}

	// Let parents know they are parents and children that they are children
	markParents();

	// Make sure derived classes (such as GTransferPopulation) have a way of finding out
	// what the desired number of children is. This is particularly important, if, in a
	// network environment, some individuals might not return and some individuals return
	// late. The factual size of the population then changes and we need to take action.
	defaultNChildren_ = getDefaultPopulationSize() - nParents_;
}

/***********************************************************************************/
/**
 * Does any necessary finalization work
 */
void GEvolutionaryAlgorithm::finalize() {
	// Last action
	GOptimizationAlgorithm::finalize();
}

/***********************************************************************************/
/**
 * Set the interval in which micro training should be performed. Set the
 * interval to 0 in order to prevent micro training.
 *
 * @param mti The desired new value of the mircoTrainingInterval_ variable
 */
void GEvolutionaryAlgorithm::setMicroTrainingInterval(const boost::uint32_t& mti) {
	microTrainingInterval_ = mti;
}

/***********************************************************************************/
/**
 * Retrieve the interval in which micro training should be performed
 *
 * @return The current value of the mircoTrainingInterval_ variable
 */
boost::uint32_t GEvolutionaryAlgorithm::getMicroTrainingInterval() const {
	return microTrainingInterval_;
}

/***********************************************************************************/
/**
 * Retrieve the number of parents as set by the user. This is a fixed parameter and
 * should not be changed after it has first been set.
 *
 * @return The number of parents in the population
 */
std::size_t GEvolutionaryAlgorithm::getNParents() const {
	return nParents_;
}

/***********************************************************************************/
/**
 * Calculates the current number of children from the number of parents and the
 * size of the vector.
 *
 * @return The number of children in the population
 */
std::size_t GEvolutionaryAlgorithm::getNChildren() const {
	return data.size() - nParents_;
}

/***********************************************************************************/
/**
 * Sets the sorting scheme. In MUPLUSNU, new parents will be selected from the entire
 * population, including the old parents. In MUCOMMANU new parents will be selected
 * from children only. MUNU1PRETAIN means that the best parent of the last generation
 * will also become a new parent (unless a better child was found). All other parents are
 * selected from children only.
 *
 * @param smode The desired sorting scheme
 */
void GEvolutionaryAlgorithm::setSortingScheme(const sortingMode& smode) {
	smode_=smode;
}

/***********************************************************************************/
/**
 * Retrieves information about the current sorting scheme (see
 * GEvolutionaryAlgorithm::setSortingScheme() for further information).
 *
 * @return The current sorting scheme
 */
sortingMode GEvolutionaryAlgorithm::getSortingScheme() const {
	return smode_;
}

/***********************************************************************************/
/**
 * This function is called from GOptimizationAlgorithm::optimize() and performs the
 * actual recombination, based on the recombination schemes defined by the user.
 *
 * Note that, in DEBUG mode, this implementation will enforce a minimum number of children,
 * as implied by the initial sizes of the population and the number of parents
 * present. If individuals can get lost in your setting, you must add mechanisms
 * to "repair" the population.
 */
void GEvolutionaryAlgorithm::recombine()
{
#ifdef DEBUG
	// We require at this stage that at least the default number of
	// children is present. If individuals can get lost in your setting,
	// you must add mechanisms to "repair" the population.
	if((data.size()-nParents_) < defaultNChildren_){
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::recombine(): Error!" << std::endl
			  << "Too few children. Got " << data.size()-nParents_ << "," << std::endl
			  << "but was expecting at least " << defaultNChildren_ << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_error_condition(error.str());
	}
#endif

	// Do the actual recombination
	this->doRecombine();

	// Let children know they are children
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin()+nParents_; it!=data.end(); ++it){
		(*it)->getEAPersonalityTraits()->setIsChild();
	}
}

/***********************************************************************************/
/**
 * This function assigns a new value to each child individual according to the chosen
 * recombination scheme.
 */
void GEvolutionaryAlgorithm::doRecombine() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	switch(recombinationMethod_){
	case DEFAULTRECOMBINE: // we want the RANDOMRECOMBINE behavior
	case RANDOMRECOMBINE:
		for(it=data.begin()+nParents_; it!= data.end(); ++it) randomRecombine(*it);
		break;

	case VALUERECOMBINE:
		// Recombination according to the parents' fitness only makes sense if
		// we have at least 2 parents. We do the recombination manually otherwise
		if(nParents_==1) {
			for(it=data.begin()+1; it!= data.end(); ++it)
				(*it)->load((data.begin())->get());
		}
		else {
			// TODO: Check whether it is sufficient to do this only once

			// Calculate a vector of recombination likelihoods for all parents
			std::size_t i;
			std::vector<double> threshold(nParents_);
			double thresholdSum=0.;
			for(i=0; i<nParents_; i++) {
#ifdef DEBUG
				thresholdSum += 1./(boost::numeric_cast<double>(i)+2.);
#else
				thresholdSum += 1./(static_cast<double>(i)+2.);
#endif /* DEBUG */
			}
			for(i=0; i<nParents_-1; i++) {
				// normalising the sum to 1
#ifdef DEBUG
				threshold[i] = (1./(boost::numeric_cast<double>(i)+2.)) / thresholdSum;
#else
				threshold[i] = (1./(static_cast<double>(i)+2.)) / thresholdSum;
#endif /* DEBUG */

				// Make sure the subsequent range is in the right position
				if(i>0) threshold[i] += threshold[i-1];
			}
			threshold[nParents_-1] = 1.; // Necessary due to rounding errors

			// Do the actual recombination
			for(it=data.begin()+nParents_; it!= data.end(); ++it) {
				// A recombination taking into account the value does not make
				// sense in generation 0, as parents might not have a suitable
				// value. Instead, this function might accidently trigger value
				// calculation in this case. Hence we fall back to random
				// recombination in generation 0. No value calculation takes
				// place there.
				if(getIteration() == 0) randomRecombine(*it);
				else valueRecombine(*it, threshold);
			}
		}

		break;
	}
}

/***********************************************************************************/
/**
 * This function implements the RANDOMRECOMBINE scheme. This functions uses BOOST's
 * numeric_cast function for safe conversion between std::size_t and uint16_t.
 *
 * @param pos The position of the individual for which a new value should be chosen
 */
void GEvolutionaryAlgorithm::randomRecombine(boost::shared_ptr<GIndividual>& p) {
	std::size_t p_pos;

	// Choose a parent to be used for the recombination. Note that
	// numeric_cat may throw. Exceptions need to be caught in surrounding functions.
	// try/catch blocks would add a non-negligible overhead in this function.
	p_pos = boost::numeric_cast<std::size_t>(gr.discreteRandom(nParents_));

	p->load((data.begin() + p_pos)->get());
}

/***********************************************************************************/
/**
 * This function implements the VALUERECOMBINE scheme. The range [0.,1.[ is divided
 * into nParents_ sub-areas with different size (the largest for the first parent,
 * the smallest for the last). Parents are chosen for recombination according to a
 * random number evenly distributed between 0 and 1. This way parents with higher
 * fitness are more likely to be chosen for recombination.
 *
 * @param pos The child individual for which a parent should be chosen
 * @param threshold A std::vector<double> holding the recombination likelihoods for each parent
 */
void GEvolutionaryAlgorithm::valueRecombine(boost::shared_ptr<GIndividual>& p, const std::vector<double>& threshold) {
	bool done=false;
	std::size_t i;
	double randTest = gr.evenRandom(); // get the test value

	for(i=0; i<nParents_; i++) {
		if(randTest<threshold[i]) {
			p->load((data.begin() + i)->get());
			done = true;

			break;
		}
	}

	if(!done) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::valueRecombine(): Error!" << std::endl
			  << "Could not recombine." << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_error_condition(error.str());
	}
}

/***********************************************************************************/
/**
 * Mutate all children in sequence. Note that this also triggers their value
 * calculation, so this function needs to be overloaded for optimization in a
 * network context.
 */
void GEvolutionaryAlgorithm::mutateChildren()
{
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We need to make sure that fitness calculation is
	// triggered for all parents. Note that it may well be that at
	// this stage we have several identical parents in the population,
	// due to the actions of the adjustPopulation function.
	if(getIteration() == 0) {
		for(it=data.begin(); it!=data.begin()+nParents_; ++it) {
			(*it)->fitness();
		}
	}

	// Next we perform the mutation of each child individual in
	// sequence. Note that this can also trigger fitness calculation.
	for(it=data.begin()+nParents_; it!=data.end(); ++it) (*it)->mutate();
}

/***********************************************************************************/
/**
 * Choose new parents, based on the selection scheme set by the user.
 */
void GEvolutionaryAlgorithm::select()
{
#ifdef DEBUG
	// We require at this stage that at least the default number of
	// children is present. If individuals can get lost in your setting,
	// you must add mechanisms to "repair" the population.
	if((data.size()-nParents_) < defaultNChildren_){
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::select(): Error!" << std::endl
			  << "Too few children. Got " << data.size()-nParents_ << "," << std::endl
			  << "but was expecting at least " << defaultNChildren_ << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_error_condition(error.str());
	}
#endif /* DEBUG */

	switch(smode_) {
	//----------------------------------------------------------------------------
	case MUPLUSNU:
		if(oneTimeMuCommaNu_) {
			this->sortMucommanuMode();
			oneTimeMuCommaNu_=false;
		}
		else this->sortMuplusnuMode();
		break;

	//----------------------------------------------------------------------------
	case MUNU1PRETAIN:
		if(oneTimeMuCommaNu_) {
			this->sortMucommanuMode();
			oneTimeMuCommaNu_=false;
		}
		else this->sortMunu1pretainMode();
		break;

	//----------------------------------------------------------------------------
	case MUCOMMANU:
		this->sortMucommanuMode();
		break;
	//----------------------------------------------------------------------------
	}

	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.begin()+nParents_; ++it) {
		(*it)->getEAPersonalityTraits()->setIsParent();
	}
}

/***********************************************************************************/
/**
 * Selection, MUPLUSNU style. All individuals of the population (including parents)
 * are sorted. The quality of the population can only increase, but the optimization
 * will stall more easily.
 */
void GEvolutionaryAlgorithm::sortMuplusnuMode() {
	// Sort the entire array
	if(getMaximize()){
		std::partial_sort(data.begin(), data.begin() + nParents_, data.end(),
				boost::bind(&GIndividual::fitness, _1) > boost::bind(&GIndividual::fitness, _2));
	}
	else{
		std::partial_sort(data.begin(), data.begin() + nParents_, data.end(),
				boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
	}
}

/***********************************************************************************/
/**
 * Selection, MUCOMMANU style. New parents are selected from children only. The quality
 * of the population may decrease occasionally from generation to generation, but the
 * optimization is less likely to stall.
 */
void GEvolutionaryAlgorithm::sortMucommanuMode() {
	// Only sort the children
	if(getMaximize()){
		std::partial_sort(data.begin() + nParents_, data.begin() + 2*nParents_, data.end(),
			  boost::bind(&GIndividual::fitness, _1) > boost::bind(&GIndividual::fitness, _2));
	}
	else{
		std::partial_sort(data.begin() + nParents_, data.begin() + 2*nParents_, data.end(),
			  boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
	}
	std::swap_ranges(data.begin(),data.begin()+nParents_,data.begin()+nParents_);
}

/***********************************************************************************/
/**
 * Selection, MUNU1PRETAIN style. This is a hybrid between MUPLUSNU and MUCOMMANU
 * mode. If a better child was found than the best parent of the last generation,
 * all former parents are replaced. If no better child was found than the best
 * parent of the last generation, then this parent stays in place. All other parents
 * are replaced by the (nParents_-1) best children. The scheme falls back to MUPLUSNU
 * mode, if only one parent is available, or if this is the first generation (so we
 * do not accidentally trigger value calculation).
 */
void GEvolutionaryAlgorithm::sortMunu1pretainMode() {
	if(nParents_==1 || getIteration()==0) { // Falls back to MUPLUSNU mode
		this->sortMuplusnuMode();
	} else {
		// Sort the children
		if(getMaximize()){
			std::partial_sort(data.begin() + nParents_, data.begin() + 2*nParents_, data.end(),
				  boost::bind(&GIndividual::fitness, _1) > boost::bind(&GIndividual::fitness, _2));
		}
		else{
			std::partial_sort(data.begin() + nParents_, data.begin() + 2*nParents_, data.end(),
				  boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
		}

		// Retrieve the best child's and the last generation's best parent's fitness
		double bestChildFitness = (*(data.begin() + nParents_))->fitness();
		double bestParentFitness = (*(data.begin()))->fitness();

		// Leave the best parent in place, if no better child was found
		if(!this->isBetter(bestChildFitness, bestParentFitness)) {
			std::swap_ranges(data.begin()+1,data.begin()+nParents_,data.begin()+nParents_);
		} else { // A better child was found. Overwrite all parents
			std::swap_ranges(data.begin(),data.begin()+nParents_,data.begin()+nParents_);
		}
	}
}

/***********************************************************************************/
/**
 * This helper function marks parents as parents and children as children.
 */
void GEvolutionaryAlgorithm::markParents() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.begin()+nParents_; ++it){
		(*it)->getEAPersonalityTraits()->setIsParent();
	}

	for(it=data.begin()+nParents_; it!=data.end(); ++it){
		(*it)->getEAPersonalityTraits()->setIsChild();
	}
}

/***********************************************************************************/
/**
 * This helper function lets all individuals know about their position in the
 * population.
 */
void GEvolutionaryAlgorithm::markIndividualPositions() {
	std::size_t pos = 0;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->getEAPersonalityTraits()->setPopulationPosition(pos++);
}

/***********************************************************************************/
/**
 * Retrieves the defaultNChildren_ parameter. E.g. in GTransferPopulation::mutateChildren() ,
 * this factor controls when a population is considered to be complete. The corresponding
 * loop which waits for new arrivals will then be stopped, which in turn allows
 * a new generation to start.
 *
 * @return The defaultNChildren_ parameter
 */
std::size_t GEvolutionaryAlgorithm::getDefaultNChildren() const {
	return defaultNChildren_;
}

/***********************************************************************************/
/**
 * Lets the user set the desired recombination method. No sanity checks for the
 * values are necessary, as we use an enum.
 *
 * @param recombinationMethod The desired recombination method
 */
void GEvolutionaryAlgorithm::setRecombinationMethod(const recoScheme& recombinationMethod) {
	recombinationMethod_ = recombinationMethod;
}

/***********************************************************************************/
/**
 * Retrieves the value of the recombinationMethod_ variable
 *
 * @return The value of the recombinationMethod_ variable
 */
recoScheme GEvolutionaryAlgorithm::getRecombinationMethod() const {
	return recombinationMethod_;
}

/***********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
