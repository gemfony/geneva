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

#include <cstdint>
#include <memory>
#include <vector>

#include "geneva/par/GBooleanObject.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleObject.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/par/GFlatParameterSet.hpp"
#include "geneva/par/GFlatParameters.hpp"
#include "geneva/par/GInt32Object.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "hap/GRandomT.hpp"

namespace gpar = Gem::Geneva::Parameters;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace {

/******************************************************************************/
/**
 * A minimal concrete individual with a mix of parameter types (plain + constrained
 * doubles, an int, two bools, and a double collection). Only fitnessCalculation()
 * and clone_() are pure on GParameterSet, so this is the whole boilerplate.
 */
class MixedIndividual : public gpar::GParameterSet {
public:
    MixedIndividual() {
        this->push_back(std::make_shared<gpar::GConstrainedDoubleObject>(1.5, -10., 10.));
        this->push_back(std::make_shared<gpar::GConstrainedDoubleObject>(-3.25, -10., 10.));
        this->push_back(std::make_shared<gpar::GDoubleObject>(2.0));
        this->push_back(std::make_shared<gpar::GInt32Object>(7));
        this->push_back(std::make_shared<gpar::GBooleanObject>(true));
        this->push_back(std::make_shared<gpar::GBooleanObject>(false));
        this->push_back(std::make_shared<gpar::GDoubleCollection>(4, -5., 5.));
    }
    MixedIndividual(const MixedIndividual &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }

private:
    gpar::GParameterSet *clone_() const override {
        return new MixedIndividual(*this);
    }
};

/** @brief An empty concrete individual, used as a host for a single flat node. */
class EmptyIndividual : public gpar::GParameterSet {
public:
    EmptyIndividual() = default;
    EmptyIndividual(const EmptyIndividual &) = default;

protected:
    double fitnessCalculation() override {
        return 0.;
    }

private:
    gpar::GParameterSet *clone_() const override {
        return new EmptyIndividual(*this);
    }
};

std::vector<std::uint8_t> asBytes(const std::vector<bool> &v) {
    return {v.begin(), v.end()};
}

/** @brief A flat-genome individual: 5 constrained doubles in [-5, 5), sphere objective. */
class FlatSphereIndividual : public gpar::GFlatParameterSet {
public:
    FlatSphereIndividual() {
        for(int i = 0; i < 5; ++i) {
            this->push_back(std::make_shared<gpar::GConstrainedDoubleObject>(3.0, -5., 5.));
        }
        this->compileToFlat(); // collapse the tree into a single flat node
    }
    FlatSphereIndividual(const FlatSphereIndividual &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }

private:
    gpar::GParameterSet *clone_() const override {
        return new FlatSphereIndividual(*this);
    }
};

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE("GFlatParameters reproduces a tree individual's values", "[flat]") {
    MixedIndividual src;

    std::vector<double> src_d;
    std::vector<float> src_f;
    std::vector<std::int32_t> src_i;
    std::vector<bool> src_b;
    src.streamline<double>(src_d);
    src.streamline<float>(src_f);
    src.streamline<std::int32_t>(src_i);
    src.streamline<bool>(src_b);

    auto flat = gpar::GFlatParameters::compileFrom(src);

    SECTION("flat node holds the same values as the source") {
        CHECK(flat->doubleValues() == src_d);
        CHECK(flat->floatValues() == src_f);
        CHECK(flat->int32Values() == src_i);
        CHECK(flat->boolValues() == asBytes(src_b));
    }

    SECTION("streamline through the GParameterSet interface round-trips") {
        // A host individual holding only the single flat node must streamline
        // exactly like the original tree individual -- this is what makes the
        // flat backend transparent to the optimization-algorithm stack.
        EmptyIndividual host;
        host.push_back(std::shared_ptr<gpar::GFlatParameters>(std::move(flat)));

        std::vector<double> host_d;
        std::vector<float> host_f;
        std::vector<std::int32_t> host_i;
        std::vector<bool> host_b;
        host.streamline<double>(host_d);
        host.streamline<float>(host_f);
        host.streamline<std::int32_t>(host_i);
        host.streamline<bool>(host_b);

        CHECK(host_d == src_d);
        CHECK(host_f == src_f);
        CHECK(host_i == src_i);
        CHECK(host_b == src_b);
    }
}

/******************************************************************************/

TEST_CASE("GFlatParameters reproduces boundaries and counts", "[flat]") {
    MixedIndividual src;

    std::vector<double> lo_src;
    std::vector<double> up_src;
    src.boundaries<double>(lo_src, up_src);

    auto flat = gpar::GFlatParameters::compileFrom(src);

    EmptyIndividual host;
    host.push_back(std::shared_ptr<gpar::GFlatParameters>(std::move(flat)));

    std::vector<double> lo_host;
    std::vector<double> up_host;
    host.boundaries<double>(lo_host, up_host);

    CHECK(lo_host == lo_src);
    CHECK(up_host == up_src);
    CHECK(host.countParameters<double>(Gem::Geneva::activityMode::ALLPARAMETERS) ==
          src.countParameters<double>(Gem::Geneva::activityMode::ALLPARAMETERS));
    CHECK(host.countParameters<std::int32_t>(Gem::Geneva::activityMode::ALLPARAMETERS) ==
          src.countParameters<std::int32_t>(Gem::Geneva::activityMode::ALLPARAMETERS));
    CHECK(host.countParameters<bool>(Gem::Geneva::activityMode::ALLPARAMETERS) ==
          src.countParameters<bool>(Gem::Geneva::activityMode::ALLPARAMETERS));
}

/******************************************************************************/

TEST_CASE("GFlatParameters assignValueVector writes values back", "[flat]") {
    MixedIndividual src;
    auto flat = gpar::GFlatParameters::compileFrom(src);

    EmptyIndividual host;
    host.push_back(std::shared_ptr<gpar::GFlatParameters>(std::move(flat)));

    std::vector<double> d;
    host.streamline<double>(d);
    for(double &x : d) {
        x += 0.5; // perturb
    }
    host.assignValueVector<double>(d);

    std::vector<double> d_back;
    host.streamline<double>(d_back);
    CHECK(d_back == d);
}

/******************************************************************************/

TEST_CASE("GFlatParameters adapt mutates parameters and respects constraints", "[flat]") {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    MixedIndividual src;
    auto flat = gpar::GFlatParameters::compileFrom(src);

    // Streamline order mirrors the container: [constrained, constrained, plain double,
    // collection(4)]. So dv_[0] and dv_[1] are the constrained doubles (bounds [-10, 10)),
    // dv_[2..] are plain doubles.
    const std::vector<double> before = flat->doubleValues();
    REQUIRE(before.size() >= 3);

    for(int i = 0; i < 50; ++i) {
        flat->adapt(gr);
    }
    const std::vector<double> after = flat->doubleValues();

    SECTION("plain doubles change") {
        bool plain_changed = false;
        for(std::size_t k = 2; k < after.size(); ++k) {
            if(after[k] != before[k]) {
                plain_changed = true;
            }
        }
        CHECK(plain_changed);
    }

    SECTION("constrained doubles are mutated and stay within their bounds") {
        CHECK(((after[0] != before[0]) || (after[1] != before[1])));
        CHECK(after[0] >= -10.0);
        CHECK(after[0] < 10.0);
        CHECK(after[1] >= -10.0);
        CHECK(after[1] < 10.0);
    }
}

/******************************************************************************/

TEST_CASE("GFlatParameters constrained fold matches GConstrainedFPT::transfer", "[flat]") {
    // A single constrained double in [-3, 5).
    class ConstrainedIndividual : public gpar::GParameterSet {
    public:
        ConstrainedIndividual() {
            this->push_back(std::make_shared<gpar::GConstrainedDoubleObject>(0.0, -3., 5.));
        }
        ConstrainedIndividual(const ConstrainedIndividual &) = default;

    protected:
        double fitnessCalculation() override {
            return 0.;
        }

    private:
        gpar::GParameterSet *clone_() const override {
            return new ConstrainedIndividual(*this);
        }
    };

    gpar::GConstrainedDoubleObject reference(0.0, -3., 5.); // its transfer() is the ground truth

    ConstrainedIndividual src;
    auto flat = gpar::GFlatParameters::compileFrom(src);
    EmptyIndividual host;
    host.push_back(std::shared_ptr<gpar::GFlatParameters>(std::move(flat)));

    // For a spread of in- and out-of-range values, the flat node's fold (applied on assignment)
    // must reproduce the original transfer() bit-for-bit.
    for(double v : {-3.0, -2.9, 0.0, 4.999, 7.3, 13.2, -8.4, -20.1, 100.5, 1000.25}) {
        const double expected = reference.transfer(v);

        std::vector<double> in{v};
        host.assignValueVector<double>(in);
        std::vector<double> out;
        host.streamline<double>(out);

        REQUIRE(out.size() == 1);
        CHECK(out[0] == expected);
        CHECK(out[0] >= -3.0);
        CHECK(out[0] < 5.0);
    }
}

/******************************************************************************/

TEST_CASE("GFlatParameters constrained integer fold matches GConstrainedIntT::transfer", "[flat]") {
    // A single constrained int in [-3, 5] (inclusive).
    class ConstrainedIntIndividual : public gpar::GParameterSet {
    public:
        ConstrainedIntIndividual() {
            this->push_back(std::make_shared<gpar::GConstrainedInt32Object>(0, -3, 5));
        }
        ConstrainedIntIndividual(const ConstrainedIntIndividual &) = default;

    protected:
        double fitnessCalculation() override {
            return 0.;
        }

    private:
        gpar::GParameterSet *clone_() const override {
            return new ConstrainedIntIndividual(*this);
        }
    };

    gpar::GConstrainedInt32Object reference(0, -3, 5); // ground-truth transfer()

    ConstrainedIntIndividual src;
    auto flat = gpar::GFlatParameters::compileFrom(src);
    EmptyIndividual host;
    host.push_back(std::shared_ptr<gpar::GFlatParameters>(std::move(flat)));

    for(std::int32_t v : {-3, 0, 5, 6, 13, -8, -20, 100, 47, -47}) {
        const std::int32_t expected = reference.transfer(v);

        std::vector<std::int32_t> in{v};
        host.assignValueVector<std::int32_t>(in);
        std::vector<std::int32_t> out;
        host.streamline<std::int32_t>(out);

        REQUIRE(out.size() == 1);
        CHECK(out[0] == expected);
        CHECK(out[0] >= -3);
        CHECK(out[0] <= 5);
    }
}

/******************************************************************************/

TEST_CASE("GFlatParameters flip adaption mutates int and bool parameters", "[flat]") {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    MixedIndividual src; // one GInt32Object, two GBooleanObject (flip adaptors)
    auto flat = gpar::GFlatParameters::compileFrom(src);

    const std::vector<std::int32_t> i_before = flat->int32Values();
    const std::vector<std::uint8_t> b_before = flat->boolValues();
    REQUIRE(not i_before.empty());
    REQUIRE(not b_before.empty());

    // Track whether the values ever deviate from their start: an int doing a +-1 random walk (and
    // bools toggling) can coincidentally return to the start by the final iteration, so checking a
    // deviation at *any* point during adaption is the robust signal that the flip kernels fire.
    bool int_changed = false;
    bool bool_changed = false;
    for(int k = 0; k < 50; ++k) {
        flat->adapt(gr);
        if(flat->int32Values() != i_before) {
            int_changed = true;
        }
        if(flat->boolValues() != b_before) {
            bool_changed = true;
        }
    }
    CHECK(int_changed);
    CHECK(bool_changed);
}

/******************************************************************************/

TEST_CASE("GFlatParameters adapt does not affect an independent clone", "[flat]") {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    MixedIndividual src;
    auto flat = gpar::GFlatParameters::compileFrom(src);
    const std::vector<double> snapshot = flat->doubleValues();

    std::unique_ptr<gpar::GFlatParameters> clone = flat->clone_unique<gpar::GFlatParameters>();

    for(int i = 0; i < 50; ++i) {
        flat->adapt(gr);
    }

    // The clone has its own value arrays and its own (deep-cloned) adaptors, so adapting
    // the original must not touch it.
    CHECK(clone->doubleValues() == snapshot);
}

/******************************************************************************/

TEST_CASE("GFlatParameters clones independently and compares equal", "[flat]") {
    MixedIndividual src;
    auto flat = gpar::GFlatParameters::compileFrom(src);

    std::unique_ptr<gpar::GFlatParameters> clone = flat->clone_unique<gpar::GFlatParameters>();

    CHECK(clone->doubleValues() == flat->doubleValues());
    CHECK(clone->int32Values() == flat->int32Values());
    CHECK(clone->boolValues() == flat->boolValues());

    // Mutating the clone must not touch the original (independent value storage).
    std::vector<double> probe = flat->doubleValues();
    std::size_t pos = 0;
    std::vector<double> bumped = probe;
    for(double &x : bumped) {
        x += 1.0;
    }
    clone->assignValueVector<double>(bumped, pos, Gem::Geneva::activityMode::ALLPARAMETERS);
    CHECK(flat->doubleValues() == probe);
    CHECK(clone->doubleValues() == bumped);
}

/******************************************************************************/

TEST_CASE("GFlatParameterSet drives an end-to-end evolutionary algorithm", "[flat][ea]") {
    // A small serial EA over a flat-genome individual. This exercises the whole stack on the flat
    // node: population cloning, adaption, evaluation, selection -- and verifies it converges while
    // respecting the parameter constraints.
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(1000); // suppress per-iteration console reporting

    for(std::size_t i = 0; i < 18; ++i) {
        FlatSphereIndividual seed;
        pop->push_back(seed.clone_unique());
    }

    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereIndividual>();
    REQUIRE(best);

    std::vector<double> v;
    best->streamline<double>(v);
    REQUIRE(v.size() == 5);

    double sphere = 0.;
    for(double x : v) {
        sphere += x * x;
        // constraints must hold end-to-end
        CHECK(x >= -5.0);
        CHECK(x < 5.0);
    }

    // The population starts at f = 5 * 3^2 = 45; a working EA drives it far below that.
    CHECK(sphere < 10.0); // robust end-to-end check (start = 45); not a convergence benchmark
}
