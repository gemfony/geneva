/**
 * @file GFloatCollection.cpp
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

#include "geneva/GFloatCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFloatCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GFloatCollection::GFloatCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GFloatCollection::GFloatCollection(
		const std::size_t& nval
		, const float& min
		, const float& max
)
	: GFPNumCollectionT<float>(nval, min, max)
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
GFloatCollection::GFloatCollection(
		const std::size_t& nval
		, const float& val
		, const float& min
		, const float& max
)
	: GFPNumCollectionT<float>(nval, val, min, max)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GFloatCollection object
 */
GFloatCollection::GFloatCollection(const GFloatCollection& cp)
	: GFPNumCollectionT<float>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFloatCollection::~GFloatCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GFloatCollection object
 * @return A constant reference to this object
 */
const GFloatCollection& GFloatCollection::operator=(const GFloatCollection& cp){
	GFloatCollection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GFloatCollection::clone_() const {
	return new GFloatCollection(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GFloatCollection object
 *
 * @param  cp A constant reference to another GFloatCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GFloatCollection::operator==(const GFloatCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFloatCollection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GFloatCollection object
 *
 * @param  cp A constant reference to another GFloatCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GFloatCollection::operator!=(const GFloatCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GFloatCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GFloatCollection::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GFloatCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GFPNumCollectionT<float>::checkRelationshipWith(cp, e, limit, "GFloatCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GFloatCollection", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFloatCollection::name() const {
   return std::string("GFloatCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GFloatCollection::floatStreamline(std::vector<float>& parVec) const {
	GFloatCollection::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		parVec.push_back(*cit);
	}
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GFloatCollection::assignFloatValueVectors(const std::map<std::string, std::vector<float> >& parMap) {
   GFloatCollection::iterator it;
   std::size_t cnt = 0;
   for(it=this->begin(); it!=this->end(); ++it) {
      *it = (Gem::Common::getMapItem(parMap,this->getParameterName())).at(cnt++);
   }
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param parVec The map to which the local value should be attached
 */
void GFloatCollection::floatStreamline(std::map<std::string, std::vector<float> >& parVec) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GFloatCollection::floatStreamline(std::map<std::string, float>& parVec) const: Error!" << std::endl
      << "No name was assigned to the object" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   std::vector<float> parameters;
   this->floatStreamline(parameters);
   parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors
 *
 * @param lBndVec A vector of lower float parameter boundaries
 * @param uBndVec A vector of upper float parameter boundaries
 */
void GFloatCollection::floatBoundaries(
		std::vector<float>& lBndVec
		, std::vector<float>& uBndVec
) const {
	// Add as man lower and upper boundaries to the vector as
	// there are variables
	GFloatCollection::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		lBndVec.push_back(this->getLowerInitBoundary());
		uBndVec.push_back(this->getUpperInitBoundary());
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of float values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of float parameters
 */
std::size_t GFloatCollection::countFloatParameters(
   const activityMode& am
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
void GFloatCollection::assignFloatValueVector(
		const std::vector<float>& parVec
		, std::size_t& pos
) {
	  for(GFloatCollection::iterator it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
		  // Do we have a valid position ?
		  if(pos >= parVec.size()) {
		     glogger
		     << "In GFloatCollection::assignFloatValueVector(const std::vector<float>&, std::size_t&):" << std::endl
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
 * Loads the data of another GObject
 *
 * @param cp A copy of another GFloatCollection object, camouflaged as a GObject
 */
void GFloatCollection::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GFloatCollection>(cp);

	// Load our parent class'es data ...
	GFPNumCollectionT<float>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFloatCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GFPNumCollectionT<float>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with some random data
 */
void GFloatCollection::fillWithData(const std::size_t& nItems) {
#ifdef GEM_TESTING
	// Make sure the collection is empty
	BOOST_CHECK_NO_THROW(this->clear());

	// Cross check that it really is
	BOOST_CHECK(this->size() == 0);
	// Use another method
	BOOST_CHECK(this->empty());

	// Add a single item of defined value, so we can test the find() and count() functions
	BOOST_CHECK_NO_THROW(this->push_back(0.));

	for(std::size_t i=1; i<nItems - 1; i++) {
		BOOST_CHECK_NO_THROW(this->push_back(gr->uniform_real<float>(-10., 10.)));
	}

	// Add a single item of defined value, so we can test the find() and count() functions
	BOOST_CHECK_NO_THROW(this->push_back(1.));

	// Cross-check the size
	BOOST_CHECK(this->size() == nItems);
	BOOST_CHECK(!this->empty());

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatCollection::fillWithData", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFloatCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// A few settings
	const std::size_t nItems = 10000;
	const std::size_t nTests = 10;
	const float FIXEDVALUEINIT = 1.;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<float> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GFloatGaussAdaptor> gdga_ptr(new GFloatGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GFPNumCollectionT<float>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test the GParameterT<T>::adaptImpl() implementation
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();
		boost::shared_ptr<GFloatCollection> p_test2 = this->clone<GFloatCollection>();

		if(p_test1->hasAdaptor()) {
			// Make sure the collection is clean
			p_test1->clear();

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(FIXEDVALUEINIT);
			}

			for(std::size_t t=0; t<nTests; t++) {
				// Load p_test1 into p_test2
				BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

				// Make sure the objects match
				BOOST_CHECK(*p_test1 == *p_test2);

				// Adapt p_test1, using the internal function
				BOOST_CHECK_NO_THROW(p_test1->adaptImpl());

				// Test whether the two objects differ now
				BOOST_CHECK(*p_test1 != *p_test2);

				// Check that each element differs
				for(std::size_t i=0; i<nItems; i++) {
					BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
				}
			}
		}
	}

	//------------------------------------------------------------------------------

	{ // Test of GParameterT<T>::swap(const GParameterT<T>&)
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();
		boost::shared_ptr<GFloatCollection> p_test2 = this->clone<GFloatCollection>();
		boost::shared_ptr<GFloatCollection> p_test3 = this->clone<GFloatCollection>();

		if(p_test1->hasAdaptor()) {
			// Make sure the collection is clean
			p_test1->clear();

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
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
			BOOST_CHECK_NO_THROW(p_test1->adaptImpl());

			// Test whether p_test1 and p_test2/3 differ now
			BOOST_CHECK(*p_test1 != *p_test2);
			BOOST_CHECK(*p_test1 != *p_test3);
			// Test whether p_test2 is still the same as p_test3
			BOOST_CHECK(*p_test3 == *p_test2);

			// Swap the data of p_test2 and p_test1
			BOOST_CHECK_NO_THROW(p_test2->swap(*p_test1));

			// Now p_test1 and p_test3 should be the same, while p_test2 differs from both
			BOOST_CHECK(*p_test1 == *p_test3);
			BOOST_CHECK(*p_test2 != *p_test1);
			BOOST_CHECK(*p_test2 != *p_test3);
		}
	}

	//------------------------------------------------------------------------------

	{ // Test the GStdSimpleVectorInterfaceT<float>::reserve(), capacity() and max_size() functions
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

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
		BOOST_CHECK(!p_test1->empty());
	}

	//------------------------------------------------------------------------------

	{ // Test the GStdSimpleVectorInterfaceT<float>::count(), find() and begin() functions
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Count the number of values == 0. . Should be >= 1
		BOOST_CHECK(p_test1->count(0.) >= 1);
		// Count the number of values == 1. . Should be >= 1
		BOOST_CHECK(p_test1->count(1.) >= 1);

		// Find the item with value 0. -- the first one is in position 0
		GFloatCollection::const_iterator find_it, pos_it;
		BOOST_CHECK_NO_THROW(pos_it = p_test1->begin());
		BOOST_CHECK_NO_THROW(find_it = p_test1->find(0.));
		BOOST_CHECK(find_it == pos_it);
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of items with the operator[] and at() functions of GStdSimpleVectorInterfaceT<float>
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

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

	{ // Test the GStdSimpleVectorInterfaceT<float>::front() and back() functions
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Check the front and back of the vector -- we know the values
		BOOST_CHECK(p_test1->front() == 0.);
		BOOST_CHECK(p_test1->back() == 1.);
	}

	//------------------------------------------------------------------------------

	{ // Test iteration over the vector and retrieval of the end() iterator (Test of GStdSimpleVectorInterfaceT<float> functionality)
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Iterate over the sequence
		GFloatCollection::iterator it;
		std::size_t itemCount = 0;
		for(it=p_test1->begin(); it!= p_test1->end(); ++it) itemCount++;
		BOOST_CHECK(itemCount == nItems);
	}

	//------------------------------------------------------------------------------

	{ // Test inserting and erasure of items, the pop_back and resize functions and the getDataCopy and operator= functions (Test of GStdSimpleVectorInterfaceT<float> functionality)
		boost::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

		// Add some data
		BOOST_CHECK_NO_THROW(p_test1->fillWithData(nItems));

		// Insert 1 item at position 1 and cross-check
		BOOST_CHECK_NO_THROW(p_test1->insert(p_test1->begin() + 1, 1.));
		BOOST_CHECK(p_test1->at(1) == 1.);
		BOOST_CHECK(p_test1->size() == nItems + 1);

		// Insert another (nItems - 1 ) items at position 0
		BOOST_CHECK_NO_THROW(p_test1->insert(p_test1->begin(), nItems-1, 1.));
		BOOST_CHECK(p_test1->size() == 2*nItems);
		BOOST_CHECK(p_test1->at(0) == 1.);

		// Erase 1 item at the beginning and cross-check
		BOOST_CHECK_NO_THROW(p_test1->erase(p_test1->begin()));
		BOOST_CHECK(p_test1->size() == 2*nItems - 1);

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

		std::vector<float> dataCopy;
		BOOST_CHECK_NO_THROW(p_test1->getDataCopy(dataCopy));
		BOOST_CHECK(dataCopy.size() == nItems);
		BOOST_CHECK((std::size_t)std::count(dataCopy.begin(), dataCopy.end(), 1.) == nItems);

		// Assign 1 to all positions and add further items
		for(std::size_t i=0; i<dataCopy.size(); i++) dataCopy[i] = 0.;
		for(std::size_t i=0; i<nItems; i++) dataCopy.push_back(0.);

		// Assign the vector to p_test1 and cross-check
		BOOST_CHECK_NO_THROW(p_test1->GStdSimpleVectorInterfaceT<float>::operator=(dataCopy));
		BOOST_CHECK(p_test1->size() == 2*nItems);
		BOOST_CHECK(p_test1->count(0.) == 2*nItems);
	}

	//------------------------------------------------------------------------------

	// Restore the object to its pristine condition
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFloatCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<float> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GFloatGaussAdaptor> gdga_ptr(new GFloatGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GFPNumCollectionT<float>::specificTestsFailuresExpected_GUnitTests();

	// Nothing to check -- no local data

	// Remove the test adaptor
	this->resetAdaptor();

	// Restore the adaptor to its pristine condition
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}


#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
