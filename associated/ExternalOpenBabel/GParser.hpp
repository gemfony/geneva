/**
 * @file GParser.hpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of the eminim program. eminim is free software:
 * you can redistribute and/or modify this file under the terms of
 * version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * eminim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here
#include <boost/program_options.hpp>

// OpenBabel headers go here
#include <openbabel/babelconfig.h>
#include <openbabel/base.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/internalcoord.h>

#ifndef GPARSER_HPP_
#define GPARSER_HPP_


// Default settings
const unsigned short DEFAULTLOGLEVEL=0;
const bool DEFAULTADDHYDROGENS=false;
const std::string DEFAULTFORCEFIELD="MMFF94s"; //"MMFF94";
const std::string DEFAULTPROTEINDESCRIPTION="unknown";

const std::string DEFAULTPARAMFILE="empty";
const boost::uint16_t DEFAULTTRANSFERMODE=0; // binary mode
const std::string DEFAULTCONFIGFILE="./eminim2.cfg"

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
			  boost::uint16_t& executionMode,
			  std::string& paramfile,
			  boost::uint16_t& transferMode,
			  bool& singleEvaluation,
			  std::string& configFile
);

bool parseConfigFile(const std::string& configFile,
		unsigned short& loglevel,
		bool& addhydrogens,
		std::string& forcefield,
		std::string& proteinDescription
);

#endif /* GPARSER_HPP_ */
