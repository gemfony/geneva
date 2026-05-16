/**
 * @file GBooleanProbabilityTest.cpp
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
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GBooleanObject.hpp"

using namespace Gem::Geneva;
using namespace boost;

/**
 * This test checks the flip probability of GBooleanObject and a GBooleanCollection. Likewise this is a test
 * for the GBooleanAdaptor class and the class's operator=; Tests include constant flip probability
 * as well as mutative adaption of the flip probability. Results can be viewed using the ROOT
 * analysis toolkit (see http://root.cern.ch).
 */

const std::size_t MAXFLIP = 10000;
const std::size_t NBIT = 10;

/************************************************************************************************/
/**
 * The main function
 */
int main(int argc, char **argv) {
    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Create test candidates
    GBooleanObject A(true), A_tmp;
    GBooleanCollection B(NBIT), B_tmp; // B is initialized with 100 random booleans

    std::shared_ptr<GBooleanAdaptor> A_adaptor(new GBooleanAdaptor(0.1));
    std::shared_ptr<GBooleanAdaptor> B_adaptor(new GBooleanAdaptor(0.2));

    A.addAdaptor(A_adaptor);
    B.addAdaptor(B_adaptor);

    std::ofstream ofs("bitflipResult.C"); // Output file

    ofs << "{" << '\n'
        << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,800);" << '\n'
        << "  cc->Divide(2,2);" << '\n'
        << '\n'
        << "  TH1F *singleFlipValueNPA = new "
           "TH1F(\"singleFlipValueNPA\",\"singleFlipValueNPA\",2,-0.5,1.5);"
        << '\n'
        << "  TH1F *collectionFlipValueNPA = new "
           "TH1F(\"collectionFlipValueNPA\",\"collectionFlipValueNPA\",2,-0.5,1.5);"
        << '\n'
        << "  TH1F *singleFlipValuePA = new "
           "TH1F(\"singleFlipValuePA\",\"singleFlipValuePA\",2,-0.5,1.5);"
        << '\n'
        << "  TH1F *collectionFlipValuePA = new "
           "TH1F(\"collectionFlipValuePA\",\"collectionFlipValuePA\",2,-0.5,1.5);"
        << '\n'
        << '\n';

    /////////////////////////////////////////////////////////////////////////////////////////
    // Tests without adaption of flip probability
    A_adaptor->setAdaptionThreshold(0);
    B_adaptor->setAdaptionThreshold(0);

    double A_noprobadapt_notflipped = 0., A_noprobadapt_flipped = 0.;
    double B_noprobadapt_notflipped = 0., B_noprobadapt_flipped = 0.;
    for(std::size_t i = 0; i < MAXFLIP; i++) {
        // GBooleanObject
        A_tmp = A;
        A.adapt(gr); // adapt
        if(A.value() == A_tmp.value()) {
            ofs << "  singleFlipValueNPA->Fill(0.);" << '\n'; // 0 means "not flipped"
            A_noprobadapt_notflipped += 1.;
        }
        else {
            ofs << "  singleFlipValueNPA->Fill(1.);" << '\n'; // 1 means "flipped"
            A_noprobadapt_flipped += 1.;
        }

        // GBooleanCollection
        B_tmp = B;
        B.adapt(gr);
        for(std::size_t j = 0; j < NBIT; j++) {
            if(B[j] == B_tmp[j]) {
                ofs << "  collectionFlipValueNPA->Fill(0.);" << '\n'; // 0 means "not flipped"
                B_noprobadapt_notflipped += 1.;
            }
            else {
                ofs << "  collectionFlipValueNPA->Fill(1.);" << '\n'; // 1 means "flipped"
                B_noprobadapt_flipped += 1.;
            }
        }
    }

    std::cout << "A flip ratio (no probability adaption): "
              << A_noprobadapt_flipped / double(MAXFLIP) << '\n'
              << "B flip ratio (no probability adaption): "
              << B_noprobadapt_flipped / double(MAXFLIP * NBIT) << '\n';

    /////////////////////////////////////////////////////////////////////////////////////////
    // Tests with adaption of flip probability
    A_adaptor->setAdaptionThreshold(10);
    B_adaptor->setAdaptionThreshold(1);

    // A_adaptor->setAdaptionParameters(0.1,0.01,0.00001,10); // This will result in a rather large adaption rate
    // B_adaptor->setAdaptionParameters(0.1,0.01,0.00001,10);

    A_adaptor->setAdaptionProbability(0.25);
    B_adaptor->setAdaptionProbability(0.5);

    double A_probadapt_notflipped = 0., A_probadapt_flipped = 0.;
    double B_probadapt_notflipped = 0., B_probadapt_flipped = 0.;
    for(std::size_t i = 0; i < MAXFLIP; i++) {
        // GBooleanObject
        A_tmp = A;
        A.adapt(gr); // adapt
        if(A.value() == A_tmp.value()) {
            ofs << "  singleFlipValuePA->Fill(0.);" << '\n'; // 0 means "not flipped"
            A_probadapt_notflipped += 1.;
        }
        else {
            ofs << "  singleFlipValuePA->Fill(1.);" << '\n'; // 1 means "flipped"
            A_probadapt_flipped += 1.;
        }

        // GBooleanCollection
        B_tmp = B;
        B.adapt(gr);
        for(std::size_t j = 0; j < NBIT; j++) {
            if(B[j] == B_tmp[j]) {
                ofs << "  collectionFlipValuePA->Fill(0.);" << '\n'; // 0 means "not flipped"
                B_probadapt_notflipped += 1.;
            }
            else {
                ofs << "  collectionFlipValuePA->Fill(1.);" << '\n'; // 1 means "flipped"
                B_probadapt_flipped += 1.;
            }
        }
    }

    std::cout << "A flip ratio (probability adaption): " << A_probadapt_flipped / double(MAXFLIP)
              << '\n'
              << "B flip ratio (probability adaption): "
              << B_probadapt_flipped / double(MAXFLIP * NBIT) << '\n';

    ofs << '\n'
        << "  cc->cd(1);" << '\n'
        << "  singleFlipValueNPA->Draw();" << '\n'
        << "  cc->cd(2);" << '\n'
        << "  collectionFlipValueNPA->Draw();" << '\n'
        << "  cc->cd(3);" << '\n'
        << "  singleFlipValuePA->Draw();" << '\n'
        << "  cc->cd(4);" << '\n'
        << "  collectionFlipValuePA->Draw();" << '\n'
        << "  cc->cd();" << '\n'
        << "}" << '\n';

    ofs.close();

    return 0;
}
