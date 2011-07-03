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
 * A standard constructor. No local, dynamically allocated data, hence this function is empty.
 */
GMultiThreadedSwarm::GMultiThreadedSwarm(
		const std::size_t& nNeighborhoods
		, const std::size_t& nNeighborhoodMembers
)
   : GSwarm(nNeighborhoods, nNeighborhoodMembers)
   , nThreads_(boost::numeric_cast<boost::uint8_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSSWARM)))
   , tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GMultiThreadedSwarm::GMultiThreadedSwarm(const GMultiThreadedSwarm& cp)
   : GSwarm(cp)
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
	const GMultiThreadedSwarm *p_load = this->conversion_cast<GMultiThreadedSwarm>(cp);

	// First load our parent class'es data ...
	GSwarm::load_(cp);

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
	const GMultiThreadedSwarm *p_load = GObject::conversion_cast<GMultiThreadedSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GSwarm::checkRelationshipWith(cp, e, limit, "GMultiThreadedSwarm", y_name, withMessages));

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
	// GSwarm sees exactly the environment it would when called from its own class
	GSwarm::init();

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	sm_value_.clear(); // Make sure we do not have "left-overs"
	// Set the server mode and store the original flag
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		sm_value_.push_back((*it)->setServerMode(true));
	}
}

/************************************************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedSwarm::finalize() {
#ifdef DEBUG
	if(data.size() != sm_value_.size()) {
		raiseException(
				"In GMultiThreadedSwarm::finalize():" << std::endl
				<< "Invalid number of serverMode flags: " << data.size() << "/" << sm_value_.size()
		);
	}
#endif /* DEBUG */

	// Restore the original values
	std::vector<bool>::iterator b_it;
	std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
	for(it=data.begin(), b_it=sm_value_.begin(); it!=data.end(); ++it, ++b_it) {
		(*it)->setServerMode(*b_it);
	}
	sm_value_.clear(); // Make sure we have no "left-overs"

	// GSwarm sees exactly the environment it would when called from its own class
	GSwarm::finalize();
}

/************************************************************************************************************/
/**
 * Updates the fitness of all individuals. This is an overloaded version of the parent class'es function
 * which enqueues tasks in the thread pool.
 */
void GMultiThreadedSwarm::swarmLogic() {
	std::size_t offset = 0;
	GMultiThreadedSwarm::iterator start = this->begin();
	boost::uint32_t iteration = getIteration();

	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
#ifdef DEBUG
		if(getIteration() > 0) {
			if(!local_bests_[neighborhood]) {
				raiseException(
						"In GMultiThreadedSwarm::swarmLogic():" << std::endl
						<< "local_bests[" << neighborhood << "] is empty."
				);
			}

			if(neighborhood==0 && !global_best_) { // Only check for the first neighborhood
				raiseException(
						"In GMultiThreadedSwarm::swarmLogic():" << std::endl
						<< "global_best_ is empty."
				);
			}
		}
#endif /* DEBUG */

		for(std::size_t member=0; member<nNeighborhoodMembers_[neighborhood]; member++) {
			GMultiThreadedSwarm::iterator current = start + offset;

			// Schedule position update and fitness calculation as a thread
			// Note: global/local bests and velocities haven't been determined yet in iteration 0 and are not needed there
			tp_.schedule(
				Gem::Common::GThreadWrapper(
					boost::bind(
						&GMultiThreadedSwarm::updateSwarm
						, this
						, iteration
						, neighborhood
						, *current
						, iteration>0?(local_bests_[neighborhood]->clone<GParameterSet>()):boost::shared_ptr<GParameterSet>()
		#ifdef DEBUG
						, iteration>0?velocities_.at(offset):boost::shared_ptr<GParameterSet>()
		#else
						, iteration>0?velocities_[offset]:boost::shared_ptr<GParameterSet>()
		#endif /* DEBUG */
						, boost::make_tuple(
							getCPersonal()
							, getCLocal()
							, getCDelta()
						)
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
void GMultiThreadedSwarm::setNThreads(const boost::uint8_t& nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint8_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSSWARM));
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
uint8_t GMultiThreadedSwarm::getNThreads() const  {
	return nThreads_;
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedSwarm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GSwarm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GSwarm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GSwarm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
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
#endif /* GENEVATESTING */
