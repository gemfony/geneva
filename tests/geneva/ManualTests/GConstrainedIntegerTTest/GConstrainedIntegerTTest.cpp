/**
 * @file GConstrainedIntTTest.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

/**
 * This test takes a GConstrainedIntT<boost::int32_t> object and examines the mapping
 * from internal to external representation of its value.
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
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"

using namespace Gem::Geneva;
using namespace boost;

const boost::uint32_t NTESTS=10000;

int main(int argc, char **argv){
	GConstrainedInt32Object gint13(-1, 3); // lower boundary -1, upper Boundary 3
	GConstrainedInt32Object gint02(0, 2); // lower boundary 0.5, upper Boundary 2

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
		    << "  double x02[" << NTESTS << "], y02[" << NTESTS << "];" << std::endl
		    << std::endl;

	for(boost::uint32_t i=0; i<NTESTS; i++){
		internalValue=-10.+20.*double(i)/double(NTESTS);

		externalValue = double(gint13.transfer(boost::int32_t(internalValue)));
		mapping << "  x13[" << i << "] = " << internalValue << ";" << std::endl
		        << "  y13[" << i << "] = " << externalValue << ";" << std::endl;
	}

	for(boost::uint32_t i=0; i<NTESTS; i++){
		internalValue=-10.+20.*double(i)/double(NTESTS);

		externalValue = double(gint02.transfer(boost::int32_t(internalValue)));
		mapping << "  x02[" << i << "] = " << internalValue << ";" << std::endl
		        << "  y02[" << i << "] = " << externalValue << ";" << std::endl;
	}

	// Set up and register an adaptor for gint13, so it
	// knows how to be adapted. We want a sigma of 0.5, sigma-adaption of 0.8 and
	// a minimum sigma of 0.02. The adaptor will be deleted automatically by the
	// GConstrainedIntT<boost::int32_t>.
	boost::shared_ptr<GInt32GaussAdaptor> gdga(new GInt32GaussAdaptor(0.5,0.8,0.02,2.));
	gint13.addAdaptor(gdga);

	// Check that an adaptor is actually present
	if(!gint13.hasAdaptor()) {
		std::cout << "Error: No adaptor present!" << std::endl;
		std::terminate();
	}

	// TODO: Check, warum geht operator=(0) nicht ??? Danach ist der Adaptor weg ...
	// gint13.setValue(0); // We can assign a value inside of the allowed value range
	gint13 = 0;

	for(boost::uint32_t i=0; i<NTESTS; i++){
		// Check that an adaptor is actually present
		if(!gint13.hasAdaptor()) {
			std::cout << "Error: No adaptor present in iteration " << i << "!" << std::endl;
			std::terminate();
		}

		// adapt the value and have a look at the
		// internal and external values.
		gint13.adapt();

		mapping << " x13adapt[" << i << "] = " << gint13.getInternalValue() << ";" << std::endl
			    << " y13adapt[" << i << "] = " << gint13.value() << ";" << std::endl;
	}

	mapping << std::endl
	        << "  TGraph *tg13 = new TGraph(" << NTESTS << ", x13, y13);" << std::endl
	        << "  TGraph *tg13adapt = new TGraph(" << NTESTS << ", x13adapt, y13adapt);" << std::endl
	        << "  TGraph *tg02 = new TGraph(" << NTESTS << ", x02, y02);" << std::endl
	        << std::endl
	        << "  tg13->SetMarkerStyle(21);" << std::endl
	        << "  tg13->SetMarkerSize(0.2);" << std::endl
	        << "  tg13->SetMarkerColor(4);" << std::endl
	        << "  tg13adapt->SetMarkerStyle(21);" << std::endl
	        << "  tg13adapt->SetMarkerSize(0.2);" << std::endl
	        << "  tg13adapt->SetMarkerColor(3);" << std::endl
	        << "  tg02->SetMarkerStyle(21);" << std::endl
	        << "  tg02->SetMarkerSize(0.2);" << std::endl
	        << "  tg02->SetMarkerColor(2);" << std::endl
	        << std::endl
	        << "  tg13->Draw(\"AP\");" << std::endl
	        << "  tg02->Draw(\"P\");" << std::endl
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
		    << "  pt->AddText(\"Test of the GConstrainedInt32Object class\");" << std::endl
		    << "  pt->Draw();" << std::endl
	        << "}" << std::endl;

	mapping.close();

	return 0;
}
