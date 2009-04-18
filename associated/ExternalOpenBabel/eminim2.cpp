/**
 * @file eminim2.cpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * Based on earlier work by Dr. Ivan Kondov.
 *
 * This file is part of the eminim2 program. eminim2 is free software:
 * you can redistribute and/or modify this file under the terms of
 * version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * eminim2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This program performs a value calculation for parameters that have been
 * handed to it by the Geneva library. It serves as an example on how it is
 * possible to use external evaluation programs with the Geneva library. Note
 * that it is distributed under a different license than the Geneva library.
 */

// standard headers go here
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

// Boost headers go here
#include <boost/random.hpp>
#include <boost/date_time.hpp>

// OpenBabel headers go here
#include <openbabel/babelconfig.h>
#include <openbabel/base.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/internalcoord.h>

// Our own headers go here
#include "intcoord.hpp"
#include "GParser.hpp"
#include "GDataExchange.hpp"

using namespace OpenBabel;

int main(int argc, char **argv)
{
	/***********************************************************************************
	 * Variable declarations
	 */
	Gem::Util::GDataExchange ge;

	boost::uint16_t executionMode, transferMode;
	bool binary=true;
	std::string paramfile;
	std::string configFile;
	bool singleEvaluation;

	std::string program_name = argv[0];

	unsigned short loglevel;
	bool add_hydrogens;
	std::string forcefield;
	std::string proteinDescription;

	/***********************************************************************************
	 * Parsing
	 */

	// Parse the command line
	if(!parseCommandLine(argc, argv,
				  executionMode,
				  paramfile,
				  transferMode,
				  singleEvaluation,
				  configFile	))
	{ exit(1); }

	// Parse the configuration file
	if(!parseConfigFile(configFile,
				loglevel,
				addhydrogens,
				forcefield,
				proteinDescription))
	{ exit(1); }

	/***********************************************************************************
	 * Find out whether data transfers happen in binary- or text-mode
	 */
	switch(transferMode) {
	case 0:
		binary=true;
		break;
	case 1:
		binary=false;
		break;
	default:
		{
			std::cerr << "Error: Invalid transfer mode" << transferMode << std::endl;
			exit(1);
		}
	}

	/***********************************************************************************
	 * Preparatory work for other program options
	 */

	OBConversion conv;

	// Find Input filetype, define output filetype
	OBFormat *format_in = conv.FormatFromExt(proteinDescription.c_str());
	OBFormat *format_out = conv.FormatFromExt(proteinDescription.c_str());

	// Check input/output format
	if (!format_in || !format_out || !conv.SetInAndOutFormats(format_in, format_out)) {
			std::cerr << "Error in " << program_name << ": cannot read input/output format!" << std::endl;
			exit (1);
	}

	// initialize selected forcefield
	OBForceField* pFF = OBForceField::FindForceField(forcefield);
	if (!pFF) {
		std::cerr <<  "Error in " << program_name << ": could not find forcefield \"" << forcefield << "\"." <<std::endl;
		exit (1);
	}

	// Set the logfile ...
	pFF->SetLogFile(&std::cerr);

	// And the loglevel
	switch(loglevel){
		case 0:
			pFF->SetLogLevel(OBFF_LOGLVL_NONE);
			break;
		case 1:
			pFF->SetLogLevel(OBFF_LOGLVL_LOW);
			break;
		case 2:
			pFF->SetLogLevel(OBFF_LOGLVL_MEDIUM);
			break;
		case 3:
			pFF->SetLogLevel(OBFF_LOGLVL_HIGH);
			break;
		default:
			std::cerr << "Error in " << program_name << ": Found invalid log level " << loglevel << std::endl;
			exit(1);
	}

	// construct a molecule object and make sure it is empty
	OBMol mol;  mol.Clear();

	/***********************************************************************************
	 * If a single evaluation was requested, act on the request and leave
	 */
	if(singleEvaluation) {
		bool readOnce;

		// declare I/O stream handles
		std::ifstream ifs;
		std::ofstream ofs;

		// Read the file
		ifs.open(proteinDescription.c_str());
		if (!ifs) {
			std::cerr <<  "Error in " << program_name << ": cannot read input file!" << std::endl;
			exit (1);
		}

		readOnce = false;
		while(conv.Read(&mol, &ifs) && !mol.Empty()) {
			// There should just be a single molecular conf
			if(readOnce) {
				std::cout << "Error: Trying to read second molecular configuration." << std::endl
								<< "This program assumes that there is only one. Leaving ..." << std::endl;
				break;
			}
			else readOnce=true;

			if (addhydrogens) mol.AddHydrogens();

			if (!pFF->Setup(mol)) {
				std::cerr <<  "Error in " << program_name << ": could not setup force field." << std::endl;
				exit (1);
			}

			// Update the coordinates in the force field and calculate the energy
			double energy=0.;
			pFF->SetCoordinates(mol);
			energy = pFF->Energy(false);

			// Let the audience know
			std::cout << "Energy of molecule in file " << proteinDescription << " is " << energy << " kcal/mol" << std::endl;

			// Clean up
			mol.Clear();
		}

		ifs.close();
		return 0;
	}

	/***********************************************************************************
	 * Act on the execeution mode
	 */

	// See the file GParser.cpp for the available modes
	switch(executionMode) {
	case 1:	// Perform initialization code
		std::cout << "Initializing ...";
		// put your code here
		std::cout << "... done." << std::endl;
		return 0;
		break;

	case 2:	// Perform finalization code
		std::cout << "Finalizing ...";
		// put your code here
		std::cout << " ... done." << std::endl;
		return 0;
		break;

	case 3: // Evaluate
		{
			// Read in the parameter data
			ge.readFromFile(paramfile, binary);

			// Loop over all parameter sets and do the actual calculation
			int i=0;
			do {
				double result = 0.;
				// Loop over all double values and calculate summed squares ("a parabola")
				std::size_t nDoubles = ge.size<double>(); // The amount of doubles in the current data set
				for(std::size_t pos = 0; pos<nDoubles; pos++) result += pow(ge.at<double>(pos), 2);
				ge.setValue(result);
			}
			while(ge.nextDataSet());

			// Write out the results
			ge.writeToFile(paramfile, binary);
		}
		break;

	case 4: // Write out template
		{
			// Here we simply want PARABOLADIM double values with boundaries [PARABOLAMIN:PARABOLAMAX]
			for(std::size_t i=0; i<PARABOLADIM; i++) {
				ge.append(boost::shared_ptr<Gem::Util::GDoubleParameter>(new Gem::Util::GDoubleParameter(100., PARABOLAMIN, PARABOLAMAX)));
			}

			ge.writeToFile(paramfile, binary);
		}
		break;

	case 5: // Write out template, initializing with random variables
		{
			boost::posix_time::ptime t1;
		    boost::uint32_t seed = (uint32_t)t1.time_of_day().total_milliseconds();
			boost::lagged_fibonacci607 lf(seed);

			for(std::size_t i=0; i<PARABOLADIM; i++) {
				ge.append(boost::shared_ptr<Gem::Util::GDoubleParameter>(new Gem::Util::GDoubleParameter(PARABOLAMIN+lf()*(PARABOLAMAX-PARABOLAMIN), PARABOLAMIN, PARABOLAMAX)));
			}

			ge.writeToFile(paramfile, binary);
		}
		break;

	case 6: // Write out the result for a given parameter set
		{
			// Read in the parameter data
			ge.readFromFile(paramfile, binary);

			// Output the data on the console
			std::size_t nDoubles = ge.size<double>(); // The amount of doubles in the current data set
			for(std::size_t pos = 0; pos<nDoubles; pos++) std::cout << ge.at<double>(pos) << std::endl;
		}
		break;

	default:
		std::cerr << "Error: Found invalid execution mode" << std::endl;
		exit(1);
	};


	return 0; // make the compiler happy


	/***********************************************************************************
	 * Perform initialization code.
	 * E.g. you could set up a directory structure here.
	 */
	if(init) {
		// Nothing to do in this case ...
		return 0;
	}

	/***********************************************************************************
	 * Perform finalization code
	 * E.g. you could delete remaining temporary files here
	 */
	if(finalize) {
		// Nothing to do in this case ...
		return 0;
	}

	/***********************************************************************************
	 * Preparatory work for other program options
	 */
	if(singleEvaluation || (paramfile != "unknown" && !paramfile.empty())) {
		// Find Input filetype, define output filetype
		OBFormat *format_in = conv.FormatFromExt(filename.c_str());
		OBFormat *format_out = conv.FormatFromExt(filename.c_str());

		// Check input/output format
		if (!format_in || !format_out || !conv.SetInAndOutFormats(format_in, format_out)) {
			std::cerr << program_name << ": cannot read input/output format!" << std::endl;
			exit (1);
		}

		// declare I/O stream handles
		std::ifstream ifs;
		std::ofstream ofs;

		// Read the file
		ifs.open(filename.c_str());
		if (!ifs) {
			std::cerr << program_name << ": cannot read input file!" << std::endl;
			exit (1);
		}

		// initialize selected forcefield
		OBForceField* pFF = OBForceField::FindForceField(ff);
		if (!pFF) {
			std::cerr << program_name << ": could not find forcefield '" << ff << "'." <<std::endl;
			exit (1);
		}

		// Set the logfile ...
		pFF->SetLogFile(&std::cerr);

		// And the loglevel
		switch(loglevel){
		case 0:
			pFF->SetLogLevel(OBFF_LOGLVL_NONE);
			break;
		case 1:
			pFF->SetLogLevel(OBFF_LOGLVL_LOW);
			break;
		case 2:
			pFF->SetLogLevel(OBFF_LOGLVL_MEDIUM);
			break;
		case 3:
			pFF->SetLogLevel(OBFF_LOGLVL_HIGH);
			break;
		default:
			std::cerr << "Error: Found invalid log level " << loglevel << std::endl;
			exit(1);
		}

		// construct a molecule object
		OBMol mol;  mol.Clear();

		/***********************************************************************************
		 * We have been asked to calculate and print the energy of a given
		 * molecular configuration.
		 */
		if(singleEvaluation) {
			readOnce = false;
			while(conv.Read(&mol, &ifs) && !mol.Empty()) {
				// There should just be a single molecular conf
				if(readOnce) {
					std::cout << "Error: Trying to read second molecular configuration." << std::endl
					<< "This program assumes that there is only one." << std::endl;
					break;
				}
				else readOnce=true;

				if (add_hydrogens) mol.AddHydrogens();

				if (!pFF->Setup(mol)) {
					std::cerr << program_name << ": could not setup force field." << std::endl;
					exit (1);
				}

				// Update the coordinates in the force field and calculate the energy
				pFF->SetCoordinates(mol);
				energy = pFF->Energy(false);

				// Let the audience know
				std::cout << "Energy of molecule in file " << filename << " is " << energy << " kcal/mol" << std::endl;

				// Clean up
				mol.Clear();
			}

			ifs.close();
			return 0;
		}

		/***********************************************************************************
		 * We have been asked to write out a template file. It has the following format:
		 * Structure of parent:
		 *   Number n of double values (int)
		 *   n initial values (double)
		 *
		 * All specifics of the population (size, number of parents, etc.) need
		 * to be specified as arguments of the optimizer itself.
		 *
		 * In order to find out about the correct number of double parameters we need
		 * to read it from the molecule file.
		 */
		if(templ) {
			// Read in the current vector of dihedrals from file:
			// iterate through all structures (conformers) in the file
			readOnce = false;
			while(conv.Read(&mol, &ifs) && !mol.Empty()) {
				// There should just be a single molecular conf
				if(readOnce) {
					std::cout << "Error: Trying to read second molecular configuration." << std::endl
					<< "This program assumes that there is only one." << std::endl;
					break;
				}
				else readOnce=true;

				if (add_hydrogens) mol.AddHydrogens();

				if (!pFF->Setup(mol)) {
					std::cerr << program_name << ": could not setup force field." << std::endl;
					exit (1);
				}

				// construct flat vector of dihedral angles ...
				vod.clear();
				if (!GetVectorOfDihedrals (&mol,vod) || vod.empty()) {
					std::cerr << "Error getting vector of dihedral angles\n";
					return 1;
				}

				// ... and get rid of the molecule
				mol.Clear();
			}

			// Open the parameter file
			std::ofstream ofstr(paramfile.c_str()); // will automatically truncate the file

			if(!ofstr.good()){
				std::cerr << "Error: output stream is in a bad state" << std::endl;
				ofstr.close();
				return 1;
			}

			// Write out the required information about the number
			// of double parameters
			ofstr << vod.size() << std::endl;

			// Not very clean - we just overwrite the existing data.
			std::vector<double>::iterator it;
			if(randInit) {
				boost::posix_time::ptime t1;
			    boost::uint32_t seed = (uint32_t)t1.time_of_day().total_milliseconds();
				boost::lagged_fibonacci607 lf(seed);

				for(it= vod.begin(); it!=vod.end(); ++it)
					*it = -180. + lf() * 360.; // spread range to [-180.,180.[
			}

			// Write out the parameters to file
			for(it= vod.begin(); it!=vod.end(); ++it) ofstr << *it << std::endl;

			// Clean up
			ofstr.close();
			vod.clear();
			return 0;
		}

		/***********************************************************************************
		 * All remaining program options require that the parameter file is read back in.
		 * Its structure is as follows:
		 * Number n of double values (int)
		 * n double values
		 */
		std::ifstream paramStream(paramfile.c_str());
		if(!paramStream) {
			std::cerr << "Error: Could not open file " << paramfile << ". Leaving ..." << std::endl;
			return 1;
		}

		std::size_t pDim = 0;
		paramStream >> pDim;

		vod.clear();
		for(std::size_t i=0; i<pDim; i++) {
			double tmp;
			paramStream >> tmp;
			vod.push_back(tmp);
		}

		// Make sure the parameter stream is closed again
		paramStream.close();

		/***********************************************************************************
		 * We have been asked to write out a result file for the parameter file.
		 */
		if(result) {
			std::cout << "Writing out result file" << std::endl;

			readOnce = false;
			while(conv.Read(&mol, &ifs) && !mol.Empty()) {
				// There should just be a single molecular conf
				if(readOnce) {
					std::cout << "Error: Trying to read second molecular configuration." << std::endl
					<< "This program assumes that there is only one." << std::endl;
					break;
				}
				else readOnce=true;

				if (add_hydrogens) mol.AddHydrogens();

				if (!pFF->Setup(mol)) {
					std::cerr << program_name << ": could not setup force field." << std::endl;
					exit (1);
				}

				if (!SetVectorOfDihedrals(&mol,vod)) {
					std::cerr << "An error occured updating the molecule" << std::endl;
					return 1;
				};

				// write structure to file
				std::ofstream resultFile("result.pdb");
				conv.Write(&mol, &resultFile);
				resultFile.close();

				// ... and get rid of the molecule
				mol.Clear();
			}

			return 0;
		}

		/***********************************************************************************
		 * O.k. so we are supposed to evaluate the content of the parameter file.
		 */
		readOnce = false;
		while(conv.Read(&mol, &ifs) && !mol.Empty()) {
			// There should just be a single molecular conf
			if(readOnce) {
				std::cout << "Error: Trying to read second molecular configuration." << std::endl
				<< "This program assumes that there is only one." << std::endl;
				break;
			}
			else readOnce=true;

			if (add_hydrogens) mol.AddHydrogens();

			if (!pFF->Setup(mol)) {
				std::cerr << program_name << ": could not setup force field." << std::endl;
				exit (1);
			}

			// Attach the parameters to the molecule
			if (!SetVectorOfDihedrals(&mol,vod)) {
				std::cerr << "An error occured updating the molecule" << std::endl;
				return 1;
			};

			// Update the coordinates in the force field and calculate the energy
			pFF->SetCoordinates(mol);
			energy = pFF->Energy(false);

			// Clean up
			mol.Clear();
		}

		// Finally we write the result to the target file
		// Open the parameter file
		std::ofstream resfile(paramfile.c_str()); // will automatically truncate the file

		if(!resfile.good()){
			std::cerr << "Error: output stream is in a bad state" << std::endl;
			resfile.close();
			return 1;
		}

		// Write out the required information and close the file
		resfile <<energy << std::endl;
		resfile.close();

		return 0;
	}

	/***********************************************************************************
	 * Something went severely wrong
	 */
	else {
		std::cerr << "Error: you did not specify a valid parameter file" << std::endl;
		return 1;
	}

	return 0; // make the compiler happy
}
