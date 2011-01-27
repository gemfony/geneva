/**
 * @file GSigmaAdaption_test.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
 * This test adapts a double a given number of times and records the values of
 * different entities of a GDoubleGaussAdaptor as a function of the iteration.
 * The output can be processed with the help of the ROOT analysis toolkit (see
 * http://root.cern.ch .
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
#include "geneva/GDoubleGaussAdaptor.hpp"

// Parsing of the command line
#include "GCommandLineParser.hpp"

using namespace Gem::Tests;
using namespace Gem::Geneva;
using namespace Gem::Hap;

int main(int argc, char **argv) {
	bool verbose;
	double sigma, sigmaSigma, minSigma, maxSigma;
	boost::uint32_t maxIter, adaptionThreshold;
	std::string resultFile;

	if (!parseCommandLine(argc, argv,
		                  sigma,
		                  sigmaSigma,
		                  minSigma,
		                  maxSigma,
		                  adaptionThreshold,
		                  resultFile,
		                  maxIter,
		                  verbose))
	{ exit(1); }

	std::ostringstream result;
	std::vector<double> y_mutVal(maxIter);
	std::vector<double> y_mutValDiff(maxIter);
	std::vector<double> y_sigma(maxIter);
	boost::shared_ptr<GDoubleGaussAdaptor> gdga(
			new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));

	result << "{" << std::endl << "  gROOT->Reset();" << std::endl
		   << "  gStyle->SetOptTitle(0);" << std::endl
		   << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);"
		   << std::endl << "  cc->Divide(2,3);" << std::endl << std::endl
		   << "  double x[" << maxIter << "];" << std::endl
		   << "  double y_mutVal[" << maxIter << "];" << std::endl
		   << "  double y_mutValDiff[" << maxIter << "];" << std::endl
		   << "  double y_sigma[" << maxIter << "];" << std::endl
		   << std::endl;

	double mutVal = 0.;
	double mutValOld=0;
	for (boost::uint32_t i = 0; i < maxIter; i++) {
		result << "  x[" << i << "] = " << (double) i << ";" << std::endl;

		mutValOld = mutVal;

		gdga->setAdaptionThreshold(adaptionThreshold);
		gdga->adapt(mutVal);

		// Store for later use
		y_mutVal[i] = mutVal;
		y_mutValDiff[i] = mutVal - mutValOld;
		y_sigma[i] = gdga->getSigma();

		result << "  y_mutVal[" << i << "] = " << y_mutVal[i] << ";" << std::endl
			   << "  y_mutValDiff[" << i << "] = " << y_mutValDiff[i] << ";" << std::endl
			   << "  y_sigma[" << i << "] = " << y_sigma[i] << ";" << std::endl;
	}

	result << std::endl
		   << "  TGraph *mutVal = new TGraph(" << maxIter << ", x, y_mutVal);" << std::endl
		   << "  TGraph *mutValDiff = new TGraph(" << maxIter << ", x, y_mutValDiff);" << std::endl
		   << "  TGraph *sigma = new TGraph(" << maxIter << ", x, y_sigma);" << std::endl
		   << std::endl;

	// Find min/max values of mutVal, mutValDiff and Sigma
	double minMutVal=0.9;
	double maxMutVal=1.;
	double minMutValDiff=0.9;
	double maxMutValDiff=1.;
	double minTstSigma=0.9;
	double maxTstSigma=1.;

	for(boost::uint32_t i=0; i<maxIter; i++){
		if(y_mutVal[i]<minMutVal) minMutVal = y_mutVal[i];
		if(y_mutVal[i]>maxMutVal) maxMutVal = y_mutVal[i];
		if(y_mutValDiff[i]<minMutValDiff) minMutValDiff = y_mutValDiff[i];
		if(y_mutValDiff[i]>maxMutValDiff) maxMutValDiff = y_mutValDiff[i];
		if(y_sigma[i]<minTstSigma) minTstSigma = y_sigma[i];
		if(y_sigma[i]>maxTstSigma) maxTstSigma = y_sigma[i];
	}

	// Create suitable histogram objects and fill the results in
	result   << "  TH1F *h_mutVal = new TH1F(\"h_mutVal\",\"h_mutVal\",100, " << minMutVal << ", " << maxMutVal << ");" << std::endl
			 << "  TH1F *h_mutValDiff = new TH1F(\"h_mutValDiff\",\"h_mutValDiff\",100, " << minMutValDiff << ", " << maxMutValDiff << ");" << std::endl
		     << "  TH1F *h_sigma = new TH1F(\"h_sigma\",\"h_sigma\",100, " << minTstSigma << ", " << maxTstSigma << ");" << std::endl
		     << std::endl;

	for(boost::uint32_t i=0; i<maxIter; i++){
		result << "  h_mutVal->Fill(" << y_mutVal[i] <<");" << std::endl
			   << "  h_mutValDiff->Fill(" << y_mutValDiff[i] <<");" << std::endl
			   << "  h_sigma->Fill(" << y_sigma[i] << ");" << std::endl;
	}


   result << std::endl
		  << "  cc->cd(1);"
		  << "  mutVal->Draw(\"AP\");" << std::endl
		  << "  cc->cd(2);" << std::endl
		  << "  h_mutVal->Draw();" << std::endl
		  << "  cc->cd(3);"
		  << "  mutValDiff->Draw(\"AP\");" << std::endl
		  << "  cc->cd(4);" << std::endl
		  << "  h_mutValDiff->Draw();" << std::endl
		  << "  cc->cd(5);" << std::endl
		  << "  sigma->Draw(\"AP\");"
		  << "  cc->cd(6);" << std::endl
		  << "  h_sigma->Draw();" << std::endl
		  << "  cc->cd();" << std::endl
		  << "}" << std::endl;

	std::ofstream fstr(resultFile.c_str());
	fstr << result.str();
	fstr.close();

	return 0;
}
