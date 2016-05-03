/**
 * @file GBooleanCollection.cpp
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

#include "geneva/GBooleanCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The standard constructor. As we have no local data, all work is done
 * by the parent class.
 */
GBooleanCollection::GBooleanCollection()
	: GParameterCollectionT<bool>() { /* nothing */ }

// Tested in this class

/******************************************************************************/
/**
 * Initializes the class with a set of nval random bits.
 *
 * @param nval The size of the collection
 */
GBooleanCollection::GBooleanCollection(const std::size_t &nval)
	: GParameterCollectionT<bool>()
{
	using namespace Gem::Common;
	using namespace Gem::Hap;

	std::bernoulli_distribution uniform_bool; // defaults to 0.5

	for (std::size_t i = 0; i < nval; i++) {
		this->push_back(uniform_bool(GRANDOM_TLS));
	}
}

// Tested in this class

/******************************************************************************/
/**
 * Initializes the class with a set of nval variables of identical value
 *
 * @param nval The size of the collection
 * @param val  The value to be assigned to each position
 */
GBooleanCollection::GBooleanCollection(const std::size_t &nval, const bool &val)
	: GParameterCollectionT<bool>(nval, val)
{ /* nothing */ }

// Tested in this class

/******************************************************************************/
/**
 * Initializes the class with nval random bits, of which probability percent
 * have the value true
 *
 * @param nval The size of the collection
 * @param probability The probability for true values in the collection
 */
GBooleanCollection::GBooleanCollection(const std::size_t &nval, const double &probability)
	: GParameterCollectionT<bool>()
{
	using namespace Gem::Common;
	using namespace Gem::Hap;

	std::bernoulli_distribution weighted_bool(probability);
	for (std::size_t i = 0; i < nval; i++) {
		this->push_back(weighted_bool(GRANDOM_TLS));
	}
}

// Tested in this class

/******************************************************************************/
/**
 * No local data, hence we can rely on the parent class.
 *
 * @param cp A copy of another GBooleanCollection object
 */
GBooleanCollection::GBooleanCollection(const GBooleanCollection &cp)
	: GParameterCollectionT<bool>(cp) { /* nothing */ }

// Tested in this class

/******************************************************************************/
/**
 * The standard destructor. No local data, hence it is empty.
 */
GBooleanCollection::~GBooleanCollection() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBooleanCollection &GBooleanCollection::operator=(
	const GBooleanCollection &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * FLips the value at a given position
 */
void GBooleanCollection::flip(const std::size_t& pos) {
#ifdef DEBUG
	if(this->size() <= pos) {
		glogger
		<< "In GBooleanCollection::flip(const std::size_t& " << pos << "): Error!" << std::endl
		<< "Tried to exist position beyond end of vector of size " << this->size() << std::endl
		<< GEXCEPTION;
	}
#endif

	if(true == this->at(pos)) {
		this->at(pos) = false;
	} else {
		this->at(pos) = true;
	}
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *GBooleanCollection::clone_() const {
	return new GBooleanCollection(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GBooleanCollection object, camouflaged as
 * a GObject.
 *
 * @param gb A pointer to another GBooleanCollection object, camouflaged as a GObject
 */
void GBooleanCollection::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GBooleanCollection * p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanCollection >(cp, this);

	GParameterCollectionT<bool>::load_(cp);
}

/******************************************************************************/
/**
 * Triggers random initialization of the parameter collection. Note that this
 * function assumes that the collection has been completely set up. Data
 * that is added later will remain unaffected.
 */
bool GBooleanCollection::randomInit_(const activityMode &) {
	bool randomized = false;

	using namespace Gem::Common;
	using namespace Gem::Hap;

	std::bernoulli_distribution uniform_bool; // defaults to 0.5

	for (std::size_t i = 0; i < this->size(); i++) {
		(*this)[i] = uniform_bool(GRANDOM_TLS);
		randomized = true;
	}

	return randomized;
}

/******************************************************************************/
/**
 * Random initialization with a given probability structure
 *
 * @param probability The probability for true values in the collection
 */
bool GBooleanCollection::randomInit_(const double &probability, const activityMode &) {
	using namespace Gem::Common;
	using namespace Gem::Hap;

	// Do some error checking
	if (probability < 0. || probability > 1.) {
		glogger
		<< "In GBooleanCollection::randomInit_(" << probability << "):" << std::endl
		<< "Requested probability outside of allowed range [0:1]" << std::endl
		<< GEXCEPTION;
	}

	std::bernoulli_distribution weighted_bool(probability);
	for (std::size_t i = 0; i < this->size(); i++) {
		(*this)[i] = weighted_bool(GRANDOM_TLS);
	}

	return true;
}

/******************************************************************************/
/**
 * Random initialization. This is a helper function, without it we'd
 * have to say things like "myGBooleanCollectionObject.GParameterBase::randomInit();".
 */
bool GBooleanCollection::randomInit(const activityMode &am) {
	return GParameterBase::randomInit(am); // This will also take into account the "blocked initialization" flag
}

/******************************************************************************/
/**
 * Random initialization with a given probability structure,
 * if re-initialization has not been blocked.
 *
 * @param probability The probability for true values in the collection
 */
bool GBooleanCollection::randomInit(const double &probability, const activityMode &am) {
	if (
		!GParameterBase::randomInitializationBlocked()
		&& this->modifiableAmMatchOrHandover(am)
		) {
		return randomInit_(probability, am);
	} else {
		return false;
	}
}

/***************************************************************************/
/**
 * Returns a "comparative range". In the case of boolean values this must
 * be considered to be more of a "dummy".
 */
bool GBooleanCollection::range() const {
	return true;
}

/******************************************************************************/
/**
 * Checks for equality with another GBooleanCollection object
 *
 * @param  cp A constant reference to another GBooleanCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanCollection::operator==(const GBooleanCollection &cp) const {
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
 * Checks for inequality with another GBooleanCollection object
 *
 * @param  cp A constant reference to another GBooleanCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBooleanCollection::operator!=(const GBooleanCollection &cp) const {
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
void GBooleanCollection::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBooleanCollection reference independent of this object and convert the pointer
	const GBooleanCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanCollection >(cp, this);

	GToken token("GBooleanCollection", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterCollectionT<bool>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanCollection::name() const {
	return std::string("GBooleanCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local values should be attached
 */
void GBooleanCollection::booleanStreamline(
	std::vector<bool> &parVec, const activityMode &
) const {
	GBooleanCollection::const_iterator cit;
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
void GBooleanCollection::booleanStreamline(
	std::map<std::string, std::vector<bool>> &parVec, const activityMode &am
) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GBooleanCollection::booleanStreamline(std::map<std::string, std::vector<bool>>& parVec) const: Error!" << std::endl
      << "No name was assigned to the object" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	std::vector<bool> parameters;
	this->booleanStreamline(parameters, am);
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
void GBooleanCollection::booleanBoundaries(
	std::vector<bool> &lBndVec, std::vector<bool> &uBndVec, const activityMode &am
) const {
	GBooleanCollection::const_iterator cit;
	for (cit = this->begin(); cit != this->end(); ++cit) {
		lBndVec.push_back(false);
		uBndVec.push_back(true);
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of bool values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of bool parameters
 */
std::size_t GBooleanCollection::countBoolParameters(
	const activityMode &am
) const {
	return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GBooleanCollection::assignBooleanValueVector(
	const std::vector<bool> &parVec, std::size_t &pos, const activityMode &am
) {
	for (GBooleanCollection::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
      // Do we have a valid position ?
      if(pos >= parVec.size()) {
         glogger
         << "In GBooleanCollection::assignBooleanValueVector(const std::vector<bool>&, std::size_t&):" << std::endl
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
void GBooleanCollection::assignBooleanValueVectors(
	const std::map<std::string, std::vector<bool>> &parMap, const activityMode &am
) {
	GBooleanCollection::iterator it;
	std::size_t cnt = 0;
	for (it = this->begin(); it != this->end(); ++it) {
		*it = (Gem::Common::getMapItem<std::vector<bool>>(parMap, this->getParameterName())).at(cnt++);
	}
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GParameterCollectionT<bool>::modify_GUnitTests()) result = true;

	this->push_back(true);
	return true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// A few settings
	const std::size_t nItems = 10000;
	const bool FIXEDVALUEINIT = true;
	const double LOWERBND = 0.8, UPPERBND = 1.2;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<bool>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
	gba_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gba_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gba_ptr);

	// Call the parent class'es function
	GParameterCollectionT<bool>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Check default constructor
		GBooleanCollection gbc;
		BOOST_CHECK(gbc.empty());
	}

	// --------------------------------------------------------------------------

	{ // Check copy construction
		GBooleanCollection gbc1;
		BOOST_CHECK_NO_THROW(gbc1.push_back(true));
		GBooleanCollection gbc2(gbc1);
		BOOST_CHECK_MESSAGE(
			gbc2.size() == 1 && gbc2.at(0) == true, "\n"
																 << "gbc2.size() = " << gbc2.size()
																 << "gbc2.at(0) = " << gbc2.at(0)
		);
	}

	// --------------------------------------------------------------------------

	{ // Check construction with a number of random bits
		GBooleanCollection gbc(nItems);

		BOOST_CHECK_MESSAGE(
			gbc.size() == nItems, "\n"
										 << "gbc.size() = " << gbc.size()
										 << "nItems = " << nItems
		);

		// Count the number of true and false values
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nItems; i++) {
			gbc.at(i) ? nTrue++ : nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue) / double(nFalse);
		BOOST_CHECK_MESSAGE(
			ratio > LOWERBND && ratio < UPPERBND, "\n"
															  << "ratio = " << ratio << "\n"
															  << "nTrue = " << nTrue << "\n"
															  << "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Check construction with a number of identical bits
		GBooleanCollection gbc(nItems, true);

		BOOST_CHECK_MESSAGE(
			gbc.size() == nItems, "\n"
										 << "gbc.size() = " << gbc.size()
										 << "nItems = " << nItems
		);

		// Count the number of true and false values
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nItems; i++) {
			gbc.at(i) ? nTrue++ : nFalse++;
		}

		BOOST_CHECK_MESSAGE(
			nTrue == nItems, "\n"
								  << "nTrue = " << nTrue << "\n"
								  << "nItems = " << nItems << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Check construction with a given probability for the value true
		GBooleanCollection gbc(nItems, 0.5);

		BOOST_CHECK_MESSAGE(
			gbc.size() == nItems, "\n"
										 << "gbc.size() = " << gbc.size()
										 << "nItems = " << nItems
		);

		// Count the number of true and false values
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nItems; i++) {
			gbc.at(i) ? nTrue++ : nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue) / double(nFalse);
		BOOST_CHECK_MESSAGE(
			ratio > LOWERBND && ratio < UPPERBND, "\n"
															  << "ratio = " << ratio << "\n"
															  << "nTrue = " << nTrue << "\n"
															  << "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that random initialization with equal probability will result in roughly the same amount of true and false values
		std::shared_ptr <GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		// Make sure the collection is empty
		BOOST_CHECK_NO_THROW(p_test->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->push_back(true);
		}

		// Check the size
		BOOST_CHECK(p_test->size() == nItems);

		// Randomly initialize, using the internal function
		BOOST_CHECK_NO_THROW(p_test->randomInit_(activityMode::ALLPARAMETERS));

		// Count the number of true and false values
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->at(i) ? nTrue++ : nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue) / double(nFalse);
		BOOST_CHECK_MESSAGE(
			ratio > LOWERBND && ratio < UPPERBND, "\n"
															  << "ratio = " << ratio << "\n"
															  << "nTrue = " << nTrue << "\n"
															  << "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Check that initialization with a probabilty of 0. for true results in just false values
		std::shared_ptr <GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		// Make sure the collection is empty
		BOOST_CHECK_NO_THROW(p_test->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->push_back(true);
		}

		// Randomly initialize, using the internal function
		BOOST_CHECK_NO_THROW(p_test->randomInit_(0., activityMode::ALLPARAMETERS));

		// Count the number of true and false values
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->at(i) ? nTrue++ : nFalse++;
		}

		// Cross-check
		BOOST_CHECK_MESSAGE(
			nTrue == 0, "\n"
							<< "nTrue = " << nTrue << "\n"
							<< "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Check that initialization with a probabilty of 1. for true results in just true values
		std::shared_ptr <GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		// Make sure the collection is empty
		BOOST_CHECK_NO_THROW(p_test->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->push_back(false);
		}

		// Randomly initialize, using the internal function
		BOOST_CHECK_NO_THROW(p_test->randomInit_(1., activityMode::ALLPARAMETERS));

		// Count the number of true and false values
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->at(i) ? nTrue++ : nFalse++;
		}

		// Cross-check
		BOOST_CHECK_MESSAGE(
			nTrue == nItems, "\n"
								  << "nTrue = " << nTrue << "\n"
								  << "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that random initialization with a given probability will result in roughly the expected amount of true and false values
		for (double d = 0.1; d < 0.9; d += 0.1) {
			std::shared_ptr <GBooleanCollection> p_test = this->clone<GBooleanCollection>();

			// Make sure the collection is empty
			BOOST_CHECK_NO_THROW(p_test->clear());

			// Add items of fixed value
			for (std::size_t i = 0; i < nItems; i++) {
				p_test->push_back(false);
			}

			// Randomly initialize, using the internal function and the required probability
			BOOST_CHECK_NO_THROW(p_test->randomInit_(d, activityMode::ALLPARAMETERS));

			// Count the number of true and false values
			std::size_t nTrue = 0;
			std::size_t nFalse = 0;
			for (std::size_t i = 0; i < nItems; i++) {
				p_test->at(i) ? nTrue++ : nFalse++;
			}

			// We allow a slight deviation, as the initialization is a random process
			double expectedTrueMin = 0.8 * d * nItems;
			double expectedTrueMax = 1.2 * d * nItems;

			BOOST_CHECK_MESSAGE(
				double(nTrue) > expectedTrueMin && double(nTrue) < expectedTrueMax, "\n"
																										  << "d = " << d << "\n"
																										  << "Allowed window = " <<
																										  expectedTrueMin << " - " <<
																										  expectedTrueMax << "\n"
																										  << "nItems = " << nItems << "\n"
																										  << "nTrue = " << nTrue << "\n"
																										  << "nFalse = " << nFalse << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization can be blocked for equal distributions
		std::shared_ptr <GBooleanCollection> p_test1 = this->clone<GBooleanCollection>();
		std::shared_ptr <GBooleanCollection> p_test2 = this->clone<GBooleanCollection>();

		// Make sure the collections are empty
		BOOST_CHECK_NO_THROW(p_test1->clear());
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test1->push_back(false);
		}

		// Block random initialization and cross check
		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == true);

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both objects are equal
		BOOST_CHECK(*p_test1 == *p_test2);

		// Check that random initialization is also blocked for p_test2
		BOOST_CHECK(p_test2->randomInitializationBlocked() == true);

		// Try to randomly initialize, using the *external* function
		BOOST_CHECK_NO_THROW(p_test1->randomInit(activityMode::ALLPARAMETERS));

		// Check that both objects are still the same
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization can be blocked for distributions with a given probability structure
		std::shared_ptr <GBooleanCollection> p_test1 = this->clone<GBooleanCollection>();
		std::shared_ptr <GBooleanCollection> p_test2 = this->clone<GBooleanCollection>();

		// Make sure the collections are empty
		BOOST_CHECK_NO_THROW(p_test1->clear());
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test1->push_back(false);
		}

		// Block random initialization and cross check
		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == true);

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both objects are equal
		BOOST_CHECK(*p_test1 == *p_test2);

		// Check that random initialization is also blocked for p_test2
		BOOST_CHECK(p_test2->randomInitializationBlocked() == true);

		// Try to randomly initialize, using the *external* function
		BOOST_CHECK_NO_THROW(p_test1->randomInit(0.7, activityMode::ALLPARAMETERS));

		// Check that both objects are still the same
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

	{ // Check that the fp-family of functions doesn't have an effect on this object
		std::shared_ptr <GBooleanCollection> p_test1 = this->GObject::clone<GBooleanCollection>();
		std::shared_ptr <GBooleanCollection> p_test2 = this->GObject::clone<GBooleanCollection>();
		std::shared_ptr <GBooleanCollection> p_test3 = this->GObject::clone<GBooleanCollection>();

		// Add a few items to p_test1
		for (std::size_t i = 0; i < nItems; i++) {
			p_test1->push_back(FIXEDVALUEINIT);
		}

		// Load into p_test2 and p_test3 and test equality
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
		BOOST_CHECK(*p_test2 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that initialization with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fixedValueInit<double>(2., activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that multiplication with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyBy<double>(2., activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(1., 2., activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that adding p_test1 to p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->add<double>(p_test1, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that subtracting p_test1 from p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->subtract<double>(p_test1, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test3 == *p_test2);
	}

	// --------------------------------------------------------------------------

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// A few settings
	std::size_t nItems = 10000;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<bool>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
	gba_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gba_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gba_ptr);

	// Call the parent class'es function
	GParameterCollectionT<bool>::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Check that random initialization with a probability < 0. throws
		std::shared_ptr <GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		// Make sure the collection is empty
		BOOST_CHECK_NO_THROW(p_test->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->push_back(true);
		}

		// Randomly initialize, using the internal function
		BOOST_CHECK_THROW(p_test->randomInit_(-1., activityMode::ALLPARAMETERS), Gem::Common::gemfony_error_condition);
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization with a probability > 1. throws
		std::shared_ptr <GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		// Make sure the collection is empty
		BOOST_CHECK_NO_THROW(p_test->clear());

		// Add items of fixed value
		for (std::size_t i = 0; i < nItems; i++) {
			p_test->push_back(true);
		}

		// Randomly initialize, using the internal function
		BOOST_CHECK_THROW(p_test->randomInit_(2., activityMode::ALLPARAMETERS), Gem::Common::gemfony_error_condition);
	}

	// --------------------------------------------------------------------------

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
