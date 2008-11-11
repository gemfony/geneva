/* createHeaders.cpp
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
  lookuptable. The test program in file testLookup.cpp shows, however,
  that there is no gain.
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdlib>

using namespace std;

const long GSINSIZE = 100000;
const long GLOGSIZE = 100000;
const long GRANDSIZE = 200000;

main(){
  cout << "Creating file GSin.hpp" << endl;
  ofstream ofgsin("GSin.hpp");
  ofgsin << "#ifndef GSIN_HPP_" << endl
         << "#define GSIN_HPP_" << endl
         << endl
	 << "const std::size_t GSINSIZE=" << GSINSIZE << ";" << endl
         << setprecision(15) << "const double GSin[GSINSIZE] = {" << endl;
  for(long i=0; i<GSINSIZE-1; i++)
    ofgsin << sin(2.*M_PI*double(i)/double(GSINSIZE)) << ", // " << i << endl;

  ofgsin << sin(2.*M_PI*double(GSINSIZE-1)/double(GSINSIZE)) << "}; // " << GSINSIZE-1 << endl
         << endl
         << "#endif /* GSIN_HPP_ */" << endl;
  ofgsin.close();

  cout << "Creating file GLog.hpp" << endl;
  ofstream ofglog("GLog.hpp");
  ofglog << "#ifndef GLOG_HPP_" << endl
         << "#define GLOG_HPP_" << endl
         << endl
	 << "const std::size_t GLOGSIZE=" << GLOGSIZE << ";" << endl
         << setprecision(15) << "const double GLog[GLOGSIZE] = {" << endl;
  for(long i=0; i<GLOGSIZE-1; i++)
    ofglog << fabs(-2. * log(1. - double(i)/double(GLOGSIZE)))  << ", // " << i << endl;

  ofglog << fabs(-2. * log(1. - double(GLOGSIZE-1)/double(GLOGSIZE))) << "}; // " << GLOGSIZE-1 << endl
	 << endl
	 << "#endif /* GLOG_HPP_ */" << endl;
  ofglog.close();

  cout << "Creating file GRand.hpp" << endl;
  ofstream ofgrand("GRand.hpp");
  srandom(10);
  ofgrand << "#ifndef GRAND_HPP_" << endl
         << "#define GRAND_HPP_" << endl
         << endl
	 << "const std::size_t GRANDSIZE=" << GRANDSIZE << ";" << endl
         << setprecision(15) << "const double GRand[GRANDSIZE] = {" << endl;
  for(long i=0; i<GRANDSIZE-1; i++)
    ofgrand << double(random())/double(RAND_MAX) << "," << endl;

  ofgrand << double(random())/double(RAND_MAX) << "}; // " << endl
	 << endl
	 << "#endif /* GRAND_HPP_ */" << endl;
  ofgrand.close();
}
