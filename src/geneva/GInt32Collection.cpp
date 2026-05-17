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
#include "geneva/GInt32Collection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32Collection) // NOLINT

namespace Gem::Geneva {

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GInt32Collection::GInt32Collection(
    const std::size_t &nval,
    const std::int32_t &min,
    const std::int32_t &max
)
  : GIntNumCollectionT<std::int32_t>(nval, min, max) { /* nothing */
}

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
    const std::size_t &nval,
    const std::int32_t &val,
    const std::int32_t &min,
    const std::int32_t &max
)
  : GIntNumCollectionT<std::int32_t>(nval, val, min, max) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32Collection::clone_() const {
    return new GInt32Collection(*this);
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
void GInt32Collection::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GInt32Collection reference independent of this object and convert the pointer
    const GInt32Collection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GInt32Collection>(cp, this);

    GToken token("GInt32Collection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GIntNumCollectionT<std::int32_t>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32Collection::name_() const {
    return std::string("GInt32Collection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param par_vec The vector to which the local values should be attached
 */
void GInt32Collection::int32Streamline(
    std::vector<std::int32_t> &par_vec,
    const activityMode & /*am*/
) const {
    GInt32Collection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        par_vec.push_back(*cit);
    }
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param par_vec The map to which the local values should be attached
 */
void GInt32Collection::int32Streamline(
    std::map<std::string, std::vector<std::int32_t>> &par_vec,
    const activityMode &am
) const {
#ifdef DEBUG
    if((this->getParameterName()).empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GInt32Collection::int32Streamline(std::map<std::string, "
               "std::vector<std::int32_t>>& par_vec) const: Error!"
            << '\n'
            << "No name was assigned to the object" << '\n'
        );
    }
#endif /* DEBUG */

    std::vector<std::int32_t> parameters;
    this->int32Streamline(parameters, am);
    par_vec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type std::int32_t to the vectors. Since this is an unbounded type,
 * we use the initialization boundaries as a replacement.
 *
 * @param l_bnd_vec A vector of lower std::int32_t parameter boundaries
 * @param u_bnd_vec A vector of upper std::int32_t parameter boundaries
 */
void GInt32Collection::int32Boundaries(
    std::vector<std::int32_t> &l_bnd_vec,
    std::vector<std::int32_t> &u_bnd_vec,
    const activityMode & /*am*/
) const {
    // Add as man lower and upper boundaries to the vector as
    // there are variables
    GInt32Collection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        l_bnd_vec.push_back(this->getLowerInitBoundary());
        u_bnd_vec.push_back(this->getUpperInitBoundary());
    }
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of std::int32_t values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of std::int32_t parameters
 */
std::size_t GInt32Collection::countInt32Parameters(
    const activityMode & /*am*/
) const {
    return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GInt32Collection::assignInt32ValueVector(
    const std::vector<std::int32_t> &par_vec,
    std::size_t &pos,
    const activityMode & /*am*/
) {
    for(GInt32Collection::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
        // Do we have a valid position ?
        if(pos >= par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GInt32Collection::assignInt32ValueVector(const std::vector<std::int32_t>&, "
                   "std::size_t&):"
                << '\n'
                << "Tried to access position beyond end of vector: " << par_vec.size() << "/" << pos
                << '\n'
            );
        }
#endif

        (*it) = par_vec[pos];
        pos++;
    }
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GInt32Collection::assignInt32ValueVectors(
    const std::map<std::string, std::vector<std::int32_t>> &par_map,
    const activityMode & /*am*/
) {
    GInt32Collection::iterator it;
    std::size_t cnt = 0;
    for(it = this->begin(); it != this->end(); ++it) {
        *it = (Gem::Common::getMapItem(par_map, this->getParameterName())).at(cnt++);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GInt32Collection::int32MultiplyByRandom(
    const std::int32_t &min,
    const std::int32_t &max,
    const activityMode & /*am*/
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_int_distribution<std::int32_t> uniform_int_distribution(min, max);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<std::int32_t>::setValue(
            pos,
            this->value(pos) * uniform_int_distribution(gr)
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a DOUBLE random value in the range [0,1[
 */
void GInt32Collection::int32MultiplyByRandom(
    const activityMode & /*am*/
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<std::int32_t>::setValue(
            pos,
            Gem::Common::narrow_cast<std::int32_t>(
                Gem::Common::narrow_cast<double>(this->value(pos)) * uniform_real_distribution(gr)
            )
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GInt32Collection::int32MultiplyBy(
    const std::int32_t &val // NOLINT(misc-unused-parameters)
    ,
    const activityMode & /*am*/
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<std::int32_t>::setValue(pos, val * this->value(pos));
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GInt32Collection::int32FixedValueInit(
    const std::int32_t &val // NOLINT(misc-unused-parameters)
    ,
    const activityMode & /*am*/
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<std::int32_t>::setValue(pos, val);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GInt32Collection::int32Add(
    std::shared_ptr<GParameterBase> p_base,
    const activityMode & /*am*/
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GInt32Collection> p =
        GParameterBase::parameterbase_cast<GInt32Collection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GInt32Collection::int32Add():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<std::int32_t>::setValue(pos, this->value(pos) + p->value(pos));
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GInt32Collection::int32Subtract(
    std::shared_ptr<GParameterBase> p_base,
    const activityMode & /*am*/
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GInt32Collection> p =
        GParameterBase::parameterbase_cast<GInt32Collection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GInt32Collection::int32Subtract():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<std::int32_t>::setValue(pos, this->value(pos) - p->value(pos));
    }
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Collection object, camouflaged as a GObject
 */
void GInt32Collection::load_(const GObject *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GInt32Collection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GInt32Collection>(cp, this);

    // Load our parent class'es data ...
    GIntNumCollectionT<std::int32_t>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Collection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GIntNumCollectionT<std::int32_t>::modify_GUnitTests_()) {
        result = true;
    }

    this->push_back(5);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GInt32Collection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Collection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<std::int32_t>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    giga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    giga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(giga_ptr);

    // Call the parent class'es function
    GIntNumCollectionT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests_();

    // no local data, nothing to test

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32Collection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32Collection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<std::int32_t>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    giga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    giga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(giga_ptr);

    // Call the parent class'es function
    GIntNumCollectionT<std::int32_t>::specificTestsFailuresExpected_GUnitTests_();

    // no local data, nothing to test

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32Collection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva */
