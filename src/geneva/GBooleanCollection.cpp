/**
 * @file GBooleanCollection.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "geneva/GBooleanCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanCollection)

namespace Gem
{
namespace Geneva
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
	  for(std::size_t i= 0; i<nval; i++) this->push_back(gr->uniform_bool());
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
	  for(std::size_t i= 0; i<nval; i++) this->push_back(gr->weighted_bool(probability));
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
	  for(std::size_t i=0; i<this->size(); i++) (*this)[i] = gr->uniform_bool();
  }

  /**********************************************************************/
  /**
   * Random initialization with a given probability structure
   *
   * @param probability The probability for true values in the collection
   */
  void GBooleanCollection::randomInit_(const double& probability) {
	  // Do some error checking
	  if(probability<0. || probability>1.) {
		  std::ostringstream error;
		  error << "In GBooleanCollection::randomInit_(" << probability << "): Error!" << std::endl
				<< "Requested probability outside of allowed range [0:1]" << std::endl;
		  throw(Gem::Common::gemfony_error_condition(error.str()));
	  }

	  for(std::size_t i=0; i<this->size(); i++) (*this)[i] = gr->weighted_bool(probability);
  }

  /**********************************************************************/
  /**
   * Random initialization. This is a helper function, without it we'd
   * have to say things like "myGBooleanCollectionObject.GParameterBase::randomInit();".
   */
  void GBooleanCollection::randomInit() {
	  GParameterBase::randomInit(); // This will also take into account the "blocked initialization" flag
  }

  /**********************************************************************/
  /**
   * Random initialization with a given probability structure,
   * if re-initialization has not been blocked.
   *
   * @param probability The probability for true values in the collection
   */
  void GBooleanCollection::randomInit(const double& probability) {
	  if(!GParameterBase::randomInitializationBlocked()) randomInit_(probability);
  }

  /**********************************************************************/
  /**
   * Checks for equality with another GBooleanCollection object
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @return A boolean indicating whether both objects are equal
   */
  bool GBooleanCollection::operator==(const GBooleanCollection& cp) const {
		using namespace Gem::Common;
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
		using namespace Gem::Common;
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
  boost::optional<std::string> GBooleanCollection::checkRelationshipWith (
		  const GObject& cp
  		, const Gem::Common::expectation& e
  		, const double& limit
  		, const std::string& caller
  		, const std::string& y_name
  		, const bool& withMessages
  ) const {
	  using namespace Gem::Common;

	  // Check for a possible self-assignment
	  GObject::selfAssignmentCheck<GBooleanCollection>(&cp);

	  // Will hold possible deviations from the expectation, including explanations
	  std::vector<boost::optional<std::string> > deviations;

	  // Check our parent class'es data ...
	  deviations.push_back(GParameterCollectionT<bool>::checkRelationshipWith(cp, e, limit, "GBooleanCollection", y_name, withMessages));

	  // no local data ...

	  return evaluateDiscrepancies("GBooleanCollection", caller, deviations, e);
  }

  /**********************************************************************/
  /**
   * Attach our local values to the vector. This is used to collect all parameters of this type
   * in the sequence in which they were registered.
   *
   * @param parVec The vector to which the local values should be attached
   */
  void GBooleanCollection::booleanStreamline(std::vector<bool>& parVec) const {
	  GBooleanCollection::const_iterator cit;
	  for(cit=this->begin(); cit!=this->end(); ++cit) {
		 parVec.push_back(*cit);
	  }
  }

  /**********************************************************************/
  /**
   * Tell the audience that we own a number of bool values
   *
   * @return The number of bool parameters
   */
  std::size_t GBooleanCollection::countBoolParameters() const {
	  return this->size();
  }

  /**********************************************************************/
  /**
   * Assigns part of a value vector to the parameter
   */
  void GBooleanCollection::assignBooleanValueVector(const std::vector<bool>& parVec, std::size_t& pos) {
	  for(GBooleanCollection::iterator it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
		  // Do we have a valid position ?
		  if(pos >= parVec.size()) {
			  std::ostringstream error;
			  error << "In GBooleanCollection::assignBooleanValueVector(const std::vector<bool>&, std::size_t&): Error!" << std::endl
					  << "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl;
			  throw(Gem::Common::gemfony_error_condition(error.str()));
		  }
#endif

		  (*it) = parVec[pos];
		  pos++;
	  }
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
	  // A few settings
	  const std::size_t nItems = 10000;
	  const bool FIXEDVALUEINIT = true;

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
	  GParameterCollectionT<bool>::specificTestsNoFailureExpected_GUnitTests();

	  //------------------------------------------------------------------------------

	  { // Test that random initialization with equal probability will result in roughly the same amount of true and false values
		  boost::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		  // Make sure the collection is empty
		  BOOST_CHECK_NO_THROW(p_test->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->push_back(true);
		  }

		  // Check the size
		  BOOST_CHECK(p_test->size() == nItems);

		  // Randomly initialize, using the internal function
		  BOOST_CHECK_NO_THROW(p_test->randomInit_());

		  // Count the number of true and false values
		  std::size_t nTrue = 0;
		  std::size_t nFalse = 0;
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->at(i)?nTrue++:nFalse++;
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

	  //------------------------------------------------------------------------------

	  { // Check that initialization with a probabilty of 0. for true results in just false values
		  boost::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		  // Make sure the collection is empty
		  BOOST_CHECK_NO_THROW(p_test->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->push_back(true);
		  }

		  // Randomly initialize, using the internal function
		  BOOST_CHECK_NO_THROW(p_test->randomInit_(0.));

		  // Count the number of true and false values
		  std::size_t nTrue = 0;
		  std::size_t nFalse = 0;
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->at(i)?nTrue++:nFalse++;
		  }

		  // Cross-check
		  BOOST_CHECK_MESSAGE(
				  nTrue == 0
				  ,  "\n"
				  << "nTrue = " << nTrue << "\n"
				  << "nFalse = " << nFalse << "\n"
		  );
	  }

	  //------------------------------------------------------------------------------

	  { // Check that initialization with a probabilty of 1. for true results in just true values
		  boost::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		  // Make sure the collection is empty
		  BOOST_CHECK_NO_THROW(p_test->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->push_back(false);
		  }

		  // Randomly initialize, using the internal function
		  BOOST_CHECK_NO_THROW(p_test->randomInit_(1.));

		  // Count the number of true and false values
		  std::size_t nTrue = 0;
		  std::size_t nFalse = 0;
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->at(i)?nTrue++:nFalse++;
		  }

		  // Cross-check
		  BOOST_CHECK_MESSAGE(
				  nTrue == nItems
				  ,  "\n"
				  << "nTrue = " << nTrue << "\n"
				  << "nFalse = " << nFalse << "\n"
		  );
	  }

	  //------------------------------------------------------------------------------

	  { // Test that random initialization with a given probability will result in roughly the expected amount of true and false values
		  for(double d=0.1; d<0.9; d+=0.1) {
			  boost::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

			  // Make sure the collection is empty
			  BOOST_CHECK_NO_THROW(p_test->clear());

			  // Add items of fixed value
			  for(std::size_t i=0; i<nItems; i++) {
				  p_test->push_back(false);
			  }

			  // Randomly initialize, using the internal function and the required probability
			  BOOST_CHECK_NO_THROW(p_test->randomInit_(d));

			  // Count the number of true and false values
			  std::size_t nTrue = 0;
			  std::size_t nFalse = 0;
			  for(std::size_t i=0; i<nItems; i++) {
				  p_test->at(i)?nTrue++:nFalse++;
			  }

			  // We allow a slight deviation, as the initialization is a random process
			  double expectedTrueMin = 0.8*d*nItems;
			  double expectedTrueMax = 1.2*d*nItems;

			  BOOST_CHECK_MESSAGE(
					  double(nTrue) > expectedTrueMin && double(nTrue) < expectedTrueMax
					  ,  "\n"
					  << "d = " << d << "\n"
					  << "Allowed window = " << expectedTrueMin << " - " << expectedTrueMax << "\n"
					  << "nItems = " << nItems << "\n"
					  << "nTrue = " << nTrue << "\n"
					  << "nFalse = " << nFalse << "\n"
			  );
		  }
	  }

	  //------------------------------------------------------------------------------

	  { // Check that random initialization can be blocked for equal distributions
		  boost::shared_ptr<GBooleanCollection> p_test1 = this->clone<GBooleanCollection>();
		  boost::shared_ptr<GBooleanCollection> p_test2 = this->clone<GBooleanCollection>();

		  // Make sure the collections are empty
		  BOOST_CHECK_NO_THROW(p_test1->clear());
		  BOOST_CHECK_NO_THROW(p_test2->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
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
		  BOOST_CHECK_NO_THROW(p_test1->randomInit());

		  // Check that both objects are still the same
		  BOOST_CHECK(*p_test1 == *p_test2);
	  }

	  //------------------------------------------------------------------------------

	  { // Check that random initialization can be blocked for distributions with a given probability structure
		  boost::shared_ptr<GBooleanCollection> p_test1 = this->clone<GBooleanCollection>();
		  boost::shared_ptr<GBooleanCollection> p_test2 = this->clone<GBooleanCollection>();

		  // Make sure the collections are empty
		  BOOST_CHECK_NO_THROW(p_test1->clear());
		  BOOST_CHECK_NO_THROW(p_test2->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
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
		  BOOST_CHECK_NO_THROW(p_test1->randomInit(0.7));

		  // Check that both objects are still the same
		  BOOST_CHECK(*p_test1 == *p_test2);
	  }

	  //------------------------------------------------------------------------------

	  { // Check that the fp-family of functions doesn't have an effect on this object
		  boost::shared_ptr<GBooleanCollection> p_test1 = this->GObject::clone<GBooleanCollection>();
		  boost::shared_ptr<GBooleanCollection> p_test2 = this->GObject::clone<GBooleanCollection>();
		  boost::shared_ptr<GBooleanCollection> p_test3 = this->GObject::clone<GBooleanCollection>();

		  // Add a few items to p_test1
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test1->push_back(FIXEDVALUEINIT);
		  }

		  // Load into p_test2 and p_test3 and test equality
		  BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		  BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
		  BOOST_CHECK(*p_test2 == *p_test1);
		  BOOST_CHECK(*p_test3 == *p_test1);
		  BOOST_CHECK(*p_test3 == *p_test2);

		  // Check that initialization with a fixed floating point value has no effect on this object
		  BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(2.));
		  BOOST_CHECK(*p_test2 == *p_test1);

		  // Check that multiplication with a fixed floating point value has no effect on this object
		  BOOST_CHECK_NO_THROW(p_test2->fpMultiplyBy(2.));
		  BOOST_CHECK(*p_test2 == *p_test1);

		  // Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
		  BOOST_CHECK_NO_THROW(p_test2->fpMultiplyByRandom(1., 2.));
		  BOOST_CHECK(*p_test2 == *p_test1);

		  // Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
		  BOOST_CHECK_NO_THROW(p_test2->fpMultiplyByRandom());
		  BOOST_CHECK(*p_test2 == *p_test1);

		  // Check that adding p_test1 to p_test3 does not have an effect
		  BOOST_CHECK_NO_THROW(p_test3->fpAdd(p_test1));
		  BOOST_CHECK(*p_test3 == *p_test2);

		  // Check that subtracting p_test1 from p_test3 does not have an effect
		  BOOST_CHECK_NO_THROW(p_test3->fpSubtract(p_test1));
		  BOOST_CHECK(*p_test3 == *p_test2);
	  }

	  //------------------------------------------------------------------------------

	  // Remove the test adaptor
	  this->resetAdaptor();

	  // Load the old adaptor, if needed
	  if(adaptorStored) {
		  this->addAdaptor(storedAdaptor);
	  }
  }

  /*****************************************************************************/
  /**
   * Performs self tests that are expected to fail. This is needed for testing purposes
   */
  void GBooleanCollection::specificTestsFailuresExpected_GUnitTests() {
	  // A few settings
	  std::size_t nItems = 10000;

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
	  GParameterCollectionT<bool>::specificTestsFailuresExpected_GUnitTests();

	  //------------------------------------------------------------------------------

	  { // Check that random initialization with a probability < 0. throws
		  boost::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		  // Make sure the collection is empty
		  BOOST_CHECK_NO_THROW(p_test->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->push_back(true);
		  }

		  // Randomly initialize, using the internal function
		  BOOST_CHECK_THROW(p_test->randomInit_(-1.), Gem::Common::gemfony_error_condition);
	  }

	  //------------------------------------------------------------------------------

	  { // Check that random initialization with a probability > 1. throws
		  boost::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

		  // Make sure the collection is empty
		  BOOST_CHECK_NO_THROW(p_test->clear());

		  // Add items of fixed value
		  for(std::size_t i=0; i<nItems; i++) {
			  p_test->push_back(true);
		  }

		  // Randomly initialize, using the internal function
		  BOOST_CHECK_THROW(p_test->randomInit_(2.), Gem::Common::gemfony_error_condition);
	  }

	  //------------------------------------------------------------------------------

	  // Remove the test adaptor
	  this->resetAdaptor();

	  // Load the old adaptor, if needed
	  if(adaptorStored) {
		  this->addAdaptor(storedAdaptor);
	  }
  }

  /*****************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
