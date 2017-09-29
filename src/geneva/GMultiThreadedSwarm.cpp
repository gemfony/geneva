/**
 * @file GMultiThreadedSwarm.cpp
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

#include "geneva/GMultiThreadedSwarm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedSwarm)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. No local, dynamically allocated data, hence this function is empty.
 */
GMultiThreadedSwarm::GMultiThreadedSwarm()
	: GBaseSwarm(), nThreads_(
	boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNSTDTHREADS))) { /* nothing */ }

/******************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data, hence this function is empty.
 */
GMultiThreadedSwarm::GMultiThreadedSwarm(
	const std::size_t &nNeighborhoods, const std::size_t &nNeighborhoodMembers
)
	: GBaseSwarm(nNeighborhoods, nNeighborhoodMembers), nThreads_(
	boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNSTDTHREADS))) { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GMultiThreadedSwarm::GMultiThreadedSwarm(const GMultiThreadedSwarm &cp)
	: GBaseSwarm(cp), nThreads_(cp.nThreads_) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedSwarm::~GMultiThreadedSwarm() { /* nothing */ }

/******************************************************************************/
/**
 * Loads the data from another GMultiThreadedSwarm object.
 *
 * @param vp Pointer to another GMultiThreadedSwarm object, camouflaged as a GObject
 */
void GMultiThreadedSwarm::load_(const GObject *cp) {
	// Check that we are dealing with a GMultiThreadedSwarm reference independent of this object and convert the pointer
	const GMultiThreadedSwarm *p_load = Gem::Common::g_convert_and_compare<GObject, GMultiThreadedSwarm>(cp, this);

	// First load our parent class'es data ...
	GBaseSwarm::load_(cp);

	// ... and then our own
	nThreads_ = p_load->nThreads_;
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GMultiThreadedSwarm &GMultiThreadedSwarm::operator=(const GMultiThreadedSwarm &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GMultiThreadedSwarm object
 *
 * @param  cp A constant reference to another GMultiThreadedSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GMultiThreadedSwarm::operator==(const GMultiThreadedSwarm &cp) const {
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
 * Checks for inequality with another GMultiThreadedSwarm object
 *
 * @param  cp A constant reference to another GMultiThreadedSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMultiThreadedSwarm::operator!=(const GMultiThreadedSwarm &cp) const {
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
void GMultiThreadedSwarm::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GMultiThreadedSwarm reference independent of this object and convert the pointer
	const GMultiThreadedSwarm *p_load = Gem::Common::g_convert_and_compare<GObject, GMultiThreadedSwarm>(cp, this);

	GToken token("GMultiThreadedSwarm", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseSwarm>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(nThreads_, p_load->nThreads_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GMultiThreadedSwarm::name() const {
	return std::string("GMultiThreadedSwarm");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedSwarm::clone_() const {
	return new GMultiThreadedSwarm(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedSwarm::init() {
	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::init();

	// Initialize our thread pool
	tp_ptr_.reset(new Gem::Common::GThreadPool(nThreads_));
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedSwarm::finalize() {
	// Check whether there were any errors during thread execution
	if (tp_ptr_->hasErrors()) {
		std::ostringstream oss;

		std::vector<std::string> errors;
		errors = tp_ptr_->getErrors();

		oss
		<< "========================================================================" << std::endl
		<< "In GMultiThreadedSwarm::finalize(): WARNING" << std::endl
		<< "There were errors during thread execution:" << std::endl
		<< std::endl;

		for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
			oss << *it << std::endl;
		}

		oss << std::endl
		<< "========================================================================" << std::endl;

		glogger // We cannot currently interrupt glogger input, all input must be transferred in one go
		<< oss.str()
		<< GWARNING;
	}

	// Terminate our thread pool
	tp_ptr_.reset();

	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GMultiThreadedSwarm::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GBaseSwarm::addConfigurationOptions(gpb);

	// add local data
	gpb.registerFileParameter<std::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, [this](std::uint16_t nt) { this->setNThreads(nt); }
	)
	<< "The number of evaluation threads" << std::endl
	<< "0 means: determine automatically";
}

/******************************************************************************/
/**
 * Updates the fitness of all individuals
 */
void GMultiThreadedSwarm::runFitnessCalculation() {
	GMultiThreadedSwarm::iterator it;
	for (it = this->begin(); it != this->end(); ++it) {
		// Do the actual scheduling
		tp_ptr_->async_schedule(
			[it]() { (*it)->process(); }
		);
	}

	// wait for the pool to run out of tasks
	tp_ptr_->wait();
}

/******************************************************************************/
/**
 * Sets the number of threads for this population. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the
 * number of hardware threading units (e.g. number of cores or hyper-threading
 * units).
 *
 * @param nThreads The number of threads this class uses
 */
void GMultiThreadedSwarm::setNThreads(std::uint16_t nThreads) {
	if (nThreads == 0) {
		nThreads_ = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNSTDTHREADS));
	} else {
		nThreads_ = nThreads;
	}
}

/******************************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
std::uint16_t GMultiThreadedSwarm::getNThreads() const {
	return nThreads_;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedSwarm::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseSwarm::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedSwarm::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSwarm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedSwarm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

