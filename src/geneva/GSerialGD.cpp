/**
 * @file GSerialGD.cpp
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

#include "geneva/GSerialGD.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialGD)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor
 */
GSerialGD::GSerialGD()
	: GBaseGD()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Initialization with the number of starting points and the size of the finite step
 */
GSerialGD::GSerialGD (
		const std::size_t& nStartingPoints
		, const float& finiteStep
		, const float& stepSize
)
	: GBaseGD(nStartingPoints, finiteStep, stepSize)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard copy constructor
 */
GSerialGD::GSerialGD(const GSerialGD& cp)
	: GBaseGD(cp)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor.
 */
GSerialGD::~GSerialGD()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A standard assignment operator for GSerialGD objects.
 *
 * @param cp Reference to another GSerialGD object
 * @return A constant reference to this object
 */
const GSerialGD& GSerialGD::operator=(const GSerialGD& cp) {
	GSerialGD::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Checks for equality with another GSerialGD object
 *
 * @param  cp A constant reference to another GSerialGD object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialGD::operator==(const GSerialGD& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSerialGD::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GSerialGD object
 *
 * @param  cp A constant reference to another GSerialGD object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialGD::operator!=(const GSerialGD& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSerialGD::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GSerialGD::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GSerialGD *p_load = GObject::gobject_conversion<GSerialGD>(&cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GSerialGD>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseGD::checkRelationshipWith(cp, e, limit, "GSerialGD", y_name, withMessages));

	// ... no local data

	return evaluateDiscrepancies("GSerialGD", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Loads the data from another GSerialGD object.
 *
 * @param vp Pointer to another GSerialGD object, camouflaged as a GObject
 */
void GSerialGD::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	// const GSerialGD *p_load = this->gobject_conversion<GSerialGD>(cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GSerialGD>(cp);

	// First load our parent class'es data ...
	GBaseGD::load_(cp);

	// ... no local data
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialGD::clone_() const  {
	return new GSerialGD(*this);
}

/************************************************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialGD::init() {
	// GSerialGD sees exactly the environment it would when called from its own class
	GBaseGD::init();

	// Add local configuration code here
}

/************************************************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialGD::finalize() {
	// Add local finalization code here

	// GSerialGD sees exactly the environment it would when called from its own class
	GBaseGD::finalize();
}

/************************************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GSerialGD::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	// Call our parent class'es function
	GBaseGD::addConfigurationOptions(gpb, showOrigin);

	// no local data
}

/************************************************************************************************************/
/**
 * Performs final optimization work. In the case of (networked) gradient descents, the starting points need
 * to be re-evaluated at the end of the optimization cycle, before the connection to the broker is cut.
 * doFitnessCalculation is overloaded in GBrokerGD.
 */
// void GSerialGD::optimizationFinalize() {
	// Make sure the fitness of the parent individuals is calculated in the final iteration
//	checkpoint(ifProgress(doFitnessCalculation(nStartingPoints_)));
// }
// TODO: What for ?

/************************************************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function can be overloaded to perform
 * fitness calculation in parallel, using threads or network communication.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
double GSerialGD::doFitnessCalculation(const std::size_t& finalPos) {
	double bestFitness = getWorstCase(); // Holds the best fitness found so far

#ifdef DEBUG
	if(finalPos > this->size()) {
		raiseException(
				"In GSerialGD::doFitnessCalculation(const std::size_t&):" << std::endl
				<< "Got invalid final position: " << finalPos << "/" << this->size()
		);
	}

	if(finalPos < getNStartingPoints()) {
		raiseException(
				"In GSerialGD::doFitnessCalculation(const std::size_t&):" << std::endl
				<< "We require finalPos to be at least " << getNStartingPoints() << ", but got " << finalPos
		);
	}
#endif

	// Trigger value calculation for all individuals (including parents)
	double fitnessFound = 0.;
	for(std::size_t i=0; i<finalPos; i++) {
#ifdef DEBUG
		// Make sure the evaluated individuals have the dirty flag set
		if(afterFirstIteration() && !this->at(i)->isDirty()) {
			raiseException(
					"In GSerialGD::doFitnessCalculation(const std::size_t&):" << std::endl
					<< "In iteration " << this->getIteration() << ": Found individual in position " << i << " whose dirty flag isn't set"
			);
		}
#endif /* DEBUG */

		fitnessFound = this->at(i)->fitness(0);

		// Update the best fitness value found
		if(i<getNStartingPoints()) {
			if(isBetter(fitnessFound, bestFitness)) {
				bestFitness = fitnessFound;
			}
		}
	}

	return bestFitness;
}



#ifdef GEM_TESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GSerialGD::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GBaseGD::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialGD::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseGD::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialGD::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseGD::specificTestsFailuresExpected_GUnitTests();
}
#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */


/************************************************************************************************************/
#ifdef GEM_TESTING
/**
 * A factory function that emits a GSerialGD object
 */
template <> boost::shared_ptr<Gem::Geneva::GSerialGD> TFactory_GUnitTests<Gem::Geneva::GSerialGD>() {
	using namespace Gem::Tests;
	boost::shared_ptr<Gem::Geneva::GSerialGD> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GSerialGD>(new Gem::Geneva::GSerialGD()));
	p->push_back(boost::shared_ptr<GTestIndividual1>(new GTestIndividual1()));
	return p;
}
#endif /* GEM_TESTING */

