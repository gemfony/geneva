/**
 * @file GStarterIndividual.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "GStarterIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GStarterIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Puts a Gem::Geneva::targetFunction item into a stream
 *
 * @param o The ostream the item should be added to
 * @param tF the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::targetFunction& tF) {
  Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(tF);
  o << tmp;
  return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::targetFunction item from a stream
 *
 * @param i The stream the item should be read from
 * @param tF The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::targetFunction& tF) {
  Gem::Common::ENUMBASETYPE tmp;
  i >> tmp;

#ifdef DEBUG
  tF = boost::numeric_cast<Gem::Geneva::targetFunction>(tmp);
#else
  tF = static_cast<Gem::Geneva::targetFunction>(tmp);
#endif /* DEBUG */

  return i;
}

/******************************************************************************/
/**
 * The default constructor -- intentionally private. Note that some data members
 * may be initialized in the class body.
 */
GStarterIndividual::GStarterIndividual() : GParameterSet() { /* nothing */
}

/******************************************************************************/
/**
 * The standard constructor. The number of parameters is determined using the number of
 * entries in the startValues vector. Note that all vector arguments need to have the
 * same dimension.
 */
GStarterIndividual::GStarterIndividual(const std::size_t& prod_id,
                                       const std::vector<double>& startValues,
                                       const std::vector<double>& lowerBoundaries,
                                       const std::vector<double>& upperBoundaries,
                                       const double& sigma, const double& sigmaSigma,
                                       const double& minSigma, const double& maxSigma,
                                       const double& adProb)
    : GParameterSet(), m_targetFunction(targetFunction::PARABOLA) {
  try {
    // The following is a static function used both here
    // and in the factory, so setup code cannot diverge
    GStarterIndividual::addContent(*this, prod_id, startValues, lowerBoundaries, upperBoundaries,
                                   sigma, sigmaSigma, minSigma, maxSigma, adProb);
  } catch (const gemfony_exception& e) {
    glogger << e.what() << GTERMINATION;
  } catch (...) {
    glogger << "Unknown exception caught" << std::endl << GTERMINATION;
  }
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GFunctionIndidivual
 */
GStarterIndividual::GStarterIndividual(const GStarterIndividual& cp)
    : GParameterSet(cp), m_targetFunction(cp.m_targetFunction) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GStarterIndividual::~GStarterIndividual() { /* nothing */
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
void GStarterIndividual::compare_(const GObject& cp, const Gem::Common::expectation& e,
                                  const double& limit) const {
  using namespace Gem::Common;

  // Check that we are dealing with a GStarterIndividual reference independent of this object and
  // convert the pointer
  const GStarterIndividual* p_load =
      Gem::Common::g_convert_and_compare<GObject, GStarterIndividual>(&cp, this);

  Gem::Common::GToken token("GStarterIndividual", e);

  // Compare our parent data ...
  Gem::Common::compare_base_t<Gem::Geneva::GParameterSet>(*this, *p_load, token);

  // ... and then the local data
  Gem::Common::compare_t(IDENTITY(m_targetFunction, p_load->m_targetFunction), token);

  // React on deviations from the expectation
  token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GStarterIndividual::addConfigurationOptions(Gem::Common::GParserBuilder& gpb) {
  // Call our parent class'es function
  GParameterSet::addConfigurationOptions(gpb);

  // Add local data. We use C++11 lambda expressions to
  // specify the function to be called for setting the
  // target functions. An alternative would be bind expressions.
  gpb.registerFileParameter<targetFunction>(
      "targetFunction"  // The name of the variable
      ,
      GO_DEF_TARGETFUNCTION  // The default value
      ,
      [this](targetFunction tF) { this->setTargetFunction(tF); })
      << "Specifies which target function should be used:" << std::endl
      << "0: Parabola" << std::endl
      << "1: Berlich";
}

/*******************************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param tF The id if the demo function
 */
void GStarterIndividual::setTargetFunction(targetFunction tF) { m_targetFunction = tF; }

/*******************************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
targetFunction GStarterIndividual::getTargetFunction() const { return m_targetFunction; }

/*******************************************************************************************/
/**
 * Retrieves the average value of all sigmas used in Gauss adaptors.
 *
 * @return The average value of sigma used in Gauss adaptors
 */
double GStarterIndividual::getAverageSigma() const {
  std::vector<double> sigmas;

  // Loop over all parameter objects
  for (std::size_t i = 0; i < this->size(); i++) {
    // Extract the parameter object
    std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr = this->at<GConstrainedDoubleObject>(i);

    // Extract the adaptor
    std::shared_ptr<GDoubleGaussAdaptor> adaptor_ptr = gcdo_ptr->getAdaptor<GDoubleGaussAdaptor>();

    // Extract the sigma value
    sigmas.push_back(adaptor_ptr->getSigma());
  }

  // Return the average
  return Gem::Common::GMean(sigmas);
}

/******************************************************************************/
/**
 * Emit information about this individual
 */
std::string GStarterIndividual::print() {
  std::ostringstream result;

  // Retrieve the parameters
  std::vector<double> parVec;
  this->streamline(parVec);

  result << "GStarterIndividual with target function "
         << (m_targetFunction == targetFunction::PARABOLA ? " PARABOLA" : " NOISY PARABOLA")
         << std::endl
         << "and raw fitness " << this->raw_fitness(0)
         << " has the following parameter values:" << std::endl;

  for (std::size_t i = 0; i < parVec.size(); i++) {
    result << i << ": " << parVec.at(i) << std::endl;
  }
  result << "The average sigma of this individual is " << this->getAverageSigma() << std::endl;

  return result.str();
}

/******************************************************************************/
/**
 * Loads the data of another GStarterIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GStarterIndividual, camouflaged as a GObject
 */
void GStarterIndividual::load_(const GObject* cp) {
  // Check that we are dealing with a GStarterIndividual reference independent of this object and
  // convert the pointer
  const GStarterIndividual* p_load =
      Gem::Common::g_convert_and_compare<GObject, GStarterIndividual>(cp, this);

  // Load our parent class'es data ...
  GParameterSet::load_(cp);

  // ... and then our local data
  m_targetFunction = p_load->m_targetFunction;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GStarterIndividual::clone_() const { return new GStarterIndividual(*this); }

/******************************************************************************/
/**
 * The actual value calculation takes place here
 *
 * @param The id of the target function (ignored here)
 * @return The value of this object, as calculated with the evaluation function
 */
double GStarterIndividual::fitnessCalculation() {
  // Retrieve the parameters
  std::vector<double> parVec;
  this->streamline(parVec);

  // Perform the actual calculation
  switch (m_targetFunction) {
    //-----------------------------------------------------------
    // A simple, multi-dimensional parabola
    case targetFunction::PARABOLA:
      return parabola(parVec);
      break;

      //-----------------------------------------------------------
      // A "noisy" parabola, i.e. a parabola with a very large
      // number of overlaid local optima
    case targetFunction::NOISYPARABOLA:
      return noisyParabola(parVec);
      break;
      //-----------------------------------------------------------
  };

  // Make the compiler happy
  return 0.;
}

/******************************************************************************/
/**
 * A simple n-dimensional parabola
 */
double GStarterIndividual::parabola(const std::vector<double>& parVec) const {
  double result = 0.;

  std::vector<double>::const_iterator cit;
  for (cit = parVec.begin(); cit != parVec.end(); ++cit) {
    result += (*cit) * (*cit);
  }

  return result;
}

/******************************************************************************/
/**
 * A "noisy" parabola
 */
double GStarterIndividual::noisyParabola(const std::vector<double>& parVec) const {
  double xsquared = 0.;

  std::vector<double>::const_iterator cit;
  for (cit = parVec.begin(); cit != parVec.end(); ++cit) {
    xsquared += (*cit) * (*cit);
  }

  return (cos(xsquared) + 2.) * xsquared;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This function is only useful
 * if you wish to run unit tests with your individual.
 *
 * @return A boolean indicating whether
 */
bool GStarterIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING
  using boost::unit_test_framework::test_case;
  using boost::unit_test_framework::test_suite;

  bool result = false;

  // Call the parent classes' functions
  if (Gem::Geneva::GParameterSet::modify_GUnitTests_()) result = true;

  // Change the parameter settings
  if (!this->empty()) {
    this->adapt();
    result = true;
  }

  // Let the audience know whether we have changed the content
  return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
  Gem::Common::condnotset("GStarterIndividual::modify_GUnitTests", "GEM_TESTING");
  return false;
#endif                   /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This function is only useful
 * if you wish to run unit tests with your individual.
 */
void GStarterIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
  using namespace Gem::Geneva;

  using boost::unit_test_framework::test_case;
  using boost::unit_test_framework::test_suite;

  // Call the parent classes' functions
  Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests_();

  //------------------------------------------------------------------------------

  {
    const std::size_t NENTRIES = 100;
    double DEFAULTSIGMA = 0.025;

    // Check standard construction and whether calculation of the average sigma works
    std::vector<double> startValues, lowerBoundaries, upperBoundaries;

    for (std::size_t n = 0; n < NENTRIES; n++) {
      startValues.push_back(1.);
      lowerBoundaries.push_back(0.);
      upperBoundaries.push_back(2.);
    }

    std::shared_ptr<GStarterIndividual> p_test;
    BOOST_CHECK_NO_THROW(
        p_test = std::shared_ptr<GStarterIndividual>(new GStarterIndividual(
            0  // indicates the first individual
            ,
            startValues, lowerBoundaries, upperBoundaries, DEFAULTSIGMA, 0.6, 0.001, 2., 0.05)));

    BOOST_CHECK_CLOSE(DEFAULTSIGMA, p_test->getAverageSigma(), 0.001);  // Should be similar
  }

  //------------------------------------------------------------------------------

  {  // Test setting and retrieval of the target function valie
    std::shared_ptr<GStarterIndividual> p_test = this->clone<GStarterIndividual>();

    BOOST_CHECK_NO_THROW(p_test->setTargetFunction(targetFunction::PARABOLA));
    BOOST_CHECK(targetFunction::PARABOLA == p_test->getTargetFunction());

    BOOST_CHECK_NO_THROW(p_test->setTargetFunction(targetFunction::NOISYPARABOLA));
    BOOST_CHECK(targetFunction::NOISYPARABOLA == p_test->getTargetFunction());
  }

  //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
  Gem::Common::condnotset("GStarterIndividual::specificTestsNoFailureExpected_GUnitTests",
                          "GEM_TESTING");
#endif                   /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This function is only useful
 * if you wish to run unit tests with your individual.
 */
void GStarterIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
  using namespace Gem::Geneva;

  using boost::unit_test_framework::test_case;
  using boost::unit_test_framework::test_suite;

  // Call the parent classes' functions
  Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests_();

  //------------------------------------------------------------------------------

  {
    /* Nothing. Add test cases here that are expected to fail.
            Enclose with a BOOST_CHECK_THROW, using the expected
            exception type as an additional argument. See the
            documentation for the Boost.Test library for further
            information */
  }

  //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
  Gem::Common::condnotset("GStarterIndividual::specificTestsFailuresExpected_GUnitTests",
                          "GEM_TESTING");
#endif                   /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to output a GStarterIndividual or convert it to a string using
 * boost::lexical_cast
 */
std::ostream& operator<<(std::ostream& stream, std::shared_ptr<GStarterIndividual> gsi_ptr) {
  stream << gsi_ptr->print();
  return stream;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param configFile The name of the configuration file
 */
GStarterIndividualFactory::GStarterIndividualFactory(std::filesystem::path const& configFile)
    : Gem::Common::GFactoryT<GParameterSet>(configFile) { /* nothing */
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GParameterSet> GStarterIndividualFactory::getObject_(
    Gem::Common::GParserBuilder& gpb, std::size_t const& id) {
  // Will hold the result
  std::shared_ptr<GStarterIndividual> target(new GStarterIndividual());

  // Make the object's local configuration options known
  target->addConfigurationOptions(gpb);

  return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GStarterIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
  // Describe our own options
  using namespace Gem::Courtier;

  // Allow our parent class to describe its options
  Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);

  // Local data
  gpb.registerFileParameter<double>("adProb", m_adProb, GSI_DEF_ADPROB)
      << "The probability for random adaptions of values in evolutionary algorithms";

  gpb.registerFileParameter<double>("sigma", m_sigma, GSI_DEF_SIGMA)
      << "The sigma for gauss-adaption in ES";

  gpb.registerFileParameter<double>("sigmaSigma", m_sigmaSigma, GSI_DEF_SIGMASIGMA)
      << "Influences the self-adaption of gauss-mutation in ES";

  gpb.registerFileParameter<double>("minSigma", m_minSigma, GSI_DEF_MINSIGMA)
      << "The minimum amount value of sigma";

  gpb.registerFileParameter<double>("maxSigma", m_maxSigma, GSI_DEF_MAXSIGMA)
      << "The maximum amount value of sigma";

  std::vector<double> defStartValues;
  defStartValues.push_back(1.);
  defStartValues.push_back(1.);
  defStartValues.push_back(1.);
  gpb.registerFileParameter<double>("startValues", m_startValues, defStartValues)
      << "The start values for all parameters" << std::endl
      << "Note that the number of entries also determines" << std::endl
      << "The number of parameter used in the optimization" << std::endl
      << "The number of entries in the vector may be changed" << std::endl
      << "in the configuration file.";

  std::vector<double> defLowerBoundaries;
  defLowerBoundaries.push_back(0.);
  defLowerBoundaries.push_back(0.);
  defLowerBoundaries.push_back(0.);
  gpb.registerFileParameter<double>("lowerBoundaries", m_lowerBoundaries, defLowerBoundaries)
      << "The lower boundaries for all parameters" << std::endl
      << "Note that as many entries are needed as" << std::endl
      << "There are entries in the startValues vector";

  std::vector<double> defUpperBoundaries;
  defUpperBoundaries.push_back(2.);
  defUpperBoundaries.push_back(2.);
  defUpperBoundaries.push_back(2.);
  gpb.registerFileParameter<double>("upperBoundaries", m_upperBoundaries, defUpperBoundaries)
      << "The upper boundaries for all parameters" << std::endl
      << "Note that as many entries are needed as" << std::endl
      << "There are entries in the startValues vector";
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we will usually add the parameter objects here. Note that a very similar constructor
 * exists for GStarterIndividual, so it may be used independently of the factory.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GStarterIndividualFactory::postProcess_(std::shared_ptr<GParameterSet>& p_base) {
  // Convert the base pointer to our local type
  std::shared_ptr<GStarterIndividual> p =
      Gem::Common::convertSmartPointer<GParameterSet, GStarterIndividual>(p_base);

  // We simply use a static function defined in the GStartIndividual header
  // to set up all parameter objects. It is used both here in the factory and
  // in one of the constructors.
  GStarterIndividual::addContent(*p, this->getId(), m_startValues, m_lowerBoundaries,
                                 m_upperBoundaries, m_sigma, m_sigmaSigma, m_minSigma, m_maxSigma,
                                 m_adProb);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
