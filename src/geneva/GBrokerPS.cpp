/**
 * @file GBrokerPS.cpp
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

#include "geneva/GBrokerPS.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerPS)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBrokerPS::GBrokerPS() : GBasePS()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 */
GBrokerPS::GBrokerPS(const GBrokerPS &cp)
	: GBasePS(cp)
   , m_gbroker_executor(cp.m_gbroker_executor)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor.
 */
GBrokerPS::~GBrokerPS() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBrokerPS &GBrokerPS::operator=(
	const GBrokerPS &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBrokerPS object
 *
 * @param  cp A constant reference to another GBrokerPS object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerPS::operator==(const GBrokerPS &cp) const {
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
 * Checks for inequality with another GBrokerPS object
 *
 * @param  cp A constant reference to another GBrokerPS object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerPS::operator!=(const GBrokerPS &cp) const {
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
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBrokerPS::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBrokerPS reference independent of this object and convert the pointer
	const GBrokerPS *p_load = Gem::Common::g_convert_and_compare<GObject, GBrokerPS >(cp, this);

	GToken token("GBrokerPS", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBasePS>(IDENTITY(*this, *p_load), token);

	// ... compare the local data
	compare_t(IDENTITY(m_gbroker_executor, p_load->m_gbroker_executor), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBrokerPS::name() const {
	return std::string("GBrokerPS");
}

/******************************************************************************/
/**
 * Checks whether this algorithm communicates via the broker. This is an overload from the corresponding
 * GOptimizableI function
 *
 * @return A boolean indicating whether this algorithm communicates via the broker
 */
bool GBrokerPS::usesBroker() const {
	return true;
}

/******************************************************************************/
/**
 * Loads the data from another GBrokerPS object.
 *
 * @param vp Pointer to another GBrokerPS object, camouflaged as a GObject
 */
void GBrokerPS::load_(const GObject *cp) {
	// Check that we are dealing with a GBrokerPS reference independent of this object and convert the pointer
	const GBrokerPS *p_load = Gem::Common::g_convert_and_compare<GObject, GBrokerPS >(cp, this);

	// Load the parent classes' data ...
	GBasePS::load_(cp);

	// ... and then our local data
	m_gbroker_executor = p_load->m_gbroker_executor;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GBrokerPS::clone_() const {
	return new GBrokerPS(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GBrokerPS::init() {
	// GBasePS sees exactly the environment it would when called from its own class
	GBasePS::init();

	// Initialize the broker connector
	m_gbroker_executor.init();
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GBrokerPS::finalize() {
	// Finalize the broker connector
	m_gbroker_executor.finalize();

	// GBasePS sees exactly the environment it would when called from its own class
	GBasePS::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GBrokerPS::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	std::string comment;

	// Call our parent class'es function
	GBasePS::addConfigurationOptions(gpb);

	// Add options from our local objects
	m_gbroker_executor.addConfigurationOptions(gpb);
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBasePS, albeit by delegating work to the broker. Items are evaluated up to a maximum position
 * in the vector. Note that we always start the evaluation with the first item in the vector.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
void GBrokerPS::runFitnessCalculation() {
	using namespace Gem::Courtier;
	bool complete = false;

#ifdef DEBUG
   GBrokerPS::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
      // Make sure the evaluated individuals have the dirty flag set
      if(!(*it)->isDirty()) {
         glogger
         << "In GBrokerPS::runFitnessCalculation():" << std::endl
         << "Found individual in position " << std::distance(this->begin(), it) << ", whose dirty flag isn't set" << std::endl
         << GEXCEPTION;
      }
   }
#endif /* DEBUG */

	//--------------------------------------------------------------------------------
	// Submit all work items and wait for their return

	std::vector<bool> workItemPos(data.size(), Gem::Courtier::GBC_UNPROCESSED);
	complete = m_gbroker_executor.workOn(
		data
		, workItemPos
		, m_oldWorkItems_vec
		, true // resubmit unprocessed items
		, "GBrokerPS::runFitnessCalculation()"
	);

	//--------------------------------------------------------------------------------
	// Some error checks

	// Check if all work items have returned
	if (!complete) {
		glogger
			<< "In GBrokerPS::runFitnessCalculation(): Error!" << std::endl
			<< "No complete set of items received" << std::endl
			<< GEXCEPTION;
	}

	// Check if work items exists whose processing function has thrown an exception.
	// This is a severe error, as we need evaluations for all work items in a gradient
	// descent.
	if(auto it = std::find_if(
		data.begin()
		, data.end()
		, [this](std::shared_ptr<GParameterSet> p) -> bool {
			return p->processing_was_unsuccessful();
		}
	) != data.end()) {
		glogger
			<< "In GBrokerPS::runFitnessCalculation(): Error!" << std::endl
			<< "At least one individual could not be processed" << std::endl
			<< "due to errors in the (possibly user-supplied) process() function." << std::endl
			<< "This is a severe error and we cannot continue" << std::endl
			<< GEXCEPTION;
	}

	//--------------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerPS::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (GBasePS::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerPS::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerPS::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBasePS::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerPS::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerPS::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GBasePS::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerPS::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
