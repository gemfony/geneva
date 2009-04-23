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
	std::string identifyer;
	std::string paramfile;
	std::string configFile;

	std::string program_name = argv[0];

	unsigned short loglevel;
	bool addhydrogens;
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
				  identifyer,
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
	 * Act on the desired execution mode
	 */

	// See the file GParser.cpp for the available modes
	switch(executionMode) {
	//------------------------------------------------------------------------------------------------------
	case 1:	// Perform initialization code
		std::cout << "Initializing ...";
		// put your code here
		std::cout << "... done." << std::endl;
		return 0;
		break;

	//------------------------------------------------------------------------------------------------------
	case 2:	// Perform finalization code
		std::cout << "Finalizing ...";
		// put your code here
		std::cout << " ... done." << std::endl;
		return 0;
		break;

	//------------------------------------------------------------------------------------------------------
	case 3: // Evaluate
		{
			// Read the protein description
			std::ifstream ifs;
			ifs.open(proteinDescription.c_str());

			if (!ifs) {
				std::cerr <<  "Error in " << program_name << ": cannot open input file " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			if(!conv.Read(&mol, &ifs) || mol.Empty()) {
				std::cerr <<  "Error in " << program_name << ": could not read conformer from " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			// We do not need the file anymore at this point
			ifs.close();

			// Add hydrogens, if requested
			if (addhydrogens) mol.AddHydrogens();

			// Attach the force field to the molecule
			if (!pFF->Setup(mol)) {
				std::cerr << "Error in " << program_name << ": could not setup force field." << std::endl;
				exit (1);
			}

			// Read in the parameter data
			ge.readFromFile(paramfile, binary);

			// Now loop over all parameter sets
			do {
				double energy = 0.;
				std::vector<double> vod;

				// Extract the individual parameters ...
				std::size_t nDoubles = ge.size<double>();
				for(std::size_t pos = 0; pos<nDoubles; pos++) vod.push_back(ge.at<double>(pos));

				// ... and attach them to the molecule
				if (!SetVectorOfDihedrals(&mol,vod)) {
					std::cerr <<  "Error in " << program_name << " while updating the molecule" << std::endl;
					return 1;
				};

				// Update the coordinates in the force field and calculate the energy
				pFF->SetCoordinates(mol);
				energy = pFF->Energy(false);

				// Finally store the result in the parameter set
				ge.setValue(energy);
			}
			while(ge.nextDataSet());

			// Clean up
			mol.Clear();

			// Write out the results
			ge.writeToFile(paramfile, binary);
		}
		break;

	//------------------------------------------------------------------------------------------------------
	case 4: // Write out template. This case can be treated together with case 5
	case 5: // Write out template, initializing with random variables
		{
			// Read the protein description
			std::ifstream ifs;
			ifs.open(proteinDescription.c_str());

			if (!ifs) {
				std::cerr <<  "Error in " << program_name << ": cannot open input file " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			if(!conv.Read(&mol, &ifs) || mol.Empty()) {
				std::cerr <<  "Error in " << program_name << ": could not read conformer from " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			// We do not need the file anymore at this point
			ifs.close();

			// Add hydrogens, if requested
			if (addhydrogens) mol.AddHydrogens();

			// Attach the force field to the molecule
			if (!pFF->Setup(mol)) {
				std::cerr << "Error in " << program_name << ": could not setup force field." << std::endl;
				exit (1);
			}

			// construct flat vector of dihedral angles ...
			std::vector<double> vod;
			if (!GetVectorOfDihedrals (&mol,vod) || vod.empty()) {
				std::cerr <<  "Error in " << program_name << " getting vector of dihedral angles" << std::endl;
				return 1;
			}

			// ... and get rid of the molecule
			mol.Clear();

			// Add the contend of vod to the current parameter set, or initialize with random numbers
			std::size_t vodSize = vod.size();
			if(executionMode == 4) {
				for(std::size_t i=0; i<vodSize; i++) {
					ge.append(boost::shared_ptr<Gem::Util::GDoubleParameter>(new Gem::Util::GDoubleParameter(vod.at(i), -180., 180.)));
				}
			}
			else if(executionMode == 5) {
				// Set up a random number generator
				boost::posix_time::ptime t1;
			    boost::uint32_t seed = (uint32_t)t1.time_of_day().total_milliseconds();
				boost::lagged_fibonacci607 lf(seed);

				for(std::size_t i=0; i<vodSize; i++) {
					ge.append(boost::shared_ptr<Gem::Util::GDoubleParameter>(new Gem::Util::GDoubleParameter( -180. + lf() * 360., -180., 180.)));
				}
			}

			// Finally emit the data
			ge.writeToFile(paramfile, binary);
		}

		return 0;
		break;


	//------------------------------------------------------------------------------------------------------
	case 6: // Write out the result for a given parameter set
		std::cout << "Writing out result file ...";

		{
			// Read in the parameter data
			ge.readFromFile(paramfile, binary);

			// Read the protein description
			std::ifstream ifs;
			ifs.open(proteinDescription.c_str());

			if (!ifs) {
				std::cerr <<  "Error in " << program_name << ": cannot open input file " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			if(!conv.Read(&mol, &ifs) || mol.Empty()) {
				std::cerr <<  "Error in " << program_name << ": could not read conformer from " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			// We do not need the file anymore at this point
			ifs.close();

			// Add hydrogens, if requested
			if (addhydrogens) mol.AddHydrogens();

			// Attach the force field to the molecule
			if (!pFF->Setup(mol)) {
				std::cerr << "Error in " << program_name << ": could not setup force field." << std::endl;
				exit (1);
			}

			// construct flat vector of dihedral angles and add the parameters to it
			std::vector<double> vod;
			std::size_t nDoubles = ge.size<double>();
			for(std::size_t i = 0; i<nDoubles; i++) vod.push_back(ge.at<double>(i));

			// Add the data to the molecule
			if (!SetVectorOfDihedrals(&mol,vod)) {
				std::cerr << "Error in " << program_name << " updating the molecule" << std::endl;
				return 1;
			};

			// and write the structure to file
			std::ofstream resultFile;
			if(identifyer != DEFAULTIDENTIFYER) {
				std::ostringstream result;
				result << identifyer << "_result.pdb";
				resultFile.open(result.str().c_str());
			}
			else {
				resultFile.open("result.pdb");
			}
			conv.Write(&mol, &resultFile);
			resultFile.close();

			// ... finally get rid of the molecule
			mol.Clear();
		}

		std:: cout << "... done." << std::endl;
		return 0;
		break;

	//------------------------------------------------------------------------------------------------------
	case 7: // Perform a single, isolated energy calculation for a given molecular configuration
		{
			// Read the protein description
			std::ifstream ifs;
			ifs.open(proteinDescription.c_str());

			if (!ifs) {
				std::cerr <<  "Error in " << program_name << ": cannot open input file " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			if(!conv.Read(&mol, &ifs) || mol.Empty()) {
				std::cerr <<  "Error in " << program_name << ": could not read conformer from " << proteinDescription << std::endl;
				ifs.close();
				exit (1);
			}

			// We do not need the file anymore at this point
			ifs.close();

			// Add hydrogens, if requested
			if (addhydrogens) mol.AddHydrogens();

			// Attach the force field to the molecule
			if (!pFF->Setup(mol)) {
				std::cerr << "Error in " << program_name << ": could not setup force field." << std::endl;
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

		return 0;
		break;

	//------------------------------------------------------------------------------------------------------
	default:
		std::cerr << "Error: Found invalid execution mode" << std::endl;
		exit(1);

	//------------------------------------------------------------------------------------------------------
	};

	return 0; // make the compiler happy
}
