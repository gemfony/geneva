/**
 * @file GTestIndividual3.cpp
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

#include "geneva-individuals/GTestIndividual3.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual3)

namespace Gem
{
namespace Tests
{

const std::size_t GTI_DEF_NITEMS = 300;
const double GTI_DEF_SIGMA = 0.5;
const double GTI_DEF_SIGMASIGMA = 0.8;
const double GTI_DEF_MINSIGMA = 0.02;
const double GTI_DEF_MAXSIGMA = 1.0;
const double GTI_DEF_ADPROB = 0.05;

/******************************************************************************/
/**
 * The default constructor
 */
GTestIndividual3::GTestIndividual3() : GParameterSet()
{
   using namespace Gem::Geneva;

   /////////////////////////////////////////////////////////////////////////////
   // Create suitable adaptors

   // Gaussian distributed random numbers
   boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr_tmpl(
         new GDoubleGaussAdaptor(
               GTI_DEF_SIGMA
               , GTI_DEF_SIGMASIGMA
               , GTI_DEF_MINSIGMA
               , GTI_DEF_MAXSIGMA
         )
   );
   gdga_ptr_tmpl->setAdaptionProbability(GTI_DEF_ADPROB);

   /////////////////////////////////////////////////////////////////////////////
   // Set up a hierarchical data structure

   // Create one GParameterObjectCollection for each data item
   for(std::size_t i_cnt=0; i_cnt<GTI_DEF_NITEMS; i_cnt++) {
      boost::shared_ptr<GParameterObjectCollection> gpoc_ptr(new GParameterObjectCollection());

      //--------------------------------------------------------------------------------------------
      boost::shared_ptr<GConstrainedDoubleCollection> a_ptr(new GConstrainedDoubleCollection(2, 0., 1.));
      a_ptr->addAdaptor(gdga_ptr_tmpl);
      gpoc_ptr->push_back(a_ptr);

      //--------------------------------------------------------------------------------------------
      boost::shared_ptr<GConstrainedDoubleObject> b_ptr(new GConstrainedDoubleObject(0., 0.3));
      b_ptr->addAdaptor(gdga_ptr_tmpl);
      gpoc_ptr->push_back(b_ptr);

      //--------------------------------------------------------------------------------------------
      boost::shared_ptr<GConstrainedDoubleCollection> c_ptr(new GConstrainedDoubleCollection(3, 0., 1.));
      c_ptr->addAdaptor(gdga_ptr_tmpl);
      gpoc_ptr->push_back(c_ptr);

      //--------------------------------------------------------------------------------------------
      boost::shared_ptr<GConstrainedDoubleCollection> d_ptr(new GConstrainedDoubleCollection(3, 0., 1.));
      d_ptr->addAdaptor(gdga_ptr_tmpl);
      gpoc_ptr->push_back(d_ptr);

      //--------------------------------------------------------------------------------------------
      boost::shared_ptr<GConstrainedDoubleObject> e_ptr(new GConstrainedDoubleObject(0.3, 0.6));
      e_ptr->addAdaptor(gdga_ptr_tmpl);
      gpoc_ptr->push_back(e_ptr);

      //--------------------------------------------------------------------------------------------

      // Finally add the collection to the individual
      this->push_back(gpoc_ptr);
   }
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual3 object
 */
GTestIndividual3::GTestIndividual3(const GTestIndividual3& cp)
: Gem::Geneva::GParameterSet(cp)
{	/* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual3::~GTestIndividual3()
{ /* nothing */	}

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GTestIndividual3 object
 * @return A constant reference to this object
 */
const GTestIndividual3& GTestIndividual3::operator=(const GTestIndividual3& cp){
	GTestIndividual3::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GTestIndividual3 object
 *
 * @param  cp A constant reference to another GTestIndividual3 object
 * @return A boolean indicating whether both objects are equal
 */
bool GTestIndividual3::operator==(const GTestIndividual3& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GTestIndividual3::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GTestIndividual3 object
 *
 * @param  cp A constant reference to another GTestIndividual3 object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GTestIndividual3::operator!=(const GTestIndividual3& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GTestIndividual3::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GTestIndividual3::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GParamterBase reference
	const GTestIndividual3 *p_load = gobject_conversion<GTestIndividual3>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(Gem::Geneva::GParameterSet::checkRelationshipWith(cp, e, limit, "GTestIndividual3", y_name, withMessages));

	// no local data

	return evaluateDiscrepancies("GTestIndividual3", caller, deviations, e);
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual3, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual3, camouflaged as a GObject
 */
void GTestIndividual3::load_(const GObject* cp)
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GTestIndividual3 object
	const GTestIndividual3 *p_load = gobject_conversion<GTestIndividual3>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GTestIndividual3::clone_() const {
	return new GTestIndividual3(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual3::fitnessCalculation(){
	double result = 0.;

	// We just calculate the square of all double values
	std::vector<double> parVec;
	this->streamline(parVec);

	// Calculate the value of the parabola
	for(std::size_t i=0; i<parVec.size(); i++) {
		result += GSQUARED(parVec[i]);
	}

	return result;
}

/******************************************************************************/
/**
 * Get all data members of this class as a plain array
 */
boost::shared_array<float> GTestIndividual3::getPlainData() const {
   using namespace Gem::Geneva;

#ifdef DEBUG
   if(this->size() != GTI_DEF_NITEMS) {
      raiseException(
         "In GTestIndividual3::getPlainData(): Error!" << std::endl
         << "Invalid number of entries in this class " << this->size() << " / " << GTI_DEF_NITEMS << std::endl
      );
   }
#endif /* DEBUG */

   boost::shared_array<float> result(new float[10*GTI_DEF_NITEMS]);
   for(std::size_t i=0; i<GTI_DEF_NITEMS; i++) {
      boost::shared_ptr<GParameterObjectCollection> gpoc_ptr = this->at<GParameterObjectCollection>(i);

      //---------------------------------------------------------
      // Extract the data of the middle of the circle
      boost::shared_ptr<GConstrainedDoubleCollection> a_ptr = gpoc_ptr->at<GConstrainedDoubleCollection>(0);
      result[i*10+0] = boost::numeric_cast<float>(a_ptr->at(0));
      result[i*10+1] = boost::numeric_cast<float>(a_ptr->at(1));

      //---------------------------------------------------------
      // Extract the radius of the circle
      boost::shared_ptr<GConstrainedDoubleObject> b_ptr = gpoc_ptr->at<GConstrainedDoubleObject>(1);
      result[i*10+2] = boost::numeric_cast<float>(b_ptr->value());

      //---------------------------------------------------------
      // Extract the three angles
      boost::shared_ptr<GConstrainedDoubleCollection> c_ptr = gpoc_ptr->at<GConstrainedDoubleCollection>(2);
      result[i*10+3] = boost::numeric_cast<float>(c_ptr->at(0));
      result[i*10+4] = boost::numeric_cast<float>(c_ptr->at(1));
      result[i*10+5] = boost::numeric_cast<float>(c_ptr->at(2));

      //---------------------------------------------------------
      // Extract the three colors
      boost::shared_ptr<GConstrainedDoubleCollection> d_ptr = gpoc_ptr->at<GConstrainedDoubleCollection>(3);
      result[i*10+6]  = boost::numeric_cast<float>(d_ptr->at(0));
      result[i*10+7]  = boost::numeric_cast<float>(d_ptr->at(1));
      result[i*10+8]  = boost::numeric_cast<float>(d_ptr->at(2));

      //---------------------------------------------------------
      // Extract the alpha channel
      boost::shared_ptr<GConstrainedDoubleObject> e_ptr = gpoc_ptr->at<GConstrainedDoubleObject>(4);
      result[i*10+9] = boost::numeric_cast<float>(e_ptr->value());

      //---------------------------------------------------------
   }

   // Let the audience know
   return result;
}

#ifdef GEM_TESTING

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual3::modify_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

	// Change the parameter settings
	result = true;

	return result;
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual3::specificTestsNoFailureExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	const std::size_t NTESTS = 100;

	//------------------------------------------------------------------------------

	{ // Test that repeated extraction of an object's data results in the same output
      boost::shared_ptr<GTestIndividual3> p;
      boost::shared_array<float> result_old, result_new;

      BOOST_CHECK_NO_THROW(p = boost::shared_ptr<GTestIndividual3>(new GTestIndividual3()));
      BOOST_CHECK_NO_THROW(result_old = p->getPlainData());
      for(std::size_t i=0; i<NTESTS; i++) {
         BOOST_CHECK_NO_THROW(result_new = p->getPlainData());
         for(std::size_t m=0; m<GTI_DEF_NITEMS*10; i++) {
            BOOST_CHECK(result_old[i] == result_new[i]);
         }
      }
	}

	//------------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual3::specificTestsFailuresExpected_GUnitTests() {
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
}

/******************************************************************************/

#endif /* GEM_TESTING */

} /* namespace Tests */
} /* namespace Gem */
