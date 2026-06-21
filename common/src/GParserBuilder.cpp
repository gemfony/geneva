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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "common/GParserBuilder.hpp"

// Standard library headers used directly in this translation unit
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <ios>
#include <iostream>
#include <mutex>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// Other Geneva headers whose symbols are used directly
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

// Boost headers used directly in this translation unit
#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace Gem::Common {

/******************************************************************************/
/**
 * Initialization of static data members
 */
std::mutex Gem::Common::GParserBuilder::configfile_parser_mutex_;
bool Gem::Common::GParserBuilder::unknown_key_is_error_ = false;
bool Gem::Common::GParserBuilder::check_unknown_keys_ = true;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The standard constructor of the comment level.
 *
 * @param cl The comment level (index into a parameter's comment vector) this object represents
 */
commentLevel::commentLevel(std::size_t cl)
  : comment_level_(cl) { /* nothing */
}

/******************************************************************************/
/**
 * @brief Retrieves the current comment level.
 *
 * @return The comment level held by this object
 */
std::size_t commentLevel::getCommentLevel() const {
    return comment_level_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief A constructor for individual items.
 *
 * @param option_name_var The single option name for this parsable item
 * @param comment_var The single comment describing this parsable item
 */
GParsableI::GParsableI(std::string const &option_name_var, std::string const &comment_var)
  : option_name_(GParsableI::makeVector(option_name_var))
  , comment_(GParsableI::makeVector(comment_var))
  , cl_(0) { /* nothing */
}

/******************************************************************************/
/**
 * @brief A constructor for vectors of option names and comments.
 *
 * @param option_name_vec The vector of option names for this parsable item
 * @param comment_vec The vector of comments (one per comment level) describing this parsable item
 */
GParsableI::GParsableI(
    std::vector<std::string> const &option_name_vec,
    std::vector<std::string> const &comment_vec
)
  : option_name_(option_name_vec)
  , comment_(comment_vec)
  , cl_(0) { /* nothing */
}

/******************************************************************************/
/**
 * @brief Retrieves the option name at the given position.
 *
 * @param pos The index into the option-name vector (defaults are defined in the header)
 * @return The option name stored at position @p pos
 */
std::string GParsableI::optionName(std::size_t pos) const {
    if(option_name_.size() <= pos) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParsableI::optionName(std::size_t): Error!" << '\n'
            << "Tried to access item at position " << pos << '\n'
            << "where the size of the vector is " << option_name_.size() << '\n'
        );
    }

    return option_name_.at(pos);
}

/******************************************************************************/
/**
 * @brief Retrieves the comment that was assigned to this variable.
 *
 * @param pos The index into the comment vector (i.e. the comment level)
 * @return The comment string stored at position @p pos
 */
std::string GParsableI::comment(std::size_t pos) const {
    if(comment_.size() <= pos) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParsableI::comment_(std::size_t): Error!" << '\n'
            << "Tried to access item at position " << pos << '\n'
            << "where the size of the vector is " << comment_.size() << '\n'
        );
    }

    return comment_.at(pos);
}

/******************************************************************************/
/**
 * @brief Checks whether comments have indeed been registered.
 *
 * @return true if at least one comment is registered, false otherwise
 */
bool GParsableI::hasComments() const {
    return not comment_.empty();
}

/******************************************************************************/
/**
 * @brief Retrieves the number of comments available.
 *
 * @return The number of registered comments (comment levels)
 */
std::size_t GParsableI::numberOfComments() const {
    return comment_.size();
}

/******************************************************************************/
/**
 * @brief Retrieves the number of option names registered for this parameter.
 *
 * @return The number of registered option names
 */
std::size_t GParsableI::numberOfOptionNames() const {
    return option_name_.size();
}

/******************************************************************************/
/**
 * @brief Appends a std::ostream manipulator to the comment at the current level.
 *
 * Needed for ostringstream.
 *
 * @param val A std::ostream manipulator function (e.g. std::endl) to be appended to the current comment
 * @return A reference to this object (to allow chaining)
 */
GParsableI &GParsableI::operator<<(std::ostream &(*val)(std::ostream &)) {
    std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
    oss << val;
    comment_.at(cl_) += oss.str();
    return *this;
}

/******************************************************************************/
/**
 * @brief Appends a std::ios manipulator to the comment at the current level.
 *
 * Needed for ostringstream.
 *
 * @param val A std::ios manipulator function to be appended to the current comment
 * @return A reference to this object (to allow chaining)
 */
GParsableI &GParsableI::operator<<(std::ios &(*val)(std::ios &)) {
    std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
    oss << val;
    comment_.at(cl_) += oss.str();
    return *this;
}

/******************************************************************************/
/**
 * @brief Appends a std::ios_base manipulator to the comment at the current level.
 *
 * Needed for ostringstream.
 *
 * @param val A std::ios_base manipulator function to be appended to the current comment
 * @return A reference to this object (to allow chaining)
 */
GParsableI &GParsableI::operator<<(std::ios_base &(*val)(std::ios_base &)) {
    std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
    oss << val;
    comment_.at(cl_) += oss.str();
    return *this;
}

/******************************************************************************/
/**
 * @brief Allows to indicate the current comment level.
 *
 * Subsequent streamed text is appended to the comment at the selected level.
 *
 * @param cl A commentLevel object carrying the comment level to switch to
 * @return A reference to this object (to allow chaining)
 */
GParsableI &GParsableI::operator<<(commentLevel const &cl) {
#ifdef DEBUG
    if(comment_.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParsableI::operator<< (commentLevel const& cl): Error!" << '\n'
            << "No comments in vector" << '\n'
        );
    }

    if(comment_.size() <= cl.getCommentLevel()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParsableI::operator<< (commentLevel const& cl): Error!" << '\n'
            << "Invalid comment level " << cl.getCommentLevel()
            << " requested, where the maximum is " << comment_.size() - 1 << '\n'
        );
    }
#endif /* DEBUG */

    cl_ = cl.getCommentLevel();
    return *this;
}

/******************************************************************************/
/**
 * @brief Allows to switch to the next comment level.
 *
 * Increments the current comment level by one; subsequent streamed text is
 * appended to the comment at the new level.
 *
 * @param nC A nextComment tag object that triggers the level increment (unused)
 * @return A reference to this object (to allow chaining)
 */
GParsableI &GParsableI::operator<<([[maybe_unused]] nextComment const & nc) {
#ifdef DEBUG
    if(comment_.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParsableI::operator<< (nextComment const& nC): Error!" << '\n'
            << "No comments in vector" << '\n'
        );
    }

    if(comment_.size() <= (cl_ + 1)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParsableI::operator<< (nextComment const& nC): Error!" << '\n'
            << "Invalid comment level " << cl_ + 1 << " requested, where the maximum is "
            << comment_.size() - 1 << '\n'
        );
    }
#endif /* DEBUG */

    // Increment the comment level
    cl_++;
    return *this;
}

/******************************************************************************/
/**
 * @brief Splits a comment into sub-tokens.
 *
 * The comment will be split in case of newlines and semicolons.
 *
 * @param comment The comment string to be split (the literal "empty" and an empty string yield no tokens)
 * @return A vector of the individual sub-tokens obtained from @p comment
 */
std::vector<std::string> GParsableI::splitComment(std::string const &comment) {
    std::vector<std::string> results;

    if(not comment.empty() && comment != "empty") {
        // First split the comment according to newlines
        std::vector<std::string> nl_comments;
        std::istringstream buffer(comment);
        std::string line; // NOLINT(cppcoreguidelines-init-variables)

        // Break the sub-comments into individual lines after each semicolon
        while(std::getline(buffer, line)) {
            for(auto const &t : Gem::Common::splitString(line, ";")) {
                results.push_back(t);
            }

            nl_comments.push_back(line);
        }
    }

    return results;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief A constructor for individual items.
 *
 * @param option_name_var The single option name for this file-parsable item
 * @param comment_var The single comment describing this file-parsable item
 * @param is_essential_var Whether this is an essential parameter (always written to the config file)
 */
GFileParsableI::GFileParsableI(
    std::string const &option_name_var,
    std::string const &comment_var,
    bool is_essential_var
)
  : GParsableI(option_name_var, comment_var)
  , is_essential_(is_essential_var) { /* nothing */
}

/******************************************************************************/
/**
 * @brief A constructor for vectors of option names and comments.
 *
 * @param option_name_vec The vector of option names for this file-parsable item
 * @param comment_vec The vector of comments (one per comment level) describing this file-parsable item
 * @param is_essential_var Whether this is an essential parameter (always written to the config file)
 */
GFileParsableI::GFileParsableI(
    std::vector<std::string> const &option_name_vec,
    std::vector<std::string> const &comment_vec,
    bool is_essential_var
)
  : GParsableI(option_name_vec, comment_vec)
  , is_essential_(is_essential_var) { /* nothing */
}

/******************************************************************************/
/**
 * @brief Checks whether this is an essential variable.
 *
 * @return true if this parameter is essential, false otherwise
 */
bool GFileParsableI::isEssential() const {
    return is_essential_;
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
GCLParsableI::GCLParsableI(std::string const &option_name_var, std::string const &comment_var)
  : GParsableI(option_name_var, comment_var) { /* nothing */
}

/******************************************************************************/
/**
 * A constructor for vectors
 */
GCLParsableI::GCLParsableI(
    std::vector<std::string> const &option_name_vec,
    std::vector<std::string> const &comment_vec
)
  : GParsableI(option_name_vec, comment_vec) { /* nothing */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GParserBuilder::GParserBuilder() {
    auto basename_opt = Gem::Common::environmentVariableAs<std::string>("GENEVA_CONFIG_BASENAME");
    if(basename_opt && !(*basename_opt).empty()) {
        // Read out the directory
        config_base_dir_ = std::filesystem::path(*basename_opt);
    }
}

/******************************************************************************/
/**
 * Applies an already-parsed configuration ptree to the registered options,
 * without touching the file. Runs the optional unknown-key diagnostic. This is
 * the "apply" half of parseConfigFile, separated so that callers (e.g. GFactoryT)
 * can read + parse a config file once and re-apply the cached ptree to many
 * freshly created objects.
 */
void GParserBuilder::loadFromPtree(
    boost::property_tree::ptree const &ptr,
    std::filesystem::path const &config_path,
    bool run_unknown_key_check
) {
    // Diagnostic: detect configuration-file keys that no registered parameter
    // consumes (config/code drift -- a renamed or stale key). Runs only on a
    // genuine file parse (run_unknown_key_check == true): GFactoryT re-applies a
    // cached ptree to every produced object, and the check must not re-run per
    // object.
    if(run_unknown_key_check && check_unknown_keys_) {
        std::set<std::string> known_keys;
        for(auto const &proxy_ptr : file_parameter_proxies_) {
            // Each parameter reports the top-level JSON key it occupies; combined
            // parameters nest their sub-options under a single group label, so the
            // label -- not the sub-option names -- is the valid top-level key.
            known_keys.insert(proxy_ptr->topLevelConfigKey());
        }
        for(auto const &key_value : ptr) {
            if(key_value.first == "header" || known_keys.contains(key_value.first)) {
                continue;
            }
            if(unknown_key_is_error_) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParserBuilder::loadFromPtree(): Error!" << '\n'
                    << "Configuration file " << config_path.string() << '\n'
                    << "contains the unknown key \"" << key_value.first
                    << "\" that no registered parameter consumes." << '\n'
                    << "This usually indicates config/code drift (a renamed or stale key)." << '\n'
                );
            }
                            glogger << "In GParserBuilder::loadFromPtree(): Warning!" << '\n'
                        << "Configuration file " << config_path.string() << '\n'
                        << "contains the unknown key \"" << key_value.first
                        << "\" that no registered parameter consumes; it will be ignored." << '\n'
                        << GLOGGING;
           
        }
    }

    // Load the data into our objects and execute the relevant call-back functions
    for(auto const &proxy_ptr : file_parameter_proxies_) {
        proxy_ptr->load_from(ptr);
        proxy_ptr->executeCallBackFunction();
    }
}

/******************************************************************************/
/**
 * Tries to parse a given configuration file for a set of options.
 *
 * @param config_file The path to the configuration file that should be parsed
 * @param captured An optional pointer to a property tree that, if non-null, receives the parsed options
 * @return A boolean indicating whether parsing was successful
 */
bool GParserBuilder::parseConfigFile(std::filesystem::path const &config_file, boost::property_tree::ptree *captured) {
    // Make sure only one entity is parsed at once. This allows us to
    // concurrently create e.g. optimization algorithms, letting them
    // parse the same config file.
    std::scoped_lock lk(GParserBuilder::configfile_parser_mutex_);

    namespace pt = boost::property_tree;

    pt::ptree
        ptr; // NOLINT(cppcoreguidelines-init-variables) — property tree, holds configuration options

    std::filesystem::path config_path;

    try {
        // Assemble a path object from the config file, possibly adding a base directory
        if(not config_base_dir_.empty()) {
            // Make sure the base directory exists, creating it if necessary. As a missing config
            // file is auto-created below, a missing config directory must be created too, so the
            // mechanism works in a fresh run directory regardless of which caller runs first.
            if(not std::filesystem::exists(config_base_dir_)) {
                std::error_code ec;
                std::filesystem::create_directories(config_base_dir_, ec);
                if(ec) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GParserBuilder::parseConfigFile(): Error!" << '\n'
                        << "Base-directory " << config_base_dir_.string()
                        << " does not exist and could not be created: " << ec.message() << '\n'
                    );
                }
                glogger << "Note: In GParserBuilder::parseConfigFile():" << '\n'
                        << "The configuration directory " << config_base_dir_.string()
                        << " did not exist and was created for you." << '\n'
                        << GLOGGING;
            }
            else if(not std::filesystem::is_directory(config_base_dir_)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParserBuilder::parseConfigFile(): Error!" << '\n'
                    << "Base-directory " << config_base_dir_.string() << " is not a directory"
                    << '\n'
                );
            }

            config_path = config_base_dir_ / config_file;
        }
        else {
            config_path = config_file;
        }

        // Check that the configuration file exists.
        // If not, create a default version
        if(not std::filesystem::exists(config_path)) {
            glogger << "Note: In GParserBuilder::parseConfigFile():" << '\n'
                    << "Configuration file " << config_path.string() << " does not exist."
                    << '\n'
                    << "We will try to create a file with default values for you." << '\n'
                    << GLOGGING;

            std::string header =
                "This configuration file was automatically created by GParserBuilder;";
            this->writeConfigFile(
                config_path,
                header,
                true // write_all == true
            );
        }
        else { // config_file exists
            // Is it a regular file ?
            if(not std::filesystem::is_regular_file(config_path)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParserBuilder::parseConfigFile(): Error!" << '\n'
                    << config_path.string() << " exists but is no regular file." << '\n'
                );
            }

            // We require the file to have the json extension
            if(not std::filesystem::path(config_path).has_extension() ||
               std::filesystem::path(config_path).extension() != ".json") {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParserBuilder::parseConfigFile(): Error!" << '\n'
                    << config_path.string() << " does not have the required extension \".json\""
                    << '\n'
                );
            }
        }

        // Unfortunately boost;::property_tree does unfortunately not accept path-arguments
        Gem::Common::read_json(config_path, ptr);

        // Optionally hand the parsed ptree back to the caller (e.g. GFactoryT) so
        // it can cache it and avoid re-reading + re-parsing the file on every
        // produce() call.
        if(captured != nullptr) {
            *captured = ptr;
        }

        // Apply the parsed values to the registered options. Factored out so a
        // cached ptree can be re-applied without touching the file again.
        this->loadFromPtree(ptr, config_path);

        return true; // Success!
    }
    catch(geneva_exception const &e) {
        glogger << "Caught geneva_exception when parsing configuration file "
                << config_path.string() << ":" << '\n'
                << e.what() << '\n'
                << GLOGGING;
        return false;
    }
    catch(std::exception const &e) {
        glogger << "Caught std::exception when parsing configuration file " << config_path.string()
                << ":" << '\n'
                << e.what() << '\n'
                << GLOGGING;
        return false;
    }
    catch(...) {
        glogger << "Unknown error while parsing the configuration file " << config_path.string()
                << '\n'
                << GLOGGING;
        return false;
    }
}

/******************************************************************************/
/**
 * Writes out a configuration file.
 *
 * @param config_file The name of the configuration file to be written
 * @param header A descriptive comment to be output at the top of the configuration file
 * @param write_all A boolean parameter that indicates whether all or only essential parameters should be written
 */
void GParserBuilder::writeConfigFile(
    std::filesystem::path const &config_file,
    std::string const &header,
    bool write_all
) const {
    namespace pt = boost::property_tree;
    namespace bf = std::filesystem;

    // Do some error checking
    {
        // Is config_file a directory ?
        if(std::filesystem::is_directory(config_file)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::writeConfigFile(): Error!" << '\n'
                << config_file.string() << " is a directory." << '\n'
            );
        }

        // We do not allow to overwrite existing files
        if(std::filesystem::exists(config_file) && std::filesystem::is_regular_file(config_file)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::writeConfigFile(): Error!" << '\n'
                << "You have specified an existing file (" << config_file.string() << ")." << '\n'
            );
        }

        // Make sure the target directory exists, creating it if necessary. We auto-create a
        // missing config FILE here (this is the "does not exist" path of parseConfigFile), so we
        // must also create a missing config DIRECTORY -- otherwise, on a fresh run directory with
        // no config/ folder, writing the very first config (e.g. Go2's own) would fail and abort
        // the whole program via GTERMINATION before any config could be created.
        const std::filesystem::path target_dir = std::filesystem::path(config_file).remove_filename();
        if(not target_dir.empty()) {
            if(std::filesystem::exists(target_dir)) {
                if(not std::filesystem::is_directory(target_dir)) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GParserBuilder::writeConfigFile(): Error!" << '\n'
                        << "The target path " << target_dir.string() << " is not a directory."
                        << '\n'
                    );
                }
            }
            else {
                std::error_code ec;
                std::filesystem::create_directories(target_dir, ec);
                if(ec) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GParserBuilder::writeConfigFile(): Error!" << '\n'
                        << "Could not create the target directory " << target_dir.string() << ": "
                        << ec.message() << '\n'
                    );
                }
                glogger << "Note: In GParserBuilder::writeConfigFile():" << '\n'
                        << "The configuration directory " << target_dir.string()
                        << " did not exist and was created for you." << '\n'
                        << GLOGGING;
            }
        }

        // Check that the configuration file has the required extension
        if(not config_file.has_extension() || config_file.extension() != ".json") {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParserBuilder::writeConfigFile(): Error!" << '\n'
                << config_file.string() << " does not have the required extension \".json\"" << '\n'
            );
        }
    }

    // Open the required configuration file
    std::ofstream ofs(config_file);
    if(not ofs) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParserBuilder::writeConfigFile(): Error writing configuration file "
            << config_file.string() << '\n'
        );
    }

    // Do some error checking
    if(file_parameter_proxies_.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParserBuilder::writeConfigFile(): No variables found!" << '\n'
        );
    }

    // Create a property tree object;
    boost::property_tree::ptree ptr; // NOLINT(cppcoreguidelines-init-variables)

    // Output a header
    if(not header.empty()) {
        // Break the header into individual tokens
        for(auto const &h : Gem::Common::splitString(header, ";")) {
            ptr.add("header.comment", std::string(h).c_str());
        }
    }
    ptr.add("header.comment", Gem::Common::currentTimeAsString());

    // Output variables and values
    for(auto const &v_ptr : file_parameter_proxies_) {
        // Only write out the parameter(s) if they are either essential or it
        // has been requested to write out all parameters regardless
        if(not write_all && not v_ptr->isEssential()) {
            continue;
        }

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
    return file_parameter_proxies_.size();
}

/******************************************************************************/
/**
 * Globally selects whether an unknown configuration-file key is treated as an
 * error (true) or merely a warning (false, the default). Intended to be called
 * once at program startup.
 */
void GParserBuilder::setCheckUnknownKeys(bool enabled) {
    check_unknown_keys_ = enabled;
}

/******************************************************************************/
/**
 * Retrieves whether the unknown-configuration-key diagnostic is enabled
 */
bool GParserBuilder::checkUnknownKeys() {
    return check_unknown_keys_;
}

/******************************************************************************/
/**
 * Globally selects whether an unknown configuration-file key is treated as an
 * error (true) or merely a warning (false, the default). Only takes effect when
 * the check is enabled via setCheckUnknownKeys(true).
 */
void GParserBuilder::setUnknownKeyIsError(bool is_error) {
    unknown_key_is_error_ = is_error;
}

/******************************************************************************/
/**
 * Retrieves whether unknown configuration-file keys are treated as an error
 */
bool GParserBuilder::unknownKeyIsError() {
    return unknown_key_is_error_;
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

    // The options description is needed both for parsing and for the usage message printed on a
    // command-line error, so it is built outside the try block below.
    std::string usage_string = std::string("Usage: ") + argv[0] + " [options]";
    po::options_description desc(usage_string);

    // We always want --help and -h to be available
    desc.add_options()("help,h", "Emit help message");

    // Add further options from the parameter objects
    for(auto const &p_ptr : cl_parameter_proxies_) {
        p_ptr->save_to(desc);
    }

    try {
        // Do the actual parsing
        po::variables_map vm;
        po::store(po::parse_command_line(argc, (const char *const *)argv, desc), vm);
        po::notify(vm);

        // Emit a help message, if necessary and let the caller of this function know
        if(vm.contains("help")) {
            std::cout << desc << '\n';
            result = GCL_HELP_REQUESTED;
        }
        else {
            if(verbose) {
                std::cout << "GParserBuilder::parseCommandLine():" << '\n'
                          << "Working with the following options:" << '\n';
                for(auto const &p_ptr : cl_parameter_proxies_) {
                    std::cout << p_ptr->content() << '\n';
                }
                std::cout << '\n';
            }
        }
    }
    catch(po::error const &e) {
        // A malformed command line is a USER error, not an internal fault. Report it together with
        // the usage and exit cleanly with a non-zero status via LOGEXIT -- rather than
        // std::terminate()-ing via GTERMINATION (which would dump core on a mere CLI typo).
        std::ostringstream usage; // NOLINT(cppcoreguidelines-init-variables)
        usage << desc;
        glogger << "Error on the command line: " << e.what() << '\n'
                << '\n'
                << usage.str()
                << LOGEXIT(EXIT_FAILURE);
    }
    catch(...) {
        // Anything other than a program_options error here is an unexpected INTERNAL fault, so the
        // hard stop is kept.
        glogger << "In GParserBuilder::parseCommandLine(int argc, char **argv):" << '\n'
                << "Unknown error while parsing the command line" << '\n'
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
    return cl_parameter_proxies_.size();
}

/******************************************************************************/

} /* namespace Gem::Common */
