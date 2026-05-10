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

#include "geneva/GParameterSet.hpp"

#include <any>
#include <memory>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSet)                  // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::parameterset_processing_result) // NOLINT
namespace Gem::Geneva {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This constructor initializes the `parameterset_processing_result` object with
 * a raw fitness value. The transformed fitness is set to the same value as the raw fitness,
 * and the flag indicating that the transformed fitness has been set is set to false.
 *
 * @param raw_fitness The raw fitness value.
 */
parameterset_processing_result::parameterset_processing_result(const double raw_fitness)
  : raw_fitness_(raw_fitness)
  , transformed_fitness_(raw_fitness_)
  , transformed_fitness_set_(false) {
    /* nothing */
}

/******************************************************************************/
/**
 * This constructor initializes the `parameterset_processing_result` object with
 * both raw and transformed fitness values. It also sets the flag indicating that
 * the transformed fitness has been set.
 *
 * @param raw_fitness The raw fitness value.
 * @param transformed_fitness The transformed fitness value.
 */
parameterset_processing_result::parameterset_processing_result(
    const double raw_fitness,
    const double transformed_fitness
)
  : raw_fitness_(raw_fitness)
  , transformed_fitness_(transformed_fitness)
  , transformed_fitness_set_(true) {
    /* nothing */
}
/******************************************************************************/
/**
 * This constructor initializes the `parameterset_processing_result` object with
 * a raw fitness value and a function to transform the fitness. The transformed fitness
 * is calculated using the provided function. If the function is empty, an error is logged.
 *
 * @param raw_fitness The raw fitness value.
 * @param f A function to transform the raw fitness value.
 */
parameterset_processing_result::parameterset_processing_result(
    const double raw_fitness,
    std::function<double(double)> f
)
  : raw_fitness_(raw_fitness) {
    if(f) {
        transformed_fitness_ = f(raw_fitness_);
        transformed_fitness_set_ = true;
    }
    else {
        glogger << "In parameterset_processing_result(double, std::function<double(double)>)"
                << std::endl
                << GTERMINATION;
    }
}

/******************************************************************************/
/**
 * Access to the raw fitness
 */
double parameterset_processing_result::rawFitness() const {
    return raw_fitness_;
}

/******************************************************************************/
/**
 * Access to the transformed fitness
 */
double parameterset_processing_result::transformedFitness() const {
    return transformed_fitness_;
}

/******************************************************************************/
/**
     * Updates the transformed fitness using an external function
     */
void parameterset_processing_result::setTransformedFitnessWith(std::function<double(double)> f) {
    if(f) {
        transformed_fitness_ = f(raw_fitness_);
        transformed_fitness_set_ = true;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In parameterset_processing_result::setTransformedFitnessWith():" << std::endl
            << "Function object f is empty." << std::endl
        );
    }
}

/******************************************************************************/
/**
     * Sets the transformed fitness to a user-defined value
     */
void parameterset_processing_result::setTransformedFitnessTo(const double transformed_fitness) {
    transformed_fitness_ = transformed_fitness;
    transformed_fitness_set_ = true;
}

/******************************************************************************/
/**
     * Sets the transformed fitness to the same value as the raw fitness
     */
void parameterset_processing_result::setTransformedFitnessToRaw() {
    transformed_fitness_ = raw_fitness_;
    transformed_fitness_set_ = true;
}

/******************************************************************************/
/**
     * Checks whether the transformed fitness was set
     */
bool parameterset_processing_result::transformedFitnessSet() const {
    return transformed_fitness_set_;
}

/******************************************************************************/
/**
     * Resets the object and stores a new raw value in the class
     */
void parameterset_processing_result::reset(const double raw_fitness) {
    raw_fitness_ = raw_fitness;
    transformed_fitness_ = raw_fitness_;
    transformed_fitness_set_ = false;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw and transformed value in the class
 */
void parameterset_processing_result::reset(
    const double raw_fitness,
    const double transformed_fitness
) {
    raw_fitness_ = raw_fitness;
    transformed_fitness_ = transformed_fitness;
    transformed_fitness_set_ = true;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw value in the class and triggers recalculation of the transformed value
 */
void parameterset_processing_result::reset(
    const double raw_fitness,
    std::function<double(double)> f
) {
    if(f) {
        raw_fitness_ = raw_fitness;
        transformed_fitness_ = f(raw_fitness_);
        transformed_fitness_set_ = true;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In parameterset_processing_result::reset():" << std::endl
            << "Function object f is empty." << std::endl
        );
    }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * The default constructor. Using this constructor will result in a single
     * fitness criterion.
     */
GParameterSet::GParameterSet()
  : Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>(1) {
    /* nothing */
}

/******************************************************************************/
/**
     * Initialization with the number of fitness criteria
     */
GParameterSet::GParameterSet(const std::size_t n_fitness_criteria)
  : Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>(
        n_fitness_criteria
    ) {
    /* nothing */
}

/******************************************************************************/
/**
     * The copy constructor.
     *
     * @param cp A copy of another GParameterSet object
     */
GParameterSet::GParameterSet(GParameterSet const &cp)
  : GObject(cp)
  , G_Interface_Mutable(cp)
  , G_Interface_Rateable(cp)
  , Gem::Common::GPtrVectorT<GParameterBase, GObject>(cp)
  , Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>(cp)
  , best_past_primary_fitness_(cp.best_past_primary_fitness_)
  , n_stalls_(cp.n_stalls_)
  , maxmode_(cp.maxmode_)
  , assigned_iteration_(cp.assigned_iteration_)
  , validity_level_(cp.validity_level_)
  , eval_policy_(cp.eval_policy_)
  , sigmoid_steepness_(cp.sigmoid_steepness_)
  , sigmoid_extremes_(cp.sigmoid_extremes_)
  , max_unsuccessful_adaptions_(cp.max_unsuccessful_adaptions_)
  , max_retries_until_valid_(cp.max_retries_until_valid_)
  , n_adaptions_(cp.n_adaptions_) {
    // Copy the personality pointer over
    Gem::Common::copyCloneableSmartPointer(cp.pt_ptr_, pt_ptr_);
    // Make sure any constraints are copied over
    Gem::Common::copyCloneableSmartPointer(
        cp.individual_constraint_ptr_,
        individual_constraint_ptr_
    );
}

/******************************************************************************/
/**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * TODO: Compare GProcessingContainerT ?
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
void GParameterSet::compare_(
    GObject const &cp,
    Gem::Common::expectation const &e,
    double const & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParameterSet reference independent of this object and convert the pointer
    const GParameterSet *p_load =
        Gem::Common::g_convert_and_compare<GObject, GParameterSet>(cp, this);

    GToken token("GParameterSet", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GObject>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(this->data_cnt_, p_load->data_cnt_), token);
    // data is actually contained in a parent class
    compare_t(IDENTITY(best_past_primary_fitness_, p_load->best_past_primary_fitness_), token);
    compare_t(IDENTITY(n_stalls_, p_load->n_stalls_), token);
    compare_t(IDENTITY(maxmode_, p_load->maxmode_), token);
    compare_t(IDENTITY(assigned_iteration_, p_load->assigned_iteration_), token);
    compare_t(IDENTITY(validity_level_, p_load->validity_level_), token);
    compare_t(IDENTITY(eval_policy_, p_load->eval_policy_), token);
    compare_t(IDENTITY(pt_ptr_, p_load->pt_ptr_), token);
    compare_t(IDENTITY(individual_constraint_ptr_, p_load->individual_constraint_ptr_), token);
    compare_t(IDENTITY(sigmoid_steepness_, p_load->sigmoid_steepness_), token);
    compare_t(IDENTITY(sigmoid_extremes_, p_load->sigmoid_extremes_), token);
    compare_t(IDENTITY(max_unsuccessful_adaptions_, p_load->max_unsuccessful_adaptions_), token);
    compare_t(IDENTITY(max_retries_until_valid_, p_load->max_retries_until_valid_), token);
    compare_t(IDENTITY(n_adaptions_, p_load->n_adaptions_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
     * Swap another object's vector with ours. We need to set the dirty flag of both
     * individuals in this case.
     */
void GParameterSet::swap(GParameterSet &cp) {
    Gem::Common::GPtrVectorT<GParameterBase, GObject>::swap(cp.data_cnt_);
    this->mark_as_due_for_processing();
    cp.mark_as_due_for_processing();
}

/* ----------------------------------------------------------------------------------
     * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Allows to randomly initialize parameter members. This function is responsible
     * for setting the dirty flag, so overloaded randomInit_ functions do not need
     * to take care of this. Note though that overloads of randomInit_() need to take
     * care to indicate whether modifications were made.
     *
     * @return A boolean indicating whether modifications where made
     */
bool GParameterSet::randomInit(activityMode const &am) {
    bool modifications_made = this->randomInit_(am);

    if(modifications_made) {
        this->mark_as_due_for_processing();
    }

    return modifications_made;
}

/******************************************************************************/
/**
     * Allows to specify whether we want to work in maximization (maxMode::MAXIMIZE) or minimization
     * (maxMode::MINIMIZE) mode (the default). The idea is that GParameterSet, depending on the maxMode,
     * changes its evaluation in such a way that the optimization algorithm always sees a
     * minimization problem.
     *
     * @param mode An enum class which indicates whether we want to work in maximization or minimization mode
     */
void GParameterSet::setMaxMode(maxMode const &mode) {
    maxmode_ = mode;
}

/******************************************************************************/
/**
     * Transformation of the individual's parameter objects into a boost::property_tree object .
     * This is e.g. used in GExternalEvaluatorIndividual for the communication with external
     * evaluation programs.
     */
void GParameterSet::toPropertyTree(pt::ptree &ptr, std::string const &baseName) const {
#ifdef DEBUG
    // Check if the object is empty. If so, complain
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::toPropertyTree(): Error!" << std::endl
            << "Object is empty." << std::endl
        );
    }
#endif

    bool dirtyFlag = (Gem::Courtier::processingStatus::DO_PROCESS == this->getProcessingStatus());
    bool hasErrors = this->has_errors();

    double rawFitness = 0., transformedFitness = 0.;

    ptr.put(baseName + ".iteration", this->getAssignedIteration());
    ptr.put(baseName + ".isDirty", dirtyFlag);
    ptr.put(baseName + ".hasErrors", hasErrors);
    ptr.put(baseName + ".isValid", hasErrors || dirtyFlag ? false : this->isValid());
    ptr.put(baseName + ".type", std::string("GParameterSet"));

    // Loop over all parameter objects and ask them to add their data to our ptree object
    ptr.put(baseName + ".nVars", this->size());
    std::string base; // NOLINT(cppcoreguidelines-init-variables)
    std::size_t pos = 0;
    for(const auto &item_ptr : *this) {
        base = baseName + ".vars.var" + Gem::Common::to_string(pos);
        item_ptr->toPropertyTree(ptr, base);
        pos++;
    }

    // Output the transformation policy
    switch(this->getEvaluationPolicy()) {
    case evaluationPolicy::USESIMPLEEVALUATION:
        ptr.put(baseName + ".transformationPolicy", "USESIMPLEEVALUATION");
        break;

    case evaluationPolicy::USESIGMOID:
        ptr.put(baseName + ".transformationPolicy", "USESIGMOID");
        break;

    case evaluationPolicy::USEWORSTCASEFORINVALID:
        ptr.put(baseName + ".transformationPolicy", "USEWORSTCASEFORINVALID");
        break;
    }

    // Output all fitness criteria. We do not enforce re-calculation of the fitness here,
    // as the property is meant to capture the current state of the individual.
    // Check the "isDirty" tag, if you need to know whether the results are current.
    ptr.put(baseName + ".nResults", this->getNStoredResults());
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        rawFitness = (dirtyFlag || hasErrors) ? this->getWorstCase() : this->raw_fitness(i);
        transformedFitness =
            (dirtyFlag || hasErrors) ? this->getWorstCase() : this->transformed_fitness(i);

        base = baseName + ".results.result" + Gem::Common::to_string(i);
        ptr.put(base, transformedFitness);
        base = baseName + ".results.rawResult" + Gem::Common::to_string(i);
        ptr.put(base, rawFitness);
    }
}

/******************************************************************************/
/**
     * Transformation of the individual's parameter objects into a list of
     * comma-separated values and fitness plus possibly the validity
     *
     * @param  withNameAndType Indicates whether a list of names and types should be prepended
     * @param  withCommas Indicates, whether commas should be printed
     * @param  useRawFitness Indicates, whether the true fitness instead of the transformed fitness should be returned
     * @return A string holding the parameter values and possibly the types
     */
std::string GParameterSet::toCSV(
    bool withNameAndType,
    bool withCommas,
    bool useRawFitness,
    bool showValidity
) const {
    std::map<std::string, std::vector<double>> dData;
    std::map<std::string, std::vector<float>> fData;
    std::map<std::string, std::vector<std::int32_t>> iData;
    std::map<std::string, std::vector<bool>> bData;

    // Retrieve the parameter maps
    this->streamline<double>(dData);
    this->streamline<float>(fData);
    this->streamline<std::int32_t>(iData);
    this->streamline<bool>(bData);

    std::vector<std::string> varNames;
    std::vector<std::string> varTypes;
    std::vector<std::string> varValues;

    // Extract the data
    for(auto const &item : dData) {
        for(std::size_t pos = 0; pos < (item.second).size(); pos++) {
            if(withNameAndType) {
                varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
                varTypes.emplace_back("double");
            }
            varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
        }
    }

    for(auto const &item : fData) {
        for(std::size_t pos = 0; pos < (item.second).size(); pos++) {
            if(withNameAndType) {
                varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
                varTypes.emplace_back("float");
            }
            varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
        }
    }

    for(auto const &item : iData) {
        for(std::size_t pos = 0; pos < (item.second).size(); pos++) {
            if(withNameAndType) {
                varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
                varTypes.emplace_back("int32");
            }
            varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
        }
    }

    for(auto const &item : bData) {
        for(std::size_t pos = 0; pos < (item.second).size(); pos++) {
            if(withNameAndType) {
                varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
                varTypes.emplace_back("bool");
            }
            varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
        }
    }

    // Note: The following will output the string "dirty" if the individual is in a "dirty" state
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        if(withNameAndType) {
            varNames.push_back(std::string("Fitness_") + Gem::Common::to_string(i));
            varTypes.emplace_back("double");
        }
        if(this->is_processed()) {
            // The individual has already been evaluated
            if(useRawFitness) {
                varValues.push_back(Gem::Common::to_string(this->raw_fitness(i)));
            }
            else {
                // Output potentially transformed fitness
                varValues.push_back(Gem::Common::to_string(this->transformed_fitness(i)));
            }
        }
        else {
            // No evaluation was performed so far
            if(this->has_errors()) {
                varValues.emplace_back("has_errors");
            }
            else {
                // "only" dirty / unevaluated
                varValues.emplace_back("dirty");
            }
        }
    }

    if(showValidity) {
        if(withNameAndType) {
            varNames.emplace_back("validity");
            varTypes.emplace_back("bool");
        }

        if(this->is_processed()) {
            // The individual has already been evaluated
            varValues.push_back(Gem::Common::to_string(this->isValid()));
        }
        else {
            varValues.push_back(Gem::Common::to_string(false));
        }
    }

    // Transfer the data into the result string
    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)
    std::vector<std::string>::const_iterator s_it;
    if(withNameAndType) {
        for(s_it = varNames.begin(); s_it != varNames.end(); ++s_it) {
            result << *s_it;
            if(s_it + 1 != varNames.end()) {
                result << (withCommas ? ",\t" : "\t");
            }
        }
        result << std::endl;

        for(s_it = varTypes.begin(); s_it != varTypes.end(); ++s_it) {
            result << *s_it;
            if(s_it + 1 != varTypes.end()) {
                result << (withCommas ? ",\t" : "\t");
            }
        }
        result << std::endl;
    }

    for(s_it = varValues.begin(); s_it != varValues.end(); ++s_it) {
        result << *s_it;
        if(s_it + 1 != varValues.end()) {
            result << (withCommas ? ",\t" : "\t");
        }
    }
    result << std::endl;

    return result.str();
}

/******************************************************************************/
/**
     * Prevent shadowing of std::vector<GParameterBase>::at()
     *
     * @param pos The position of the item we aim to retrieve from the std::vector<GParameterBase>
     * @return The item we aim to retrieve from the std::vector<GParameterBase>
     */
Gem::Common::GPtrVectorT<GParameterBase, GObject>::reference
GParameterSet::at(std::size_t const &pos) {
    return Gem::Common::GPtrVectorT<GParameterBase, GObject>::at(pos);
}

/* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Checks whether this object is better than a given set of evaluations. This
     * function compares "real" boundaries with evaluations, hence we use "raw"
     * measurements here instead of transformed measurements.
     */
bool GParameterSet::isGoodEnough(std::vector<double> const &boundaries) {
#ifdef DEBUG
    // Does the number of fitness criteria match the number of boundaries ?
    if(boundaries.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::isGoodEnough(): Error!" << std::endl
            << "Number of boundaries does not match number of fitness criteria" << std::endl
        );
    }

    // Has the individual been processed
    if(not this->is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::isGoodEnough(): Error!" << std::endl
            << "Trying to compare fitness values although the individual isn't processed"
            << std::endl
        );
    }
#endif /* DEBUG */

    // Check the fitness values. If we find at least one
    // which is worse than the one supplied by the boundaries
    // vector, then this individual fails the test
    if(maxMode::MAXIMIZE == this->getMaxMode()) {
        // Maximization
        for(std::size_t i = 0; i < boundaries.size(); i++) {
            if(this->raw_fitness(i) < boundaries.at(i)) {
                return false;
            }
        }
    }
    else {
        // maxMode::MINIMIZE
        for(std::size_t i = 0; i < boundaries.size(); i++) {
            if(this->raw_fitness(i) > boundaries.at(i)) {
                return false;
            }
        }
    }

    // All fitness values are better than those supplied by boundaries
    return true;
}

/******************************************************************************/
/**
     * Retrieval of a suitable position for cross over inside of a vector
     *
     * @param lower The lower (inclusive) boundary for retrieval of a cross-over position
     * @param upper The upper (exclusive) boundary for retrieval of a cross-over position
     * @return A suitable cross-over position in the range [lower, upper[
     */
std::size_t GParameterSet::getCrossOverPos(const std::size_t lower, const std::size_t upper) {
    // Make sure the boundaries are suitable.
    assert(lower > 0);
    // Should be at least 1, or a cross-over doesn't make sense
    assert(upper > lower);

    return uniform_int_(
        gr_,
        std::uniform_int_distribution<std::size_t>::param_type(lower, upper - 1)
    );
}

/******************************************************************************/
/**
     * Perform a fusion operation between this object and another.
     */
std::shared_ptr<GParameterSet>
GParameterSet::crossOverWith(std::shared_ptr<GParameterSet> const &cp) const {
    // Create a copy of this object
    std::shared_ptr<GParameterSet> this_cp = this->GObject::clone<GParameterSet>();

    // Extract all data items
    std::vector<double> this_double_cnt, cp_double_cnt;
    std::vector<float> this_float_cnt, cp_float_cnt;
    std::vector<bool> this_bool_cnt, cp_bool_cnt;
    std::vector<std::int32_t> this_int_cnt, cp_int_cnt;

    this_cp->streamline(this_double_cnt);
    this_cp->streamline(this_float_cnt);
    this_cp->streamline(this_bool_cnt);
    this_cp->streamline(this_int_cnt);

    cp->streamline(cp_double_cnt);
    cp->streamline(cp_float_cnt);
    cp->streamline(cp_bool_cnt);
    cp->streamline(cp_int_cnt);

#ifdef DEBUG
    // Do some error checking
    if(this_double_cnt.size() != cp_double_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
            << "Got invalid sizes (double): " << this_double_cnt.size() << " / "
            << cp_double_cnt.size() << std::endl
        );
    }
    if(this_float_cnt.size() != cp_float_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
            << "Got invalid sizes (float): " << this_float_cnt.size() << " / "
            << cp_float_cnt.size() << std::endl
        );
    }
    if(this_bool_cnt.size() != cp_bool_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
            << "Got invalid sizes (bool): " << this_bool_cnt.size() << " / " << cp_bool_cnt.size()
            << std::endl
        );
    }
    if(this_int_cnt.size() != cp_int_cnt.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
            << "Got invalid sizes (std::int32_t): " << this_int_cnt.size() << " / "
            << cp_int_cnt.size() << std::endl
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
void GParameterSet::updateAdaptorsOnStall(const std::uint32_t nStalls) {
    for(auto const &item_ptr : *this) {
        item_ptr->updateAdaptorsOnStall(nStalls);
    }
}

/******************************************************************************/
/**
     * Retrieves information from adaptors with a given property
     *
     * @param adaoptorName The name of the adaptor to be queried
     * @param property The property for which information is sought
     * @param data A vector, to which the properties should be added
     */
void GParameterSet::queryAdaptor(
    std::string const &adaptorName,
    std::string const &property,
    std::vector<std::any> &data
) const {
    for(const auto &item_ptr : *this) {
        item_ptr->queryAdaptor(adaptorName, property, data);
    }
}

/******************************************************************************/
/**
     * Retrieves parameters relevant for the evaluation from another GParameterSet.
     * NOTE: The other parameter set will be an empty shell afterwards. The function may
     * only be called for "clean" foreign parameter sets
     */
void GParameterSet::cannibalize(GParameterSet &cp) {
    // Check whether the "foreign" entity is processed
    if(cp.is_due_for_processing() || cp.has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::cannibalize(const GParameterSet& cp)" << std::endl
            << "cp isn't processed or has errors" << std::endl
        );
    }

    // Make sure we have no local parameters
    this->clear();

    // Copy all "foreign" parameters over
    for(const auto &t_ptr : cp) {
        this->push_back(t_ptr);
    }

    // Empty the foreign GParmeterSet object
    cp.clear();

    // Set our own fitness according to the foreign individual. This will also
    // clear our local dirty flag (if set).
    this->setFitness_(cp.raw_fitness_vec());
}

/******************************************************************************/
/**
     * The adaption interface. Triggers adaption of the individual, using each parameter object's
     * adaptor. Sets the dirty flag, as the parameters have been changed. This facility is mostly
     * used in Evolutionary Algorithms and Simulated Annealing. Other algorithms, such as
     * PSO and Gradient Descents, may choose to change parameters directly. Adaptions will be performed
     * until actual changes were done to the object AND a valid parameter set was found.
     */
std::size_t GParameterSet::adapt() {
    std::size_t nAdaptionAttempts = 0;
    std::size_t nAdaptions = 0;
    // This is a measure of the "effective" adaption probability
    std::size_t nInvalidAdaptions = 0;
    double validity = 0;

    // Perform adaptions until a valid solution was find. In the context
    // of evolutionary algorithms, this process is indeed equivalent to
    // a larger population, if invalid solutions were produced. The downside
    // may be, that the algorithm moves closer to MUPLUSNU. Thus, if you find
    // yourself stuck in local optima too often, consider setting max_retries_until_valid_
    // to 0, using the appropriate function.
    while(true) {
        // Make sure at least one modification is performed. E.g., for low
        // adaption probabilities combined with few parameters, it may happen
        // otherwise that individuals remain unchanged after a call to adapt()
        while(true) {
            // Try again if no adaption has taken place
            // Perform the actual adaption; Terminate, if at least one adaption was performed
            if((nAdaptions = this->customAdaptions()) >
               0) // NOLINT(bugprone-assignment-in-if-condition)
            {
                break;
            }

            // Terminate, if the maximum number of adaptions has been exceeded
            if(max_unsuccessful_adaptions_ > 0 &&
               ++nAdaptionAttempts > max_unsuccessful_adaptions_) {
                break;
            }
        }

        if(this->parameterSetFulfillsConstraints(validity) ||
           ++nInvalidAdaptions > max_retries_until_valid_) {
            break;
        }
    }

    // Make sure the individual is re-evaluated when fitness(...) is called next time
    if(nAdaptions > 0) {
        this->mark_as_due_for_processing();
    }

    // Store the number of adaptions for later use and let the audience know
    return (n_adaptions_ = nAdaptions);
}

/* ----------------------------------------------------------------------------------
     * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Retrieves the stored raw fitness with a given id
     */
double GParameterSet::raw_fitness_(const std::size_t id) const {
    return this->getStoredResult(id).rawFitness();
}

/******************************************************************************/
/**
     * Retrieves the stored transformed fitness with a given id
     */
double GParameterSet::transformed_fitness_(const std::size_t id) const {
    return this->getStoredResult(id).transformedFitness();
}

/******************************************************************************/
/**
     * Returns all raw fitness results in a std::vector
     */
std::vector<double> GParameterSet::raw_fitness_vec_() const {
    std::size_t nFitnessCriteria = this->getNStoredResults();
    std::vector<double> resultVec;

    for(std::size_t i = 0; i < nFitnessCriteria; i++) {
        resultVec.push_back(this->raw_fitness(i));
    }

    return resultVec;
}

/******************************************************************************/
/**
     * Returns all transformed fitness results in a std::vector
     */
std::vector<double> GParameterSet::transformed_fitness_vec_() const {
    std::size_t nFitnessCriteria = this->getNStoredResults();
    std::vector<double> resultVec;

    for(std::size_t i = 0; i < nFitnessCriteria; i++) {
        resultVec.push_back(this->transformed_fitness(i));
    }

    return resultVec;
}

/******************************************************************************/
/**
     * Register another result value of the fitness calculation. Multiple fitness
     * criteria are used in multi-criterion optimization. fitnessCalculation() returns
     * the main fitness value, but may also add further, secondary results. Note that,
     * whether these are actually used, depends on the optimization algorithm being
     * used. Transformation for the second fitness value will be done in the process_()
     * function. You may store the primary fitness value with this function as well.
     * As the primary (raw) value is however also returned by fitnessCalculation() and
     * integrated into the list of results, this is redundant.
     *
     * @param id The position of the fitness criterion (must be >= 0 !)
     * @param value The fitness value to be registered
     */
void GParameterSet::setResult(const std::size_t id, const double value) {
#ifdef DEBUG
    if(id >= this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::setResult(...): Error!" << std::endl
            << "Invalid position in vector: " << id << " (expected min 0 and max "
            << this->getNStoredResults() - 1 << ")" << std::endl
        );
    }
#endif /* DEBUG */

    this->modifyStoredResult(id).reset(value);
}

/******************************************************************************/
/**
     * Determines whether more than one fitness criterion is present for this individual
     *
     * @return A boolean indicating whether more than one target function is present
     */
bool GParameterSet::hasMultipleFitnessCriteria() const {
    return this->getNStoredResults() > 1;
}

/******************************************************************************/
/**
     * Retrieve the fitness tuple at a given evaluation position.
     */
std::tuple<double, double> GParameterSet::getFitnessTuple(const std::uint32_t id) const {
    return std::make_tuple<double, double>(this->raw_fitness(id), this->transformed_fitness(id));
}

/******************************************************************************/
/**
     * Allows to retrieve the maxmode_ parameter
     *
     * @return The current value of the maxmode_ parameter
     */
maxMode GParameterSet::getMaxMode() const {
    return maxmode_;
}

/* ----------------------------------------------------------------------------------
     * Retrieval is tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/***************************************************************************/
/**
     * Helper function that emits the worst case value depending on whether maximization
     * or minimization is performed.
     *
     * @return The worst case value, depending on maximization or minimization
     */
double GParameterSet::getWorstCase() const {
    return (
        (maxMode::MAXIMIZE == this->getMaxMode()) ? boost::numeric::bounds<double>::lowest()
                                                  : boost::numeric::bounds<double>::highest()
    );
}

/******************************************************************************/
/**
     * Retrieves the best possible evaluation result, depending on whether we are in
     * maximization or minimization mode
     */
double GParameterSet::getBestCase() const {
    return (
        (maxMode::MAXIMIZE == this->getMaxMode()) ? boost::numeric::bounds<double>::highest()
                                                  : boost::numeric::bounds<double>::lowest()
    );
}

/******************************************************************************/
/**
     * Retrieves the steepness_ variable (used for the sigmoid transformation)
     */
double GParameterSet::getSteepness() const {
    return sigmoid_steepness_;
}

/******************************************************************************/
/**
     * Sets the steepness variable (used for the sigmoid transformation)
     */
void GParameterSet::setSteepness(const double steepness) {
    if(steepness <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::setSteepness(double steepness): Error!" << std::endl
            << "Invalid value of steepness parameter: " << steepness << std::endl
        );
    }

    sigmoid_steepness_ = steepness;
}

/******************************************************************************/
/**
     * Retrieves the barrier_ variable (used for the sigmoid transformation)
     */
double GParameterSet::getBarrier() const {
    return sigmoid_extremes_;
}

/******************************************************************************/
/**
     * Sets the barrier variable (used for the sigmoid transformation)
     */
void GParameterSet::setBarrier(const double barrier) {
    if(barrier <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::setBarrier(double barrier): Error!" << std::endl
            << "Invalid value of barrier parameter: " << barrier << std::endl
        );
    }

    sigmoid_extremes_ = barrier;
}

/******************************************************************************/
/**
     * Sets the maximum number of adaption attempts that may pass without
     * actual modifications. Setting this to 0 disables this check. You should only
     * do this if you are sure that an adaption will eventually happen. Otherwise
     * you would get an endless loop.
     */
void GParameterSet::setMaxUnsuccessfulAdaptions(const std::size_t maxUnsuccessfulAdaptions) {
    max_unsuccessful_adaptions_ = maxUnsuccessfulAdaptions;
}

/******************************************************************************/
/**
     * Retrieves the maximum number of adaption attempts that may pass without
     * actual modifications
     */
std::size_t GParameterSet::getMaxUnsuccessfulAdaptions() const {
    return max_unsuccessful_adaptions_;
}

/******************************************************************************/
/**
     * Allows to set the maximum number of retries during the adaption of individuals
     * until a valid individual was found. Setting this value to 0 will disable retries.
     */
void GParameterSet::setMaxRetriesUntilValid(const std::size_t maxRetriesUntilValid) {
    max_retries_until_valid_ = maxRetriesUntilValid;
}

/******************************************************************************/
/**
     * Allows to retrieve the current maximum number of retries during the adaption of
     * individuals until a valid individual was found.
     */
std::size_t GParameterSet::getMaxRetriesUntilValid() const {
    return max_retries_until_valid_;
}

/******************************************************************************/
/**
     * Retrieves the number of adaptions performed during the last call to adapt()
     * (or 0, if no adaptions were performed so far).
     */
std::size_t GParameterSet::getNAdaptions() const {
    return n_adaptions_;
}

/******************************************************************************/
/**
     * Allows to set the current iteration of the parent optimization algorithm.
     *
     * @param parentAlgIteration The current iteration of the optimization algorithm
     */
void GParameterSet::setAssignedIteration(std::uint32_t const &parentAlgIteration) {
    assigned_iteration_ = parentAlgIteration;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Gives access to the parent optimization algorithm's iteration
     *
     * @return The parent optimization algorithm's current iteration
     */
std::uint32_t GParameterSet::getAssignedIteration() const {
    return assigned_iteration_;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Allows to specify the number of optimization cycles without improvement of the primary fitness criterion
     *
     * @param nStalls The number of optimization cycles without improvement in the parent algorithm
     */
void GParameterSet::setNStalls(std::uint32_t const &nStalls) {
    n_stalls_ = nStalls;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion
     *
     * @return The number of optimization cycles without improvement in the parent algorithm
     */
std::uint32_t GParameterSet::getNStalls() const {
    return n_stalls_;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Retrieves the current personality of this individual
     *
     * @return An identifier for the current personality of this object
     */
std::string GParameterSet::getPersonality() const {
    if(pt_ptr_) {
        return pt_ptr_->name();
    }
    else {
        return std::string("PERSONALITY_NONE");
    }
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Allows to check whether random crashs of individuals are enabled
     */
std::tuple<bool, double> GParameterSet::getRandomCrash() const {
    return std::tuple<bool, double>{useRandomCrash_, randomCrashProb_};
};

/******************************************************************************/
/**
     * Allows to enable random crashs of individuals for testing purposes
     */
void GParameterSet::setRandomCrash(const bool useRandomCrash, const double crashProb) {
    // Check that the crash probability is in the allowed value range
    Gem::Common::checkRangeCompliance(crashProb, 0., 1., "GParameterSet::setRandomCrash()");

    // Set the value as demanded
    useRandomCrash_ = useRandomCrash;
    randomCrashProb_ = crashProb;
}

/******************************************************************************/
/**
     * This function returns the current personality traits base pointer. Note that there
     * is another version of the same command that does on-the-fly conversion of the
     * personality traits to the derived class.
     *
     * @return A shared pointer to the personality traits base class
     */
std::shared_ptr<GPersonalityTraits> GParameterSet::getPersonalityTraits() {
#ifdef DEBUG
    // Do some error checking
    if(not pt_ptr_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::getPersonalityTraits():" << std::endl
            << "Pointer to personality traits object is empty." << std::endl
        );
    }
#endif

    return pt_ptr_;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * Tested in GParameterSet::specificTestsFailuresExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Sets the current personality of this individual
     *
     * @param gpt A pointer to an object representing the new personality of this object
     */
void GParameterSet::setPersonality(std::shared_ptr<GPersonalityTraits> gpt) {
    // Make sure we haven't been given an empty pointer
    if(not gpt) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::setPersonality(): Error!" << std::endl
            << "Received empty personality traits pointer" << std::endl
        );
    }

    // Add the personality traits object to our local pointer
    pt_ptr_ = gpt;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Resets the current personality to PERSONALITY_NONE
     */
void GParameterSet::resetPersonality() {
    pt_ptr_.reset();
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Retrieves the mnemonic used for the optimization of this object
     */
std::string GParameterSet::getMnemonic() const {
    if(pt_ptr_) {
        return pt_ptr_->getMnemonic();
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::getMnemonic():" << std::endl
            << "Pointer to personality traits object is empty." << std::endl
        );
    }

    // Make the compiler happy
    return std::string();
}

/******************************************************************************/
/**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
void GParameterSet::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GObject::addConfigurationOptions_(gpb);

    // Add local data
    gpb.registerFileParameter<evaluationPolicy>(
        "evalPolicy" // The name of the variable
        ,
        Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION
        // The default value
        ,
        [this](const evaluationPolicy ep) { this->setEvaluationPolicy(ep); }
    ) << "Specifies which strategy should be used to calculate the evaluation:"
      << std::endl
      << "0 (a.k.a. USESIMPLEEVALUATION): Always call the evaluation function, even for invalid "
         "solutions"
      << std::endl
      << "1 (a.k.a. USEWORSTCASEFORINVALID) : Assign the worst possible value to our fitness and "
         "evaluate only valid solutions"
      << std::endl
      << "2 (a.k.a. USESIGMOID): Assign a multiple of validity_level_ and sigmoid barrier to "
         "invalid solutions, apply a sigmoid function to valid evaluations"
      << std::endl;

    gpb.registerFileParameter<double>(
        "steepness" // The name of the variable
        ,
        Gem::Geneva::FITNESSSIGMOIDSTEEPNESS // The default value
        ,
        [this](const double ss) { this->setSteepness(ss); }
    ) << "When using a sigmoid function to transform the individual's fitness,"
      << std::endl
      << "this parameter influences the steepness of the function at the center of the sigmoid."
      << std::endl
      << "The parameter must have a value > 0.";

    gpb.registerFileParameter<double>(
        "barrier" // The name of the variable
        ,
        Gem::Geneva::WORSTALLOWEDVALIDFITNESS // The default value
        ,
        [this](const double barrier) { this->setBarrier(barrier); }
    ) << "When using a sigmoid function to transform the individual's fitness,"
      << std::endl
      << "this parameter sets the upper/lower boundary of the sigmoid." << std::endl
      << "The parameter must have a value > 0.;";

    gpb.registerFileParameter<std::size_t>(
        "maxUnsuccessfulAdaptions" // The name of the variable
        ,
        DEFMAXUNSUCCESSFULADAPTIONS // The default value
        ,
        [this](const std::size_t mua) { this->setMaxUnsuccessfulAdaptions(mua); }
    ) << "The maximum number of unsuccessful adaptions in a row for one call to adapt()";

    gpb.registerFileParameter<std::size_t>(
        "maxRetriesUntilValid" // The name of the variable
        ,
        DEFMAXRETRIESUNTILVALID // The default value
        ,
        [this](const std::size_t mruv) { this->setMaxRetriesUntilValid(mruv); }
    ) << "The maximum allowed number of retries during the"
      << std::endl
      << "adaption of individuals until a valid solution was found" << std::endl
      << "A parameter set is considered to be \"valid\" if" << std::endl
      << "it passes all validity checks;";

    // Add local data
    gpb.registerFileParameter<maxMode>(
        "maxmode" // The name of the variable
        ,
        maxMode::MINIMIZE // The default value
        ,
        [this](const maxMode mm) { this->setMaxMode(mm); }
    ) << "Specifies whether the individual should be maximized (1) or minimized (0)"
      << std::endl
      << "Note that minimization is the by far most common option.";

    gpb.registerFileParameter<bool, double>(
        "useRandomCrash" // The name of the variable
        ,
        "randomCrashProb",
        GPS_DEF_USE_RANDOMCRASH // The default value
        ,
        GPS_DEF_RANDOMCRASHPROB,
        [this](const bool use_rc, const double rc_prob) { this->setRandomCrash(use_rc, rc_prob); },
        "randomCrashParameters"
    ) << "Indicates whether random crashes should occur for debugging purposes"
      << std::endl
      << Gem::Common::nextComment() << "The probability of a random crash to occur";
}

/******************************************************************************/
/**
     * Emits a name for this class / object
     *
     * @return The name of this class / object
     */
std::string GParameterSet::name_() const {
    return std::string("GParameterSet");
}

/******************************************************************************/
/**
     * Check how valid a given solution is
     *
     * @return The validity level of this solution
     */
double GParameterSet::getValidityLevel() const {
    return validity_level_;
}

/******************************************************************************/
/**
     * @return A boolean indicating, whether all constraints were fulfilled
     */
bool GParameterSet::constraintsFulfilled() const {
    if(validity_level_ <= 1.)
        return true;
    else
        return false;
}

/******************************************************************************/
/**
     * Allows to register a constraint with this individual. Note that the constraint
     * object will be cloned.
     */
void GParameterSet::registerConstraint(
    std::shared_ptr<GPreEvaluationValidityCheckT<GParameterSet>> c_ptr
) {
    if(not c_ptr) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::registerConstraint(): Error!" << std::endl
            << "Tried to register empty constraint object" << std::endl
        );
    }

    // We store clones, so individual objects do not share the same object
    individual_constraint_ptr_ =
        c_ptr->GObject::clone<GPreEvaluationValidityCheckT<GParameterSet>>();
}

/******************************************************************************/
/**
     * Allows to set the policy to use in case this individual represents an invalid solution
     */
void GParameterSet::setEvaluationPolicy(const evaluationPolicy evalPolicy) {
    eval_policy_ = evalPolicy;
}

/******************************************************************************/
/**
     * Allows to retrieve the current policy in case this individual represents an invalid solution
     */
evaluationPolicy GParameterSet::getEvaluationPolicy() const {
    return eval_policy_;
}

/******************************************************************************/
/**
     * Checks whether this solution is valid. This function is meant to be called
     * for "clean" individuals only and will throw when called for unprocessed or
     * erroneous individuals.
     */
bool GParameterSet::isValid() const {
#ifdef DEBUG
    if(this->is_due_for_processing() || this->has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::isValid():" << std::endl
            << "Function was called for unprocessed or erroneous individual" << std::endl
        );
    }
#endif

    if(validity_level_ <= 1.) {
        return true;
    }
    else {
        return false;
    }
}

/******************************************************************************/
/**
     * Checks whether this solution is invalid
     */
bool GParameterSet::isInValid() const {
    return not this->isValid();
}

/******************************************************************************/
/**
     * Allows to set the globally best known primary fitness so far
     *
     * @param bnf The best known primary fitness so far
     */
void GParameterSet::setBestKnownPrimaryFitness(const std::tuple<double, double> &bnf) {
    best_past_primary_fitness_ = bnf;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Retrieves the value of the globally best known primary fitness so far
     *
     * @return The best known primary fitness so far
     */
std::tuple<double, double> GParameterSet::getBestKnownPrimaryFitness() const {
    return best_past_primary_fitness_;
}

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Performs all necessary (remote-)processing steps for this object.
     */
void GParameterSet::process_(const std::vector<parameterset_processing_result> &res_vec) {
#ifdef DEBUG
    //---------------------------------------------
    // Crash if we have been asked to (only active in DEBUG mode)
    if(useRandomCrash_) {
        std::uniform_real_distribution<double> dist01{0., 1.};
        if(dist01(this->gr_) <= randomCrashProb_) {
            glogger << "GParameterSet is performing random crash for debugging purposes"
                    << std::endl
                    << std::endl
                    << GLOGGING;

            throw;
        }
    }
#endif

    // Find out, whether this is a valid solution
    if(this->parameterSetFulfillsConstraints(validity_level_)
       // Needs to be called first, or else the validity_level_ will not be filled
       || evaluationPolicy::USESIMPLEEVALUATION == eval_policy_) {
        // Trigger actual fitness calculation using the user-supplied function. This will
        // also register any secondary "raw" fitness values used in multi-criterion optimization.
        // Transformation of values is taken care of below.
        double main_raw_result = 0.;

        try {
            if(not res_vec.empty()) {
                // Check that sizes match
                if(res_vec.size() != this->getNStoredResults()) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "In GParameterSet::process_ : Error!" << std::endl
                        << "res_vec has invalid size. Got " << res_vec.size() << std::endl
                        << "Expected " << this->getNStoredResults() << std::endl
                    );
                }

                // Just assign the main *raw* result
                main_raw_result = res_vec.begin()->rawFitness();

                // Extract all additional *raw* results. Then we are on par with fitnessCalculation()
                std::size_t pos = 0;
                for(const auto &res : res_vec) {
                    if(pos == 0)
                        continue; // Skip the main raw result

                    this->setResult(pos, res_vec.at(pos).rawFitness());

                    pos++;
                }
            }
            else {
                // If we are dealing with multiple fitness criteria,
                // then fitnessCalculation() will set additional raw values
                main_raw_result = this->fitnessCalculation();
            }
        }
        catch(...) {
            // Make sure we invalidate all fitness values, if an exception was thrown
            this->setAllFitnessTo(this->getWorstCase());

            // Rethrow the exception
            throw;
        }

        // Make sure the main result is stored
        // TODO: result setting should be done in the parent class'es process()-function, not in process_()
        this->setResult(0, main_raw_result);
        // TODO: When using multiple criteria: Are we setting the other transformed results also to raw?
        this->modifyStoredResult(0).setTransformedFitnessToRaw();

        // Take care of erroneous calculations, flagged by the user. It is assumed here that marking
        // entire solutions as invalid after the evaluation happens relatively rarely so that a flat
        // "worst" quality surface for such solutions does not hinder progress of the optimization
        // procedure too much
        if(this->error_flagged_by_user()) {
            // has the user indicated a problem without throwing an error ?
            // Fill the raw and transformed vectors with the worst case scenario.
            this->setAllFitnessTo(this->getWorstCase());
        }
        else {
            // So this is a valid solution!
            for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
                if(evaluationPolicy::USESIGMOID == eval_policy_) {
                    // Update the fitness value to use sigmoidal values
                    this->modifyStoredResult(i).setTransformedFitnessWith(
                        [this](const double rawValue) {
                            return Gem::Common::grational_sigmoid(
                                rawValue,
                                this->sigmoid_extremes_,
                                this->sigmoid_steepness_
                            );
                        }
                    );
                }
                else {
                    // All other transformation policies use the same value for the transformed fitness as a (valid) raw fitness
                    this->modifyStoredResult(i).setTransformedFitnessToRaw();
                }
            }
        }
    }
    else {
        // Some constraints were violated. Act on the chosen policy
        if(evaluationPolicy::USEWORSTCASEFORINVALID == eval_policy_) {
            this->setAllFitnessTo(this->getWorstCase());
        }
        else if(evaluationPolicy::USESIGMOID == eval_policy_) {
            double uniformFitnessValue = 0.;
            if(maxMode::MAXIMIZE == this->getMaxMode()) {
                // maximize
                if(boost::numeric::bounds<double>::highest() == validity_level_) {
                    uniformFitnessValue = this->getWorstCase();
                }
                else {
                    uniformFitnessValue = -validity_level_ * sigmoid_extremes_;
                }
            }
            else {
                // minimize
                if(boost::numeric::bounds<double>::highest() == validity_level_) {
                    uniformFitnessValue = this->getWorstCase();
                }
                else {
                    uniformFitnessValue = validity_level_ * sigmoid_extremes_;
                }
            }

            this->setAllFitnessTo(this->getWorstCase(), uniformFitnessValue);
        }
    }
}

/******************************************************************************/
/**
     * Loads the data of another GParameterSet object, camouflaged as a GObject.
     *         *
     * @param cp A copy of another GParameterSet object, camouflaged as a GObject
     */
void GParameterSet::load_(const GObject *cp) {
    // Check that we are dealing with a GParameterSet reference independent of this object and convert the pointer
    const GParameterSet *p_load =
        Gem::Common::g_convert_and_compare<GObject, GParameterSet>(cp, this);

    // Load the parent class'es data
    GObject::load_(cp);
    Gem::Common::GPtrVectorT<GParameterBase, GObject>::operator=(*p_load);
    Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>::load_pc(
        p_load
    );

    // and then our local data
    best_past_primary_fitness_ = p_load->best_past_primary_fitness_;
    n_stalls_ = p_load->n_stalls_;
    maxmode_ = p_load->maxmode_;
    assigned_iteration_ = p_load->assigned_iteration_;
    validity_level_ = p_load->validity_level_;
    eval_policy_ = p_load->eval_policy_;
    sigmoid_steepness_ = p_load->sigmoid_steepness_;
    sigmoid_extremes_ = p_load->sigmoid_extremes_;
    max_unsuccessful_adaptions_ = p_load->max_unsuccessful_adaptions_;
    max_retries_until_valid_ = p_load->max_retries_until_valid_;
    n_adaptions_ = p_load->n_adaptions_;

    Gem::Common::copyCloneableSmartPointer(p_load->pt_ptr_, pt_ptr_);
    Gem::Common::copyCloneableSmartPointer(
        p_load->individual_constraint_ptr_,
        individual_constraint_ptr_
    );
}

/******************************************************************************/
/**
     * Allows to randomly initialize parameter members. This function may be overloaded
     * by derived classes, but should be called by them. This function recursively initializes
     * parameters randomly.
     *
     * @return A boolean indicating whether modifications where made
     */
bool GParameterSet::randomInit_(activityMode const &am) {
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

/* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/**********************************************************************************/
/**
     * The actual adaption operations. Easy, as we know that all objects
     * in this collection must implement the adapt() function, as they are
     * derived from the GMutableI class / interface.
     */
std::size_t GParameterSet::customAdaptions() {
    std::size_t nAdaptions = 0;
    for(const auto &par_ptr : *this) {
        nAdaptions += par_ptr->adapt(gr_);
    }

    return nAdaptions;
}

/* ----------------------------------------------------------------------------------
     * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

/******************************************************************************/
/**
     * Sets the fitness to a given set of values and clears the dirty flag. This is meant
     * to be used by external methods of performing the actual evaluation, such as the
     * OpenCL-Consumer. The fitness vector is interpreted as raw fitness values, and
     * transformed fitness values are calculated as needed.
     *
     * @param f_cnt A vector of raw fitness values
     */
void GParameterSet::setFitness_(std::vector<double> const &f_cnt) {
#ifdef DEBUG
    if(f_cnt.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::setFitness_(...): Error!" << std::endl
            << "Invalid size of fitness vector: " << std::endl
            << f_cnt.size() << ", expected: " << this->getNStoredResults() << std::endl
        );
    }
#endif /* DEBUG */

    // Find out, whether this is a valid solution
    if(this->parameterSetFulfillsConstraints(validity_level_)
       // Needs to be called first, or else the validity_level_ will not be filled
       || evaluationPolicy::USESIMPLEEVALUATION == eval_policy_) {
        // Create a vector of parameterset_processing_result objects
        std::vector<parameterset_processing_result> processing_results(
            f_cnt.size(),
            parameterset_processing_result()
        );

        // Take care of the transformed fitness
        std::size_t pos = 0;
        for(auto &p : processing_results) {
            // Set the raw fitness
            p.reset(f_cnt.at(pos));

            if(evaluationPolicy::USESIGMOID == eval_policy_) {
                // Update the fitness value to use sigmoidal values
                p.setTransformedFitnessWith([this](const double rawValue) {
                    return Gem::Common::grational_sigmoid(
                        rawValue,
                        this->sigmoid_extremes_,
                        this->sigmoid_steepness_
                    );
                });
            }
            else {
                // All other transformation policies use the same value for the transformed fitness as a (valid) raw fitness
                p.setTransformedFitnessToRaw();
            }

            pos++;
        }

        // Transfer the data into the individual
        this->markAsProcessedWith(processing_results);
    }
    else {
        // Some constraints were violated. Act on the chosen policy
        if(evaluationPolicy::USEWORSTCASEFORINVALID == eval_policy_) {
            this->setAllFitnessTo(this->getWorstCase());
        }
        else if(evaluationPolicy::USESIGMOID == eval_policy_) {
            double uniformFitnessValue = 0.;
            if(maxMode::MAXIMIZE == this->getMaxMode()) {
                // maximize
                if(boost::numeric::bounds<double>::highest() == validity_level_) {
                    uniformFitnessValue = this->getWorstCase();
                }
                else {
                    uniformFitnessValue = -validity_level_ * sigmoid_extremes_;
                }
            }
            else {
                // minimize
                if(boost::numeric::bounds<double>::highest() == validity_level_) {
                    uniformFitnessValue = this->getWorstCase();
                }
                else {
                    uniformFitnessValue = validity_level_ * sigmoid_extremes_;
                }
            }

            this->setAllFitnessTo(this->getWorstCase(), uniformFitnessValue);
        }
    }
}

/******************************************************************************/
/**
     * Combines evaluation results by adding the individual results
     *
     *  @return The result of the combination
     */
double GParameterSet::sumCombiner() const {
    double result = 0.;

    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += this->transformed_fitness(id);
    }

    return result;
}

/******************************************************************************/
/**
     * Combines evaluation results by adding the absolute values of individual results
     *
     *  @return The result of the combination
     */
double GParameterSet::fabsSumCombiner() const {
    double result = 0.;

    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += std::abs(this->transformed_fitness(id));
    }

    return result;
}

/******************************************************************************/
/**
     * Combines evaluation results by calculating the square root of the squared sum.
     * It is assumed that the result of this function is returned as
     * the main result of the fitnessCalculation() function.
     *
     * @return The result of the combination
     */
double GParameterSet::squaredSumCombiner() const {
    double result = 0.;

    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += GSQUARED(this->transformed_fitness(id));
    }

    return sqrt(result);
}

/******************************************************************************/
/**
     * Combines evaluation results by calculating the square root of the weighed squared sum. Note that we
     * only evaluate the secondary results here. It is assumed that the result of this function is returned as
     * the main result of the fitnessCalculation() function.
     *
     * @param weights The weights to be multiplied with the cached results
     * @return The result of the combination
     */
double GParameterSet::weighedSquaredSumCombiner(std::vector<double> const &weights) const {
    if(this->getNStoredResults() != weights.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::weighedSquaredSumCombine(): Error!" << std::endl
            << "Sizes of transformedCurrentFitnessVec_ and the weights vector don't match: "
            << this->getNStoredResults() << " / " << weights.size() << std::endl
        );
    }

    double result = 0.;
    auto cit_weights = weights.begin();

    for(std::size_t id = 0; id < this->getNStoredResults(); id++, ++cit_weights) {
        result += GSQUARED((*cit_weights) * this->transformed_fitness(id));
    }

    return sqrt(result);
}

/******************************************************************************/
/**
     * Checks whether this solution fulfills the set of constraints. Note that this
     * function may be called prior to evaluation in order to check
     */
bool GParameterSet::parameterSetFulfillsConstraints(double &validityLevel) const {
    if(individual_constraint_ptr_) {
        return individual_constraint_ptr_->isValid(this, validityLevel);
    }
    else {
        // Always valid, if no constraint object has been registered
        validityLevel = 0.;
        return true;
    }

    // Make the compiler happy
    return false;
}

/******************************************************************************/
/**
     * Retrieves a parameter of a given type at the specified position
     */
std::any GParameterSet::getVarVal(
    std::string const &descr,
    std::tuple<std::size_t, std::string, std::size_t> const &target
) {
    std::any result;

    if(descr == "d") {
        result = GParameterSet::getVarItem<double>(target);
    }
    else if(descr == "f") {
        result = GParameterSet::getVarItem<float>(target);
    }
    else if(descr == "i") {
        result = GParameterSet::getVarItem<std::int32_t>(target);
    }
    else if(descr == "b") {
        result = GParameterSet::getVarItem<bool>(target);
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSet::getVarVal(): Error!" << std::endl
            << "Received invalid type description" << std::endl
        );
    }

    return result;
}

/***************************************************************************/
/**
     * Allows to set all fitnesses to the same value (raw and transformed values seperately)
     */
void GParameterSet::setAllFitnessTo(const double rawValue, const double transformedValue) {
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        this->modifyStoredResult(i).reset(rawValue);
        this->modifyStoredResult(i).setTransformedFitnessTo(transformedValue);
    }
}

/***************************************************************************/
/**
     * Allows to set all fitnesses to the same value (both raw and transformed values)
     */
void GParameterSet::setAllFitnessTo(const double val) {
    this->setAllFitnessTo(val, val);
}

/******************************************************************************/
/**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
bool GParameterSet::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GObject::modify_GUnitTests_())
        result = true;
    if(Gem::Common::GPtrVectorT<GParameterBase, GObject>::modify_GUnitTests_())
        result = true;

    for(const auto &o_ptr : *this) {
        if(o_ptr->modify_GUnitTests())
            result = true;
    }

    if(this->randomInit(activityMode::ALLPARAMETERS)) {
        result = true;
    }

    // A relatively harmless change
    n_stalls_++;
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw

    Gem::Common::condnotset("GParameterSet::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
void GParameterSet::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Access to uniformly distributed double random numbers
    std::uniform_real_distribution<double> uniform_real_distribution;

    // Call the parent class'es function
    GObject::specificTestsNoFailureExpected_GUnitTests_();
    Gem::Common::GPtrVectorT<GParameterBase, GObject>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the maximization mode flag
        std::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();

        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MAXIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MAXIMIZE);
        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MINIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MINIMIZE);
    }

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the surrounding optimization algorithm's current iteration
        std::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();

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
        std::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();

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

            if((d += 0.1) >= 1.) // NOLINT(bugprone-assignment-in-if-condition)
                break;
        }
    }

    // --------------------------------------------------------------------------

    {
        // Test setting and retrieval of the number of consecutive stalls
        std::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();

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
        const double MINGCONSTRDOUBLE = -4.;
        const double MAXGCONSTRDOUBLE = 4.;
        const double MINGDOUBLE = -5.;
        const double MAXGDOUBLE = 5.;
        const double MINGDOUBLECOLL = -3.;
        const double MAXGDOUBLECOLL = 3.;
        const std::size_t NGDOUBLECOLL = 10;
        const std::size_t FPLOOPCOUNT = 5;
        const double FPFIXEDVALINITMIN = -3.;
        const double FPFIXEDVALINITMAX = 3.;
        const double FPMULTIPLYBYRANDMIN = -5.;
        const double FPMULTIPLYBYRANDMAX = 5.;
        const double FPADD = 2.;

        // Create a GParameterSet object as a clone of this object for further usage
        std::shared_ptr<GParameterSet> p_test_0 = this->clone<GParameterSet>();
        // Clear the collection
        p_test_0->clear();
        // Make sure it is really empty
        CHECK(p_test_0->empty());
        // Add some floating pount parameters
        for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
            p_test_0->push_back(
                std::make_shared<GConstrainedDoubleObject>(
                    uniform_real_distribution(
                        gr_,
                        std::uniform_real_distribution<double>::param_type(
                            MINGCONSTRDOUBLE,
                            MAXGCONSTRDOUBLE
                        )
                    ),
                    MINGCONSTRDOUBLE,
                    MAXGCONSTRDOUBLE
                )
            );
            p_test_0->push_back(
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE)
                ))
            );
            p_test_0->push_back(
                std::make_shared<GDoubleCollection>(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL)
            );
        }

        // Attach a few other parameter types
        p_test_0->push_back(std::make_shared<GConstrainedInt32Object>(7, -10, 10));
        p_test_0->push_back(std::make_shared<GBooleanObject>(true));

        //-----------------------------------------------------------------

        {
            // Test random initialization
            // Create a GParameterSet object as a clone of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

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
            double d = FPFIXEDVALINITMIN;
            while(true) {
                // Create a GParameterSet object as a clone of p_test_0 for further usage
                std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

                // Initialize all fp-values with 0.
                p_test->fixedValueInit<double>(d, activityMode::ALLPARAMETERS);

                // Make sure the dirty flag is set
                CHECK(p_test->is_due_for_processing() == true);

                // Cross-check
                std::size_t counter = 0;
                for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
                    CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() == d);
                    counter++;
                    CHECK(p_test->at<GDoubleObject>(counter)->value() == d);
                    counter++;
                    std::shared_ptr<GDoubleCollection> p_gdc;
                    CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                    for(std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
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

                if((d += 1.) >= FPFIXEDVALINITMAX) // NOLINT(bugprone-assignment-in-if-condition)
                    break;
            }
        }

        //-----------------------------------------------------------------

        {
            // Test multiplication of all fp parameters with a fixed value
            double d = -3.;
            while(true) {
                // Create a GParameterSet object as a clone of p_test_0 for further usage
                std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

                // Initialize all fp-values with FPFIXEDVALINITMAX
                CHECK_NOTHROW(
                    p_test->fixedValueInit<double>(FPFIXEDVALINITMAX, activityMode::ALLPARAMETERS)
                );

                // Multiply this fixed value by d
                CHECK_NOTHROW(p_test->multiplyBy<double>(d, activityMode::ALLPARAMETERS));

                // Make sure the dirty flag is set
                CHECK(p_test->is_due_for_processing() == true);

                // Cross-check
                std::size_t counter = 0;
                for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
                    // A constrained value does not have to assume the value d*FPFIXEDVALINITMAX,
                    // but needs to stay within its boundaries
                    CHECK(
                        p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE
                    );
                    CHECK(
                        p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE
                    );
                    counter++;
                    CHECK(p_test->at<GDoubleObject>(counter)->value() == d * FPFIXEDVALINITMAX);
                    counter++;
                    std::shared_ptr<GDoubleCollection> p_gdc;
                    CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                    for(std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
                        INFO(
                            "\n"
                            << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                            << "expected " << d * FPFIXEDVALINITMAX << "\n"
                            << "iteration = " << gdc_cnt << "\n"
                        );
                        CHECK(p_gdc->at(gdc_cnt) == d * FPFIXEDVALINITMAX);
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

                if((d += 1.) >= 3.) // NOLINT(bugprone-assignment-in-if-condition)
                    break;
            }
        }

        //-----------------------------------------------------------------

        {
            // Test that fpMultiplyByRandom(min,max) changes every single parameter
            // Create a GParameterSet object as a clone of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

            // Multiply each floating point value with a constrained random value
            CHECK_NOTHROW(p_test->multiplyByRandom<double>(
                FPMULTIPLYBYRANDMIN,
                FPMULTIPLYBYRANDMAX,
                activityMode::ALLPARAMETERS
            ));

            // Make sure the dirty flag is set
            CHECK(p_test->is_due_for_processing() == true);

            // Cross-check
            std::size_t counter = 0;
            for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
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
                for(std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
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
            // Create a GParameterSet object as a clone of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

            // Multiply each floating point value with a constrained random value
            CHECK_NOTHROW(p_test->multiplyByRandom<double>(activityMode::ALLPARAMETERS));

            // Make sure the dirty flag is set
            CHECK(p_test->is_due_for_processing() == true);

            // Cross-check
            std::size_t counter = 0;
            for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
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
                for(std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
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
            // Create two GParameterSet objects as a clone of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();
            std::shared_ptr<GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

            // Initialize all fp-values of the "add" individual with a fixed value
            CHECK_NOTHROW(p_test_fixed->fixedValueInit<double>(FPADD, activityMode::ALLPARAMETERS));

            // Add p_test_fixed to p_test
            CHECK_NOTHROW(p_test->add<double>(p_test_fixed, activityMode::ALLPARAMETERS));

            // Check the results
            std::size_t counter = 0;
            for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
                // A constrained value does not have to assume the value value()+FPADD
                // but needs to stay within its boundaries
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
                counter++;
                CHECK(
                    p_test->at<GDoubleObject>(counter)->value() ==
                    p_test_0->at<GDoubleObject>(counter)->value() + FPADD
                );
                counter++;
                std::shared_ptr<GDoubleCollection> p_gdc;
                std::shared_ptr<GDoubleCollection> p_gdc_0;
                CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                CHECK_NOTHROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
                for(std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
                    INFO(
                        "\n"
                        << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                        << "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
                        << "FPADD = " << FPADD
                        << "p_gdc_0->at(gdc_cnt) + FPADD = " << p_gdc_0->at(gdc_cnt) + FPADD << "\n"
                        << "iteration = " << gdc_cnt << "\n"
                    );
                    CHECK(p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) + FPADD);
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
            constexpr double FPSUBTRACT = 2.;

            // Check subtraction of individuals
            // Create two GParameterSet objects as a clone of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();
            std::shared_ptr<GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

            // Initialize all fp-values of the "add" individual with a fixed valie
            CHECK_NOTHROW(
                p_test_fixed->fixedValueInit<double>(FPSUBTRACT, activityMode::ALLPARAMETERS)
            );

            // Add p_test_fixed to p_test
            CHECK_NOTHROW(p_test->subtract<double>(p_test_fixed, activityMode::ALLPARAMETERS));

            // Check the results
            std::size_t counter = 0;
            for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
                // A constrained value does not have to assume the value value()-FPSUBTRACT
                // but needs to stay within its boundaries
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
                CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
                counter++;
                CHECK(
                    p_test->at<GDoubleObject>(counter)->value() ==
                    p_test_0->at<GDoubleObject>(counter)->value() - FPSUBTRACT
                );
                counter++;
                std::shared_ptr<GDoubleCollection> p_gdc;
                std::shared_ptr<GDoubleCollection> p_gdc_0;
                CHECK_NOTHROW(p_gdc = p_test->at<GDoubleCollection>(counter));
                CHECK_NOTHROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
                for(std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
                    INFO(
                        "\n"
                        << "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
                        << "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
                        << "FPSUBTRACT = " << FPSUBTRACT << "p_gdc_0->at(gdc_cnt) - FPSUBTRACT = "
                        << p_gdc_0->at(gdc_cnt) - FPSUBTRACT << "\n"
                        << "iteration = " << gdc_cnt << "\n"
                    );
                    CHECK(p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) - FPSUBTRACT);
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
        const double MINGCONSTRDOUBLE = -4.;
        const double MAXGCONSTRDOUBLE = 4.;
        const double MINGDOUBLE = -5.;
        const double MAXGDOUBLE = 5.;
        const double MINGDOUBLECOLL = -3.;
        const double MAXGDOUBLECOLL = 3.;
        const std::size_t NGDOUBLECOLL = 10;
        const std::size_t NINTCOLL = 10;
        const std::size_t NINTBOOLOBJ = 10;
        const std::int32_t MINGINT = -100;
        const std::int32_t MAXGINT = 100;
        const std::size_t FPLOOPCOUNT = 5;
        const double FPFIXEDVALINITMIN = -3.;
        const double FPFIXEDVALINITMAX = 3.;
        const double FPMULTIPLYBYRANDMIN = -5.;
        const double FPMULTIPLYBYRANDMAX = 5.;
        const double FPADD = 2.;
        const double FPSUBTRACT = 2.;

        // Create a GParameterSet object as a clone of this object for further usage
        std::shared_ptr<GParameterSet> p_test_0 = this->clone<GParameterSet>();
        // Clear the collection
        p_test_0->clear();
        // Make sure it is really empty
        CHECK(p_test_0->empty());

        // Add some floating point parameters
        for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
            std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr =
                std::make_shared<GConstrainedDoubleObject>(
                    uniform_real_distribution(
                        gr_,
                        std::uniform_real_distribution<double>::param_type(
                            MINGCONSTRDOUBLE,
                            MAXGCONSTRDOUBLE
                        )
                    ),
                    MINGCONSTRDOUBLE,
                    MAXGCONSTRDOUBLE
                );
            std::shared_ptr<GDoubleObject> gdo_ptr =
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE)
                ));
            std::shared_ptr<GDoubleCollection> gdc_ptr =
                std::make_shared<GDoubleCollection>(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL);

            // Mark the last parameter type as inactive
            gdc_ptr->setAdaptionsInactive();

            // Add the parameter objects to the parameter set
            p_test_0->push_back(gcdo_ptr);
            p_test_0->push_back(gdo_ptr);
            p_test_0->push_back(gdc_ptr);
        }

        // Attach a few other parameter types
        for(std::size_t i = 0; i < NINTBOOLOBJ; i++) {
            p_test_0->push_back(std::make_shared<GConstrainedInt32Object>(7, MINGINT, MAXGINT));
            p_test_0->push_back(std::make_shared<GBooleanObject>(true));
        }

        // Finally we add a tree structure
        std::shared_ptr<GParameterObjectCollection> poc_ptr =
            std::make_shared<GParameterObjectCollection>();

        for(std::size_t i = 0; i < FPLOOPCOUNT; i++) {
            std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr =
                std::make_shared<GConstrainedDoubleObject>(
                    uniform_real_distribution(
                        gr_,
                        std::uniform_real_distribution<double>::param_type(
                            MINGCONSTRDOUBLE,
                            MAXGCONSTRDOUBLE
                        )
                    ),
                    MINGCONSTRDOUBLE,
                    MAXGCONSTRDOUBLE
                );
            std::shared_ptr<GDoubleObject> gdo_ptr =
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE)
                ));
            std::shared_ptr<GConstrainedInt32ObjectCollection> gcioc_ptr =
                std::make_shared<GConstrainedInt32ObjectCollection>();

            std::shared_ptr<GParameterObjectCollection> sub_poc_ptr =
                std::make_shared<GParameterObjectCollection>();

            for(std::size_t ip = 0; ip < NINTCOLL; ip++) {
                std::shared_ptr<GConstrainedInt32Object> gci32o_ptr =
                    std::make_shared<GConstrainedInt32Object>(MINGINT, MAXGINT);
                gci32o_ptr->setAdaptionsInactive();
                // The parameter should not be modifiable now

                sub_poc_ptr->push_back(gci32o_ptr);
            }

            std::shared_ptr<GDoubleObject> gdo2_ptr =
                std::make_shared<GDoubleObject>(uniform_real_distribution(
                    gr_,
                    std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE)
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
        std::size_t NDOUBLEACTIVE = 2 * FPLOOPCOUNT + 2 * FPLOOPCOUNT;
        std::size_t NDOUBLEINACTIVE = NGDOUBLECOLL * FPLOOPCOUNT + FPLOOPCOUNT;
        std::size_t NDOUBLEALL = NDOUBLEINACTIVE + NDOUBLEACTIVE;
        std::size_t NINTACTIVE = NINTBOOLOBJ;
        std::size_t NINTINACTIVE = NINTCOLL * FPLOOPCOUNT;
        std::size_t NINTALL = NINTINACTIVE + NINTACTIVE;
        std::size_t NBOOLACTIVE = NINTBOOLOBJ;
        std::size_t NBOOLINACTIVE = 0;
        std::size_t NBOOLALL = NBOOLINACTIVE + NBOOLACTIVE;

        //-----------------------------------------------------------------

        {
            // Test counting of parameters
            // Create a GParameterSet object as a clone of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

            // Count the number of parameters and compare with the expected number
            CHECK(p_test->countParameters<double>(activityMode::ACTIVEONLY) == NDOUBLEACTIVE);
            CHECK(p_test->countParameters<double>(activityMode::INACTIVEONLY) == NDOUBLEINACTIVE);
            CHECK(p_test->countParameters<double>(activityMode::ALLPARAMETERS) == NDOUBLEALL);
            CHECK(p_test->countParameters<std::int32_t>(activityMode::ACTIVEONLY) == NINTACTIVE);
            CHECK(
                p_test->countParameters<std::int32_t>(activityMode::INACTIVEONLY) == NINTINACTIVE
            );
            CHECK(p_test->countParameters<std::int32_t>(activityMode::ALLPARAMETERS) == NINTALL);
            CHECK(p_test->countParameters<bool>(activityMode::ACTIVEONLY) == NBOOLACTIVE);
            CHECK(p_test->countParameters<bool>(activityMode::INACTIVEONLY) == NBOOLINACTIVE);
            CHECK(p_test->countParameters<bool>(activityMode::ALLPARAMETERS) == NBOOLALL);
        }

        //-----------------------------------------------------------------

        {
            // Check that streamline(activityMode::INACTIVEONLY) yields unchanged results before and after randomInit(activityMode::ACTIVEONLY)
            // Create two GParameterSet objects as clones of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test_orig = p_test_0->clone<GParameterSet>();
            std::shared_ptr<GParameterSet> p_test_rand = p_test_0->clone<GParameterSet>();

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
            CHECK(orig_d_inactive.size() == NDOUBLEINACTIVE);
            CHECK(orig_d_inactive == rand_d_inactive);
            CHECK(orig_i_inactive.size() == NINTINACTIVE);
            CHECK(orig_i_inactive == rand_i_inactive);
            CHECK(orig_b_inactive.size() == NBOOLINACTIVE);
            CHECK(orig_b_inactive == rand_b_inactive);
        }

        //-----------------------------------------------------------------

        {
            // Check that streamline(activityMode::ACTIVEONLY) yields changed results after randomInit(activityMode::ACTIVEONLY)
            // Create a GParameterSet object as a clone of p_test_0 for further usage
            // Create two GParameterSet objects as clones of p_test_0 for further usage
            std::shared_ptr<GParameterSet> p_test_orig = p_test_0->clone<GParameterSet>();
            std::shared_ptr<GParameterSet> p_test_rand = p_test_0->clone<GParameterSet>();

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
            CHECK(orig_d_active.size() == NDOUBLEACTIVE);
            CHECK(rand_d_active.size() == NDOUBLEACTIVE);
            CHECK(orig_d_active != rand_d_active);
            CHECK(orig_i_active.size() == NINTACTIVE);
            CHECK(rand_i_active.size() == NINTACTIVE);
            INFO(
                "orig_i_active: " << Gem::Common::vecToString(orig_i_active) << "\n"
                                  << "rand_i_active: " << Gem::Common::vecToString(rand_i_active)
                                  << "\n"
            );
            CHECK(orig_i_active != rand_i_active);
            CHECK(orig_b_active.size() == NBOOLACTIVE);
            CHECK(rand_b_active.size() == NBOOLACTIVE);

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
        "GParameterSet::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
void GParameterSet::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    GObject::specificTestsFailuresExpected_GUnitTests_();
    Gem::Common::GPtrVectorT<GParameterBase, GObject>::specificTestsFailuresExpected_GUnitTests_();

    // no tests here yet

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw

    Gem::Common::condnotset(
        "GParameterSet::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva */
