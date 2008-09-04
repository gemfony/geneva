/**
 * @file GSigmaAdaption_test.cpp
 *
 * This test mutates a double a given number of times and records the values of
 * different entities of a GDoubleGaussAdaptor as a function of the iteration.
 * The output can be processed with the help of the ROOT analysis toolkit (see
 * http://root.cern.ch .
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

// Standard header files go here
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "GDoubleGaussAdaptor.hpp"

// Parsing of the command line
#include "GCommandLineParser.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

int main(int argc, char **argv) {
	bool verbose;
	double sigma, sigmaSigma, minSigma;
	boost::uint32_t maxIter;
	std::string resultFile;

	if (!parseCommandLine(argc, argv,
		                  sigma,
		                  sigmaSigma,
		                  minSigma,
		                  resultFile,
		                  maxIter,
		                  verbose))
	{
		exit(1);
	}

	std::ostringstream result;
	boost::shared_ptr<GDoubleGaussAdaptor> gdga(
			new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, "Adaptor"));

	result << "{" << std::endl << "  gROOT->Reset();" << std::endl
		   << "  gStyle->SetOptTitle(0);" << std::endl
		   << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);"
		   << std::endl << "  cc->Divide(1,2);" << std::endl << std::endl
		   << "  double x[" << maxIter << "];" << std::endl
		   << "  double y_mutVal[" << maxIter << "];" << std::endl
		   << "  double y_sigma[" << maxIter << "];" << std::endl
		   << std::endl;

	double mutVal = 0.;

	for (boost::uint32_t i = 0; i < maxIter; i++) {
		result << "  x[" << i << "] = " << (double) i << ";" << std::endl;

		gdga->mutate(mutVal);
		result << "  y_mutVal[" << i << "] = " << mutVal << ";" << std::endl
			   << "  y_sigma[" << i << "] = " << gdga->getSigma() << ";"
			   << std::endl;
	}

	result << std::endl
		   << "  TGraph *mutVal = new TGraph(" << maxIter << ", x, y_mutVal);" << std::endl
		   << "  TGraph *sigma = new TGraph(" << maxIter << ", x, y_sigma);" << std::endl
		   << std::endl
		   << "  cc->cd(1);"
		   << "  mutVal->Draw(\"AP\");" << std::endl
		   << "  cc->cd(2);" << std::endl
		   << "  sigma->Draw(\"AP\");"
		   << "  cc->cd();" << std::endl
		   << "}"
		   << std::endl;

	ofstream fstr(resultFile.c_str());
	fstr << result.str();
	fstr.close();

	return 0;
}
