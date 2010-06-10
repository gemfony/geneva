/**
 * @file GBooleanCollection.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GBooleanCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBooleanCollection)

namespace Gem
{
namespace GenEvA
{
  /**********************************************************************/
  /**
   * The standard constructor. As we have no local data, all work is done
   * by the parent class.
   */
  GBooleanCollection::GBooleanCollection()
	: GParameterCollectionT<bool>()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Initializes the class with a set of nval random bits.
   *
   * @param nval The size of the collection
   */
  GBooleanCollection::GBooleanCollection(const std::size_t& nval)
    : GParameterCollectionT<bool>()
  {
	Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
	for(std::size_t i= 0; i<nval; i++) this->push_back(gr.boolRandom());
  }

  /**********************************************************************/
  /**
   * Initializes the class with nval random bits, of which probability percent
   * have the value true
   *
   * @param nval The size of the collection
   * @param probability The probability for true values in the collection
   */
  GBooleanCollection::GBooleanCollection(const std::size_t& nval, const double& probability)
	: GParameterCollectionT<bool>()
  {
	  Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
	  for(std::size_t i= 0; i<nval; i++) this->push_back(gr.boolRandom(probability));
  }

  /**********************************************************************/
  /**
   * No local data, hence we can rely on the parent class.
   *
   * @param cp A copy of another GBooleanCollection object
   */
  GBooleanCollection::GBooleanCollection(const GBooleanCollection& cp)
    : GParameterCollectionT<bool>(cp)
  { /* nothing */ }

  /**********************************************************************/
  /**
   * The standard destructor. No local data, hence it is empty.
   */
  GBooleanCollection::~GBooleanCollection()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * A standard assignment operator for  GBooleanCollection objects.
   *
   * @param cp A copy of another GBooleanCollection object
   */
  const GBooleanCollection& GBooleanCollection::operator=(const GBooleanCollection& cp){
    GBooleanCollection::load_(&cp);
    return *this;
  }

  /**********************************************************************/
  /**
   * Creates a deep clone of this object
   *
   * @return A deep clone of this object
   */
  GObject *GBooleanCollection::clone_() const{
	  return new GBooleanCollection(*this);
  }

  /**********************************************************************/
  /**
   * Loads the data of another GBooleanCollection object, camouflaged as
   * a GObject.
   *
   * @param gb A pointer to another GBooleanCollection object, camouflaged as a GObject
   */
  void GBooleanCollection::load_(const GObject * cp){
	// Check for a possible self-assignment
	GObject::selfAssignmentCheck<GBooleanCollection>(cp);

    GParameterCollectionT<bool>::load_(cp);
  }

  /**********************************************************************/
  /**
   * Triggers random initialization of the parameter collection. Note that this
   * function assumes that the collection has been completely set up. Data
   * that is added later will remain unaffected.
   */
  void GBooleanCollection::randomInit_() {
	  Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
	  for(std::size_t i=0; i<this->size(); i++) (*this)[i] = gr.boolRandom();
  }

  /**********************************************************************/
  /**
   * Random initialization with a given probability structure
   *
   * @param probability The probability for true values in the collection
   */
  void GBooleanCollection::randomInit_(const double& probability) {
	  Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
	  for(std::size_t i=0; i<this->size(); i++) (*this)[i] = gr.boolRandom(probability);
  }

  /**********************************************************************/
  /**
   * Random initialization. This is a helper function, without it we'd
   * have to say things like "myGBooleanCollectionObject.GParameterBase::randomInit();".
   */
  void GBooleanCollection::randomInit() {
	  GParameterBase::randomInit();
  }

  /**********************************************************************/
  /**
   * Random initialization with a given probability structure,
   * if re-initialization has not been blocked.
   *
   * @param probability The probability for true values in the collection
   */
  void GBooleanCollection::randomInit(const double& probability) {
	  if(!GParameterBase::initializationBlocked()) randomInit_(probability);
  }

  /**********************************************************************/
  /**
   * Checks for equality with another GBooleanCollection object
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @return A boolean indicating whether both objects are equal
   */
  bool GBooleanCollection::operator==(const GBooleanCollection& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBooleanCollection::operator==","cp", CE_SILENT);
  }

  /**********************************************************************/
  /**
   * Checks for inequality with another GBooleanCollection object
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @return A boolean indicating whether both objects are inequal
   */
  bool GBooleanCollection::operator!=(const GBooleanCollection& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBooleanCollection::operator!=","cp", CE_SILENT);
  }

  /**********************************************************************/
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
  boost::optional<std::string> GBooleanCollection::checkRelationshipWith(const GObject& cp,
  		const Gem::Util::expectation& e,
  		const double& limit,
  		const std::string& caller,
  		const std::string& y_name,
  		const bool& withMessages) const
  {
	  using namespace Gem::Util;
	  using namespace Gem::Util::POD;

	// Check for a possible self-assignment
	GObject::selfAssignmentCheck<GBooleanCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	  std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterCollectionT<bool>::checkRelationshipWith(cp, e, limit, "GBooleanCollection", y_name, withMessages));

	// no local data ...

	return POD::evaluateDiscrepancies("GBooleanCollection", caller, deviations, e);
  }

#ifdef GENEVATESTING

  /**********************************************************************/
  /**
   * Applies modifications to this object. This is needed for testing purposes
   *
   * @return A boolean which indicates whether modifications were made
   */
  bool GBooleanCollection::modify_GUnitTests() {
  	bool result = false;

  	// Call the parent class'es function
  	if(GParameterCollectionT<bool>::modify_GUnitTests()) result = true;

  	return result;
  }

  /*****************************************************************************/
  /**
   * Performs self tests that are expected to succeed. This is needed for testing purposes
   */
  void GBooleanCollection::specificTestsNoFailureExpected_GUnitTests() {
  	// Call the parent class'es function
	  GParameterCollectionT<bool>::specificTestsNoFailureExpected_GUnitTests();
  }

  /*****************************************************************************/
  /**
   * Performs self tests that are expected to fail. This is needed for testing purposes
   */
  void GBooleanCollection::specificTestsFailuresExpected_GUnitTests() {
  	// Call the parent class'es function
	  GParameterCollectionT<bool>::specificTestsFailuresExpected_GUnitTests();
  }

  /*****************************************************************************/

#endif /* GENEVATESTING */

} /* namespace GenEvA */
} /* namespace Gem */
