/**
 * @file GBrokerSA.cpp
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

#include "geneva/GBrokerSA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerSA)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. nThreads_ is initialized in the class body.
 */
GBrokerSA::GBrokerSA()
	: GBaseSA()
	, Gem::Courtier::GBrokerConnectorT<GParameterSet>(Gem::Courtier::submissionReturnMode::INCOMPLETERETURN)
	, nThreads_(boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerSA object
 */
GBrokerSA::GBrokerSA(const GBrokerSA &cp)
	: GBaseSA(cp)
	, Gem::Courtier::GBrokerConnectorT<GParameterSet>(cp)
	, nThreads_(cp.nThreads_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerSA::~GBrokerSA()
{ /* nothing */}

/******************************************************************************/
/**
 * Loads the data of another GBrokerSA object, camouflaged as a
 * pointer to a GObject
 *
 * @param cp A pointer to another GBrokerSA object, camouflaged as a GObject
 */
void GBrokerSA::load_(const GObject *cp) {
	// Check that we are dealing with a GBrokerSA reference independent of this object and convert the pointer
	const GBrokerSA *p_load = Gem::Common::g_convert_and_compare<GObject, GBrokerSA >(cp, this);

	// Load the parent classes' data ...
	GBaseSA::load_(cp);
	Gem::Courtier::GBrokerConnectorT<GParameterSet>::load(p_load);

	// ... and then our own
	nThreads_ = p_load->nThreads_;
}

/******************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GBrokerSA::clone_() const {
	return new GBrokerSA(*this);
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBrokerSA &GBrokerSA::operator=(
	const GBrokerSA &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBrokerSA object
 *
 * @param  cp A constant reference to another GBrokerSA object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerSA::operator==(const GBrokerSA &cp) const {
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
 * Checks for inequality with another GBrokerSA object
 *
 * @param  cp A constant reference to another GBrokerSA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerSA::operator!=(const GBrokerSA &cp) const {
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
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBrokerSA::compare(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBrokerSA reference independent of this object and convert the pointer
	const GBrokerSA *p_load = Gem::Common::g_convert_and_compare<GObject, GBrokerSA >(cp, this);

	GToken token("GBrokerSA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseSA>(IDENTITY(*this, *p_load), token);

	// We do not compare the broker data

	// ... and then the local data
	compare_t(IDENTITY(nThreads_, p_load->nThreads_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBrokerSA::name() const {
	return std::string("GBrokerSA");
}

/******************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerSA::init() {
	// Prevent usage of this brokered algorithms inside of this broker population - check type of individuals
	// Note that evolutionary algorithms may store arbitrary "GParameterSet"-derivatives, hence it is also possible
	// to store brokered optimization algorithms in it, which does not make sense.
	{
		std::vector<std::shared_ptr < GParameterSet>> ::iterator
		it;
		for (it = this->begin(); it != this->end(); ++it) {
			if ((*it)->getIndividualCharacteristic() == "GENEVA_BROKEROPTALG"
				 || (*it)->getIndividualCharacteristic() == "GENEVA_GO2WRAPPER") {
				glogger
				<< "In GBrokerSA::optimize(): Error" << std::endl
				<< "GBrokerSA or Go2 stored as an individual inside of" << std::endl
				<< "the population." << std::endl
				<< GEXCEPTION;
			}
		}
	}

	// GBaseSA sees exactly the environment it would when called from its own class
	GBaseSA::init();

	// Initialize the broker connector
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>::init();

	// Initialize our thread pool
	tp_ptr_.reset(new Gem::Common::GThreadPool(nThreads_));
}

/******************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerSA::finalize() {
	// Check whether there were any errors during thread execution
	if (tp_ptr_->hasErrors()) {
		std::vector<std::string> errors;
		errors = tp_ptr_->getErrors();

		glogger
		<< "========================================================================" << std::endl
		<< "In GBrokerSA::finalize():" << std::endl
		<< "There were errors during thread execution:" << std::endl
		<< std::endl;

		for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
			glogger << *it << std::endl;
		}

		glogger << std::endl
		<< "========================================================================" << std::endl
		<< GEXCEPTION;
	}

	// Terminate our thread pool
	tp_ptr_.reset();

	// Finalize the broker connector
	Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>::finalize();

	// GBaseSA sees exactly the environment it would when called from its own class
	GBaseSA::finalize();
}

/******************************************************************************/
/**
 * Checks whether this algorithm communicates via the broker. This is an overload from the corresponding
 * GOptimizableI function
 *
 * @return A boolean indicating whether this algorithm communicates via the broker
 */
bool GBrokerSA::usesBroker() const {
	return true;
}

/******************************************************************************/
/**
 * Adapt all children in parallel. Evaluation is done in a seperate function (runFitnessCalculation).
 */
void GBrokerSA::adaptChildren() {
	std::tuple<std::size_t, std::size_t> range = getAdaptionRange();
	std::vector<std::shared_ptr < GParameterSet>> ::iterator
	it;

	for (it = data.begin() + std::get<0>(range); it != data.begin() + std::get<1>(range); ++it) {
		tp_ptr_->async_schedule([it]() { (*it)->adapt(); });
	}

	// Wait for all threads in the pool to complete their work
	tp_ptr_->wait();
}

/******************************************************************************/
/**
 * We submit individuals to the broker and wait for processed items.
 */
void GBrokerSA::runFitnessCalculation() {
	//--------------------------------------------------------------------------------
	// Start by marking the work to be done in the individuals.
	// "range" will hold the start- and end-points of the range
	// to be worked on
	std::tuple<std::size_t, std::size_t> range = getEvaluationRange();

#ifdef DEBUG
   // There should be no situation in which a "clean" child is submitted
   // through this function. There MAY be situations, where in the first iteration
   // parents are clean, e.g. when they were extracted from another optimization.
   for(std::size_t i=this->getNParents(); i<this->size(); i++) {
      if(!this->at(i)->isDirty()) {
         glogger
         << "In GBrokerSA::runFitnessCalculation(): Error!" << std::endl
         << "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
         << "but found \"clean\" individual in position " << i << std::endl
         << GEXCEPTION;
      }
   }
#endif

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results.
	Gem::Courtier::GBrokerConnectorT<GParameterSet>::workOn(
		data, range, oldWorkItems_, true // Remove unprocessed items
	);

	//--------------------------------------------------------------------------------
	// Now fix the population -- it may be smaller than its nominal size
	fixAfterJobSubmission();
}

/******************************************************************************/
/**
 * Fixes the population after a job submission
 */
void GBrokerSA::fixAfterJobSubmission() {
	std::vector<std::shared_ptr < GParameterSet>> ::iterator
	it;
	std::size_t np = getNParents();
	std::uint32_t iteration = getIteration();

	// Remove parents from older iterations from old work items -- we do not want them.
	// Note that "remove_if" simply moves items not satisfying the predicate to the end of the list.
	// We thus need to explicitly erase these items. remove_if returns the iterator position right after
	// the last item not satisfying the predicate.
	oldWorkItems_.erase(
		std::remove_if(oldWorkItems_.begin(), oldWorkItems_.end(), isOldParent(iteration)), oldWorkItems_.end()
	);

	// Make it known to remaining old individuals that they are now part of a new iteration
	std::for_each(
		oldWorkItems_.begin(), oldWorkItems_.end(),
		[iteration](std::shared_ptr <GParameterSet> p) { p->setAssignedIteration(iteration); }
	);

	// Make sure that parents are at the beginning of the array.
	sort(data.begin(), data.end(), indParentComp());

	// Attach all old work items to the end of the current population and clear the array of old items
	for (it = oldWorkItems_.begin(); it != oldWorkItems_.end(); ++it) {
		data.push_back(*it);
	}
	oldWorkItems_.clear();

	// Add missing individuals, as clones of the last item
	if (data.size() < getDefaultPopulationSize()) {
		std::size_t fixSize = getDefaultPopulationSize() - data.size();
		for (std::size_t i = 0; i < fixSize; i++) {
			// This function will create a clone of its argument
			this->push_back_clone(data.back());
		}
	}

	// Mark the first nParents_ individuals as parents in the first iteration. We want to have a "sane" population.
	if (inFirstIteration()) {
		for (it = this->begin(); it != this->begin() + np; ++it) {
			(*it)->getPersonalityTraits<GSAPersonalityTraits>()->setIsParent();
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
void GBrokerSA::selectBest() {
	////////////////////////////////////////////////////////////
	// Great - we are at least at the default level and are
	// ready to call the actual selectBest() function. This will
	// automatically take care of the selection modes.
	GBaseSA::selectBest();

#ifdef DEBUG
   // Make sure our population is not smaller than its nominal size -- this
   // should have been taken care of in fixAfterJobSubmission() .
   if(data.size() < getDefaultPopulationSize()) {
      glogger
      << "In GBrokerSA::selectBest(): Error!" << std::endl
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
 */
void GBrokerSA::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GBaseSA::addConfigurationOptions(gpb);
	Gem::Courtier::GBrokerConnectorT<GParameterSet>::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<std::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, [this](std::uint16_t nt) { this->setNThreads(nt); }
	)
	<< "The number of threads used to simultaneously adapt individuals";
}

/******************************************************************************/
/**
 * Sets the number of threads this population uses for adaption. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the number of hardware
 * threading units (e.g. number of cores or hyperthreading units).
 *
 * @param nThreads The number of threads this class uses
 */
void GBrokerSA::setNThreads(std::uint16_t nThreads) {
	if (nThreads == 0) {
		nThreads_ = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
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
std::uint16_t GBrokerSA::getNThreads() const {
	return nThreads_;
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerSA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GBrokerSA::getIndividualCharacteristic() const {
	return std::string("GENEVA_BROKEROPTALG");
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerSA::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseSA::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerSA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerSA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSA::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerSA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerSA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSA::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerSA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
