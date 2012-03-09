/**
 * @file GMultiThreadedEA.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "geneva/GMultiThreadedEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedEA)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GMultiThreadedEA::GMultiThreadedEA()
   : GBaseEA()
   , nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSEA)))
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor. Note that we do not copy serverMode_ as
 * it is used for internal caching only.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GMultiThreadedEA::GMultiThreadedEA(const GMultiThreadedEA& cp)
   : GBaseEA(cp)
   , nThreads_(cp.nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedEA::~GMultiThreadedEA()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard assignment operator for GMultiThreadedEA objects.
 *
 * @param cp Reference to another GMultiThreadedEA object
 * @return A constant reference to this object
 */
const GMultiThreadedEA& GMultiThreadedEA::operator=(const GMultiThreadedEA& cp) {
	GMultiThreadedEA::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data from another GMultiThreadedEA object.
 *
 * @param vp Pointer to another GMultiThreadedEA object, camouflaged as a GObject
 */
void GMultiThreadedEA::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	const GMultiThreadedEA *p_load = this->gobject_conversion<GMultiThreadedEA>(cp);

	// First load our parent class'es data ...
	GBaseEA::load_(cp);

	// ... and then our own
	nThreads_ = p_load->nThreads_;

	// Note that we do not copy serverMode_ as it is used for internal caching only
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedEA::clone_() const  {
	return new GMultiThreadedEA(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GMultiThreadedEA object
 *
 * @param  cp A constant reference to another GMultiThreadedEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GMultiThreadedEA::operator==(const GMultiThreadedEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMultiThreadedEA::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GMultiThreadedEA object
 *
 * @param  cp A constant reference to another GMultiThreadedEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMultiThreadedEA::operator!=(const GMultiThreadedEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMultiThreadedEA::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GMultiThreadedEA::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GMultiThreadedEA *p_load = GObject::gobject_conversion<GMultiThreadedEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseEA::checkRelationshipWith(cp, e, limit, "GMultiThreadedEA", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GMultiThreadedEA", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

	return evaluateDiscrepancies("GMultiThreadedEA", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedEA::init() {
	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::init();

	// Initialize our thread pool
	tp_.reset(new Gem::Common::GThreadPool(nThreads_));

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	// The function will throw if not all individuals have the same server mode flag.

	// Set the server mode and store the original flag
	bool first = true;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		if(first){
			serverMode_ = (*it)->getServerMode();
			first = false;
		}

		if(serverMode_ != (*it)->setServerMode(true)) {
			raiseException(
				"In GMultiThreadedEA::init():" << std::endl
				<< "Not all server mode flags have the same value!"
			);
		}
	}
}

/************************************************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedEA::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(serverMode_);
	}

	// Terminate our thread pool
	tp_.reset();

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::finalize();
}

/************************************************************************************************************/
/**
 * An overloaded version of GBaseEA::adaptChildren() . Adaption
 * and evaluation of children is handled by threads in a thread pool. The maximum
 * number of threads is DEFAULTBOOSTTHREADSEA (possibly 2) and can be overridden
 * with the GMultiThreadedEA::setMaxThreads() function.
 */
void GMultiThreadedEA::adaptChildren() {
	std::size_t nParents = GBaseEA::getNParents();
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We start with the parents, if this is the first iteration. Their
	// initial fitness needs to be determined in some selection models.
	// Make sure we also evaluate the parents in the first iteration, if needed.
	if(inFirstIteration()) {
		switch(getSortingScheme()) {
		//--------------------------------------------------------------
		case SA_SINGLEEVAL:
		case MUPLUSNU_SINGLEEVAL:
		case MUPLUSNU_PARETO:
		case MUCOMMANU_PARETO: // The current setup will still allow some old parents to become new parents
		case MUNU1PRETAIN_SINGLEEVAL: // same procedure. We do not know which parent is best
		{
			for(it=data.begin(); it!=data.begin() + nParents; ++it) {
				// Make re-evaluation accessible
				(*it)->setServerMode(false);
				// Schedule the actual job; enforce fitness calculation of parents
				tp_->schedule(boost::function<double()>(boost::bind(&GIndividual::doFitnessCalculation, *it)));
			}
		}
			break;

		case MUCOMMANU_SINGLEEVAL:
			break; // nothing
		}
	}

	// Next we adapt the children
	for(it=data.begin()+nParents; it!=data.end(); ++it) {
		// Make re-evaluation accessible
		(*it)->setServerMode(false);
		// Schedule the actual job
		tp_->schedule(boost::function<double()>(boost::bind(&GIndividual::adaptAndEvaluate, *it)));
	}

	// ... and wait for all tasks to complete
	tp_->wait();

	// Restart the server mode for parents
	if(inFirstIteration()) {
		switch(getSortingScheme()) {
		//--------------------------------------------------------------
		case SA_SINGLEEVAL:
		case MUPLUSNU_SINGLEEVAL:
		case MUPLUSNU_PARETO:
		case MUCOMMANU_PARETO: // The current setup will still allow s	ome old parents to become new parents
		case MUNU1PRETAIN_SINGLEEVAL: // same procedure
			for(it=data.begin(); it!=data.begin() + nParents; ++it) {
				// Make re-evaluation impossible
				(*it)->setServerMode(true);
			}
			break;

		case MUCOMMANU_SINGLEEVAL:
			break; // nothing
		}
	}

	// Restart the server mode for children
	for(it=data.begin()+nParents; it!=data.end(); ++it) {
		(*it)->setServerMode(true);
	}
}

/************************************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GMultiThreadedEA::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GBaseEA::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	comment = ""; // Reset the comment string
	comment += "The number of threads used to simultaneously process individuals;";
	if(showOrigin) comment += "[GMultiThreadedEA]";
	gpb.registerFileParameter<boost::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, boost::bind(
			&GMultiThreadedEA::setNThreads
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);
}

/************************************************************************************************************/
/**
 * Sets the number of threads for this population. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the
 * number of hardware threading units (e.g. number of cores or hyperthreading
 * units).
 *
 * @param nThreads The number of threads this class uses
 */
void GMultiThreadedEA::setNThreads(boost::uint16_t nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSEA));
	}
	else {
		nThreads_ = nThreads;
	}
}

/************************************************************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
boost::uint16_t GMultiThreadedEA::getNThreads() const  {
	return nThreads_;
}

#ifdef GEM_TESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedEA::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GBaseEA::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedEA::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseEA::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedEA::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseEA::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */
