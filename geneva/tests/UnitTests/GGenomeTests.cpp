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

#include <algorithm>
#include <any>
#include <cmath>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <random>
#include <ranges>
#include <vector>
#include <span>

#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GWireSerializationContext.hpp" // layout send-once: wire context + registry
#include "courtier/GConsumerRegistry.hpp"
#include "courtier/GSubmissionPolicy.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "courtier/consumers/GWebsocketConsumerT.hpp"
#include "courtier/transport/GWebsocketTransportT.hpp"
#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/transport/GAsioTransportT.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenomeLayout.hpp"
#include "geneva/ind/GGenomeLayoutSerialization.hpp" // ChannelLayout (de)serialisation (layout interning)
#include "geneva/Go2.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomFactory.hpp"
#include "geneva/ind/GGenome.hpp"
#include "geneva/ind/GIndividualFactory.hpp"
#include "geneva/GModuleLoader.hpp"
#include "geneva/GMarshallerPlugin.hpp" // marshallerManifest / GMarshallerProviderPtr / the marshaller store
#include "geneva/ind/GGenomeT.hpp"
#include "geneva/ind/GGenomeArchitecture.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/individuals/GNeuralNetworkIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Geneva::Genome;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace Gem::Tests {

/******************************************************************************/
// b2 guard (compile-time regression): GGenomeT's generated empty localMembers_() is gated on the
// public `gemfony_flat_individual` marker. A tagged genome-only leaf resolves a viable localMembers_();
// an UNtagged leaf that also declares no own localMembers_() has NONE -- GGenomeT's default is
// constrained out and the GReflectiveInterfaceBaseT fallback is =deleted -- so GReflectiveInterfaceAccess::members()
// is ill-formed and the class cannot compile. This pins the guard so a stateful leaf can never silently
// drop its state (Inv 15). The unmarked probe is only ever named in an unevaluated context, so its
// virtuals / vtable are never instantiated and it never hard-errors here.
namespace {
struct FlatTaggedProbe : public GGenomeT<FlatTaggedProbe> {
    using gemfony_flat_individual = void; // opts into the generated empty member list
};
struct FlatUntaggedProbe : public GGenomeT<FlatUntaggedProbe> {}; // no marker AND no own localMembers_()
static_assert(Gem::Common::GReflectiveInterfaceAccess::has_members<FlatTaggedProbe>,
    "b2: a marked genome-only leaf must resolve GGenomeT's generated empty member list");
static_assert(!Gem::Common::GReflectiveInterfaceAccess::has_members<FlatUntaggedProbe>,
    "b2: an unmarked leaf with no own localMembers_() must NOT resolve a member list (guard rotted)");
} // namespace

/******************************************************************************/
/**
 * A minimal flat individual: a sphere over n constrained doubles sharing one Gauss adaptor. It
 * demonstrates the intended authoring effort -- a constructor that builds the genome, a
 * evaluate(), and the serialize hook; clone/load/compare come from the CRTP base +
 * GGenome.
 */
class Sphere : public GGenomeT<Sphere> {
public:
    using gemfony_flat_individual = void; // b2: genome-only flat leaf -- opt into GGenomeT's empty localMembers_()
public:
    Sphere() { buildGenome(5); }
    explicit Sphere(std::size_t n) { buildGenome(n); }
    Sphere(const Sphere &) = default;

    /** @brief The OA-owned Gauss adaption config for this genome's double group(s). */
    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
            cfg->groupDouble(i).gauss(0.5, 0.8, 1e-3, 2., 1.);
        }
        return cfg;
    }

protected:
    std::vector<double> evaluate() override {
        std::vector<double> v;
        this->streamline<double>(v);
        return {std::ranges::fold_left(
            v | std::views::transform([](double x) { return x * x; }), 0., std::plus{})};
    }

private:
    void buildGenome(std::size_t n) {
        GGenomeBuilder b;
        b.addDoubleGroup(n, -10., 10.).init(1.0); // structure only; the adaptor lives on the OA config
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GGenomeT",
            boost::serialization::base_object<GGenomeT<Sphere>>(*this)
        );
    }
};

/******************************************************************************/
/**
 * A config-driven (Tier-2) flat individual: the same sphere, but its genome is built by the generic
 * GIndividualFactory from a Config that the factory reads from a configuration file. The default
 * constructor leaves the genome empty -- the factory installs it via setGenome() in postProcess_.
 */
class FactorySphere : public GGenomeT<FactorySphere> {
public:
    using gemfony_flat_individual = void; // b2: genome-only flat leaf -- opt into GGenomeT's empty localMembers_()
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
    static gen::GenomeData buildGenome(const Config &c) {
        GGenomeBuilder b;
        b.addDoubleGroup(c.par_dim, c.min, c.max); // structure only; the adaptor lives on the OA config
        return b.build();
    }

    /** @brief Optional hook: the OA-owned Gauss adaption config for a genome this individual produces.
     *  Exercised through GIndividualFactory::getAdaptionConfig(). */
    static std::shared_ptr<oa::GAdaptionConfigBase>
    buildAdaptionConfig(const Gem::Geneva::Genome::GGenome &sample, const Config &c) {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(sample);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
            cfg->groupDouble(i).gauss(c.sigma, 0.8, 1e-3, 2., 1.);
        }
        return cfg;
    }

protected:
    std::vector<double> evaluate() override {
        std::vector<double> v;
        this->streamline<double>(v);
        return {std::ranges::fold_left(
            v | std::views::transform([](double x) { return x * x; }), 0., std::plus{})};
    }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GGenomeT",
            boost::serialization::base_object<GGenomeT<FactorySphere>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::Sphere)    // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::FactorySphere) // NOLINT

using Gem::Tests::FactorySphere;
using Gem::Tests::Sphere;

/******************************************************************************/
// NOLINTNEXTLINE(readability-function-cognitive-complexity) -- one coherent test inspecting a single built layout+adaptor from several angles (structure fields, adaptor config, per-element bounds)
TEST_CASE("GGenomeBuilder produces the expected shared layout", "[flat]") {
    GGenomeBuilder b;
    b.addDoubleGroup(5, -10., 10.).init(1.0); // structure only -- adaptors are authored on the OA config
    gen::GenomeData g = b.build();

    REQUIRE(g.layout);
    const ChannelLayout<double> &ch = g.layout->d;
    REQUIRE(ch.size() == 5);
    REQUIRE(ch.groups.size() == 1);

    // The layout's group is STRUCTURE only: start / len / active (no adaptor fields, no stored range --
    // the natural scale is computed on demand from the bounds via ngScale).
    const GroupStructure<double> &grp = ch.groups[0];
    CHECK(grp.start == 0u);
    CHECK(grp.len == 5u);
    CHECK(grp.active);
    CHECK(grp.label_id == -1);
    CHECK(ngScale(ch, 0) == 20.); // upper - lower, computed from the bounds

    // The adaptor itself is authored on the OA-owned config (built from the genome's structure). It
    // mirrors the builder's old gaussAdaptor(...) one-to-one.
    Sphere ind;
    ind.setGenome(g);
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(ind);
    cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);
    const GroupSpec<double> &cgrp = cfg->doubleGroups()[0];
    CHECK(cgrp.has_gauss);
    CHECK(cgrp.start_sigma == 0.5);
    CHECK(cgrp.gauss.sigma_sigma == 0.8);
    CHECK(cgrp.gauss.min_sigma == 1e-3);
    CHECK(cgrp.gauss.max_sigma == 2.);

    for(std::size_t k = 0; k < 5; ++k) {
        CHECK(ch.lower[k] == -10.);
        CHECK(ch.upper[k] == 10.);
        CHECK(ch.fold[k]); // bounded
        CHECK(g.dv[k] == 1.0);
    }
}

/******************************************************************************/
// NOLINTNEXTLINE(readability-function-cognitive-complexity) -- one coherent property (layoutId is a stable, structure-sensitive content hash) checked from several angles against the SAME set of layouts; the checks are tightly coupled and would only be scattered by splitting
TEST_CASE("GGenomeLayout::layoutId is a stable content hash", "[flat][layoutid]") {
    auto buildLayout = [](std::size_t n, double lo, double hi) {
        GGenomeBuilder b;
        b.addDoubleGroup(n, lo, hi);
        return b.build().layout;
    };

    // Identical structure -> identical id, and sameStructure() agrees.
    auto a = buildLayout(5, -10., 10.);
    auto a2 = buildLayout(5, -10., 10.);
    REQUIRE(a);
    REQUIRE(a2);
    CHECK(a->layoutId() == a2->layoutId());
    CHECK(a->sameStructure(*a2));

    // Calling layoutId() twice returns the same cached value (idempotent).
    CHECK(a->layoutId() == a->layoutId());

    // A different parameter count, different bounds, or different grouping each changes the id.
    CHECK(a->layoutId() != buildLayout(6, -10., 10.)->layoutId());  // count
    CHECK(a->layoutId() != buildLayout(5, -1., 1.)->layoutId());    // bounds
    CHECK_FALSE(a->sameStructure(*buildLayout(6, -10., 10.)));

    {
        GGenomeBuilder b;
        b.addDoubleArray(5, -10., 10.); // 5 groups of 1 instead of one group of 5
        auto grouped_differently = b.build().layout;
        CHECK(a->layoutId() != grouped_differently->layoutId());
        CHECK_FALSE(a->sameStructure(*grouped_differently));
    }

    // Interned labels are part of the structure -> they participate in the id.
    {
        GGenomeBuilder lb;
        lb.addDoubleGroup(5, -10., 10.).label("x");
        auto labelled = lb.build().layout;
        CHECK(a->layoutId() != labelled->layoutId());
    }

    // A copy has a cold cache but must recompute the identical id (copy preserves structure).
    GGenomeLayout const copy(*a);
    CHECK(copy.layoutId() == a->layoutId());
    CHECK(copy.sameStructure(*a));

    // A different channel mix (an int group added) changes the id.
    {
        GGenomeBuilder mb;
        mb.addDoubleGroup(5, -10., 10.);
        mb.addInt32Group(2, -5, 5);
        CHECK(a->layoutId() != mb.build().layout->layoutId());
    }
}

/******************************************************************************/
// NOLINTNEXTLINE(readability-function-cognitive-complexity) -- one coherent wire-blob round-trip (encode -> decode -> re-hash) verified from several angles against the SAME layout; the steps are sequentially coupled and would only be scattered by splitting
TEST_CASE("GGenomeLayout::layoutId survives the WIRE-BLOB round-trip", "[flat][layoutid][wireblob]") {
    using Gem::Geneva::Genome::layoutToWireBlob;
    using Gem::Geneva::Genome::layoutFromWireBlob;

    auto roundtrip = [](const std::shared_ptr<const GGenomeLayout> &orig) {
        const LayoutId id1 = orig->layoutId();
        const std::string blob = layoutToWireBlob(*orig);
        const auto back = layoutFromWireBlob(blob);
        const LayoutId id2 = back->layoutId();
        const bool same = back->sameStructure(*orig);
        // id-stable AND structure-stable: this is what send-once relies on for a return-trip layout.
        CHECK(same);                 // false -> reconstruction changed the structure (serialize bug)
        CHECK(id2 == id1);           // false while same==true -> the HASH is unstable (hash bug)

        // The genome's inline layout-by-value path uses the OUTER archive, whose format follows the
        // consumer's serialization mode (may be XML/text, not binary). Does a non-binary round-trip
        // preserve the content id (which folds exact double bit patterns)?
        std::ostringstream oss;
        { boost::archive::xml_oarchive oa(oss); GGenomeLayout cp = *orig;
          oa << boost::serialization::make_nvp("l", cp); }
        auto back_xml = std::make_shared<GGenomeLayout>();
        std::istringstream iss(oss.str());
        { boost::archive::xml_iarchive ia(iss); ia >> boost::serialization::make_nvp("l", *back_xml); }
        CHECK(back_xml->sameStructure(*orig));      // structurally equal (same VALUES)?
        CHECK(back_xml->layoutId() == id1);         // but is the content id (bit-pattern hash) preserved?

        // Multi-hop: server->worker->server. Does re-serializing the RECONSTRUCTED layout keep the id?
        const auto back2 = layoutFromWireBlob(layoutToWireBlob(*back));
        CHECK(back2->layoutId() == id1);
        CHECK(back2->sameStructure(*orig));
    };

    SECTION("double array (n single-value groups)") {
        GGenomeBuilder b; b.addDoubleArray(1000, -10., 10.);
        Sphere ind; ind.setGenome(b.build()); roundtrip(ind.getLayout());
    }
    SECTION("double groups of 10 (the image case)") {
        GGenomeBuilder b; for(int t = 0; t < 100; ++t) { b.addDoubleGroup(10, -1., 1.); }
        Sphere ind; ind.setGenome(b.build()); roundtrip(ind.getLayout());
    }
    SECTION("the existing mixed layout") {
        GGenomeBuilder b;
        b.addDoubleGroup(4, -10., 10.); b.addDoubleArray(3, -2., 2.);
        b.addInt32Group(2, -5, 5); b.addBoolGroup(2);
        Sphere ind; ind.setGenome(b.build()); roundtrip(ind.getLayout());
    }
}

/******************************************************************************/
TEST_CASE("GGenomeLayout::layoutId survives a serialization round-trip", "[flat][layoutid]") {
    // The structure round-trips losslessly, so a deserialised genome's layout must carry the same id as
    // the original -- this is exactly what lets a receiver match a sent layout to a cached one by id.
    GGenomeBuilder b;
    b.addDoubleGroup(4, -10., 10.);
    b.addDoubleArray(3, -2., 2.);
    b.addInt32Group(2, -5, 5);
    b.addBoolGroup(2);
    Sphere ind;
    ind.setGenome(b.build());
    const LayoutId before = ind.getLayout()->layoutId();

    Sphere restored;
    restored.fromString(ind.toString(Gem::Common::serializationMode::BINARY),
                        Gem::Common::serializationMode::BINARY);
    CHECK(restored.getLayout()->layoutId() == before);
    CHECK(restored.getLayout()->sameStructure(*ind.getLayout()));
}

/******************************************************************************/
TEST_CASE("GGenome::streamlineInto matches streamline (bulk-flatten fast path)", "[flat]") {
    // streamlineInto() is the GPU marshallers' bulk-flatten fast path: it must produce EXACTLY the same
    // external (range-folded) values as streamline<T>(), just written straight into a caller buffer with
    // no temporary vector. Sphere's group is Constrained, so the per-element fold is exercised.
    Sphere ind(7);
    ind.randomInit(activityMode::ACTIVEONLY); // vary the stored values so the fold actually does work

    std::vector<double> via_streamline;
    ind.streamline<double>(via_streamline);

    std::vector<double> via_into(via_streamline.size() + 1, -987.0); // +1 sentinel to catch any overrun
    const std::size_t n = ind.streamlineInto(via_into.data());

    REQUIRE(n == via_streamline.size());
    for(std::size_t i = 0; i < n; ++i) {
        CHECK(via_into[i] == via_streamline[i]); // identical folded values, identical order
    }
    CHECK(via_into[n] == -987.0); // wrote exactly n values, nothing past the end
}

/******************************************************************************/
TEST_CASE("GGenomeBuilder: groups vs arrays vs single parameters", "[flat]") {
    GGenomeBuilder b;
    b.addDouble(0., -1., 1.);          // group of 1
    b.addDoubleGroup(4, -2., 2.);      // ONE group of 4 (shared sigma)
    b.addDoubleArray(3, -3., 3.);      // 3 groups of 1
    gen::GenomeData const g = b.build();

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
    gen::GenomeData const g = b.build();

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

    // Serialize round-trip (through a GGenome) preserves the labels + their resolution.
    Sphere ind;
    ind.setGenome(g);
    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);

    Sphere restored;
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
// individual's GENOME payload is OA-agnostic -- the OA scratch (the personality object + per-group
// adaption POD state) rides in the individual's own GAuxiliaryStore, excluded from the compared identity
// and omitted on the wire (see the [aux] tests), and post-processing eligibility is decided by the
// algorithm and vetoed on the work item's processing metadata, leaving no OA-identity mnemonic on the genome.)

/******************************************************************************/
TEST_CASE("GGenome: layout interning round-trips losslessly (compact + escape route)", "[flat]") {
    // COMPACT PATH: a genome whose channels are all group-uniform (the normal case) must round-trip with
    // its per-value layout arrays reconstructed EXACTLY from the compact per-group wire form.
    GGenomeBuilder b;
    b.addDoubleGroup(4, -10., 10.); // one uniform double group of 4
    b.addDoubleArray(3, -2., 2.);   // 3 single-value double groups
    b.addInt32Group(2, -5, 5);      // one int group
    b.addBoolGroup(2);              // one bool group
    Sphere ind;
    ind.setGenome(b.build());
    auto L = ind.getLayout();
    REQUIRE(L);

    Sphere restored;
    restored.fromString(ind.toString(Gem::Common::serializationMode::BINARY),
                        Gem::Common::serializationMode::BINARY);
    auto RL = restored.getLayout();
    REQUIRE(RL);
    // The reconstructed per-value arrays must be identical to the original.
    CHECK(RL->d.lower == L->d.lower);
    CHECK(RL->d.upper == L->d.upper);
    CHECK(RL->d.init_lower == L->d.init_lower);
    CHECK(RL->d.init_upper == L->d.init_upper);
    CHECK(RL->d.fold == L->d.fold);
    CHECK(RL->d.active == L->d.active);
    CHECK(RL->i.lower == L->i.lower);
    CHECK(RL->i.upper == L->i.upper);
    CHECK(RL->i.fold == L->i.fold);
    CHECK(RL->b.active == L->b.active);

    // ESCAPE ROUTE: a channel with per-value variation WITHIN a group (not producible by the builder, but
    // representable by the data structure) must fall back to full per-value serialisation and round-trip
    // exactly. Build one by hand and round-trip the ChannelLayout directly.
    ChannelLayout<double> ch;
    ch.groups.push_back(GroupStructure<double>{0u, 3u, -1, true}); // one group of 3, NON-uniform below
    ch.lower = {-1., -5., -9.};
    ch.upper = {1., 5., 9.};
    ch.init_lower = {-1., -5., -9.};
    ch.init_upper = {1., 5., 9.};
    ch.fold = {std::uint8_t{1}, std::uint8_t{1}, std::uint8_t{0}};
    ch.active = {1, 1, 1};

    std::ostringstream oss;
    {
        boost::archive::binary_oarchive oa(oss);
        oa << ch;
    }
    ChannelLayout<double> ch2;
    {
        std::istringstream iss(oss.str());
        boost::archive::binary_iarchive ia(iss);
        ia >> ch2;
    }
    CHECK(ch2.lower == ch.lower);
    CHECK(ch2.upper == ch.upper);
    CHECK(ch2.init_lower == ch.init_lower);
    CHECK(ch2.init_upper == ch.init_upper);
    CHECK(ch2.fold == ch.fold); // including the per-value bounded/unbounded variation
    CHECK(ch2.active == ch.active);
}

/******************************************************************************/
TEST_CASE("GGenome: value round-trip (streamline / assignValueVector)", "[flat]") {
    Sphere ind(4);

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
// NOLINTNEXTLINE(readability-function-cognitive-complexity) -- one coherent property (the constrained-fold helpers map values into their range) exercised across many boundary/interior inputs in a tight data-driven loop; the length is input breadth, not branching complexity
TEST_CASE("Constrained fold helpers map into range", "[flat]") {
    // FP: half-open [lo, hi)
    CHECK(foldConstrainedFP<double>(5., -10., 10.) == 5.);   // in range -> identity
    CHECK(foldConstrainedFP<double>(25., -10., 10.) == -5.); // reflected
    for(double const x : {-37.3, -11., 9.999, 100.25, 10.0}) {
        const double f = foldConstrainedFP<double>(x, -10., 10.);
        CHECK(f >= -10.);
        CHECK(f < 10.);
    }

    // Int: closed [lo, hi]
    CHECK(foldConstrainedInt<std::int32_t>(7, -10, 10) == 7);
    for(std::int32_t const x : {-100, -11, 11, 250}) {
        const std::int32_t f = foldConstrainedInt<std::int32_t>(x, -10, 10);
        CHECK(f >= -10);
        CHECK(f <= 10);
    }

    // FROZEN parameter: lo == hi is a degenerate/empty range; any value folds to that single point
    // (no division by a zero range). Mirrors a user freezing a parameter by equal bounds.
    CHECK(foldConstrainedFP<double>(4., 4., 4.) == 4.);
    CHECK(foldConstrainedFP<double>(-7.3, 4., 4.) == 4.);
    CHECK(foldConstrainedFP<double>(100., 4., 4.) == 4.);
}

/******************************************************************************/
TEST_CASE("GGenome: clone is independent", "[flat]") {
    Sphere const ind(6);

    auto twin = ind.clone<Sphere>();

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
TEST_CASE("GGenome: serialization round-trip", "[flat]") {
    Sphere ind(5);
    ind.assignValueVector<double>(std::vector<double>{1., -2., 3., -4., 5.});

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);

    Sphere restored;
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
// CHARACTERIZATION NET (B0, 2026-06-28): outcome-pins for the individual-architecture swap. These
// assert behaviour that must survive the GProcessable / GOptimizableEntity / GGenome rebuild,
// independent of the mechanisms being retired (results-only wire form, the population slot, genome_omitted).
// See prompts/2026-06-28-characterization-net.md.

// External-result acceptance (D14): a precomputed evaluation injected via process(res_vec) is taken
// VERBATIM and marks the item PROCESSED, WITHOUT invoking the local evaluate(). This is the
// GPU / external-marshaller path; pinned so the contract survives the swap (and a prospective
// acceptEvaluationResults rename of the injection entry point).
TEST_CASE("GGenome: external evaluation result is accepted verbatim (no local evaluate())",
          "[flat][external]") {
    Sphere ind(5);
    // Place the genome where the true sphere fitness is a known NON-zero value (five 1.0s -> 5.0), so
    // an injected result that differs proves the external value was taken, not locally computed.
    ind.assignValueVector<double>(std::vector<double>{1., 1., 1., 1., 1.});

    const std::size_t n = ind.getNStoredResults();
    REQUIRE(n >= 1);
    const double injected = 42.0; // deliberately != the true sphere fitness (5.0)
    std::vector<individual_processing_result> res;
    res.emplace_back(injected);
    for(std::size_t k = 1; k < n; ++k) {
        res.emplace_back(0.0); // res_vec size must match the stored-result count
    }

    ind.mark_as_due_for_processing();
    ind.process(res);

    CHECK(ind.is_processed());
    CHECK(ind.getStoredResult(0).rawFitness() == injected); // external value taken verbatim
}

// A derived individual round-trips its value identity in ALL THREE archive formats. The basic
// round-trip above exercises XML only; the swap rewrites the individual inheritance graph, and Boost
// class export/tracking is sensitive to that exact graph, so a re-run of this same test post-swap
// proves the new export macros produce valid archives in text, XML and binary alike.
TEST_CASE("GGenome: a derived individual round-trips in TEXT, XML and BINARY",
          "[flat][serialize][formats]") {
    using mode = Gem::Common::serializationMode;
    const std::vector<double> vals{1., -2., 3., -4., 5.};

    for(auto m : {mode::TEXT, mode::XML, mode::BINARY}) {
        Sphere ind(5);
        ind.assignValueVector<double>(vals);

        Sphere restored;
        REQUIRE_NOTHROW(restored.fromString(ind.toString(m), m));

        CHECK_NOTHROW(restored.compare(
            ind, Gem::Common::expectation::EQUALITY, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE));

        std::vector<double> v;
        restored.streamline<double>(v);
        CHECK(v == vals);
    }
}

/******************************************************************************/
TEST_CASE("GGenome: adapt() mutates within bounds", "[flat]") {
    Sphere ind(8);

    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    std::size_t total = 0;
    for(int round = 0; round < 20; ++round) {
        total += adapter.adapt(ind);

        std::vector<double> v;
        ind.streamline<double>(v);
        REQUIRE(v.size() == 8);
        for(double const x : v) {
            CHECK(x >= -10.); // streamline always folds back into the external range
            CHECK(x < 10.);
        }
    }
    CHECK(total > 0); // ad_prob == 1 -> every value is adapted every round
}

/******************************************************************************/
TEST_CASE("GGenome: randomInit stays within bounds and changes values", "[flat]") {
    Sphere ind(10);

    std::vector<double> before;
    ind.streamline<double>(before);

    CHECK(ind.randomInit(activityMode::ALLPARAMETERS));

    std::vector<double> after;
    ind.streamline<double>(after);
    CHECK(before != after);
    for(double const x : after) {
        CHECK(x >= -10.);
        CHECK(x < 10.);
    }
}

/******************************************************************************/
TEST_CASE("GGenome: OA stall-reset restores sigma to its seed", "[flat][oa]") {
    Sphere ind(3);

    // The OA-owned config drives both the sigma readout and the stall-reset. It is the
    // config the individual authors; the per-group adaption STATE is OA-owned scratch (held in the
    // individual's own GAuxiliaryStore in a live run) -- here a standalone GAuxiliaryStore, seeded from the config.
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

class Mixed : public GGenomeT<Mixed> {
public:
    using gemfony_flat_individual = void; // b2: genome-only flat leaf -- opt into GGenomeT's empty localMembers_()
public:
    Mixed() {
        GGenomeBuilder b;
        b.addDoubleGroup(3, -5., 5.).init(2.0); // structure only; the adaptors live on the OA config
        b.addInt32Group(4, -10, 10).init(3);
        b.addBoolGroup(5).init(false);
        this->setGenome(b.build());
    }
    Mixed(const Mixed &) = default;

    /** @brief The OA-owned config: a bi-Gauss adaptor on the FP group, flip adaptors on the int + bool groups. */
    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupDouble(0).biGauss(0.5, 0.8, 1e-3, 2., 0.5, 0.8, 1e-3, 2., 0.5, 0.8, 0., 2., 1.);
        cfg->groupInt32(0).flip(1.0);
        cfg->groupBool(0).flip(1.0);
        return cfg;
    }

protected:
    std::vector<double> evaluate() override { return {0.}; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GGenomeT",
            boost::serialization::base_object<GGenomeT<Mixed>>(*this)
        );
    }
};

/**
 * A flat individual carrying an integer Gauss adaptor (the prerequisite for GMetaOptimizerIndividualT,
 * which mutates n_children with GInt32GaussAdaptor while n_parents uses a flip adaptor). Four int32
 * groups: three (groups 0–2) driven by the integer Gauss kernel and one (group 3) by the flip kernel, so the
 * test confirms both int adaptor kinds coexist on the same channel.
 */
class IntGauss : public GGenomeT<IntGauss> {
public:
    using gemfony_flat_individual = void; // b2: genome-only flat leaf -- opt into GGenomeT's empty localMembers_()
public:
    IntGauss() {
        GGenomeBuilder b;
        // Gauss-adapted constrained ints in [-50, 50], own state each (groups 0..2).
        b.addInt32Array(3, -50, 50).init(0);
        // Flip-adapted constrained ints in [-10, 10], shared state (group 3).
        b.addInt32Group(2, -10, 10).init(5);
        this->setGenome(b.build());
    }
    IntGauss(const IntGauss &) = default;

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
    std::vector<double> evaluate() override { return {0.}; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GGenomeT",
            boost::serialization::base_object<GGenomeT<IntGauss>>(*this)
        );
    }
};

/**
 * A flat individual with MANY single-value double groups (an addDoubleArray). Its layout is O(groups),
 * so it makes the transport layout send-once visible: the full layout is sizeable, but every item after
 * the first to a peer carries only a 16-byte layout id.
 */
class ManyGroups : public GGenomeT<ManyGroups> {
public:
    using gemfony_flat_individual = void; // b2: genome-only flat leaf -- opt into GGenomeT's empty localMembers_()
public:
    ManyGroups() { build(64); }
    explicit ManyGroups(std::size_t n) { build(n); }
    ManyGroups(const ManyGroups &) = default;

protected:
    std::vector<double> evaluate() override {
        std::vector<double> v;
        this->streamline<double>(v);
        return {std::ranges::fold_left(
            v | std::views::transform([](double x) { return x * x; }), 0., std::plus{})};
    }

private:
    void build(std::size_t n) {
        GGenomeBuilder b;
        b.addDoubleArray(n, -10., 10.); // n single-value groups -> an O(n) layout
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GGenomeT",
            boost::serialization::base_object<GGenomeT<ManyGroups>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::Mixed)       // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::IntGauss)    // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::ManyGroups)  // NOLINT

using Gem::Tests::ManyGroups;
using Gem::Tests::Mixed;

/******************************************************************************/
TEST_CASE("GGenome: flip adaptor mutates int32 and bool channels", "[flat][flip]") {
    Mixed ind;

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
        for(std::int32_t const x : i_now) {
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
TEST_CASE("GGenome: bi-gaussian adaptor mutates the FP channel within bounds", "[flat][bigauss]") {
    Mixed ind;

    std::vector<double> before;
    ind.streamline<double>(before);
    REQUIRE(before.size() == 3);

    std::size_t total = 0;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int round = 0; round < 20; ++round) {
        total += adapter.adapt(ind);
        std::vector<double> v;
        ind.streamline<double>(v);
        for(double const x : v) {
            CHECK(x >= -5.);
            CHECK(x < 5.);
        }
    }
    CHECK(total > 0);
}

/******************************************************************************/
TEST_CASE("GGenome: mixed flip/bigauss genome serialises round-trip", "[flat][flip][bigauss]") {
    Mixed ind;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int i = 0; i < 5; ++i) {
        adapter.adapt(ind);
    }

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);
    Mixed restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    CHECK_NOTHROW(restored.compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));
}

using Gem::Tests::IntGauss;

/******************************************************************************/
// NOLINTNEXTLINE(readability-function-cognitive-complexity) -- one coherent stochastic robustness test: the 30-round mutation loop checks Gauss- and flip-group bounds together against the SAME evolving state
TEST_CASE("GGenome: integer Gauss adaptor mutates int32 within bounds", "[flat][intgauss]") {
    IntGauss ind;

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
TEST_CASE("GGenome: integer Gauss genome serialises round-trip", "[flat][intgauss]") {
    IntGauss ind;
    oa::StandaloneAdapter adapter(ind, ind.getAdaptionConfig());
    for(int i = 0; i < 5; ++i) {
        adapter.adapt(ind);
    }

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);
    IntGauss restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    CHECK_NOTHROW(restored.compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));
}

/******************************************************************************/
TEST_CASE("GGridArchitecture reads any genome through the §2 seam (flat)", "[flat][architecture]") {
    Sphere const ind(12); // 12 FP values -> a 3x4 grid
    GGridArchitecture const grid(3, 4);

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
TEST_CASE("GIndividualFactory: one shared layout across N produced individuals", "[flat][factory]") {
    // A temporary config file. writeConfigFile() generates it from the individual's describeConfig()
    // defaults; get_as<>() then reads it back and produces individuals.
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "geneva_flat_factory_tests";
    fs::create_directories(base);
    const fs::path cfg = base / "FactorySphere.json";

    GIndividualFactory<FactorySphere> f(cfg);
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
TEST_CASE("GIndividualFactory::getAdaptionConfig delegates to buildAdaptionConfig", "[flat][factory]") {
    // The factory's getAdaptionConfig() forwards to the individual's optional buildAdaptionConfig hook
    // and returns the OA-owned config matching the produced genome (used with registerAdaptionConfig).
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "geneva_flat_factory_tests";
    fs::create_directories(base);
    const fs::path cfg = base / "FactorySphereAdaption.json";

    GIndividualFactory<FactorySphere> f(cfg);
    f.writeConfigFile("FactorySphere adaption-config test");

    auto sample = f.get_as<FactorySphere>();
    REQUIRE(sample);

    auto ac = f.getAdaptionConfig(*sample);
    REQUIRE(ac);                              // the hook is present -> a real config (not null)
    CHECK(ac->doubleGroups().size() == 1);    // one double group, matching the par_dim==5 genome

    fs::remove(cfg);
}

/******************************************************************************/
TEST_CASE("GNeuralNetworkArchitecture computes per-layer weight offsets", "[architecture][flat]") {
    // A 2-4-4-1 feed-forward network (the example-09 default geometry).
    Gem::Geneva::Individuals::GNeuralNetworkArchitecture const arch(
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
// Wire transport: the layout send-once wire form. These exercise GGenome::save()/load() under an
// active Gem::Courtier::GWireSerializationScope, simulating the server->worker wire path WITHOUT any real
// transport: a server-side registry interns each distinct layout, the first item to a peer carries the
// full layout and every later item only its 16-byte id, and a worker resolves an id-only item from its
// local cache or (on a miss) via a fetch callback.

namespace {
using Gem::Courtier::GWireLayoutId;
using Gem::Courtier::GWireLayoutRegistry;
using Gem::Courtier::GWireSerializationContext;
using Gem::Courtier::GWireSerializationScope;

GWireLayoutId widOf(const ManyGroups &ind) {
    const auto lid = ind.getLayout()->layoutId();
    return GWireLayoutId{lid.hi, lid.lo};
}
std::vector<double> valuesOf(ManyGroups &ind) {
    std::vector<double> v;
    ind.streamline<double>(v);
    return v;
}
} // namespace

/******************************************************************************/
TEST_CASE("Wire send-once: first item carries the layout, later items only the id", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    ManyGroups a(64);
    ManyGroups c(64);
    a.randomInit(activityMode::ALLPARAMETERS);
    c.randomInit(activityMode::ALLPARAMETERS);
    // Two independently-built genomes of the same shape share one content id.
    REQUIRE(widOf(a) == widOf(c));
    const std::vector<double> a_vals = valuesOf(a);
    const std::vector<double> c_vals = valuesOf(c);

    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.peer = 1;
    server_ctx.registry = &server_reg;

    std::string s_first;
    std::string s_second;
    {
        GWireSerializationScope const scope(&server_ctx);
        s_first = a.toString(mode::BINARY);  // first to peer 1 -> full layout inline
        s_second = c.toString(mode::BINARY); // same layout id -> id-only
    }
    // The layout was interned once and peer 1 is recorded as holding it.
    CHECK(server_reg.size() == 1);
    CHECK(server_reg.peerHasLayout(1, widOf(a)));
    // The id-only item is materially smaller (the whole O(groups) layout dropped to a 16-byte id).
    CHECK(s_second.size() < s_first.size());

    // Worker side: a fresh cache. The first item installs the layout; the id-only second resolves locally.
    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.peer = 0;
    worker_ctx.registry = &worker_reg;

    ManyGroups ra;
    ManyGroups rc;
    {
        GWireSerializationScope const scope(&worker_ctx);
        ra.fromString(s_first, mode::BINARY);
        CHECK(worker_reg.size() == 1); // the worker cached the layout it received
        rc.fromString(s_second, mode::BINARY);
    }
    REQUIRE(ra.getLayout());
    REQUIRE(rc.getLayout());
    CHECK(ra.getLayout()->sameStructure(*a.getLayout()));
    CHECK(rc.getLayout()->sameStructure(*c.getLayout()));
    CHECK(valuesOf(ra) == a_vals);
    CHECK(valuesOf(rc) == c_vals); // id-only item reconstructed its own values + the shared layout
}

/******************************************************************************/
TEST_CASE("Wire send-once: a cache miss is resolved by the fetch fallback", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    ManyGroups a(48);
    a.randomInit(activityMode::ALLPARAMETERS);
    const std::vector<double> a_vals = valuesOf(a);

    // Server interns the layout and emits an id-only item (peer already "holds" the layout).
    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.peer = 1;
    server_ctx.registry = &server_reg;
    std::string s_idonly;
    {
        GWireSerializationScope const scope(&server_ctx);
        (void)a.toString(mode::BINARY);          // first send: marks peer 1 as holding the layout
        s_idonly = a.toString(mode::BINARY);     // second send: id-only
    }

    // A late-joining / reconnected worker with an EMPTY cache receives the id-only item. Its fetch
    // callback pulls the blob from the server registry (the REQUEST_LAYOUT/SEND_LAYOUT round trip).
    GWireLayoutRegistry worker_reg;
    std::size_t fetch_calls = 0;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.peer = 0;
    worker_ctx.registry = &worker_reg;
    worker_ctx.fetch_blob = [&](const GWireLayoutId &id) -> std::expected<std::string, std::string> {
        ++fetch_calls;
        std::string blob;
        if(server_reg.tryGet(id, blob)) {
            return blob;
        }
        return std::unexpected(std::string("layout id not held by the server"));
    };

    ManyGroups r;
    {
        GWireSerializationScope const scope(&worker_ctx);
        r.fromString(s_idonly, mode::BINARY); // miss -> fetch -> reconstruct
    }
    CHECK(fetch_calls == 1);
    CHECK(worker_reg.size() == 1); // the fetched blob is now cached
    REQUIRE(r.getLayout());
    CHECK(r.getLayout()->sameStructure(*a.getLayout()));
    CHECK(valuesOf(r) == a_vals);
}

/******************************************************************************/
TEST_CASE("Wire send-once: an unresolvable id-only reference throws", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    ManyGroups const a(16);
    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.peer = 1;
    server_ctx.registry = &server_reg;
    std::string s_idonly;
    {
        GWireSerializationScope const scope(&server_ctx);
        (void)a.toString(mode::BINARY);
        s_idonly = a.toString(mode::BINARY);
    }

    // Empty cache, no fetch callback -> the miss cannot be resolved and load() must throw rather than
    // silently produce a wrong/empty layout.
    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    ManyGroups r;
    {
        GWireSerializationScope const scope(&worker_ctx);
        CHECK_THROWS(r.fromString(s_idonly, mode::BINARY));
    }
}

/******************************************************************************/
TEST_CASE("Wire send-once: default-off encoding is self-contained and interoperable", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    ManyGroups a(32);
    a.randomInit(activityMode::ALLPARAMETERS);
    const std::vector<double> a_vals = valuesOf(a);

    // No active scope -> the self-contained full-layout form (the checkpoint / file path).
    const std::string s_full = a.toString(mode::BINARY);

    ManyGroups r;
    r.fromString(s_full, mode::BINARY); // loads with no scope
    REQUIRE(r.getLayout());
    CHECK(r.getLayout()->sameStructure(*a.getLayout()));
    CHECK(valuesOf(r) == a_vals);

    // A self-contained (interned=false) stream also loads fine UNDER a worker scope (the tag, not the
    // reader's scope, decides the form).
    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    ManyGroups r2;
    {
        GWireSerializationScope const scope(&worker_ctx);
        r2.fromString(s_full, mode::BINARY);
    }
    CHECK(r2.getLayout()->sameStructure(*a.getLayout()));
    CHECK(valuesOf(r2) == a_vals);
    CHECK(worker_reg.size() == 0); // a full-form stream does not populate the cache by id
}

/******************************************************************************/
TEST_CASE("Wire results-only return: genome omitted, grafted from the original", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    // The originally-submitted item the server still holds (full genome).
    ManyGroups original(24);
    original.randomInit(activityMode::ALLPARAMETERS);
    const std::vector<double> original_vals = valuesOf(original);

    // The worker's processed copy: same genome, plus a computed result.
    auto worker_copy = original.clone<ManyGroups>();
    worker_copy->process(); // evaluates -> PROCESSED with a stored result
    REQUIRE(worker_copy->is_processed());
    const double worker_fitness = worker_copy->getStoredResult(0).rawFitness();

    // Worker serializes a RESULT in the default (results-only) return form.
    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    worker_ctx.returning = true; // this endpoint returns results to the server
    std::string s_results_only;
    {
        GWireSerializationScope const scope(&worker_ctx);
        s_results_only = worker_copy->toString(mode::BINARY);
    }

    // A full serialization of the same item is materially larger (it carries the 24-group genome).
    auto full_copy = original.clone<ManyGroups>();
    full_copy->process();
    const std::string s_full = full_copy->toString(mode::BINARY); // no scope -> self-contained
    CHECK(s_results_only.size() < s_full.size());

    // Server deserializes the results-only return: genome omitted, results present.
    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.registry = &server_reg; // returning stays false (the server submits, not returns)
    ManyGroups received;
    {
        GWireSerializationScope const scope(&server_ctx);
        received.fromString(s_results_only, mode::BINARY);
    }
    CHECK(received.inputDataOmitted());
    CHECK(received.countParameters<double>() == 0);   // no input parameters arrived
    CHECK(received.is_processed());                    // but the computed result did
    CHECK(received.getStoredResult(0).rawFitness() == worker_fitness);

    // Graft the parameters from the originally-submitted item (what checkin() does on the server).
    received.graftInputDataFrom(original);
    CHECK_FALSE(received.inputDataOmitted());
    CHECK(valuesOf(received) == original_vals);         // parameters restored from the original
    CHECK(received.getStoredResult(0).rawFitness() == worker_fitness); // result preserved
}

/******************************************************************************/
TEST_CASE("In-place return-reconciliation primitives keep genome / relocate nothing", "[flat][identity]") {
    // Direct guard for the two GProcessable primitives that make the networked reconciliation
    // pointer-stable (the demo-container policy tests exercise only the clone-and-replace FALLBACK, not
    // the geneva overrides):
    //   - absorbResultsFrom(): the server keeps THIS element's genome (+ scratch) and takes only results;
    //   - loadContentFrom():   a full in-place deep copy (genome + results) without relocation.
    ManyGroups server_item(24);
    server_item.randomInit(activityMode::ALLPARAMETERS);
    const std::vector<double> server_vals = valuesOf(server_item);

    // A returned worker result with a DIFFERENT genome (independent RNG draw off the shared stream), so
    // "genome kept" vs "genome taken" is observable; processed, so it carries a result + status.
    auto returned = server_item.clone<ManyGroups>();
    returned->randomInit(activityMode::ALLPARAMETERS);
    returned->process();
    REQUIRE(returned->is_processed());
    const std::vector<double> returned_vals = valuesOf(*returned);
    const double returned_fitness = returned->getStoredResult(0).rawFitness();
    REQUIRE(server_vals != returned_vals); // the two genomes really differ (guards the checks below)

    // absorbResultsFrom(): results absorbed, genome KEPT.
    server_item.absorbResultsFrom(*returned);
    CHECK(server_item.is_processed());
    CHECK(server_item.getStoredResult(0).rawFitness() == returned_fitness);
    CHECK(valuesOf(server_item) == server_vals);   // genome untouched (results-only semantics)

    // loadContentFrom(): full in-place deep copy; the object is not relocated (its address is fixed here,
    // documenting the contract the networked refill relies on) and returns true for a geneva individual.
    ManyGroups failed(24);
    failed.randomInit(activityMode::ALLPARAMETERS);
    const GOptimizableEntity *failed_addr = &failed;
    const bool did_load = failed.loadContentFrom(*returned);
    CHECK(did_load);                               // geneva supports in-place substitution
    CHECK(&failed == failed_addr);                 // no relocation
    CHECK(valuesOf(failed) == returned_vals);      // now a full copy of the source's genome
    CHECK(failed.is_processed());
    CHECK(failed.getStoredResult(0).rawFitness() == returned_fitness);
}

/******************************************************************************/
TEST_CASE("Wire results-only return: a client may opt into a full return", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    // A network-tiered client modifies the individual (here: re-initialises it) and returns it in full.
    ManyGroups submitted(24);
    submitted.randomInit(activityMode::ALLPARAMETERS);

    auto worker_copy = submitted.clone<ManyGroups>();
    worker_copy->randomInit(activityMode::ALLPARAMETERS); // a "better" individual the worker found
    worker_copy->process();
    worker_copy->setReturnFullIndividual(true); // <-- the opt-in
    const std::vector<double> worker_vals = valuesOf(*worker_copy);

    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    worker_ctx.returning = true;
    std::string s;
    {
        GWireSerializationScope const scope(&worker_ctx);
        s = worker_copy->toString(mode::BINARY);
    }

    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.registry = &server_reg;
    ManyGroups received;
    {
        GWireSerializationScope const scope(&server_ctx);
        received.fromString(s, mode::BINARY);
    }
    // A full return carried the (modified) genome -- no graft needed.
    CHECK_FALSE(received.inputDataOmitted());
    CHECK(received.countParameters<double>() == 24);
    CHECK(valuesOf(received) == worker_vals); // the worker's modified parameters travelled back
}

/******************************************************************************/
TEST_CASE("Wire send-once: large-genome wire-size before/after", "[flat][wire]") {
    // Quantifies the headline win on a large structured genome (2000 single-value groups -> an O(2000)
    // layout). Reports and guards the per-item wire size for the submit direction (full layout vs
    // id-only) and the return direction (full individual vs results-only).
    using mode = Gem::Common::serializationMode;

    ManyGroups big(2000);
    big.randomInit(activityMode::ALLPARAMETERS);

    // --- submit direction ---
    // Self-contained full encoding (no scope) -- the size before send-once.
    const std::size_t full_submit = big.toString(mode::BINARY).size();

    // Send-once: the first item to a peer carries the full layout, every later one only the 16-byte id.
    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.peer = 1;
    server_ctx.registry = &server_reg;
    std::size_t first_submit = 0;
    std::size_t idonly_submit = 0;
    {
        GWireSerializationScope const scope(&server_ctx);
        first_submit = big.toString(mode::BINARY).size();   // present=true (carries the layout)
        idonly_submit = big.toString(mode::BINARY).size();  // id-only
    }

    // --- return direction ---
    big.process(); // give it a result to return
    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    worker_ctx.returning = true;
    std::size_t full_return = 0;
    std::size_t results_only_return = 0;
    {
        GWireSerializationScope const scope(&worker_ctx);
        big.setReturnFullIndividual(true);
        full_return = big.toString(mode::BINARY).size();
        big.setReturnFullIndividual(false);
        results_only_return = big.toString(mode::BINARY).size();
    }

    WARN("Wire size (2000-group genome, binary bytes):"
         << "\n  submit  full=" << full_submit << "  first=" << first_submit
         << "  id-only=" << idonly_submit
         << "  (id-only is " << (100 * idonly_submit / full_submit) << "% of full)"
         << "\n  return  full=" << full_return << "  results-only=" << results_only_return
         << "  (results-only is " << (100 * results_only_return / full_return) << "% of full)");

    // The id-only submit drops the whole O(2000) layout, keeping only the values + a 16-byte id.
    CHECK(idonly_submit < full_submit);
    // The first send still carries the whole O(2000) layout: it is essentially the size of the
    // self-contained full encoding and dwarfs the id-only form. It is in fact a handful of bytes SMALLER
    // than the self-contained encoding, because a wire submit additionally omits the OA scratch (the
    // GAuxiliaryStore) that the self-contained encoding serializes by value -- so the meaningful pin is
    // that the first send is NOT the compressed id-only form, differing from `full` only by that tiny
    // omitted-scratch delta.
    CHECK(first_submit > idonly_submit);
    CHECK(full_submit >= first_submit);
    CHECK(full_submit - first_submit < 256);
    // The results-only return drops both the values and the layout, keeping only the computed results.
    CHECK(results_only_return < full_return);
    CHECK(results_only_return < idonly_submit); // no parameter values at all on a results-only return
}

/******************************************************************************/
TEST_CASE("Wire send-once over a real websocket loopback interns one layout", "[flat][wire][net]") {
    // End-to-end proof that the layout send-once form is correctly engaged on the live websocket path:
    // a population of identically-structured flat individuals is evaluated over real sockets, and the
    // server must intern exactly ONE layout for the whole population (across several clients), while
    // every work item still comes back correctly processed.
    namespace c2 = Gem::Courtier;
    namespace ccons = Gem::Courtier::Consumers;
    constexpr auto BIN = Gem::Common::serializationMode::BINARY;

    constexpr std::size_t N = 100;
    std::vector<std::unique_ptr<GOptimizableEntity>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        auto ind = std::make_unique<ManyGroups>(40); // all share one layout structure -> one id
        ind->randomInit(activityMode::ALLPARAMETERS);
        items.push_back(std::move(ind));
    }

    auto consumer =
        std::make_shared<c2::GWebsocketConsumerT<GOptimizableEntity>>(/*port=*/0, /*threads=*/2, BIN);
    // The work item is the abstract GOptimizableEntity base, so the consumer needs a polymorphic clone
    // (copy-construction would slice). This mirrors GConsumerSetup's individualCloneFunction().
    consumer->setCloneFunction(
        [](const std::unique_ptr<GOptimizableEntity> &p) { return p->clone_unique(); }
    );
    consumer->startServer();
    const unsigned short port = consumer->getPort();

    constexpr std::size_t n_clients = 3;
    std::vector<std::shared_ptr<ccons::GWebsocketClientT<GOptimizableEntity>>> clients;
    std::vector<std::thread> client_threads;
    std::atomic<bool> any_threw{false};
    for(std::size_t c = 0; c < n_clients; ++c) {
        auto client = std::make_shared<ccons::GWebsocketClientT<GOptimizableEntity>>(
            "127.0.0.1", port, BIN, /*verbose_control_frames=*/false, /*prefetch_depth=*/4
        );
        clients.push_back(client);
        client_threads.emplace_back([client, &any_threw] {
            try {
                client->run();
            }
            catch(...) {
                any_threw.store(true);
            }
        });
    }

    consumer->processBatch(std::span<std::unique_ptr<GOptimizableEntity>>(items.data(), items.size()),
                           c2::GSubmissionPolicy::full_success_or_fatal());

    for(auto &client : clients) {
        client->flagCloseRequested();
    }
    for(auto &t : client_threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    std::size_t processed = 0;
    std::size_t with_genome = 0;
    for(const auto &it : items) {
        if(it && it->is_processed()) {
            ++processed;
        }
        if(it && it->countParameters<double>() == 40) {
            ++with_genome;
        }
    }
    CHECK(processed == N); // correctness: every item came back evaluated
    CHECK(with_genome == N); // results-only returns must still leave each item with its full genome

    // Send-once: a single layout served the whole population over all clients (had each item carried its
    // own layout copy this would still be 1, since the blob store keys by content id -- but more to the
    // point, the interning path was exercised and is consistent).
    CHECK(consumer->getInternedLayoutCount() == 1);

    consumer->stopServer();
    CHECK_FALSE(any_threw.load());
}

/******************************************************************************/
TEST_CASE("Wire send-once over a real ASIO loopback interns one layout", "[flat][wire][net]") {
    // The ASIO twin of the websocket loopback test. ASIO uses a fresh one-shot connection per exchange,
    // so the server keys its per-peer send-once tracking on a stable id the client announces; with
    // prefetch + several clients this also exercises the cache-miss REQUEST_LAYOUT/SEND_LAYOUT fetch
    // path. Every item must still come back processed, and the server must intern exactly one layout.
    namespace c2 = Gem::Courtier;
    namespace ccons = Gem::Courtier::Consumers;
    constexpr auto BIN = Gem::Common::serializationMode::BINARY;

    constexpr std::size_t N = 100;
    std::vector<std::unique_ptr<GOptimizableEntity>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        auto ind = std::make_unique<ManyGroups>(40);
        ind->randomInit(activityMode::ALLPARAMETERS);
        items.push_back(std::move(ind));
    }

    auto consumer = std::make_shared<c2::GAsioConsumerT<GOptimizableEntity>>(/*port=*/0, /*threads=*/2, BIN);
    consumer->setCloneFunction(
        [](const std::unique_ptr<GOptimizableEntity> &p) { return p->clone_unique(); }
    );
    consumer->startServer();
    const unsigned short port = consumer->getPort();

    constexpr std::size_t n_clients = 3;
    std::vector<std::shared_ptr<ccons::GAsioConsumerClientT<GOptimizableEntity>>> clients;
    std::vector<std::thread> client_threads;
    std::atomic<bool> any_threw{false};
    for(std::size_t c = 0; c < n_clients; ++c) {
        auto client = std::make_shared<ccons::GAsioConsumerClientT<GOptimizableEntity>>(
            "127.0.0.1", port, BIN, /*max_reconnects=*/50, /*prefetch_depth=*/8
        );
        clients.push_back(client);
        client_threads.emplace_back([client, &any_threw] {
            try {
                client->run();
            }
            catch(...) {
                any_threw.store(true);
            }
        });
    }

    consumer->processBatch(std::span<std::unique_ptr<GOptimizableEntity>>(items.data(), items.size()),
                           c2::GSubmissionPolicy::full_success_or_fatal());

    for(auto &client : clients) {
        client->flagCloseRequested();
    }
    for(auto &t : client_threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    std::size_t processed = 0;
    std::size_t with_genome = 0;
    for(const auto &it : items) {
        if(it && it->is_processed()) {
            ++processed;
        }
        if(it && it->countParameters<double>() == 40) {
            ++with_genome;
        }
    }
    CHECK(processed == N);
    CHECK(with_genome == N); // results-only returns must still leave each item with its full genome
    CHECK(consumer->getInternedLayoutCount() == 1);

    consumer->stopServer();
    CHECK_FALSE(any_threw.load());
}

/******************************************************************************/
TEST_CASE("Networked reconciliation keeps population elements at stable addresses",
          "[flat][wire][net][identity]") {
    // Regression guard (Inv 15) for the in-place return reconciliation: a networked return must be
    // absorbed INTO the originally-submitted population element, keeping its heap address, rather than
    // swapping the deserialized return in (which would free the original and relocate it). Address
    // stability is what lets a per-individual prefetch hold a snapshot of population addresses across a
    // submission -- if checkin() ever reverts to `slot = std::move(returned)`, the recorded addresses
    // change and this fails.
    namespace c2 = Gem::Courtier;
    namespace ccons = Gem::Courtier::Consumers;
    constexpr auto BIN = Gem::Common::serializationMode::BINARY;

    constexpr std::size_t N = 60;
    std::vector<std::unique_ptr<GOptimizableEntity>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        auto ind = std::make_unique<ManyGroups>(16);
        ind->randomInit(activityMode::ALLPARAMETERS);
        items.push_back(std::move(ind));
    }

    // Snapshot the object addresses BEFORE submission -- the fix must leave every one of them unchanged.
    const auto addrs_before = items
        | std::views::transform([](const auto &it) -> const GOptimizableEntity * { return it.get(); })
        | std::ranges::to<std::vector<const GOptimizableEntity *>>();

    auto consumer = std::make_shared<c2::GWebsocketConsumerT<GOptimizableEntity>>(/*port=*/0, /*threads=*/2, BIN);
    consumer->setCloneFunction(
        [](const std::unique_ptr<GOptimizableEntity> &p) { return p->clone_unique(); }
    );
    consumer->startServer();
    const unsigned short port = consumer->getPort();

    constexpr std::size_t n_clients = 3;
    std::vector<std::shared_ptr<ccons::GWebsocketClientT<GOptimizableEntity>>> clients;
    std::vector<std::thread> client_threads;
    std::atomic<bool> any_threw{false};
    for(std::size_t c = 0; c < n_clients; ++c) {
        auto client = std::make_shared<ccons::GWebsocketClientT<GOptimizableEntity>>(
            "127.0.0.1", port, BIN, /*verbose_control_frames=*/false, /*prefetch_depth=*/4
        );
        clients.push_back(client);
        client_threads.emplace_back([client, &any_threw] {
            try {
                client->run();
            }
            catch(...) {
                any_threw.store(true);
            }
        });
    }

    consumer->processBatch(std::span<std::unique_ptr<GOptimizableEntity>>(items.data(), items.size()),
                           c2::GSubmissionPolicy::full_success_or_fatal());

    for(auto &client : clients) {
        client->flagCloseRequested();
    }
    for(auto &t : client_threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    std::size_t processed = 0;
    std::size_t stable_address = 0;
    std::size_t with_genome = 0;
    for(std::size_t i = 0; i < items.size(); ++i) {
        if(items[i] && items[i]->is_processed()) {
            ++processed;
        }
        if(items[i].get() == addrs_before[i]) {
            ++stable_address; // the original object was reconciled in place, not swapped out
        }
        if(items[i] && items[i]->countParameters<double>() == 16) {
            ++with_genome;
        }
    }
    CHECK(processed == N);       // every item came back evaluated
    CHECK(stable_address == N);  // and at its ORIGINAL heap address (in-place reconciliation)
    CHECK(with_genome == N);     // results-only return still leaves each item its full genome

    consumer->stopServer();
    c2::GConsumerRegistryT<GOptimizableEntity>::instance().clear();
    CHECK_FALSE(any_threw.load());
}

/******************************************************************************/
TEST_CASE("Wire send-once: many distinct layouts under a bounded registry stay correct",
          "[flat][wire][net]") {
    // Dynamic-structure stress: a population mixing SEVERAL distinct genome structures (so the server
    // interns several layouts -- the same thing a NEAT-style run does as architectures evolve), evaluated
    // over real sockets with the server's layout cache bounded BELOW the number of distinct structures.
    // That forces least-recently-used eviction mid-run; the eviction-clears-acks rule then makes the
    // server re-inline an evicted layout on its next use. Every item must still come back processed.
    namespace c2 = Gem::Courtier;
    namespace ccons = Gem::Courtier::Consumers;
    constexpr auto BIN = Gem::Common::serializationMode::BINARY;

    const std::vector<std::size_t> sizes{8, 16, 24, 32}; // four distinct layout structures
    constexpr std::size_t N = 80;
    std::vector<std::unique_ptr<GOptimizableEntity>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        auto ind = std::make_unique<ManyGroups>(sizes[i % sizes.size()]);
        ind->randomInit(activityMode::ALLPARAMETERS);
        items.push_back(std::move(ind));
    }

    auto consumer = std::make_shared<c2::GWebsocketConsumerT<GOptimizableEntity>>(/*port=*/0, /*threads=*/2, BIN);
    consumer->setCloneFunction(
        [](const std::unique_ptr<GOptimizableEntity> &p) { return p->clone_unique(); }
    );
    consumer->setInternedLayoutCapacity(2); // below the 4 distinct layouts -> eviction is forced
    consumer->startServer();
    const unsigned short port = consumer->getPort();

    constexpr std::size_t n_clients = 3;
    std::vector<std::shared_ptr<ccons::GWebsocketClientT<GOptimizableEntity>>> clients;
    std::vector<std::thread> client_threads;
    std::atomic<bool> any_threw{false};
    for(std::size_t c = 0; c < n_clients; ++c) {
        auto client = std::make_shared<ccons::GWebsocketClientT<GOptimizableEntity>>(
            "127.0.0.1", port, BIN, /*verbose_control_frames=*/false, /*prefetch_depth=*/4
        );
        clients.push_back(client);
        client_threads.emplace_back([client, &any_threw] {
            try {
                client->run();
            }
            catch(...) {
                any_threw.store(true);
            }
        });
    }

    consumer->processBatch(std::span<std::unique_ptr<GOptimizableEntity>>(items.data(), items.size()),
                           c2::GSubmissionPolicy::full_success_or_fatal());

    for(auto &client : clients) {
        client->flagCloseRequested();
    }
    for(auto &t : client_threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    std::size_t processed = 0;
    std::size_t with_genome = 0;
    for(std::size_t i = 0; i < items.size(); ++i) {
        if(items[i] && items[i]->is_processed()) {
            ++processed;
        }
        // Each item must come back with ITS OWN (correctly-sized) genome, even across eviction + the
        // varying layouts -- not an empty or a wrong-layout genome.
        if(items[i] && items[i]->countParameters<double>() == sizes[i % sizes.size()]) {
            ++with_genome;
        }
    }
    CHECK(processed == N);                              // correct despite eviction + re-inline mid-run
    CHECK(with_genome == N);                           // each item kept its own full genome
    CHECK(consumer->getInternedLayoutCount() <= 2);    // the capacity bound was honoured

    consumer->stopServer();
    CHECK_FALSE(any_threw.load());
}

/******************************************************************************/
TEST_CASE("EA over a websocket consumer with results-only returns keeps full genomes", "[wire][net][ea]") {
    // Isolation test for the results-only-return path under a REAL optimization (many generations,
    // selection + adaption), as opposed to the single-batch executor.workOn loopbacks above. Runs an EA
    // over the websocket consumer for enough generations to pass the point where the MPI path was seen
    // to collapse, then asserts the best individual still has its full genome (not an empty one that
    // would evaluate a sphere to a spurious 0).
    namespace c2 = Gem::Courtier;
    namespace ccons = Gem::Courtier::Consumers;
    namespace oa = Gem::Geneva::OptimizationAlgorithms;
    constexpr auto BIN = Gem::Common::serializationMode::BINARY;

    auto consumer = std::make_shared<c2::GWebsocketConsumerT<GOptimizableEntity>>(/*port=*/0, /*threads=*/4, BIN);
    consumer->setCloneFunction(
        [](const std::unique_ptr<GOptimizableEntity> &p) { return p->clone_unique(); }
    );
    consumer->startServer();
    const unsigned short port = consumer->getPort();

    std::vector<std::shared_ptr<ccons::GWebsocketClientT<GOptimizableEntity>>> clients;
    std::vector<std::thread> client_threads;
    std::atomic<bool> any_threw{false};
    for(std::size_t c = 0; c < 4; ++c) {
        auto client = std::make_shared<ccons::GWebsocketClientT<GOptimizableEntity>>(
            "127.0.0.1", port, BIN, /*verbose=*/false, /*prefetch_depth=*/4
        );
        clients.push_back(client);
        client_threads.emplace_back([client, &any_threw] {
            try { client->run(); } catch(...) { any_threw.store(true); }
        });
    }

    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(40, 6);
    pop->setMaxIteration(120);
    Sphere const proto(8);
    for(std::size_t i = 0; i < 40; ++i) {
        pop->push_back(proto.clone_unique());
    }
    pop->setAdaptionConfig(proto.getAdaptionConfig());
    c2::GConsumerRegistryT<GOptimizableEntity>::instance().setConsumer(consumer);

    pop->optimize();
    auto best = pop->getBestGlobalIndividual<Sphere>();

    for(auto &client : clients) { client->flagCloseRequested(); }
    for(auto &t : client_threads) { if(t.joinable()) t.join(); }
    consumer->stopServer();
    c2::GConsumerRegistryT<GOptimizableEntity>::instance().clear(); // don't leak into other test cases

    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    CHECK(v.size() == 8);          // the best individual must keep its full genome
    CHECK_FALSE(any_threw.load());
}

/******************************************************************************/
// Runtime individual-plugin mechanism: the loader's failure handling and the one-individual-per-process
// rule. The successful load-and-optimize path is exercised end-to-end by example 19's integration test
// (it must build a .so, which a unit test cannot).
TEST_CASE("Module loader rejects a missing library cleanly", "[genome][plugin]") {
    // A missing file must raise a clean geneva exception (a "could not load" diagnostic), never crash.
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "geneva_no_such_module_xyz.so";
    CHECK_THROWS(Gem::Geneva::loadModule(missing));
}

// Toolchain-compatibility gate (GenevaCompat). The loader validates a module's fingerprint before touching
// any of its C++ contributions; here we exercise that gate directly (a .so is not needed) via the exposed
// comparator. Regression guard for the silent-crash hole where a same-GENEVA_VERSION module built with an
// incompatible compiler/stdlib/Boost/build-mode used to pass and then corrupt the process.
TEST_CASE("Module compat gate accepts this host's own fingerprint", "[flat][plugin][compat]") {
    const GenevaCompat self = Gem::Geneva::thisHostCompat();
    CHECK(Gem::Geneva::moduleCompatMismatch(self).empty());
}

TEST_CASE("Module compat gate rejects a mismatched toolchain, naming the axis", "[flat][plugin][compat]") {
    using Gem::Geneva::moduleCompatMismatch;
    const GenevaCompat host = Gem::Geneva::thisHostCompat();

    SECTION("compiler version") {
        GenevaCompat m = host;
        m.compiler_major += 1;
        const std::string d = moduleCompatMismatch(m);
        CHECK_FALSE(d.empty());
        CHECK(d.find("compiler major") != std::string::npos);
    }
    SECTION("standard-library version") {
        GenevaCompat m = host;
        m.stdlib_version += 1;
        const std::string d = moduleCompatMismatch(m);
        CHECK_FALSE(d.empty());
        CHECK(d.find("standard-library version") != std::string::npos);
    }
    SECTION("build-mode ABI flags") {
        GenevaCompat m = host;
        m.abi_flags ^= GENEVA_ABI_FLAG_ASAN; // pretend the module was built with AddressSanitizer
        const std::string d = moduleCompatMismatch(m);
        CHECK_FALSE(d.empty());
        CHECK(d.find("ABI flags") != std::string::npos);
    }
    SECTION("Geneva version") {
        GenevaCompat m = host;
        m.geneva_version += 1;
        const std::string d = moduleCompatMismatch(m);
        CHECK_FALSE(d.empty());
        CHECK(d.find("Geneva version") != std::string::npos);
    }
}

namespace {
/** @brief A trivial host-only GPU marshaller: the flat-genome flatten/scatter scaffolding is complete in the
 *  base, so a concrete marshaller is pure HOST code and needs no device code -- the problem's device kernel is
 *  a separate, config-referenced source the GPU consumer's backend compiles at runtime (NVRTC), not part of
 *  the marshaller. Used only to witness that a marshaller module contribution registers correctly. */
class ProbeGPUMarshaller final : public GBaseGPUMarshallerT<double> {};
} // anonymous namespace

// A runtime module may contribute a GPU marshaller (manifest kind GENEVA_CONTRIBUTION_MARSHALLER): the device
// adapter that makes --consumer gpu work for a loaded problem. This pins the author helper (marshallerManifest)
// and the registration mechanics the loader uses -- the whole marshaller (device target, GPU-config path and,
// via the produced handle, the scalar kind) travels inside the provider with no manifest ABI change.
TEST_CASE("Marshaller module manifest contributes a registrable GPU marshaller", "[flat][plugin][marshaller]") {
    // The author helper builds a one-contribution manifest tagged as the marshaller kind, carrying this host's
    // own toolchain fingerprint (so it would pass the loader's compat gate).
    const GenevaModuleManifest *manifest =
        marshallerManifest<ProbeGPUMarshaller, "cuda", "config/ProbeGPU.json">();
    REQUIRE(manifest != nullptr);
    CHECK(Gem::Geneva::moduleCompatMismatch(manifest->compat).empty());
    REQUIRE(manifest->contributions_count == 1);
    const GenevaContribution &contribution = manifest->contributions[0];
    CHECK(contribution.kind == GENEVA_CONTRIBUTION_MARSHALLER);
    CHECK(std::string(contribution.name_or_mnemonic) == "cuda");

    // The contribution thunk yields the marshaller provider across the plain-C void* boundary (the same
    // move-out the loader does), carrying the device target, the GPU-consumer config path, and -- via the
    // handle it hands out -- the device scalar kind.
    REQUIRE(contribution.make_factory != nullptr);
    void *raw = contribution.make_factory();
    REQUIRE(raw != nullptr);
    auto *holder = static_cast<GMarshallerProviderPtr *>(raw);
    GMarshallerProviderPtr provider = std::move(*holder);
    delete holder;
    REQUIRE(provider);
    CHECK(provider->getMnemonic() == "cuda");
    CHECK(provider->gpuConfigFile() == "config/ProbeGPU.json");
    CHECK(provider->provide()->scalarKind() == GPUScalarKind::Double);

    // It registers into marshallerProviderStore() exactly like a compiled-in registerGPUMarshaller, and the
    // one-per-target rule (setOnce) rejects a second marshaller for the same device target -- one problem per
    // process, so a module cannot shadow an existing marshaller.
    auto store = Gem::Geneva::marshallerProviderStore();
    store->remove("cuda"); // start from a clean slot (independent of any other test / registration)
    CHECK(store->setOnce("cuda", provider));

    const GenevaModuleManifest *other =
        marshallerManifest<ProbeGPUMarshaller, "cuda", "config/OtherGPU.json">();
    auto *holder2 = static_cast<GMarshallerProviderPtr *>(other->contributions[0].make_factory());
    GMarshallerProviderPtr const provider2 = std::move(*holder2);
    delete holder2;
    CHECK_FALSE(store->setOnce("cuda", provider2)); // collision: at most one marshaller per device target

    store->remove("cuda"); // leave the process-global store as we found it
}

TEST_CASE("Go2 enforces exactly one individual (optimization problem) per process", "[flat][plugin][go2]") {
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "geneva_claimonce_tests";
    fs::create_directories(base);

    int const argc = 1;
    char arg0[] = "unit-test";
    char *argv[] = {arg0, nullptr};
    Go2 go(argc, argv, base / "Go2.json");

    auto f1 = std::make_shared<GIndividualFactory<FactorySphere>>(base / "claimonce1.json");
    auto f2 = std::make_shared<GIndividualFactory<FactorySphere>>(base / "claimonce2.json");

    go.registerContentCreator(f1); // first claim succeeds
    REQUIRE(go.getContentCreator());
    // A second registration (here a second compiled-in creator; the same guard fires for a plugin load
    // when one is already provided) is refused: one individual type per process.
    CHECK_THROWS(go.registerContentCreator(f2));
}

/******************************************************************************/
// Two-phase Go2 configuration (D3/D4): the constructor only PARSES the command line / config; a lazy
// ensureConfigured_() -- fired by the first of optimize() / clientRun() / clientMode() -- then builds the
// consumer and resolves the algorithm chain. A setter called AFTER construction is therefore honoured, and a
// value given on the command line overrides a programmatic one. Regression guard for that precedence and for
// the new setter/getter symmetry (a defect here would silently ignore a main()-side configuration or let it
// win over an explicit command-line flag).
TEST_CASE("Go2 two-phase configuration: programmatic setters and CLI precedence", "[flat][go2][config]") {
    namespace fs = std::filesystem;
    namespace c2 = Gem::Courtier;
    const fs::path base = fs::temp_directory_path() / "geneva_twophase_tests";
    fs::create_directories(base);

    SECTION("setters and getters are symmetric (before configuration is finalized)") {
        int const argc = 1;
        char arg0[] = "unit-test";
        char *argv[] = {arg0, nullptr};
        Go2 go(argc, argv, base / "Go2.json");

        go.setConsumerName("beast");
        go.setModulePaths({"first.so", "second.so"});
        go.addModulePath("third.so");
        go.setAlgorithmChain({"ea", "sa"});

        CHECK(go.getConsumerName() == "beast");
        CHECK(go.getModulePaths() == std::vector<std::string>{"first.so", "second.so", "third.so"});
        CHECK(go.getAlgorithmChain() == std::vector<std::string>{"ea", "sa"});
    }

    // Configuration is finalized (consumer resolved, algorithm chain resolved, precedence applied) lazily at
    // the start of optimize()/clientRun(). optimize() runs ensureConfigured_() FIRST, then throws here
    // because no individual was registered -- so wrapping it in CHECK_THROWS is a cheap way to force
    // finalization and then inspect the effective (post-precedence) configuration, without running a full
    // optimization.
    SECTION("a --consumer on the command line overrides a programmatic setConsumerName") {
        int const argc = 3;
        char arg0[] = "unit-test";
        char arg1[] = "--consumer";
        char arg2[] = "stc";
        char *argv[] = {arg0, arg1, arg2, nullptr};
        Go2 go(argc, argv, base / "Go2.json");

        CHECK(go.getConsumerName() == "stc");   // bound from the command line at construction
        go.setConsumerName("beast");             // a later, programmatic attempt to change it
        CHECK(go.getConsumerName() == "beast");  // the member now holds the programmatic value...

        CHECK_THROWS(go.optimize());             // ...but finalizing configuration restores the CLI value
        CHECK(go.getConsumerName() == "stc");

        c2::GConsumerRegistryT<GOptimizableEntity>::instance().clear();
    }

    SECTION("a --optimizationAlgorithms list overrides a programmatic setAlgorithmChain") {
        int const argc = 3;
        char arg0[] = "unit-test";
        char arg1[] = "--optimizationAlgorithms";
        char arg2[] = "ea";
        char *argv[] = {arg0, arg1, arg2, nullptr};
        Go2 go(argc, argv, base / "Go2.json");

        go.setConsumerName("stc");
        go.setAlgorithmChain({"sa", "cgd"});     // a two-element programmatic chain, to be overridden

        CHECK_THROWS(go.optimize());             // finalizes: the CLI's single "ea" wins over the two above
        CHECK(go.getNAlgorithms() == 1);

        c2::GConsumerRegistryT<GOptimizableEntity>::instance().clear();
    }

    SECTION("with no command-line algorithms, the programmatic chain is resolved") {
        int const argc = 1;
        char arg0[] = "unit-test";
        char *argv[] = {arg0, nullptr};
        Go2 go(argc, argv, base / "Go2.json");

        go.setConsumerName("stc");
        go.setAlgorithmChain({"ea", "sa"});

        CHECK_THROWS(go.optimize());
        CHECK(go.getNAlgorithms() == 2);         // both programmatically-set algorithms were resolved

        c2::GConsumerRegistryT<GOptimizableEntity>::instance().clear();
    }

    SECTION("clientMode() finalizes only for a consumer whose role is unknown until runtime") {
        // stc takes its role from --client (known at construction), so clientMode() is a plain getter for it
        // and does NOT finalize configuration -- the programmatic chain is still unresolved afterwards. It is
        // resolved only once the run actually starts. (A role-at-runtime consumer such as mpi would instead
        // finalize here, so its rank-derived client/server role is settled before the caller dispatches; that
        // path needs a live MPI environment and is covered by the MPI examples.)
        int const argc = 1;
        char arg0[] = "unit-test";
        char *argv[] = {arg0, nullptr};
        Go2 go(argc, argv, base / "Go2.json");

        go.setConsumerName("stc");
        go.setAlgorithmChain({"ea", "sa"});

        (void) go.clientMode();
        CHECK(go.getNAlgorithms() == 0);         // stc: clientMode() did not finalize -> chain still unresolved

        CHECK_THROWS(go.optimize());             // ...the run start finalizes it
        CHECK(go.getNAlgorithms() == 2);

        c2::GConsumerRegistryT<GOptimizableEntity>::instance().clear();
    }
}

/******************************************************************************/

TEST_CASE("destroying a GenevaInitializer keeps the process RNG alive", "[go2][rng][regression]") {
    // Regression guard for the ctest hang. GenevaInitializer -- embedded in every Go2 as Go2::gi_ --
    // must NOT finalize the process-global Hap random-number factory on destruction. If it does, the
    // factory's producer threads are joined and its buffers are terminally closed, and the very next
    // random-number consumer spins forever in GRandomT::getNewRandomContainer() (a 100%-CPU livelock
    // that hung the whole GenevaStandardTests binary right after the "Go2 enforces exactly one
    // individual" case destroyed its Go2). We destroy a GenevaInitializer in a nested scope and then
    // exercise the RNG: constructing a GRandom already pulls a fresh container from the factory, and a
    // run of draws forces at least one buffer refill. This must COMPLETE -- not hang, not throw. It
    // fails on the unfixed code (the factory reports finalized(), so the draw throws) and passes once
    // ~GenevaInitializer() no longer finalizes the shared singleton (the factory is torn down only by
    // its own singleton destructor at process exit).
    { Gem::Geneva::GenevaInitializer const gi; } // came online here; must NOT tear the factory down at scope exit

    REQUIRE_FALSE(Gem::Hap::randomFactory()->finalized()); // the shared factory must still be live

    Gem::Hap::GRandom gr; // ctor pulls a fresh container -- would spin (unfixed) or throw (guarded) if dead
    std::uniform_real_distribution<double> u(0., 1.);
    double x = 0.;
    for(int i = 0; i < 4096; ++i) { // enough draws to exhaust a container and force a refill
        x = u(gr);
        CHECK(x >= 0.);
        CHECK(x < 1.);
    }
    CHECK(std::isfinite(x));
}

/******************************************************************************/
// Regression guard for the GOptimizableEntityFactory copy-constructor defect: it deep-cloned the
// POST-processor twice and never copied the PRE-processor, so any factory copy (GIndividualFactory
// ::clone(), the GOAFactoryT content-creator copy, the meta-optimizer's factory->clone()) silently
// dropped a registered pre-processor. This exercises the copy ctor directly and asserts both processors
// survive as independent deep clones. It fails on the unfixed code (copy.pre() is null) and passes once
// the ctor copies pre_processor_.
namespace Gem::Tests {

/** @brief A trivial, cloneable pre/post-processor used only to witness factory-copy behaviour. */
class ProbeProcessor : public Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity> {
public:
    ProbeProcessor() = default;

protected:
    /** @brief No-op: the test never runs the processor, it only checks it is carried across a copy. */
    bool process_([[maybe_unused]] GOptimizableEntity &p) override { return true; }

private:
    [[nodiscard]] Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity> *clone_() const override {
        return new ProbeProcessor(*this);
    }
};

/** @brief Exposes the protected pre-/post-processor slots so a factory copy is observable in a test. */
class ProbeFactory : public GIndividualFactory<FactorySphere> {
public:
    using GIndividualFactory<FactorySphere>::GIndividualFactory;

    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> pre() const {
        return this->pre_processor_;
    }
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> post() const {
        return this->post_processor_;
    }
};

} // namespace Gem::Tests

TEST_CASE("GOptimizableEntityFactory copy retains BOTH pre- and post-processors", "[flat][factory][regression]") {
    // No config file is read (we never call get_()), so a placeholder path is fine.
    Gem::Tests::ProbeFactory orig(
        std::filesystem::temp_directory_path() / "geneva_factory_proc_probe.json"
    );
    orig.registerPreProcessor(std::make_shared<Gem::Tests::ProbeProcessor>());
    orig.registerPostProcessor(std::make_shared<Gem::Tests::ProbeProcessor>());
    REQUIRE(orig.pre());
    REQUIRE(orig.post());

    // The buggy path: GOptimizableEntityFactory's copy ctor (reached via the derived copy ctor).
    Gem::Tests::ProbeFactory const copy(orig);

    CHECK(copy.pre());   // regressed to null on the unfixed code (pre_processor_ was never copied)
    CHECK(copy.post());
    // Deep-cloned, not aliased to the source's processors...
    CHECK(copy.pre().get() != orig.pre().get());
    CHECK(copy.post().get() != orig.post().get());
    // ...and the pre slot must not have been aliased to the (double-copied) post processor.
    CHECK(copy.pre().get() != copy.post().get());
}

/******************************************************************************/
// NOTE: the former "GGridArchitecture reads a TREE genome identically" case was removed when the
// tree hierarchy was deleted. The "[flat][architecture]" case above already proves the
// architecture is layout-agnostic by reading the genome purely through the §2 streamlineFP() seam.
