/**
 * @file GMultiThreadedEA.cpp
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

#include "geneva/GMultiThreadedEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedEA)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty. getNHardwareThreads will default to
 * a constant number of threads, if the number of hardware threads
 * cannot be determined.
 */
GMultiThreadedEA::GMultiThreadedEA()
	: GBaseEA(), nThreads_(boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads())) { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GMultiThreadedEA::GMultiThreadedEA(const GMultiThreadedEA &cp)
	: GBaseEA(cp), nThreads_(cp.nThreads_) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor.
 */
GMultiThreadedEA::~GMultiThreadedEA() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GMultiThreadedEA &GMultiThreadedEA::operator=(const GMultiThreadedEA &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Loads the data from another GMultiThreadedEA object.
 *
 * @param vp Pointer to another GMultiThreadedEA object, camouflaged as a GObject
 */
void GMultiThreadedEA::load_(const GObject *cp) {
	// Check that we are dealing with a GMultiThreadedEA reference independent of this object and convert the pointer
	const GMultiThreadedEA *p_load = Gem::Common::g_convert_and_compare<GObject, GMultiThreadedEA>(cp, this);

	// First load our parent class'es data ...
	GBaseEA::load_(cp);

	// ... and then our own
	nThreads_ = p_load->nThreads_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedEA::clone_() const {
	return new GMultiThreadedEA(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GMultiThreadedEA object
 *
 * @param  cp A constant reference to another GMultiThreadedEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GMultiThreadedEA::operator==(const GMultiThreadedEA &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GMultiThreadedEA object
 *
 * @param  cp A constant reference to another GMultiThreadedEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMultiThreadedEA::operator!=(const GMultiThreadedEA &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
void GMultiThreadedEA::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GMultiThreadedEA reference independent of this object and convert the pointer
	const GMultiThreadedEA *p_load = Gem::Common::g_convert_and_compare<GObject, GMultiThreadedEA>(cp, this);

	GToken token("GMultiThreadedEA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseEA>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(nThreads_, p_load->nThreads_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GMultiThreadedEA::name() const {
	return std::string("GMultiThreadedEA");
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedEA::init() {
	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::init();

	// Initialize our thread pool
	tp_ptr_.reset(new Gem::Common::GThreadPool(nThreads_));
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedEA::finalize() {
	// Check whether there were any errors during thread execution
	if (tp_ptr_->hasErrors()) {
		std::ostringstream oss;
		std::vector<std::string> errors;
		errors = tp_ptr_->getErrors();

		for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
			oss << *it << std::endl;
		}

		glogger
		<< "Warning: There were errors during thread execution in GThreadPool:" << std::endl
		<< oss.str() << std::endl
		<< GWARNING;
	}

	// Terminate our thread pool
	tp_ptr_.reset();

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::finalize();
}

/******************************************************************************/
/**
 * Adapt all children in parallel. Evaluation is done in a seperate function (runFitnessCalculation).
 */
void GMultiThreadedEA::adaptChildren() {
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
 * Evaluate all children (and possibly parents, depending on the iteration and sorting mode) in parallel
 */
void GMultiThreadedEA::runFitnessCalculation() {
	std::tuple<std::size_t, std::size_t> range = getEvaluationRange();
	std::vector<std::shared_ptr < GParameterSet>> ::iterator
	it;

#ifdef DEBUG
   // There should be no situation in which a "clean" child is submitted
   // through this function. There MAY be situations, where in the first iteration
	// parents are clean, e.g. when they were extracted from another optimization.
   for(std::size_t i=this->getNParents(); i<this->size(); i++) {
      if(!this->at(i)->isDirty()) {
         glogger
         << "In GMultiThreadedEA::runFitnessCalculation(): Error!" << std::endl
         << "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
         << "but found \"clean\" individual in position " << i << std::endl
         << GEXCEPTION;
      }
   }
#endif

	// Make evaluation possible and initiate the worker threads
	for (it = data.begin() + std::get<0>(range); it != data.begin() + std::get<1>(range); ++it) {
		// Do the actual scheduling
		tp_ptr_->async_schedule(
			[it]() { (*it)->nonConstFitness(0, ALLOWREEVALUATION, USETRANSFORMEDFITNESS); }
		);
	}

	// Wait for all threads in the pool to complete their work
	tp_ptr_->wait();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GMultiThreadedEA::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GBaseEA::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<std::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, [this](std::uint16_t nt) { this->setNThreads(nt); }
	)
	<< "The number of threads used to simultaneously process individuals";
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GMultiThreadedEA::getIndividualCharacteristic() const {
	return std::string("GENEVA_MTOPTALG");
}

/******************************************************************************/
/**
 * Sets the number of threads for this population. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the
 * number of hardware threading units (e.g. number of cores or hyperthreading
 * units).
 *
 * @param nThreads The number of threads this class uses
 */
void GMultiThreadedEA::setNThreads(std::uint16_t nThreads) {
	if (nThreads == 0) {
		nThreads_ = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
	}
	else {
		nThreads_ = nThreads;
	}
}

/******************************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
std::uint16_t GMultiThreadedEA::getNThreads() const {
	return nThreads_;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedEA::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseEA::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedEA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedEA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseEA::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedEA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedEA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseEA::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
