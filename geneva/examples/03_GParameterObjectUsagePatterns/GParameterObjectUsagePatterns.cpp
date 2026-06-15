/**
 * @file GParameterObjectUsagePatterns.cpp
 */

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

// Standard header files go here
#include <cstdint>
#include <iostream>
#include <tuple>
#include <vector>

// Boost header files go here
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"

using namespace Gem::Geneva;

/******************************************************************************/
/**
 * A minimal flat individual, used here only to demonstrate genome inspection /
 * adaption. It carries no extra members, so the CRTP base GFlatIndividualT
 * supplies clone_(); only a constructor (which authors the genome) and a trivial
 * fitnessCalculation() remain.
 */
class GDemoIndividual : public gpar::GFlatIndividualT<GDemoIndividual> {
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<gpar::GFlatIndividualT<GDemoIndividual>>(*this)
        );
    }

public:
    GDemoIndividual() {
        gpar::GGenomeBuilder b;
        // Two constrained doubles in [-10, 10] sharing one Gauss adaptor ...
        b.addDoubleGroup(2, -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.);
        // ... and one constrained int32 in [0, 5] with a flip adaptor.
        b.addInt32(0, 0, 5).flipAdaptor(1.);
        this->setGenome(b.build());
        this->randomInit(activityMode::ALLPARAMETERS);
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline(v);
        double result = 0.;
        for(double x : v) {
            result += x * x;
        }
        return result;
    }
};

BOOST_CLASS_EXPORT(GDemoIndividual) // NOLINT

/******************************************************************************/
/**
 * Prints the shape of a freshly built genome: the per-channel value arrays and
 * the number of adaption groups registered for each channel.
 */
void printGenome(const std::string &title, const gpar::Genome &g) {
    std::cout << title << '\n';
    std::cout << "  double values (" << g.dv.size() << "): ";
    for(double v : g.dv) {
        std::cout << v << ' ';
    }
    std::cout << "\n  float  values (" << g.fv.size() << "): ";
    for(float v : g.fv) {
        std::cout << v << ' ';
    }
    std::cout << "\n  int32  values (" << g.iv.size() << "): ";
    for(std::int32_t v : g.iv) {
        std::cout << v << ' ';
    }
    std::cout << "\n  bool   values (" << g.bv.size() << "): ";
    for(std::uint8_t v : g.bv) {
        std::cout << static_cast<int>(v) << ' ';
    }
    std::cout << "\n  adaption groups -> double: " << g.layout->d.groups.size()
              << ", float: " << g.layout->f.groups.size()
              << ", int32: " << g.layout->i.groups.size()
              << ", bool: " << g.layout->b.groups.size() << "\n\n";
}

/******************************************************************************/
/**
 * This example demonstrates how a flat genome is authored with the GGenomeBuilder
 * and how it is inspected. With the object-tree parameter hierarchy removed, all
 * parameters of an individual are declared once via the builder rather than by
 * pushing back individual parameter objects.
 */
int main() {
    //===========================================================================
    // 1) A complete individual's genome, dumped to a boost::property_tree.
    //    The factory produces an individual with a full flat genome from its JSON
    //    config; toPropertyTree() serialises that genome's parameters.

    {
        std::shared_ptr<gind::GFunctionIndividualFactory> gfi_ptr(
            new gind::GFunctionIndividualFactory("./config/GFunctionIndividual.json")
        );

        std::shared_ptr<gpar::GOptimizableEntity> gfi_test = gfi_ptr->get();

        // Make sure the individual is "clean", i.e. the processed flag is set
        gfi_test->set_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
        gfi_test->process();

        boost::property_tree::ptree ptr;
        gfi_test->toPropertyTree(ptr);

#if BOOST_VERSION > 105500
        boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#else
        boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#endif /* BOOST_VERSION */
        boost::property_tree::write_xml("result.xml", ptr, std::locale(), settings);

        // Now run this program and see the file "result.xml" for the output
    }

    //===========================================================================
    // 2) Authoring patterns with the GGenomeBuilder.
    //
    // Three group shapes exist per parameter type:
    //   addX(init[,min,max])      -- one parameter, its own adaption group
    //   addXGroup(n[,min,max])    -- n parameters sharing ONE adaption group (a collection)
    //   addXArray(n[,min,max])    -- n parameters, each its own adaption group
    // The fluent handle returned by each add* call attaches an adaptor and tunes
    // the init policy (.init / .perimeter / .adaptionMode).

    { // Constrained doubles: a single scalar, a shared-sigma group, and an array.
        gpar::GGenomeBuilder b;
        b.addDouble(1., -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.);
        b.addDoubleGroup(3, -5., 5.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.);
        b.addDoubleArray(2, -1., 1.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.);
        printGenome("Constrained doubles (single + group + array):", b.build());
    }

    { // Unbounded ("plain") double collection: min/max set the random-init perimeter
      // and the Gauss step range, but are NOT constraints (no folding takes place).
        gpar::GGenomeBuilder b;
        b.addDouble(0.5); // a bare unbounded scalar (init perimeter [0,1])
        b.addDoublePlainGroup(4, -10., 10.).gaussAdaptor(0.025, 0.1, 0., 1., 1.);
        printGenome("Unbounded (plain) doubles:", b.build());
    }

    { // Floats behave exactly like doubles, on their own value channel.
        gpar::GGenomeBuilder b;
        b.addFloat(0.f, -2.f, 2.f).gaussAdaptor(0.5f, 0.8f, 1e-3f, 2.f, 1.f);
        printGenome("Constrained floats:", b.build());
    }

    { // A bi-gaussian adaptor (two superimposed gaussians) on a constrained double.
        gpar::GGenomeBuilder b;
        b.addDouble(0., -10., 10.).biGaussAdaptor(
            0.5, 0.8, 1e-3, 2.,  // sigma1 + self-adaption + bounds
            0.5, 0.8, 1e-3, 2.,  // sigma2 + self-adaption + bounds
            0.5, 0.8, 0., 2.,    // delta + self-adaption + bounds
            1.                   // ad_prob
        );
        printGenome("Constrained double with a bi-gaussian adaptor:", b.build());
    }

    { // Integer parameters: an integer Gauss adaptor and a flip adaptor.
        gpar::GGenomeBuilder b;
        b.addInt32(0, -100, 100).intGaussAdaptor(2., 0.8, 1., 5., 1.);
        b.addInt32Group(3, 0, 9).flipAdaptor(1.);
        printGenome("Constrained int32 (gauss + flip):", b.build());
    }

    { // Boolean parameters: a single switch, a group, and an array, all flip-adapted.
        gpar::GGenomeBuilder b;
        b.addBool(true).flipAdaptor(1.);
        b.addBoolGroup(4).flipAdaptor(0.5);
        printGenome("Booleans (single + group):", b.build());
    }

    { // Per-group tuning: a fixed start value, a custom random-init perimeter, and
      // disabling adaption for a group via adaptionMode(NEVER) (an "inactive" group).
        gpar::GGenomeBuilder b;
        b.addDoubleGroup(2, -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.).init(3.);
        b.addDoubleGroup(2, -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.).perimeter(-1., 1.);
        b.addDoubleGroup(2, -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.)
            .adaptionMode(adaptionMode::NEVER); // frozen parameters
        printGenome("Per-group init / perimeter / frozen:", b.build());
    }

    //===========================================================================
    // 3) Inspecting and mutating a flat individual.

    {
        GDemoIndividual ind;

        std::vector<double> dvals;
        ind.streamline(dvals);
        std::cout << "GDemoIndividual double values: ";
        for(double v : dvals) {
            std::cout << v << ' ';
        }
        std::cout << '\n';

        std::vector<std::int32_t> ivals;
        ind.streamline(ivals);
        std::cout << "GDemoIndividual int32 values: ";
        for(std::int32_t v : ivals) {
            std::cout << v << ' ';
        }
        std::cout << '\n';

        std::vector<double> lower;
        std::vector<double> upper;
        ind.boundaries(lower, upper);
        std::cout << "Double boundaries:";
        for(std::size_t i = 0; i < lower.size(); ++i) {
            std::cout << " [" << lower[i] << ", " << upper[i] << "]";
        }
        std::cout << '\n';

        // Mutate the parameters. The adaption state + logic are OA-owned (Phase 10); a standalone
        // individual drives them via a self-owned scratch + config (StandaloneAdapter). We expect changes.
        Gem::Geneva::OptimizationAlgorithms::StandaloneAdapter(ind).adapt(ind);
        std::vector<double> after;
        ind.streamline(after);
        std::cout << "GDemoIndividual double values after adapt(): ";
        for(double v : after) {
            std::cout << v << ' ';
        }
        std::cout << '\n';

        std::cout << "GDemoIndividual as CSV: " << ind.toCSV() << '\n';
    }

    //===========================================================================

    return 0;
}
