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
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "geneva/ind/GAdaptionKernels.hpp"
#include "geneva/ind/GAdaptionLayout.hpp"

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
void serialize(Archive &ar, Gem::Geneva::Parameters::GroupSpec<T> &g, const unsigned int) {
    ar &make_nvp("start", g.start) &make_nvp("len", g.len) &make_nvp("active", g.active) &
        make_nvp("has_gauss", g.has_gauss) &make_nvp("gauss", g.gauss) &
        make_nvp("start_sigma", g.start_sigma) &make_nvp("start_ad_prob", g.start_ad_prob) &
        make_nvp("range", g.range);
}

template <class Archive, typename T>
void serialize(Archive &ar, Gem::Geneva::Parameters::ChannelLayout<T> &c, const unsigned int) {
    ar &make_nvp("lower", c.lower) &make_nvp("upper", c.upper) &
        make_nvp("init_lower", c.init_lower) &make_nvp("init_upper", c.init_upper) &
        make_nvp("kind", c.kind) &make_nvp("active", c.active) &make_nvp("groups", c.groups);
}

template <class Archive>
void serialize(Archive &ar, Gem::Geneva::Parameters::GAdaptionLayout &l, const unsigned int) {
    ar &make_nvp("d", l.d) &make_nvp("f", l.f) &make_nvp("i", l.i) &make_nvp("b", l.b);
}

} /* namespace boost::serialization */
