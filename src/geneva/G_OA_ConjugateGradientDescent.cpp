/**
 * @file
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

#include "geneva/G_OA_ConjugateGradientDescent.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConjugateGradientDescent)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GConjugateGradientDescent::GConjugateGradientDescent()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp A copy of another ConjugateGradientDescent object
 */
GConjugateGradientDescent::GConjugateGradientDescent(const GConjugateGradientDescent &cp)
	: G_OptimizationAlgorithm_Base(cp)
{
	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * The destructor
 */
GConjugateGradientDescent::~GConjugateGradientDescent() { /* nothing */ }

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GConjugateGradientDescent::getAlgorithmPersonalityType() const {
	return "PERSONALITY_GD";
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GConjugateGradientDescent::getNProcessableItems() const {
	return this->size(); // Evaluation always needs to be done for the entire population
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GConjugateGradientDescent::getAlgorithmName() const {
	return std::string("Gradient Descent");
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
void GConjugateGradientDescent::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GConjugateGradientDescent reference independent of this object and convert the pointer
	const GConjugateGradientDescent *p_load = Gem::Common::g_convert_and_compare<GObject, GConjugateGradientDescent>(cp, this);

	GToken token("GConjugateGradientDescent", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(*this, *p_load, token);

	// ... and then the local data
	// ...

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConjugateGradientDescent::name_() const {
	return std::string("GConjugateGradientDescent");
}

/******************************************************************************/
/**
 * Loads the data of another population
 *
 * @param cp A pointer to another GConjugateGradientDescent object, camouflaged as a GObject
 */
void GConjugateGradientDescent::load_(const GObject *cp) {
	// Check that we are dealing with a GConjugateGradientDescent reference independent of this object and convert the pointer
	const GConjugateGradientDescent *p_load = Gem::Common::g_convert_and_compare<GObject, GConjugateGradientDescent>(cp, this);

	// First load the parent class'es data.
	// This will also take care of copying all individuals.
	G_OptimizationAlgorithm_Base::load_(cp);

	// ... and then our own data
	// ...
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GConjugateGradientDescent::clone_() const {
	return new GConjugateGradientDescent(*this);
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * @return The value of the best individual found in this iteration
 */
std::tuple<double, double> GConjugateGradientDescent::cycleLogic() {
	// return bestFitness;
	return std::tuple<double, double>{0., 0.};
}


/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GConjugateGradientDescent::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);

	// ...
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBaseGD, albeit by delegating work to the broker. Items are evaluated up to the maximum position
 * in the vector. Note that we always start the evaluation with the first item in the vector.
 */
void GConjugateGradientDescent::runFitnessCalculation() {
	// ...
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GConjugateGradientDescent::init() {
	// To be performed before any other action
	G_OptimizationAlgorithm_Base::init();

	// ...
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GConjugateGradientDescent::finalize() {
	// Last action
	G_OptimizationAlgorithm_Base::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr <GPersonalityTraits> GConjugateGradientDescent::getPersonalityTraits() const {
	return std::shared_ptr<GGradientDescent_PersonalityTraits>(new GGradientDescent_PersonalityTraits());
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 */
void GConjugateGradientDescent::adjustPopulation() {
	// ...
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GConjugateGradientDescent::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (G_OptimizationAlgorithm_Base::modify_GUnitTests()) result = true;

	return result;
#else /* GEM_TESTING */
   Gem::Common::condnotset("GConjugateGradientDescent::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConjugateGradientDescent::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


