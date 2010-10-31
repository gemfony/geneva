/**
 * @file GParserBuilder.hpp
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

#include "common/GParserBuilder.hpp"

namespace Gem {
namespace Common {

/**************************************************************************************/
/**
 * The standard constructor, initializes the name of the configuration file
 *
 * @param configurationFile The name of the configuration file
 */
GParserBuilder::GParserBuilder(const std::string& configurationFile)
	: configurationFile_(configurationFile)
	, config_(configurationFile_.c_str())
{ /* nothing */ }

/**************************************************************************************/
/**
 * Tries to parse a given configuration file for a set of options
 *
 * @return A boolean indicating whether parsing was successful
 */
bool GParserBuilder::parse()
{
    namespace po = boost::program_options;

	bool result = false;

	try {
		po::variables_map vm;
		std::ifstream ifs(configurationFile_.c_str());
		if (!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configurationFile_ << std::endl;
			exit(1);
		}

		po::store(po::parse_config_file(ifs, config_), vm);
		po::notify(vm);

		ifs.close();
		result = true;
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the configuration file " << configurationFile_ << ":" << std::endl
				  << e.what() << std::endl;
		result=false;
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the configuration file " << configurationFile_ << std::endl;
		result=false;
	}

    return result;
}

/**************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
