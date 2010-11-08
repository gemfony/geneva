/**
 * @file GEvolutionaryAlgorithm.cpp
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
#include "geneva/GEvolutionaryAlgorithm.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GEvolutionaryAlgorithm)
BOOST_CLASS_EXPORT(Gem::Geneva::GEvolutionaryAlgorithm::GEAOptimizationMonitor)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor, As we do not have any individuals yet, we set the population
 * size, and number of parents to 0. It is the philosophy of this class not
 * to provide constructors for each and every use case. Instead, you should set
 * vital parameters, such as the population size or the parent individuals by hand.
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm()
	: GOptimizationAlgorithmT<Gem::Geneva::GIndividual>()
	, nParents_(0)
	, microTrainingInterval_(DEFAULTMICROTRAININGINTERVAL)
	, recombinationMethod_(DEFAULTRECOMBINE)
	, smode_(DEFAULTSMODE)
	, defaultNChildren_(0)
	, oneTimeMuCommaNu_(false)
	, logOldParents_(DEFAULTMARKOLDPARENTS)
{
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::setOptimizationAlgorithm(EA);

	// Register the default optimization monitor
	this->registerOptimizationMonitor(
			boost::shared_ptr<GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT>(
					new GEAOptimizationMonitor()
			)
	);
}

/************************************************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GEvolutionaryAlgorithm object
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm& cp)
	: GOptimizationAlgorithmT<Gem::Geneva::GIndividual>(cp)
	, nParents_(cp.nParents_)
	, microTrainingInterval_(cp.microTrainingInterval_)
	, recombinationMethod_(cp.recombinationMethod_)
	, smode_(cp.smode_)
	, defaultNChildren_(cp.defaultNChildren_)
	, oneTimeMuCommaNu_(cp.oneTimeMuCommaNu_)
	, logOldParents_(cp.logOldParents_)
{
	// Copy the old parents array over if requested
	if(logOldParents_) {
		std::vector<boost::shared_ptr<GIndividual> >::const_iterator cit;
		for(cit=cp.oldParents_.begin(); cit!=cp.oldParents_.end(); ++cit) {
			oldParents_.push_back((*cit)->clone<GIndividual>());
		}
	}

	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/************************************************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GEvolutionaryAlgorithm::~GEvolutionaryAlgorithm()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp Another GEvolutionaryAlgorithm object
 * @return A constant reference to this object
 */
const GEvolutionaryAlgorithm& GEvolutionaryAlgorithm::operator=(const GEvolutionaryAlgorithm& cp) {
	GEvolutionaryAlgorithm::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data of another GEvolutionaryAlgorithm object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GEvolutionaryAlgorithm object, camouflaged as a GObject
 */
void GEvolutionaryAlgorithm::load_(const GObject * cp)
{
	const GEvolutionaryAlgorithm *p_load = conversion_cast<GEvolutionaryAlgorithm>(cp);

	// First load the parent class'es data ...
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::load_(cp);

	// ... and then our own data
	nParents_ = p_load->nParents_;
	microTrainingInterval_ = p_load->microTrainingInterval_;
	recombinationMethod_ = p_load->recombinationMethod_;
	smode_ = p_load->smode_;
	defaultNChildren_ = p_load->defaultNChildren_;
	oneTimeMuCommaNu_ = p_load->oneTimeMuCommaNu_;
	logOldParents_ = p_load->logOldParents_;
	if(logOldParents_) {
		oldParents_.clear();
		std::vector<boost::shared_ptr<GIndividual> >::const_iterator cit;
		for(cit=p_load->oldParents_.begin(); cit!=p_load->oldParents_.end(); ++cit) {
			oldParents_.push_back((*cit)->clone<GIndividual>());
		}
	}
}

/************************************************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object, camouflaged as a pointer to a GObject
 */
GObject *GEvolutionaryAlgorithm::clone_() const  {
	return new GEvolutionaryAlgorithm(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GEvolutionaryAlgorithm object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithm::operator==(const GEvolutionaryAlgorithm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GEvolutionaryAlgorithm::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GEvolutionaryAlgorithm object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEvolutionaryAlgorithm::operator!=(const GEvolutionaryAlgorithm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GEvolutionaryAlgorithm::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
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
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEvolutionaryAlgorithm *p_load = GObject::conversion_cast<GEvolutionaryAlgorithm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::checkRelationshipWith(cp, e, limit, "GEvolutionaryAlgorithm", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", nParents_, p_load->nParents_, "nParents_", "p_load->nParents_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", microTrainingInterval_, p_load->microTrainingInterval_, "microTrainingInterval_", "p_load->microTrainingInterval_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", recombinationMethod_, p_load->recombinationMethod_, "recombinationMethod_", "p_load->recombinationMethod_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", smode_, p_load->smode_, "smode_", "p_load->smode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", defaultNChildren_, p_load->defaultNChildren_, "defaultNChildren_", "p_load->defaultNChildren_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", oneTimeMuCommaNu_, p_load->oneTimeMuCommaNu_, "oneTimeMuCommaNu_", "p_load->oneTimeMuCommaNu_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", logOldParents_, p_load->logOldParents_, "logOldParents_", "p_load->logOldParents_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm", oldParents_, p_load->oldParents_, "oldParents_", "p_load->oldParents_", e , limit));

	return evaluateDiscrepancies("GEvolutionaryAlgorithm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Sets the individual's personality types to EA
 */
void GEvolutionaryAlgorithm::setIndividualPersonalities() {
	GEvolutionaryAlgorithm::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) (*it)->setPersonality(EA);
}

/************************************************************************************************************/
/**
 * Enforces a one-time selection policy of MUCOMMANU. This is used for updates of
 * the parents' structure in the optimize() function. As the quality of updated
 * parents may decrease, it is important to ensure that the next generation's parents
 * are chosen from children with new structure.
 */
void GEvolutionaryAlgorithm::setOneTimeMuCommaNu() {
	oneTimeMuCommaNu_ = true;
}

/************************************************************************************************************/
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

/************************************************************************************************************/
/**
 * Saves the state of the class to disc. The function adds the current generation
 * and the fitness to the base name. We do not save the entire population, but only
 * the best individuals, as these contain the "real" information. Note that no real
 * copying of the individual's data takes place here, as we are dealing with
 * boost::shared_ptr objects.
 */
void GEvolutionaryAlgorithm::saveCheckpoint() const {
	// Copy the nParents best individuals to a vector
	std::vector<boost::shared_ptr<Gem::Geneva::GIndividual> > bestIndividuals;
	GEvolutionaryAlgorithm::const_iterator it;
	for(it=this->begin(); it!=this->begin() + getNParents(); ++it)
		bestIndividuals.push_back(*it);

#ifdef DEBUG // Cross check so we do not accidently trigger value calculation
	if(this->at(0)->isDirty()) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::saveCheckpoint():" << std::endl
			  << "Error: class member has the dirty flag set" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	double newValue = this->at(0)->fitness();

	// Determine a suitable name for the output file
	std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(getIteration()) + "_"
		+ boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

	// Create the output stream and check that it is in good order
	std::ofstream checkpointStream(outputFile.c_str());
	if(!checkpointStream) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::saveCheckpoint()" << std::endl
			  << "Error: Could not open output file" << outputFile.c_str() << std::endl;
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
 * Loads the state of the class from disc. We do not load the entire population,
 * but only the best individuals of a former optimization run, as these contain the
 * "real" information.
 */
void GEvolutionaryAlgorithm::loadCheckpoint(const std::string& cpFile) {
	// Create a vector to hold the best individuals
	std::vector<boost::shared_ptr<Gem::Geneva::GIndividual> > bestIndividuals;

	// Check that the file indeed exists
	if(!boost::filesystem::exists(cpFile)) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::loadCheckpoint(const std::string&)" << std::endl
			  << "Got invalid checkpoint file name " << cpFile << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Create the input stream and check that it is in good order
	std::ifstream checkpointStream(cpFile.c_str());
	if(!checkpointStream) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::loadCheckpoint(const std::string&)" << std::endl
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
 * Instruct the algorithm whether it should log old parents for one iteration
 *
 * @param logOldParents The desired new value of the logOldParents_ flag
 */
void GEvolutionaryAlgorithm::setLogOldParents(const bool& logOldParents) {
	logOldParents_ = logOldParents;
}

/************************************************************************************************************/
/**
 * Retrieves the current value of the logOldParents_ flag
 *
 * @return The current value of the logOldParents_ flag
 */
bool GEvolutionaryAlgorithm::oldParentsLogged() const {
	return logOldParents_;
}

/************************************************************************************************************/
/**
 * Specifies the default size of the population plus the number of parents.
 * The population will be filled with additional individuals later, as required --
 * see GEvolutionaryAlgorithm::adjustPopulation() . Also, all error checking is done in
 * that function.
 *
 * @param popSize The desired size of the population
 * @param nParents The desired number of parents
 */
void GEvolutionaryAlgorithm::setDefaultPopulationSize(const std::size_t& popSize, const std::size_t& nParents) {
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::setDefaultPopulationSize(popSize);
	nParents_ = nParents;
}

/************************************************************************************************************/
/**
 * This function implements the logic that constitutes evolutionary algorithms. The
 * function is called by GOptimizationAlgorithmT<Gem::Geneva::GIndividual> for each cycle of the optimization,
 *
 * @return The value of the best individual found
 */
double GEvolutionaryAlgorithm::cycleLogic() {
	// create new children from parents
	recombine();
	// adapt children and calculate their (and possibly their parent's) values
	adaptChildren();

	// Create a copy of the old parents, if requested
	if(logOldParents_) {
		// Make sure we are dealing with an empty vector
		oldParents_.clear();
		// Attach copies of the current parent individuals
		GEvolutionaryAlgorithm::iterator it;
		for(it=this->begin(); it!= this->begin() + nParents_; ++it) {
			oldParents_.push_back((*it)->clone<GIndividual>());
		}
	}

	// find out the best individuals of the population
	select();

	boost::uint32_t stallCounter = getStallCounter();
	if(microTrainingInterval_ && stallCounter && stallCounter%microTrainingInterval_ == 0) {
#ifdef DEBUG
		std::cout << "Updating parents ..." << std::endl;
#endif /* DEBUG */

		if(updateParentStructure()) {
			setOneTimeMuCommaNu();
		}
	}

	// Retrieve the fitness of the best individual in the collection
	bool isDirty = false;
	double bestFitness = this->at(0)->getCurrentFitness(isDirty);

#ifdef DEBUG
	if(isDirty) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::cycleLogic(): Found dirty individual when it should not be" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */

	return bestFitness;
}


/************************************************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::optimize(), before the
 * actual optimization cycle starts.
 */
void GEvolutionaryAlgorithm::init() {
	// To be performed before any other action
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::init();

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
		error << "In GEvolutionaryAlgorithm::init() : Error!" << std::endl
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
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Let parents know they are parents
	markParents();
	// Let children know they are children

	// Make sure derived classes (such as GTransferPopulation) have a way of finding out
	// what the desired number of children is. This is particularly important, if, in a
	// network environment, some individuals might not return and some individuals return
	// late. The factual size of the population then changes and we need to take action.
	defaultNChildren_ = getDefaultPopulationSize() - nParents_;
}

/************************************************************************************************************/
/**
 * Does any necessary finalization work
 */
void GEvolutionaryAlgorithm::finalize() {
	// Last action
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::finalize();
}

/************************************************************************************************************/
/**
 * The function checks that the population size meets the requirements and resizes the
 * population to the appropriate size, if required. An obvious precondition is that at
 * least one individual has been added to the population. Individuals that have already
 * been added will not be replaced. This function is called once before the optimization
 * cycle from within GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::optimize()
 */
void GEvolutionaryAlgorithm::adjustPopulation() {
	// Has the population size been set at all ?
	if(getDefaultPopulationSize() == 0) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::adjustPopulation() : Error!" << std::endl
			  << "The population size is 0." << std::endl
			  << "Did you call GOptimizationAlgorithmT<Gem::Geneva::GIndividual>:setDefaultPopulationSize() ?" << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Check how many individuals have been added already. At least one is required.
	std::size_t this_sz = data.size();
	if(this_sz == 0) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::adjustPopulation() : Error!" << std::endl
			  << "size of population is 0. Did you add any individuals?" << std::endl
			  << "We need at least one local individual" << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Do the smart pointers actually point to any objects ?
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		if(!(*it)) { // shared_ptr can be implicitly converted to bool
			std::ostringstream error;
			error << "In GEvolutionaryAlgorithm::adjustPopulation() : Error!" << std::endl
				  << "Found empty smart pointer." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
	}

	// Fill up as required. We are now sure we have a suitable number of individuals to do so
	if(this_sz < getDefaultPopulationSize()) {
		this->resize_clone(getDefaultPopulationSize(), data[0]);

		// Randomly initialize new items.
		// (Note: This will currently only have an effect on GParameterSet-derivatives)
		for(it=data.begin()+this_sz; it!=data.end(); ++it) {
			(*it)->randomInit();
		}
	}
}

/************************************************************************************************************/
/**
 * Set the interval in which micro training should be performed. Set the
 * interval to 0 in order to prevent micro training.
 *
 * @param mti The desired new value of the mircoTrainingInterval_ variable
 */
void GEvolutionaryAlgorithm::setMicroTrainingInterval(const boost::uint32_t& mti) {
	microTrainingInterval_ = mti;
}

/************************************************************************************************************/
/**
 * Retrieve the interval in which micro training should be performed
 *
 * @return The current value of the mircoTrainingInterval_ variable
 */
boost::uint32_t GEvolutionaryAlgorithm::getMicroTrainingInterval() const {
	return microTrainingInterval_;
}

/************************************************************************************************************/
/**
 * Retrieve the number of parents as set by the user. This is a fixed parameter and
 * should not be changed after it has first been set.
 *
 * @return The number of parents in the population
 */
std::size_t GEvolutionaryAlgorithm::getNParents() const {
	return nParents_;
}

/************************************************************************************************************/
/**
 * Calculates the current number of children from the number of parents and the
 * size of the vector.
 *
 * @return The number of children in the population
 */
std::size_t GEvolutionaryAlgorithm::getNChildren() const {
	return data.size() - nParents_;
}

/************************************************************************************************************/
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

/************************************************************************************************************/
/**
 * Retrieves information about the current sorting scheme (see
 * GEvolutionaryAlgorithm::setSortingScheme() for further information).
 *
 * @return The current sorting scheme
 */
sortingMode GEvolutionaryAlgorithm::getSortingScheme() const {
	return smode_;
}

/************************************************************************************************************/
/**
 * This function is called from GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::optimize() and performs the
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
		throw Gem::Common::gemfony_error_condition(error.str());
	}
#endif

	// Do the actual recombination
	doRecombine();

	// Let children know they are children
	markChildren();

	// Tell individuals about their ids
	markIndividualPositions();
}

/************************************************************************************************************/
/**
 * This function assigns a new value to each child individual according to the chosen
 * recombination scheme.
 */
void GEvolutionaryAlgorithm::doRecombine() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	switch(recombinationMethod_){
	case DEFAULTRECOMBINE: // we want the RANDOMRECOMBINE behavior
	case RANDOMRECOMBINE:
	{
		std::size_t child_id=0;
		for(it=data.begin()+nParents_; it!= data.end(); ++it) {
			randomRecombine(*it);
		}
	}
		break;

	case VALUERECOMBINE:
		// Recombination according to the parents' fitness only makes sense if
		// we have at least 2 parents. We do the recombination manually otherwise
		if(nParents_==1) {
			std::size_t p_pos = 0;
			for(it=data.begin()+1; it!= data.end(); ++it) {
				(*it)->GObject::load(*(data.begin()));
				(*it)->getEAPersonalityTraits()->setParentId(p_pos++);
			}
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

/************************************************************************************************************/
/**
 * This function implements the RANDOMRECOMBINE scheme. This functions uses BOOST's
 * numeric_cast function for safe conversion between std::size_t and uint16_t.
 *
 * @param pos The position of the individual for which a new value should be chosen
 */
void GEvolutionaryAlgorithm::randomRecombine(boost::shared_ptr<GIndividual>& child) {
	std::size_t parent_pos;

	if(nParents_==1) {
		parent_pos = 0;

	} else {
		// Choose a parent to be used for the recombination. Note that
		// numeric_cast may throw. Exceptions need to be caught in surrounding functions.
		// try/catch blocks would add a non-negligible overhead in this function. uniform_int(max)
		// returns integer values in the range [0,max]. As we want to have values in the range
		// 0,1, ... nParents_-1, we need to subtract one from the argument.
		parent_pos = boost::numeric_cast<std::size_t>(gr.uniform_int(nParents_-1));
	}

	// Load the parent data into the individual
	child->GObject::load(*(data.begin() + parent_pos));

	// Let the individual know the id of the parent
	child->getEAPersonalityTraits()->setParentId(parent_pos);
}

/************************************************************************************************************/
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
	double randTest = gr.uniform_01(); // get the test value

	for(std::size_t par=0; par<nParents_; par++) {
		if(randTest<threshold[par]) {
			// Load the parent's data
			p->GObject::load(*(data.begin() + par));
			// Let the individual know the parent's id
			p->getEAPersonalityTraits()->setParentId(par);
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
		throw Gem::Common::gemfony_error_condition(error.str());
	}
}

/************************************************************************************************************/
/**
 * Adapt all children in sequence. Note that this also triggers their value
 * calculation, so this function needs to be overloaded for optimization in a
 * network context.
 */
void GEvolutionaryAlgorithm::adaptChildren()
{
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We start with the parents, if this is iteration 0. Their
	// initial fitness needs to be determined, if this is the MUPLUSNU
	// or MUNU1PRETAIN selection model.
	// Make sure we also evaluate the parents in the first iteration, if needed.
	// This is only applicable to the MUPLUSNU and MUNU1PRETAIN modes.
	if(getIteration()==0) {
		switch(getSortingScheme()) {
		//--------------------------------------------------------------
		case MUPLUSNU:
		case MUNU1PRETAIN: // same procedure. We do not know which parent is best
			for(it=data.begin(); it!=data.begin() + nParents_; ++it) {
				(*it)->fitness();
			}
			break;

		case MUCOMMANU:
			break; // nothing
		}
	}

	// Next we perform the adaption of each child individual in
	// sequence. Note that this can also trigger fitness calculation.
	for(it=data.begin()+nParents_; it!=data.end(); ++it) (*it)->adapt();
}

/************************************************************************************************************/
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
		throw Gem::Common::gemfony_error_condition(error.str());
	}
#endif /* DEBUG */

	switch(smode_) {
	//----------------------------------------------------------------------------
	case MUPLUSNU:
		if(oneTimeMuCommaNu_) {
			sortMucommanuMode();
			oneTimeMuCommaNu_=false;
		}
		else sortMuplusnuMode();
		break;

	//----------------------------------------------------------------------------
	case MUNU1PRETAIN:
		if(oneTimeMuCommaNu_) {
			sortMucommanuMode();
			oneTimeMuCommaNu_=false;
		}
		else sortMunu1pretainMode();
		break;

	//----------------------------------------------------------------------------
	case MUCOMMANU:
		sortMucommanuMode();
		break;
	//----------------------------------------------------------------------------
	}

	// Let parents know they are parents
	markParents();
}

/************************************************************************************************************/
/**
 * Selection, MUPLUSNU style. Note that not all individuals of the population (including parents)
 * are sorted -- only the nParents best individuals are identified. The quality of the population can only
 * increase, but the optimization will stall more easily in MUPLUSNU mode.
 */
void GEvolutionaryAlgorithm::sortMuplusnuMode() {
	// Only partially sort the arrays
	if(getMaximize()){
		std::partial_sort(data.begin(), data.begin() + nParents_, data.end(),
				boost::bind(&GIndividual::fitness, _1) > boost::bind(&GIndividual::fitness, _2));
	}
	else{ // Minimization
		std::partial_sort(data.begin(), data.begin() + nParents_, data.end(),
				boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
	}
}

/************************************************************************************************************/
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
		/*
		std::sort(data.begin()+nParents_, data.end(),
				boost::bind(&GIndividual::fitness, _1) > boost::bind(&GIndividual::fitness, _2));
		*/
	}
	else{
		std::partial_sort(data.begin() + nParents_, data.begin() + 2*nParents_, data.end(),
			  boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
		/*
		std::sort(data.begin()+nParents_, data.end(),
				boost::bind(&GIndividual::fitness, _1) < boost::bind(&GIndividual::fitness, _2));
		*/
	}
	std::swap_ranges(data.begin(),data.begin()+nParents_,data.begin()+nParents_);
}

/************************************************************************************************************/
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
		sortMuplusnuMode();
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
		if(!isBetter(bestChildFitness, bestParentFitness)) {
			std::swap_ranges(data.begin()+1,data.begin()+nParents_,data.begin()+nParents_);
		} else { // A better child was found. Overwrite all parents
			std::swap_ranges(data.begin(),data.begin()+nParents_,data.begin()+nParents_);
		}
	}
}

/************************************************************************************************************/
/**
 * This helper function marks parents as parents and children as children.
 */
void GEvolutionaryAlgorithm::markParents() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.begin()+nParents_; ++it){
		(*it)->getEAPersonalityTraits()->setIsParent();
	}
}

/************************************************************************************************************/
/**
 * This helper function marks children as children
 */
void GEvolutionaryAlgorithm::markChildren() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin()+nParents_; it!=data.end(); ++it){
		(*it)->getEAPersonalityTraits()->setIsChild();
	}
}

/************************************************************************************************************/
/**
 * This helper function lets all individuals know about their position in the
 * population.
 */
void GEvolutionaryAlgorithm::markIndividualPositions() {
	std::size_t pos = 0;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->getEAPersonalityTraits()->setPopulationPosition(pos++);
}

/************************************************************************************************************/
/**
 * Retrieves the defaultNChildren_ parameter. E.g. in GTransferPopulation::adaptChildren() ,
 * this factor controls when a population is considered to be complete. The corresponding
 * loop which waits for new arrivals will then be stopped, which in turn allows
 * a new generation to start.
 *
 * @return The defaultNChildren_ parameter
 */
std::size_t GEvolutionaryAlgorithm::getDefaultNChildren() const {
	return defaultNChildren_;
}

/************************************************************************************************************/
/**
 * Lets the user set the desired recombination method. No sanity checks for the
 * values are necessary, as we use an enum.
 *
 * @param recombinationMethod The desired recombination method
 */
void GEvolutionaryAlgorithm::setRecombinationMethod(const recoScheme& recombinationMethod) {
	recombinationMethod_ = recombinationMethod;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the recombinationMethod_ variable
 *
 * @return The value of the recombinationMethod_ variable
 */
recoScheme GEvolutionaryAlgorithm::getRecombinationMethod() const {
	return recombinationMethod_;
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEvolutionaryAlgorithm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

/**********************************************************************************/
/**
 * The default constructor
 */
GEvolutionaryAlgorithm::GEAOptimizationMonitor::GEAOptimizationMonitor()
	: xDim_(DEFAULTXDIMOM)
	, yDim_(DEFAULTYDIMOM)
	, nMonitorInds_(0)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GEAOptimizationMonitor object
 */
GEvolutionaryAlgorithm::GEAOptimizationMonitor::GEAOptimizationMonitor(const GEvolutionaryAlgorithm::GEAOptimizationMonitor& cp)
	: GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT(cp)
	, xDim_(cp.xDim_)
	, yDim_(cp.yDim_)
	, nMonitorInds_(cp.nMonitorInds_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The destructor
 */
GEvolutionaryAlgorithm::GEAOptimizationMonitor::~GEAOptimizationMonitor()
{ /* nothing */ }

/**********************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GEAOptimizationMonitor object
 * @return A constant reference to this object
 */
const GEvolutionaryAlgorithm::GEAOptimizationMonitor& GEvolutionaryAlgorithm::GEAOptimizationMonitor::operator=(const GEvolutionaryAlgorithm::GEAOptimizationMonitor& cp){
	GEvolutionaryAlgorithm::GEAOptimizationMonitor::load_(&cp);
	return *this;
}

/**********************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GEAOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithm::GEAOptimizationMonitor::operator==(const GEvolutionaryAlgorithm::GEAOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GEvolutionaryAlgorithm::GEAOptimizationMonitor::operator==","cp", CE_SILENT);
}

/**********************************************************************************/
/**
 * Checks for inequality with another GEAOptimizationMonitor object
 *
 * @param  cp A constant reference to another GEAOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEvolutionaryAlgorithm::GEAOptimizationMonitor::operator!=(const GEvolutionaryAlgorithm::GEAOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GEvolutionaryAlgorithm::GEAOptimizationMonitor::operator!=","cp", CE_SILENT);
}

/**********************************************************************************/
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
boost::optional<std::string> GEvolutionaryAlgorithm::GEAOptimizationMonitor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEvolutionaryAlgorithm::GEAOptimizationMonitor *p_load = GObject::conversion_cast<GEvolutionaryAlgorithm::GEAOptimizationMonitor >(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::checkRelationshipWith(cp, e, limit, "GEvolutionaryAlgorithm::GEAOptimizationMonitor", y_name, withMessages));

	// ... and then our local data.
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm::GEAOptimizationMonitor", xDim_, p_load->xDim_, "xDim_", "p_load->xDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm::GEAOptimizationMonitor", yDim_, p_load->yDim_, "yDim_", "p_load->yDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEvolutionaryAlgorithm::GEAOptimizationMonitor", nMonitorInds_, p_load->nMonitorInds_, "nMonitorInds_", "p_load->nMonitorInds_", e , limit));

	return evaluateDiscrepancies("GEvolutionaryAlgorithm::GEAOptimizationMonitor", caller, deviations, e);
}

/**********************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 * @return A string containing information to written to the output file (if any)
 */
std::string GEvolutionaryAlgorithm::GEAOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GIndividual> * const goa) {
	// This should always be the first statement in a custom optimization monitor
	std::cout << GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::firstInformation(goa);

	// Perform the conversion to the target algorithm
#ifdef DEBUG
	if(goa->getOptimizationAlgorithm() != EA) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::GEAOptimizationMonitor::firstInformation(): Error!" << std::endl
			  << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	GEvolutionaryAlgorithm * const ea = static_cast<GEvolutionaryAlgorithm * const>(goa);

	// Determine a suitable number of monitored individuals, if it hasn't already
	// been set externally. We allow a maximum of 3 monitored individuals by default
	// (or the number of parents, if <= 3).
	if(nMonitorInds_ == 0) {
		nMonitorInds_ = std::min(ea->getNParents(), std::size_t(3));
	}

	// Output the header to the summary stream
	return eaFirstInformation(ea);
}

/**********************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 * @return A string containing information to written to the output file (if any)
 */
std::string GEvolutionaryAlgorithm::GEAOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GIndividual> * const goa) {
	// Let the audience know what the parent has to say
	std::cout << GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::cycleInformation(goa);

	// Perform the conversion to the target algorithm
#ifdef DEBUG
	if(goa->getOptimizationAlgorithm() != EA) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::GEAOptimizationMonitor::cycleInformation(): Error!" << std::endl
			  << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	GEvolutionaryAlgorithm * const ea = static_cast<GEvolutionaryAlgorithm * const>(goa);

	return eaCycleInformation(ea);
}

/**********************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 * @return A string containing information to written to the output file (if any)
 */
std::string GEvolutionaryAlgorithm::GEAOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GIndividual> * const goa) {
	// Perform the conversion to the target algorithm
#ifdef DEBUG
	if(goa->getOptimizationAlgorithm() != EA) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::GEAOptimizationMonitor::lastInformation(): Error!" << std::endl
			  << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	GEvolutionaryAlgorithm * const ea = static_cast<GEvolutionaryAlgorithm * const>(goa);

	// Do the actual information gathering
	std::ostringstream result;
	result << eaLastInformation(ea);

	// This should always be the last statement in a custom optimization monitor
	std::cout << GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::lastInformation(goa);

	return result.str();
}

/**********************************************************************************/
/**
 * A function that is called once before the optimization starts, acting on evolutionary
 * algorithms
 */
std::string GEvolutionaryAlgorithm::GEAOptimizationMonitor::eaFirstInformation(GEvolutionaryAlgorithm * const ea) {
	std::ostringstream result;

	// Output the header to the summary stream
	result << "{" << std::endl
		   << "  gROOT->Reset();" << std::endl
		   << "  gStyle->SetOptTitle(0);" << std::endl
		   << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0," << xDim_ << "," << yDim_ << ");" << std::endl
		   << "  cc->Divide(1," << nMonitorInds_ << ");" << std::endl
		   << std::endl;

	result << "  std::vector<long> iteration;" << std::endl
		   << std::endl;
	for(std::size_t i=0; i<nMonitorInds_; i++) {
		result << "  std::vector<double> evaluation" << i << ";" << std::endl
			   << std::endl;
	}

	return result.str();
}

/**********************************************************************************/
/**
 * A function that is called during each optimization cycle, acting on evolutionary
 * algorithms
 *
 */
std::string GEvolutionaryAlgorithm::GEAOptimizationMonitor::eaCycleInformation(GEvolutionaryAlgorithm * const ea) {
	std::ostringstream result;

	bool isDirty = false;
	double currentEvaluation = 0.;

	// Retrieve the current iteration
	boost::uint32_t iteration = ea->getIteration();

	result << "  iteration.push_back(" << iteration << ");" << std::endl;

	for(std::size_t i=0; i<nMonitorInds_; i++) {
		// Get access to the individual
		boost::shared_ptr<GIndividual> gi_ptr = ea->individual_cast<GIndividual>(i);

		// Retrieve the fitness of this individual
		isDirty = false;
		currentEvaluation = gi_ptr->getCurrentFitness(isDirty);

		// Write information to the output stream
		result << "  evaluation" << i << ".push_back(" <<  currentEvaluation << ");" << (isDirty?" // dirty flag is set":"") << std::endl;
	}
	result << std::endl; // Improves readability of the output data

	return result.str();
}

/**********************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle acting
 * on evolutionary algorithms
 *
 */
std::string GEvolutionaryAlgorithm::GEAOptimizationMonitor::eaLastInformation(GEvolutionaryAlgorithm * const ea) {
	std::ostringstream result;

	// Output final print logic to the stream
	result << "  // Transfer the vectors into arrays" << std::endl
			<< "  double iteration_arr[iteration.size()];" << std::endl;

	for(std::size_t i=0; i<nMonitorInds_; i++) {
		result << "  double evaluation" << i << "_arr[evaluation" << i << ".size()];" << std::endl
			   << std::endl
			   << "  for(std::size_t i=0; i<iteration.size(); i++) {" << std::endl
			   << "     iteration_arr[i] = (double)iteration[i];"
			   << "     evaluation" << i << "_arr[i] = evaluation" << i << "[i];" << std::endl
			   << "  }" << std::endl
			   << std::endl
			   << "  // Create a TGraph object" << std::endl
			   << "  TGraph *evGraph" << i << " = new TGraph(evaluation" << i << ".size(), iteration_arr, evaluation" << i << "_arr);" << std::endl
			   << "  // Set the axis titles" << std::endl
			   << "  evGraph" << i << "->GetXaxis()->SetTitle(\"Iteration\");" << std::endl
			   << "  evGraph" << i << "->GetYaxis()->SetTitleOffset(1.1);" << std::endl
			   << "  evGraph" << i << "->GetYaxis()->SetTitle(\"Fitness\");" << std::endl
			   << std::endl;
	}

	result << "  // Do the actual drawing" << std::endl;

	for(std::size_t i=0; i<nMonitorInds_; i++) {
		result << "  cc->cd(" << i+1 << ");" << std::endl
			   << "  evGraph" << i << "->Draw(\"AP\");" << std::endl;
	}

	result << "  cc->cd();" << std::endl
		   << "}" << std::endl;

	return result.str();
}

/**********************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param xDim The desired dimension of the canvas in x-direction
 * @param yDim The desired dimension of the canvas in y-direction
 */
void GEvolutionaryAlgorithm::GEAOptimizationMonitor::setDims(const boost::uint16_t& xDim, const boost::uint16_t& yDim) {
	xDim_ = xDim;
	yDim_ = yDim;
}

/**********************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
boost::uint16_t GEvolutionaryAlgorithm::GEAOptimizationMonitor::getXDim() const {
	return xDim_;
}

/**********************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
boost::uint16_t GEvolutionaryAlgorithm::GEAOptimizationMonitor::getYDim() const {
	return yDim_;
}

/**********************************************************************************/
/**
 * Sets the number of individuals in the population that should be monitored
 *
 * @oaram nMonitorInds The number of individuals in the population that should be monitored
 */
void GEvolutionaryAlgorithm::GEAOptimizationMonitor::setNMonitorIndividuals(const std::size_t& nMonitorInds) {
	if(nMonitorInds == 0) {
		std::ostringstream error;
		error << "In GEvolutionaryAlgorithm::GEAOptimizationMonitor::setNMonitorIndividuals(): Error!" << std::endl
		      << "Number of monitored individuals is set to 0." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	nMonitorInds_ = nMonitorInds;
}

/**********************************************************************************/
/**
 * Retrieves the number of individuals that are being monitored
 *
 * @return The number of individuals in the population being monitored
 */
std::size_t GEvolutionaryAlgorithm::GEAOptimizationMonitor::getNMonitorIndividuals() const {
	return nMonitorInds_;
}

/**********************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GEvolutionaryAlgorithm::GEAOptimizationMonitor object, camouflaged as a GObject
 */
void GEvolutionaryAlgorithm::GEAOptimizationMonitor::load_(const GObject* cp) {
	const GEvolutionaryAlgorithm::GEAOptimizationMonitor *p_load = conversion_cast<GEvolutionaryAlgorithm::GEAOptimizationMonitor>(cp);

	// Load the parent classes' data ...
	GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::load_(cp);

	// ... and then our local data
	xDim_ = p_load->xDim_;
	yDim_ = p_load->yDim_;
	nMonitorInds_ = p_load->nMonitorInds_;
}

/**********************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GEvolutionaryAlgorithm::GEAOptimizationMonitor::clone_() const {
	return new GEvolutionaryAlgorithm::GEAOptimizationMonitor(*this);
}

#ifdef GENEVATESTING
/**********************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GEvolutionaryAlgorithm::GEAOptimizationMonitor::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

	return result;
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm::GEAOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();
}

/**********************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm::GEAOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();
}

/**********************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
