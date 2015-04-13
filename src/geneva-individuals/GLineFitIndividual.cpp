/**
 * @file GLineFitIndividual.cpp
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

#include "geneva-individuals/GLineFitIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GLineFitIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor -- private, as it is only needed for (de-)serialization purposes
 */
GLineFitIndividual::GLineFitIndividual() : GParameterSet()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard constructor, sets up the internal data structures
 *
 * @param nObjects The number of parameters to be added to this individual
 */
GLineFitIndividual::GLineFitIndividual(
   const std::vector<boost::tuple<double, double> >& dataPoints
)
   : GParameterSet()
   , dataPoints_(dataPoints)
{
	using namespace Gem::Geneva;

   for(std::size_t i=0; i<2; i++) {
      boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject());
      boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0.0001, 0.4, 1.)); // sigma, sigmaSigma, minSigma, maxSigma, adProb
      gdo_ptr->addAdaptor(gdga_ptr);
      this->push_back(gdo_ptr);
   }
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GLineFitIndividual object
 */
GLineFitIndividual::GLineFitIndividual(const GLineFitIndividual& cp)
   : Gem::Geneva::GParameterSet(cp)
   , dataPoints_(cp.dataPoints_)
{	/* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GLineFitIndividual::~GLineFitIndividual()
{ /* nothing */	}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GLineFitIndividual& GLineFitIndividual::operator=(const GLineFitIndividual& cp) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GLineFitIndividual object
 *
 * @param  cp A constant reference to another GLineFitIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GLineFitIndividual::operator==(const GLineFitIndividual& cp) const {
   using namespace Gem::Common;
   try {
      this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
      return true;
   } catch(g_expectation_violation&) {
      return false;
   }
}

/******************************************************************************/
/**
 * Checks for inequality with another GLineFitIndividual object
 *
 * @param  cp A constant reference to another GLineFitIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GLineFitIndividual::operator!=(const GLineFitIndividual& cp) const {
   using namespace Gem::Common;
   try {
      this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
      return true;
   } catch(g_expectation_violation&) {
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
void GLineFitIndividual::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GLineFitIndividual *p_load = GObject::gobject_conversion<GLineFitIndividual>(&cp);

   GToken token("GLineFitIndividual", e);

   // Compare our parent data ...
   Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

   // ... and then the local data
   compare_t(IDENTITY(dataPoints_, p_load->dataPoints_), token);

   // React on deviations from the expectation
   token.evaluate();
}

/******************************************************************************/
/**
 * Retrieves the tuple (a,b) of the line represented by this object
 */
boost::tuple<double, double> GLineFitIndividual::getLine() const {
   std::vector<double> parVec;
   this->streamline(parVec);
   return boost::tuple<double,double>(parVec.at(0), parVec.at(1));
}

/******************************************************************************/
/**
 * Loads the data of another GLineFitIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GLineFitIndividual, camouflaged as a GObject
 */
void GLineFitIndividual::load_(const GObject* cp)
{
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GLineFitIndividual object
	const GLineFitIndividual *p_load = gobject_conversion<GLineFitIndividual>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// and then our local data
	dataPoints_ = p_load->dataPoints_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GLineFitIndividual::clone_() const {
	return new GLineFitIndividual(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GLineFitIndividual::fitnessCalculation() {
	double result = 0.;

	// We just calculate the square of all double values
	std::vector<double> parVec;
	this->streamline(parVec);

	double a = parVec.at(0);
	double b = parVec.at(1);

	// Sum up the square deviation of line and data points
	double deviation = 0.;
	std::vector<boost::tuple<double, double> >::iterator it;
	for(it=dataPoints_.begin(); it!=dataPoints_.end(); ++it) {
	   deviation = (a+b*boost::get<0>(*it)) - boost::get<1>(*it);
		result += GSQUARED(deviation);
	}

	return sqrt(result);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GLineFitIndividual::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if(Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

	// Change the parameter settings
	result = true;

	return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GLineFitIndividual::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GLineFitIndividual::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

   // Call the parent classes' functions
   Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GLineFitIndividual::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GLineFitIndividual::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GLineFitIndividual::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization through a config file
 *
 * @param configFile The name of the configuration file
 */
GLineFitIndividualFactory::GLineFitIndividualFactory(
   const std::vector<boost::tuple<double, double> >& dataPoints
   , const std::string& configFile
)
   : Gem::Common::GFactoryT<GParameterSet>(configFile)
   , dataPoints_(dataPoints)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GLineFitIndividualFactory::~GLineFitIndividualFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GParameterSet> GLineFitIndividualFactory::getObject_(
   Gem::Common::GParserBuilder& gpb
   , const std::size_t& id
) {
   // Will hold the result
   boost::shared_ptr<GLineFitIndividual> target(new GLineFitIndividual(this->dataPoints_));

   // Make the object's local configuration options known
   target->addConfigurationOptions(gpb);

   return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GLineFitIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
   // Describe our own options
   using namespace Gem::Courtier;

   std::string comment;

   // No local options

   // Allow our parent class to describe its options
   Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we will usually add the parameter objects here. Note that a very similar constructor
 * exists for GLineFitIndividual, so it may be used independently of the factory.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GLineFitIndividualFactory::postProcess_(boost::shared_ptr<GParameterSet>& p_base) {
   // Convert the base pointer to our local type
   boost::shared_ptr<GLineFitIndividual> p
      = Gem::Common::convertSmartPointer<GParameterSet, GLineFitIndividual>(p_base);

   // Nothing to be done here
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
