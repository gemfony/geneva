/**
 * @file GParserBuilder.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
			return false;
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
/**
 * Writes out a configuration file. If no argument is given, the configuration file
 * will be written to the stored file name.
 *
 * @param fileName The name of the configuration file to be written
 * @param header A descriptive comment to be output at the top of the configuration file
 */
void GParserBuilder::writeConfigFile(
		const std::string& fileName
		, const std::string& header
) const {
	// Open the required configuration file
	std::string localConfigFile;
	if(fileName == "") {
		localConfigFile = fileName;
	} else {
		localConfigFile = configurationFile_;
	}
	std::ofstream ofs(localConfigFile.c_str());
	if (!ofs.good()) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): Error writing the configuration file " << localConfigFile << std::endl
		);
	}

	// Do some error checking
	if(variables_.size() == 0) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): No variables found!" << std::endl
		);
	}
	if(variables_.size() != defaultValues_.size() || variables_.size() != comments_.size()) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): Invalid vector sizes found: " << variables_.size() << "/" << defaultValues_.size() << "/" << comments_.size() << std::endl
		);
	}

	// Output the header
	ofs << "################################################################" << std::endl
		<< "# " << header << std::endl
		<< "# File creation date: " << boost::posix_time::second_clock::local_time() << std::endl
		<< "################################################################" << std::endl
		<< std::endl;

	// Output variables and values. Note that we output the newest additions first
	for(std::size_t var = variables_.size()-1; var >= 0 ; var--) {
		if(comments_.at(var) != "") { // Only output useful comments
			ofs << "# " << comments_.at(var) << std::endl;
		}
		ofs << "# default value: " << defaultValues_.at(var) << std::endl
			<< variables_.at(var) << " = " << defaultValues_.at(var) << std::endl;
	}

	// Close the file handle
	ofs.close();
}

/**************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
