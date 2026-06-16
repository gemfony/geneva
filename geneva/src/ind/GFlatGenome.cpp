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

#include "geneva/ind/GFlatGenome.hpp"

#include <algorithm>
#include <any>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionAuxKeys.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {

// The per-group adaption-state auxiliary-store keys (AUXKEY_*) are defined in GAdaptionAuxKeys.hpp,
// shared with the OA-side adaption logic so both address the very same state blocks.

/******************************************************************************/
/**
 * The default constructor. Results in a single fitness criterion and an empty genome (to be filled
 * by setGenome()).
 */
GFlatGenome::GFlatGenome()
  : GOptimizableEntity() {
    /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of fitness criteria.
 */
GFlatGenome::GFlatGenome(const std::size_t n_fitness_criteria)
  : GOptimizableEntity(n_fitness_criteria) {
    /* nothing */
}

/******************************************************************************/
/**
 * The copy constructor. The base copy constructor copies the per-individual auxiliary store,
 * including the evolving Gauss adaption state, so a clone inherits the current sigma.
 */
GFlatGenome::GFlatGenome(GFlatGenome const &cp)
  : GOptimizableEntity(cp)
  , dv_(cp.dv_)
  , fv_(cp.fv_)
  , iv_(cp.iv_)
  , bv_(cp.bv_)
  , layout_(cp.layout_) {
    /* nothing */
}

/******************************************************************************/
/**
 * Installs the value arrays + shared layout produced by a GGenomeBuilder. The per-group adaption state
 * is OA-owned scratch (it lives on the GIndividualSlot, not on the individual) and is no longer seeded
 * here -- an optimization algorithm seeds each slot's scratch from its adaption config at setup.
 */
void GFlatGenome::setGenome(Genome const &g) {
    dv_ = g.dv;
    fv_ = g.fv;
    iv_ = g.iv;
    bv_ = g.bv;
    layout_ = g.layout ? g.layout : std::make_shared<const GGenomeLayout>();

    this->mark_as_due_for_processing();
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object of the same type. The
 * shared structural layout is problem metadata (not per-individual identity), so only the value
 * arrays and the individual-level base data participate in the comparison.
 */
void GFlatGenome::compare_(
    GOptimizableEntity const &cp,
    Gem::Common::expectation const &e,
    [[maybe_unused]] double const &limit
) const {
    using namespace Gem::Common;

    const GFlatGenome *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GFlatGenome>(cp, this);

    GToken token("GFlatGenome", e);

    Gem::Common::compare_base_t<GOptimizableEntity>(*this, *p_load, token);

    compare_t(IDENTITY(this->dv_, p_load->dv_), token);
    compare_t(IDENTITY(this->fv_, p_load->fv_), token);
    compare_t(IDENTITY(this->iv_, p_load->iv_), token);
    compare_t(IDENTITY(this->bv_, p_load->bv_), token);

    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GFlatGenome object, camouflaged as a GOptimizableEntity. The base load
 * copies the auxiliary store (personality + the per-group Gauss state), so the loaded genome keeps
 * the source's evolved sigma; the value arrays are copied and the (immutable) layout is shared.
 */
void GFlatGenome::load_(const GOptimizableEntity *cp) {
    const GFlatGenome *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GFlatGenome>(cp, this);

    GOptimizableEntity::load_(cp);

    dv_ = p_load->dv_;
    fv_ = p_load->fv_;
    iv_ = p_load->iv_;
    bv_ = p_load->bv_;
    layout_ = p_load->layout_;
}

/******************************************************************************/
/**
 * Random initialization of the active parameters. Constrained values are drawn within their bounds
 * (and stored as their own internal representation); plain values within their init perimeter.
 */
bool GFlatGenome::randomInit_(activityMode const &am) {
    bool modified = false;
    if(randomInitFP<double>(dv_, layout_->d, am)) {
        modified = true;
    }
    if(randomInitFP<float>(fv_, layout_->f, am)) {
        modified = true;
    }
    if(randomInitInt(am)) {
        modified = true;
    }
    if(randomInitBool(am)) {
        modified = true;
    }
    return modified;
}

template <typename T>
bool GFlatGenome::randomInitFP(std::vector<T> &store, ChannelLayout<T> const &ch, activityMode const &am) {
    bool modified = false;
    std::uniform_real_distribution<T> dist;
    for(std::size_t k = 0; k < store.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        store[k] = dist(
            gr_,
            typename std::uniform_real_distribution<T>::param_type(ch.init_lower[k], ch.init_upper[k])
        );
        modified = true;
    }
    return modified;
}

bool GFlatGenome::randomInitInt(activityMode const &am) {
    bool modified = false;
    std::uniform_int_distribution<std::int32_t> dist;
    const ChannelLayout<std::int32_t> &ch = layout_->i;
    for(std::size_t k = 0; k < iv_.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        std::int32_t lo =
            (ch.kind[k] == ParamKind::Constrained) ? ch.lower[k] : ch.init_lower[k];
        std::int32_t hi =
            (ch.kind[k] == ParamKind::Constrained) ? ch.upper[k] : ch.init_upper[k];
        if(hi < lo) {
            std::swap(lo, hi);
        }
        iv_[k] = dist(gr_, std::uniform_int_distribution<std::int32_t>::param_type(lo, hi));
        modified = true;
    }
    return modified;
}

bool GFlatGenome::randomInitBool(activityMode const &am) {
    bool modified = false;
    std::bernoulli_distribution dist(0.5);
    const ChannelLayout<bool> &ch = layout_->b;
    for(std::size_t k = 0; k < bv_.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        bv_[k] = dist(gr_) ? std::uint8_t(1) : std::uint8_t(0);
        modified = true;
    }
    return modified;
}

/******************************************************************************/
/**
 * Retrieves the boundaries of the boolean channel (always [false, true]).
 */
void GFlatGenome::boundaries_(
    std::vector<bool> &l,
    std::vector<bool> &u,
    activityMode const &am
) const {
    l.clear();
    u.clear();
    const ChannelLayout<bool> &ch = layout_->b;
    for(std::size_t k = 0; k < ch.active.size(); ++k) {
        if(not amMatch(ch.active[k], am)) {
            continue;
        }
        l.push_back(false);
        u.push_back(true);
    }
}

/******************************************************************************/
/**
 * Retrieval of a suitable position for cross over inside of a vector, in the range [lower, upper[.
 */
std::size_t GFlatGenome::getCrossOverPos(const std::size_t lower, const std::size_t upper) {
    if(lower == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFlatGenome::getCrossOverPos(): Error!" << '\n'
            << "lower boundary is 0, but must be > 0" << '\n'
        );
    }
    if(upper <= lower) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFlatGenome::getCrossOverPos(): Error!" << '\n'
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
 * Perform a cross-over operation between this object and another. Mirrors the tree: streamline both
 * genomes per channel, splice each channel at a random position and assign the result back.
 */
std::shared_ptr<GOptimizableEntity>
GFlatGenome::crossOverWith(GOptimizableEntity const &cp_base) const {
    const auto &cp = dynamic_cast<GFlatGenome const &>(cp_base);

    std::shared_ptr<GFlatGenome> this_cp = this->clone<GFlatGenome>();

    std::vector<double> this_d;
    std::vector<double> cp_d;
    std::vector<float> this_f;
    std::vector<float> cp_f;
    std::vector<bool> this_b;
    std::vector<bool> cp_b;
    std::vector<std::int32_t> this_i;
    std::vector<std::int32_t> cp_i;

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
 * Retrieves parameters relevant for the evaluation from another GFlatGenome. The foreign genome is
 * left empty afterwards. May only be called for a "clean" (processed, error-free) foreign genome.
 */
void GFlatGenome::cannibalize(GOptimizableEntity &cp_base) {
    auto &cp = dynamic_cast<GFlatGenome &>(cp_base);

    if(cp.is_due_for_processing() || cp.has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFlatGenome::cannibalize(GOptimizableEntity& cp)" << '\n'
            << "cp isn't processed or has errors" << '\n'
        );
    }

    dv_ = std::move(cp.dv_);
    fv_ = std::move(cp.fv_);
    iv_ = std::move(cp.iv_);
    bv_ = std::move(cp.bv_);
    layout_ = cp.layout_;

    cp.dv_.clear();
    cp.fv_.clear();
    cp.iv_.clear();
    cp.bv_.clear();

    this->setFitness_(cp.raw_fitness_vec());
}

/******************************************************************************/
/**
 * Retrieve the active parameter at the given (positional) index, per type. These are the typed
 * dispatch targets of GOptimizableEntity::getVarVal<T>() -- one streamline of the relevant channel,
 * then the indexed element (no std::any boxing).
 */
double GFlatGenome::getVarVal_d_(std::size_t idx) {
    std::vector<double> v;
    this->streamline<double>(v);
    return v.at(idx);
}

float GFlatGenome::getVarVal_f_(std::size_t idx) {
    std::vector<float> v;
    this->streamline<float>(v);
    return v.at(idx);
}

std::int32_t GFlatGenome::getVarVal_i_(std::size_t idx) {
    std::vector<std::int32_t> v;
    this->streamline<std::int32_t>(v);
    return v.at(idx);
}

bool GFlatGenome::getVarVal_b_(std::size_t idx) {
    std::vector<bool> v;
    this->streamline<bool>(v);
    return static_cast<bool>(v.at(idx));
}

/******************************************************************************/
/**
 * Transformation of the individual's parameters into a boost::property_tree object.
 */
void GFlatGenome::toPropertyTree(pt::ptree &ptr, std::string const &base_name) const {
    bool dirty_flag = (Gem::Courtier::processingStatus::DO_PROCESS == this->getProcessingStatus());
    bool has_errors = this->has_errors();

    ptr.put(base_name + ".iteration", this->getAssignedIteration());
    ptr.put(base_name + ".is_dirty", dirty_flag);
    ptr.put(base_name + ".has_errors", has_errors);
    ptr.put(base_name + ".isValid", has_errors || dirty_flag ? false : this->isValid());
    ptr.put(base_name + ".type", std::string("GFlatGenome"));

    std::vector<double> d_data;
    std::vector<float> f_data;
    std::vector<std::int32_t> i_data;
    std::vector<bool> b_data;
    this->streamline<double>(d_data);
    this->streamline<float>(f_data);
    this->streamline<std::int32_t>(i_data);
    this->streamline<bool>(b_data);

    const std::size_t n_vars = d_data.size() + f_data.size() + i_data.size() + b_data.size();
    ptr.put(base_name + ".nVars", n_vars);

    std::size_t pos = 0;
    auto emit = [&](auto const &vec, const char *type_name) {
        for(auto const &val : vec) {
            const std::string base = base_name + ".vars.var" + Gem::Common::to_string(pos);
            ptr.put(base + ".value", val);
            ptr.put(base + ".type", std::string(type_name));
            ++pos;
        }
    };
    emit(d_data, "double");
    emit(f_data, "float");
    emit(i_data, "int32");
    emit(b_data, "bool");

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

    ptr.put(base_name + ".n_results", this->getNStoredResults());
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        const double raw_fitness =
            (dirty_flag || has_errors) ? this->getWorstCase() : this->raw_fitness(i);
        const double transformed_fitness =
            (dirty_flag || has_errors) ? this->getWorstCase() : this->transformed_fitness(i);
        ptr.put(base_name + ".results.result" + Gem::Common::to_string(i), transformed_fitness);
        ptr.put(base_name + ".results.rawResult" + Gem::Common::to_string(i), raw_fitness);
    }
}

/******************************************************************************/
/**
 * Transformation of the individual's parameters into a list of comma-separated values plus fitness
 * and validity. Identical in spirit to GTreeGenome::toCSV() (it operates purely on the streamlined
 * value vectors and the fitness bookkeeping).
 */
std::string GFlatGenome::toCSV(
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

    std::ostringstream result;
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
 * Emits a name for this class / object.
 */
std::string GFlatGenome::name_() const {
    return std::string("GFlatGenome");
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes.
 */
bool GFlatGenome::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(this->randomInit(activityMode::ALLPARAMETERS)) {
        result = true;
    }

    this->setNStalls(this->getNStalls() + 1);
    result = true;

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GFlatGenome::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GFlatGenome::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    {
        std::shared_ptr<GFlatGenome> p_test = this->clone<GFlatGenome>();
        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MAXIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MAXIMIZE);
        CHECK_NOTHROW(p_test->setMaxMode(maxMode::MINIMIZE));
        CHECK(p_test->getMaxMode() == maxMode::MINIMIZE);
    }
    {
        std::shared_ptr<GFlatGenome> p_test = this->clone<GFlatGenome>();
        for(std::uint32_t i = 1; i < 10; i++) {
            CHECK_NOTHROW(p_test->setAssignedIteration(i));
            CHECK(p_test->getAssignedIteration() == i);
        }
    }
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GFlatGenome::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GFlatGenome::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // no tests here yet
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GFlatGenome::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::Parameters */
