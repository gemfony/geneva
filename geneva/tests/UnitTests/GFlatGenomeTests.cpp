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

#include <any>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenomeLayout.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualFactory.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeArchitecture.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/individuals/GNeuralNetworkIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Geneva::Parameters;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace Gem::Tests {

/******************************************************************************/
/**
 * A minimal flat individual: a sphere over n constrained doubles sharing one Gauss adaptor. It
 * demonstrates the intended authoring effort -- a constructor that builds the genome, a
 * fitnessCalculation(), and the serialize hook; clone/load/compare come from the CRTP base +
 * GFlatGenome.
 */
class FlatSphere : public GFlatIndividualT<FlatSphere> {
public:
    FlatSphere() { buildGenome(5); }
    explicit FlatSphere(std::size_t n) { buildGenome(n); }
    FlatSphere(const FlatSphere &) = default;

    /** @brief The OA-owned Gauss adaption config for this genome's double group(s). */
    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
            cfg->groupDouble(i).gauss(0.5, 0.8, 1e-3, 2., 1.);
        }
        return cfg;
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double sum = 0.;
        for(double x : v) {
            sum += x * x;
        }
        return sum;
    }

private:
    void buildGenome(std::size_t n) {
        GGenomeBuilder b;
        b.addDoubleGroup(n, -10., 10.).init(1.0); // structure only; the adaptor lives on the OA config
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FlatSphere>>(*this)
        );
    }
};

/******************************************************************************/
/**
 * A config-driven (Tier-2) flat individual: the same sphere, but its genome is built by the generic
 * GFlatIndividualFactory from a Config that the factory reads from a configuration file. The default
 * constructor leaves the genome empty -- the factory installs it via setGenome() in postProcess_.
 */
class FactorySphere : public GFlatIndividualT<FactorySphere> {
public:
    FactorySphere() = default; // the factory installs the genome
    FactorySphere(const FactorySphere &) = default;

    /** @brief The configurable values read from the config file */
    struct Config {
        std::size_t par_dim = 5;
        double min = -10.;
        double max = 10.;
        double sigma = 0.5;
    };

    /** @brief Registers the config-file options, binding them to the passed Config */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c) {
        gpb.registerFileParameter<std::size_t>(
            "par_dim", c.par_dim, c.par_dim, Gem::Common::VAR_IS_ESSENTIAL, "Number of parameters"
        );
        gpb.registerFileParameter<double>(
            "min", c.min, c.min, Gem::Common::VAR_IS_ESSENTIAL, "Lower bound"
        );
        gpb.registerFileParameter<double>(
            "max", c.max, c.max, Gem::Common::VAR_IS_ESSENTIAL, "Upper bound"
        );
        gpb.registerFileParameter<double>(
            "sigma", c.sigma, c.sigma, Gem::Common::VAR_IS_ESSENTIAL, "Gauss sigma"
        );
    }

    /** @brief Builds the value arrays + the shared, immutable layout from the parsed config */
    static Genome buildGenome(const Config &c) {
        GGenomeBuilder b;
        b.addDoubleGroup(c.par_dim, c.min, c.max); // structure only; the adaptor lives on the OA config
        return b.build();
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double sum = 0.;
        for(double x : v) {
            sum += x * x;
        }
        return sum;
    }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FactorySphere>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::FlatSphere)    // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::FactorySphere) // NOLINT

using Gem::Tests::FactorySphere;
using Gem::Tests::FlatSphere;

/******************************************************************************/
TEST_CASE("GGenomeBuilder produces the expected shared layout", "[flat]") {
    GGenomeBuilder b;
    b.addDoubleGroup(5, -10., 10.).init(1.0); // structure only -- adaptors are authored on the OA config
    Genome g = b.build();

    REQUIRE(g.layout);
    const ChannelLayout<double> &ch = g.layout->d;
    REQUIRE(ch.size() == 5);
    REQUIRE(ch.groups.size() == 1);

    // The layout's group is STRUCTURE only: start / len / active / range (no adaptor fields).
    const GroupStructure<double> &grp = ch.groups[0];
    CHECK(grp.start == 0u);
    CHECK(grp.len == 5u);
    CHECK(grp.active);
    CHECK(grp.label_id == -1);
    CHECK(grp.range == 20.); // upper - lower

    // The adaptor itself is authored on the OA-owned config (built from the genome's structure). It
    // mirrors the builder's old gaussAdaptor(...) one-to-one.
    FlatSphere ind;
    ind.setGenome(g);
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(ind);
    cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);
    const GroupSpec<double> &cgrp = cfg->doubleGroups()[0];
    CHECK(cgrp.has_gauss);
    CHECK(cgrp.start_sigma == 0.5);
    CHECK(cgrp.gauss.sigma_sigma == 0.8);
    CHECK(cgrp.gauss.min_sigma == 1e-3);
    CHECK(cgrp.gauss.max_sigma == 2.);
    CHECK(cgrp.range == 20.); // the structural range is snapshotted into the config too

    for(std::size_t k = 0; k < 5; ++k) {
        CHECK(ch.lower[k] == -10.);
        CHECK(ch.upper[k] == 10.);
        CHECK(ch.kind[k] == ParamKind::Constrained);
        CHECK(g.dv[k] == 1.0);
    }
}

/******************************************************************************/
TEST_CASE("GGenomeBuilder: groups vs arrays vs single parameters", "[flat]") {
    GGenomeBuilder b;
    b.addDouble(0., -1., 1.);          // group of 1
    b.addDoubleGroup(4, -2., 2.);      // ONE group of 4 (shared sigma)
    b.addDoubleArray(3, -3., 3.);      // 3 groups of 1
    Genome g = b.build();

    const ChannelLayout<double> &ch = g.layout->d;
    CHECK(ch.size() == 1 + 4 + 3);
    CHECK(ch.groups.size() == 1 + 1 + 3); // single + grouped + array
}

/******************************************************************************/
TEST_CASE("GGenomeBuilder: interned group labels", "[flat]") {
    GGenomeBuilder b;
    b.addDoubleGroup(3, -1., 1.).label("position"); // structural label
    b.addDoubleGroup(2, -1., 1.).label("position"); // group 1
    b.addDouble(0., -1., 1.).label("scale");        // group 2
    b.addDouble(0., -1., 1.);                        // group 3 (unlabeled)
    Genome g = b.build();

    std::shared_ptr<const GGenomeLayout> L = g.layout;
    REQUIRE(L);
    REQUIRE(L->d.groups.size() == 4);

    // Interning: two distinct strings -> a two-entry table; "position" used twice reuses one id.
    REQUIRE(L->labels.size() == 2);
    CHECK(L->d.groups[0].label_id >= 0);
    CHECK(L->d.groups[0].label_id == L->d.groups[1].label_id);  // same string -> same id
    CHECK(L->d.groups[2].label_id != L->d.groups[0].label_id);  // different string -> different id
    CHECK(L->d.groups[3].label_id == -1);                       // unlabeled default

    // id <-> name resolution.
    CHECK(L->labelName(L->d.groups[0].label_id) == "position");
    CHECK(L->labelName(L->d.groups[2].label_id) == "scale");
    CHECK(L->labelName(-1).empty());

    // One-to-many resolution: "position" tags two groups, "scale" one, an absent label none.
    std::vector<GroupRef> pos = L->groupsForLabel("position");
    REQUIRE(pos.size() == 2);
    CHECK(pos[0].channel == ChannelTag::Double);
    CHECK(pos[0].index == 0);
    CHECK(pos[1].index == 1);
    CHECK(L->groupsForLabel("scale").size() == 1);
    CHECK(L->groupsForLabel("absent").empty());

    // Serialize round-trip (through a GFlatGenome) preserves the labels + their resolution.
    FlatSphere ind;
    ind.setGenome(g);
    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);

    FlatSphere restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    std::shared_ptr<const GGenomeLayout> RL = restored.getLayout();
    REQUIRE(RL);
    CHECK(RL->labels == L->labels);
    CHECK(RL->groupsForLabel("position").size() == 2);
    CHECK(RL->labelName(RL->d.groups[2].label_id) == "scale");
    CHECK(RL->d.groups[3].label_id == -1);
}

/******************************************************************************/
// (The former "personality is OA scratch" / "OA-identity mnemonic" individual test was retired: the
// individual is now fully OA-agnostic -- it carries neither the personality object (which lives on the
// GIndividualSlot, see the [slot] tests) nor any OA-identity mnemonic (post-processing eligibility is
// decided by the algorithm and vetoed on the work item's processing metadata).)

/******************************************************************************/
TEST_CASE("GFlatGenome: value round-trip (streamline / assignValueVector)", "[flat]") {
    FlatSphere ind(4);

    std::vector<double> v;
    ind.streamline<double>(v);
    REQUIRE(v.size() == 4);
    CHECK(ind.countParameters<double>() == 4);

    const std::vector<double> assigned{2., 3., -4., 5.};
    ind.assignValueVector<double>(assigned);
    ind.streamline<double>(v);
    CHECK(v == assigned); // in-range constrained values round-trip identically
}

/******************************************************************************/
TEST_CASE("Constrained fold helpers map into range", "[flat]") {
    // FP: half-open [lo, hi)
    CHECK(foldConstrainedFP<double>(5., -10., 10.) == 5.);   // in range -> identity
    CHECK(foldConstrainedFP<double>(25., -10., 10.) == -5.); // reflected
    for(double x : {-37.3, -11., 9.999, 100.25, 10.0}) {
        const double f = foldConstrainedFP<double>(x, -10., 10.);
        CHECK(f >= -10.);
        CHECK(f < 10.);
    }

    // Int: closed [lo, hi]
    CHECK(foldConstrainedInt<std::int32_t>(7, -10, 10) == 7);
    for(std::int32_t x : {-100, -11, 11, 250}) {
        const std::int32_t f = foldConstrainedInt<std::int32_t>(x, -10, 10);
        CHECK(f >= -10);
        CHECK(f <= 10);
    }
}

/******************************************************************************/
TEST_CASE("GFlatGenome: clone is independent", "[flat]") {
    FlatSphere ind(6);

    auto twin = ind.clone<FlatSphere>();

    // Equal right after cloning.
    CHECK_NOTHROW(twin->compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));

    // Mutating the clone leaves the original untouched.
    twin->randomInit(activityMode::ALLPARAMETERS);

    std::vector<double> orig;
    std::vector<double> mutated;
    ind.streamline<double>(orig);
    twin->streamline<double>(mutated);
    CHECK(orig != mutated);

    bool equal = true;
    try {
        twin->compare(
            ind,
            Gem::Common::expectation::EQUALITY,
            Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
        );
    }
    catch(const g_expectation_violation &) {
        equal = false;
    }
    CHECK_FALSE(equal);
}

/******************************************************************************/
TEST_CASE("GFlatGenome: serialization round-trip", "[flat]") {
    FlatSphere ind(5);
    ind.assignValueVector<double>(std::vector<double>{1., -2., 3., -4., 5.});

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);

    FlatSphere restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    CHECK_NOTHROW(restored.compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));

    std::vector<double> v;
    restored.streamline<double>(v);
    CHECK(v == std::vector<double>{1., -2., 3., -4., 5.});
}

/******************************************************************************/
TEST_CASE("GFlatGenome: adapt() mutates within bounds", "[flat]") {
    FlatSphere ind(8);

    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    std::size_t total = 0;
    for(int round = 0; round < 20; ++round) {
        total += adapter.adapt(ind);

        std::vector<double> v;
        ind.streamline<double>(v);
        REQUIRE(v.size() == 8);
        for(double x : v) {
            CHECK(x >= -10.); // streamline always folds back into the external range
            CHECK(x < 10.);
        }
    }
    CHECK(total > 0); // ad_prob == 1 -> every value is adapted every round
}

/******************************************************************************/
TEST_CASE("GFlatGenome: randomInit stays within bounds and changes values", "[flat]") {
    FlatSphere ind(10);

    std::vector<double> before;
    ind.streamline<double>(before);

    CHECK(ind.randomInit(activityMode::ALLPARAMETERS));

    std::vector<double> after;
    ind.streamline<double>(after);
    CHECK(before != after);
    for(double x : after) {
        CHECK(x >= -10.);
        CHECK(x < 10.);
    }
}

/******************************************************************************/
TEST_CASE("GFlatGenome: OA stall-reset restores sigma to its seed", "[flat][oa]") {
    FlatSphere ind(3);

    // The OA-owned config drives both the sigma readout and the stall-reset (Phase 8 / 10). It is the
    // config the individual authors; the per-group adaption STATE is OA-owned scratch (held on the
    // GIndividualSlot in a live run) -- here a standalone GAuxiliaryStore, seeded from the config.
    auto cfg_ptr = ind.getAdaptionConfig();
    auto &cfg = *cfg_ptr;
    GAuxiliaryStore scratch;
    cfg.installInto(scratch);

    // Drive the sigma self-adaption (adaption_threshold defaults to 1 -> sigma adapts every step).
    for(int i = 0; i < 30; ++i) {
        oa::adaptIndividual(ind, scratch, cfg);
    }

    auto sigmaNow = [&]() {
        std::vector<double> s = oa::readAdaptionSigmas(scratch, cfg, "GDoubleGaussAdaptor");
        REQUIRE(s.size() == 1);
        return s[0];
    };

    const double drifted = sigmaNow();
    CHECK(drifted != 0.5); // sigma has evolved away from its seed

    oa::resetAdaptionState(scratch, cfg);
    CHECK(sigmaNow() == 0.5); // reset to the configured seed
}

/******************************************************************************/
/**
 * A flat individual mixing a bi-gaussian FP group, an int32 flip group and a bool flip group, used to
 * exercise the flip / bi-gauss adaption paths and their serialisation.
 */
namespace Gem::Tests {

class FlatMixed : public GFlatIndividualT<FlatMixed> {
public:
    FlatMixed() {
        GGenomeBuilder b;
        b.addDoubleGroup(3, -5., 5.).init(2.0); // structure only; the adaptors live on the OA config
        b.addInt32Group(4, -10, 10).init(3);
        b.addBoolGroup(5).init(false);
        this->setGenome(b.build());
    }
    FlatMixed(const FlatMixed &) = default;

    /** @brief The OA-owned config: a bi-Gauss adaptor on the FP group, flip adaptors on the int + bool groups. */
    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupDouble(0).biGauss(0.5, 0.8, 1e-3, 2., 0.5, 0.8, 1e-3, 2., 0.5, 0.8, 0., 2., 1.);
        cfg->groupInt32(0).flip(1.0);
        cfg->groupBool(0).flip(1.0);
        return cfg;
    }

protected:
    double fitnessCalculation() override { return 0.; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FlatMixed>>(*this)
        );
    }
};

/**
 * A flat individual carrying an integer Gauss adaptor (the prerequisite for GMetaOptimizerIndividualT,
 * which mutates n_children with GInt32GaussAdaptor while n_parents uses a flip adaptor). Two int32
 * groups: a constrained one driven by the integer Gauss kernel and another by the flip kernel, so the
 * test confirms both int adaptor kinds coexist on the same channel.
 */
class FlatIntGauss : public GFlatIndividualT<FlatIntGauss> {
public:
    FlatIntGauss() {
        GGenomeBuilder b;
        // Gauss-adapted constrained ints in [-50, 50], own state each (groups 0..2).
        b.addInt32Array(3, -50, 50).init(0);
        // Flip-adapted constrained ints in [-10, 10], shared state (group 3).
        b.addInt32Group(2, -10, 10).init(5);
        this->setGenome(b.build());
    }
    FlatIntGauss(const FlatIntGauss &) = default;

    /** @brief The OA-owned config: an integer Gauss adaptor on groups 0..2, a flip adaptor on group 3. */
    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupInt32(0).intGauss(0.2, 0.8, 1e-3, 2., 1.);
        cfg->groupInt32(1).intGauss(0.2, 0.8, 1e-3, 2., 1.);
        cfg->groupInt32(2).intGauss(0.2, 0.8, 1e-3, 2., 1.);
        cfg->groupInt32(3).flip(1.0);
        return cfg;
    }

protected:
    double fitnessCalculation() override { return 0.; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FlatIntGauss>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::FlatMixed)    // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::FlatIntGauss) // NOLINT

using Gem::Tests::FlatMixed;

/******************************************************************************/
TEST_CASE("GFlatGenome: flip adaptor mutates int32 and bool channels", "[flat][flip]") {
    FlatMixed ind;

    std::vector<std::int32_t> i_before;
    std::vector<bool> b_before;
    ind.streamline<std::int32_t>(i_before);
    ind.streamline<bool>(b_before);
    REQUIRE(i_before.size() == 4);
    REQUIRE(b_before.size() == 5);

    bool int_changed = false;
    bool bool_changed = false;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int round = 0; round < 20; ++round) {
        adapter.adapt(ind);

        std::vector<std::int32_t> i_now;
        std::vector<bool> b_now;
        ind.streamline<std::int32_t>(i_now);
        ind.streamline<bool>(b_now);
        for(std::int32_t x : i_now) {
            CHECK(x >= -10);
            CHECK(x <= 10);
        }
        if(i_now != i_before) {
            int_changed = true;
        }
        if(b_now != b_before) {
            bool_changed = true;
        }
    }
    CHECK(int_changed);
    CHECK(bool_changed);
}

/******************************************************************************/
TEST_CASE("GFlatGenome: bi-gaussian adaptor mutates the FP channel within bounds", "[flat][bigauss]") {
    FlatMixed ind;

    std::vector<double> before;
    ind.streamline<double>(before);
    REQUIRE(before.size() == 3);

    std::size_t total = 0;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int round = 0; round < 20; ++round) {
        total += adapter.adapt(ind);
        std::vector<double> v;
        ind.streamline<double>(v);
        for(double x : v) {
            CHECK(x >= -5.);
            CHECK(x < 5.);
        }
    }
    CHECK(total > 0);
}

/******************************************************************************/
TEST_CASE("GFlatGenome: mixed flip/bigauss genome serialises round-trip", "[flat][flip][bigauss]") {
    FlatMixed ind;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int i = 0; i < 5; ++i) {
        adapter.adapt(ind);
    }

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);
    FlatMixed restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    CHECK_NOTHROW(restored.compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));
}

using Gem::Tests::FlatIntGauss;

/******************************************************************************/
TEST_CASE("GFlatGenome: integer Gauss adaptor mutates int32 within bounds", "[flat][intgauss]") {
    FlatIntGauss ind;

    std::vector<std::int32_t> before;
    ind.streamline<std::int32_t>(before);
    REQUIRE(before.size() == 5); // 3 Gauss + 2 flip

    bool changed = false;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int round = 0; round < 30; ++round) {
        adapter.adapt(ind);
        std::vector<std::int32_t> now;
        ind.streamline<std::int32_t>(now);
        REQUIRE(now.size() == 5);
        for(std::size_t k = 0; k < 3; ++k) { // Gauss group bounds
            CHECK(now[k] >= -50);
            CHECK(now[k] <= 50);
        }
        for(std::size_t k = 3; k < 5; ++k) { // flip group bounds
            CHECK(now[k] >= -10);
            CHECK(now[k] <= 10);
        }
        if(now != before) {
            changed = true;
        }
    }
    CHECK(changed);
}

/******************************************************************************/
TEST_CASE("GFlatGenome: integer Gauss genome serialises round-trip", "[flat][intgauss]") {
    FlatIntGauss ind;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int i = 0; i < 5; ++i) {
        adapter.adapt(ind);
    }

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);
    FlatIntGauss restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    CHECK_NOTHROW(restored.compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));
}

/******************************************************************************/
TEST_CASE("GGridArchitecture reads any genome through the §2 seam (flat)", "[flat][architecture]") {
    FlatSphere ind(12); // 12 FP values -> a 3x4 grid
    GGridArchitecture grid(3, 4);

    CHECK(grid.name() == "GGridArchitecture");
    CHECK(grid.expectedFPSize() == 12);

    std::vector<double> flat;
    ind.streamlineFP(flat);
    REQUIRE(flat.size() == 12);

    for(std::size_t r = 0; r < grid.rows(); ++r) {
        const std::vector<double> row = grid.row(ind, r);
        REQUIRE(row.size() == grid.cols());
        for(std::size_t c = 0; c < grid.cols(); ++c) {
            CHECK(grid.at(ind, r, c) == flat[r * grid.cols() + c]);
            CHECK(row[c] == flat[r * grid.cols() + c]);
        }
    }
}

/******************************************************************************/
TEST_CASE("GFlatIndividualFactory: one shared layout across N produced individuals", "[flat][factory]") {
    // A temporary config file. writeConfigFile() generates it from the individual's describeConfig()
    // defaults; get_as<>() then reads it back and produces individuals.
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "geneva_flat_factory_tests";
    fs::create_directories(base);
    const fs::path cfg = base / "FactorySphere.json";

    GFlatIndividualFactory<FactorySphere> f(cfg);
    f.writeConfigFile("FactorySphere test configuration");

    constexpr std::size_t N = 4;
    std::vector<std::shared_ptr<FactorySphere>> inds;
    for(std::size_t i = 0; i < N; ++i) {
        auto ind = f.get_as<FactorySphere>();
        REQUIRE(ind);
        inds.push_back(ind);
    }

    // Every produced individual binds to the SAME shared layout instance (built once).
    std::shared_ptr<const GGenomeLayout> layout0 = inds[0]->getLayout();
    REQUIRE(layout0);
    for(const auto &ind : inds) {
        CHECK(ind->getLayout().get() == layout0.get()); // pointer identity: one shared layout
    }

    // The layout reflects the config defaults (par_dim == 5 constrained doubles).
    CHECK(layout0->d.size() == 5);
    CHECK(inds[0]->countParameters<double>() == 5);

    // The value arrays are per-individual (independent): mutating one leaves the others untouched.
    std::vector<double> v0_before;
    std::vector<double> v1_before;
    inds[0]->streamline<double>(v0_before);
    inds[1]->streamline<double>(v1_before);
    CHECK(v0_before == v1_before); // same start values out of the shared genome

    inds[0]->randomInit(activityMode::ALLPARAMETERS);

    std::vector<double> v0_after;
    std::vector<double> v1_after;
    inds[0]->streamline<double>(v0_after);
    inds[1]->streamline<double>(v1_after);
    CHECK(v0_after != v0_before);  // inds[0] changed
    CHECK(v1_after == v1_before);  // inds[1] untouched -> independent value storage

    fs::remove(cfg);
}

/******************************************************************************/
TEST_CASE("GNeuralNetworkArchitecture computes per-layer weight offsets", "[architecture][flat]") {
    // A 2-4-4-1 feed-forward network (the example-09 default geometry).
    Gem::Geneva::Individuals::GNeuralNetworkArchitecture arch(
        std::vector<std::size_t>{2, 4, 4, 1}
    );

    CHECK(arch.name() == "GNeuralNetworkArchitecture");
    CHECK(arch.nLayers() == 4);

    // Weight counts: layer 0 = 2*2 = 4; layer 1 = 4*(2+1) = 12; layer 2 = 4*(4+1) = 20;
    // layer 3 = 1*(4+1) = 5. Total = 41.
    CHECK(arch.weightCount(0) == 4);
    CHECK(arch.weightCount(1) == 12);
    CHECK(arch.weightCount(2) == 20);
    CHECK(arch.weightCount(3) == 5);
    CHECK(arch.expectedFPSize() == 41);

    // Offsets are the running sum of the preceding layers' weight counts.
    CHECK(arch.layerOffset(0) == 0);
    CHECK(arch.layerOffset(1) == 4);
    CHECK(arch.layerOffset(2) == 16);
    CHECK(arch.layerOffset(3) == 36);

    CHECK(arch.layerSize(0) == 2);
    CHECK(arch.layerSize(3) == 1);
}

/******************************************************************************/
// NOTE: the former "GGridArchitecture reads a TREE genome identically" case was removed when the
// tree hierarchy was deleted (Phase 7). The "[flat][architecture]" case above already proves the
// architecture is layout-agnostic by reading the genome purely through the §2 streamlineFP() seam.
