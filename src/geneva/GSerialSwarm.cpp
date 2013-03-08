/**
 * @file GSerialSwarm.cpp
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
#include "geneva/GSerialSwarm.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialSwarm)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Intentionally empty, as it is only needed for de-serialization purposes
 */
GSerialSwarm::GSerialSwarm()
	: GBaseSwarm()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data, hence this function is empty.
 */
GSerialSwarm::GSerialSwarm(
	const std::size_t& nNeighborhoods
	, const std::size_t& nNeighborhoodMembers
)
   : GBaseSwarm(nNeighborhoods, nNeighborhoodMembers)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GMultiThreadedEA object
 */
GSerialSwarm::GSerialSwarm(const GSerialSwarm& cp)
   : GBaseSwarm(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor.
 */
GSerialSwarm::~GSerialSwarm()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator for GSerialSwarm objects.
 *
 * @param cp Reference to another GSerialSwarm object
 * @return A constant reference to this object
 */
const GSerialSwarm& GSerialSwarm::operator=(const GSerialSwarm& cp) {
	GSerialSwarm::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Loads the data from another GSerialSwarm object.
 *
 * @param vp Pointer to another GSerialSwarm object, camouflaged as a GObject
 */
void GSerialSwarm::load_(const GObject *cp) {
	// Convert GObject pointer to local format
	// const GSerialSwarm *p_load = this->gobject_conversion<GSerialSwarm>(cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GSerialSwarm>(cp);

	// First load our parent class'es data ...
	GBaseSwarm::load_(cp);

	// no local data
}

/******************************************************************************/
/**
 * Checks for equality with another GSerialSwarm object
 *
 * @param  cp A constant reference to another GSerialSwarm object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialSwarm::operator==(const GSerialSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSerialSwarm::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GSerialSwarm object
 *
 * @param  cp A constant reference to another GSerialSwarm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialSwarm::operator!=(const GSerialSwarm& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSerialSwarm::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GSerialSwarm::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GSerialSwarm *p_load = GObject::gobject_conversion<GSerialSwarm>(&cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GSerialSwarm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseSwarm::checkRelationshipWith(cp, e, limit, "GSerialSwarm", y_name, withMessages));

	// no local data

	return evaluateDiscrepancies("GSerialSwarm", caller, deviations, e);
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSerialSwarm::name() const {
   return std::string("GSerialSwarm");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialSwarm::clone_() const  {
	return new GSerialSwarm(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialSwarm::init() {
	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::init();

	// Add local initialization code here
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialSwarm::finalize() {
	// Add local finalization code here

	// GBaseSwarm sees exactly the environment it would when called from its own class
	GBaseSwarm::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GSerialSwarm::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	// Call our parent class'es function
	GBaseSwarm::addConfigurationOptions(gpb, showOrigin);

	// no local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GSerialSwarm::getIndividualCharacteristic() const {
	return std::string("GENEVA_SERIALOPTALG");
}

/******************************************************************************/
/**
 * Updates the fitness of all individuals
 */
void GSerialSwarm::updateFitness() {
	std::size_t offset = 0;
	GSerialSwarm::iterator start = this->begin();
	boost::uint32_t iteration = getIteration();

	// Then start the evaluation threads
	for(std::size_t neighborhood=0; neighborhood<nNeighborhoods_; neighborhood++) {
		for(std::size_t member=0; member<nNeighborhoodMembers_[neighborhood]; member++) {
			GSerialSwarm::iterator current = start + offset;

			// Update the fitness
			updateIndividualFitness(
				iteration
				, (*current)
			);

			offset++;
		}
	}
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSerialSwarm::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GBaseSwarm::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSwarm::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialSwarm::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSwarm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialSwarm::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBaseSwarm::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialSwarm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
 * As Gem::Geneva::GSerialSwarm has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GSerialSwarm> TFactory_GUnitTests<Gem::Geneva::GSerialSwarm>() {
	using namespace Gem::Tests;
	const std::size_t NNEIGHBORHOODS=2;
	const std::size_t NNEIGHBORHOODMEMBERS=3;
	boost::shared_ptr<Gem::Geneva::GSerialSwarm> p;
	BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GSerialSwarm>(new Gem::Geneva::GSerialSwarm(NNEIGHBORHOODS, NNEIGHBORHOODMEMBERS)));
	for(std::size_t i=0; i<NNEIGHBORHOODS*NNEIGHBORHOODMEMBERS; i++) {
		p->push_back(boost::shared_ptr<GTestIndividual1>(new GTestIndividual1()));
	}
	return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
#endif /* GEM_TESTING */
