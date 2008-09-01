/**
 * @file testNetwork.cpp
 *
 * This program allows to visualize the output of the training example.
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

/*
 * Can be compiled with the command
 * g++ -g -o testNetwork -I/opt/boost136/include/boost-1_36/ testNetwork.cpp
 * on OpenSUSE 11 (assuming that Boost in installed under /opt in your
 * system.
 *
 * NOTE: This program does currently not give useful results.
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/random.hpp>

#include "trainingResult.hpp"

const boost::uint32_t MAXPOINTS=10000;

using namespace Gem::NeuralNetwork;

main(){
  boost::lagged_fibonacci607 lf(123);

  double x=0., y=0., result=0;
  std::vector<double> in;
  std::vector<double> out;

  std::vector<double> x_inside, y_inside;
  std::vector<double> x_outside, y_outside;

  // Create random numbers and check the output
  for(boost::uint32_t i=0; i<MAXPOINTS; i++){
    x=-1. + 2.*lf();
    y=-1. + 2.*lf();

    in.clear();
    out.clear();

    in.push_back(x);
    in.push_back(y);

    if(!network(in,out) || out.size()==0){
      std::cout << "Error in calculation of network output" << std::endl;
      exit(1);
    }

    double output = out[0];
    std::cout << output << std::endl;

    if(output < 0.5) {
      x_inside.push_back(x);
      y_inside.push_back(y);
    }
    else{
      x_outside.push_back(x);
      y_outside.push_back(y);
    }
  }

  // Write test results
  std::ostringstream results;
  results << "{" << std::endl
	  << "  double x_inside[" << x_inside.size() << "];" << std::endl
	  << "  double y_inside[" << y_inside.size() << "];" << std::endl
	  << "  double x_outside[" << x_outside.size() << "];" << std::endl
	  << "  double y_outside[" << y_outside.size() << "];" << std::endl
	  << std::endl;

  for(std::size_t i=0; i<x_inside.size(); i++){
    results << "  x_inside[" << i << "] = " << x_inside[i] << ";" << std::endl
	    << "  y_inside[" << i << "] = " << y_inside[i] << ";" << std::endl;
  }

  for(std::size_t i=0; i<x_outside.size(); i++){
    results << "  x_outside[" << i << "] = " << x_outside[i] << ";" << std::endl
	    << "  y_outside[" << i << "] = " << y_outside[i] << ";" << std::endl;
  }

  results << std::endl
	  << "  TGraph *inside = new TGraph(" << x_inside.size() << ", x_inside, y_inside);" << std::endl
	  << "  TGraph *outside = new TGraph(" << x_outside.size() << ", x_outside, y_outside);" << std::endl
	  << std::endl
	  << "  inside->SetMarkerStyle(21);" << std::endl
	  << "  inside->SetMarkerSize(0.2);" << std::endl
	  << "  inside->SetMarkerColor(4);" << std::endl
	  << "  outside->SetMarkerStyle(21);" << std::endl
	  << "  outside->SetMarkerSize(0.2);" << std::endl
	  << "  outside->SetMarkerColor(3);" << std::endl
          << std::endl
          << "  inside->Draw(\"AP\");" << std::endl
	  << "  outside->Draw(\"P\");" << std::endl
	  << "}" << std::endl;

  std::ofstream fstr("testResults.C");
  fstr << results.str();
  fstr.close();
}
