/**
 * @file progName.cpp
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

/*
 * This file complements systest.cpp. See there for an explanation of
 * its purpose.
 *
 * Compilation can be done similar to
 * g++ -o progName progName.cpp
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

main(int argc, char **argv)
{
  std::vector<double> dParm;
  unsigned int nDParm;
  double current;

  if(argc != 2) {
    std::cerr << "Error" << std::endl;
    exit(1);
  }

  std::string fname = argv[1];
  std::ifstream parameters(fname.c_str(), ios_base::in | ios_base::binary);

  parameters.read((char *)&nDParm, sizeof(unsigned int));
  for(unsigned int i=0; i<nDParm; i++){
    parameters.read((char *)&current, sizeof(double));
    dParm.push_back(current);
  }

  parameters.close();

  double result;
  for(unsigned int i=0; i<dParm.size(); i++){
    result += pow(dParm[i], 2);
  }

  std::ofstream resultFile(fname.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
  resultFile.write((char *)&result, sizeof(double));
  resultFile.close();
}
