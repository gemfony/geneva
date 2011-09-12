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
 * A constructor for individual items
 */
GParsableI::GParsableI(
	const std::string& optionNameVar
	, const std::string& defaultValueVar
	, const std::string& commentVar
	, const bool& isEssentialVar
)
	: optionName_(GParsableI::makeVector(optionNameVar))
	, defaultValueStr_(GParsableI::makeVector(defaultValueVar))
	, comment_(GParsableI::makeVector(commentVar))
	, isEssential_(isEssentialVar)
{ /* nothing */ }

/**************************************************************************************/
/**
 * A constructor for vectors
 */
GParsableI::GParsableI(
	const std::vector<std::string>& optionNameVec
	, const std::vector<std::string>& defaultValueVec
	, const std::vector<std::string>& commentVec
	, const bool& isEssentialVar
)
	: optionName_(optionNameVec)
	, defaultValueStr_(defaultValueVec)
	, comment_(commentVec)
	, isEssential_(isEssentialVar)
{ /* nothing */ }

/**************************************************************************************/
/**
 * The destructor
 */
GParsableI::~GParsableI()
{ /* nothing */ }

/**************************************************************************************/
/**
 * Retrieves the option name
 */
std::string GParsableI::optionName(std::size_t pos) const {
	if(optionName_.size() <= pos) {
		raiseException(
			"In GParsableI::optionName(std::size_t): Error!" << std::endl
			<< "Tried to access item at position " << pos << std::endl
			<< "where the size of the vector is " << optionName_.size() << std::endl
		);
	}

	return optionName_.at(pos);
}

/**************************************************************************************/
/**
 * Retrieves a string-representation of the default value
 */
std::string GParsableI::defaultValue(std::size_t pos) const {
	if(defaultValueStr_.size() <= pos) {
		raiseException(
			"In GParsableI::defaultValue(std::size_t): Error!" << std::endl
			<< "Tried to access item at position " << pos << std::endl
			<< "where the size of the vector is " << defaultValueStr_.size() << std::endl
		);
	}

	return defaultValueStr_.at(pos);
}

/**************************************************************************************/
/**
 * Retrieves the comment that was assigned to this variable
 */
std::string GParsableI::comment(std::size_t pos) const {
	if(comment_.size() <= pos) {
		raiseException(
			"In GParsableI::comment_(std::size_t): Error!" << std::endl
			<< "Tried to access item at position " << pos << std::endl
			<< "where the size of the vector is " << comment_.size() << std::endl
		);
	}

	return comment_.at(pos);
}

/**************************************************************************************/
/**
 * Checks whether this is an essential variable
 */
bool GParsableI::isEssential() const {
	return isEssential_;
}

/**************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
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
 * Tries to parse a given configuration file for a set of options. Note that parsing
 * is a one-time effort.
 *
 * @param configFile The name of the configuration file to be parsed
 * @return A boolean indicating whether parsing was successful
 */
bool GParserBuilder::parseConfigFile(const std::string& configFile)
{
    namespace po = boost::program_options;

	bool result = false;

	try {
		// Do some error checking. Also check that the configuration file exists.
		// If not, create a default version
		{
			using namespace boost::filesystem;

			if(exists(configFile) && !is_regular_file(configFile)) {
				raiseException(
					"In GParserBuilder::parseConfigFile(): Error!" << std::endl
					<< configFile << " exists but is no regular file." << std::endl
				);
			}

			// We automatically create a configuration file
			if(!exists(configFile)) {
				std::cerr
					<< "Note: In GParserBuilder::parseConfigFile():" << std::endl
					<< "Configuration file " << configFile << " does not exist." << std::endl
					<< "We will try to create a file with default values for you." << std::endl;

				std::string header = "This configuration file was automatically created by GParserBuilder;";
				this->writeConfigFile(
					configFile
					, header
					, true // writeAll == true
				);
			}
		}

		// Do the actual parsing
		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if (!ifs.good()) {
			std::cerr << "Error opening configuration file " << configFile << std::endl;
			return false;
		}

		po::store(po::parse_config_file(ifs, config_), vm);
		po::notify(vm);

		ifs.close();

		// Execute all call-back functions
		std::vector<boost::shared_ptr<GParsableI> >::iterator it;
		for(it=parameter_proxies_.begin(); it!=parameter_proxies_.end(); ++it) {
			(*it)->executeCallBackFunction();
		}

		result = true;
	}
	catch(const gemfony_error_condition& e) {
		throw e;
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

	// Do some error checking
	{
		using namespace boost::filesystem;

		// Is configFile a directory ?
		if(is_directory(configFile)) {
			raiseException(
				"In GParserBuilder::writeConfigFile(): Error!" << std::endl
				<< configFile << " is a directory." << std::endl
			);
		}

		// We do not allow to overwrite existing files
		if(exists(configFile) && is_regular_file(configFile)) {
			raiseException(
				"In GParserBuilder::writeConfigFile(): Error!" << std::endl
				<< "You have specified an existing file (" << configFile << ")." << std::endl
			);
		}

		// Check that the target path exists and is a directory
		if(!exists(path(configFile).remove_filename()) || !is_directory(path(configFile).remove_filename())) {
			raiseException(
				"In GParserBuilder::writeConfigFile(): Error!" << std::endl
				<< "The target path " << path(configFile).remove_filename() << " does not exist or is no directory."
			);
		}
	}

	// Open the required configuration file
	std::ofstream ofs(configFile.c_str());
	if (!ofs.good()) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): Error writing the configuration file " << configFile << std::endl
		);
	}

	// Do some error checking
	if(parameter_proxies_.size() == 0) {
		raiseException(
			"In GParserBuilder::writeConfigFile(): No variables found!" << std::endl
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
	std::vector<boost::shared_ptr<GParsableI> >::const_iterator cit;
	for(cit=parameter_proxies_.begin(); cit!=parameter_proxies_.end(); ++cit) {
		// Only write out the parameter(s) if they are either essential or it
		// has been requested to write out all parameters regardless
		if(!writeAll && !(*cit)->isEssential()) continue;

		// Output the actual data of this parameter object
		ofs << (*cit)->configData();
	}

	// Close the file handle
	ofs.close();
}

/**************************************************************************************/
/**
 * Provides information on the number of configuration options stored in this class
 *
 * @return The number of configuration options stored in this class
 */
std::size_t GParserBuilder::numberOfOptions() const {
	return parameter_proxies_.size();
}

/**************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
