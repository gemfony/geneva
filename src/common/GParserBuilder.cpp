/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "common/GParserBuilder.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Initialization of static data members
 */
std::mutex Gem::Common::GParserBuilder::m_configfile_parser_mutex;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor of the comment level
 */
commentLevel::commentLevel(std::size_t cl)
	: m_comment_level(cl)
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves the current comment level
 */
std::size_t commentLevel::getCommentLevel() const {
	return m_comment_level;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor for individual items
 */
GParsableI::GParsableI(
	std::string const & optionNameVar
	, std::string const & commentVar
)
	: m_option_name(GParsableI::makeVector(optionNameVar))
	, m_comment(GParsableI::makeVector(commentVar))
	, m_cl(0)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor for vectors
 */
GParsableI::GParsableI(
	std::vector<std::string> const &optionNameVec
	, std::vector<std::string> const & commentVec
)
	: m_option_name(optionNameVec)
	, m_comment(commentVec)
	, m_cl(0)
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves the option name
 */
std::string GParsableI::optionName(std::size_t pos) const {
	if (m_option_name.size() <= pos) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParsableI::optionName(std::size_t): Error!" << std::endl
				<< "Tried to access item at position " << pos << std::endl
				<< "where the size of the vector is " << m_option_name.size() << std::endl
		);
	}

	return m_option_name.at(pos);
}

/******************************************************************************/
/**
 * Retrieves the comment that was assigned to this variable
 */
std::string GParsableI::comment(std::size_t pos) const {
	if (m_comment.size() <= pos) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParsableI::m_comment(std::size_t): Error!" << std::endl
				<< "Tried to access item at position " << pos << std::endl
				<< "where the size of the vector is " << m_comment.size() << std::endl
		);
	}

	return m_comment.at(pos);
}

/******************************************************************************/
/**
 * Checks whether comments have indeed been registered
 */
bool GParsableI::hasComments() const {
	return not m_comment.empty();
}

/******************************************************************************/
/**
 * Retrieves the number of comments available
 */
std::size_t GParsableI::numberOfComments() const {
	return m_comment.size();
}

/******************************************************************************/
/**
 * Needed for ostringstream
 */
GParsableI &GParsableI::operator<<(std::ostream &( *val )(std::ostream &)) {
	std::ostringstream oss;
	oss << val;
	m_comment.at(m_cl) += oss.str();
	return *this;
}

/******************************************************************************/
/**
 * Needed for ostringstream
 */
GParsableI &GParsableI::operator<<(std::ios &( *val )(std::ios &)) {
	std::ostringstream oss;
	oss << val;
	m_comment.at(m_cl) += oss.str();
	return *this;
}

/******************************************************************************/
/**
 *  Needed for ostringstream
 */
GParsableI &GParsableI::operator<<(std::ios_base &( *val )(std::ios_base &)) {
	std::ostringstream oss;
	oss << val;
	m_comment.at(m_cl) += oss.str();
	return *this;
}

/******************************************************************************/
/**
 * Allows to indicate the current comment level
 */
GParsableI &GParsableI::operator<<(commentLevel const& cl) {
#ifdef DEBUG
	if(m_comment.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParsableI::operator<< (commentLevel const& cl): Error!" << std::endl
				<< "No comments in vector" << std::endl
		);
	}

	if(m_comment.size() <= cl.getCommentLevel()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParsableI::operator<< (commentLevel const& cl): Error!" << std::endl
				<< "Invalid comment level " << cl.getCommentLevel() << " requested, where the maximum is " << m_comment.size() - 1 << std::endl
		);
	}
#endif /* DEBUG */

	m_cl = cl.getCommentLevel();
	return *this;
}

/******************************************************************************/
/**
 * Allows to switch to the next comment level
 */
GParsableI &GParsableI::operator<<(nextComment const & nC) {
#ifdef DEBUG
	if(m_comment.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParsableI::operator<< (nextComment const& nC): Error!" << std::endl
				<< "No comments in vector" << std::endl
		);
	}

	if(m_comment.size() <= (m_cl+1)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParsableI::operator<< (nextComment const& nC): Error!" << std::endl
				<< "Invalid comment level " << m_cl+1 << " requested, where the maximum is " << m_comment.size() - 1 << std::endl
		);
	}
#endif /* DEBUG */

	// Increment the comment level
	m_cl++;
	return *this;
}

/******************************************************************************/
/**
 * Splits a comment into sub-tokens. The comment will be split in case of newlines
 * and semicolons.
 */
std::vector<std::string> GParsableI::splitComment(std::string const & comment) const {
	std::vector<std::string> results;

	// Needed for the separation of comment strings
	using tokenizer = boost::tokenizer<boost::char_separator<char>>;
	boost::char_separator<char> semicolon_sep(";");

	if (not comment.empty() && comment != "empty") {
		// First split the comment according to newlines
		std::vector<std::string> nlComments;
		std::istringstream buffer(comment);
		std::string line;

		// Break the sub-comments into individual lines after each semicolon
		while (std::getline(buffer, line)) {
			tokenizer commentTokenizer(line, semicolon_sep);
			for(auto const& t: commentTokenizer) {
				results.push_back(t);
			}

			nlComments.push_back(line);
		}
	}

	return results;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor for individual items
 */
GFileParsableI::GFileParsableI(
	std::string const &optionNameVar
	, std::string const &commentVar
	, bool isEssentialVar
)
	: GParsableI(optionNameVar, commentVar), m_is_essential(isEssentialVar)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor for vectors
 */
GFileParsableI::GFileParsableI(
	std::vector<std::string> const &optionNameVec
	, std::vector<std::string> const &commentVec
	, bool isEssentialVar
)
	: GParsableI(optionNameVec, commentVec), m_is_essential(isEssentialVar)
{ /* nothing */ }

/******************************************************************************/
/**
 * Checks whether this is an essential variable
 */
bool GFileParsableI::isEssential() const {
	return m_is_essential;
}

/******************************************************************************/
/** @brief Executes a stored callbacl function */
void GFileParsableI::executeCallBackFunction() {
	executeCallBackFunction_();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor for individual items
 */
GCLParsableI::GCLParsableI(
	std::string const &optionNameVar
	, std::string const &commentVar
)
	: GParsableI(optionNameVar, commentVar)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor for vectors
 */
GCLParsableI::GCLParsableI(
	std::vector<std::string> const &optionNameVec
	, std::vector<std::string> const &commentVec
)
	: GParsableI(optionNameVec, commentVec)
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GParserBuilder::GParserBuilder()
{
	auto basename_opt = Gem::Common::environmentVariableAs<std::string>("GENEVA_CONFIG_BASENAME");
	if(basename_opt && ! (*basename_opt).empty()) {
		// Read out the directory
		m_config_base_dir = std::filesystem::path(*basename_opt);
	}
}

/******************************************************************************/
/**
 * Tries to parse a given configuration file for a set of options.
 *
 * @param configFile The name of the configuration file to be parsed, possibly including an absolute or relative path
 * @return A boolean indicating whether parsing was successful
 */
bool GParserBuilder::parseConfigFile(std::filesystem::path const & configFile) {
	// Make sure only one entity is parsed at once. This allows us to
	// concurrently create e.g. optimization algorithms, letting them
	// parse the same config file.
	std::unique_lock<std::mutex> lk(GParserBuilder::m_configfile_parser_mutex);

	namespace pt = boost::property_tree;

	pt::ptree ptr; // A property tree object. Will hold the configuration options;

	std::filesystem::path config_path;

	try {
		// Assemble a path object from the config file, possibly adding a base directory
		if(not m_config_base_dir.empty()) {
			// Check that the base directory exists
			if(not std::filesystem::exists(m_config_base_dir)) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GParserBuilder::parseConfigFile(): Error!" << std::endl
						<< "Base-directory " << m_config_base_dir.string() << " does not exist" << std::endl
				);
			}

			config_path = m_config_base_dir / configFile;
		} else {
			config_path = configFile;
		}

		// Check that the configuration file exists.
		// If not, create a default version
		if (not std::filesystem::exists(config_path)) {
			glogger
				<< "Note: In GParserBuilder::parseConfigFile():" << std::endl
				<< "Configuration file " << config_path.string() << " does not exist." << std::endl
				<< "We will try to create a file with default values for you." << std::endl
				<< GLOGGING;

			std::string header = "This configuration file was automatically created by GParserBuilder;";
			this->writeConfigFile(
				config_path
				, header
				, true // writeAll == true
			);
		} else { // configFile exists
			// Is it a regular file ?
			if (not std::filesystem::is_regular_file(config_path)) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GParserBuilder::parseConfigFile(): Error!" << std::endl
						<< config_path.string() << " exists but is no regular file." << std::endl
				);
			}

			// We require the file to have the json extension
			if (not std::filesystem::path(config_path).has_extension() || std::filesystem::path(config_path).extension() != ".json") {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GParserBuilder::parseConfigFile(): Error!" << std::endl
						<< config_path.string() << " does not have the required extension \".json\"" << std::endl
				);
			}
		}

		// Unfortunately boost;::property_tree does unfortunately not accept path-arguments
		Gem::Common::read_json(config_path, ptr);

		// Load the data into our objects and execute the relevant call-back functions
		for(auto const& proxy_ptr: m_file_parameter_proxies) {
			proxy_ptr->load_from(ptr);
			proxy_ptr->executeCallBackFunction();
		}

		return true; // Success!
	} catch (gemfony_exception const & e) {
		glogger
			<< "Caught gemfony_exception when parsing configuration file " << config_path.string() << ":" << std::endl
			<< e.what() << std::endl
			<< GLOGGING;
		return false;
	} catch (std::exception const & e) {
		glogger
			<< "Caught std::exception when parsing configuration file " << config_path.string() << ":" << std::endl
			<< e.what() << std::endl
			<< GLOGGING;
		return false;
	} catch (...) {
		glogger
			<< "Unknown error while parsing the configuration file " << config_path.string() << std::endl
			<< GLOGGING;
		return false;
	}
}

/******************************************************************************/
/**
 * Writes out a configuration file.
 *
 * @param fileName The name of the configuration file to be written
 * @param header A descriptive comment to be output at the top of the configuration file
 * @param writeAll A boolean parameter that indicates whether all or only essential parameters should be written
 */
void GParserBuilder::writeConfigFile(
	std::filesystem::path const & configFile
	, std::string const &header
	, bool writeAll
) const {
	namespace pt = boost::property_tree;
	namespace bf = std::filesystem;

	// Do some error checking
	{
		// Is configFile a directory ?
		if (std::filesystem::is_directory(configFile)) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GParserBuilder::writeConfigFile(): Error!" << std::endl
					<< configFile.string() << " is a directory." << std::endl
			);
		}

		// We do not allow to overwrite existing files
		if (std::filesystem::exists(configFile) && std::filesystem::is_regular_file(configFile)) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GParserBuilder::writeConfigFile(): Error!" << std::endl
					<< "You have specified an existing file (" << configFile.string() << ")." << std::endl
			);
		}

		// Check that the target path exists and is a directory
		if (not std::filesystem::exists(std::filesystem::path(configFile).remove_filename()) ||
			 not std::filesystem::is_directory(std::filesystem::path(configFile).remove_filename())) { // We need to act on a copy
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GParserBuilder::writeConfigFile(): Error!" << std::endl
					<< "The target path " << std::filesystem::path(configFile).remove_filename().string()
					<< " does not exist or is no directory." << std::endl
			);
		}

		// Check that the configuration file has the required extension
		if (not configFile.has_extension() || configFile.extension() != ".json") {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GParserBuilder::writeConfigFile(): Error!" << std::endl
					<< configFile.string() << " does not have the required extension \".json\"" << std::endl
			);
		}
	}

	// Open the required configuration file
	std::ofstream ofs(configFile);
	if (not ofs) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParserBuilder::writeConfigFile(): Error writing configuration file " << configFile.string() << std::endl
		);
	}

	// Do some error checking
	if (m_file_parameter_proxies.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParserBuilder::writeConfigFile(): No variables found!" << std::endl
		);
	}

	// Needed for the separation of comment strings
	using tokenizer = boost::tokenizer<boost::char_separator<char>>;
	boost::char_separator<char> semicolon_sep(";");

	// Create a property tree object;
	boost::property_tree::ptree ptr;

	// Output a header
	if (not header.empty()) {
		// Break the header into individual tokens
		tokenizer headerTokenizer(header, semicolon_sep);
		for(auto const& h: headerTokenizer) {
			ptr.add("header.comment", std::string(h).c_str());
		}
	}
	ptr.add("header.comment", Gem::Common::currentTimeAsString());

	// Output variables and values
	for(auto const& v_ptr: m_file_parameter_proxies) {
		// Only write out the parameter(s) if they are either essential or it
		// has been requested to write out all parameters regardless
		if (not writeAll && not v_ptr->isEssential()) continue;

		// Output the actual data of this parameter object to the property tree
		v_ptr->save_to(ptr);
	}

	// Write the configuration data to disk
	pt::write_json(ofs, ptr);

	// Close the file handle
	ofs.close();
}

/******************************************************************************/
/**
 * Provides information on the number of configuration options stored in this class
 *
 * @return The number of configuration options stored in this class
 */
std::size_t GParserBuilder::numberOfFileOptions() const {
	return m_file_parameter_proxies.size();
}

/******************************************************************************/
/**
 * Parses the command line for options
 *
 * @param argc The argument count
 * @param argv The argument vector
 * @param verbose If set to true, the function will emit information about the parsed parameters
 * @return A boolean indicating whether help was requested (true) or not (false)
 */
bool GParserBuilder::parseCommandLine(int argc, char **argv, bool verbose) {
	namespace po = boost::program_options;

	bool result = GCL_NO_HELP_REQUESTED;

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString);

		// We always want --help and -h to be available
		desc.add_options()("help,h", "Emit help message");

		// Add further options from the parameter objects
		for(auto const& p_ptr: m_cl_parameter_proxies) {
			p_ptr->save_to(desc);
		}

		// Do the actual parsing
		po::variables_map vm;
		po::store(po::parse_command_line(argc, (const char *const *) argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary and let the caller of this function know
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			result = GCL_HELP_REQUESTED;
		} else {
			if (verbose) {
				std::cout
					<< "GParserBuilder::parseCommandLine():" << std::endl
					<< "Working with the following options:" << std::endl;
				for(auto const& p_ptr: m_cl_parameter_proxies) {
					std::cout << p_ptr->content() << std::endl;
				}
				std::cout << std::endl;
			}
		}
	} catch (po::error const & e) {
		glogger
			<< "In GParserBuilder::parseCommandLine(int argc, char **argv):" << std::endl
			<< "Error parsing the command line:" << std::endl
			<< e.what() << std::endl
			<< GTERMINATION;
	} catch (...) {
		glogger
			<< "In GParserBuilder::parseCommandLine(int argc, char **argv):" << std::endl
			<< "Unknown error while parsing the command line" << std::endl
			<< GTERMINATION;
	}

	return result;
}

/******************************************************************************/
/**
 * Provides information on the number of command line configuration options
 * stored in this class
 */
std::size_t GParserBuilder::numberOfCLOptions() const {
	return m_cl_parameter_proxies.size();
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
