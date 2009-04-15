/**
 * @file commandLineParser.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 * Copyright (C) 2009 Forschungszentrum Karlsruhe GmbH
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

#ifndef COMMANDLINEPARSER_HPP_
#define COMMANDLINEPARSER_HPP_

// Default settings
const std::string DEFAULTPARAMFILE="unknown";

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
			  bool& init,
			  bool& finalize,
			  std::string& paramfile,
		      bool& templ,
		      bool& result);

#endif /* COMMANDLINEPARSER_HPP_ */
