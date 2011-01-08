/**
 * @file GCommandLineParser.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#ifndef GCOMMANDLINEPARSER_HPP_
#define GCOMMANDLINEPARSER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

namespace Gem
{
namespace Tests
{

// Default settings
const std::size_t DEFAULTNENTRIES=20000;
const boost::uint16_t DEFAULTNPRODUCERTHREADS=10;
const bool DEFAULTVERBOSE=true;
const boost::uint16_t DEFAULTRNRPRODUCTIONMODE=0;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
					  std::size_t& nEntries,
					  boost::uint16_t& nProducerThreads,
					  boost::uint16_t& rnrProductionMode,
					  bool& verbose);

} /* namespace Tests */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
