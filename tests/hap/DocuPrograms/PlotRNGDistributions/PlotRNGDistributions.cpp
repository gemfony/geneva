/**
 * @file PlotRNGDistributions.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

/**
 * This test creates nEntries random numbers each for different random
 * number distributions. The results is output in the ROOT format -- see
 * http://root.cern.ch for further information. When executing the ROOT
 * script, a number of PNGs are created, suitable for inclusion in the
 * documentation.
 */

// Standard header files
#include <vector>
#include <iostream>
#include <fstream>
#include <random>

// Boost header files
#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// Geneva header files
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

using namespace Gem::Hap;
using namespace boost;

enum class distType {
	GAUSSIAN,
	DOUBLEGAUSSIAN,
	EVEN,
	EVENWITHBOUNDARIES,
	DISCRETE,
	DISCRETEBOUND,
	BITPROB,
	BITSIMPLE
};

template <class T>
void createRandomVector(std::vector<T>& vec_t, const distType& dType, const std::size_t& nEntries, std::shared_ptr<Gem::Hap::GRandomBase> gr_ptr){
	std::size_t i;

	std::normal_distribution<double> normal_distribution(0.,0.5);
	Gem::Hap::bi_normal_distribution<double> bi_normal_distribution(0.,0.5,0.5, 2.);
    std::uniform_real_distribution<double> uniform_real_distribution_01;
    std::uniform_int_distribution<std::int32_t> uniform_int_distribution;

	switch(dType){
		case distType::GAUSSIAN: // standard distribution
			for(i=0; i<nEntries; i++) {
				vec_t.push_back(T(normal_distribution(*gr_ptr)));
			}
			break;

		case distType::DOUBLEGAUSSIAN:
			for(i=0; i<nEntries; i++) {
				vec_t.push_back(T(bi_normal_distribution(*gr_ptr)));
			} // (mean, sigma, distance)
			break;

		case distType::EVEN: // double in the range [0,1[
			for(i=0; i<nEntries; i++) {
                vec_t.push_back(T(uniform_real_distribution_01(*gr_ptr)));
            }
			break;

		case distType::EVENWITHBOUNDARIES: // double in the range [-3,2[
			for(i=0; i<nEntries; i++) {
                vec_t.push_back(T(uniform_real_distribution_01(*gr_ptr, std::uniform_real_distribution<double>::param_type(-3.,2.))));
            }
			break;

		case distType::DISCRETE:
			for(i=0; i<nEntries; i++) {
                vec_t.push_back(uniform_int_distribution(*gr_ptr, std::uniform_int_distribution<std::int32_t>::param_type(0,10)));
            }
			break;

		case distType::DISCRETEBOUND:
			for(i=0; i<nEntries; i++) {
                vec_t.push_back(uniform_int_distribution(*gr_ptr, std::uniform_int_distribution<std::int32_t>::param_type(-3,10)));
            }
			break;

		case distType::BITPROB: {
				std::bernoulli_distribution weighted_bool(0.7);
				for (i = 0; i < nEntries; i++) {
					if (weighted_bool(*gr_ptr))
						vec_t.push_back(1);
					else
						vec_t.push_back(0);
				}
			}
			break;
cd
		case distType::BITSIMPLE: {
				std::bernoulli_distribution uniform_bool; // defaults to 0.5
				for (i = 0; i < nEntries; i++) {
					if (uniform_bool(*gr_ptr))
						vec_t.push_back(1);
					else
						vec_t.push_back(0);
				}
			}
			break;
	};
}

int main(int argc, char **argv){
	std::shared_ptr<Gem::Hap::GRandomBase> gr_ptr;

	bool verbose;
	const std::size_t nEntries = 6000;
	const std::uint16_t nProducerThreads = 4;

	std::size_t i;
	std::vector<double> gaussian, doublegaussian, even, evenwithboundaries;
	std::vector<std::int32_t> discrete, discretebound, bitprob, bitsimple;

	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Create a random number proxy
	gr_ptr = std::shared_ptr<GRandomT<RANDFLAVOURS::RANDOMPROXY>>(new GRandomT<RANDFLAVOURS::RANDOMPROXY>());

	boost::filesystem::ofstream ofs("rootPlotRNGDistributions.C");
	if(!ofs) {
		glogger
		<< "Error: Could not write file" << std::endl
		<< GWARNING;
		return 1;
	}

	// The header of the root file
	ofs << "{" << std::endl
	<< "  gROOT->Reset();" << std::endl
	<< "  gStyle->SetOptTitle(0);" << std::endl
	<< "  gStyle->SetOptStat(0);" << std::endl
	<< "  gStyle->SetCanvasColor(0);" << std::endl
	<< "  gStyle->SetStatBorderSize(1);" << std::endl
	<< std::endl
	<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,1200,800);" << std::endl
	<< std::endl
	<< "  TH1F *gauss = new TH1F(\"gauss\",\"gauss\",200,-2.6,2.6);" << std::endl
	<< "  TH1F *dgauss = new TH1F(\"dgauss\",\"dgauss\",200,-4.,4.);" << std::endl
	<< "  TH1F *even = new TH1F(\"even\",\"even\",200,-0.5,1.5);" << std::endl
	<< "  TH1F *evenwb = new TH1F(\"evenwb\",\"evenwb\",200,-3.5,2.5);" << std::endl
	<< "  TH1I *discrete = new TH1I(\"discrete\",\"discrete\",12,-1,10);" << std::endl
	<< "  TH1I *discretewb = new TH1I(\"discretewb\",\"discretewb\",16,-4,11);" << std::endl
	<< "  TH1I *bitprob = new TH1I(\"bitprob\",\"bitprob\",4,-1,2);" << std::endl
	<< "  TH1I *bitsimple = new TH1I(\"bitsimple\",\"bitsimple\",4,-1,2);" << std::endl
	<< std::endl;

	createRandomVector<double>(gaussian, distType::GAUSSIAN, nEntries, gr_ptr);
	createRandomVector<double>(doublegaussian, distType::DOUBLEGAUSSIAN, nEntries, gr_ptr);
	createRandomVector<double>(even, distType::EVEN, nEntries, gr_ptr);
	createRandomVector<double>(evenwithboundaries, distType::EVENWITHBOUNDARIES, nEntries, gr_ptr);
	createRandomVector<std::int32_t>(discrete, distType::DISCRETE, nEntries,gr_ptr);
	createRandomVector<std::int32_t>(discretebound, distType::DISCRETEBOUND, nEntries, gr_ptr);
	createRandomVector<std::int32_t>(bitprob, distType::BITPROB, nEntries, gr_ptr);
	createRandomVector<std::int32_t>(bitsimple, distType::BITSIMPLE, nEntries, gr_ptr);

	for(i=0; i<nEntries; i++){
		ofs << "  gauss->Fill(" << gaussian.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  dgauss->Fill(" << doublegaussian.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  even->Fill(" << even.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  evenwb->Fill(" << evenwithboundaries.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  discrete->Fill(" << discrete.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  discretewb->Fill(" << discretebound.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  bitprob->Fill(" << bitprob.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

	for(i=0; i<nEntries; i++){
		ofs << "  bitsimple->Fill(" << bitsimple.at(i) << ");" << std::endl;
	}
	ofs << std::endl;

//---------------------------------------------------------------------------------

	ofs << "  gauss->GetXaxis()->SetTitle(\"x\");" << std::endl
	<< "  gauss->GetYaxis()->SetTitle(\"number of entries\");" << std::endl
	<< "  gauss->GetYaxis()->SetTitleOffset(1.2);" << std::endl
	<< "  gauss->Draw();" << std::endl
	<< "  TLatex *gaussText1 = new TLatex(0.8,1200,\"Normal distribution\");" << std::endl
	<< "  TLatex *gaussText2 = new TLatex(0.8,1115,\"with mean=0, #sigma=0.5\");" << std::endl
	<< "  gaussText1->SetTextSize(0.035);" << std::endl
	<< "  gaussText2->SetTextSize(0.035);" << std::endl
	<< "  gaussText1->Draw();" << std::endl
	<< "  gaussText2->Draw();" << std::endl
	<< "  gPad->Update();" << std::endl
	<< "  double ymax = gPad->GetUymax();" << std::endl
	<< "  TLine *gaussLine = new TLine(0.,0.,0., ymax);" << std::endl
	<< "  gaussLine->SetLineStyle(2);" << std::endl
	<< "  gaussLine->Draw();" << std::endl
	<< "  cc->SaveAs(\"gauss.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  dgauss->GetXaxis()->SetTitle(\"x\");" << std::endl
	<< "  dgauss->GetYaxis()->SetTitle(\"number of entries\");" << std::endl
	<< "  dgauss->GetYaxis()->SetTitleOffset(1.2);" << std::endl
	<< "  dgauss->Draw();" << std::endl
	<< "  TLatex *dgaussText1 = new TLatex(1.5, 970, \"Two normal distributions with\");" << std::endl
	<< "  TLatex *dgaussText2 = new TLatex(1.5, 920, \"mean=0.5, #sigma=0.5 and distance\");" << std::endl
	<< "  TLatex *dgaussText3 = new TLatex(1.5, 870, \"between the mean values of 2\");" << std::endl
	<< "  dgaussText1->SetTextSize(0.025);" << std::endl
	<< "  dgaussText2->SetTextSize(0.025);" << std::endl
	<< "  dgaussText3->SetTextSize(0.025);" << std::endl
	<< "  dgaussText1->Draw();" << std::endl
	<< "  dgaussText2->Draw();" << std::endl
	<< "  dgaussText3->Draw();" << std::endl
	<< "  gPad->Update();" << std::endl
	<< "  TLine *dgaussLine1 = new TLine(-1.,0.,-1., ymax);" << std::endl
	<< "  dgaussLine1->SetLineStyle(2);" << std::endl
	<< "  dgaussLine1->Draw();" << std::endl
	<< "  TLine *dgaussLine2 = new TLine(1.,0.,1., ymax);" << std::endl
	<< "  dgaussLine2->SetLineStyle(2);" << std::endl
	<< "  dgaussLine2->Draw();" << std::endl
	<< "  cc->SaveAs(\"dgauss.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  even->Draw();" << std::endl
	<< "  cc->SaveAs(\"even.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  evenwb->Draw();" << std::endl
	<< "  cc->SaveAs(\"evenwb.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  discrete->Draw();" << std::endl
	<< "  cc->SaveAs(\"discrete.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  discretewb->Draw();" << std::endl
	<< "  cc->SaveAs(\"discretewb.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  bitprob->Draw();" << std::endl
	<< "  cc->SaveAs(\"bitprob.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "  bitsimple->Draw();" << std::endl
	<< "  cc->SaveAs(\"bitsimple.png\");" << std::endl

	//---------------------------------------------------------------------------------

	<< "}" << std::endl;

	ofs.close();

	return 0;
}
