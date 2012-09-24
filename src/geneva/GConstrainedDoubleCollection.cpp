/**
 * @file GConstrainedDoubleCollection.cpp
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

#include "geneva/GConstrainedDoubleCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedDoubleCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Protected, as it is only needed for de-serialization purposes.
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection()
	: GConstrainedFPNumCollectionT<double> ()
{ /* nothing */ }

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
GConstrainedDoubleCollection::GConstrainedDoubleCollection (
		const std::size_t& size
		, const double& lowerBoundary
		, const double& upperBoundary
)
	: GConstrainedFPNumCollectionT<double> (size, lowerBoundary, upperBoundary)
{ /* nothing */ }

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
GConstrainedDoubleCollection::GConstrainedDoubleCollection (
		const std::size_t& size
		, const double& val
		, const double& lowerBoundary
		, const double& upperBoundary
)
	: GConstrainedFPNumCollectionT<double> (size, val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GConstrainedDoubleCollection object
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection(const GConstrainedDoubleCollection& cp)
	: GConstrainedFPNumCollectionT<double> (cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GConstrainedDoubleCollection::~GConstrainedDoubleCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp A copy of another GConstrainedDoubleCollection object
 * @return A constant reference to this object
 */
const GConstrainedDoubleCollection& GConstrainedDoubleCollection::operator=(const GConstrainedDoubleCollection& cp){
	GConstrainedDoubleCollection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GConstrainedDoubleCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedDoubleCollection::operator==(const GConstrainedDoubleCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedDoubleCollection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GConstrainedDoubleCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedDoubleCollection::operator!=(const GConstrainedDoubleCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedDoubleCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedDoubleCollection::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const	{
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GConstrainedDoubleCollection  *p_load = GObject::gobject_conversion<GConstrainedDoubleCollection>(&cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GConstrainedDoubleCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GConstrainedFPNumCollectionT<double>::checkRelationshipWith(cp, e, limit, "GConstrainedDoubleCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedDoubleCollection", caller, deviations, e);
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedDoubleCollection::doubleStreamline(std::vector<double>& parVec) const {
	for(std::size_t pos = 0; pos < this->size(); pos++) {
		parVec.push_back(this->transfer(this->at(pos)));
	}
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors.
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GConstrainedDoubleCollection::doubleBoundaries(
		std::vector<double>& lBndVec
		, std::vector<double>& uBndVec
) const {
	// Add a lower and upper boundary to the vectors
	// for each variable in the collection
	for(std::size_t pos = 0; pos < this->size(); pos++) {
		lBndVec.push_back(this->getLowerBoundary());
		uBndVec.push_back(this->getUpperBoundary());
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of double values
 *
 * @return The number of double parameters
 */
std::size_t GConstrainedDoubleCollection::countDoubleParameters() const {
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
void GConstrainedDoubleCollection::assignDoubleValueVector(const std::vector<double>& parVec, std::size_t& pos) {
	for(std::size_t i=0; i<this->size(); i++) {
#ifdef DEBUG
		  // Do we have a valid position ?
		  if(pos >= parVec.size()) {
			  raiseException(
					  "In GConstrainedDoubleCollection::assignDoubleValueVector(const std::vector<double>&, std::size_t&):" << std::endl
					  << "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos
			  );
		  }
#endif

		  this->setValue(i, this->transfer(parVec[pos]));
		  pos++;
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
void GConstrainedDoubleCollection::load_(const GObject *cp){
	// Convert cp into local format
	// const GConstrainedDoubleCollection *p_load = GObject::gobject_conversion<GConstrainedDoubleCollection>(cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GConstrainedDoubleCollection>(cp);

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
GObject* GConstrainedDoubleCollection::clone_() const {
	return new GConstrainedDoubleCollection(*this);
}

#ifdef GEM_TESTING
/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleCollection::modify_GUnitTests() {
	bool result = false;

	// Call the parent classes' functions
	if(GConstrainedFPNumCollectionT<double>::modify_GUnitTests()) result = true;

	return result;
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleCollection::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<double>::specificTestsNoFailureExpected_GUnitTests();

	// Some parameters
	const std::size_t DEFSIZE=10;
	const double DEFVAL=1.;
	const double DEFMIN=-10.;
	const double DEFMAX= 10.;

	//---------------------------------------------------------------------

	{ // Check that initialization with a fixed value-range yields the desired values
		boost::shared_ptr<GConstrainedDoubleCollection> p_test;

		BOOST_CHECK_NO_THROW(p_test = boost::shared_ptr<GConstrainedDoubleCollection>(new GConstrainedDoubleCollection(DEFSIZE, DEFMIN, DEFMAX)));
		BOOST_CHECK(p_test->size() == DEFSIZE && DEFSIZE>1);
		for(std::size_t i=1; i<DEFSIZE; i++) { // Check that consecutive values are different
			BOOST_CHECK(p_test->at(i) != p_test->at(i-1));
		}
		BOOST_CHECK(p_test->getLowerBoundary() == DEFMIN);
		BOOST_CHECK(p_test->getUpperBoundary() == boost::math::float_prior<double>(DEFMAX)); // The upper boundary is an open one
	}

	//---------------------------------------------------------------------

	{ // Check that initialization with a fixed value and range yields the desired values
		boost::shared_ptr<GConstrainedDoubleCollection> p_test;

		BOOST_CHECK_NO_THROW(p_test = boost::shared_ptr<GConstrainedDoubleCollection>(new GConstrainedDoubleCollection(DEFSIZE, DEFVAL, DEFMIN, DEFMAX)));
		BOOST_CHECK(p_test->size() == DEFSIZE);
		for(std::size_t i=0; i<DEFSIZE; i++) {
			BOOST_CHECK(p_test->at(i) == DEFVAL);
		}
		BOOST_CHECK(p_test->getLowerBoundary() == DEFMIN);
		BOOST_CHECK(p_test->getUpperBoundary() == boost::math::float_prior<double>(DEFMAX)); // The upper boundary is an open one
	}

	//---------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleCollection::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<double>::specificTestsFailuresExpected_GUnitTests();
}

/******************************************************************************/

#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GConstrainedDoubleCollection has a private default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GConstrainedDoubleCollection> TFactory_GUnitTests<Gem::Geneva::GConstrainedDoubleCollection>() {
	const std::size_t NPARAMETERS = 100;
	double LOWERBOUNDARY = -10.;
	double UPPERBOUNDARY =  10.;
	boost::shared_ptr<Gem::Geneva::GConstrainedDoubleCollection> p;
	BOOST_CHECK_NO_THROW(
			p= boost::shared_ptr<Gem::Geneva::GConstrainedDoubleCollection>(
					new Gem::Geneva::GConstrainedDoubleCollection(NPARAMETERS, LOWERBOUNDARY, UPPERBOUNDARY)
			)
	);
	return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */
