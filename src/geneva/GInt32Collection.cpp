/**
 * @file GInt32Collection.cpp
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
#include "geneva/GInt32Collection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32Collection)


namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GInt32Collection::GInt32Collection() { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GInt32Collection::GInt32Collection(
	const std::size_t &nval, const std::int32_t &min, const std::int32_t &max
)
	: GIntNumCollectionT<std::int32_t>(nval, min, max) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a predefined value in all positions
 *
 * @param nval The amount of random values
 * @param val A value to be assigned to all positions
 * @param min The minimum random value
 * @param max The maximum random value
 */
GInt32Collection::GInt32Collection(
	const std::size_t &nval, const std::int32_t &val, const std::int32_t &min, const std::int32_t &max
)
	: GIntNumCollectionT<std::int32_t>(nval, val, min, max) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32Collection object
 */
GInt32Collection::GInt32Collection(const GInt32Collection &cp)
	: GIntNumCollectionT<std::int32_t>(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GInt32Collection::~GInt32Collection() { /* nothing */ }

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32Collection::clone_() const {
	return new GInt32Collection(*this);
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GInt32Collection &GInt32Collection::operator=(const GInt32Collection &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GInt32Collection object
 *
 * @param  cp A constant reference to another GInt32Collection object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32Collection::operator==(const GInt32Collection &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GInt32Collection object
 *
 * @param  cp A constant reference to another GInt32Collection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32Collection::operator!=(const GInt32Collection &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
void GInt32Collection::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GInt32Collection reference independent of this object and convert the pointer
	const GInt32Collection *p_load = Gem::Common::g_convert_and_compare<GObject, GInt32Collection>(cp, this);

	GToken token("GInt32Collection", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GIntNumCollectionT<std::int32_t>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32Collection::name() const {
	return std::string("GInt32Collection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local values should be attached
 */
void GInt32Collection::int32Streamline(
	std::vector<std::int32_t> &parVec, const activityMode &am
) const {
	GInt32Collection::const_iterator cit;
	for (cit = this->begin(); cit != this->end(); ++cit) {
		parVec.push_back(*cit);
	}
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param parVec The map to which the local values should be attached
 */
void GInt32Collection::int32Streamline(
	std::map<std::string, std::vector<std::int32_t>> &parVec, const activityMode &am
) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GInt32Collection::int32Streamline(std::map<std::string, std::vector<std::int32_t>>& parVec) const: Error!" << std::endl
      << "No name was assigned to the object" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	std::vector<std::int32_t> parameters;
	this->int32Streamline(parameters, am);
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type std::int32_t to the vectors. Since this is an unbounded type,
 * we use the initialization boundaries as a replacement.
 *
 * @param lBndVec A vector of lower std::int32_t parameter boundaries
 * @param uBndVec A vector of upper std::int32_t parameter boundaries
 */
void GInt32Collection::int32Boundaries(
	std::vector<std::int32_t> &lBndVec, std::vector<std::int32_t> &uBndVec, const activityMode &am
) const {
	// Add as man lower and upper boundaries to the vector as
	// there are variables
	GInt32Collection::const_iterator cit;
	for (cit = this->begin(); cit != this->end(); ++cit) {
		lBndVec.push_back(this->getLowerInitBoundary());
		uBndVec.push_back(this->getUpperInitBoundary());
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of std::int32_t values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of std::int32_t parameters
 */
std::size_t GInt32Collection::countInt32Parameters(
	const activityMode &am
) const {
	return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GInt32Collection::assignInt32ValueVector(
	const std::vector<std::int32_t> &parVec, std::size_t &pos, const activityMode &am
) {
	for (GInt32Collection::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
      // Do we have a valid position ?
      if(pos >= parVec.size()) {
         glogger
         << "In GInt32Collection::assignInt32ValueVector(const std::vector<std::int32_t>&, std::size_t&):" << std::endl
         << "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
         << GEXCEPTION;
      }
#endif

		(*it) = parVec[pos];
		pos++;
	}
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GInt32Collection::assignInt32ValueVectors(
	const std::map<std::string, std::vector<std::int32_t>> &parMap, const activityMode &am
) {
	GInt32Collection::iterator it;
	std::size_t cnt = 0;
	for (it = this->begin(); it != this->end(); ++it) {
		*it = (Gem::Common::getMapItem(parMap, this->getParameterName())).at(cnt++);
	}
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Collection object, camouflaged as a GObject
 */
void GInt32Collection::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GInt32Collection * p_load = Gem::Common::g_convert_and_compare<GObject, GInt32Collection>(cp, this);

	// Load our parent class'es data ...
	GIntNumCollectionT<std::int32_t>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Collection::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GIntNumCollectionT<std::int32_t>::modify_GUnitTests()) result = true;

	this->push_back(5);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32Collection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Collection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<std::int32_t>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GIntNumCollectionT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	// no local data, nothing to test

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32Collection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32Collection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<std::int32_t>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GIntNumCollectionT<std::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// no local data, nothing to test

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32Collection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
