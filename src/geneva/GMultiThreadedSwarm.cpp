/**
 * @file GMultiThreadedSwarm.cpp
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

#include "geneva/GMultiThreadedSwarm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedSwarm)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor. No local, dynamically allocated data, hence this function is empty.
 */
GMultiThreadedSwarm::GMultiThreadedSwarm()
   : GBaseSwarm()
   , nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSSWARM)))
   , tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data, hence this function is empty.
 */
GMultiThreadedSwarm::GMultiThreadedSwarm(
		const std::size_t& nNeighborhoods
		, const std::size_t& nNeighborhoodMembers
)
   : GBaseSwarm(nNeighborhoods, nNeighborhoodMembers)
   , nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSSWARM)))
   , tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GMultiThreadedSwarm::GMultiThreadedSwarm(const GMultiThreadedSwarm& cp)
   : GBaseSwarm(cp)
   , nThreads_(cp.nThreads_)
   , tp_(nThreads_) // Make sure we initialize the threadpool appropriately
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedSwarm::~GMultiThreadedSwarm() {
	tp_.clear();
	tp_.wait();
}

/************************************************************************************************************/
/**
 * A standard assignment operator for GMultiThreadedSwarm objects.
 *
 * @param cp Reference to another GMultiThreadedSwarm object
 * @return A constant reference to this object
 */
const GMultiThreadedSwarm& GMultiThreadedSwarm::operator=(const GMultiThreadedSwarm& cp) {
	GMultiThreadedSwarm::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data from another GMultiThreadedSwarm object.
 *
 * @param vp Pointer to another GMultiThreadedSwarm object, camouflaged as a GObject
 */
void GMultiThreadedSwarm::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	const GMultiThreadedSwarm *p_load = this->gobject_conversion<GMultiThreadedSwarm>(cp);

	// First load our parent class'es data ...
	GBaseSwarm::load_(cp);

	// ... and then our own
	if(nThreads_ != p_load->nThreads_) {
		nThreads_ = p_load->nThreads_;
		tp_.clear(); // TODO: is this needed ?
		tp_.size_controller().resize(nThreads_);
	}
}

/************************************************************************************************************/
/**
 * Checks for equality with another GMultiThreadedSwarm object
 *
 * @param  cp A constant reference to another GMultiThreadedSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GMultiThreadedSwarm::operator==(const GMultiThreadedSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMultiThreadedSwarm::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GMultiThreadedSwarm object
 *
 * @param  cp A constant reference to another GMultiThreadedSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMultiThreadedSwarm::operator!=(const GMultiThreadedSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMultiThreadedSwarm::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GMultiThreadedSwarm::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GMultiThreadedSwarm *p_load = GObject::gobject_conversion<GMultiThreadedSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseSwarm::checkRelationshipWith(cp, e, limit, "GMultiThreadedSwarm", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GMultiThreadedSwarm", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

	return evaluateDiscrepancies("GMultiThreadedSwarm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedSwarm::clone_() const  {
	return new GMultiThreadedSwarm(*this);
}

/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedSwarm::init() {
	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::init();

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	// The function will throw if not all individuals have the same server mode flag.

	// Set the server mode and store the original flag
	bool first = true;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		if(first){
			serverMode_ = (*it)->getServerMode();
			first = false;
		}

		if(serverMode_ != (*it)->setServerMode(true)) {
			raiseException(
				"In GMultiThreadedSwarm::init():" << std::endl
				<< "Not all server mode flags have the same vaue!"
			);
		}
	}
}

/************************************************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedSwarm::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(serverMode_);
	}

	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::finalize();
}

/************************************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GMultiThreadedSwarm::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GBaseSwarm::addConfigurationOptions(gpb, showOrigin);

	// add local data
	comment = ""; // Reset the comment string
	comment += "The number of evaluation threads;";
	comment += "0 means: determine automatically;";
	if(showOrigin) comment += "[GMultiThreadedSwarm]";
	gpb.registerFileParameter<boost::uint16_t>(
		"nEvaluationThreads" // The name of the variable
		, 0 // The default value
		, boost::bind(
			&GMultiThreadedSwarm::setNThreads
			, this
			, _1
		)
		, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
		, comment
	);
}

/************************************************************************************************************/
/**
 * Updates the fitness of all individuals
 */
void GMultiThreadedSwarm::updateFitness() {
	std::size_t offset = 0;
	GBaseSwarm::iterator start = this->begin();
	boost::uint32_t iteration = getIteration();

	// Then start the evaluation threads
	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
		for(std::size_t member=0; member<nNeighborhoodMembers_[neighborhood]; member++) {
			GMultiThreadedSwarm::iterator current = start + offset;

			// Schedule the fitness calculation as a thread
			tp_.schedule(
				Gem::Common::GThreadWrapper(
					boost::bind(
					    &GMultiThreadedSwarm::updateIndividualFitness
						, this
						, iteration
						, neighborhood
						, (*current)
					)
				)
			);

			offset++;
		}
	}

	// wait for the pool to run out of tasks
	tp_.wait();
}

/************************************************************************************************************/
/**
 * Sets the number of threads for this population. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the
 * number of hardware threading units (e.g. number of cores or hyper-threading
 * units).
 *
 * @param nThreads The number of threads this class uses
 */
void GMultiThreadedSwarm::setNThreads(boost::uint16_t nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSSWARM));
	}
	else {
		nThreads_ = nThreads;
	}

	tp_.size_controller().resize(nThreads_);
}

/************************************************************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
boost::uint16_t GMultiThreadedSwarm::getNThreads() const  {
	return nThreads_;
}

#ifdef GEM_TESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedSwarm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GBaseSwarm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseSwarm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseSwarm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/**
 * As Gem::Geneva::GMultiThreadedSwarm has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GMultiThreadedSwarm> TFactory_GUnitTests<Gem::Geneva::GMultiThreadedSwarm>() {
	boost::shared_ptr<Gem::Geneva::GMultiThreadedSwarm> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GMultiThreadedSwarm>(new Gem::Geneva::GMultiThreadedSwarm(5,10)));
	return p;
}

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GEM_TESTING */
