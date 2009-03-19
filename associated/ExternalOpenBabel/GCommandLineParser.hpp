/**
 * @file GCommandLineParser.hpp
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
 * GNU Affero General Public License for more details.
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

#ifndef GCOMMANDLINEPARSER_HPP_
#define GCOMMANDLINEPARSER_HPP_


// Default settings
const std::string DEFAULTPARAMFILE="unknown";
const unsigned short DEFAULTLOGLEVEL=0;
const bool DEFAULTADDHYDROGENS=false;
const std::string DEFAULTFORCEFIELD="MMFF94s"; //"MMFF94";
const std::string DEFAULTFILENAME="unknown";

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,			  
		      bool& init,
		      bool& finalize,
		      bool& singleEvaluation,
		      std::string& paramfile,
		      bool& templ,
		      bool& result,
		      unsigned short& loglevel,
		      bool& addhydrogens,
		      std::string& forcefield,
		      std::string& filename);

#endif /* GCOMMANDLINEPARSER_HPP_ */
