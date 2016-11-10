/**
 * @file GBaseSA.cpp
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
#include "geneva/GBaseSA.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. All initialization work of member variable
 * is done in the class body.
 */
GBaseSA::GBaseSA() : GParameterSetParChild()
{
	// Make sure we start with a valid population size if the user does not supply these values
	this->setPopulationSizes(100, 1);
}

/******************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GBaseSA object
 */
GBaseSA::GBaseSA(const GBaseSA &cp)
	: GParameterSetParChild(cp)
	, t0_(cp.t0_)
	, t_(cp.t_)
	, alpha_(cp.alpha_)
{
	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GBaseSA::~GBaseSA()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBaseSA &GBaseSA::operator=(const GBaseSA &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBaseSA object
 *
 * @param  cp A constant reference to another GBaseSA object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseSA::operator==(const GBaseSA &cp) const {
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
 * Checks for inequality with another GBaseSA object
 *
 * @param  cp A constant reference to another GBaseSA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseSA::operator!=(const GBaseSA &cp) const {
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
 * Loads the data of another GBaseSA object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GBaseSA object, camouflaged as a GObject
 */
void GBaseSA::load_(const GObject *cp) {
	// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
	const GBaseSA *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSA>(cp, this);

	// First load the parent class'es data ...
	GParameterSetParChild::load_(cp);

	// ... and then our own data
	t0_ = p_load->t0_;
	t_ = p_load->t_;
	alpha_ = p_load->alpha_;
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
void GBaseSA::compare(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
	const GBaseSA *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSA>(cp, this);

	GToken token("GBaseSA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSetParChild>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(t0_, p_load->t0_), token);
	compare_t(IDENTITY(t_, p_load->t_), token);
	compare_t(IDENTITY(alpha_, p_load->alpha_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBaseSA::name() const {
	return std::string("GBaseSA");
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GBaseSA::getOptimizationAlgorithm() const {
	return "PERSONALITY_SA";
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GBaseSA::getAlgorithmName() const {
	return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::optimize(), before the
 * actual optimization cycle starts.
 */
void GBaseSA::init() {
	// To be performed before any other action
	GParameterSetParChild::init();
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBaseSA::finalize() {
	// Last action
	GParameterSetParChild::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr <GPersonalityTraits> GBaseSA::getPersonalityTraits() const {
	return std::shared_ptr<GSAPersonalityTraits>(new GSAPersonalityTraits());
}

/******************************************************************************/
/**
 * Performs a simulated annealing style sorting and selection
 */
void GBaseSA::sortSAMode() {
	// Position the nParents best children of the population right behind the parents
	std::partial_sort(
		data.begin() + nParents_, data.begin() + 2 * nParents_, data.end(),
		[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);

	// Check for each parent whether it should be replaced by the corresponding child
	for (std::size_t np = 0; np < nParents_; np++) {
		double pPass = saProb(this->at(np)->minOnly_fitness(), this->at(nParents_ + np)->minOnly_fitness());
		if (pPass >= 1.) {
			this->at(np)->GObject::load(this->at(nParents_ + np));
		} else {
			double challenge =
                    GOptimizationAlgorithmT<GParameterSet>::m_uniform_real_distribution(gr, std::uniform_real_distribution<double>::param_type(0.,1.));
			if (challenge < pPass) {
				this->at(np)->GObject::load(this->at(nParents_ + np));
			}
		}
	}

	// Sort the parents -- it is possible that a child with a worse fitness has replaced a parent
	std::sort(
		data.begin(), data.begin() + nParents_,
		[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);

	// Make sure the temperature gets updated
	updateTemperature();
}

/******************************************************************************/
/**
 * Calculates the simulated annealing probability for a child to replace a parent
 *
 * @param qParent The fitness of the parent
 * @param qChild The fitness of the child
 * @return A double value in the range [0,1[, representing the likelihood for the child to replace the parent
 */
double GBaseSA::saProb(const double &qParent, const double &qChild) {
	// We do not have to do anything if the child is better than the parent
	if (this->isBetter(qChild, qParent)) {
		return 2.;
	}

	double result = 0.;
	if (this->getMaxMode()) {
		result = exp(-(qParent - qChild) / t_);
	} else {
		result = exp(-(qChild - qParent) / t_);
	}

	return result;
}

/******************************************************************************/
/**
 * Updates the temperature. This function is used for simulated annealing.
 */
void GBaseSA::updateTemperature() {
	t_ *= alpha_;
}

/******************************************************************************/
/**
 * Determines the strength of the temperature degradation. This function is used for simulated annealing.
 *
 * @param alpha The degradation speed of the temperature
 */
void GBaseSA::setTDegradationStrength(double alpha) {
	if (alpha <= 0.) {
		glogger
		<< "In GBaseSA::setTDegradationStrength(const double&):" << std::endl
		<< "Got negative alpha: " << alpha << std::endl
		<< GEXCEPTION;
	}

	alpha_ = alpha;
}

/******************************************************************************/
/**
 * Retrieves the temperature degradation strength. This function is used for simulated annealing.
 *
 * @return The temperature degradation strength
 */
double GBaseSA::getTDegradationStrength() const {
	return alpha_;
}

/******************************************************************************/
/**
 * Sets the start temperature. This function is used for simulated annealing.
 *
 * @param t0 The start temperature
 */
void GBaseSA::setT0(double t0) {
	if (t0 <= 0.) {
		glogger
		<< "In GBaseSA::setT0(const double&):" << std::endl
		<< "Got negative start temperature: " << t0 << std::endl
		<< GEXCEPTION;
	}

	t0_ = t0;
}

/******************************************************************************/
/**
 * Retrieves the start temperature. This function is used for simulated annealing.
 *
 * @return The start temperature
 */
double GBaseSA::getT0() const {
	return t0_;
}

/******************************************************************************/
/**
 * Retrieves the current temperature. This function is used for simulated annealing.
 *
 * @return The current temperature
 */
double GBaseSA::getT() const {
	return t_;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GBaseSA::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetParChild::addConfigurationOptions(gpb);

	gpb.registerFileParameter<double>(
		"t0" // The name of the variable
		, SA_T0 // The default value
		, [this](double sat0) { this->setT0(sat0); }
	)
	<< "The start temperature used in simulated annealing";

	gpb.registerFileParameter<double>(
		"alpha" // The name of the variable
		, SA_ALPHA // The default value
		, [this](double ds) { this->setTDegradationStrength(ds); }
	)
	<< "The degradation strength used in the cooling" << std::endl
	<< "schedule in simulated annealing;";
}

/******************************************************************************/
/**
 * Some error checks related to population sizes
 */
void GBaseSA::populationSanityChecks() const {
	// First check that we have been given a suitable value for the number of parents.
	// Note that a number of checks (e.g. population size != 0) has already been done
	// in the parent class.
	if (nParents_ == 0) {
		glogger
		<< "In GBaseSA::populationSanityChecks(): Error!" << std::endl
		<< "Number of parents is set to 0"
		<< GEXCEPTION;
	}

	// We need at least as many children as parents
	std::size_t popSize = getPopulationSize();
	if (popSize <= nParents_) {
		glogger
		<< "In GBaseSA::populationSanityChecks() :" << std::endl
		<< "Requested size of population is too small :" << popSize << " " << nParents_ << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Choose new parents, based on the SA selection scheme.
 */
void GBaseSA::selectBest() {
	// Sort according to the "Simulated Annealing" scheme
	sortSAMode();

	// Let parents know they are parents
	markParents();
}

/******************************************************************************/
/**
 * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
 * iteration and sorting scheme, the start point will be different. The end-point is not meant
 * to be inclusive.
 *
 * @return The range inside which evaluation should take place
 */
std::tuple<std::size_t, std::size_t> GBaseSA::getEvaluationRange() const {
	// We evaluate all individuals in the first iteration This happens so pluggable
	// optimization monitors do not need to distinguish between algorithms
	return std::tuple<std::size_t, std::size_t>(
		inFirstIteration() ? 0 : getNParents(), data.size()
	);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBaseSA::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (GParameterSetParChild::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseSA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterSetParChild::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseSA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterSetParChild::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */
   condnotset("GBaseSA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
