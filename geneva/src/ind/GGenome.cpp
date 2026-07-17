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

#include "geneva/ind/GGenome.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <ranges>
#include <string>
#include <vector>

#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "hap/GRandomLeasePool.hpp"
#include "geneva/GOptimizationEnums.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * @brief The default constructor (a single fitness criterion, an empty genome to be filled by setGenome()).
 */
GGenome::GGenome() = default;

/******************************************************************************/
/**
 * @brief Initialization with the number of fitness criteria.
 * @param n_fitness_criteria The number of fitness criteria this genome will evaluate to
 */
GGenome::GGenome(const std::size_t n_fitness_criteria)
  : GOptimizableEntity(n_fitness_criteria) {
    /* nothing */
}

/******************************************************************************/
/**
 * @brief The copy constructor.
 * @param cp A constant reference to another GGenome object to be copied
 */
GGenome::GGenome(GGenome const &cp)
  : GOptimizableEntity(cp)
  , dv_(cp.dv_)
  , fv_(cp.fv_)
  , iv_(cp.iv_)
  , bv_(cp.bv_)
  , layout_(cp.layout_) {
    /* nothing -- the count cache is lazily (re)built on first use */
}

/******************************************************************************/
/**
 * @brief Installs the value arrays and shared structural layout produced by a GGenomeBuilder.
 * @param g The GenomeData bundle (double/float/int/bool value arrays plus the shared layout) to install
 */
void GGenome::setGenome(GenomeData const &g) {
    // Install the new layout (re-keys the parameter-count cache).
    this->setLayout(g.layout ? g.layout : std::make_shared<const GGenomeLayout>());

    // Reject inverted bounds up front with a clear message (lower == upper is a valid frozen parameter).
    auto checkBounds = [](auto const &ch, const char *type_name) {
        for(std::size_t k = 0; k < ch.size(); ++k) {
            if(ch.fold[k] && ch.lower[k] > ch.upper[k]) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GGenome::setGenome(): Error!" << '\n'
                    << "Bounded " << type_name << " parameter " << k << " has inverted bounds (lower "
                    << ch.lower[k] << " > upper " << ch.upper[k] << ")." << '\n'
                );
            }
        }
    };
    checkBounds(layout_->d, "double");
    checkBounds(layout_->f, "float");

    // The builder's floating-point start values are EXTERNAL; convert each to the normalized internal
    // store (range-validated). The int / bool channels are not normalized and are stored as-is.
    dv_.resize(g.dv.size());
    for(std::size_t k = 0; k < g.dv.size(); ++k) {
        dv_[k] = externalToInternalChecked<double>(layout_->d, g.dv[k], k, /*allow_upper_bound=*/true);
    }
    fv_.resize(g.fv.size());
    for(std::size_t k = 0; k < g.fv.size(); ++k) {
        fv_[k] = externalToInternalChecked<float>(layout_->f, g.fv[k], k, /*allow_upper_bound=*/true);
    }
    iv_ = g.iv;
    bv_ = g.bv;

    this->mark_as_due_for_processing();
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object of the same type.
 * @param cp A constant reference to another GGenome, camouflaged as a GOptimizableEntity
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation tolerated for floating point comparisons (unused here)
 */
void GGenome::compare_(
    GOptimizableEntity const &cp,
    Gem::Common::expectation const &e,
    [[maybe_unused]] double const &limit
) const {
    using namespace Gem::Common;

    const GGenome *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GGenome>(cp, this);

    GToken token("GGenome", e);

    Gem::Common::compare_base_t<GOptimizableEntity>(*this, *p_load, token);

    // The value channels (the shared layout is problem metadata, not per-individual identity).
    Gem::Common::g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Loads the data of another GGenome object, camouflaged as a GOptimizableEntity.
 * @param cp A pointer to another GGenome object, camouflaged as a GOptimizableEntity
 */
void GGenome::load_(const GOptimizableEntity *cp) {
    const GGenome *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GGenome>(cp, this);

    GOptimizableEntity::load_(cp);

    // The value channels, derived from the single localMembers() declaration ...
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
    // ... and the manual tail: the shared (immutable) layout is shared, not value-copied (re-keys cache).
    this->setLayout(p_load->layout_);
    // Propagate the transient results-only marker (a load_()-based copy reflects the pending graft).
    input_omitted_ = p_load->input_omitted_;
}

/******************************************************************************/
/**
 * @brief Random initialization of the active parameters across all four channels.
 * @param am The activity mode that selects which parameters are initialized
 * @return true if at least one parameter value was modified, false otherwise
 */
bool GGenome::randomInit_(activityMode const &am) {
    // The candidate holds no RNG of its own -- lease a proxy for the duration of this initialization.
    auto             lease = Gem::Hap::randomLeasePool().acquire();
    Gem::Hap::GRandomBase &gr = *lease;

    bool modified = false;
    if(randomInitFP<double>(dv_, layout_->d, am, gr)) { modified = true; }
    if(randomInitFP<float>(fv_, layout_->f, am, gr)) { modified = true; }
    if(randomInitInt(am, gr)) { modified = true; }
    if(randomInitBool(am, gr)) { modified = true; }
    return modified;
}

/******************************************************************************/
/**
 * @brief Randomly (re-)initializes the active entries of one floating-point value channel.
 * @tparam T The floating-point value type of the channel (double or float)
 * @param store The value array of this channel, modified in place
 * @param ch The channel layout providing per-element activity flags and interval bounds
 * @param am The activity mode that selects which entries are initialized
 * @return true if at least one entry was modified, false otherwise
 */
template <typename T>
bool GGenome::randomInitFP(std::vector<T> &store, ChannelLayout<T> const &ch, activityMode const &am, Gem::Hap::GRandomBase &gr) {
    bool modified = false;
    std::uniform_real_distribution<T> dist;
    for(std::size_t k = 0; k < store.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        const T x = dist(
            gr,
            typename std::uniform_real_distribution<T>::param_type(ch.init_lower[k], ch.init_upper[k])
        );
        store[k] = externalToInternalChecked<T>(ch, x, k);
        modified = true;
    }
    return modified;
}

/******************************************************************************/
/**
 * @brief Randomly (re-)initializes the active entries of the int32 value channel.
 * @param am The activity mode that selects which entries are initialized
 * @return true if at least one entry was modified, false otherwise
 */
bool GGenome::randomInitInt(activityMode const &am, Gem::Hap::GRandomBase &gr) {
    bool modified = false;
    std::uniform_int_distribution<std::int32_t> dist;
    const ChannelLayout<std::int32_t> &ch = layout_->i;
    for(std::size_t k = 0; k < iv_.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        std::int32_t lo = ch.fold[k] ? ch.lower[k] : ch.init_lower[k];
        std::int32_t hi = ch.fold[k] ? ch.upper[k] : ch.init_upper[k];
        if(hi < lo) {
            std::swap(lo, hi);
        }
        iv_[k] = dist(gr, std::uniform_int_distribution<std::int32_t>::param_type(lo, hi));
        modified = true;
    }
    return modified;
}

/******************************************************************************/
/**
 * @brief Randomly (re-)initializes the active entries of the boolean value channel.
 * @param am The activity mode that selects which entries are initialized
 * @return true if at least one entry was modified, false otherwise
 */
bool GGenome::randomInitBool(activityMode const &am, Gem::Hap::GRandomBase &gr) {
    bool modified = false;
    std::bernoulli_distribution dist(0.5);
    const ChannelLayout<bool> &ch = layout_->b;
    for(std::size_t k = 0; k < bv_.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        bv_[k] = dist(gr) ? static_cast<std::uint8_t>(1) : static_cast<std::uint8_t>(0);
        modified = true;
    }
    return modified;
}

/******************************************************************************/
/**
 * @brief Retrieval of a suitable position for cross over inside of a vector, in the range [lower, upper[.
 * @param lower The (inclusive) lower bound of the position range; must be > 0
 * @param upper The (exclusive) upper bound of the position range; must be > lower
 * @return A uniformly drawn position in the half-open range [lower, upper)
 */
std::size_t GGenome::getCrossOverPos(const std::size_t lower, const std::size_t upper) {
    if(lower == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GGenome::getCrossOverPos(): Error!" << '\n'
            << "lower boundary is 0, but must be > 0" << '\n'
        );
    }
    if(upper <= lower) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GGenome::getCrossOverPos(): Error!" << '\n'
            << "Invalid range: upper (" << upper << ") must be > lower (" << lower << ")" << '\n'
        );
    }
    auto lease = Gem::Hap::randomLeasePool().acquire();
    std::uniform_int_distribution<std::size_t> dist;
    return dist(*lease, std::uniform_int_distribution<std::size_t>::param_type(lower, upper - 1));
}

/******************************************************************************/
/**
 * @brief Perform a cross-over operation between this genome and another.
 * @param cp_base A constant reference to the other parent (a GGenome)
 * @return A newly created GGenome holding the crossed-over genome, upcast to GOptimizableEntity
 */
std::shared_ptr<GOptimizableEntity> GGenome::crossOverWith(GOptimizableEntity const &cp_base) const {
    const auto &cp = dynamic_cast<const GGenome &>(cp_base);
    std::shared_ptr<GGenome> this_cp = this->clone<GGenome>();

    std::vector<double> this_d, cp_d;
    std::vector<float> this_f, cp_f;
    std::vector<bool> this_b, cp_b;
    std::vector<std::int32_t> this_i, cp_i;

    this_cp->streamline(this_d);
    this_cp->streamline(this_f);
    this_cp->streamline(this_b);
    this_cp->streamline(this_i);

    cp.streamline(cp_d);
    cp.streamline(cp_f);
    cp.streamline(cp_b);
    cp.streamline(cp_i);

    auto splice = [&](auto &this_vec, auto const &cp_vec) {
        if(not this_vec.empty() && this_vec.size() == cp_vec.size()) {
            const auto pos = this_cp->getCrossOverPos(1, this_vec.size());
            std::copy(
                cp_vec.begin() + static_cast<std::ptrdiff_t>(pos),
                cp_vec.end(),
                this_vec.begin() + static_cast<std::ptrdiff_t>(pos)
            );
        }
    };
    splice(this_d, cp_d);
    splice(this_f, cp_f);
    splice(this_b, cp_b);
    splice(this_i, cp_i);

    this_cp->assignValueVector(this_d);
    this_cp->assignValueVector(this_f);
    this_cp->assignValueVector(this_b);
    this_cp->assignValueVector(this_i);

    this_cp->mark_as_due_for_processing();

    return this_cp;
}

/******************************************************************************/
/**
 * @brief Retrieves parameters relevant for the evaluation from another GGenome.
 * @param cp A reference to the foreign genome (a GGenome); it is moved-from and cleared
 */
void GGenome::cannibalize(GOptimizableEntity &cp_base) {
    auto &cp = dynamic_cast<GGenome &>(cp_base);
    if(cp.is_due_for_processing() || cp.has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GGenome::cannibalize(): Error!" << '\n'
            << "cp isn't processed or has errors" << '\n'
        );
    }

    dv_ = std::move(cp.dv_);
    fv_ = std::move(cp.fv_);
    iv_ = std::move(cp.iv_);
    bv_ = std::move(cp.bv_);
    this->setLayout(cp.layout_);

    cp.dv_.clear();
    cp.fv_.clear();
    cp.iv_.clear();
    cp.bv_.clear();

    this->setFitness_(cp.raw_fitness_vec());
}

/******************************************************************************/
/**
 * @brief Retrieve the active double parameter at the given positional index.
 * @param idx The positional index into the streamlined double channel
 * @return The double value at the given index
 */
double GGenome::getVarVal_d_(std::size_t idx) {
    std::vector<double> v;
    this->streamline<double>(v);
    return v.at(idx);
}

/**
 * @brief Retrieve the active float parameter at the given positional index.
 * @param idx The positional index into the streamlined float channel
 * @return The float value at the given index
 */
float GGenome::getVarVal_f_(std::size_t idx) {
    std::vector<float> v;
    this->streamline<float>(v);
    return v.at(idx);
}

/**
 * @brief Retrieve the active int32 parameter at the given positional index.
 * @param idx The positional index into the streamlined int32 channel
 * @return The int32 value at the given index
 */
std::int32_t GGenome::getVarVal_i_(std::size_t idx) {
    std::vector<std::int32_t> v;
    this->streamline<std::int32_t>(v);
    return v.at(idx);
}

/**
 * @brief Retrieve the active boolean parameter at the given positional index.
 * @param idx The positional index into the streamlined boolean channel
 * @return The boolean value at the given index
 */
bool GGenome::getVarVal_b_(std::size_t idx) {
    std::vector<bool> v;
    this->streamline<bool>(v);
    return static_cast<bool>(v.at(idx));
}

/******************************************************************************/
/**
 * @brief Transformation of the individual's parameters into a JSON object.
 *
 * Returns the individual body only (no wrapping key); callers place it wherever they need it. The
 * parameters live in a "vars" JSON array (index = position) and the results in a "results" JSON array,
 * replacing the former positional var0/result0 keys. Numeric and boolean values are native JSON numbers
 * and bools; the transformation policy is a string.
 *
 * @return A boost::json::object holding this individual's parameters, metadata and results
 */
boost::json::object GGenome::toJSON() const {
    namespace json = boost::json;

    bool dirty_flag = (Gem::Courtier::processingStatus::DO_PROCESS == this->getProcessingStatus());
    bool has_errors = this->has_errors();

    json::object body;
    body["iteration"] = this->getAssignedIteration();
    body["is_dirty"] = dirty_flag;
    body["has_errors"] = has_errors;
    body["isValid"] = has_errors || dirty_flag ? false : this->isValid();
    body["type"] = "GGenome";

    std::vector<double> d_data;
    std::vector<float> f_data;
    std::vector<std::int32_t> i_data;
    std::vector<bool> b_data;
    this->streamline<double>(d_data);
    this->streamline<float>(f_data);
    this->streamline<std::int32_t>(i_data);
    this->streamline<bool>(b_data);

    const std::size_t n_vars = d_data.size() + f_data.size() + i_data.size() + b_data.size();
    body["nVars"] = n_vars;

    json::array vars;
    auto emit = [&](auto const &vec, const char *type_name) {
        for(auto const &val : vec) {
            json::object var;
            var["value"] = val;
            var["type"] = type_name;
            vars.push_back(std::move(var));
        }
    };
    emit(d_data, "double");
    emit(f_data, "float");
    emit(i_data, "int32");
    emit(b_data, "bool");
    body["vars"] = std::move(vars);

    switch(this->getEvaluationPolicy()) {
    case evaluationPolicy::USESIMPLEEVALUATION:
        body["transformationPolicy"] = "USESIMPLEEVALUATION";
        break;
    case evaluationPolicy::USESIGMOID:
        body["transformationPolicy"] = "USESIGMOID";
        break;
    case evaluationPolicy::USEWORSTCASEFORINVALID:
        body["transformationPolicy"] = "USEWORSTCASEFORINVALID";
        break;
    }

    body["n_results"] = this->getNStoredResults();
    json::array results;
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        const double raw_fitness =
            (dirty_flag || has_errors) ? this->getWorstCase() : this->raw_fitness(i);
        const double transformed_fitness =
            (dirty_flag || has_errors) ? this->getWorstCase() : this->transformed_fitness(i);
        json::object result;
        result["result"] = transformed_fitness;
        result["rawResult"] = raw_fitness;
        results.push_back(std::move(result));
    }
    body["results"] = std::move(results);

    return body;
}

/******************************************************************************/
/**
 * @brief Transformation of the individual's parameters into a list of comma-separated values.
 * @param with_name_and_type If true, prepend a row of variable names and a row of variable types
 * @param with_commas If true, separate fields with ",\\t"; otherwise with a plain tab
 * @param use_raw_fitness If true, emit raw fitness values; otherwise emit transformed fitness values
 * @param show_validity If true, append a trailing validity column
 * @return A string holding the CSV (tab-separated) representation, terminated with a newline
 */
std::string GGenome::toCSV(
    bool with_name_and_type,
    bool with_commas,
    bool use_raw_fitness,
    bool show_validity
) const {
    std::vector<double> d_data;
    std::vector<float> f_data;
    std::vector<std::int32_t> i_data;
    std::vector<bool> b_data;

    this->streamline<double>(d_data);
    this->streamline<float>(f_data);
    this->streamline<std::int32_t>(i_data);
    this->streamline<bool>(b_data);

    std::vector<std::string> var_names;
    std::vector<std::string> var_types;
    std::vector<std::string> var_values;

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

    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        if(with_name_and_type) {
            var_names.push_back(std::string("Fitness_") + Gem::Common::to_string(i));
            var_types.emplace_back("double");
        }
        if(this->is_processed()) {
            if(use_raw_fitness) {
                var_values.push_back(Gem::Common::to_string(this->raw_fitness(i)));
            }
            else {
                var_values.push_back(Gem::Common::to_string(this->transformed_fitness(i)));
            }
        }
        else {
            if(this->has_errors()) {
                var_values.emplace_back("has_errors");
            }
            else {
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
            var_values.push_back(Gem::Common::to_string(this->isValid()));
        }
        else {
            var_values.push_back(Gem::Common::to_string(false));
        }
    }

    // Each row joins its fields with a between-fields separator only (no trailing separator) and is
    // terminated by a single newline -- byte-identical to the former "write separator unless last" loops.
    std::string const separator = with_commas ? ",\t" : "\t";
    auto const joinRow = [&separator](std::vector<std::string> const &fields) {
        return fields | std::views::join_with(separator) | std::ranges::to<std::string>();
    };

    std::string result;
    if(with_name_and_type) {
        result += joinRow(var_names);
        result += '\n';
        result += joinRow(var_types);
        result += '\n';
    }
    result += joinRow(var_values);
    result += '\n';

    return result;
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes.
 * @return true if modifications were made, false otherwise
 */
bool GGenome::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(this->randomInit(activityMode::ALLPARAMETERS)) {
        result = true;
    }

    // Guarantee an observable change to a compared member even for individuals with no randomisable
    // parameters (e.g. GDelayIndividual), so the clone-independence / serialize-round-trip tests remain
    // meaningful. This bumps assigned_iteration_ (a compared base member) -- the data-oriented successor
    // of the old n_stalls_ bump, which was retired when stall bookkeeping moved wholly onto the OA.
    this->setAssignedIteration(this->getAssignedIteration() + 1);
    result = true;

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GGenome::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GGenome::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    {
        std::shared_ptr<GGenome> p_test = this->clone<GGenome>();
        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MAXIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MAXIMIZE);
        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MINIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MINIMIZE);
    }
    {
        std::shared_ptr<GGenome> p_test = this->clone<GGenome>();
        for(std::uint32_t i = 1; i < 10; i++) {
            CHECK_NOTHROW(p_test->setAssignedIteration(i));
            CHECK(p_test->getAssignedIteration() == i);
        }
    }
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GGenome::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GGenome::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // no tests here yet
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GGenome::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::Genome */
