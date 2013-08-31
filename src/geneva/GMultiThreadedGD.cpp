/**
 * @file GMultiThreadedGD.cpp
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

#include "geneva/GMultiThreadedGD.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedGD)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GMultiThreadedGD::GMultiThreadedGD()
	: GBaseGD()
	, nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
   , storedServerMode_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of starting points and the size of the finite step
 */
GMultiThreadedGD::GMultiThreadedGD (
		const std::size_t& nStartingPoints
		, const float& finiteStep
		, const float& stepSize
)
	: GBaseGD(nStartingPoints, finiteStep, stepSize)
	, nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
   , storedServerMode_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 */
GMultiThreadedGD::GMultiThreadedGD(const GMultiThreadedGD& cp)
	: GBaseGD(cp)
	, nThreads_(cp.nThreads_)
   , storedServerMode_(cp.storedServerMode_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedGD::~GMultiThreadedGD()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator for GMultiThreadedGD objects.
 *
 * @param cp Reference to another GMultiThreadedGD object
 * @return A constant reference to this object
 */
const GMultiThreadedGD& GMultiThreadedGD::operator=(const GMultiThreadedGD& cp) {
	GMultiThreadedGD::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GMultiThreadedGD object
 *
 * @param  cp A constant reference to another GMultiThreadedGD object
 * @return A boolean indicating whether both objects are equal
 */
bool GMultiThreadedGD::operator==(const GMultiThreadedGD& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMultiThreadedGD::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GMultiThreadedGD object
 *
 * @param  cp A constant reference to another GMultiThreadedGD object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMultiThreadedGD::operator!=(const GMultiThreadedGD& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMultiThreadedGD::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GMultiThreadedGD::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GMultiThreadedGD *p_load = GObject::gobject_conversion<GMultiThreadedGD>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseGD::checkRelationshipWith(cp, e, limit, "GMultiThreadedGD", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GMultiThreadedGD", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

	return evaluateDiscrepancies("GMultiThreadedGD", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GMultiThreadedGD::name() const {
   return std::string("GMultiThreadedGD");
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
void GMultiThreadedGD::setNThreads(boost::uint16_t nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
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
boost::uint16_t GMultiThreadedGD::getNThreads() const  {
	return nThreads_;
}

/******************************************************************************/
/**
 * Loads the data from another GMultiThreadedGD object.
 *
 * @param vp Pointer to another GMultiThreadedGD object, camouflaged as a GObject
 */
void GMultiThreadedGD::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	const GMultiThreadedGD *p_load = this->gobject_conversion<GMultiThreadedGD>(cp);

	// First load our parent class'es data ...
	GBaseGD::load_(cp);

	// ... and then our own
	nThreads_ = p_load->nThreads_;

	// Note that we do not copy storedServerMode_ as it is used for internal caching only
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedGD::clone_() const  {
	return new GMultiThreadedGD(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedGD::init() {
	// GBaseGD sees exactly the environment it would when called from its own class
	GBaseGD::init();

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
		   << "In GMultiThreadedGD::init():" << std::endl
         << "Not all server mode flags have the same value!" << std::endl
         << GEXCEPTION;
		}
	}
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedGD::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(storedServerMode_);
	}

	// Terminate our thread pool
	tp_.reset();

	// GBaseGD sees exactly the environment it would when called from its own class
	GBaseGD::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GMultiThreadedGD::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GBaseGD::addConfigurationOptions(gpb, showOrigin);

	// add local data
	comment = ""; // Reset the comment string
	comment += "The number of evaluation threads;";
	comment += "0 means: determine automatically;";
	if(showOrigin) comment += "[GMultiThreadedGD]";
	gpb.registerFileParameter<boost::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, boost::bind(
			&GMultiThreadedGD::setNThreads
			, this
			, _1
		)
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GMultiThreadedGD::getIndividualCharacteristic() const {
	return std::string("GENEVA_MTOPTALG");
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBaseGD, albeit multi-threaded.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
double GMultiThreadedGD::doFitnessCalculation(const std::size_t& finalPos) {
	GMultiThreadedGD::iterator it; // An iterator that allows us to loop over the collection
	double bestFitness = getWorstCase(); // Holds the best fitness found so far
	std::size_t nStartingPoints = this->getNStartingPoints(); // The number of simultaneous starting points

#ifdef DEBUG
	if(finalPos > this->size()) {
	   glogger
	   << "In GMultiThreadedGD::doFitnessCalculation(const std::size_t&):" << std::endl
      << "Got invalid final position: " << finalPos << "/" << this->size() << std::endl
      << GEXCEPTION;
	}

	if(finalPos < nStartingPoints) {
	   glogger
	   << "In GMultiThreadedGD::doFitnessCalculation(const std::size_t&):" << std::endl
      << "We require finalPos to be at least " << nStartingPoints << ", but got " << finalPos << std::endl
      << GEXCEPTION;
	}
#endif

	// Trigger value calculation for all individuals (including parents)
	for(it=this->begin(); it!=this->begin() + finalPos; ++it) {
#ifdef DEBUG
		// Make sure the evaluated individuals have the dirty flag set
		if(afterFirstIteration() && !(*it)->isDirty()) {
		   glogger
		   << "In GMultiThreadedGD::doFitnessCalculation(const std::size_t&):" << std::endl
         << "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag isn't set" << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		// Make sure we are allowed to perform value calculation
		(*it)->setServerMode(false);

		// Submit the actual task
		tp_->schedule(boost::function<double()>(boost::bind(&GParameterSet::fitness, *it, 0)));
	}

	// wait for the pool to run out of tasks
	tp_->wait();

	// Retrieve information about the best fitness found and disallow re-evaluation
	double fitnessFound = 0.;
	for(it=this->begin(); it!=this->begin() + finalPos; ++it) {
		// Prevents re-evaluation
		(*it)->setServerMode(true);

		if(boost::numeric_cast<std::size_t>(std::distance(this->begin(), it)) < nStartingPoints) {
			fitnessFound = (*it)->fitness(0);

			if(isBetter(fitnessFound, bestFitness)) {
				bestFitness = fitnessFound;
			}
		}
	}

	return bestFitness;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedGD::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GBaseGD::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedGD::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedGD::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseGD::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedGD::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedGD::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseGD::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedGD::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING
// Tests of this class (and parent classes)
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GMultiThreadedGD has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GMultiThreadedGD> TFactory_GUnitTests<Gem::Geneva::GMultiThreadedGD>() {
	boost::shared_ptr<Gem::Geneva::GMultiThreadedGD> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GMultiThreadedGD>(new Gem::Geneva::GMultiThreadedGD()));
	return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */
