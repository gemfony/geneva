/**
 * @file GCommandLineParser.hpp
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

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#ifndef GCOMMANDLINEPARSER_HPP_
#define GCOMMANDLINEPARSER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Examples {

// Default settings
const double CMD_DEFAULTSIGMA = 1.;
const double CMD_DEFAULTSIGMASIGMA = 0.001;
const double CMD_DEFAULTMINSIGMA = 0.002;
const double CMD_DEFAULTMAXSIGMA = 4.;
const boost::uint32_t CMD_DEFAULTMAXITER = 100000;
const std::string CMD_DEFAULTRESULTFILE = "result.C";
const bool CMD_DEFAULTVERBOSE = true;
const boost::uint32_t CMD_DEFAULTADAPTIONTHRESHOLD=1;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
		double& sigma,
		double& sigmaSigma,
		double& minSigma,
		double& maxSigma,
		boost::uint32_t& adaptionThreshold,
		std::string& resultFile,
		boost::uint32_t& maxIter,
		bool& verbose);

} /* namespace Examples */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
