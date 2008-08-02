/**
 * @file
 */

/* gboundeddoubletest.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Standard header files
#include <vector>
#include <iostream>
#include <fstream>

// Boost header files
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/cstdint.hpp>

#include "GBoundedDouble.hpp"

using namespace Gem::GenEvA;
using namespace boost;

const boost::uint32_t NTESTS=10000;

int main(int argc, char **argv){
	GBoundedDouble gbd13(-1.,3.); // lower boundary -1, upper Boundary 3
	GBoundedDouble gbd052(0.5,2.); // lower boundary 0.5, upper Boundary 2
	double internalValue = 0., externalValue = 0.;

	std::ofstream mapping("mapping.C");

	mapping << "{" << std::endl
		    << "  double x13[" << NTESTS << "], y13[" << NTESTS << "];" << std::endl
		    << "  double x052[" << NTESTS << "], y052[" << NTESTS << "];" << std::endl
		    << std::endl;

	for(boost::uint32_t i=0; i<NTESTS; i++){
		internalValue=-10.+20.*double(i)/double(NTESTS);

		externalValue = gbd13.calculateExternalValue(internalValue);

		// std::cout << internalValue << " " << externalValue << std::endl;

		mapping << "  x13[" << i << "] = " << internalValue << ";" << std::endl
		        << "  y13[" << i << "] = " << externalValue << ";" << std::endl;
	}

	for(boost::uint32_t i=0; i<NTESTS; i++){
		internalValue=-10.+20.*double(i)/double(NTESTS);

		externalValue = gbd052.calculateExternalValue(internalValue);
		mapping << "  x052[" << i << "] = " << internalValue << ";" << std::endl
		        << "  y052[" << i << "] = " << externalValue << ";" << std::endl;
	}

	mapping << std::endl
	        << "  TGraph *tg13 = new TGraph(" << NTESTS << ", x13, y13);" << std::endl
	        << "  TGraph *tg052 = new TGraph(" << NTESTS << ", x052, y052);" << std::endl
	        << std::endl
	        << "  tg13->SetMarkerStyle(21);" << std::endl
	        << "  tg13->SetMarkerSize(0.2);" << std::endl
	        << "  tg13->SetMarkerColor(4);" << std::endl
	        << "  tg052->SetMarkerStyle(21);" << std::endl
	        << "  tg052->SetMarkerSize(0.2);" << std::endl
	        << "  tg052->SetMarkerColor(2);" << std::endl
	        << std::endl
	        << "  tg13->Draw(\"ALP\");" << std::endl
	        << "  tg052->Draw(\"LP\");" << std::endl
		    << std::endl
		    << "TLine *xaxis = new TLine(-12.,0.,12.,0.);" << std::endl
		    << "TLine *yaxis = new TLine(0.,-1.4,0.,3.4);" << std::endl
		    << std::endl
		    << "xaxis->Draw();" << std::endl
		    << "yaxis->Draw();" << std::endl
		    << std::endl
	        << "}" << std::endl;

	mapping.close();

	return 0;
}
