/**
 * @file GConstrainedFPTTest.cpp
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

/**
 * This test takes a GConstrainedDoubleObject object:
 * a) It examines the mapping from internal to external representation of its value.
 * b) It tests the "distortion" of a gaussian when going through the mapping from
 *    internal to external value.
 *
 * Additional tests (including error handling) of the GConstrainedDoubleObject class have been
 * implemented as part of the unit tests.
 *
 * In order to see the results of this test, you need the Root toolkit from http://root.cern.ch.
 * Once installed call "root -l mapping.C" .
 */

// Standard header files go here
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GPlotDesigner.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "hap/GRandomT.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace boost;

const std::uint32_t NTESTS = 10000;

int main(int argc, char **argv) {
    //***************************************************************************
    // Test a: Mapping from internal to external value

    double internalValue = 0., externalValue = 0.;
    std::shared_ptr<GGraph2D> mapping_ptr(new GGraph2D());
    mapping_ptr->setPlotLabel("Mapping from internal to external value");

    gpar::GConstrainedDoubleObject gbd13(-1., 3.); // lower boundary -1, upper Boundary 3

    for(std::uint32_t i = 0; i < NTESTS; i++) {
        internalValue = -30. + 50. * static_cast<double>(i) / static_cast<double>(NTESTS);

        externalValue = gbd13.transfer(internalValue);
        *mapping_ptr &std::tuple<double, double>(internalValue, externalValue);
    }

    GPlotDesigner gpd("Manual tests of GConstrainedDoubleObject", 1, 1);

    gpd.setCanvasDimensions(1200, 1200);
    gpd.registerPlotter(mapping_ptr);

    gpd.writeToFile("mapping.C");

    //***************************************************************************
    // Test b: Distortian of gaussians when being translated from internal to
    // external value. The test produces 10 gaussian random number distributions,
    // whose mean is shifted from left to right of a [-1,1] range

    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;
    gpar::GConstrainedDoubleObject gbd_distortion(-1., 1.);

    std::ofstream distortion("distortion.C");

    distortion
        << "{" << '\n'
        << "  gROOT->SetStyle(\"Plain\");" << '\n'
        << "  gStyle -> SetOptStat(kFALSE);" << '\n'
        << '\n'
        << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);" << '\n'
        << "  cc->Divide(2,7);" << '\n'
        << '\n'
        << "  TH1F *external0 = new TH1F(\"external0\",\"external0\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external1 = new TH1F(\"external1\",\"external1\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external2 = new TH1F(\"external2\",\"external2\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external3 = new TH1F(\"external3\",\"external3\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external4 = new TH1F(\"external4\",\"external4\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external5 = new TH1F(\"external5\",\"external5\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external6 = new TH1F(\"external6\",\"external6\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external7 = new TH1F(\"external7\",\"external7\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external8 = new TH1F(\"external8\",\"external8\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external9 = new TH1F(\"external9\",\"external9\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external10 = new TH1F(\"external10\",\"external10\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external11 = new TH1F(\"external11\",\"external11\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external12 = new TH1F(\"external12\",\"external12\",301,-1.5,1.5);" << '\n'
        << "  TH1F *external13 = new TH1F(\"external13\",\"external13\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal0 = new TH1F(\"internal0\",\"internal0\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal1 = new TH1F(\"internal1\",\"internal1\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal2 = new TH1F(\"internal2\",\"internal2\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal3 = new TH1F(\"internal3\",\"internal3\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal4 = new TH1F(\"internal4\",\"internal4\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal5 = new TH1F(\"internal5\",\"internal5\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal6 = new TH1F(\"internal6\",\"internal6\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal7 = new TH1F(\"internal7\",\"internal7\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal8 = new TH1F(\"internal8\",\"internal8\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal9 = new TH1F(\"internal9\",\"internal9\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal10 = new TH1F(\"internal10\",\"internal10\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal11 = new TH1F(\"internal11\",\"internal11\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal12 = new TH1F(\"internal12\",\"internal12\",301,-1.5,1.5);" << '\n'
        << "  TH1F *internal13 = new TH1F(\"internal13\",\"internal13\",301,-1.5,1.5);" << '\n'
        << '\n';

    std::normal_distribution<double> normal_distribution(0., 0.1);
    for(std::size_t i = 0; i < NTESTS; i++) {
        internalValue = normal_distribution(gr);

        distortion << "  external0->Fill(" << gbd_distortion.transfer(internalValue - 1.1) << ");"
                   << '\n'
                   << "  external1->Fill(" << gbd_distortion.transfer(internalValue - 1.0) << ");"
                   << '\n'
                   << "  external2->Fill(" << gbd_distortion.transfer(internalValue - 0.9) << ");"
                   << '\n'
                   << "  external3->Fill(" << gbd_distortion.transfer(internalValue - 0.7) << ");"
                   << '\n'
                   << "  external4->Fill(" << gbd_distortion.transfer(internalValue - 0.5) << ");"
                   << '\n'
                   << "  external5->Fill(" << gbd_distortion.transfer(internalValue - 0.3) << ");"
                   << '\n'
                   << "  external6->Fill(" << gbd_distortion.transfer(internalValue - 0.1) << ");"
                   << '\n'
                   << "  external7->Fill(" << gbd_distortion.transfer(internalValue + 0.1) << ");"
                   << '\n'
                   << "  external8->Fill(" << gbd_distortion.transfer(internalValue + 0.3) << ");"
                   << '\n'
                   << "  external9->Fill(" << gbd_distortion.transfer(internalValue + 0.5) << ");"
                   << '\n'
                   << "  external10->Fill(" << gbd_distortion.transfer(internalValue + 0.7) << ");"
                   << '\n'
                   << "  external11->Fill(" << gbd_distortion.transfer(internalValue + 0.9) << ");"
                   << '\n'
                   << "  external12->Fill(" << gbd_distortion.transfer(internalValue + 1.0) << ");"
                   << '\n'
                   << "  external13->Fill(" << gbd_distortion.transfer(internalValue + 1.1) << ");"
                   << '\n'
                   << "  internal0->Fill(" << internalValue - 1.1 << ");" << '\n'
                   << "  internal1->Fill(" << internalValue - 1.0 << ");" << '\n'
                   << "  internal2->Fill(" << internalValue - 0.9 << ");" << '\n'
                   << "  internal3->Fill(" << internalValue - 0.7 << ");" << '\n'
                   << "  internal4->Fill(" << internalValue - 0.5 << ");" << '\n'
                   << "  internal5->Fill(" << internalValue - 0.3 << ");" << '\n'
                   << "  internal6->Fill(" << internalValue - 0.1 << ");" << '\n'
                   << "  internal7->Fill(" << internalValue + 0.1 << ");" << '\n'
                   << "  internal8->Fill(" << internalValue + 0.3 << ");" << '\n'
                   << "  internal9->Fill(" << internalValue + 0.5 << ");" << '\n'
                   << "  internal10->Fill(" << internalValue + 0.7 << ");" << '\n'
                   << "  internal11->Fill(" << internalValue + 0.9 << ");" << '\n'
                   << "  internal12->Fill(" << internalValue + 1.0 << ");" << '\n'
                   << "  internal13->Fill(" << internalValue + 1.1 << ");" << '\n';
    }

    distortion << '\n'
               << "  cc->cd(1);" << '\n'
               << "  external0->SetFillColor(4);" << '\n'
               << "  external0->SetFillStyle(1001);" << '\n'
               << "  external0->Draw();" << '\n'
               << "  internal0->SetFillColor(2);" << '\n'
               << "  internal0->SetFillStyle(3004);" << '\n'
               << "  internal0->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(2);" << '\n'
               << "  external1->SetFillColor(4);" << '\n'
               << "  external1->SetFillStyle(1001);" << '\n'
               << "  external1->Draw();" << '\n'
               << "  internal1->SetFillColor(2);" << '\n'
               << "  internal1->SetFillStyle(3004);" << '\n'
               << "  internal1->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(3);" << '\n'
               << "  external2->SetFillColor(4);" << '\n'
               << "  external2->SetFillStyle(1001);" << '\n'
               << "  external2->Draw();" << '\n'
               << "  internal2->SetFillColor(2);" << '\n'
               << "  internal2->SetFillStyle(3004);" << '\n'
               << "  internal2->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(4);" << '\n'
               << "  external3->SetFillColor(4);" << '\n'
               << "  external3->SetFillStyle(1001);" << '\n'
               << "  external3->Draw();" << '\n'
               << "  internal3->SetFillColor(2);" << '\n'
               << "  internal3->SetFillStyle(3004);" << '\n'
               << "  internal3->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(5);" << '\n'
               << "  external4->SetFillColor(4);" << '\n'
               << "  external4->SetFillStyle(1001);" << '\n'
               << "  external4->Draw();" << '\n'
               << "  internal4->SetFillColor(2);" << '\n'
               << "  internal4->SetFillStyle(3004);" << '\n'
               << "  internal4->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(6);" << '\n'
               << "  external5->SetFillColor(4);" << '\n'
               << "  external5->SetFillStyle(1001);" << '\n'
               << "  external5->Draw();" << '\n'
               << "  internal5->SetFillColor(2);" << '\n'
               << "  internal5->SetFillStyle(3004);" << '\n'
               << "  internal5->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(7);" << '\n'
               << "  external6->SetFillColor(4);" << '\n'
               << "  external6->SetFillStyle(1001);" << '\n'
               << "  external6->Draw();" << '\n'
               << "  internal6->SetFillColor(2);" << '\n'
               << "  internal6->SetFillStyle(3004);" << '\n'
               << "  internal6->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(8);" << '\n'
               << "  external7->SetFillColor(4);" << '\n'
               << "  external7->SetFillStyle(1001);" << '\n'
               << "  external7->Draw();" << '\n'
               << "  internal7->SetFillColor(2);" << '\n'
               << "  internal7->SetFillStyle(3004);" << '\n'
               << "  internal7->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(9);" << '\n'
               << "  external8->SetFillColor(4);" << '\n'
               << "  external8->SetFillStyle(1001);" << '\n'
               << "  external8->Draw();" << '\n'
               << "  internal8->SetFillColor(2);" << '\n'
               << "  internal8->SetFillStyle(3004);" << '\n'
               << "  internal8->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(10);" << '\n'
               << "  external9->SetFillColor(4);" << '\n'
               << "  external9->SetFillStyle(1001);" << '\n'
               << "  external9->Draw();" << '\n'
               << "  internal9->SetFillColor(2);" << '\n'
               << "  internal9->SetFillStyle(3004);" << '\n'
               << "  internal9->Draw(\"same\");" << '\n'
               << "  cc->cd();" << '\n'
               << '\n'
               << "  cc->cd(11);" << '\n'
               << "  external10->SetFillColor(4);" << '\n'
               << "  external10->SetFillStyle(1001);" << '\n'
               << "  external10->Draw();" << '\n'
               << "  internal10->SetFillColor(2);" << '\n'
               << "  internal10->SetFillStyle(3004);" << '\n'
               << "  internal10->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(12);" << '\n'
               << "  external11->SetFillColor(4);" << '\n'
               << "  external11->SetFillStyle(1001);" << '\n'
               << "  external11->Draw();" << '\n'
               << "  internal11->SetFillColor(2);" << '\n'
               << "  internal11->SetFillStyle(3004);" << '\n'
               << "  internal11->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(13);" << '\n'
               << "  external12->SetFillColor(4);" << '\n'
               << "  external12->SetFillStyle(1001);" << '\n'
               << "  external12->Draw();" << '\n'
               << "  internal12->SetFillColor(2);" << '\n'
               << "  internal12->SetFillStyle(3004);" << '\n'
               << "  internal12->Draw(\"same\");" << '\n'
               << '\n'
               << "  cc->cd(14);" << '\n'
               << "  external13->SetFillColor(4);" << '\n'
               << "  external13->SetFillStyle(1001);" << '\n'
               << "  external13->Draw();" << '\n'
               << "  internal13->SetFillColor(2);" << '\n'
               << "  internal13->SetFillStyle(3004);" << '\n'
               << "  internal13->Draw(\"same\");" << '\n'
               << "  cc->cd();" << '\n'
               << "}" << '\n';

    distortion.close();

    return 0;
}
