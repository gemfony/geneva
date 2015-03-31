/**
 * @file GBooleanObject.cpp
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

#include "geneva/GBooleanObject.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanObject)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBooleanObject::GBooleanObject()
	: GParameterT<bool>()
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBooleanObject object
 */
GBooleanObject::GBooleanObject(const GBooleanObject& cp)
	: GParameterT<bool>(cp)
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GBooleanObject::GBooleanObject(const bool& val)
	: GParameterT<bool>(val)
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a given probability for "true"
 *
 * @param prob The probability for the value "true"
 */
GBooleanObject::GBooleanObject(const double& probability) {
   using namespace Gem::Common;
   using namespace Gem::Hap;

	this->setValue(GObject::gr_ptr()->weighted_bool(probability));
}

// Tested in this file

/******************************************************************************/
/**
 * The destructor
 */
GBooleanObject::~GBooleanObject()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBooleanObject& GBooleanObject::operator=(
   const GBooleanObject& cp
) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
bool GBooleanObject::operator=(const bool& val) {
	return GParameterT<bool>::operator=(val);
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBooleanObject::clone_() const {
	return new GBooleanObject(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GBooleanObject object
 *
 * @param  cp A constant reference to another GBooleanObject object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanObject::operator==(const GBooleanObject& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBooleanObject::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBooleanObject object
 *
 * @param  cp A constant reference to another GBooleanObject object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBooleanObject::operator!=(const GBooleanObject& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBooleanObject::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Random initialization. This is a helper function, without it we'd
 * have to say things like "myGBooleanObject.GParameterBase::randomInit();".
 */
void GBooleanObject::randomInit(const activityMode& am) {
	  GParameterBase::randomInit(am);
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure
 */
void GBooleanObject::randomInit(const double& probability, const activityMode& am) {
  if(
     !GParameterBase::randomInitializationBlocked()
     && this->modifiableAmMatchOrHandover(am)
  ) {
     randomInit_(probability, am);
  }
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
void GBooleanObject::randomInit_(const activityMode&) {
   using namespace Gem::Common;
   using namespace Gem::Hap;

	this->setValue(GObject::gr_ptr()->uniform_bool());
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure.
 * This function holds the actual initialization logic, used in the publicly accessible
 * GBooleanObject::randomInit(const double& probability) function.
 */
void GBooleanObject::randomInit_(const double& probability, const activityMode& am) {
   using namespace Gem::Common;
   using namespace Gem::Hap;

   this->setValue(GObject::gr_ptr()->weighted_bool(probability));
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***************************************************************************/
/**
 * Returns a "comparative range". In the case of boolean values this must
 * be considered to be more of a "dummy".
 */
bool GBooleanObject::range() const {
   return true;
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
boost::optional<std::string> GBooleanObject::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check for a possible self-assignment
	GObject::selfAssignmentCheck<GBooleanObject>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterT<bool>::checkRelationshipWith(cp, e, limit, "GBooleanObject", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GBooleanObject", caller, deviations, e);
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
void GBooleanObject::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GBooleanObject *p_load = GObject::gobject_conversion<GBooleanObject>(&cp);

   try {
      // Check our parent class'es data ...
      GParameterT<bool>::compare(cp, e, limit);

      // ... no local data

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      g.add("g_expectation_violation caught by GBooleanObject");
      throw g;
   }
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanObject::name() const {
   return std::string("GBooleanObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GBooleanObject::booleanStreamline(
   std::vector<bool>& parVec
   , const activityMode& am
) const {
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 *
 * @param parVec The map to which the local value should be attached
 */
void GBooleanObject::booleanStreamline(
   std::map<std::string, std::vector<bool> >& parVec
   , const activityMode& am
) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GBooleanObject::booleanStreamline(std::map<std::string, std::vector<bool> >& parVec) const: Error!" << std::endl
      << "No name was assigned to the object" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   std::vector<bool> parameters;
   parameters.push_back(this->value());
   parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors. This function has been added for
 * completeness reasons only.
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
void GBooleanObject::booleanBoundaries(
   std::vector<bool>& lBndVec
   , std::vector<bool>& uBndVec
   , const activityMode& am
) const {
	lBndVec.push_back(false);
	uBndVec.push_back(true);
}


/******************************************************************************/
/**
 * Tell the audience that we own a bool value
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of active, incactive or all float parameters
 */
std::size_t GBooleanObject::countBoolParameters(
   const activityMode& am
) const {
   return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GBooleanObject::assignBooleanValueVector(
   const std::vector<bool>& parVec
   , std::size_t& pos
   , const activityMode& am
) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
	   glogger
	   << "In GBooleanObject::assignBooleanValueVector(const std::vector<bool>&, std::size_t&):" << std::endl
      << "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
      << GEXCEPTION;
	}
#endif

	this->setValue(parVec[pos]);
	pos++;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GBooleanObject::assignBooleanValueVectors(
   const std::map<std::string
   , std::vector<bool> >& parMap
   , const activityMode& am
) {
   this->setValue((Gem::Common::getMapItem<std::vector<bool> >(parMap,this->getParameterName())).at(0));
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanObject object, camouflaged as a GObject
 */
void GBooleanObject::load_(const GObject* cp){
	// Check for a possible self-assignment
	GObject::selfAssignmentCheck<GBooleanObject>(cp);

	// Load our parent class'es data ...
	GParameterT<bool>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanObject::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GParameterT<bool>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObject::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanObject::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Some general settings
	const bool FIXEDVALUEINIT = true;
    const double LOWERBND = 0.8, UPPERBND = 1.2;
	const std::size_t nTests = 10000;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<bool> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
	gba_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gba_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gba_ptr);

	// Call the parent class'es function
	GParameterT<bool>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Test default constructor
		GBooleanObject gbo;
		BOOST_CHECK_MESSAGE (
				gbo.value() == Gem::Common::GDefaultValueT<bool>::value()
				, "\n"
				<< "gbo.value() = " << gbo.value()
				<< "DEFBOVAL = " << Gem::Common::GDefaultValueT<bool>::value()
		);
	}

	// --------------------------------------------------------------------------

	{ // Test copy construction and construction with value
		GBooleanObject gbo1(false), gbo2(gbo1);

		BOOST_CHECK_MESSAGE (
				gbo1.value()==false && gbo2.value()==gbo1.value()
				, "\n"
				<< "gbo1.value() = " << gbo1.value()
				<< "gbo2.value() = " << gbo2.value()
		);
	}

	// --------------------------------------------------------------------------

	{ // Check construction with a given probability for the value "true"
		std::size_t nTrue=0, nFalse=0;
		for(std::size_t i=0; i<nTests; i++) {
			GBooleanObject gbo(0.5);
			gbo.value()==true?nTrue++:nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue)/double(nFalse);
		BOOST_CHECK_MESSAGE(
				ratio>LOWERBND && ratio<UPPERBND
				,  "\n"
				<< "ratio = " << ratio << "\n"
				<< "nTrue = " << nTrue << "\n"
				<< "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that random initialization with equal probability for true and false will result in roughly the same amount of corresponding values
		boost::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test = true);
		// Cross-check
		BOOST_CHECK(p_test->value() == true);

		// Count the number of true and false values for a number of subsequent initializations
		// with the internal randomInit_ function.
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for(std::size_t i=0; i<nTests; i++) {
			p_test->randomInit_(ALLPARAMETERS);
			p_test->value()?nTrue++:nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue)/double(nFalse);
		BOOST_CHECK_MESSAGE(
				ratio>0.8 && ratio<1.2
				,  "\n"
				<< "ratio = " << ratio << "\n"
				<< "nTrue = " << nTrue << "\n"
				<< "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that initialization with a probability of 1 for true will only result in true values
		boost::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test = false);
		// Cross-check
		BOOST_CHECK(p_test->value() == false);

		// Count the number of true and false values for a number of subsequent initializations
		// with the internal randomInit_ function.
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for(std::size_t i=0; i<nTests; i++) {
			p_test->randomInit_(1., ALLPARAMETERS);
			p_test->value()?nTrue++:nFalse++;
		}

		// We should have received only true values
		BOOST_CHECK(nTrue ==nTests);
	}

	// --------------------------------------------------------------------------

	{ // Test that initialization with a probability of 0 for true will only result in false values
		boost::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test = true);
		// Cross-check
		BOOST_CHECK(p_test->value() == true);

		// Count the number of true and false values for a number of subsequent initializations
		// with the internal randomInit_ function.
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for(std::size_t i=0; i<nTests; i++) {
			p_test->randomInit_(0., ALLPARAMETERS);
			p_test->value()?nTrue++:nFalse++;
		}

		// We should have received only true values
		BOOST_CHECK(nFalse ==nTests);
	}

	//-----------------------------------------------------------------------------

	{ // Test that random initialization with a given probability for true will result in roughly the expected amount of corresponding values
		for(double d=0.1; d<0.9; d+=0.1) {
			boost::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

			// Assign a boolean value true
			BOOST_CHECK_NO_THROW(*p_test = true);
			// Cross-check
			BOOST_CHECK(p_test->value() == true);

			// Randomly initialize, using the internal function, with the current probability
			BOOST_CHECK_NO_THROW(p_test->randomInit_(d, ALLPARAMETERS));

			// Count the number of true and false values for a number of subsequent initializations
			// with the internal randomInit_ function.
			std::size_t nTrue = 0;
			std::size_t nFalse = 0;
			for(std::size_t i=0; i<nTests; i++) {
				p_test->randomInit_(d, ALLPARAMETERS);
				p_test->value()?nTrue++:nFalse++;
			}

			// We allow a slight deviation, as the initialization is a random process
			double expectedTrueMin = 0.8*d*nTests;
			double expectedTrueMax = 1.2*d*nTests;

			BOOST_CHECK_MESSAGE(
					double(nTrue) > expectedTrueMin && double(nTrue) < expectedTrueMax
					,  "\n"
					<< "d = " << d << "\n"
					<< "Allowed window = " << expectedTrueMin << " - " << expectedTrueMax << "\n"
					<< "nTests = " << nTests << "\n"
					<< "nTrue = " << nTrue << "\n"
					<< "nFalse = " << nFalse << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization can be blocked for equal distributions
		boost::shared_ptr<GBooleanObject> p_test1 = this->clone<GBooleanObject>();
		boost::shared_ptr<GBooleanObject> p_test2 = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = true);
		// Cross-check
		BOOST_CHECK(p_test1->value() == true);

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
		BOOST_CHECK_NO_THROW(p_test1->randomInit(ALLPARAMETERS));

		// Check that both objects are still the same
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization can be blocked for distributions with a given probability structure
		boost::shared_ptr<GBooleanObject> p_test1 = this->clone<GBooleanObject>();
		boost::shared_ptr<GBooleanObject> p_test2 = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = true);
		// Cross-check
		BOOST_CHECK(p_test1->value() == true);

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
		BOOST_CHECK_NO_THROW(p_test1->randomInit(0.7, ALLPARAMETERS));

		// Check that both objects are still the same
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

	{ // Check that the fp-family of functions doesn't have an effect on this object
		boost::shared_ptr<GBooleanObject> p_test1 = this->GObject::clone<GBooleanObject>();
		boost::shared_ptr<GBooleanObject> p_test2 = this->GObject::clone<GBooleanObject>();
		boost::shared_ptr<GBooleanObject> p_test3 = this->GObject::clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = true);
		// Cross-check
		BOOST_CHECK(p_test1->value() == true);

		// Load into p_test2 and p_test3 and test equality
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
		BOOST_CHECK(*p_test2 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that initialization with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fixedValueInit<double>(2., ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that multiplication with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyBy<double>(2., ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(1., 2., ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that adding p_test1 to p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->add<double>(p_test1, ALLPARAMETERS));
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that subtracting p_test1 from p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->subtract<double>(p_test1, ALLPARAMETERS));
		BOOST_CHECK(*p_test3 == *p_test2);
	}

	// --------------------------------------------------------------------------

	// Restore the adaptor to pristine condition
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObject::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanObject::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<bool> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
	gba_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gba_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(gba_ptr);

	// Call the parent class'es function
	GParameterT<bool>::specificTestsFailuresExpected_GUnitTests();

	// Restore the adaptor to pristine condition
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObject::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
