/**
 * @file GRandomUsage.cpp
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


/**
 * This test creates NENTRIES random numbers each for numbers and items with
 * different characteristics. Each set is created in its own thread.
 * Note that the random numbers are usually not created in the GRandom
 * object, but by the GRandomFactory class in a different thread.
 * GRandom just acts as a user-interface.
 *
 * The results of the test are output in the ROOT format. See
 * http://root.cern.ch for further information.
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
#include <boost/cast.hpp>

// GenEvA header files
#include "GRandom.hpp"

// Parsing of the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace boost;

enum distType {
	GAUSSIAN,
	DOUBLEGAUSSIAN,
	EVEN,
	EVENWITHBOUNDARIES,
	DISCRETE,
	DISCRETEBOUND,
	BITPROB,
	BITSIMPLE,
	CHARRND
};

template <class T>
void createRandomVector(std::vector<T>& vec_t, const distType& dType, const std::size_t& nEntries, boost::shared_ptr<Gem::Util::GRandom> gr_ptr){
	std::size_t i;

	switch(dType){
	case GAUSSIAN: // standard distribution
		for(i=0; i<nEntries; i++) vec_t.push_back(T(gr_ptr->gaussRandom(-3.,1.)));
		break;

	case DOUBLEGAUSSIAN:
		for(i=0; i<nEntries; i++) vec_t.push_back(T(gr_ptr->doubleGaussRandom(-3.,0.5,3.)));
		break;

	case EVEN: // double in the range [0,1[
		for(i=0; i<nEntries; i++) vec_t.push_back(T(gr_ptr->evenRandom()));
		break;

	case EVENWITHBOUNDARIES: // double in the range [-3,2[
		for(i=0; i<nEntries; i++) vec_t.push_back(T(gr_ptr->evenRandom(-3.,2.)));
		break;

	case DISCRETE:
		for(i=0; i<nEntries; i++) vec_t.push_back(boost::numeric_cast<boost::int32_t>(gr_ptr->discreteRandom(10)));
		break;

	case DISCRETEBOUND:
		for(i=0; i<nEntries; i++) vec_t.push_back(boost::numeric_cast<boost::int32_t>(gr_ptr->discreteRandom(-3,10)));
		break;

	case BITPROB:
		for(i=0; i<nEntries; i++){
			if(gr_ptr->boolRandom(0.7))
				vec_t.push_back(1);
			else
				vec_t.push_back(0);
		}
		break;

	case BITSIMPLE:
		for(i=0; i<nEntries; i++){
			if(gr_ptr->boolRandom())
				vec_t.push_back(1);
			else
				vec_t.push_back(0);
		}
		break;

	case CHARRND:
		for(i=0; i<nEntries; i++){
			char tmp = gr_ptr->charRandom(false); // also non-printable ASCII characters
			vec_t.push_back((int16_t)tmp);
		}
		break;
	}
}

int main(int argc, char **argv){
	boost::shared_ptr<Gem::Util::GRandom> gr_ptr(new GRandom());

	bool verbose;
	std::size_t nEntries;
	boost::uint16_t nProducerThreads;
	boost::uint16_t rnrProductionMode;

	if(!parseCommandLine(argc, argv,
						 nEntries,
						 nProducerThreads,
						 rnrProductionMode,
						 verbose))
	{ exit(1); }

	std::size_t i;
	std::vector<double> gaussian, doublegaussian, even, evenwithboundaries;
	std::vector<boost::int32_t> discrete, discretebound, bitprob, bitsimple, charrnd;

	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Set the random number generation mode as requested
	switch(rnrProductionMode) {
	case 0:
		gr_ptr->setRNRFactoryMode();
		break;

	case 1:
		gr_ptr->setRNRLocalMode();
		break;
	};

	createRandomVector<double>(gaussian, GAUSSIAN, nEntries, gr_ptr);
	createRandomVector<double>(doublegaussian, DOUBLEGAUSSIAN, nEntries, gr_ptr);
	createRandomVector<double>(even, EVEN, nEntries, gr_ptr);
	createRandomVector<double>(evenwithboundaries, EVENWITHBOUNDARIES, nEntries, gr_ptr);
	createRandomVector<boost::int32_t>(discrete, DISCRETE, nEntries,gr_ptr);
	createRandomVector<boost::int32_t>(discretebound, DISCRETEBOUND, nEntries, gr_ptr);
	createRandomVector<boost::int32_t>(bitprob, BITPROB, nEntries, gr_ptr);
	createRandomVector<boost::int32_t>(bitsimple, BITSIMPLE, nEntries, gr_ptr);
	createRandomVector<boost::int32_t>(charrnd, CHARRND, nEntries, gr_ptr);

	if(gaussian.size() != nEntries ||
	   doublegaussian.size() != nEntries ||
	   even.size() != nEntries ||
	   evenwithboundaries.size() != nEntries ||
	   discrete.size() != nEntries ||
	   discretebound.size() != nEntries ||
	   bitprob.size() != nEntries ||
	   bitsimple.size() != nEntries ||
	   charrnd.size() != nEntries){
		std::cout << "Error: received invalid sizes for at least one vector" << std::endl;
		return 1;
	}

	std::ofstream ofs("randomResult.C");

	if(ofs){ // Output the data
		// The header of the root file
		ofs << "{" << std::endl;
		ofs << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);" << std::endl
			<< "  cc->Divide(2,5);" << std::endl
			<< std::endl;
		ofs << "  TH1F *gauss = new TH1F(\"gauss\",\"gauss\",200,-8.,2.);" << std::endl;
		ofs << "  TH1F *dgauss = new TH1F(\"dgauss\",\"dgauss\",200,-8.,2.);" << std::endl;
		ofs << "  TH1F *even = new TH1F(\"even\",\"even\",200,-0.5,1.5);" << std::endl;
		ofs << "  TH1F *evenwb = new TH1F(\"evenwb\",\"evenwb\",200,-3.5,2.5);" << std::endl;
		ofs << "  TH1I *discrete = new TH1I(\"discrete\",\"discrete\",12,-1,10);" << std::endl;
		ofs << "  TH1I *discretewb = new TH1I(\"discretewb\",\"discretewb\",16,-4,11);" << std::endl;
		ofs << "  TH1I *bitprob = new TH1I(\"bitprob\",\"bitprob\",4,-1,2);" << std::endl;
		ofs << "  TH1I *bitsimple = new TH1I(\"bitsimple\",\"bitsimple\",4,-1,2);" << std::endl;
		ofs << "  TH1I *charrnd = new TH1I(\"charrnd\",\"charrnd\",131,-1,129);" << std::endl;
		ofs << "  TH2F *evenCorrelation = new TH2F(\"evenCorrelation\",\"evenCorrelation\",100,-1.,1.,100,-1.,1.);" << std::endl
			<< std::endl;

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

		for(i=0; i<nEntries; i++){
			ofs << "  charrnd->Fill(" << charrnd.at(i) << ");" << std::endl;
		}
		ofs << std::endl;

		for(i=0; i<nEntries; i++){
			ofs << "  evenCorrelation->Fill(" << gr_ptr->evenRandom(-1.,1.) << ", " << gr_ptr->evenRandom(-1.,1.)  << ");" << std::endl;
		}
		ofs << std::endl;

		ofs << "  cc->cd(1);" << std::endl
			<< "  gauss->Draw();" << std::endl
			<< "  cc->cd(2);" << std::endl
			<< "  dgauss->Draw();" << std::endl
			<< "  cc->cd(3);" << std::endl
			<< "  even->Draw();" << std::endl
			<< "  cc->cd(4);" << std::endl
			<< "  evenwb->Draw();" << std::endl
			<< "  cc->cd(5);" << std::endl
			<< "  discrete->Draw();" << std::endl
			<< "  cc->cd(6);" << std::endl
			<< "  discretewb->Draw();" << std::endl
			<< "  cc->cd(7);" << std::endl
			<< "  bitprob->Draw();" << std::endl
			<< "  cc->cd(8);" << std::endl
			<< "  bitsimple->Draw();" << std::endl
			<< "  cc->cd(9);" << std::endl
			<< "  charrnd->Draw();" << std::endl
			<< "  cc->cd(10);" << std::endl
			<< "  evenCorrelation->Draw(\"contour\");" << std::endl
			<< "  cc->cd();" << std::endl;
		ofs	<< "}" << std::endl;

		ofs.close();
	}
	else {
		std::cout << "Error: Could not write file" << std::endl;
		return 1;
	}

	return 0;
}
