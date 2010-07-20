/**
 * @file GBooleanProbabilityTest.cpp
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <vector>

// Boost header files go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

// GenEvA header files go here
#include "optimization/GBoolean.hpp"
#include "optimization/GBooleanCollection.hpp"
#include "optimization/GBooleanAdaptor.hpp"

using namespace boost;
using namespace Gem;
using namespace Gem::GenEvA;

/**
 * This test checks the flip probability of GBoolean and a GBooleanCollection. Likewise this is a test
 * for the GBooleanAdaptor class and the class's operator=; Tests include constant flip probability
 * as well as mutative adaption of the flip probability. Results can be viewed using the ROOT
 * analysis toolkit (see http://root.cern.ch).
 */

const std::size_t MAXFLIP=10000;
const std::size_t NBIT=10;

/************************************************************************************************/
/**
 * The main function
 */
int main(int argc, char **argv){
	// Create test candidates
	GBoolean A(true), A_tmp;
	GBooleanCollection B(NBIT), B_tmp; // B is initialized with 100 random booleans

	boost::shared_ptr<GBooleanAdaptor> A_adaptor(new GBooleanAdaptor(0.1));
	boost::shared_ptr<GBooleanAdaptor> B_adaptor(new GBooleanAdaptor(0.2));

	A.addAdaptor(A_adaptor);
	B.addAdaptor(B_adaptor);

	std::ofstream ofs("bitflipResult.C"); // Output file

	ofs << "{" << std::endl
		<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,800);" << std::endl
		<< "  cc->Divide(2,2);" << std::endl
		<< std::endl
		<< "  TH1F *singleFlipValueNPA = new TH1F(\"singleFlipValueNPA\",\"singleFlipValueNPA\",2,-0.5,1.5);" << std::endl
		<< "  TH1F *collectionFlipValueNPA = new TH1F(\"collectionFlipValueNPA\",\"collectionFlipValueNPA\",2,-0.5,1.5);" << std::endl
		<< "  TH1F *singleFlipValuePA = new TH1F(\"singleFlipValuePA\",\"singleFlipValuePA\",2,-0.5,1.5);" << std::endl
		<< "  TH1F *collectionFlipValuePA = new TH1F(\"collectionFlipValuePA\",\"collectionFlipValuePA\",2,-0.5,1.5);" << std::endl
		<< std::endl;

	/////////////////////////////////////////////////////////////////////////////////////////
	// Tests without adaption of flip probability
	A_adaptor->setAdaptionThreshold(0);
	B_adaptor->setAdaptionThreshold(0);

	double A_noprobadapt_notflipped=0., A_noprobadapt_flipped=0.;
	double B_noprobadapt_notflipped=0., B_noprobadapt_flipped=0.;
	for(std::size_t i=0; i<MAXFLIP; i++){
		// GBoolean
		A_tmp = A;
		A.adapt(); // adapt
		if(A.value() == A_tmp.value()){
			ofs << "  singleFlipValueNPA->Fill(0.);" << std::endl; // 0 means "not flipped"
			A_noprobadapt_notflipped += 1.;
		}
		else{
			ofs << "  singleFlipValueNPA->Fill(1.);" << std::endl; // 1 means "flipped"
			A_noprobadapt_flipped += 1.;
		}

		// GBooleanCollection
		B_tmp = B;
		B.adapt();
		for(std::size_t j=0; j<NBIT; j++){
			if(B[j] == B_tmp[j]){
				ofs << "  collectionFlipValueNPA->Fill(0.);" << std::endl; // 0 means "not flipped"
				B_noprobadapt_notflipped += 1.;
			}
			else {
				ofs << "  collectionFlipValueNPA->Fill(1.);" << std::endl; // 1 means "flipped"
				B_noprobadapt_flipped += 1.;
			}
		}
	}

	std::cout << "A flip ratio (no probability adaption): " << A_noprobadapt_flipped/double(MAXFLIP) << std::endl
		      << "B flip ratio (no probability adaption): " << B_noprobadapt_flipped/double(MAXFLIP*NBIT) << std::endl;

	/////////////////////////////////////////////////////////////////////////////////////////
	// Tests with adaption of flip probability
	A_adaptor->setAdaptionThreshold(10);
	B_adaptor->setAdaptionThreshold(1);

	// A_adaptor->setAdaptionParameters(0.1,0.01,0.00001,10); // This will result in a rather large adaption rate
	// B_adaptor->setAdaptionParameters(0.1,0.01,0.00001,10);

	A_adaptor->setAdaptionProbability(0.25);
	B_adaptor->setAdaptionProbability(0.5);

	double A_probadapt_notflipped=0., A_probadapt_flipped=0.;
	double B_probadapt_notflipped=0., B_probadapt_flipped=0.;
	for(std::size_t i=0; i<MAXFLIP; i++){
		// GBoolean
		A_tmp = A;
		A.adapt(); // adapt
		if(A.value() == A_tmp.value()){
			ofs << "  singleFlipValuePA->Fill(0.);" << std::endl; // 0 means "not flipped"
			A_probadapt_notflipped += 1.;
		}
		else{
			ofs << "  singleFlipValuePA->Fill(1.);" << std::endl; // 1 means "flipped"
			A_probadapt_flipped += 1.;
		}

		// GBooleanCollection
		B_tmp = B;
		B.adapt();
		for(std::size_t j=0; j<NBIT; j++){
			if(B[j] == B_tmp[j]){
				ofs << "  collectionFlipValuePA->Fill(0.);" << std::endl; // 0 means "not flipped"
				B_probadapt_notflipped += 1.;
			}
			else {
				ofs << "  collectionFlipValuePA->Fill(1.);" << std::endl; // 1 means "flipped"
				B_probadapt_flipped += 1.;
			}
		}
	}

	std::cout << "A flip ratio (probability adaption): " << A_probadapt_flipped/double(MAXFLIP) << std::endl
		      << "B flip ratio (probability adaption): " << B_probadapt_flipped/double(MAXFLIP*NBIT) << std::endl;

	ofs << std::endl
		<< "  cc->cd(1);" << std::endl
		<< "  singleFlipValueNPA->Draw();" << std::endl
		<< "  cc->cd(2);" << std::endl
		<< "  collectionFlipValueNPA->Draw();" << std::endl
		<< "  cc->cd(3);" << std::endl
		<< "  singleFlipValuePA->Draw();" << std::endl
		<< "  cc->cd(4);" << std::endl
		<< "  collectionFlipValuePA->Draw();" << std::endl
		<< "  cc->cd();" << std::endl
		<< "}" << std::endl;

	ofs.close();

	return 0;
}
