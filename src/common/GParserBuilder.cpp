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

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor for individual items
 */
GParsableI::GParsableI(
	const std::string& optionNameVar
	, const std::string& commentVar
)
	: optionName_(GParsableI::makeVector(optionNameVar))
	, comment_(GParsableI::makeVector(commentVar))
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor for vectors
 */
GParsableI::GParsableI(
	const std::vector<std::string>& optionNameVec
	, const std::vector<std::string>& commentVec
)
	: optionName_(optionNameVec)
	, comment_(commentVec)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParsableI::~GParsableI()
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves the option name
 */
std::string GParsableI::optionName(std::size_t pos) const {
	if(optionName_.size() <= pos) {
		glogger
		<< "In GParsableI::optionName(std::size_t): Error!" << std::endl
      << "Tried to access item at position " << pos << std::endl
      << "where the size of the vector is " << optionName_.size() << std::endl
      << GEXCEPTION;
	}

	return optionName_.at(pos);
}

/******************************************************************************/
/**
 * Retrieves the comment that was assigned to this variable
 */
std::string GParsableI::comment(std::size_t pos) const {
	if(comment_.size() <= pos) {
		glogger
		<< "In GParsableI::comment_(std::size_t): Error!" << std::endl
      << "Tried to access item at position " << pos << std::endl
      << "where the size of the vector is " << comment_.size() << std::endl
      << GEXCEPTION;
	}

	return comment_.at(pos);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor for individual items
 */
GFileParsableI::GFileParsableI(
   const std::string& optionNameVar
   , const std::string& commentVar
   , const bool& isEssentialVar
)
   : GParsableI(optionNameVar, commentVar)
   , isEssential_(isEssentialVar)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor for vectors
 */
GFileParsableI::GFileParsableI(
   const std::vector<std::string>& optionNameVec
   , const std::vector<std::string>& commentVec
   , const bool& isEssentialVar
)
   : GParsableI(optionNameVec, commentVec)
   , isEssential_(isEssentialVar)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFileParsableI::~GFileParsableI()
{ /* nothing */ }

/******************************************************************************/
/**
 * Checks whether this is an essential variable
 */
bool GFileParsableI::isEssential() const {
   return isEssential_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor for individual items
 */
GCLParsableI::GCLParsableI(
   const std::string& optionNameVar
   , const std::string& commentVar
)
   : GParsableI(optionNameVar, commentVar)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor for vectors
 */
GCLParsableI::GCLParsableI(
   const std::vector<std::string>& optionNameVec
   , const std::vector<std::string>& commentVec
)
   : GParsableI(optionNameVec, commentVec)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GCLParsableI::~GCLParsableI()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 *
 * @param configurationFile The name of the configuration file
 */
GParserBuilder::GParserBuilder()
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParserBuilder::~GParserBuilder()
{ /* nothing */ }

/******************************************************************************/
/**
 * Tries to parse a given configuration file for a set of options. Note that parsing
 * is a one-time effort.
 *
 * @param configFile The name of the configuration file to be parsed
 * @return A boolean indicating whether parsing was successful
 */
bool GParserBuilder::parseConfigFile(const std::string& configFile) {
	namespace pt = boost::property_tree;
	namespace bf = boost::filesystem;

	bool result = false;
	pt::ptree ptr; // A property tree object;

	try
	{
		// Do some error checking. Also check that the configuration file exists.
		// If not, create a default version
		if(!bf::exists(configFile)) {
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
		} else { // configFile exists
			// Is it a regular file ?
			if(!bf::is_regular_file(configFile)) {
				glogger
				<< "In GParserBuilder::parseConfigFile(): Error!" << std::endl
            << configFile << " exists but is no regular file." << std::endl
            << GEXCEPTION;
			}

			// We require the file to have the json extension
#if (BOOST_VERSION>104300)
			if(!bf::path(configFile).has_extension() || bf::path(configFile).extension() != ".json") {
#else
			if(bf::path(configFile).extension() != ".json") {
#endif
				glogger
				<< "In GParserBuilder::parseConfigFile(): Error!" << std::endl
            << configFile << " does not have the required extension \".json\"" << std::endl
            << GEXCEPTION;
			}
		}

		// Do the actual parsing
		pt::read_json(configFile, ptr);

		// Load the data into our objects and execute the relevant call-back functions
		std::vector<boost::shared_ptr<GFileParsableI> >::iterator it;
		for(it=parameter_proxies_.begin(); it!=parameter_proxies_.end(); ++it) {
			(*it)->load(ptr);
			(*it)->executeCallBackFunction();
		}

		result = true;
	} catch(const gemfony_error_condition& e) {
		std::cerr << "Caught gemfony_error_condition when parsing configuration file " << configFile << ":" << std::endl
				  << e.what() << std::endl;
		result=false;
	} catch(const std::exception& e) {
		std::cerr << "Caught std::exception when parsing configuration file " << configFile << ":" << std::endl
				  << e.what() << std::endl;
		result=false;
	} catch(...) {
		std::cerr << "Unknown error while parsing the configuration file " << configFile << std::endl;
		result=false;
	}

	return result;
}

/******************************************************************************/
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
    using namespace boost::property_tree;
    using namespace boost::filesystem;

    namespace bf = boost::filesystem;

	// Needed for the separation of comment strings
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> semicolon_sep(";");

	// Do some error checking
	{
		// Is configFile a directory ?
		if(is_directory(configFile)) {
			glogger
			<< "In GParserBuilder::writeConfigFile(): Error!" << std::endl
         << configFile << " is a directory." << std::endl
         << GEXCEPTION;
		}

		// We do not allow to overwrite existing files
		if(exists(configFile) && is_regular_file(configFile)) {
		   glogger
		   << "In GParserBuilder::writeConfigFile(): Error!" << std::endl
         << "You have specified an existing file (" << configFile << ")." << std::endl
         << GEXCEPTION;
		}

		// Check that the target path exists and is a directory
		if(!exists(bf::path(configFile).remove_filename()) || !is_directory(bf::path(configFile).remove_filename())) {
		   glogger
		   << "In GParserBuilder::writeConfigFile(): Error!" << std::endl
         << "The target path " << bf::path(configFile).remove_filename() << " does not exist or is no directory." << std::endl
         << GEXCEPTION;
		}

		// Check that the configuration file has the required extension
#if BOOST_VERSION>104300
		if(!bf::path(configFile).has_extension() || bf::path(configFile).extension() != ".json") {
#else
		if(bf::path(configFile).extension() != ".json") { // This is a hack
#endif
		   glogger
		   << "In GParserBuilder::writeConfigFile(): Error!" << std::endl
         << configFile << " does not have the required extension \".json\"" << std::endl
         << GEXCEPTION;
		}
	}

	// Open the required configuration file
	std::ofstream ofs(configFile.c_str());
	if (!ofs.good()) {
		glogger
		<< "In GParserBuilder::writeConfigFile(): Error writing the configuration file " << configFile << std::endl
		<< GEXCEPTION;
	}

	// Do some error checking
	if(parameter_proxies_.size() == 0) {
		glogger
		<< "In GParserBuilder::writeConfigFile(): No variables found!" << std::endl
		<< GEXCEPTION;
	}

	// Output the header
	ofs << "//-----------------------------------------------------------------" << std::endl;
	if(header != "") {
		// Break the header into individual tokens
		tokenizer headerTokenizer(header, semicolon_sep);
		for(tokenizer::iterator h=headerTokenizer.begin(); h!=headerTokenizer.end(); ++h) {
			ofs << "// " << *h << std::endl;
		}
	}
	ofs << "// File creation date: " << boost::posix_time::second_clock::local_time() << std::endl
		<< "//-----------------------------------------------------------------" << std::endl
		<< std::endl;

	// Create a property tree object;
	ptree pt;

	// Output variables and values
	std::vector<boost::shared_ptr<GFileParsableI> >::const_iterator cit;
	for(cit=parameter_proxies_.begin(); cit!=parameter_proxies_.end(); ++cit) {
		// Only write out the parameter(s) if they are either essential or it
		// has been requested to write out all parameters regardless
		if(!writeAll && !(*cit)->isEssential()) continue;

		// Output the actual data of this parameter object to the property tree
		(*cit)->save(pt);
	}

	// Write the configuration data to disk
	write_json(ofs, pt);

	// Close the file handle
	ofs.close();

}

/******************************************************************************/
/**
 * Provides information on the number of configuration options stored in this class
 *
 * @return The number of configuration options stored in this class
 */
std::size_t GParserBuilder::numberOfOptions() const {
	return parameter_proxies_.size();
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
