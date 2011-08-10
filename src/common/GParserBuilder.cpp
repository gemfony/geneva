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
 * The default constructor
 *
 * @param configurationFile The name of the configuration file
 */
GParserBuilder::GParserBuilder()
{ /* nothing */ }

/**************************************************************************************/
/**
 * The destructor
 */
GParserBuilder::~GParserBuilder()
{ /* nothing */ }

/**************************************************************************************/
/**
 * Tries to parse a given configuration file for a set of options
 *
 * @param configFile The name of the configuration file to be parsed
 * @return A boolean indicating whether parsing was successful
 */
bool GParserBuilder::parseConfigFile(const std::string& configFile)
{
    namespace po = boost::program_options;

	bool result = false;

	try {
		// Do the actual parsing
		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if (!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configFile << std::endl;
			return false;
		}

		po::store(po::parse_config_file(ifs, config_), vm);
		po::notify(vm);

		ifs.close();

		// Execute all stored call-back functions
		std::vector<boost::shared_ptr<GParsableI> >::iterator it;
		for(it=many_.begin(); it!=many_.end(); ++it) {
			(*it)->executeCallBackFunction();
		}

		result = true;
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the configuration file " << configFile << ":" << std::endl
				  << e.what() << std::endl;
		result=false;
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the configuration file " << configFile << std::endl;
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
 * @param writeAll A boolean parameter that indicates whether all or only essential parameters should be written
 */
void GParserBuilder::writeConfigFile(
		const std::string& configFile
		, const std::string& header
		, bool writeAll
) const {
	// Needed for the separation of comment strings
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> semicolon_sep(";");

	// Open the required configuration file
	std::ofstream ofs(configFile.c_str());
	if (!ofs.good()) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): Error writing the configuration file " << configFile << std::endl
		);
	}

	// Do some error checking
	if(variables_.size() == 0) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): No variables found!" << std::endl
		);
	}
	if(variables_.size() != defaultValues_.size() || variables_.size() != comments_.size() || variables_.size() != isEssential_.size()) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): Invalid vector sizes found: " << variables_.size() << "/" << defaultValues_.size() << "/" << comments_.size() << "/" << isEssential_.size() << std::endl
		);
	}

	// Output the header
	ofs << "################################################################" << std::endl;
	if(header != "") {
		// Break the header into individual tokens
		tokenizer headerTokenizer(header, semicolon_sep);
		for(tokenizer::iterator h=headerTokenizer.begin(); h!=headerTokenizer.end(); ++h) {
			ofs << "# " << *h << std::endl;
		}
	}
	ofs << "# File creation date: " << boost::posix_time::second_clock::local_time() << std::endl
		<< "################################################################" << std::endl
		<< std::endl;

	// Output variables and values
	for(std::size_t var = 0; var < variables_.size(); var++) {
		// Only write out this variable if it is either essential or it
		// has been requested to write out all parameters
		if(!writeAll && !isEssential_.at(var)) continue;

		if(comments_.at(var) != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer sleepTokenizer(comments_.at(var), semicolon_sep);
			for(tokenizer::iterator c=sleepTokenizer.begin(); c!=sleepTokenizer.end(); ++c) {
				ofs << "# " << *c << std::endl;
			}
		}
		ofs << "# default value: " << defaultValues_.at(var) << std::endl
			<< variables_.at(var) << " = " << defaultValues_.at(var) << std::endl
			<< std::endl;
	}

	// Close the file handle
	ofs.close();
}

/**************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
