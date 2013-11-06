/**
 * @file GConstrainedFloatCollection.cpp
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

#include "geneva/GConstrainedFloatCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedFloatCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Protected, as it is only needed for de-serialization purposes.
 */
GConstrainedFloatCollection::GConstrainedFloatCollection()
	: GConstrainedFPNumCollectionT<float> ()
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
GConstrainedFloatCollection::GConstrainedFloatCollection (
		const std::size_t& size
		, const float& lowerBoundary
		, const float& upperBoundary
)
	: GConstrainedFPNumCollectionT<float> (size, lowerBoundary, upperBoundary)
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
GConstrainedFloatCollection::GConstrainedFloatCollection (
		const std::size_t& size
		, const float& val
		, const float& lowerBoundary
		, const float& upperBoundary
)
	: GConstrainedFPNumCollectionT<float> (size, val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GConstrainedFloatCollection object
 */
GConstrainedFloatCollection::GConstrainedFloatCollection(const GConstrainedFloatCollection& cp)
	: GConstrainedFPNumCollectionT<float> (cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GConstrainedFloatCollection::~GConstrainedFloatCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp A copy of another GConstrainedFloatCollection object
 * @return A constant reference to this object
 */
const GConstrainedFloatCollection& GConstrainedFloatCollection::operator=(const GConstrainedFloatCollection& cp){
	GConstrainedFloatCollection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GConstrainedFloatCollection object
 *
 * @param  cp A constant reference to another GConstrainedFloatCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedFloatCollection::operator==(const GConstrainedFloatCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedFloatCollection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GConstrainedFloatCollection object
 *
 * @param  cp A constant reference to another GConstrainedFloatCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedFloatCollection::operator!=(const GConstrainedFloatCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedFloatCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedFloatCollection::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const	{
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	// const GConstrainedFloatCollection  *p_load = GObject::gobject_conversion<GConstrainedFloatCollection>(&cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GConstrainedFloatCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GConstrainedFPNumCollectionT<float>::checkRelationshipWith(cp, e, limit, "GConstrainedFloatCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedFloatCollection", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedFloatCollection::name() const {
   return std::string("GConstrainedFloatCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedFloatCollection::floatStreamline(std::vector<float>& parVec) const {
   GConstrainedFloatCollection::const_iterator cit;
   for(cit=this->begin(); cit!=this->end(); ++cit) {
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
void GConstrainedFloatCollection::floatStreamline(std::map<std::string, std::vector<float> >& parVec) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GConstrainedFloatCollection::floatStreamline(std::map<std::string, std::vector<float> >& parVec) const: Error!" << std::endl
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
 * Attach boundaries of type float to the vectors.
 *
 * @param lBndVec A vector of lower float parameter boundaries
 * @param uBndVec A vector of upper float parameter boundaries
 */
void GConstrainedFloatCollection::floatBoundaries(
		std::vector<float>& lBndVec
		, std::vector<float>& uBndVec
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
 * Tell the audience that we own a number of float values
 *
 * @return The number of float parameters
 */
std::size_t GConstrainedFloatCollection::countFloatParameters() const {
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
void GConstrainedFloatCollection::assignFloatValueVector(const std::vector<float>& parVec, std::size_t& pos) {
	for(std::size_t i=0; i<this->size(); i++) {
#ifdef DEBUG
		  // Do we have a valid position ?
		  if(pos >= parVec.size()) {
		     glogger
		     << "In GConstrainedFloatCollection::assignFloatValueVector(const std::vector<float>&, std::size_t&):" << std::endl
           << "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
           << GEXCEPTION;
		  }
#endif

		  this->setValue(i, this->transfer(parVec[pos]));
		  pos++;
	}
}

/******************************************************************************/
/**
 * Loads the data of another GConstrainedFloatCollection object,
 * camouflaged as a GObject. We have no local data, so
 * all we need to do is to the standard identity check,
 * preventing that an object is assigned to itself.
 *
 * @param cp A copy of another GConstrainedFloatCollection object, camouflaged as a GObject
 */
void GConstrainedFloatCollection::load_(const GObject *cp){
	// Convert cp into local format
	// const GConstrainedFloatCollection *p_load = GObject::gobject_conversion<GConstrainedFloatCollection>(cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GConstrainedFloatCollection>(cp);

	// Load our parent class'es data ...
	GConstrainedFPNumCollectionT<float>::load_(cp);

	// no local data ...
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedFloatCollection::clone_() const {
	return new GConstrainedFloatCollection(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedFloatCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent classes' functions
	if(GConstrainedFPNumCollectionT<float>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFloatCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedFloatCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<float>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFloatCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedFloatCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<float>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GEM_TESTING::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GConstrainedFloatCollection has a private default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GConstrainedFloatCollection> TFactory_GUnitTests<Gem::Geneva::GConstrainedFloatCollection>() {
	const std::size_t NPARAMETERS = 100;
	float LOWERBOUNDARY = -10.;
	float UPPERBOUNDARY =  10.;
	boost::shared_ptr<Gem::Geneva::GConstrainedFloatCollection> p;
	BOOST_CHECK_NO_THROW(
			p= boost::shared_ptr<Gem::Geneva::GConstrainedFloatCollection>(
					new Gem::Geneva::GConstrainedFloatCollection(NPARAMETERS, LOWERBOUNDARY, UPPERBOUNDARY)
			)
	);
	return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */
