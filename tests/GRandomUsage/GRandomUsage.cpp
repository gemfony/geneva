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
CHARRND
};

const std::size_t NENTRIES=20000;

template <class T>
void createRandomVector(std::vector<T>& vec_t, const distType& dType, const std::size_t& nEntries){
	Gem::Util::GRandom gr; // create our own local consumer
	std::size_t i;

	switch(dType){
	case GAUSSIAN: // standard distribution
		for(i=0; i<nEntries; i++) vec_t.push_back(gr.gaussRandom(-3.,1.));
		break;
	case DOUBLEGAUSSIAN:
		for(i=0; i<nEntries; i++) vec_t.push_back(gr.doubleGaussRandom(-3.,0.5,3.));
		break;
	case EVEN: // double in the range [0,1[
		for(i=0; i<nEntries; i++) vec_t.push_back(gr.evenRandom());
		break;
	case EVENWITHBOUNDARIES: // double in the range [-3,2[
		for(i=0; i<nEntries; i++) vec_t.push_back(gr.evenRandom(-3.,2.));
		break;
	case DISCRETE:
		for(i=0; i<nEntries; i++) vec_t.push_back(gr.discreteRandom(10));
		break;
	case DISCRETEBOUND:
		for(i=0; i<nEntries; i++) vec_t.push_back(gr.discreteRandom(-3,10));
		break;
	case BITPROB:
		for(i=0; i<nEntries; i++){
			if(gr.bitRandom(0.7) == Gem::GenEvA::TRUE)
				vec_t.push_back(1);
			else
				vec_t.push_back(0);
		}
		break;
	case CHARRND:
		for(i=0; i<nEntries; i++){
			char tmp = gr.charRandom(false); // also non-printable ASCII characters
			vec_t.push_back((int16_t)tmp);
		}
		break;
	}
}

int main(int argc, char **argv){
	bool verbose;
	std::size_t nEntries;
	boost::uint16_t nProducerThreads;

	if(!parseCommandLine(argc, argv,
						 nEntries,
						 nProducerThreads,
						 verbose))
	{ exit(1); }

	std::size_t i;
	std::vector<double> gaussian, doublegaussian, even, evenwithboundaries;
	std::vector<boost::int16_t> discrete, discretebound, bitprob, charrnd;

	// 5 threads produce even [0,1[ random numbers,
	// 3 threads produce random numbers with a gaussian distribution
	GRANDOMFACTORY.setNProducerThreads(nProducerThreads);

	boost::thread g_thread(boost::bind(createRandomVector<double>, boost::ref(gaussian), GAUSSIAN, nEntries));
	boost::thread dg_thread(boost::bind(createRandomVector<double>, boost::ref(doublegaussian), DOUBLEGAUSSIAN, nEntries));
	boost::thread e_thread(boost::bind(createRandomVector<double>, boost::ref(even), EVEN, nEntries));
	boost::thread ewb_thread(boost::bind(createRandomVector<double>, boost::ref(evenwithboundaries), EVENWITHBOUNDARIES, nEntries));
	boost::thread dc_thread(boost::bind(createRandomVector<boost::int16_t>, boost::ref(discrete), DISCRETE, nEntries));
	boost::thread dcb_thread(boost::bind(createRandomVector<boost::int16_t>, boost::ref(discretebound), DISCRETEBOUND, nEntries));
	boost::thread bp_thread(boost::bind(createRandomVector<boost::int16_t>, boost::ref(bitprob), BITPROB, nEntries));
	boost::thread cr_thread(boost::bind(createRandomVector<boost::int16_t>, boost::ref(charrnd), CHARRND, nEntries));

	g_thread.join();
	dg_thread.join();
	e_thread.join();
	ewb_thread.join();
	dc_thread.join();
	dcb_thread.join();
	bp_thread.join();
	cr_thread.join();

	if(gaussian.size() != nEntries ||
	   doublegaussian.size() != nEntries ||
	   even.size() != nEntries ||
	   evenwithboundaries.size() != nEntries ||
	   discrete.size() != nEntries ||
	   discretebound.size() != nEntries ||
	   bitprob.size() != nEntries ||
	   charrnd.size() != nEntries){
		std::cout << "Error: received invalid sizes for at least one vector" << std::endl;
		return 1;
	}

	std::ofstream ofs("randomResult.C");

	if(ofs){ // Output the data
		// The header of the root file
		ofs << "{" << std::endl;
		ofs << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);" << std::endl
			<< "  cc->Divide(2,4);" << std::endl
			<< std::endl;
		ofs << "  TH1F *gauss = new TH1F(\"gauss\",\"gauss\",200,-8.,2.);" << std::endl;
		ofs << "  TH1F *dgauss = new TH1F(\"dgauss\",\"dgauss\",200,-8.,2.);" << std::endl;
		ofs << "  TH1F *even = new TH1F(\"even\",\"even\",200,-0.5,1.5);" << std::endl;
		ofs << "  TH1F *evenwb = new TH1F(\"evenwb\",\"evenwb\",200,-3.5,2.5);" << std::endl;
		ofs << "  TH1I *discrete = new TH1I(\"discrete\",\"discrete\",12,-1,10);" << std::endl;
		ofs << "  TH1I *discretewb = new TH1I(\"discretewb\",\"discretewb\",16,-4,11);" << std::endl;
		ofs << "  TH1I *bitprob = new TH1I(\"bitprob\",\"bitprob\",4,-1,2);" << std::endl;
		ofs << "  TH1I *charrnd = new TH1I(\"charrnd\",\"charrnd\",131,-1,129);" << std::endl
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
			ofs << "  charrnd->Fill(" << charrnd.at(i) << ");" << std::endl;
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
			<< "  charrnd->Draw();" << std::endl
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
