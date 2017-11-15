/**
 * @file GInt32Object.cpp
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

#include "geneva/GInt32Object.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32Object)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GInt32Object::GInt32Object()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32Object object
 */
GInt32Object::GInt32Object(const GInt32Object &cp)
	: GNumIntT<std::int32_t>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GInt32Object::GInt32Object(const std::int32_t &val)
	: GNumIntT<std::int32_t>(val)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GInt32Object::GInt32Object(
	const std::int32_t &lowerBoundary
	, const std::int32_t &upperBoundary
)
	: GNumIntT<std::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by a fixed value, plus the boundaries for random initialization. Note
 * that we do not enforce val to be inside of the initialization boundaries
 *
 * @param val The value to be assigned to the object
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GInt32Object::GInt32Object(
	const std::int32_t &val
	, const std::int32_t &lowerBoundary
	, const std::int32_t &upperBoundary
)
	: GNumIntT<std::int32_t>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GInt32Object::~GInt32Object() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GInt32Object &GInt32Object::operator=(const GInt32Object &cp) {
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
std::int32_t GInt32Object::operator=(const std::int32_t &val) {
	return GNumIntT<std::int32_t>::operator=(val);
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32Object::clone_() const {
	return new GInt32Object(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GInt32Object object
 *
 * @param  cp A constant reference to another GInt32Object object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32Object::operator==(const GInt32Object &cp) const {
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
 * Checks for inequality with another GInt32Object object
 *
 * @param  cp A constant reference to another GInt32Object object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32Object::operator!=(const GInt32Object &cp) const {
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
void GInt32Object::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GInt32Object reference independent of this object and convert the pointer
	const GInt32Object *p_load = Gem::Common::g_convert_and_compare<GObject, GInt32Object>(cp, this);

	GToken token("GInt32Object", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GNumIntT<std::int32_t>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32Object::name() const {
	return std::string("GInt32Object");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GInt32Object::int32Streamline(
	std::vector<std::int32_t> &parVec, const activityMode &am
) const {
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 *
 * @param parVec The map to which the local value should be attached
 */
void GInt32Object::int32Streamline(
	std::map<std::string, std::vector<std::int32_t>> &parVec, const activityMode &am
) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GInt32Object::int32Streamline(std::map<std::string, std::vector<std::int32_t>>& parVec) const: Error!" << std::endl
      << "No name was assigned to the object" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	std::vector<std::int32_t> parameters;
	parameters.push_back(this->value());
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
void GInt32Object::int32Boundaries(
	std::vector<std::int32_t> &lBndVec, std::vector<std::int32_t> &uBndVec, const activityMode &am
) const {
	lBndVec.push_back(this->getLowerInitBoundary());
	uBndVec.push_back(this->getUpperInitBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a std::int32_t value
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number 1, as we own a single std::int32_t parameter
 */
std::size_t GInt32Object::countInt32Parameters(
	const activityMode &am
) const {
	return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GInt32Object::assignInt32ValueVector(
	const std::vector<std::int32_t> &parVec, std::size_t &pos, const activityMode &am
) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
	   glogger
	   << "In GBooleanObject::assignInt32ValueVector(const std::vector<std::int32_t>&, std::size_t&):" << std::endl
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
void GInt32Object::assignInt32ValueVectors(
	const std::map<std::string, std::vector<std::int32_t>> &parMap, const activityMode &am
) {
	this->setValue((Gem::Common::getMapItem(parMap, this->getParameterName())).at(0));
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GInt32Object::int32MultiplyByRandom(
	const std::int32_t &min
	, const std::int32_t &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_int_distribution<std::int32_t> uniform_int_distribution(min, max);
	GParameterT<std::int32_t>::setValue(GParameterT<std::int32_t>::value() * uniform_int_distribution(gr));
}

/******************************************************************************/
/**
 * Multiplication with a random DOUBLE value in the range [0,1[
 */
void GInt32Object::int32MultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
	GParameterT<std::int32_t>::setValue(
		boost::numeric_cast<std::int32_t>(
		  boost::numeric_cast<double>(GParameterT<std::int32_t>::value())
		  * uniform_real_distribution(gr)
		)
	);
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GInt32Object::int32MultiplyBy(
	const std::int32_t &val
	, const activityMode &am
) {
	GParameterT<std::int32_t>::setValue(val * GParameterT<std::int32_t>::value());
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GInt32Object::int32FixedValueInit(
	const std::int32_t &val
	, const activityMode &am
) {
	GParameterT<std::int32_t>::setValue(val);
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GInt32Object::int32Add(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GInt32Object> p = GParameterBase::parameterbase_cast<GInt32Object>(p_base);
	GParameterT<std::int32_t>::setValue(this->value() + p->value());
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GInt32Object::int32Subtract(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GInt32Object> p = GParameterBase::parameterbase_cast<GInt32Object>(p_base);
	GParameterT<std::int32_t>::setValue(this->value() - p->value());
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Object object, camouflaged as a GObject
 */
void GInt32Object::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GInt32Object * p_load = Gem::Common::g_convert_and_compare<GObject, GInt32Object>(cp, this);

	// Load our parent class'es data ...
	GNumIntT<std::int32_t>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Object::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (GNumIntT<std::int32_t>::modify_GUnitTests()) result = true;

	this->setValue(this->value() + 1);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GInt32Object::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Object::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	// A few settings
	const std::size_t nTests = 10000;
	const std::int32_t LOWERINITBOUNDARY = -10;
	const std::int32_t UPPERINITBOUNDARY = 10;
	const std::int32_t FIXEDVALUEINIT = 1;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<std::int32_t>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 0.5, 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GNumIntT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Test different ways of adding an adaptor (Test of GParameterBaseWithAdaptorsT<T> functions)
		std::shared_ptr <GInt32Object> p_test = this->clone<GInt32Object>();

		// Make sure we start in pristine condition. This will add a GInt32FlipAdaptor.
		BOOST_CHECK_NO_THROW(p_test->resetAdaptor());

		//********************************
		// Adding an adaptor of different type present should clone the adaptor
		BOOST_CHECK_NO_THROW(p_test->addAdaptor(giga_ptr));

		// Check that the addresses of both adaptors differ
		std::shared_ptr <GInt32GaussAdaptor> giga_clone_ptr;
		BOOST_CHECK_NO_THROW(giga_clone_ptr = p_test->getAdaptor<GInt32GaussAdaptor>());
		BOOST_CHECK(giga_clone_ptr.get() != giga_ptr.get());

		//********************************
		// Adding an adaptor when an adaptor of the same type is present should leave the original address intact

		// Make a note of the stored adaptor's address
		GInt32GaussAdaptor *ptr_store = giga_clone_ptr.get();

		// Add the "global" adaptor again, should be load()-ed
		BOOST_CHECK_NO_THROW(p_test->addAdaptor(giga_ptr));

		// Retrieve the adaptor again
		std::shared_ptr <GInt32GaussAdaptor> giga_clone2_ptr;
		BOOST_CHECK_NO_THROW(giga_clone2_ptr = p_test->getAdaptor<GInt32GaussAdaptor>());

		// Check that the address hasn't changed
		BOOST_CHECK(ptr_store == giga_clone2_ptr.get());

		//********************************
	}

	// Reset to the original state
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

	// --------------------------------------------------------------------------

	{ // Check that construction with initialization boundaries leads to random content

		std::int32_t previous = -1;
		for (std::size_t i = 0; i < 10; i++) {
			GInt32Object p(0, 10000000);
			BOOST_CHECK(p.value() != previous);
			previous = p.value();
		}
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GInt32Object::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32Object::specificTestsFailuresExpected_GUnitTests() {
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
	GNumIntT<std::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------

#ifdef DEBUG
	{ // Check that retrieval of the adaptor with simultaneous conversion to an incorrect target type throws (Test of GParameterBaseWithAdaptorsT<T> functions)
		std::shared_ptr<GInt32Object> p_test = this->clone<GInt32Object>();

		// Make sure an adaptor is present
		BOOST_REQUIRE(p_test->hasAdaptor() == true);

		// Make sure the local adaptor has the type we expect
		BOOST_CHECK(p_test->getAdaptor()->getAdaptorId() == adaptorId::GINT32GAUSSADAPTOR);

		// Attempted conversion to an invalid target type should throw
		BOOST_CHECK_THROW(p_test->getAdaptor<GInt32FlipAdaptor>(), gemfony_error_condition);
	}
#endif /* DEBUG */

	// --------------------------------------------------------------------------

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GInt32Object::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
