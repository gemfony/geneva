/**
 * @file GConstrainedDoubleCollection.cpp
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

#include "geneva/GConstrainedDoubleCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedDoubleCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Protected, as it is only needed for de-serialization purposes.
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection()
	: GConstrainedFPNumCollectionT<double>() { /* nothing */ }

/******************************************************************************/
/**
 * Initialize with the lower and upper boundaries for data members of this class and
 * a number of random values within this range. Note that all action will take place in the
 * range [lowerBoundary, upperBoundary[.
 *
 * @param size The desired size of the collection
 * @param lowerBoundary The lower boundary for data members
 * @param upperBoundary The upper boundary for data members
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection(
	const std::size_t &size, const double &lowerBoundary, const double &upperBoundary
)
	: GConstrainedFPNumCollectionT<double>(size, lowerBoundary, upperBoundary) { /* nothing */ }

/******************************************************************************/
/**
 * Initialize with the lower and upper boundaries for data members of this class and
 * a fixed value for all items in the vector. Note that all action will take place in the
 * range [lowerBoundary, upperBoundary[.
 *
 * @param size The desired size of the collection
 * @param val The value to be assigned to all positions
 * @param lowerBoundary The lower boundary for data members
 * @param upperBoundary The upper boundary for data members
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection(
	const std::size_t &size, const double &val, const double &lowerBoundary, const double &upperBoundary
)
	: GConstrainedFPNumCollectionT<double>(size, val, lowerBoundary, upperBoundary) { /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GConstrainedDoubleCollection object
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection(const GConstrainedDoubleCollection &cp)
	: GConstrainedFPNumCollectionT<double>(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GConstrainedDoubleCollection::~GConstrainedDoubleCollection() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GConstrainedDoubleCollection &GConstrainedDoubleCollection::operator=(
	const GConstrainedDoubleCollection &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GConstrainedDoubleCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedDoubleCollection::operator==(const GConstrainedDoubleCollection &cp) const {
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
 * Checks for inequality with another GConstrainedDoubleCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedDoubleCollection::operator!=(const GConstrainedDoubleCollection &cp) const {
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
void GConstrainedDoubleCollection::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GConstrainedDoubleCollection reference independent of this object and convert the pointer
	const GConstrainedDoubleCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedDoubleCollection>(cp, this);

	GToken token("GConstrainedDoubleCollection", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GConstrainedFPNumCollectionT<double>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedDoubleCollection::name() const {
	return std::string("GConstrainedDoubleCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedDoubleCollection::doubleStreamline(
	std::vector<double> &parVec, const activityMode &am
) const {
	GConstrainedDoubleCollection::const_iterator cit;
	for (cit = this->begin(); cit != this->end(); ++cit) {
		parVec.push_back(this->transfer(*cit));
	}
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param parVec The map to which the local value should be attached
 */
void GConstrainedDoubleCollection::doubleStreamline(
	std::map<std::string, std::vector<double>> &parVec, const activityMode &am
) const {
#ifdef DEBUG
	if((this->getParameterName()).empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GConstrainedDoubleCollection::doubleStreamline(std::map<std::string, std::vector<double>>& parVec) const: Error!" << std::endl
				<< "No name was assigned to the object" << std::endl
		);
	}
#endif /* DEBUG */

	std::vector<double> parameters;
	this->doubleStreamline(parameters, am);
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors.
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GConstrainedDoubleCollection::doubleBoundaries(
	std::vector<double> &lBndVec, std::vector<double> &uBndVec, const activityMode &am
) const {
	// Add a lower and upper boundary to the vectors
	// for each variable in the collection
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		lBndVec.push_back(this->getLowerBoundary());
		uBndVec.push_back(this->getUpperBoundary());
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of double values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of double parameters
 */
std::size_t GConstrainedDoubleCollection::countDoubleParameters(
	const activityMode &am
) const {
	return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation to the
 * vector, so that it lies inside of the allowed value range.
 *
 * @param parVec The vector from which the data should be taken
 * @param pos The position inside of the vector from which the data is extracted in each turn of the loop
 */
void GConstrainedDoubleCollection::assignDoubleValueVector(
	const std::vector<double> &parVec, std::size_t &pos, const activityMode &am
) {
	for (std::size_t i = 0; i < this->size(); i++) {
#ifdef DEBUG
		// Do we have a valid position ?
		if(pos >= parVec.size()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GConstrainedDoubleCollection::assignDoubleValueVector(const std::vector<double>&, std::size_t&):" << std::endl
					<< "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
			);
		}
#endif

		this->setValue(i, this->transfer(parVec[pos]));
		pos++;
	}
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GConstrainedDoubleCollection::assignDoubleValueVectors(
	const std::map<std::string, std::vector<double>> &parMap, const activityMode &am
) {
	for (std::size_t i = 0; i < this->size(); i++) {
		this->setValue(i, this->transfer((Gem::Common::getMapItem(parMap, this->getParameterName())).at(i)));
	}
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GConstrainedDoubleCollection::doubleMultiplyByRandom(
	const double &min
	, const double &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(min,max);
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(
			pos, transfer(this->value(pos) * uniform_real_distribution(gr))
		);
	}
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GConstrainedDoubleCollection::doubleMultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(
			pos, transfer(this->value(pos) * uniform_real_distribution(gr))
		);
	}
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedDoubleCollection::doubleMultiplyBy(
	const double &val, const activityMode &am
) {
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, transfer(val * this->value(pos)));
	}
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
/**
 * Initialization with a constant value
 */
void GConstrainedDoubleCollection::doubleFixedValueInit(
	const double &val, const activityMode &am
) {
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, transfer(val));
	}
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedDoubleCollection::doubleAdd(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GConstrainedDoubleCollection> p = GParameterBase::parameterbase_cast<GConstrainedDoubleCollection>(p_base);

	// Cross-check that the sizes match
	if(this->size() != p->size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GConstrainedDoubleCollection::doubleAdd():" << std::endl
				<< "Sizes of vectors don't match: " << this->size() << "/" << p->size() << std::endl
		);
	}

	for(std::size_t pos = 0; pos<this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, transfer(this->value(pos) + p->value(pos)));
	}
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedDoubleCollection::doubleSubtract(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GConstrainedDoubleCollection> p = GParameterBase::parameterbase_cast<GConstrainedDoubleCollection>(p_base);

	// Cross-check that the sizes match
	if(this->size() != p->size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GConstrainedDoubleCollection::doubleSubtract():" << std::endl
				<< "Sizes of vectors don't match: " << this->size() << "/" << p->size() << std::endl
		);
	}

	for(std::size_t pos = 0; pos<this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, transfer(this->value(pos) - p->value(pos)));
	}
}

/******************************************************************************/
/**
 * Loads the data of another GConstrainedDoubleCollection object,
 * camouflaged as a GObject. We have no local data, so
 * all we need to do is to the standard identity check,
 * preventing that an object is assigned to itself.
 *
 * @param cp A copy of another GConstrainedDoubleCollection object, camouflaged as a GObject
 */
void GConstrainedDoubleCollection::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GConstrainedDoubleCollection * p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedDoubleCollection>(cp, this);

	// Load our parent class'es data ...
	GConstrainedFPNumCollectionT<double>::load_(cp);

	// no local data ...
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GConstrainedDoubleCollection::clone_() const {
	return new GConstrainedDoubleCollection(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	bool result = false;

	// Call the parent classes' functions
	if (GConstrainedFPNumCollectionT<double>::modify_GUnitTests()) result = true;

	this->randomInit(activityMode::ALLPARAMETERS, gr);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedDoubleCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<double>::specificTestsNoFailureExpected_GUnitTests();

	// Some parameters
	const std::size_t DEFSIZE = 10;
	const double DEFVAL = 1.;
	const double DEFMIN = -10.;
	const double DEFMAX = 10.;

	//---------------------------------------------------------------------

	{ // Check that initialization with a fixed value-range yields the desired values
		std::shared_ptr <GConstrainedDoubleCollection> p_test;

		BOOST_CHECK_NO_THROW(p_test = std::shared_ptr<GConstrainedDoubleCollection>(
			new GConstrainedDoubleCollection(DEFSIZE, DEFMIN, DEFMAX)));
		BOOST_CHECK(p_test->size() == DEFSIZE && DEFSIZE > 1);
		for (std::size_t i = 1; i < DEFSIZE; i++) { // Check that consecutive values are different
			BOOST_CHECK(p_test->at(i) != p_test->at(i - 1));
		}
		BOOST_CHECK(p_test->getLowerBoundary() == DEFMIN);
		BOOST_CHECK(
			p_test->getUpperBoundary() == boost::math::float_prior<double>(DEFMAX)); // The upper boundary is an open one
	}

	//---------------------------------------------------------------------

	{ // Check that initialization with a fixed value and range yields the desired values
		std::shared_ptr <GConstrainedDoubleCollection> p_test;

		BOOST_CHECK_NO_THROW(p_test = std::shared_ptr<GConstrainedDoubleCollection>(
			new GConstrainedDoubleCollection(DEFSIZE, DEFVAL, DEFMIN, DEFMAX)));
		BOOST_CHECK(p_test->size() == DEFSIZE);
		for (std::size_t i = 0; i < DEFSIZE; i++) {
			BOOST_CHECK(p_test->at(i) == DEFVAL);
		}
		BOOST_CHECK(p_test->getLowerBoundary() == DEFMIN);
		BOOST_CHECK(
			p_test->getUpperBoundary() == boost::math::float_prior<double>(DEFMAX)); // The upper boundary is an open one
	}

	//---------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedDoubleCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<double>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBrokerEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

