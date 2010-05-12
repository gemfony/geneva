/**
 * @file GMultiThreadedEA.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "GMultiThreadedEA.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GMultiThreadedEA)

namespace Gem {
namespace GenEvA {

/************************************************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GMultiThreadedEA::GMultiThreadedEA()
   : GEvolutionaryAlgorithm()
   , nThreads_(DEFAULTBOOSTTHREADS)
   , tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor. Note that we do not copy le_value_ as
 * it is used for internal caching only.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GMultiThreadedEA::GMultiThreadedEA(const GMultiThreadedEA& cp)
   : GEvolutionaryAlgorithm(cp)
   , nThreads_(cp.nThreads_)
   , tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedEA::~GMultiThreadedEA() {
	tp_.clear();
	tp_.wait();
}

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
	const GMultiThreadedEA *p_load = this->conversion_cast<GMultiThreadedEA>(cp);

	// First load our parent class'es data ...
	GEvolutionaryAlgorithm::load_(cp);

	// ... and then our own
	if(nThreads_ != p_load->nThreads_) {
		nThreads_ = p_load->nThreads_;
		tp_.clear(); // TODO: is this needed ?
		tp_.size_controller().resize(nThreads_);
	}

	// Note that we do not copy le_value_ as it is used for internal caching only
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
	using namespace Gem::Util;
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
	using namespace Gem::Util;
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
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GMultiThreadedEA *p_load = GObject::conversion_cast<GMultiThreadedEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GEvolutionaryAlgorithm::checkRelationshipWith(cp, e, limit, "GMultiThreadedEA", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GMultiThreadedEA", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

	return evaluateDiscrepancies("GMultiThreadedEA", caller, deviations, e);
}


/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedEA::init() {
	// GEvolutionaryAlgorithm sees exactly the environment it would when called from its own class
	GEvolutionaryAlgorithm::init();

	// We want to prevent lazy evaluation, as all value calculation
	// shall take place in the threads. By the same token, though, we want to be
	// able to restore the original values.
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		le_value_.push_back((*it)->setAllowLazyEvaluation(false));
	}
}

/************************************************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedEA::finalize() {
	// Restore the original values
	std::vector<bool>::iterator b_it;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(), b_it=le_value_.begin();
		it!=data.end(), b_it != le_value_.end();
		++it, ++b_it)
	{
		(*it)->setAllowLazyEvaluation(*b_it);
	}

	// GEvolutionaryAlgorithm sees exactly the environment it would when called from its own class
	GEvolutionaryAlgorithm::finalize();
}

/************************************************************************************************************/
/**
 * An overloaded version of GEvolutionaryAlgorithm::adaptChildren() . Adaption
 * and evaluation of children is handled by threads in a thread pool. The maximum
 * number of threads is DEFAULTBOOSTTHREADS (possibly 2) and can be overridden
 * with the GMultiThreadedEA::setMaxThreads() function.
 */
void GMultiThreadedEA::adaptChildren() {
	std::size_t nParents = GEvolutionaryAlgorithm::getNParents();
	boost::uint32_t generation = GEvolutionaryAlgorithm::getIteration();
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We start with the parents, if this is generation 0. Their
	// initial fitness needs to be determined, if this is the MUPLUSNU
	// or MUNU1PRETAIN selection model.
	if(generation==0 && (this->getSortingScheme()==MUPLUSNU || this->getSortingScheme()==MUNU1PRETAIN)) {
		for(it=data.begin(); it!=data.begin() + nParents; ++it) {
			// tp_.schedule(boost::bind(&GIndividual::checkedFitness, it->get()));
			tp_.schedule(boost::bind(&GIndividual::checkedFitness, *it));
		}
	}

	// Next we adapt the children
	for(it=data.begin() + nParents; it!=data.end(); ++it) {
		// tp_.schedule(boost::bind(&GIndividual::checkedAdaption, it->get()));
		tp_.schedule(boost::bind(&GIndividual::checkedAdaption, *it));
	}

	// ... and wait for the pool to become empty
	tp_.wait();
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
void GMultiThreadedEA::setNThreads(const boost::uint8_t& nThreads) {
	if(nThreads == 0) {
		boost::uint8_t hardwareThreads = boost::numeric_cast<boost::uint8_t>(boost::thread::hardware_concurrency());
		if(hardwareThreads > 0) {
			std::cout << "Determined " << (int)hardwareThreads << " as a suitable number of processing threads for this architecture." << std::endl;
			nThreads_ = hardwareThreads;
		}
		else {
			nThreads_ = DEFAULTBOOSTTHREADS;
		}
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
uint8_t GMultiThreadedEA::getNThreads() const  {
	return nThreads_;
}

/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedEA::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GEvolutionaryAlgorithm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedEA::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedEA::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
