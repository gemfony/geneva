/* systest.cpp
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
 * In this example we try to repeatedly call an external program with a
 * parameter set and to retrieve its results back. This is for the purpose
 * of writing an indidual that calls an external program for evaluation.
 * Note that there is a second file called progName.cpp which belongs to
 * this test program.
 *
 * Compilation can be done similar to
 * g++ -o systest -lboost_thread-mt systest.cpp
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace std;

void runProgram(std::string progName, double num)
{
  unsigned int i;

  for(i=0; i<100; i++) {
    std::string commandLine;
    std::ostringstream fname;
    double numCalc = 0.;

    // Make the parameters known externally
    fname << "parameters_" << (unsigned int)num;
    std::ofstream parameters(fname.str().c_str(), ios_base::out | ios_base::binary | ios_base::trunc);

    // First emit information about the number of double values
    unsigned int nDParm = 1;
    parameters.write((char *)&nDParm, sizeof(unsigned int));
    // Then write out the actual parameter value
    parameters.write((char *)&num, sizeof(double));

    // Finally close the file
    parameters.close();

    commandLine = progName + " " + fname.str();

    // Run the external program ...
    std::cout << "Calling \"" << commandLine << "\" in thread " << num << std::endl;
    system(commandLine.c_str());

    // ... and retrieve the output
    std::ifstream result(fname.str().c_str(), ios_base::in | ios_base::binary);
    result.read((char *)&numCalc, sizeof(double));

    // Finally close the file
    result.close();

    std::cout << "In thread " << num << ": calculated " << numCalc << std::endl;
  }
}

main() {
  boost::thread thread1(boost::bind(runProgram, "./progName", 1.));
  boost::thread thread2(boost::bind(runProgram, "./progName", 2.));
  boost::thread thread3(boost::bind(runProgram, "./progName", 3.));

  thread1.join();
  thread2.join();
  thread3.join();
}
