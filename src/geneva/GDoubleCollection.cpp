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

#include "geneva/GDoubleCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GDoubleCollection::GDoubleCollection() { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GDoubleCollection::GDoubleCollection(
	const std::size_t &nval
	, const double &min
	, const double &max
)
	: GFPNumCollectionT<double>(nval, min, max)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of predefined values in all positions
 *
 * @param nval The amount of random values
 * @param val The predefined value to be assigned to all positions
 * @param min The minimum random value
 * @param max The maximum random value
 */
GDoubleCollection::GDoubleCollection(
	const std::size_t &nval
	, const double &val
	, const double &min
	, const double &max
)
	: GFPNumCollectionT<double>(nval, val, min, max)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleCollection object
 */
GDoubleCollection::GDoubleCollection(const GDoubleCollection &cp)
	: GFPNumCollectionT<double>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GDoubleCollection::~GDoubleCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GDoubleCollection::clone_() const {
	return new GDoubleCollection(*this);
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
void GDoubleCollection::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDoubleCollection reference independent of this object and convert the pointer
	const GDoubleCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleCollection>(cp, this);

	GToken token("GDoubleCollection", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GFPNumCollectionT<double>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GDoubleCollection::name() const {
	return std::string("GDoubleCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GDoubleCollection::doubleStreamline(
	std::vector<double> &parVec, const activityMode &am
) const {
	GDoubleCollection::const_iterator cit;
	for (cit = this->begin(); cit != this->end(); ++cit) {
		parVec.push_back(*cit);
	}
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param parVec The map to which the local value should be attached
 */
void GDoubleCollection::doubleStreamline(
	std::map<std::string, std::vector<double>> &parVec, const activityMode &am
) const {
#ifdef DEBUG
	if((this->getParameterName()).empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GDoubleCollection::doubleStreamline(std::map<std::string, std::vector<double>>& parVec) const: Error!" << std::endl
				<< "No name was assigned to the object" << std::endl
		);
	}
#endif /* DEBUG */

	std::vector<double> parameters;
	this->streamline(parameters, am);
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GDoubleCollection::doubleBoundaries(
	std::vector<double> &lBndVec, std::vector<double> &uBndVec, const activityMode &am
) const {
	// Add as man lower and upper boundaries to the vector as
	// there are variables
	GDoubleCollection::const_iterator cit;
	for (cit = this->begin(); cit != this->end(); ++cit) {
		lBndVec.push_back(this->getLowerInitBoundary());
		uBndVec.push_back(this->getUpperInitBoundary());
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of double values
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of double parameters
 */
std::size_t GDoubleCollection::countDoubleParameters(
	const activityMode &am
) const {
	return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 *
 * @param parVec The vector from which the data should be taken
 * @param pos The position inside of the vector from which the data is extracted in each turn of the loop
 */
void GDoubleCollection::assignDoubleValueVector(
	const std::vector<double> &parVec, std::size_t &pos, const activityMode &am
) {
	for (GDoubleCollection::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
		// Do we have a valid position ?
		if(pos >= parVec.size()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GDoubleCollection::assignDoubleValueVector(const std::vector<double>&, std::size_t&):" << std::endl
					<< "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
			);
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
void GDoubleCollection::assignDoubleValueVectors(
	const std::map<std::string, std::vector<double>> &parMap, const activityMode &am
) {
	GDoubleCollection::iterator it;
	std::size_t cnt = 0;
	for (it = this->begin(); it != this->end(); ++it) {
		*it = (Gem::Common::getMapItem(parMap, this->getParameterName())).at(cnt++);
	}
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GDoubleCollection::doubleMultiplyByRandom(
	const double &min
	, const double &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(min,max);
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(
			pos
			, this->value(pos) * uniform_real_distribution(gr)
		);
	}
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GDoubleCollection::doubleMultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(
			pos
			, this->value(pos) * uniform_real_distribution(gr)
		);
	}
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GDoubleCollection::doubleMultiplyBy(
	const double &val
	, const activityMode &am
) {
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, val * this->value(pos));
	}
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GDoubleCollection::doubleFixedValueInit(
	const double &val, const activityMode &am
) {
	for (std::size_t pos = 0; pos < this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, val);
	}
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GDoubleCollection::doubleAdd(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr <GDoubleCollection> p = GParameterBase::parameterbase_cast<GDoubleCollection>(p_base);

	// Cross-check that the sizes match
	if(this->size() != p->size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GDoubleCollection::doubleAdd():" << std::endl
				<< "Sizes of vectors don't match: " << this->size() << "/" << p->size() << std::endl );
	}

	for(std::size_t pos = 0; pos<this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, this->value(pos) + p->value(pos));
	}
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GDoubleCollection::doubleSubtract(
	std::shared_ptr< GParameterBase > p_base, const
activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr <GDoubleCollection> p = GParameterBase::parameterbase_cast<GDoubleCollection>(p_base);

// Cross-check that the sizes match
	if(this->size() != p->size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GDoubleCollection::doubleSubtract():" << std::endl
				<< "Sizes of vectors don't match: " << this->size() << "/" << p->size() << std::endl
		);
	}

	for(std::size_t pos = 0; pos<this->size(); pos++) {
		GParameterCollectionT<double>::setValue(pos, this->value(pos) - p->value(pos));
	}
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleCollection object, camouflaged as a GObject
 */
void GDoubleCollection::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GDoubleCollection * p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleCollection>(cp, this);

	// Load our parent class'es data ...
	GFPNumCollectionT<double>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GFPNumCollectionT<double>::modify_GUnitTests()) result = true;

	this->push_back(5.);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GDoubleCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with some random data
 */
void GDoubleCollection::fillWithData(const std::size_t &nItems) {
#ifdef GEM_TESTING
	// Get a random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	// Make sure the collection is empty
	BOOST_CHECK_NO_THROW(this->clear());

	// Cross check that it really is
	BOOST_CHECK(this->size() == 0);
	// Use another method
	BOOST_CHECK(this->empty());

	// Add a single item of defined value, so we can test the find() and count() functions
	BOOST_CHECK_NO_THROW(this->push_back(0.));

	std::uniform_real_distribution<double> uniform_real_distribution(-10.,10.);
	for (std::size_t i = 1; i < nItems - 1; i++) {
		BOOST_CHECK_NO_THROW(this->push_back(uniform_real_distribution(gr)));
	}

	// Add a single item of defined value, so we can test the find() and count() functions
	BOOST_CHECK_NO_THROW(this->push_back(1.));

	// Cross-check the size
	BOOST_CHECK(this->size() == nItems);
	BOOST_CHECK(not this->empty());

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GDoubleCollection::fillWithData", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// A few settings
	const std::size_t nItems = 10000;
	const std::size_t nTests = 10;
	const double FIXEDVALUEINIT = 1.;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<double>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GFPNumCollectionT<double>::specificTestsNoFailureExpected_GUnitTests();

	// Get a random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	//------------------------------------------------------------------------------

	{ // Test the GParameterT<T>::adaptImpl() implementation
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();
		std::shared_ptr <GDoubleCollection> p_test2 = this->clone<GDoubleCollection>();

		if (p_test1->hasAdaptor()) {
			// Make sure the collection is clean
			p_test1->clear();

			// Add a few items
			for (std::size_t i = 0; i < nItems; i++) {
				p_test1->push_back(FIXEDVALUEINIT);
			}

			for (std::size_t t = 0; t < nTests; t++) {
				// Load p_test1 into p_test2
				BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

				// Make sure the objects match
				BOOST_CHECK(*p_test1 == *p_test2);

				// Adapt p_test1, using the internal function
				BOOST_CHECK_NO_THROW(p_test1->adaptImpl(gr));

				// Test whether the two objects differ now
				BOOST_CHECK(*p_test1 != *p_test2);

				// Check that each element differs
				for (std::size_t i = 0; i < nItems; i++) {
					BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
				}
			}
		}
	}

	//------------------------------------------------------------------------------

	{ // Test of GParameterCollectionT<T>::swap(const GParameterCollectionT<T>&)
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();
		std::shared_ptr <GDoubleCollection> p_test2 = this->clone<GDoubleCollection>();
		std::shared_ptr <GDoubleCollection> p_test3 = this->clone<GDoubleCollection>();

		if (p_test1->hasAdaptor()) {
			// Make sure the collection is clean
			p_test1->clear();

			// Add a few items
			for (std::size_t i = 0; i < nItems; i++) {
				p_test1->push_back(FIXEDVALUEINIT);
			}

			// Load p_test1 into p_test2 and p_test3
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			BOOST_CHECK_NO_THROW(p_test3->load(p_test1));

			// Make sure the objects match
			BOOST_CHECK(*p_test1 == *p_test2);
			BOOST_CHECK(*p_test1 == *p_test3);
			BOOST_CHECK(*p_test3 == *p_test2);

			// Adapt p_test1, using the internal function
			BOOST_CHECK_NO_THROW(p_test1->adaptImpl(gr));

			// Test whether p_test1 and p_test2/3 differ now
			BOOST_CHECK(*p_test1 != *p_test2);
			BOOST_CHECK(*p_test1 != *p_test3);
			// Test whether p_test2 is still the same as p_test3
			BOOST_CHECK(*p_test3 == *p_test2);

			// Swap the data of p_test2 and p_test1
			BOOST_CHECK_NO_THROW(p_test2->swap(*p_test1));

			// Extract the data vectors from p_test1 and p_test3
			std::vector<double> data1, data3;
			BOOST_CHECK_NO_THROW(p_test1->Gem::Common::GStdSimpleVectorInterfaceT<double>::getDataCopy(data1));
			BOOST_CHECK_NO_THROW(p_test3->Gem::Common::GStdSimpleVectorInterfaceT<double>::getDataCopy(data3));

			// Now p_test1->data  and p_test3->data should be the same, while p_test2 differs from both
			BOOST_CHECK(data1 == data3);
			BOOST_CHECK(*p_test2 != *p_test1);
			BOOST_CHECK(*p_test2 != *p_test3);
		}
	}

	//------------------------------------------------------------------------------

	{ // Test the GStdSimpleVectorInterfaceT<double>::reserve(), capacity() and max_size() functions
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

		// Make sure the collection is empty
		BOOST_CHECK_NO_THROW(p_test1->clear());

		// Check the site
		BOOST_CHECK(p_test1->size() == 0);
		BOOST_CHECK(p_test1->empty());

		// Check that the maximum size is > 0
		BOOST_CHECK(p_test1->max_size() > 0);

		// Reserve some space
		BOOST_CHECK_NO_THROW(p_test1->reserve(nItems));

		// Check that the capacity is > 0
		BOOST_CHECK(p_test1->capacity() > 0);

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Check the size again
		BOOST_CHECK(p_test1->size() == nItems);
		BOOST_CHECK(not p_test1->empty());
	}

	//------------------------------------------------------------------------------

	{ // Test the GStdSimpleVectorInterfaceT<double>::count(), find() and begin() functions
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Count the number of values == 0. . Should be >= 1
		BOOST_CHECK(p_test1->count(0.) >= 1);
		// Count the number of values == 1. . Should be >= 1
		BOOST_CHECK(p_test1->count(1.) >= 1);

		// Find the item with value 0. -- the first one is in position 0
		GDoubleCollection::const_iterator find_it, pos_it;
		BOOST_CHECK_NO_THROW(pos_it = p_test1->begin());
		BOOST_CHECK_NO_THROW(find_it = p_test1->find(0.));
		BOOST_CHECK(find_it == pos_it);
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of items with the operator[] and at() functions of GStdSimpleVectorInterfaceT<double>
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Retrieve items
		BOOST_CHECK((*p_test1)[0] == 0.);
		BOOST_CHECK(p_test1->at(0) == 0.);

		// Set and retrieve an item using two different functions
		BOOST_CHECK_NO_THROW((*p_test1)[0] = 1.);
		BOOST_CHECK((*p_test1)[0] == 1.);
		BOOST_CHECK_NO_THROW(p_test1->at(0) = 2.);
		BOOST_CHECK(p_test1->at(0) == 2.);
	}

	//------------------------------------------------------------------------------

	{ // Test the GStdSimpleVectorInterfaceT<double>::front() and back() functions
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Check the front and back of the vector -- we know the values
		BOOST_CHECK(p_test1->front() == 0.);
		BOOST_CHECK(p_test1->back() == 1.);
	}

	//------------------------------------------------------------------------------

	{ // Test iteration over the vector and retrieval of the end() iterator (Test of GStdSimpleVectorInterfaceT<double> functionality)
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Iterate over the sequence
		GDoubleCollection::iterator it;
		std::size_t itemCount = 0;
		for (it = p_test1->begin(); it != p_test1->end(); ++it) itemCount++;
		BOOST_CHECK(itemCount == nItems);
	}

	//------------------------------------------------------------------------------

	{ // Test inserting and erasure of items, the pop_and_block and resize functions and the getDataCopy and operator= functions (Test of GStdSimpleVectorInterfaceT<double> functionality)
		std::shared_ptr <GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Insert 1 item at position 1 and cross-check
		BOOST_CHECK_NO_THROW(p_test1->insert(p_test1->begin() + 1, 1.));
		BOOST_CHECK(p_test1->at(1) == 1.);
		BOOST_CHECK(p_test1->size() == nItems + 1);

		// Insert another (nItems - 1 ) items at position 0
		BOOST_CHECK_NO_THROW(p_test1->insert(p_test1->begin(), nItems - 1, 1.));
		BOOST_CHECK(p_test1->size() == 2 * nItems);
		BOOST_CHECK(p_test1->at(0) == 1.);

		// Erase 1 item at the beginning and cross-check
		BOOST_CHECK_NO_THROW(p_test1->erase(p_test1->begin()));
		BOOST_CHECK(p_test1->size() == 2 * nItems - 1);

		// Erase another nItems - 1 items from the beginning
		BOOST_CHECK_NO_THROW(p_test1->erase(p_test1->begin(), p_test1->begin() + nItems - 1));
		BOOST_CHECK(p_test1->size() == nItems);

		// Remove another item at the end
		BOOST_CHECK_NO_THROW(p_test1->pop_back());
		BOOST_CHECK(p_test1->size() == nItems - 1);

		// Remove all remaining items
		BOOST_CHECK_NO_THROW(p_test1->resize(0, 0.));
		BOOST_CHECK(p_test1->size() == 0);

		// Add a number of identical items, using the resize() function and cross-check
		BOOST_CHECK_NO_THROW(p_test1->resize(nItems, 1.));
		BOOST_CHECK(p_test1->size() == nItems);
		BOOST_CHECK(p_test1->count(1.) == nItems);

		std::vector<double> dataCopy;
		BOOST_CHECK_NO_THROW(p_test1->getDataCopy(dataCopy));
		BOOST_CHECK(dataCopy.size() == nItems);
		BOOST_CHECK((std::size_t) std::count(dataCopy.begin(), dataCopy.end(), 1.) == nItems);

		// Assign 1 to all positions and add further items
		for (std::size_t i = 0; i < dataCopy.size(); i++) dataCopy[i] = 0.;
		for (std::size_t i = 0; i < nItems; i++) dataCopy.push_back(0.);

		// Assign the vector to p_test1 and cross-check
		BOOST_CHECK_NO_THROW(p_test1->Gem::Common::GStdSimpleVectorInterfaceT<double>::operator=(dataCopy));
		BOOST_CHECK(p_test1->size() == 2 * nItems);
		BOOST_CHECK(p_test1->count(0.) == 2 * nItems);
	}

	//------------------------------------------------------------------------------

	// Restore the object to its pristine condition
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GDoubleCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<double>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GFPNumCollectionT<double>::specificTestsFailuresExpected_GUnitTests();

	// Nothing to check -- no local data

	// Remove the test adaptor
	this->resetAdaptor();

	// Restore the adaptor to its pristine condition
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GDoubleCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
