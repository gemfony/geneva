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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "geneva/ind/GAdaptionKernels.hpp"
#include "geneva/ind/GGenomeLayout.hpp"

/******************************************************************************/
/**
 * Non-intrusive Boost.Serialization support for the (otherwise POD-clean) adaption-layout structs.
 * The layout is serialised by value as part of GFlatGenome's transport encoding (it carries no
 * pointers and no evolving state, so a flat serialisation is sufficient and self-contained).
 */
namespace boost::serialization {

template <class Archive, typename T>
void serialize(Archive &ar, Gem::Geneva::Parameters::GaussConfig<T> &g, const unsigned int) {
    ar &make_nvp("sigma_sigma", g.sigma_sigma) &make_nvp("min_sigma", g.min_sigma) &
        make_nvp("max_sigma", g.max_sigma) &make_nvp("min_ad_prob", g.min_ad_prob) &
        make_nvp("max_ad_prob", g.max_ad_prob) &make_nvp("adapt_ad_prob", g.adapt_ad_prob) &
        make_nvp("adapt_sigma_prob", g.adapt_sigma_prob) &
        make_nvp("adaption_threshold", g.adaption_threshold) &make_nvp("mode", g.mode);
}

template <class Archive, typename T>
void serialize(Archive &ar, Gem::Geneva::Parameters::BiGaussConfig<T> &g, const unsigned int) {
    ar &make_nvp("sigma_sigma1", g.sigma_sigma1) &make_nvp("sigma_sigma2", g.sigma_sigma2) &
        make_nvp("sigma_delta", g.sigma_delta) &make_nvp("min_sigma1", g.min_sigma1) &
        make_nvp("max_sigma1", g.max_sigma1) &make_nvp("min_sigma2", g.min_sigma2) &
        make_nvp("max_sigma2", g.max_sigma2) &make_nvp("min_delta", g.min_delta) &
        make_nvp("max_delta", g.max_delta) &make_nvp("min_ad_prob", g.min_ad_prob) &
        make_nvp("max_ad_prob", g.max_ad_prob) &make_nvp("adapt_ad_prob", g.adapt_ad_prob) &
        make_nvp("adapt_sigma_prob", g.adapt_sigma_prob) &
        make_nvp("adaption_threshold", g.adaption_threshold) &
        make_nvp("use_symmetric_sigmas", g.use_symmetric_sigmas) &make_nvp("mode", g.mode);
}

template <class Archive>
inline void
serialize(Archive &ar, Gem::Geneva::Parameters::FlipConfig &g, const unsigned int) {
    ar &make_nvp("min_ad_prob", g.min_ad_prob) &make_nvp("max_ad_prob", g.max_ad_prob) &
        make_nvp("adapt_ad_prob", g.adapt_ad_prob) &make_nvp("mode", g.mode);
}

// The genome layout holds structure-only groups. The adaptor configuration lives on the (transient,
// non-serialized) OA-owned GAdaptionConfig, so only the structure is serialized here.
template <class Archive, typename T>
void serialize(Archive &ar, Gem::Geneva::Parameters::GroupStructure<T> &g, const unsigned int) {
    ar &make_nvp("start", g.start) &make_nvp("len", g.len) &make_nvp("label_id", g.label_id) &
        make_nvp("active", g.active) &make_nvp("range", g.range);
}

template <class Archive, typename T>
void serialize(Archive &ar, Gem::Geneva::Parameters::ChannelLayout<T> &c, const unsigned int) {
    ar &make_nvp("lower", c.lower) &make_nvp("upper", c.upper) &
        make_nvp("init_lower", c.init_lower) &make_nvp("init_upper", c.init_upper) &
        make_nvp("kind", c.kind) &make_nvp("active", c.active) &make_nvp("groups", c.groups);
}

template <class Archive>
void serialize(Archive &ar, Gem::Geneva::Parameters::GGenomeLayout &l, const unsigned int) {
    ar &make_nvp("d", l.d) &make_nvp("f", l.f) &make_nvp("i", l.i) &make_nvp("b", l.b) &
        make_nvp("labels", l.labels);
}

} /* namespace boost::serialization */
