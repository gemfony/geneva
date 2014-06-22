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
GInt32Collection::GInt32Collection()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GInt32Collection::GInt32Collection(
		const std::size_t& nval
		, const boost::int32_t& min
		, const boost::int32_t& max
)
	: GIntNumCollectionT<boost::int32_t>(nval, min, max)
{ /* nothing */ }

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
		const std::size_t& nval
		, const boost::int32_t& val
		, const boost::int32_t& min
		, const boost::int32_t& max
)
	: GIntNumCollectionT<boost::int32_t>(nval, val, min, max)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32Collection object
 */
GInt32Collection::GInt32Collection(const GInt32Collection& cp)
	: GIntNumCollectionT<boost::int32_t>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GInt32Collection::~GInt32Collection()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32Collection object
 * @return A constant reference to this object
 */
const GInt32Collection& GInt32Collection::operator=(const GInt32Collection& cp){
	GInt32Collection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32Collection::clone_() const {
	return new GInt32Collection(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GInt32Collection object
 *
 * @param  cp A constant reference to another GInt32Collection object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32Collection::operator==(const GInt32Collection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInt32Collection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GInt32Collection object
 *
 * @param  cp A constant reference to another GInt32Collection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32Collection::operator!=(const GInt32Collection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInt32Collection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GInt32Collection::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Collection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GIntNumCollectionT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GInt32Collection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32Collection", caller, deviations, e);
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
void GInt32Collection::int32Streamline(std::vector<boost::int32_t>& parVec) const {
	GInt32Collection::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
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
void GInt32Collection::int32Streamline(std::map<std::string, std::vector<boost::int32_t> >& parVec) const {
#ifdef DEBUG
   if((this->getParameterName()).empty()) {
      glogger
      << "In GInt32Collection::int32Streamline(std::map<std::string, std::vector<boost::int32_t> >& parVec) const: Error!" << std::endl
      << "No name was assigned to the object" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   std::vector<boost::int32_t> parameters;
   this->int32Streamline(parameters);
   parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type boost::int32_t to the vectors. Since this is an unbounded type,
 * we use the initialization boundaries as a replacement.
 *
 * @param lBndVec A vector of lower boost::int32_t parameter boundaries
 * @param uBndVec A vector of upper boost::int32_t parameter boundaries
 */
void GInt32Collection::int32Boundaries(
		std::vector<boost::int32_t>& lBndVec
		, std::vector<boost::int32_t>& uBndVec
) const {
	// Add as man lower and upper boundaries to the vector as
	// there are variables
	GInt32Collection::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		lBndVec.push_back(this->getLowerInitBoundary());
		uBndVec.push_back(this->getUpperInitBoundary());
	}
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of boost::int32_t values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of boost::int32_t parameters
 */
std::size_t GInt32Collection::countInt32Parameters(
   const activityMode& am
) const {
   switch(am) {
      case ACTIVEONLY:
      {
         if(this->adaptionsActive()) {
            return this->size();
         } else {
            return 0;
         }
      }
      break;

      case ALLPARAMETERS:
      {
         return this->size();
      }
      break;

      case INACTIVEONLY:
      {
         if(this->adaptionsInactive()) {
            return this->size();
         } else {
            return 0;
         }
      }
      break;

      default:
      {
         glogger
         << "In GInt32Collection::countFloatParameters(): Error!" << std::endl
         << "Got invalid activity mode " << am << std::endl
         << GEXCEPTION;
      }
      break;
   }

   glogger
   << "In GInt32Collection::countFloatParameters(): Error!" << std::endl
   << "This line should never be reached" << std::endl;

   // Make the compiler happy
   return 0;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GInt32Collection::assignInt32ValueVector(const std::vector<boost::int32_t>& parVec, std::size_t& pos) {
	  for(GInt32Collection::iterator it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
		  // Do we have a valid position ?
		  if(pos >= parVec.size()) {
		     glogger
		     << "In GInt32Collection::assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&):" << std::endl
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
void GInt32Collection::assignInt32ValueVectors(const std::map<std::string, std::vector<boost::int32_t> >& parMap) {
   GInt32Collection::iterator it;
   std::size_t cnt = 0;
   for(it=this->begin(); it!=this->end(); ++it) {
      *it = (Gem::Common::getMapItem(parMap,this->getParameterName())).at(cnt++);
   }
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Collection object, camouflaged as a GObject
 */
void GInt32Collection::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Collection>(cp);

	// Load our parent class'es data ...
	GIntNumCollectionT<boost::int32_t>::load_(cp);

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
	if(GIntNumCollectionT<boost::int32_t>::modify_GUnitTests()) result = true;

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
	boost::shared_ptr<GAdaptorT<boost::int32_t> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GIntNumCollectionT<boost::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	// no local data, nothing to test

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
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
	boost::shared_ptr<GAdaptorT<boost::int32_t> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GIntNumCollectionT<boost::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// no local data, nothing to test

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32Collection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
