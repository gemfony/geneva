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

#include "geneva/GConstrainedInt32Object.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedInt32Object)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization with boundaries only. A random value inside of the allowed ranges will
 * be assigned to the object.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedInt32Object::GConstrainedInt32Object(
	const std::int32_t &lowerBoundary, const std::int32_t &upperBoundary
)
	: GConstrainedIntT<std::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedInt32Object::GConstrainedInt32Object(
	const std::int32_t &val, const std::int32_t &lowerBoundary, const std::int32_t &upperBoundary
)
	: GConstrainedIntT<std::int32_t>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedInt32Object::GConstrainedInt32Object(const std::int32_t &val)
	: GConstrainedIntT<std::int32_t>(val)
{ /* nothing */ }

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GConstrainedInt32Object& GConstrainedInt32Object::operator=(const std::int32_t &val) {
	GConstrainedIntT<std::int32_t>::operator=(val);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GConstrainedInt32Object::clone_() const {
	return new GConstrainedInt32Object(*this);
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
void GConstrainedInt32Object::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GConstrainedInt32Object reference independent of this object and convert the pointer
	const GConstrainedInt32Object *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedInt32Object>(cp, this);

	GToken token("GConstrainedInt32Object", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GConstrainedIntT<std::int32_t>>(*this, *p_load, token);

	// .... no local data

	// React on deviations from the expectation
	token.evaluate();
}


/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedInt32Object::name_() const {
	return std::string("GConstrainedInt32Object");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedInt32Object::int32Streamline(
	std::vector<std::int32_t> &parVec, const activityMode &am
) const {
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedInt32Object::int32Streamline(
	std::map<std::string, std::vector<std::int32_t>> &parVec, const activityMode &am
) const {
	std::vector<std::int32_t> parameters;
	parameters.push_back(this->value());
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type std::int32_t to the vectors.
 *
 * @param lBndVec A vector of lower std::int32_t parameter boundaries
 * @param uBndVec A vector of upper std::int32_t parameter boundaries
 */
void GConstrainedInt32Object::int32Boundaries(
	std::vector<std::int32_t> &lBndVec, std::vector<std::int32_t> &uBndVec, const activityMode &am
) const {
	lBndVec.push_back(this->getLowerBoundary());
	uBndVec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a std::int32_t value
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number 1, as we own a single std::int32_t parameter
 */
std::size_t GConstrainedInt32Object::countInt32Parameters(
	const activityMode &am
) const {
	return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation
 * to the assigned value, so that it lies inside of the allowed value range.
 */
void GConstrainedInt32Object::assignInt32ValueVector(
	const std::vector<std::int32_t> &parVec, std::size_t &pos, const activityMode &am
) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GConstrainedInt32Object::assignInt32ValueVector(const std::vector<std::int32_t>&, std::size_t&):" << std::endl
				<< "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
		);
	}
#endif

	this->setValue(this->transfer(parVec[pos]));
	pos++;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GConstrainedInt32Object::assignInt32ValueVectors(
	const std::map<std::string, std::vector<std::int32_t>> &parMap, const activityMode &am
) {
	this->setValue(
		this->transfer(
			Gem::Common::getMapItem(
				parMap, this->getParameterName()
			).at(0)
		)
	);
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GConstrainedInt32Object::int32MultiplyByRandom(
	const std::int32_t &min
	, const std::int32_t &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_int_distribution<std::int32_t> uniform_int_distribution(min, max);
	GParameterT<std::int32_t>::setValue(
		transfer(GParameterT<std::int32_t>::value() * uniform_int_distribution(gr))
	);
}

/******************************************************************************/
/**
 * Multiplication with a random DOUBLE value in the range [0,1[
 */
void GConstrainedInt32Object::int32MultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
	GParameterT<std::int32_t>::setValue(
		transfer(
			boost::numeric_cast<std::int32_t>(
				boost::numeric_cast<double>(GParameterT<std::int32_t>::value())
				* uniform_real_distribution(gr)
			)
		)
	);
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedInt32Object::int32MultiplyBy(
	const std::int32_t &val
	, const activityMode &am
) {
	GParameterT<std::int32_t>::setValue(transfer(val * GParameterT<std::int32_t>::value()));
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GConstrainedInt32Object::int32FixedValueInit(
	const std::int32_t &val
	, const activityMode &am
) {
	GParameterT<std::int32_t>::setValue(transfer(val));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedInt32Object::int32Add(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GConstrainedInt32Object> p = GParameterBase::parameterbase_cast<GConstrainedInt32Object>(p_base);
	GParameterT<std::int32_t>::setValue(transfer(this->value() + p->value()));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedInt32Object::int32Subtract(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GConstrainedInt32Object> p = GParameterBase::parameterbase_cast<GConstrainedInt32Object>(p_base);
	GParameterT<std::int32_t>::setValue(transfer(this->value() - p->value()));
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedInt32Object object, camouflaged as a GObject
 */
void GConstrainedInt32Object::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GConstrainedInt32Object * p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedInt32Object>(cp, this);

	// Load our parent class'es data ...
	GConstrainedIntT<std::int32_t>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
bool GConstrainedInt32Object::randomInit_(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	return GConstrainedIntT<std::int32_t>::randomInit_(am, gr);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedInt32Object::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GConstrainedIntT<std::int32_t>::modify_GUnitTests()) result = true;

	if(this->value() == this->getLowerBoundary()) {
		this->setValue(this->getLowerBoundary() + 1);
	} else {
		this->setValue(this->getLowerBoundary());
	}
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedInt32Object::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedInt32Object::specificTestsNoFailureExpected_GUnitTests() {
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
	giga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GConstrainedIntT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedInt32Object::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedInt32Object::specificTestsFailuresExpected_GUnitTests() {
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
	giga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GConstrainedIntT<std::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedInt32Object::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
