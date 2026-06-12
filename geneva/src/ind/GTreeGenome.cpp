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

#include "geneva/ind/GTreeGenome.hpp"

#include <any>
#include <memory>
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GContainerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/par/GBooleanObject.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleObject.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterObjectCollection.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * The default constructor. Using this constructor will result in a single
     * fitness criterion.
     */
GTreeGenome::GTreeGenome()
  : GOptimizableEntity() {
    /* nothing */
}

/******************************************************************************/
/**
     * Initialization with the number of fitness criteria
     */
GTreeGenome::GTreeGenome(const std::size_t n_fitness_criteria)
  : GOptimizableEntity(n_fitness_criteria) {
    /* nothing */
}

/******************************************************************************/
/**
     * The copy constructor.
     *
     * @param cp A copy of another GTreeGenome object
     */
GTreeGenome::GTreeGenome(GTreeGenome const &cp)
  : GOptimizableEntity(cp)
  , Gem::Common::GUniquePtrContainerT<GParameterBase>(cp) {
    /* nothing */
}

/******************************************************************************/
/**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GTreeGenome object, camouflaged as a GOptimizableEntity
     * @param e The expected outcome of the comparison
     */
void GTreeGenome::compare_(
    GOptimizableEntity const &cp,
    Gem::Common::expectation const &e,
    [[maybe_unused]] double const & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GTreeGenome reference independent of this object and convert the pointer
    const GTreeGenome *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GTreeGenome>(cp, this);

    GToken token("GTreeGenome", e);

    // Compare our individual-level base data ...
    Gem::Common::compare_base_t<GOptimizableEntity>(*this, *p_load, token);

    // The container base'es data -- compared explicitly, as it is a base-object
    // rather than a local member (the data is actually contained in a parent class).
    compare_t(IDENTITY(this->data_cnt_, p_load->data_cnt_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
     * Transformation of the individual's parameter objects into a boost::property_tree object .
     * This is e.g. used in GExternalEvaluatorIndividual for the communication with external
     * evaluation programs.
     */
void GTreeGenome::toPropertyTree(pt::ptree &ptr, std::string const &base_name) const {
#ifdef DEBUG
    // Check if the object is empty. If so, complain
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::toPropertyTree(): Error!" << '\n'
            << "Object is empty." << '\n'
        );
    }
#endif

    bool dirty_flag = (Gem::Courtier::processingStatus::DO_PROCESS == this->getProcessingStatus());
    bool has_errors = this->has_errors();

    double raw_fitness = 0.;
    double transformed_fitness = 0.;

    ptr.put(base_name + ".iteration", this->getAssignedIteration());
    ptr.put(base_name + ".is_dirty", dirty_flag);
    ptr.put(base_name + ".has_errors", has_errors);
    ptr.put(base_name + ".isValid", has_errors || dirty_flag ? false : this->isValid());
    ptr.put(base_name + ".type", std::string("GTreeGenome"));

    // Loop over all parameter objects and ask them to add their data to our ptree object
    ptr.put(base_name + ".nVars", this->size());
    std::string base; // NOLINT(cppcoreguidelines-init-variables)
    std::size_t pos = 0;
    for(const auto &item_ptr : *this) {
        base = base_name + ".vars.var" + Gem::Common::to_string(pos);
        item_ptr->toPropertyTree(ptr, base);
        pos++;
    }

    // Output the transformation policy
    switch(this->getEvaluationPolicy()) {
    case evaluationPolicy::USESIMPLEEVALUATION:
        ptr.put(base_name + ".transformationPolicy", "USESIMPLEEVALUATION");
        break;

    case evaluationPolicy::USESIGMOID:
        ptr.put(base_name + ".transformationPolicy", "USESIGMOID");
        break;

    case evaluationPolicy::USEWORSTCASEFORINVALID:
        ptr.put(base_name + ".transformationPolicy", "USEWORSTCASEFORINVALID");
        break;
    }

    // Output all fitness criteria. We do not enforce re-calculation of the fitness here,
    // as the property is meant to capture the current state of the individual.
    // Check the "is_dirty" tag, if you need to know whether the results are current.
    ptr.put(base_name + ".n_results", this->getNStoredResults());
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        raw_fitness = (dirty_flag || has_errors) ? this->getWorstCase() : this->raw_fitness(i);
        transformed_fitness =
            (dirty_flag || has_errors) ? this->getWorstCase() : this->transformed_fitness(i);

        base = base_name + ".results.result" + Gem::Common::to_string(i);
        ptr.put(base, transformed_fitness);
        base = base_name + ".results.rawResult" + Gem::Common::to_string(i);
        ptr.put(base, raw_fitness);
    }
}

/******************************************************************************/
/**
     * Transformation of the individual's parameter objects into a list of
     * comma-separated values and fitness plus possibly the validity
     *
     * @return A string holding the parameter values and possibly the types
     */
std::string GTreeGenome::toCSV(
    bool with_name_and_type,
    bool with_commas,
    bool use_raw_fitness,
    bool show_validity
) const {
    std::vector<double> d_data;
    std::vector<float> f_data;
    std::vector<std::int32_t> i_data;
    std::vector<bool> b_data;

    // Retrieve the parameter vectors (positional ordering -- parameters are no longer named)
    this->streamline<double>(d_data);
    this->streamline<float>(f_data);
    this->streamline<std::int32_t>(i_data);
    this->streamline<bool>(b_data);

    std::vector<std::string> var_names;
    std::vector<std::string> var_types;
    std::vector<std::string> var_values;

    // Extract the data. Names are synthesised positionally as "var<i>" (the same convention the
    // formula-constraint parser uses), since parameters carry no intrinsic name anymore.
    std::size_t var_index = 0;
    auto emit = [&](auto const &vec, const char *type_name) {
        for(std::size_t pos = 0; pos < vec.size(); ++pos) {
            if(with_name_and_type) {
                var_names.push_back(std::string("var") + Gem::Common::to_string(var_index));
                var_types.emplace_back(type_name);
            }
            var_values.push_back(Gem::Common::to_string(vec.at(pos)));
            ++var_index;
        }
    };
    emit(d_data, "double");
    emit(f_data, "float");
    emit(i_data, "int32");
    emit(b_data, "bool");

    // Note: The following will output the string "dirty" if the individual is in a "dirty" state
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        if(with_name_and_type) {
            var_names.push_back(std::string("Fitness_") + Gem::Common::to_string(i));
            var_types.emplace_back("double");
        }
        if(this->is_processed()) {
            // The individual has already been evaluated
            if(use_raw_fitness) {
                var_values.push_back(Gem::Common::to_string(this->raw_fitness(i)));
            }
            else {
                // Output potentially transformed fitness
                var_values.push_back(Gem::Common::to_string(this->transformed_fitness(i)));
            }
        }
        else {
            // No evaluation was performed so far
            if(this->has_errors()) {
                var_values.emplace_back("has_errors");
            }
            else {
                // "only" dirty / unevaluated
                var_values.emplace_back("dirty");
            }
        }
    }

    if(show_validity) {
        if(with_name_and_type) {
            var_names.emplace_back("validity");
            var_types.emplace_back("bool");
        }

        if(this->is_processed()) {
            // The individual has already been evaluated
            var_values.push_back(Gem::Common::to_string(this->isValid()));
        }
        else {
            var_values.push_back(Gem::Common::to_string(false));
        }
    }

    // Transfer the data into the result string
    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)
    std::vector<std::string>::const_iterator s_it;
    if(with_name_and_type) {
        for(s_it = var_names.begin(); s_it != var_names.end(); ++s_it) {
            result << *s_it;
            if(s_it + 1 != var_names.end()) {
                result << (with_commas ? ",\t" : "\t");
            }
        }
        result << '\n';

        for(s_it = var_types.begin(); s_it != var_types.end(); ++s_it) {
            result << *s_it;
            if(s_it + 1 != var_types.end()) {
                result << (with_commas ? ",\t" : "\t");
            }
        }
        result << '\n';
    }

    for(s_it = var_values.begin(); s_it != var_values.end(); ++s_it) {
        result << *s_it;
        if(s_it + 1 != var_values.end()) {
            result << (with_commas ? ",\t" : "\t");
        }
    }
    result << '\n';

    return result.str();
}

/******************************************************************************/
/**
     * Prevent shadowing of std::vector<GParameterBase>::at()
     *
     * @param pos The position of the item we aim to retrieve from the std::vector<GParameterBase>
     * @return The item we aim to retrieve from the std::vector<GParameterBase>
     */
Gem::Common::GUniquePtrContainerT<GParameterBase>::reference
GTreeGenome::at(std::size_t const &pos) {
    return Gem::Common::GUniquePtrContainerT<GParameterBase>::at(pos);
}

/******************************************************************************/
/**
     * Retrieval of a suitable position for cross over inside of a vector
     *
     * @param lower The lower (inclusive) boundary for retrieval of a cross-over position
     * @param upper The upper (exclusive) boundary for retrieval of a cross-over position
     * @return A suitable cross-over position in the range [lower, upper[
     */
std::size_t GTreeGenome::getCrossOverPos(const std::size_t lower, const std::size_t upper) {
    // Make sure the boundaries are suitable. These were DEBUG-only asserts;
    // under NDEBUG an invalid range (e.g. upper == lower for a single-element
    // collection) produced an inverted std::uniform_int_distribution range,
    // which is undefined behaviour. Enforce unconditionally instead.
    if(lower == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::getCrossOverPos(): Error!" << '\n'
            << "lower boundary is 0, but must be > 0" << '\n'
        );
    }
    if(upper <= lower) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::getCrossOverPos(): Error!" << '\n'
            << "Invalid range: upper (" << upper << ") must be > lower (" << lower << ")" << '\n'
        );
    }

    return uniform_int_(
        gr_,
        std::uniform_int_distribution<std::size_t>::param_type(lower, upper - 1)
    );
}

/******************************************************************************/
/**
     * Perform a fusion operation between this object and another.
     */
std::shared_ptr<GOptimizableEntity>
GTreeGenome::crossOverWith(GOptimizableEntity const &cp_base) const {
    // The cross-over operates on the parameter tree, so the partner must be a GTreeGenome
    const auto &cp = dynamic_cast<GTreeGenome const &>(cp_base);

    // Create a copy of this object
    std::shared_ptr<GTreeGenome> this_cp = this->clone<GTreeGenome>();

    // Extract all data items
    std::vector<double> this_double_cnt;
    std::vector<double> cp_double_cnt;
    std::vector<float> this_float_cnt;
    std::vector<float> cp_float_cnt;
    std::vector<bool> this_bool_cnt;
    std::vector<bool> cp_bool_cnt;
    std::vector<std::int32_t> this_int_cnt;
    std::vector<std::int32_t> cp_int_cnt;

    this_cp->streamline(this_double_cnt);
    this_cp->streamline(this_float_cnt);
    this_cp->streamline(this_bool_cnt);
    this_cp->streamline(this_int_cnt);

    cp.streamline(cp_double_cnt);
    cp.streamline(cp_float_cnt);
    cp.streamline(cp_bool_cnt);
    cp.streamline(cp_int_cnt);

#ifdef DEBUG
    // Do some error checking
    if(this_double_cnt.size() != cp_double_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::perItemCrossOver(): Error!" << '\n'
            << "Got invalid sizes (double): " << this_double_cnt.size() << " / "
            << cp_double_cnt.size() << '\n'
        );
    }
    if(this_float_cnt.size() != cp_float_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::perItemCrossOver(): Error!" << '\n'
            << "Got invalid sizes (float): " << this_float_cnt.size() << " / "
            << cp_float_cnt.size() << '\n'
        );
    }
    if(this_bool_cnt.size() != cp_bool_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::perItemCrossOver(): Error!" << '\n'
            << "Got invalid sizes (bool): " << this_bool_cnt.size() << " / " << cp_bool_cnt.size()
            << '\n'
        );
    }
    if(this_int_cnt.size() != cp_int_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::perItemCrossOver(): Error!" << '\n'
            << "Got invalid sizes (std::int32_t): " << this_int_cnt.size() << " / "
            << cp_int_cnt.size() << '\n'
        );
    }
#endif /* DEBUG */

    // Do the actual cross-over
    if(not this_double_cnt.empty()) {
        // Calculate a suitable position for the cross-over
        // We use this->cp as the source for getCrossOverPos in order to avoid
        // having to mark getCrossOverPos const and this gr_ mutable.
        const auto pos = this_cp->getCrossOverPos(1, this_double_cnt.size());

        // Perform the actual cross-over operation. This is in fact
        // a "half" cross-over, as we only need one output vector
        std::copy(cp_double_cnt.begin() + pos, cp_double_cnt.end(), this_double_cnt.begin() + pos);
    }

    if(not this_float_cnt.empty()) {
        // Calculate a suitable position for the cross-over
        const auto pos = this_cp->getCrossOverPos(1, this_float_cnt.size());

        // Perform the actual cross-over operation. This is in fact
        // a "half" cross-over, as we only need one output vector
        std::copy(cp_float_cnt.begin() + pos, cp_float_cnt.end(), this_float_cnt.begin() + pos);
    }

    if(not this_bool_cnt.empty()) {
        // Calculate a suitable position for the cross-over
        const auto pos = this_cp->getCrossOverPos(1, this_bool_cnt.size());

        // Perform the actual cross-over operation. This is in fact
        // a "half" cross-over, as we only need one output vector
        std::copy(cp_bool_cnt.begin() + pos, cp_bool_cnt.end(), this_bool_cnt.begin() + pos);
    }

    if(not this_int_cnt.empty()) {
        // Calculate a suitable position for the cross-over
        const auto pos = this_cp->getCrossOverPos(1, this_int_cnt.size());

        // Perform the actual cross-over operation. This is in fact
        // a "half" cross-over, as we only need one output vector
        std::copy(cp_int_cnt.begin() + pos, cp_int_cnt.end(), this_int_cnt.begin() + pos);
    }

    // Load the data vectors back into this object
    this_cp->assignValueVector(this_double_cnt);
    this_cp->assignValueVector(this_float_cnt);
    this_cp->assignValueVector(this_bool_cnt);
    this_cp->assignValueVector(this_int_cnt);

    // Mark this individual as "dirty"
    this_cp->mark_as_due_for_processing();

    return this_cp;
}

/******************************************************************************/
/**
     * Triggers updates of adaptors contained in this object.
     */
void GTreeGenome::updateAdaptorsOnStall(const std::uint32_t n_stalls) {
    for(auto const &item_ptr : *this) {
        item_ptr->updateAdaptorsOnStall(n_stalls);
    }
}

/******************************************************************************/
/**
     * Retrieves information from adaptors with a given property
     *
     * @param adaptor_name The name of the adaptor to be queried
     * @param property The property for which information is sought
     * @param data A vector, to which the properties should be added
     */
void GTreeGenome::queryAdaptor(
    std::string const &adaptor_name,
    std::string const &property,
    std::vector<std::any> &data
) const {
    for(const auto &item_ptr : *this) {
        item_ptr->queryAdaptor(adaptor_name, property, data);
    }
}

/******************************************************************************/
/**
     * Retrieves parameters relevant for the evaluation from another GTreeGenome.
     * NOTE: The other parameter set will be an empty shell afterwards. The function may
     * only be called for "clean" foreign parameter sets
     */
void GTreeGenome::cannibalize(GOptimizableEntity &cp_base) {
    // The cannibalisation moves the parameter tree over, so the partner must be a GTreeGenome
    auto &cp = dynamic_cast<GTreeGenome &>(cp_base);

    // Check whether the "foreign" entity is processed
    if(cp.is_due_for_processing() || cp.has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::cannibalize(GOptimizableEntity& cp)" << '\n'
            << "cp isn't processed or has errors" << '\n'
        );
    }

    // Make sure we have no local parameters
    this->clear();

    // Move all "foreign" parameters over (cp is emptied right after -- a true cannibalisation, so the
    // uniquely-owned parameters are transferred, not cloned).
    for(auto &t_ptr : cp) {
        this->push_back(std::move(t_ptr));
    }

    // Empty the foreign GParmeterSet object
    cp.clear();

    // Set our own fitness according to the foreign individual. This will also
    // clear our local dirty flag (if set).
    this->setFitness_(cp.raw_fitness_vec());
}

/******************************************************************************/
/**
     * Loads the data of another GTreeGenome object, camouflaged as a GOptimizableEntity.
     *
     * @param cp A copy of another GTreeGenome object, camouflaged as a GOptimizableEntity
     */
void GTreeGenome::load_(const GOptimizableEntity *cp) {
    // Check that we are dealing with a GTreeGenome reference independent of this object and convert the pointer
    const GTreeGenome *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GTreeGenome>(cp, this);

    // Load the individual-level base class' data
    GOptimizableEntity::load_(cp);

    // Load the container base class' data
    Gem::Common::GUniquePtrContainerT<GParameterBase>::operator=(*p_load);
}

/******************************************************************************/
/**
     * Allows to randomly initialize parameter members. This function may be overloaded
     * by derived classes, but should be called by them. This function recursively initializes
     * parameters randomly.
     *
     * @return A boolean indicating whether modifications where made
     */
bool GTreeGenome::randomInit_(activityMode const &am) {
    bool modifications_made = false;

    // Trigger random initialization of all our parameter objects
    for(auto &parm_ptr : *this) {
        if(parm_ptr->randomInit(am, gr_)) {
            modifications_made = true;
        }
    }

    // This also takes care of empty parameter sets, as modifications_made
    // will remain false in this case.
    return modifications_made;
}

/**********************************************************************************/
/**
     * The actual adaption operations. Easy, as we know that all objects
     * in this collection must implement the adapt() function, as they are
     * derived from the GMutableI class / interface.
     */
std::size_t GTreeGenome::customAdaptions() {
    std::size_t n_adaptions = 0;
    for(const auto &par_ptr : *this) {
        n_adaptions += par_ptr->adapt(gr_);
    }

    return n_adaptions;
}

/******************************************************************************/
/**
     * Retrieves a parameter of a given type at the specified position
     */
std::any GTreeGenome::getVarValImpl(
    std::string const &descr,
    std::tuple<std::size_t, std::string, std::size_t> const &target
) {
    std::any result;

    if(descr == "d") {
        result = GTreeGenome::getVarItem<double>(target);
    }
    else if(descr == "f") {
        result = GTreeGenome::getVarItem<float>(target);
    }
    else if(descr == "i") {
        result = GTreeGenome::getVarItem<std::int32_t>(target);
    }
    else if(descr == "b") {
        result = GTreeGenome::getVarItem<bool>(target);
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTreeGenome::getVarValImpl(): Error!" << '\n'
            << "Received invalid type description" << '\n'
        );
    }

    return result;
}

/******************************************************************************/
/**
     * Emits a name for this class / object
     *
     * @return The name of this class / object
     */
std::string GTreeGenome::name_() const {
    return std::string("GTreeGenome");
}

/******************************************************************************/
/**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
bool GTreeGenome::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the stateful base class'es function
    if(Gem::Common::GUniquePtrContainerT<GParameterBase>::modify_GUnitTests_()) {
        result = true;
    }

    for(const auto &o_ptr : *this) {
        if(o_ptr->modify_GUnitTests()) {
            result = true;
        }
    }

    if(this->randomInit(activityMode::ALLPARAMETERS)) {
        result = true;
    }

    // A relatively harmless change
    this->setNStalls(this->getNStalls() + 1);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw

    Gem::Common::condnotset("GTreeGenome::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
void GTreeGenome::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Access to uniformly distributed double random numbers
    std::uniform_real_distribution<double> uniform_real_distribution;

    // Call the stateful base class'es function
    Gem::Common::GUniquePtrContainerT<GParameterBase>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the maximization mode flag
        std::shared_ptr<GTreeGenome> p_test = this->clone<GTreeGenome>();

        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MAXIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MAXIMIZE);
        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MINIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MINIMIZE);
    }

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the surrounding optimization algorithm's current iteration
        std::shared_ptr<GTreeGenome> p_test = this->clone<GTreeGenome>();

        for(std::uint32_t i = 1; i < 10; i++) {
            CHECK_NOTHROW(p_test->setAssignedIteration(i));
            INFO(
                "\n"
                << "p_test->getAssignedIteration() = " << p_test->getAssignedIteration() << "\n"
                << "i = " << i << "\n"
            );
            CHECK(p_test->getAssignedIteration() == i);
        }
    }

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the best known fitness so far
        std::shared_ptr<GTreeGenome> p_test = this->clone<GTreeGenome>();

        double d = 0.;
        while(true) {
            CHECK_NOTHROW(p_test->setBestKnownPrimaryFitness(std::make_tuple(d, d)));
            INFO(
                "\n"
                << "p_test->getBestKnownPrimaryFitness() = "
                << Gem::Common::g_to_string(p_test->getBestKnownPrimaryFitness()) << "\n"
                << "d = " << d << "\n"
            );
            CHECK(p_test->getBestKnownPrimaryFitness() == std::make_tuple(d, d));

            if((d += 0.1) >= 1.) { // NOLINT(bugprone-assignment-in-if-condition)
                break;
            }
        }
    }

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the number of consecutive stalls
        std::shared_ptr<GTreeGenome> p_test = this->clone<GTreeGenome>();

        for(std::uint32_t i = 1; i < 10; i++) {
            CHECK_NOTHROW(p_test->setNStalls(i));
            INFO(
                "\n"
                << "p_test->getNStalls() = " << p_test->getNStalls() << "\n"
                << "i = " << i << "\n"
            );
            CHECK(p_test->getNStalls() == i);
        }
    }

    //---------------------------------------------------------------------

    {
        // All tests below use the same, cloned collection
        // Some settings for the collection of tests below
        constexpr double mingconstrdouble = -4.;
        constexpr double maxgconstrdouble = 4.;
        constexpr double mingdouble = -5.;
        constexpr double maxgdouble = 5.;
        constexpr double mingdoublecoll = -3.;
        constexpr double maxgdoublecoll = 3.;
        constexpr std::size_t ngdoublecoll = 10;
        constexpr std::size_t fploopcount = 5;
        constexpr double fpfixedvalinitmin = -3.;
        constexpr double fpfixedvalinitmax = 3.;
        constexpr double fpmultiplybyrandmin = -5.;
        constexpr double fpmultiplybyrandmax = 5.;
        constexpr double fpadd = 2.;

        // Create a GTreeGenome object as a clone of this object for further usage
        std::shared_ptr<GTreeGenome> p_test_0 = this->clone<GTreeGenome>();
        // Clear the collection
        p_test_0->clear();
        // Make sure it is really empty
        CHECK(p_test_0->empty());
        // Add some floating pount parameters
        for(std::size_t i = 0; i < fploopcount; i++) {
            p_test_0->push_back(
                std::make_shared<GConstrainedDoubleObject>(
                    uniform_real_distribution(
                        gr_,
                        std::uniform_real_distribution<double>::param_type(
                            mingconstrdouble,
                            maxgconstrdouble
                        )
                    ),
                    mingconstrdouble,
                    maxgconstrdouble
                )
            );
            p_test_0->push_back(
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(mingdouble, maxgdouble)
                ))
            );
            p_test_0->push_back(
                std::make_shared<GDoubleCollection>(ngdoublecoll, mingdoublecoll, maxgdoublecoll)
            );
        }

        // Attach a few other parameter types
        p_test_0->push_back(std::make_shared<GConstrainedInt32Object>(7, -10, 10));
        p_test_0->push_back(std::make_shared<GBooleanObject>(true));

        //-----------------------------------------------------------------

        {
            // Test random initialization
            // Create a GTreeGenome object as a clone of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();

            CHECK_NOTHROW(p_test->randomInit(activityMode::ALLPARAMETERS));

            bool objects_equal = false;
            try {
                p_test->compare(
                    *p_test_0,
                    Gem::Common::expectation::INEQUALITY,
                    Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
                );
            }
            catch(const g_expectation_violation &) {
                objects_equal = true;
            }

            CHECK(false == objects_equal);
        }

        //-----------------------------------------------------------------
        {
            // Test initialization of all fp parameters with a fixed value
            double d = fpfixedvalinitmin;
            while(true) {
                // Create a GTreeGenome object as a clone of p_test_0 for further usage
                std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();

                // Initialize all fp-values with 0.
                p_test->fixedValueInit<double>(d, activityMode::ALLPARAMETERS);

                // Make sure the dirty flag is set
                CHECK(p_test->is_due_for_processing() == true);

                // Cross-check
                std::size_t counter = 0;
                for(std::size_t i = 0; i < fploopcount; i++) {
                    CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() == d);
                    counter++;
                    CHECK(p_test->at<GDoubleObject>(counter)->value() == d);
                    counter++;
                    std::shared_ptr<GDoubleCollection> p_gdc;
                    CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                    for(std::size_t gdc_cnt = 0; gdc_cnt < ngdoublecoll; gdc_cnt++) {
                        INFO(
                            "\n"
                            << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                            << "expected " << d << "\n"
                            << "iteration = " << gdc_cnt << "\n"
                        );
                        CHECK(p_gdc->at(gdc_cnt) == d);
                    }
                    counter++;
                }

                // The int32 parameter should have stayed the same
                std::shared_ptr<GConstrainedInt32Object> p_int32_0;
                std::shared_ptr<GConstrainedInt32Object> p_int32;
                CHECK_NOTHROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
                CHECK_NOTHROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
                CHECK(*p_int32_0 == *p_int32);
                counter++;

                // Likewise, the boolean parameter should have stayed the same
                std::shared_ptr<GBooleanObject> p_boolean_orig;
                std::shared_ptr<GBooleanObject> p_boolean_cloned;
                CHECK_NOTHROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
                CHECK_NOTHROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
                CHECK(*p_boolean_orig == *p_boolean_cloned);
                counter++;

                if((d += 1.) >= fpfixedvalinitmax) { // NOLINT(bugprone-assignment-in-if-condition)
                    break;
                }
            }
        }

        //-----------------------------------------------------------------

        {
            // Test multiplication of all fp parameters with a fixed value
            double d = -3.;
            while(true) {
                // Create a GTreeGenome object as a clone of p_test_0 for further usage
                std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();

                // Initialize all fp-values with FPFIXEDVALINITMAX
                CHECK_NOTHROW(
                    p_test->fixedValueInit<double>(fpfixedvalinitmax, activityMode::ALLPARAMETERS)
                );

                // Multiply this fixed value by d
                CHECK_NOTHROW(p_test->multiplyBy<double>(d, activityMode::ALLPARAMETERS));

                // Make sure the dirty flag is set
                CHECK(p_test->is_due_for_processing() == true);

                // Cross-check
                std::size_t counter = 0;
                for(std::size_t i = 0; i < fploopcount; i++) {
                    // A constrained value does not have to assume the value d*FPFIXEDVALINITMAX,
                    // but needs to stay within its boundaries
                    CHECK(
                        p_test->at<GConstrainedDoubleObject>(counter)->value() >= mingconstrdouble
                    );
                    CHECK(
                        p_test->at<GConstrainedDoubleObject>(counter)->value() <= maxgconstrdouble
                    );
                    counter++;
                    CHECK(p_test->at<GDoubleObject>(counter)->value() == d * fpfixedvalinitmax);
                    counter++;
                    std::shared_ptr<GDoubleCollection> p_gdc;
                    CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                    for(std::size_t gdc_cnt = 0; gdc_cnt < ngdoublecoll; gdc_cnt++) {
                        INFO(
                            "\n"
                            << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                            << "expected " << d * fpfixedvalinitmax << "\n"
                            << "iteration = " << gdc_cnt << "\n"
                        );
                        CHECK(p_gdc->at(gdc_cnt) == d * fpfixedvalinitmax);
                    }
                    counter++;
                }

                // The int32 parameter should have stayed the same
                std::shared_ptr<GConstrainedInt32Object> p_int32_0;
                std::shared_ptr<GConstrainedInt32Object> p_int32;
                CHECK_NOTHROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
                CHECK_NOTHROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
                CHECK(*p_int32_0 == *p_int32);
                counter++;

                // Likewise, the boolean parameter should have stayed the same
                std::shared_ptr<GBooleanObject> p_boolean_orig;
                std::shared_ptr<GBooleanObject> p_boolean_cloned;
                CHECK_NOTHROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
                CHECK_NOTHROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
                CHECK(*p_boolean_orig == *p_boolean_cloned);
                counter++;

                if((d += 1.) >= 3.) { // NOLINT(bugprone-assignment-in-if-condition)
                    break;
                }
            }
        }

        //-----------------------------------------------------------------

        {
            // Test that fpMultiplyByRandom(min,max) changes every single parameter
            // Create a GTreeGenome object as a clone of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();

            // Multiply each floating point value with a constrained random value
            CHECK_NOTHROW(p_test->multiplyByRandom<double>(
                fpmultiplybyrandmin,
                fpmultiplybyrandmax,
                activityMode::ALLPARAMETERS
            ));

            // Make sure the dirty flag is set
            CHECK(p_test->is_due_for_processing() == true);

            // Cross-check
            std::size_t counter = 0;
            for(std::size_t i = 0; i < fploopcount; i++) {
                CHECK(
                    p_test->at<GConstrainedDoubleObject>(counter)->value() !=
                    p_test_0->at<GConstrainedDoubleObject>(counter)->value()
                );
                counter++;
                CHECK(
                    p_test->at<GDoubleObject>(counter)->value() !=
                    p_test_0->at<GDoubleObject>(counter)->value()
                );
                counter++;
                std::shared_ptr<GDoubleCollection> p_gdc;
                std::shared_ptr<GDoubleCollection> p_gdc_0;
                CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                CHECK_NOTHROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
                for(std::size_t gdc_cnt = 0; gdc_cnt < ngdoublecoll; gdc_cnt++) {
                    INFO(
                        "\n"
                        << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                        << "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
                        << "iteration = " << gdc_cnt << "\n"
                    );
                    CHECK(p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt));
                }
                counter++;
            }

            // The int32 parameter should have stayed the same
            std::shared_ptr<GConstrainedInt32Object> p_int32_0;
            std::shared_ptr<GConstrainedInt32Object> p_int32;
            CHECK_NOTHROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
            CHECK_NOTHROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
            CHECK(*p_int32_0 == *p_int32);
            counter++;

            // Likewise, the boolean parameter should have stayed the same
            std::shared_ptr<GBooleanObject> p_boolean_orig;
            std::shared_ptr<GBooleanObject> p_boolean_cloned;
            CHECK_NOTHROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
            CHECK_NOTHROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
            CHECK(*p_boolean_orig == *p_boolean_cloned);
            counter++;
        }

        //-----------------------------------------------------------------

        {
            // Test that fpMultiplyByRandom() changes every single parameter
            // Create a GTreeGenome object as a clone of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();

            // Multiply each floating point value with a constrained random value
            CHECK_NOTHROW(p_test->multiplyByRandom<double>(activityMode::ALLPARAMETERS));

            // Make sure the dirty flag is set
            CHECK(p_test->is_due_for_processing() == true);

            // Cross-check
            std::size_t counter = 0;
            for(std::size_t i = 0; i < fploopcount; i++) {
                CHECK(
                    p_test->at<GConstrainedDoubleObject>(counter)->value() !=
                    p_test_0->at<GConstrainedDoubleObject>(counter)->value()
                );
                counter++;
                CHECK(
                    p_test->at<GDoubleObject>(counter)->value() !=
                    p_test_0->at<GDoubleObject>(counter)->value()
                );
                counter++;
                std::shared_ptr<GDoubleCollection> p_gdc;
                std::shared_ptr<GDoubleCollection> p_gdc_0;
                CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                CHECK_NOTHROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
                for(std::size_t gdc_cnt = 0; gdc_cnt < ngdoublecoll; gdc_cnt++) {
                    INFO(
                        "\n"
                        << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                        << "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
                        << "iteration = " << gdc_cnt << "\n"
                    );
                    CHECK(p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt));
                }
                counter++;
            }

            // The int32 parameter should have stayed the same
            std::shared_ptr<GConstrainedInt32Object> p_int32_0;
            std::shared_ptr<GConstrainedInt32Object> p_int32;
            CHECK_NOTHROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
            CHECK_NOTHROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
            CHECK(*p_int32_0 == *p_int32);
            counter++;

            // Likewise, the boolean parameter should have stayed the same
            std::shared_ptr<GBooleanObject> p_boolean_orig;
            std::shared_ptr<GBooleanObject> p_boolean_cloned;
            CHECK_NOTHROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
            CHECK_NOTHROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
            CHECK(*p_boolean_orig == *p_boolean_cloned);
            counter++;
        }

        //-----------------------------------------------------------------

        {
            // Check adding of individuals
            // Create two GTreeGenome objects as a clone of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();
            std::shared_ptr<GTreeGenome> p_test_fixed = p_test_0->clone<GTreeGenome>();

            // Initialize all fp-values of the "add" individual with a fixed value
            CHECK_NOTHROW(p_test_fixed->fixedValueInit<double>(fpadd, activityMode::ALLPARAMETERS));

            // Add p_test_fixed to p_test
            CHECK_NOTHROW(p_test->add<double>(p_test_fixed, activityMode::ALLPARAMETERS));

            // Check the results
            std::size_t counter = 0;
            for(std::size_t i = 0; i < fploopcount; i++) {
                // A constrained value does not have to assume the value value()+FPADD
                // but needs to stay within its boundaries
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= mingconstrdouble);
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= maxgconstrdouble);
                counter++;
                CHECK(
                    p_test->at<GDoubleObject>(counter)->value() ==
                    p_test_0->at<GDoubleObject>(counter)->value() + fpadd
                );
                counter++;
                std::shared_ptr<GDoubleCollection> p_gdc;
                std::shared_ptr<GDoubleCollection> p_gdc_0;
                CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                CHECK_NOTHROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
                for(std::size_t gdc_cnt = 0; gdc_cnt < ngdoublecoll; gdc_cnt++) {
                    INFO(
                        "\n"
                        << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                        << "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
                        << "FPADD = " << fpadd
                        << "p_gdc_0->at(gdc_cnt) + FPADD = " << p_gdc_0->at(gdc_cnt) + fpadd << "\n"
                        << "iteration = " << gdc_cnt << "\n"
                    );
                    CHECK(p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) + fpadd);
                }
                counter++;
            }

            // The int32 parameter should have stayed the same
            std::shared_ptr<GConstrainedInt32Object> p_int32_0;
            std::shared_ptr<GConstrainedInt32Object> p_int32;
            CHECK_NOTHROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
            CHECK_NOTHROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
            CHECK(*p_int32_0 == *p_int32);
            counter++;

            // Likewise, the boolean parameter should have stayed the same
            std::shared_ptr<GBooleanObject> p_boolean_orig;
            std::shared_ptr<GBooleanObject> p_boolean_cloned;
            CHECK_NOTHROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
            CHECK_NOTHROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
            CHECK(*p_boolean_orig == *p_boolean_cloned);
            counter++;
        }

        //-----------------------------------------------------------------

        {
            constexpr double fpsubtract = 2.;

            // Check subtraction of individuals
            // Create two GTreeGenome objects as a clone of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();
            std::shared_ptr<GTreeGenome> p_test_fixed = p_test_0->clone<GTreeGenome>();

            // Initialize all fp-values of the "add" individual with a fixed valie
            CHECK_NOTHROW(
                p_test_fixed->fixedValueInit<double>(fpsubtract, activityMode::ALLPARAMETERS)
            );

            // Add p_test_fixed to p_test
            CHECK_NOTHROW(p_test->subtract<double>(p_test_fixed, activityMode::ALLPARAMETERS));

            // Check the results
            std::size_t counter = 0;
            for(std::size_t i = 0; i < fploopcount; i++) {
                // A constrained value does not have to assume the value value()-FPSUBTRACT
                // but needs to stay within its boundaries
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= mingconstrdouble);
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= maxgconstrdouble);
                counter++;
                CHECK(
                    p_test->at<GDoubleObject>(counter)->value() ==
                    p_test_0->at<GDoubleObject>(counter)->value() - fpsubtract
                );
                counter++;
                std::shared_ptr<GDoubleCollection> p_gdc;
                std::shared_ptr<GDoubleCollection> p_gdc_0;
                CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                CHECK_NOTHROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
                for(std::size_t gdc_cnt = 0; gdc_cnt < ngdoublecoll; gdc_cnt++) {
                    INFO(
                        "\n"
                        << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                        << "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
                        << "FPSUBTRACT = " << fpsubtract << "p_gdc_0->at(gdc_cnt) - FPSUBTRACT = "
                        << p_gdc_0->at(gdc_cnt) - fpsubtract << "\n"
                        << "iteration = " << gdc_cnt << "\n"
                    );
                    CHECK(p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) - fpsubtract);
                }
                counter++;
            }

            // The int32 parameter should have stayed the same
            std::shared_ptr<GConstrainedInt32Object> p_int32_0;
            std::shared_ptr<GConstrainedInt32Object> p_int32;
            CHECK_NOTHROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
            CHECK_NOTHROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
            CHECK(*p_int32_0 == *p_int32);
            counter++;

            // Likewise, the boolean parameter should have stayed the same
            std::shared_ptr<GBooleanObject> p_boolean_orig;
            std::shared_ptr<GBooleanObject> p_boolean_cloned;
            CHECK_NOTHROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
            CHECK_NOTHROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
            CHECK(*p_boolean_orig == *p_boolean_cloned);
            counter++;
        }

        //-----------------------------------------------------------------
    }

    //---------------------------------------------------------------------

    {
        // Check counting of active and inactive parameters
        // Some settings for the collection of tests below
        constexpr double mingconstrdouble = -4.;
        constexpr double maxgconstrdouble = 4.;
        constexpr double mingdouble = -5.;
        constexpr double maxgdouble = 5.;
        constexpr double mingdoublecoll = -3.;
        constexpr double maxgdoublecoll = 3.;
        constexpr std::size_t ngdoublecoll = 10;
        constexpr std::size_t nintcoll = 10;
        constexpr std::size_t nintboolobj = 10;
        constexpr std::int32_t mingint = -100;
        constexpr std::int32_t maxgint = 100;
        constexpr std::size_t fploopcount = 5;

        // Create a GTreeGenome object as a clone of this object for further usage
        std::shared_ptr<GTreeGenome> p_test_0 = this->clone<GTreeGenome>();
        // Clear the collection
        p_test_0->clear();
        // Make sure it is really empty
        CHECK(p_test_0->empty());

        // Add some floating point parameters
        for(std::size_t i = 0; i < fploopcount; i++) {
            std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr =
                std::make_shared<GConstrainedDoubleObject>(
                    uniform_real_distribution(
                        gr_,
                        std::uniform_real_distribution<double>::param_type(
                            mingconstrdouble,
                            maxgconstrdouble
                        )
                    ),
                    mingconstrdouble,
                    maxgconstrdouble
                );
            std::shared_ptr<GDoubleObject> gdo_ptr =
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(mingdouble, maxgdouble)
                ));
            std::shared_ptr<GDoubleCollection> gdc_ptr =
                std::make_shared<GDoubleCollection>(ngdoublecoll, mingdoublecoll, maxgdoublecoll);

            // Mark the last parameter type as inactive
            gdc_ptr->setAdaptionsInactive();

            // Add the parameter objects to the parameter set
            p_test_0->push_back(gcdo_ptr);
            p_test_0->push_back(gdo_ptr);
            p_test_0->push_back(gdc_ptr);
        }

        // Attach a few other parameter types
        for(std::size_t i = 0; i < nintboolobj; i++) {
            p_test_0->push_back(std::make_shared<GConstrainedInt32Object>(7, mingint, maxgint));
            p_test_0->push_back(std::make_shared<GBooleanObject>(true));
        }

        // Finally we add a tree structure
        std::shared_ptr<GParameterObjectCollection> poc_ptr =
            std::make_shared<GParameterObjectCollection>();

        for(std::size_t i = 0; i < fploopcount; i++) {
            std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr =
                std::make_shared<GConstrainedDoubleObject>(
                    uniform_real_distribution(
                        gr_,
                        std::uniform_real_distribution<double>::param_type(
                            mingconstrdouble,
                            maxgconstrdouble
                        )
                    ),
                    mingconstrdouble,
                    maxgconstrdouble
                );
            std::shared_ptr<GDoubleObject> gdo_ptr =
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(mingdouble, maxgdouble)
                ));
            std::shared_ptr<GConstrainedInt32ObjectCollection> gcioc_ptr =
                std::make_shared<GConstrainedInt32ObjectCollection>();

            std::shared_ptr<GParameterObjectCollection> sub_poc_ptr =
                std::make_shared<GParameterObjectCollection>();

            for(std::size_t ip = 0; ip < nintcoll; ip++) {
                std::shared_ptr<GConstrainedInt32Object> gci32o_ptr =
                    std::make_shared<GConstrainedInt32Object>(mingint, maxgint);
                gci32o_ptr->setAdaptionsInactive();
                // The parameter should not be modifiable now

                sub_poc_ptr->push_back(gci32o_ptr);
            }

            std::shared_ptr<GDoubleObject> gdo2_ptr =
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(mingdouble, maxgdouble)
                ));
            gdo2_ptr->setAdaptionsInactive();
            sub_poc_ptr->push_back(gdo2_ptr);

            // Add the parameter objects to the parameter set
            poc_ptr->push_back(gcdo_ptr);
            poc_ptr->push_back(gdo_ptr);
            poc_ptr->push_back(gcioc_ptr);
            poc_ptr->push_back(sub_poc_ptr);
        }

        p_test_0->push_back(poc_ptr);

        // The amount of parameters of a given category
        std::size_t ndoubleactive = 2 * fploopcount + 2 * fploopcount;
        std::size_t ndoubleinactive = ngdoublecoll * fploopcount + fploopcount;
        std::size_t ndoubleall = ndoubleinactive + ndoubleactive;
        std::size_t nintactive = nintboolobj;
        std::size_t nintinactive = nintcoll * fploopcount;
        std::size_t nintall = nintinactive + nintactive;
        std::size_t nboolactive = nintboolobj;
        std::size_t nboolinactive = 0;
        std::size_t nboolall = nboolinactive + nboolactive;

        //-----------------------------------------------------------------

        {
            // Test counting of parameters
            // Create a GTreeGenome object as a clone of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test = p_test_0->clone<GTreeGenome>();

            // Count the number of parameters and compare with the expected number
            CHECK(p_test->countParameters<double>(activityMode::ACTIVEONLY) == ndoubleactive);
            CHECK(p_test->countParameters<double>(activityMode::INACTIVEONLY) == ndoubleinactive);
            CHECK(p_test->countParameters<double>(activityMode::ALLPARAMETERS) == ndoubleall);
            CHECK(p_test->countParameters<std::int32_t>(activityMode::ACTIVEONLY) == nintactive);
            CHECK(
                p_test->countParameters<std::int32_t>(activityMode::INACTIVEONLY) == nintinactive
            );
            CHECK(p_test->countParameters<std::int32_t>(activityMode::ALLPARAMETERS) == nintall);
            CHECK(p_test->countParameters<bool>(activityMode::ACTIVEONLY) == nboolactive);
            CHECK(p_test->countParameters<bool>(activityMode::INACTIVEONLY) == nboolinactive);
            CHECK(p_test->countParameters<bool>(activityMode::ALLPARAMETERS) == nboolall);
        }

        //-----------------------------------------------------------------

        {
            // Check that streamline(activityMode::INACTIVEONLY) yields unchanged results before and after randomInit(activityMode::ACTIVEONLY)
            // Create two GTreeGenome objects as clones of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test_orig = p_test_0->clone<GTreeGenome>();
            std::shared_ptr<GTreeGenome> p_test_rand = p_test_0->clone<GTreeGenome>();

            // Randomly initialize active components of p_test2
            CHECK_NOTHROW(p_test_rand->randomInit(activityMode::ACTIVEONLY));

            std::vector<double> orig_d_inactive;
            std::vector<double> rand_d_inactive;

            std::vector<std::int32_t> orig_i_inactive;
            std::vector<std::int32_t> rand_i_inactive;

            std::vector<bool> orig_b_inactive;
            std::vector<bool> rand_b_inactive;

            // Extract the parameters
            CHECK_NOTHROW(
                p_test_orig->streamline<double>(orig_d_inactive, activityMode::INACTIVEONLY)
            );
            CHECK_NOTHROW(
                p_test_rand->streamline<double>(rand_d_inactive, activityMode::INACTIVEONLY)
            );
            CHECK_NOTHROW(
                p_test_orig->streamline<std::int32_t>(orig_i_inactive, activityMode::INACTIVEONLY)
            );
            CHECK_NOTHROW(
                p_test_rand->streamline<std::int32_t>(rand_i_inactive, activityMode::INACTIVEONLY)
            );
            CHECK_NOTHROW(
                p_test_orig->streamline<bool>(orig_b_inactive, activityMode::INACTIVEONLY)
            );
            CHECK_NOTHROW(
                p_test_rand->streamline<bool>(rand_b_inactive, activityMode::INACTIVEONLY)
            );

            // Check that the "inactive" vectors have the expected characteristics
            CHECK(orig_d_inactive.size() == ndoubleinactive);
            CHECK(orig_d_inactive == rand_d_inactive);
            CHECK(orig_i_inactive.size() == nintinactive);
            CHECK(orig_i_inactive == rand_i_inactive);
            CHECK(orig_b_inactive.size() == nboolinactive);
            CHECK(orig_b_inactive == rand_b_inactive);
        }

        //-----------------------------------------------------------------

        {
            // Check that streamline(activityMode::ACTIVEONLY) yields changed results after randomInit(activityMode::ACTIVEONLY)
            // Create a GTreeGenome object as a clone of p_test_0 for further usage
            // Create two GTreeGenome objects as clones of p_test_0 for further usage
            std::shared_ptr<GTreeGenome> p_test_orig = p_test_0->clone<GTreeGenome>();
            std::shared_ptr<GTreeGenome> p_test_rand = p_test_0->clone<GTreeGenome>();

            // Randomly initialize active components of p_test2
            CHECK_NOTHROW(p_test_rand->randomInit(activityMode::ACTIVEONLY));

            std::vector<double> orig_d_active;
            std::vector<double> rand_d_active;

            std::vector<std::int32_t> orig_i_active;
            std::vector<std::int32_t> rand_i_active;

            std::vector<bool> orig_b_active;
            std::vector<bool> rand_b_active;

            // Extract the parameters
            CHECK_NOTHROW(p_test_orig->streamline<double>(orig_d_active, activityMode::ACTIVEONLY));
            CHECK_NOTHROW(p_test_rand->streamline<double>(rand_d_active, activityMode::ACTIVEONLY));
            CHECK_NOTHROW(
                p_test_orig->streamline<std::int32_t>(orig_i_active, activityMode::ACTIVEONLY)
            );
            CHECK_NOTHROW(
                p_test_rand->streamline<std::int32_t>(rand_i_active, activityMode::ACTIVEONLY)
            );
            CHECK_NOTHROW(p_test_orig->streamline<bool>(orig_b_active, activityMode::ACTIVEONLY));
            CHECK_NOTHROW(p_test_rand->streamline<bool>(rand_b_active, activityMode::ACTIVEONLY));

            // Check that the "active" vectors' contents indeed differ
            CHECK(orig_d_active.size() == ndoubleactive);
            CHECK(rand_d_active.size() == ndoubleactive);
            CHECK(orig_d_active != rand_d_active);
            CHECK(orig_i_active.size() == nintactive);
            CHECK(rand_i_active.size() == nintactive);
            INFO(
                "orig_i_active: " << Gem::Common::vecToString(orig_i_active) << "\n"
                                  << "rand_i_active: " << Gem::Common::vecToString(rand_i_active)
                                  << "\n"
            );
            CHECK(orig_i_active != rand_i_active);
            CHECK(orig_b_active.size() == nboolactive);
            CHECK(rand_b_active.size() == nboolactive);

            // We do not compare the (single) boolean value here, as there are just
            // two distinct values it may assume, so the likelihood for identical values
            // and thus failure of this test is high.
            // CHECK(orig_b_active != rand_b_active);
        }

        //-----------------------------------------------------------------
    }

    //---------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw

    Gem::Common::condnotset(
        "GTreeGenome::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
void GTreeGenome::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the stateful base class'es function
    Gem::Common::GUniquePtrContainerT<GParameterBase>::specificTestsFailuresExpected_GUnitTests_();

    // no tests here yet

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw

    Gem::Common::condnotset(
        "GTreeGenome::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::Parameters */
