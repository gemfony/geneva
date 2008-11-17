/**
 * @file commandLineParser.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the Geneva library.
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

#ifndef COMMANDLINEPARSER_HPP_
#define COMMANDLINEPARSER_HPP_


// Default settings
const std::string DEFAULTPARAMFILE="unknown";
const bool DEFAULTWRITETEMPLATE=false;
const bool DEFAULTVERBOSE=true;


namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
		      std::string& paramfile,
		      bool& writeTemplate,
		      bool& verbose);

#endif /* COMMANDLINEPARSER_HPP_ */
