/* testLookup.cpp
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

/*
  The following was an attempt to replace the supposedly "expensive"
  sin() and log() operations in the gauss calculation with a
  lookuptable. The following test program shows, however, that there
  is no gain. What's more, the gaussian created with this method looks 
  "chunky" (have a look at the ROOT output in file failedLookupTable.pdf .
  Check out http://root.cern.ch for further information on Root.
 */


#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <fstream>

#include "GSin.hpp"
#include "GLog.hpp"
#include "GRand.hpp"

using namespace std;

main(){
  std::size_t j=0, k=0;
  const long MAXRANDNR=GRANDSIZE/2;

// define PRINTRESULTS in order to get results in Root format (see http://root.cern.ch)
#ifdef PRINTRESULTS
  ofstream gauss("gauss.C");
  gauss << "{" << endl
	<< "  TH1F *h1 = new TH1F(\"gauss\",\"gauss\",100,-3.,3.);" << endl
        << endl;
#endif

  for(std::size_t i = 0; i<200000000; i+=2){
    j=i%MAXRANDNR;  k=(i+1)%MAXRANDNR;
    // Use one of the following two lines
    double testVal = sqrt(fabs(-2. * log(1. - GRand[j]))) * sin(2. * M_PI * GRand[k]);
    // double testVal = GLog[(std::size_t)(GLOGSIZE*GRand[j])] * GSin[(std::size_t)(GSINSIZE*GRand[k])];

    // Stop g++ from optimizing the calculation away
    if(i%20000000==0) cout << i << " " << testVal << endl;

#ifdef PRINTRESULTS
    if(j%1000L==0)  gauss << "  gauss->Fill(" << testVal << ");" << endl;
#endif

  }

#ifdef PRINTRESULTS
  gauss << "}" << endl;
#endif
}
