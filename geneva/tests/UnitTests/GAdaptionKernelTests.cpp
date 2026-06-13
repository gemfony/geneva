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

#include <catch2/catch_test_macros.hpp>

#include <span>
#include <vector>

#include "geneva/ind/GAdaptionKernels.hpp"
#include "hap/GRandomT.hpp"

using namespace Gem::Geneva;
using namespace Gem::Geneva::Parameters;

namespace {
Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;
}

/******************************************************************************/
TEST_CASE("adaptGaussGroup: mode NEVER leaves values and state untouched", "[kernel]") {
    GaussConfig<double> cfg;
    cfg.mode = adaptionMode::NEVER;
    GaussState<double> st;
    std::vector<double> v{1., 2., 3.};
    const auto before = v;

    const auto n = adaptGaussGroup<double>(cfg, st, std::span<double>(v), 1.0, gr);
    CHECK(n == 0);
    CHECK(v == before);
}

/******************************************************************************/
TEST_CASE("adaptGaussGroup: mode ALWAYS adapts every value (ULP-guaranteed change)", "[kernel]") {
    GaussConfig<double> cfg;
    cfg.mode = adaptionMode::ALWAYS;
    cfg.min_sigma = 1e-6;
    cfg.max_sigma = 10.;
    GaussState<double> st;
    st.sigma = 0.5;

    std::vector<double> v(50, 0.);
    const auto before = v;
    const auto n = adaptGaussGroup<double>(cfg, st, std::span<double>(v), 1.0, gr);
    CHECK(n == v.size());
    for(std::size_t i = 0; i < v.size(); ++i) {
        CHECK(v[i] != before[i]);
    }
}

/******************************************************************************/
TEST_CASE("adaptGaussGroup: WITHPROBABILITY honours the ad_prob extremes", "[kernel]") {
    GaussConfig<double> cfg;
    cfg.mode = adaptionMode::WITHPROBABILITY;

    // ad_prob == 1 => every value is adapted
    {
        GaussState<double> st;
        st.ad_prob = 1.;
        std::vector<double> v(40, 0.);
        CHECK(adaptGaussGroup<double>(cfg, st, std::span<double>(v), 1., gr) == v.size());
    }
    // ad_prob == 0 => no value is adapted
    {
        GaussState<double> st;
        st.ad_prob = 0.;
        std::vector<double> v(40, 0.);
        const auto before = v;
        CHECK(adaptGaussGroup<double>(cfg, st, std::span<double>(v), 1., gr) == 0);
        CHECK(v == before);
    }
}

/******************************************************************************/
TEST_CASE("adaptGaussGroup: sigma self-adapts and stays within [min,max]", "[kernel]") {
    GaussConfig<double> cfg;
    cfg.mode = adaptionMode::ALWAYS;
    cfg.adaption_threshold = 1;
    cfg.min_sigma = 0.01;
    cfg.max_sigma = 2.0;
    cfg.sigma_sigma = 0.8;
    GaussState<double> st;
    st.sigma = 0.5;
    const double first = st.sigma;
    bool changed = false;

    std::vector<double> v(1, 0.);
    for(int i = 0; i < 200; ++i) {
        adaptGaussGroup<double>(cfg, st, std::span<double>(v), 1., gr);
        CHECK(st.sigma >= cfg.min_sigma);
        CHECK(st.sigma <= cfg.max_sigma);
        if(st.sigma != first) {
            changed = true;
        }
    }
    CHECK(changed);
}

/******************************************************************************/
TEST_CASE("adaptGaussGroup: ad_prob self-adapts and stays within [min,max]", "[kernel]") {
    GaussConfig<double> cfg;
    cfg.mode = adaptionMode::ALWAYS;
    cfg.adaption_threshold = 1;
    cfg.adapt_ad_prob = 0.1;
    cfg.min_ad_prob = 0.05;
    cfg.max_ad_prob = 1.0;
    GaussState<double> st;
    st.ad_prob = 0.5;

    std::vector<double> v(1, 0.);
    for(int i = 0; i < 200; ++i) {
        adaptGaussGroup<double>(cfg, st, std::span<double>(v), 1., gr);
        CHECK(st.ad_prob >= cfg.min_ad_prob);
        CHECK(st.ad_prob <= cfg.max_ad_prob);
    }
}

/******************************************************************************/
TEST_CASE("adaptGaussGroup: works in float precision", "[kernel]") {
    GaussConfig<float> cfg;
    cfg.mode = adaptionMode::ALWAYS;
    cfg.min_sigma = 1e-4f;
    cfg.max_sigma = 5.f;
    GaussState<float> st;
    st.sigma = 0.5f;

    std::vector<float> v(20, 0.f);
    const auto before = v;
    CHECK(adaptGaussGroup<float>(cfg, st, std::span<float>(v), 1.f, gr) == v.size());
    for(std::size_t i = 0; i < v.size(); ++i) {
        CHECK(v[i] != before[i]);
    }
}
