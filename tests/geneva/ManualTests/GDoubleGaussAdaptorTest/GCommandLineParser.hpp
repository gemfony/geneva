/**
 * @file GCommandLineParser.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here
#include <boost/program_options.hpp>

#ifndef GCOMMANDLINEPARSER_HPP_
#define GCOMMANDLINEPARSER_HPP_

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Tests {

// Default settings
const double CMD_DEFAULTSIGMA = 0.025;
const double CMD_DEFAULTSIGMASIGMA = 0.1;
const double CMD_DEFAULTMINSIGMA = 0.001;
const double CMD_DEFAULTMAXSIGMA = 1.;
const std::uint32_t CMD_DEFAULTMAXITER = 100000;
const std::string CMD_DEFAULTRESULTFILE = "result.C";
const bool CMD_DEFAULTVERBOSE = true;
const std::uint32_t CMD_DEFAULTADAPTIONTHRESHOLD=1;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
							 double& sigma,
							 double& sigmaSigma,
							 double& minSigma,
							 double& maxSigma,
							 std::uint32_t& adaptionThreshold,
							 std::string& resultFile,
							 std::uint32_t& maxIter,
							 bool& verbose);

} /* namespace Tests */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
