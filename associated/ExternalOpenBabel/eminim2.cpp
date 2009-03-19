/**
 * @file eminim2.cpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * Based on earlier work by Dr. Ivan Kondov.
 *
 * This file is part of the eminim program. eminim is free software:
 * you can redistribute and/or modify this file under the terms of
 * version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * eminim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
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

// OpenBabel headers go here
#include <openbabel/babelconfig.h>
#include <openbabel/base.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/internalcoord.h>

// Our own headers go here
#include "intcoord.hpp"
#include "GCommandLineParser.hpp"

using namespace OpenBabel;

int main(int argc, char **argv)
{
  std::string program_name = argv[0];
  unsigned short loglevel;
  bool add_hydrogens = false;
  std::string filename, ff;
  OBConversion conv;

  std::string paramfile;
  bool init, finalize, singleEvaluation, templ, result;
  

  bool readOnce = false;
  std::vector<double> vod;

  double energy;

  // Parse the command line
  if(!parseCommandLine(argc, argv,			  
		       init,
		       finalize,
		       singleEvaluation,
		       paramfile,
		       templ,
		       result,
		       loglevel,
		       add_hydrogens,
		       ff,
		       filename))
    { exit(1); }

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
      
      std::vector<double>::iterator it;
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
