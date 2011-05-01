/**
 * @file GMultiThreadedGD.cpp
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

#include "geneva/GMultiThreadedGD.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedGD)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor
 */
GMultiThreadedGD::GMultiThreadedGD()
	: GGradientDescent()
	, nThreads_(boost::numeric_cast<boost::uint8_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSGD)))
	, tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Initialization with the number of starting points and the size of the finite step
 */
GMultiThreadedGD::GMultiThreadedGD (
		const std::size_t& nStartingPoints
		, const float& finiteStep
		, const float& stepSize
)
	: GGradientDescent(nStartingPoints, finiteStep, stepSize)
	, nThreads_(boost::numeric_cast<boost::uint8_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSGD)))
	, tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor
 */
GMultiThreadedGD::GMultiThreadedGD(const GMultiThreadedGD& cp)
	: GGradientDescent(cp)
	, nThreads_(cp.nThreads_)
	, tp_(nThreads_) // Make sure we initialize the threadpool appropriately
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedGD::~GMultiThreadedGD() {
	tp_.clear();
	tp_.wait();
}

/************************************************************************************************************/
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

/************************************************************************************************************/
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

/************************************************************************************************************/
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
	const GMultiThreadedGD *p_load = GObject::conversion_cast<GMultiThreadedGD>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GGradientDescent::checkRelationshipWith(cp, e, limit, "GMultiThreadedGD", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GMultiThreadedGD", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

	return evaluateDiscrepancies("GMultiThreadedGD", caller, deviations, e);
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
void GMultiThreadedGD::setNThreads(const boost::uint8_t& nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint8_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSGD));
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
uint8_t GMultiThreadedGD::getNThreads() const  {
	return nThreads_;
}

/************************************************************************************************************/
/**
 * Loads the data from another GMultiThreadedGD object.
 *
 * @param vp Pointer to another GMultiThreadedGD object, camouflaged as a GObject
 */
void GMultiThreadedGD::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	const GMultiThreadedGD *p_load = this->conversion_cast<GMultiThreadedGD>(cp);

	// First load our parent class'es data ...
	GGradientDescent::load_(cp);

	// ... and then our own
	if(nThreads_ != p_load->nThreads_) {
		nThreads_ = p_load->nThreads_;
		tp_.clear(); // TODO: is this needed ?
		tp_.size_controller().resize(nThreads_);
	}
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedGD::clone_() const  {
	return new GMultiThreadedGD(*this);
}

/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedGD::init() {
	// GGradientDescent sees exactly the environment it would when called from its own class
	GGradientDescent::init();

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
void GMultiThreadedGD::finalize() {
#ifdef DEBUG
	if(data.size() != sm_value_.size()) {
		raiseException(
				"In GMultiThreadedGD::finalize():" << std::endl
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

	// GGradientDescent sees exactly the environment it would when called from its own class
	GGradientDescent::finalize();
}

/************************************************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GGradientDescent, albeit multi-threaded.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
double GMultiThreadedGD::doFitnessCalculation(const std::size_t& finalPos) {
	double bestFitness = getWorstCase(); // Holds the best fitness found so far
	std::size_t nStartingPoints = this->getNStartingPoints();

#ifdef DEBUG
	if(finalPos > this->size()) {
		raiseException(
				"In GMultiThreadedGD::doFitnessCalculation(const std::size_t&):" << std::endl
				<< "Got invalid final position: " << finalPos << "/" << this->size()
		);
	}

	if(finalPos < nStartingPoints) {
		raiseException(
				"In GMultiThreadedGD::doFitnessCalculation(const std::size_t&):" << std::endl
				<< "We require finalPos to be at least " << nStartingPoints << ", but got " << finalPos
		);
	}
#endif

	// Trigger value calculation for all individuals (including parents)
	for(std::size_t i=0; i<finalPos; i++) {
#ifdef DEBUG
		// Make sure the evaluated individuals have the dirty flag set
		if(!this->at(i)->isDirty()) {
			raiseException(
					"In GMultiThreadedGD::doFitnessCalculation(const std::size_t&):" << std::endl
					<< "Found individual in position " << i << " whose dirty flag isn't set"
			);
		}
#endif /* DEBUG*/

		// Make sure we are allowed to perform value calculation
		this->at(i)->setServerMode(false);

		tp_.schedule(
			Gem::Common::GThreadWrapper(
				boost::bind(
					&GParameterSet::fitness
					, this->at(i)
					, 0
				)
			)
		);
	}

	// wait for the pool to run out of tasks
	tp_.wait();

	// Retrieve information about the best fitness found and disallow re-evaluation
	double fitnessFound = 0.;
	for(std::size_t i=0; i<this->size(); i++) {
		// Prevents re-evaluation
		this->at(i)->setServerMode(true);

		if(i<nStartingPoints) {
			fitnessFound = this->at(i)->fitness(0);

			if(isBetter(fitnessFound, bestFitness)) {
				bestFitness = fitnessFound;
			}
		}
	}

	return bestFitness;
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedGD::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GGradientDescent::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedGD::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GGradientDescent::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedGD::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GGradientDescent::specificTestsFailuresExpected_GUnitTests();
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
 * As Gem::Geneva::GMultiThreadedGD has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GMultiThreadedGD> TFactory_GUnitTests<Gem::Geneva::GMultiThreadedGD>() {
	boost::shared_ptr<Gem::Geneva::GMultiThreadedGD> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GMultiThreadedGD>(new Gem::Geneva::GMultiThreadedGD()));
	return p;
}

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */
