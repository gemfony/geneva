/**
 * @file evaluator.cpp
 */

/*
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
 * This file complements the GExternalProgram example. It represents the
 * fitness calculation triggered by the corresponding function.
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#include "commandLineParser.hpp"

using namespace std;

int main(int argc, char **argv)
{
  std::vector<double> dParm;
  std::size_t nDParm;
  double current;

  if(argc != 2) {
    std::cerr << "Error" << std::endl;
    exit(1);
  }

  std::string fname = argv[1];
  std::ifstream parameters(fname.c_str(), ios_base::in | ios_base::binary);

  parameters.read((char *)&nDParm, sizeof(std::size_t));
  for(std::size_t i=0; i<nDParm; i++){
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

  return 0;
}
