/**
 * @file GBrokerEA.cpp
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

#include "geneva/GBrokerEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerEA)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBrokerEA::GBrokerEA()
	: GBaseEA()
	, Gem::Courtier::GBrokerConnector2T<GParameterSet>(Gem::Courtier::INCOMPLETERETURN)
   // , Gem::Courtier::GBrokerConnector2T<GParameterSet>(Gem::Courtier::EXPECTFULLRETURN)
	, nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
	, storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerEA object
 */
GBrokerEA::GBrokerEA(const GBrokerEA& cp)
	: GBaseEA(cp)
	, Gem::Courtier::GBrokerConnector2T<GParameterSet>(cp)
	, nThreads_(cp.nThreads_)
	, storedServerMode_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerEA::~GBrokerEA()
{ /* nothing */}

/******************************************************************************/
/**
 * A standard assignment operator for GBrokerEA objects,
 *
 * @param cp A copy of another GBrokerEA object
 * @return A constant reference to this object
 */
const GBrokerEA& GBrokerEA::operator=(const GBrokerEA& cp) {
	GBrokerEA::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Loads the data of another GBrokerEA object, camouflaged as a
 * pointer to a GObject
 *
 * @param cp A pointer to another GBrokerEA object, camouflaged as a GObject
 */
void GBrokerEA::load_(const GObject * cp) {
	const GBrokerEA *p_load = gobject_conversion<GBrokerEA>(cp);

	// Load the parent classes' data ...
	GBaseEA::load_(cp);
	Gem::Courtier::GBrokerConnector2T<GParameterSet>::load(p_load);

	// ... and then our own
	nThreads_ = p_load->nThreads_;

	// Note that we do not copy storedServerMode_ as it is used for internal caching only
}

/******************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GBrokerEA::clone_() const {
	return new GBrokerEA(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GBrokerEA object
 *
 * @param  cp A constant reference to another GBrokerEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerEA::operator==(const GBrokerEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerEA::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBrokerEA object
 *
 * @param  cp A constant reference to another GBrokerEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerEA::operator!=(const GBrokerEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerEA::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBrokerEA::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;
    using namespace Gem::Courtier;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerEA *p_load = GObject::gobject_conversion<GBrokerEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GBaseEA::checkRelationshipWith(cp, e, limit, "GBrokerEA", y_name, withMessages));
	deviations.push_back(GBrokerConnector2T<GParameterSet>::checkRelationshipWith(*p_load, e, limit, "GBrokerEA", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GBrokerEA", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

	return evaluateDiscrepancies("GBrokerEA", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBrokerEA::name() const {
   return std::string("GBrokerEA");
}

/******************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerEA::init() {
	// Prevent usage of this brokered algorithms inside of this broker population - check type of individuals
	// Note that evolutionary algorithms may store arbitrary "GParameterSet"-derivatives, hence it is also possible
	// to store brokered optimization algorithms in it, which does not make sense.
	{
		std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			if((*it)->getIndividualCharacteristic() == "GENEVA_BROKEROPTALG"
			   || (*it)->getIndividualCharacteristic() == "GENEVA_GO2WRAPPER") {
			   glogger
			   << "In GBrokerEA::optimize(): Error" << std::endl
            << "GBrokerEA or Go2 stored as an individual inside of" << std::endl
            << "the population." << std::endl
            << GEXCEPTION;
			}
		}
	}

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::init();

	// Initialize the broker connector
	Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>::init();

	// Initialize our thread pool
	tp_.reset(new Gem::Common::GThreadPool(nThreads_));

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	// The function will throw if not all individuals have the same server mode flag.

	// Set the server mode and store the original flag
	bool first = true;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		if(first){
			storedServerMode_ = (*it)->getServerMode();
			first = false;
		}

		if(storedServerMode_ != (*it)->setServerMode(true)) {
		   glogger
		   << "In GBrokerEA::init():" << std::endl
         << "Not all server mode flags have the same value!" << std::endl
         << GEXCEPTION;
		}
	}
}

/******************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerEA::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(storedServerMode_);
	}

	// Terminate our thread pool
	tp_.reset();

   // Finalize the broker connector
   Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>::finalize();

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::finalize();
}

/******************************************************************************/
/**
 * Checks whether this algorithm communicates via the broker. This is an overload from the corresponding
 * GOptimizableI function
 *
 * @return A boolean indicating whether this algorithm communicates via the broker
 */
bool GBrokerEA::usesBroker() const {
	return true;
}

/******************************************************************************/
/**
 * Adapt all children in parallel. Evaluation is done in a seperate function (evaluateChildren).
 */
void GBrokerEA::adaptChildren()
{
	boost::tuple<std::size_t,std::size_t> range = getAdaptionRange();
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;

	for(it=data.begin()+boost::get<0>(range); it!=data.begin()+boost::get<1>(range); ++it) {
		tp_->schedule(boost::function<void()>(boost::bind(&GParameterSet::adapt, *it)));
	}

	// Wait for all threads in the pool to complete their work
	tp_->wait();
}

/******************************************************************************/
/**
 * We submit individuals to the broker and wait for processed items.
 */
void GBrokerEA::evaluateChildren() {
	//--------------------------------------------------------------------------------
	// Start by marking the work to be done in the individuals.
	// "range" will hold the start- and end-points of the range
	// to be worked on
	boost::tuple<std::size_t, std::size_t> range = getEvaluationRange();

#ifdef DEBUG
   // There should be no situation in which a "clean" child is submitted
   // through this function
   for(std::size_t i=this->getNParents(); i<this->size(); i++) {
      if(!this->at(i)->isDirty()) {
         glogger
         << "In GBrokerEA::evaluateChildren(): Error!" << std::endl
         << "Tried to evaluate children in range " << boost::get<0>(range) << " - " << boost::get<1>(range) << std::endl
         << "but found \"clean\" individual in position " << i << std::endl
         << GEXCEPTION;
      }
   }
#endif

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results.
	Gem::Courtier::GBrokerConnector2T<GParameterSet>::workOn(
      data
      , range
      , oldWorkItems_
      , true // Remove unprocessed items
	);

	//--------------------------------------------------------------------------------
	// Now fix the population -- it may be smaller than its nominal size
	fixAfterJobSubmission();
}

/******************************************************************************/
/**
 * Fixes the population after a job submission
 */
void GBrokerEA::fixAfterJobSubmission() {
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	std::size_t np = getNParents();
	boost::uint32_t iteration=getIteration();

	// Remove parents from older iterations from old work items -- we do not want them.
   // Note that "remove_if" simply moves items not satisfying the predicate to the end of the list.
   // We thus need to explicitly erase these items. remove_if returns the iterator position right after
   // the last item not satisfying the predicate.
   oldWorkItems_.erase(
      std::remove_if(oldWorkItems_.begin(), oldWorkItems_.end(), isOldParent(iteration))
	   , oldWorkItems_.end()
	);

	// Make it known to remaining old individuals that they are now part of a new iteration
	std::for_each(oldWorkItems_.begin(), oldWorkItems_.end(), boost::bind(&GParameterSet::setAssignedIteration, _1, iteration));

	// Make sure that parents are at the beginning of the array.
	sort(data.begin(), data.end(), indParentComp());

	// Attach all old work items to the end of the current population and clear the array of old items
	for(it=oldWorkItems_.begin(); it!=oldWorkItems_.end(); ++it) {
	   data.push_back(*it);
	}
	oldWorkItems_.clear();

	// Add missing individuals, as clones of the last item
	if(data.size() < getDefaultPopulationSize()){
		std::size_t fixSize= getDefaultPopulationSize() - data.size();
		for(std::size_t i=0; i<fixSize; i++){
			// This function will create a clone of its argument
			this->push_back_clone(data.back());
		}
	}

	// Mark the first nParents_ individuals as parents in the first iteration. We want to have a "sane" population.
	if(inFirstIteration()){
		for(it=this->begin(); it!=this->begin() + np; ++it) {
			(*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsParent();
		}
	}

	// We care for too many returned individuals in the selectBest() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.
}

/******************************************************************************/
/**
 * We will at this point have a population with at least the default number
 * of individuals. More individuals are allowed. the population will be
 * resized to nominal values at the end of this function.
 */
void GBrokerEA::selectBest() {
	////////////////////////////////////////////////////////////
	// Great - we are at least at the default level and are
	// ready to call the actual selectBest() function. This will
	// automatically take care of the selection modes.
	GBaseEA::selectBest();

#ifdef DEBUG
	// Make sure our population is not smaller than its nominal size -- this
	// should have been taken care of in fixAfterJobSubmission() .
	if(data.size() < getDefaultPopulationSize()) {
	   glogger
	   << "In GBrokerEA::selectBest(): Error!" << std::endl
      << "Size of population is smaller than expected: " << data.size() << " / " << getDefaultPopulationSize() << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next iteration finds a "standard" population. This
	// function will remove the last items.
	data.resize(getNParents() + getDefaultNChildren());

	// Everything should be back to normal ...
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GBrokerEA::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GBaseEA::addConfigurationOptions(gpb, showOrigin);
	Gem::Courtier::GBrokerConnector2T<GParameterSet>::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	comment = ""; // Reset the comment string
	comment += "The number of threads used to simultaneously adapt individuals;";
	if(showOrigin) comment += "[GBrokerEA]";
	gpb.registerFileParameter<boost::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, boost::bind(
			&GBrokerEA::setNThreads
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);
}

/******************************************************************************/
/**
 * Sets the number of threads this population uses for adaption. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the number of hardware
 * threading units (e.g. number of cores or hyper-threading units).
 *
 * @param nThreads The number of threads this class uses
 */
void GBrokerEA::setNThreads(boost::uint16_t nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
	}
	else {
		nThreads_ = nThreads;
	}
}

/******************************************************************************/
/**
 * Retrieves the number of threads this population uses for adaption
 *
 * @return The maximum number of allowed threads
 */
boost::uint16_t GBrokerEA::getNThreads() const  {
	return nThreads_;
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GBrokerEA::getIndividualCharacteristic() const {
	return std::string("GENEVA_BROKEROPTALG");
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerEA::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GBaseEA::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerEA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerEA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseEA::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerEA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerEA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseEA::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
