/**
 * @file GBoundedDoubleTest.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */


/**
 * This test takes a GBoundedDouble object and examines the mapping
 * from internal to external representation of its value. It also tests
 * the error handling of this class.
 *
 * In order to see the results, you need the Root toolkit from http://root.cern.ch.
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

// GenEvA header files go here

#include "GBoundedDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"

using namespace Gem::GenEvA;
using namespace boost;

const boost::uint32_t NTESTS=10000;

int main(int argc, char **argv){
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
		    << "  double x13mutate[" << NTESTS << "], y13mutate[" << NTESTS << "];" << std::endl
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

	// Set up and register an adaptor for dbd13, so it
	// knows how to be mutated. We want a sigma of 0.5, sigma-adaption of 0.05 and
	// a minimum sigma of 0.02. The adaptor will be deleted automatically by the
	// GBoundedDouble.
	boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.5,0.05,0.02,"gauss_mutation"));
	gbd13.addAdaptor(gdga);

	gbd13 = 0.; // We can assign a value inside of the allowed value range
	for(boost::uint32_t i=0; i<NTESTS; i++){
		// mutate the value and have a look at the
		// internal and external values.
		gbd13.mutate();

		mapping << " x13mutate[" << i << "] = " << gbd13.getInternalValue() << ";" << std::endl
			    << " y13mutate[" << i << "] = " << gbd13.value() << ";" << std::endl;
	}

	mapping << std::endl
	        << "  TGraph *tg13 = new TGraph(" << NTESTS << ", x13, y13);" << std::endl
	        << "  TGraph *tg13mutate = new TGraph(" << NTESTS << ", x13mutate, y13mutate);" << std::endl
	        << "  TGraph *tg052 = new TGraph(" << NTESTS << ", x052, y052);" << std::endl
	        << std::endl
	        << "  tg13->SetMarkerStyle(21);" << std::endl
	        << "  tg13->SetMarkerSize(0.2);" << std::endl
	        << "  tg13->SetMarkerColor(4);" << std::endl
	        << "  tg13mutate->SetMarkerStyle(21);" << std::endl
	        << "  tg13mutate->SetMarkerSize(0.2);" << std::endl
	        << "  tg13mutate->SetMarkerColor(3);" << std::endl
	        << "  tg052->SetMarkerStyle(21);" << std::endl
	        << "  tg052->SetMarkerSize(0.2);" << std::endl
	        << "  tg052->SetMarkerColor(2);" << std::endl
	        << std::endl
	        << "  tg13->Draw(\"AP\");" << std::endl
	        << "  tg052->Draw(\"P\");" << std::endl
	        << "  tg13mutate->Draw(\"P\");" << std::endl
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

	// gbd13 has a value range of [-1,3]. Try to assign 4:
	try{
		gbd13 = 4;
	}
	catch(geneva_error_condition& error){
		std::cout << "In GBoundedDouble test: Caught geneva_error_condition" << std::endl
				  << "exception with message " << std::endl
				  << error.diagnostic_information() << std::endl
				  << std::endl
				  << "Don't worry - this is expected!" << std::endl
			      << std::endl;
	}

	std::cout << "Now serializing gbd052 and gbd13 in TEXT und XML mode ..." << std::endl;

	// Serialize gbd13 in XML mode and save to a file
	std::ofstream serialRepresentation1("GBoundedDouble-gbd13.xml");
	serialRepresentation1 << gbd13.toString(XMLSERIALIZATION);
	serialRepresentation1.close();

	// Serialize gbd052 in XML mode
	std::ofstream serialRepresentation2("GBoundedDouble-gbd052.xml");
	serialRepresentation2 << gbd052.toString(XMLSERIALIZATION);
	serialRepresentation2.close();

	// Serialize gbd13 in text mode and save to file
	// Serialize gbd052 in text mode
	std::ofstream serialRepresentation3("GBoundedDouble-gbd13.txt");
	serialRepresentation3 << gbd13.toString(TEXTSERIALIZATION);
	serialRepresentation3.close();

	// Serialize gbd052 in text mode
	std::ofstream serialRepresentation4("GBoundedDouble-gbd052.txt");
	serialRepresentation4 << gbd052.toString(TEXTSERIALIZATION);
	serialRepresentation4.close();

	// Serialize gbd13 in text mode and load into gbd052. Note that both
	// must have the same serialization mode ...
	gbd052.fromString(gbd13.toString(TEXTSERIALIZATION),TEXTSERIALIZATION);

	// Serialize gbd13 in text mode and save to file
	std::ofstream serialRepresentation5("GBoundedDouble-gbd052-gbd13clone.xml");
	serialRepresentation5 << gbd052.toString(XMLSERIALIZATION);
	serialRepresentation5.close();

	return 0;
}
