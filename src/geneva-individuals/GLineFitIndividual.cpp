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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva-individuals/GLineFitIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GLineFitIndividual) // NOLINT
namespace Gem::Geneva {

/******************************************************************************/
/**
 * The default constructor -- private, as it is only needed for (de-)serialization purposes
 */
GLineFitIndividual::GLineFitIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * The standard constructor, sets up the internal data structures
 *
 * @param n_objects The number of parameters to be added to this individual
 */
GLineFitIndividual::GLineFitIndividual(const std::vector<std::tuple<double, double>> &data_points)
  : dataPoints_(data_points) {
    using namespace Gem::Geneva;

    for(std::size_t i = 0; i < 2; i++) {
        std::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject());
        std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
            new GDoubleGaussAdaptor(0.025, 0.1, 0.0001, 0.4, 1.)
        ); // sigma, sigma_sigma, min_sigma, max_sigma, ad_prob
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
GLineFitIndividual::GLineFitIndividual(const GLineFitIndividual &cp)
  : Gem::Geneva::GParameterSet(cp)
  , dataPoints_(cp.dataPoints_) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GLineFitIndividual::~GLineFitIndividual() { /* nothing */
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
void GLineFitIndividual::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GLineFitIndividual reference independent of this object and convert the pointer
    const GLineFitIndividual *p_load =
        Gem::Common::g_convert_and_compare<GObject, GLineFitIndividual>(cp, this);

    GToken token("GLineFitIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterSet>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(dataPoints_, p_load->dataPoints_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieves the tuple (a,b) of the line represented by this object
 */
std::tuple<double, double> GLineFitIndividual::getLine() const {
    std::vector<double> par_vec;
    this->streamline(par_vec);
    return std::tuple<double, double>(par_vec.at(0), par_vec.at(1));
}

/******************************************************************************/
/**
 * Loads the data of another GLineFitIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GLineFitIndividual, camouflaged as a GObject
 */
void GLineFitIndividual::load_(const GObject *cp) {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GLineFitIndividual reference independent of this object and convert the pointer
    const GLineFitIndividual *p_load =
        Gem::Common::g_convert_and_compare<GObject, GLineFitIndividual>(cp, this);

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
Gem::Geneva::GObject *GLineFitIndividual::clone_() const {
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
    std::vector<double> par_vec;
    this->streamline(par_vec);

    double a = par_vec.at(0);
    double b = par_vec.at(1);

    // Sum up the square deviation of line and data points
    double deviation = 0.;
    std::vector<std::tuple<double, double>>::iterator it;
    for(it = dataPoints_.begin(); it != dataPoints_.end(); ++it) {
        deviation = (a + b * std::get<0>(*it)) - std::get<1>(*it);
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
bool GLineFitIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(Gem::Geneva::GParameterSet::modify_GUnitTests_()) {
        result = true;
    }

    // Change the parameter settings
    result = true;

    return result;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GLineFitIndividual::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GLineFitIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GLineFitIndividual::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GLineFitIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GLineFitIndividual::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization through a config file
 *
 * @param config_file The name of the configuration file
 */
GLineFitIndividualFactory::GLineFitIndividualFactory(
    const std::vector<std::tuple<double, double>> &data_points,
    std::filesystem::path const &config_file
)
  : Gem::Common::GFactoryT<GParameterSet>(config_file)
  , dataPoints_(data_points) { /* nothing */
}

/******************************************************************************/
/**
 * The destructor
 */
GLineFitIndividualFactory::~GLineFitIndividualFactory() { /* nothing */
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GParameterSet> GLineFitIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    const std::size_t & /*id*/
) {
    // Will hold the result
    std::shared_ptr<GLineFitIndividual> target(new GLineFitIndividual(this->dataPoints_));

    // Make the object's local configuration options known
    target->addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GLineFitIndividualFactory::describeLocalOptions_(
    Gem::Common::GParserBuilder &gpb
) { // NOLINT(misc-unused-parameters)
    // Describe our own options
    using namespace Gem::Courtier;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)

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
void GLineFitIndividualFactory::postProcess_(std::shared_ptr<GParameterSet> &p_base) {
    // Convert the base pointer to our local type
    std::shared_ptr<GLineFitIndividual> p =
        Gem::Common::convertSmartPointer<GParameterSet, GLineFitIndividual>(p_base);

    // Nothing to be done here
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
