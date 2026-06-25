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
#include "courtier/consumers/GWebsocketConsumerT.hpp"
#include "courtier/transport/GWebsocketTransportT.hpp"
#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/transport/GAsioTransportT.hpp"
#include <atomic>
#include <thread>
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenomeLayout.hpp"
#include "geneva/ind/GGenomeLayoutSerialization.hpp" // ChannelLayout (de)serialisation (layout interning)
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualFactory.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
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
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
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
    static gen::GenomeData buildGenome(const Config &c) {
        GGenomeBuilder b;
        b.addDoubleGroup(c.par_dim, c.min, c.max); // structure only; the adaptor lives on the OA config
        return b.build();
    }

    /** @brief Optional hook: the OA-owned Gauss adaption config for a genome this individual produces.
     *  Exercised through GFlatIndividualFactory::getAdaptionConfig(). */
    static std::shared_ptr<oa::GAdaptionConfigBase>
    buildAdaptionConfig(const Gem::Geneva::Genome::GFlatGenome &sample, const Config &c) {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(sample);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
            cfg->groupDouble(i).gauss(c.sigma, 0.8, 1e-3, 2., 1.);
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
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
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

    for(std::size_t k = 0; k < 5; ++k) {
        CHECK(ch.lower[k] == -10.);
        CHECK(ch.upper[k] == 10.);
        CHECK(ch.fold[k]); // bounded
        CHECK(g.dv[k] == 1.0);
    }
}

/******************************************************************************/
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
    GGenomeLayout copy(*a);
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
TEST_CASE("Wire: an id-only layout the receiver lacks self-heals via fetch, not a fatal throw",
          "[flat][wire][layoutfetch]") {
    // Reproduces the live send-once unsoundness (minus threading): a sender's peer-tracking says the peer
    // already holds the layout, so it emits the layout id-only -- but the receiver's registry does NOT
    // hold it (peer-id reuse across non-shared caches). The receiver must self-heal by FETCHing the
    // layout (REQUEST_LAYOUT), NOT throw a fatal exception that tears the session down.
    using mode = Gem::Common::serializationMode;
    namespace c2 = Gem::Courtier;

    FlatSphere item(40);
    item.randomInit(activityMode::ALLPARAMETERS);
    const LayoutId lid = item.getLayout()->layoutId();
    const c2::GWireLayoutId wid{lid.hi, lid.lo};

    // The "server": holds the layout and (per possibly-unsound peer tracking) believes peer 7 has it,
    // so it emits an id-only reference.
    c2::GWireLayoutRegistry server_reg;
    server_reg.put(wid, layoutToWireBlob(*item.getLayout()));
    server_reg.markPeerHasLayout(7, wid);
    c2::GWireSerializationContext send_ctx;
    send_ctx.enabled = true; send_ctx.peer = 7; send_ctx.registry = &server_reg;
    std::string s;
    { c2::GWireSerializationScope scope(&send_ctx); s = item.toString(mode::BINARY); }

    // The receiver: an EMPTY registry (it never actually received the layout), but a REQUEST_LAYOUT fetch
    // is available (answered from the server's registry). The decode must RESOLVE, not throw.
    c2::GWireLayoutRegistry recv_reg;
    c2::GWireSerializationContext recv_ctx;
    recv_ctx.enabled = true; recv_ctx.registry = &recv_reg;
    recv_ctx.fetch_blob = [&](const c2::GWireLayoutId &id) -> std::string {
        std::string blob; server_reg.tryGet(id, blob); return blob;
    };
    FlatSphere received;
    CHECK_NOTHROW([&] {
        c2::GWireSerializationScope scope(&recv_ctx);
        received.fromString(s, mode::BINARY);
    }());
    CHECK(received.getLayout()->layoutId() == lid);
}

/******************************************************************************/
TEST_CASE("Wire: a RETURN carries a resolvable layout even when the receiver has no fetch",
          "[flat][wire][layoutreturn]") {
    // The send-once peer-tracking unsoundness only bites where the receiver cannot recover by fetching.
    // On the RETURN direction the receiving server cannot issue a REQUEST_LAYOUT back to a worker, so a
    // returned item must NEVER reference its layout id-only -- it must always carry the layout in full.
    // (Submit direction is unaffected: a worker CAN fetch from the server.)
    using mode = Gem::Common::serializationMode;
    namespace c2 = Gem::Courtier;

    FlatSphere item(40);
    item.randomInit(activityMode::ALLPARAMETERS);
    item.process();
    const LayoutId lid = item.getLayout()->layoutId();
    const c2::GWireLayoutId wid{lid.hi, lid.lo};

    // Worker side: RETURNING, registry holds the layout, and (possibly-unsound) peer-tracking marks the
    // server peer as already having it -- which under the OLD code makes save() emit the layout id-only.
    c2::GWireLayoutRegistry worker_reg;
    worker_reg.put(wid, layoutToWireBlob(*item.getLayout()));
    worker_reg.markPeerHasLayout(0, wid);
    c2::GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true; worker_ctx.peer = 0; worker_ctx.returning = true;
    worker_ctx.registry = &worker_reg;
    std::string s;
    { c2::GWireSerializationScope scope(&worker_ctx); s = item.toString(mode::BINARY); }

    // Server side: an EMPTY registry and NO fetch (it cannot pull a layout from a worker). The returned
    // item must still decode -- i.e. the worker must have shipped the layout in full on the return.
    c2::GWireLayoutRegistry server_reg;
    c2::GWireSerializationContext server_ctx;
    server_ctx.enabled = true; server_ctx.registry = &server_reg; // no fetch_blob
    FlatSphere received;
    CHECK_NOTHROW([&] {
        c2::GWireSerializationScope scope(&server_ctx);
        received.fromString(s, mode::BINARY);
    }());
    CHECK(received.getLayout()->layoutId() == lid);
}

/******************************************************************************/
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
        FlatSphere ind; ind.setGenome(b.build()); roundtrip(ind.getLayout());
    }
    SECTION("double groups of 10 (the image case)") {
        GGenomeBuilder b; for(int t = 0; t < 100; ++t) { b.addDoubleGroup(10, -1., 1.); }
        FlatSphere ind; ind.setGenome(b.build()); roundtrip(ind.getLayout());
    }
    SECTION("the existing mixed layout") {
        GGenomeBuilder b;
        b.addDoubleGroup(4, -10., 10.); b.addDoubleArray(3, -2., 2.);
        b.addInt32Group(2, -5, 5); b.addBoolGroup(2);
        FlatSphere ind; ind.setGenome(b.build()); roundtrip(ind.getLayout());
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
    FlatSphere ind;
    ind.setGenome(b.build());
    const LayoutId before = ind.getLayout()->layoutId();

    FlatSphere restored;
    restored.fromString(ind.toString(Gem::Common::serializationMode::BINARY),
                        Gem::Common::serializationMode::BINARY);
    CHECK(restored.getLayout()->layoutId() == before);
    CHECK(restored.getLayout()->sameStructure(*ind.getLayout()));
}

/******************************************************************************/
TEST_CASE("GFlatGenome::streamlineInto matches streamline (bulk-flatten fast path)", "[flat]") {
    // streamlineInto() is the GPU marshallers' bulk-flatten fast path: it must produce EXACTLY the same
    // external (range-folded) values as streamline<T>(), just written straight into a caller buffer with
    // no temporary vector. FlatSphere's group is Constrained, so the per-element fold is exercised.
    FlatSphere ind(7);
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
    gen::GenomeData g = b.build();

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
    gen::GenomeData g = b.build();

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
TEST_CASE("GFlatGenome: layout interning round-trips losslessly (compact + escape route)", "[flat]") {
    // COMPACT PATH: a genome whose channels are all group-uniform (the normal case) must round-trip with
    // its per-value layout arrays reconstructed EXACTLY from the compact per-group wire form.
    GGenomeBuilder b;
    b.addDoubleGroup(4, -10., 10.); // one uniform double group of 4
    b.addDoubleArray(3, -2., 2.);   // 3 single-value double groups
    b.addInt32Group(2, -5, 5);      // one int group
    b.addBoolGroup(2);              // one bool group
    FlatSphere ind;
    ind.setGenome(b.build());
    auto L = ind.getLayout();
    REQUIRE(L);

    FlatSphere restored;
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

    // FROZEN parameter: lo == hi is a degenerate/empty range; any value folds to that single point
    // (no division by a zero range). Mirrors a user freezing a parameter by equal bounds.
    CHECK(foldConstrainedFP<double>(4., 4., 4.) == 4.);
    CHECK(foldConstrainedFP<double>(-7.3, 4., 4.) == 4.);
    CHECK(foldConstrainedFP<double>(100., 4., 4.) == 4.);
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

    // The OA-owned config drives both the sigma readout and the stall-reset. It is the
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
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
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
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FlatIntGauss>>(*this)
        );
    }
};

/**
 * A flat individual with MANY single-value double groups (an addDoubleArray). Its layout is O(groups),
 * so it makes the transport layout send-once visible: the full layout is sizeable, but every item after
 * the first to a peer carries only a 16-byte layout id.
 */
class FlatManyGroups : public GFlatIndividualT<FlatManyGroups> {
public:
    FlatManyGroups() { build(64); }
    explicit FlatManyGroups(std::size_t n) { build(n); }
    FlatManyGroups(const FlatManyGroups &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) { s += x * x; }
        return s;
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
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FlatManyGroups>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::FlatMixed)       // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::FlatIntGauss)    // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::FlatManyGroups)  // NOLINT

using Gem::Tests::FlatManyGroups;
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
TEST_CASE("GFlatIndividualFactory::getAdaptionConfig delegates to buildAdaptionConfig", "[flat][factory]") {
    // The factory's getAdaptionConfig() forwards to the individual's optional buildAdaptionConfig hook
    // and returns the OA-owned config matching the produced genome (used with registerAdaptionConfig).
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() / "geneva_flat_factory_tests";
    fs::create_directories(base);
    const fs::path cfg = base / "FactorySphereAdaption.json";

    GFlatIndividualFactory<FactorySphere> f(cfg);
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
// Wire transport: the layout send-once wire form. These exercise GFlatGenome::save()/load() under an
// active Gem::Courtier::GWireSerializationScope, simulating the server->worker wire path WITHOUT any real
// transport: a server-side registry interns each distinct layout, the first item to a peer carries the
// full layout and every later item only its 16-byte id, and a worker resolves an id-only item from its
// local cache or (on a miss) via a fetch callback.

namespace {
using Gem::Courtier::GWireLayoutId;
using Gem::Courtier::GWireLayoutRegistry;
using Gem::Courtier::GWireSerializationContext;
using Gem::Courtier::GWireSerializationScope;

GWireLayoutId widOf(const FlatManyGroups &ind) {
    const auto lid = ind.getLayout()->layoutId();
    return GWireLayoutId{lid.hi, lid.lo};
}
std::vector<double> valuesOf(FlatManyGroups &ind) {
    std::vector<double> v;
    ind.streamline<double>(v);
    return v;
}
} // namespace

/******************************************************************************/
TEST_CASE("Wire send-once: first item carries the layout, later items only the id", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    FlatManyGroups a(64);
    FlatManyGroups c(64);
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
        GWireSerializationScope scope(&server_ctx);
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

    FlatManyGroups ra;
    FlatManyGroups rc;
    {
        GWireSerializationScope scope(&worker_ctx);
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

    FlatManyGroups a(48);
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
        GWireSerializationScope scope(&server_ctx);
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
    worker_ctx.fetch_blob = [&](const GWireLayoutId &id) -> std::string {
        ++fetch_calls;
        std::string blob;
        server_reg.tryGet(id, blob);
        return blob;
    };

    FlatManyGroups r;
    {
        GWireSerializationScope scope(&worker_ctx);
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

    FlatManyGroups a(16);
    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.peer = 1;
    server_ctx.registry = &server_reg;
    std::string s_idonly;
    {
        GWireSerializationScope scope(&server_ctx);
        (void)a.toString(mode::BINARY);
        s_idonly = a.toString(mode::BINARY);
    }

    // Empty cache, no fetch callback -> the miss cannot be resolved and load() must throw rather than
    // silently produce a wrong/empty layout.
    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    FlatManyGroups r;
    {
        GWireSerializationScope scope(&worker_ctx);
        CHECK_THROWS(r.fromString(s_idonly, mode::BINARY));
    }
}

/******************************************************************************/
TEST_CASE("Wire send-once: default-off encoding is self-contained and interoperable", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    FlatManyGroups a(32);
    a.randomInit(activityMode::ALLPARAMETERS);
    const std::vector<double> a_vals = valuesOf(a);

    // No active scope -> the self-contained full-layout form (the checkpoint / file path).
    const std::string s_full = a.toString(mode::BINARY);

    FlatManyGroups r;
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
    FlatManyGroups r2;
    {
        GWireSerializationScope scope(&worker_ctx);
        r2.fromString(s_full, mode::BINARY);
    }
    CHECK(r2.getLayout()->sameStructure(*a.getLayout()));
    CHECK(valuesOf(r2) == a_vals);
    CHECK(worker_reg.size() == 0); // a full-form stream does not populate the cache by id
}

/******************************************************************************/
// (removed) "Wire results-only return: genome omitted, grafted from the original" -- the results-only
// return form was dropped; a processed worker now always returns the whole genome (no graft). The full
// round-trip is covered by the wire send-once and net-loopback tests.

/******************************************************************************/
TEST_CASE("Wire return: a worker's modified genome travels back in full", "[flat][wire]") {
    using mode = Gem::Common::serializationMode;

    // A network-tiered client modifies the individual (here: re-initialises it) and returns it. A return
    // always carries the whole genome (there is no results-only form), so the modified parameters travel.
    FlatManyGroups submitted(24);
    submitted.randomInit(activityMode::ALLPARAMETERS);

    auto worker_copy = submitted.clone<FlatManyGroups>();
    worker_copy->randomInit(activityMode::ALLPARAMETERS); // a "better" individual the worker found
    worker_copy->process();
    const std::vector<double> worker_vals = valuesOf(*worker_copy);

    GWireLayoutRegistry worker_reg;
    GWireSerializationContext worker_ctx;
    worker_ctx.enabled = true;
    worker_ctx.registry = &worker_reg;
    worker_ctx.returning = true;
    std::string s;
    {
        GWireSerializationScope scope(&worker_ctx);
        s = worker_copy->toString(mode::BINARY);
    }

    GWireLayoutRegistry server_reg;
    GWireSerializationContext server_ctx;
    server_ctx.enabled = true;
    server_ctx.registry = &server_reg;
    FlatManyGroups received;
    {
        GWireSerializationScope scope(&server_ctx);
        received.fromString(s, mode::BINARY);
    }
    // The full return carried the (modified) genome -- no graft needed.
    CHECK(received.countParameters<double>() == 24);
    CHECK(valuesOf(received) == worker_vals); // the worker's modified parameters travelled back
}

/******************************************************************************/
TEST_CASE("Wire send-once: large-genome wire-size before/after", "[flat][wire]") {
    // Quantifies the headline win on a large structured genome (2000 single-value groups -> an O(2000)
    // layout). Reports and guards the per-item wire size for the submit direction (full layout vs id-only).
    using mode = Gem::Common::serializationMode;

    FlatManyGroups big(2000);
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
        GWireSerializationScope scope(&server_ctx);
        first_submit = big.toString(mode::BINARY).size();   // present=true (carries the layout)
        idonly_submit = big.toString(mode::BINARY).size();  // id-only
    }

    WARN("Wire size (2000-group genome, binary bytes):"
         << "\n  submit  full=" << full_submit << "  first=" << first_submit
         << "  id-only=" << idonly_submit
         << "  (id-only is " << (100 * idonly_submit / full_submit) << "% of full)");

    // The id-only submit drops the whole O(2000) layout, keeping only the values + a 16-byte id.
    CHECK(idonly_submit < full_submit);
    CHECK(first_submit >= full_submit); // the first send still carries the layout (plus the id framing)
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
        auto ind = std::make_unique<FlatManyGroups>(40); // all share one layout structure -> one id
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
        auto ind = std::make_unique<FlatManyGroups>(40);
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
        auto ind = std::make_unique<FlatManyGroups>(sizes[i % sizes.size()]);
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
    FlatSphere proto(8);
    for(std::size_t i = 0; i < 40; ++i) {
        pop->push_back(proto.clone_unique());
    }
    pop->setAdaptionConfig(proto.getAdaptionConfig());
    c2::GConsumerRegistryT<GOptimizableEntity>::instance().setConsumer(consumer);

    pop->optimize();
    auto best = pop->getBestGlobalIndividual<FlatSphere>();

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
// NOTE: the former "GGridArchitecture reads a TREE genome identically" case was removed when the
// tree hierarchy was deleted. The "[flat][architecture]" case above already proves the
// architecture is layout-agnostic by reading the genome purely through the §2 streamlineFP() seam.
