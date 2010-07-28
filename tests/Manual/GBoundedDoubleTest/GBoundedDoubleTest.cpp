/**
 * @file GBoundedDoubleTest.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

/**
 * This test takes a GBoundedDouble object:
 * a) It examines the mapping from internal to external representation of its value.
 * b) It tests the "distortion" of a gaussian when going through the mapping from
 *    internal to external value.
 *
 * Additional tests (including error handling) of the GBoundedDouble class have been
 * implemented as part of the unit tests.
 *
 * In order to see the results of this test, you need the Root toolkit from http://root.cern.ch.
 * Once installed call "root -l mapping.C" .
 */

// Standard header files go here
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

// Boost header files go here
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "hap/GRandomT.hpp"
#include "geneva/GBoundedDouble.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

using namespace Gem::Geneva;
using namespace boost;

const boost::uint32_t NTESTS=10000;

int main(int argc, char **argv){
	//***************************************************************************
	// Test a: Mapping from internal to external value

	GBoundedDouble gbd13(-1.,3.); // lower boundary -1, upper Boundary 3
	GBoundedDouble gbd052(0.5,2.); // lower boundary 0.5, upper Boundary 2

	double internalValue = 0., externalValue = 0.;

	std::ofstream mapping("mapping.C");

	// Essentially the following will create a C++ program as input for the
	// Root interpreter. It will then display the results of this test.
	mapping << "{" << std::endl
	        << "  gROOT->Reset();" << std::endl
	        << "  gStyle->SetOptTitle(0);" << std::endl
	        << std::endl
		    << "  double x13[" << NTESTS << "], y13[" << NTESTS << "];" << std::endl
		    << "  double x13adapt[" << NTESTS << "], y13adapt[" << NTESTS << "];" << std::endl
		    << "  double x052[" << NTESTS << "], y052[" << NTESTS << "];" << std::endl
		    << std::endl;

	for(boost::uint32_t i=0; i<NTESTS; i++){
		internalValue=-10.+20.*double(i)/double(NTESTS);

		externalValue = gbd13.calculateExternalValue(internalValue);
		mapping << "  x13[" << i << "] = " << internalValue << ";" << std::endl
		        << "  y13[" << i << "] = " << externalValue << ";" << std::endl;
	}

	for(boost::uint32_t i=0; i<NTESTS; i++){
		internalValue=-10.+20.*double(i)/double(NTESTS);

		externalValue = gbd052.calculateExternalValue(internalValue);
		mapping << "  x052[" << i << "] = " << internalValue << ";" << std::endl
		        << "  y052[" << i << "] = " << externalValue << ";" << std::endl;
	}

	// Set up and register an adaptor for gbd13, so it
	// knows how to be adaptd. We want a sigma of 0.5, sigma-adaption of 0.05 and
	// a minimum sigma of 0.02. The adaptor will be deleted automatically by the
	// GBoundedDouble.
	boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.5,0.05,0.02,2.));
	gbd13.addAdaptor(gdga);

	gbd13 = 0.; // We can assign a value inside of the allowed value range
	for(boost::uint32_t i=0; i<NTESTS; i++){
		// adapt the value and have a look at the
		// internal and external values.
		gbd13.adapt();

		mapping << " x13adapt[" << i << "] = " << gbd13.getInternalValue() << ";" << std::endl
			    << " y13adapt[" << i << "] = " << gbd13.value() << ";" << std::endl;
	}

	mapping << std::endl
	        << "  TGraph *tg13 = new TGraph(" << NTESTS << ", x13, y13);" << std::endl
	        << "  TGraph *tg13adapt = new TGraph(" << NTESTS << ", x13adapt, y13adapt);" << std::endl
	        << "  TGraph *tg052 = new TGraph(" << NTESTS << ", x052, y052);" << std::endl
	        << std::endl
	        << "  tg13->SetMarkerStyle(21);" << std::endl
	        << "  tg13->SetMarkerSize(0.2);" << std::endl
	        << "  tg13->SetMarkerColor(4);" << std::endl
	        << "  tg13adapt->SetMarkerStyle(21);" << std::endl
	        << "  tg13adapt->SetMarkerSize(0.2);" << std::endl
	        << "  tg13adapt->SetMarkerColor(3);" << std::endl
	        << "  tg052->SetMarkerStyle(21);" << std::endl
	        << "  tg052->SetMarkerSize(0.2);" << std::endl
	        << "  tg052->SetMarkerColor(2);" << std::endl
	        << std::endl
	        << "  tg13->Draw(\"AP\");" << std::endl
	        << "  tg052->Draw(\"P\");" << std::endl
	        << "  tg13adapt->Draw(\"P\");" << std::endl
		    << std::endl
		    << "  TLine *xaxis = new TLine(-12.,0.,12.,0.);" << std::endl
		    << "  TLine *yaxis = new TLine(0.,-1.4,0.,3.4);" << std::endl
		    << std::endl
		    << "  xaxis->Draw();" << std::endl
		    << "  yaxis->Draw();" << std::endl
		    << std::endl
		    << "  TPaveText *pt = new TPaveText(0.349138,0.872881,0.637931,0.963983,\"blNDC\");" << std::endl
		    << "  pt->SetBorderSize(2);" << std::endl
		    << "  pt->SetFillColor(19);" << std::endl
		    << "  pt->AddText(\"Test of the GBoundedDouble class\");" << std::endl
		    << "  pt->Draw();" << std::endl
	        << "}" << std::endl;

	mapping.close();

	//***************************************************************************
	// Test b: Distortian of gaussians when being translated from internal to
	// external value. The test produces 10 gaussian random number distributions,
	// whose mean is shifted from left to right of a [-1,1] range

	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr;
	GBoundedDouble gbd_distortion(-1.,1.);

	std::ofstream distortion("distortion.C");

	distortion << "{" << std::endl
	           << "  gROOT->SetStyle(\"Plain\");" << std::endl
	           << "  gStyle -> SetOptStat(kFALSE);" << std::endl
               << std::endl
			   << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);" << std::endl
			   << "  cc->Divide(2,7);" << std::endl
			   << std::endl
			   << "  TH1F *external0 = new TH1F(\"external0\",\"external0\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external1 = new TH1F(\"external1\",\"external1\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external2 = new TH1F(\"external2\",\"external2\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external3 = new TH1F(\"external3\",\"external3\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external4 = new TH1F(\"external4\",\"external4\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external5 = new TH1F(\"external5\",\"external5\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external6 = new TH1F(\"external6\",\"external6\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external7 = new TH1F(\"external7\",\"external7\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external8 = new TH1F(\"external8\",\"external8\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external9 = new TH1F(\"external9\",\"external9\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external10 = new TH1F(\"external10\",\"external10\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external11 = new TH1F(\"external11\",\"external11\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external12 = new TH1F(\"external12\",\"external12\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *external13 = new TH1F(\"external13\",\"external13\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal0 = new TH1F(\"internal0\",\"internal0\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal1 = new TH1F(\"internal1\",\"internal1\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal2 = new TH1F(\"internal2\",\"internal2\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal3 = new TH1F(\"internal3\",\"internal3\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal4 = new TH1F(\"internal4\",\"internal4\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal5 = new TH1F(\"internal5\",\"internal5\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal6 = new TH1F(\"internal6\",\"internal6\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal7 = new TH1F(\"internal7\",\"internal7\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal8 = new TH1F(\"internal8\",\"internal8\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal9 = new TH1F(\"internal9\",\"internal9\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal10 = new TH1F(\"internal10\",\"internal10\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal11 = new TH1F(\"internal11\",\"internal11\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal12 = new TH1F(\"internal12\",\"internal12\",301,-1.5,1.5);" << std::endl
			   << "  TH1F *internal13 = new TH1F(\"internal13\",\"internal13\",301,-1.5,1.5);" << std::endl
			   << std::endl;

	for(std::size_t i=0; i<NTESTS; i++) {
		internalValue = gr.normal_distribution(0.1);

		distortion << "  external0->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 1.1) << ");" << std::endl
				   << "  external1->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 1.0) << ");" << std::endl
			       << "  external2->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 0.9) << ");" << std::endl
				   << "  external3->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 0.7) << ");" << std::endl
				   << "  external4->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 0.5) << ");" << std::endl
				   << "  external5->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 0.3) << ");" << std::endl
				   << "  external6->Fill(" << gbd_distortion.calculateExternalValue(internalValue - 0.1) << ");" << std::endl
				   << "  external7->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 0.1) << ");" << std::endl
				   << "  external8->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 0.3) << ");" << std::endl
				   << "  external9->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 0.5) << ");" << std::endl
				   << "  external10->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 0.7) << ");" << std::endl
				   << "  external11->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 0.9) << ");" << std::endl
				   << "  external12->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 1.0) << ");" << std::endl
				   << "  external13->Fill(" << gbd_distortion.calculateExternalValue(internalValue + 1.1) << ");" << std::endl
				   << "  internal0->Fill(" << internalValue - 1.1 << ");" << std::endl
				   << "  internal1->Fill(" << internalValue - 1.0 << ");" << std::endl
				   << "  internal2->Fill(" << internalValue - 0.9 << ");" << std::endl
				   << "  internal3->Fill(" << internalValue - 0.7 << ");" << std::endl
				   << "  internal4->Fill(" << internalValue - 0.5 << ");" << std::endl
				   << "  internal5->Fill(" << internalValue - 0.3 << ");" << std::endl
				   << "  internal6->Fill(" << internalValue - 0.1 << ");" << std::endl
				   << "  internal7->Fill(" << internalValue + 0.1 << ");" << std::endl
				   << "  internal8->Fill(" << internalValue + 0.3 << ");" << std::endl
				   << "  internal9->Fill(" << internalValue + 0.5 << ");" << std::endl
				   << "  internal10->Fill(" << internalValue + 0.7 << ");" << std::endl
				   << "  internal11->Fill(" << internalValue + 0.9 << ");" << std::endl
				   << "  internal12->Fill(" << internalValue + 1.0 << ");" << std::endl
				   << "  internal13->Fill(" << internalValue + 1.1 << ");" << std::endl;
	}

	distortion << std::endl
			   << "  cc->cd(1);" << std::endl
			   << "  external0->SetFillColor(4);" << std::endl
			   << "  external0->SetFillStyle(1001);" << std::endl
		       << "  external0->Draw();" << std::endl
			   << "  internal0->SetFillColor(2);" << std::endl
			   << "  internal0->SetFillStyle(3004);" << std::endl
		       << "  internal0->Draw(\"same\");" << std::endl
		       << std::endl
		       << "  cc->cd(2);" << std::endl
			   << "  external1->SetFillColor(4);" << std::endl
			   << "  external1->SetFillStyle(1001);" << std::endl
			   << "  external1->Draw();" << std::endl
			   << "  internal1->SetFillColor(2);" << std::endl
			   << "  internal1->SetFillStyle(3004);" << std::endl
		       << "  internal1->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(3);" << std::endl
			   << "  external2->SetFillColor(4);" << std::endl
			   << "  external2->SetFillStyle(1001);" << std::endl
			   << "  external2->Draw();" << std::endl
			   << "  internal2->SetFillColor(2);" << std::endl
			   << "  internal2->SetFillStyle(3004);" << std::endl
		       << "  internal2->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(4);" << std::endl
			   << "  external3->SetFillColor(4);" << std::endl
			   << "  external3->SetFillStyle(1001);" << std::endl
			   << "  external3->Draw();" << std::endl
			   << "  internal3->SetFillColor(2);" << std::endl
			   << "  internal3->SetFillStyle(3004);" << std::endl
		       << "  internal3->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(5);" << std::endl
			   << "  external4->SetFillColor(4);" << std::endl
			   << "  external4->SetFillStyle(1001);" << std::endl
			   << "  external4->Draw();" << std::endl
			   << "  internal4->SetFillColor(2);" << std::endl
			   << "  internal4->SetFillStyle(3004);" << std::endl
		       << "  internal4->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(6);" << std::endl
			   << "  external5->SetFillColor(4);" << std::endl
			   << "  external5->SetFillStyle(1001);" << std::endl
			   << "  external5->Draw();" << std::endl
			   << "  internal5->SetFillColor(2);" << std::endl
			   << "  internal5->SetFillStyle(3004);" << std::endl
		       << "  internal5->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(7);" << std::endl
			   << "  external6->SetFillColor(4);" << std::endl
			   << "  external6->SetFillStyle(1001);" << std::endl
			   << "  external6->Draw();" << std::endl
			   << "  internal6->SetFillColor(2);" << std::endl
			   << "  internal6->SetFillStyle(3004);" << std::endl
		       << "  internal6->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(8);" << std::endl
			   << "  external7->SetFillColor(4);" << std::endl
			   << "  external7->SetFillStyle(1001);" << std::endl
			   << "  external7->Draw();" << std::endl
			   << "  internal7->SetFillColor(2);" << std::endl
			   << "  internal7->SetFillStyle(3004);" << std::endl
		       << "  internal7->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(9);" << std::endl
			   << "  external8->SetFillColor(4);" << std::endl
			   << "  external8->SetFillStyle(1001);" << std::endl
			   << "  external8->Draw();" << std::endl
			   << "  internal8->SetFillColor(2);" << std::endl
			   << "  internal8->SetFillStyle(3004);" << std::endl
		       << "  internal8->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(10);" << std::endl
			   << "  external9->SetFillColor(4);" << std::endl
			   << "  external9->SetFillStyle(1001);" << std::endl
			   << "  external9->Draw();" << std::endl
			   << "  internal9->SetFillColor(2);" << std::endl
			   << "  internal9->SetFillStyle(3004);" << std::endl
		       << "  internal9->Draw(\"same\");" << std::endl
			   << "  cc->cd();" << std::endl
		       << std::endl
			   << "  cc->cd(11);" << std::endl
			   << "  external10->SetFillColor(4);" << std::endl
			   << "  external10->SetFillStyle(1001);" << std::endl
			   << "  external10->Draw();" << std::endl
			   << "  internal10->SetFillColor(2);" << std::endl
			   << "  internal10->SetFillStyle(3004);" << std::endl
		       << "  internal10->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(12);" << std::endl
			   << "  external11->SetFillColor(4);" << std::endl
			   << "  external11->SetFillStyle(1001);" << std::endl
			   << "  external11->Draw();" << std::endl
			   << "  internal11->SetFillColor(2);" << std::endl
			   << "  internal11->SetFillStyle(3004);" << std::endl
		       << "  internal11->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(13);" << std::endl
			   << "  external12->SetFillColor(4);" << std::endl
			   << "  external12->SetFillStyle(1001);" << std::endl
			   << "  external12->Draw();" << std::endl
			   << "  internal12->SetFillColor(2);" << std::endl
			   << "  internal12->SetFillStyle(3004);" << std::endl
		       << "  internal12->Draw(\"same\");" << std::endl
		       << std::endl
			   << "  cc->cd(14);" << std::endl
			   << "  external13->SetFillColor(4);" << std::endl
			   << "  external13->SetFillStyle(1001);" << std::endl
			   << "  external13->Draw();" << std::endl
			   << "  internal13->SetFillColor(2);" << std::endl
			   << "  internal13->SetFillStyle(3004);" << std::endl
		       << "  internal13->Draw(\"same\");" << std::endl
			   << "  cc->cd();" << std::endl
			   << "}" << std::endl;

	distortion.close();

	return 0;
}
