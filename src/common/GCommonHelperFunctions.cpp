/**
 * @file GCommonHelperFunctions.cpp
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

#include "common/GCommonHelperFunctions.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This function retrieves the number of CPU cores on the system (possibly including "virtual cores" such
 * as in the case of hyperthreading), which is regarded as a suitable default number for the amount of threads
 * of a specific kind. The function is thread-safe. When called multiple times, it will read the number of
 * hardware threads from a local cache. A maximum number of threads may be set. When the maxNThreads parameter
 * is set to 0 (the default), the maximum number is unlimited.
 *
 * @param defaultNThreads The default number of threads to be returned if hardware concurrency cannot be determined
 * @return A guess at a suitable number of threads for this architecture, used as a fallback
 */

std::mutex g_hwt_read_mutex;
std::atomic<bool> g_hwt_read(false); // global in this file
std::atomic<unsigned int> g_nHardwareThreads(Gem::Common::DEFAULTNHARDWARETHREADS); // global in this file

unsigned int getNHardwareThreads(
	const unsigned int &defaultNThreads
	, const unsigned int& maxNThreads
) {
	if (!g_hwt_read) {
		std::unique_lock<std::mutex>(g_hwt_read_mutex);
		if (!g_hwt_read) {
			g_hwt_read = true;

			if(maxNThreads > 0) {
				g_nHardwareThreads.store(
					(std::min)(Gem::Common::DEFAULTMAXNHARDWARETHREADS, std::thread::hardware_concurrency())
				);
			} else {
				g_nHardwareThreads.store(std::thread::hardware_concurrency());
			}

			if(g_nHardwareThreads.load() == 0) {
				unsigned int nFallBackThreads = 0;
				if(maxNThreads > 0) {
					nFallBackThreads = (std::min)(Gem::Common::DEFAULTMAXNHARDWARETHREADS, Gem::Common::DEFAULTNHARDWARETHREADS);
				} else {
					nFallBackThreads = Gem::Common::DEFAULTNHARDWARETHREADS;
				}

				glogger
					<< "Could not get information regarding suitable number of threads." << std::endl
					<< "Using the default value  = " << nFallBackThreads << " instead." << std::endl
					<< GWARNING;

				g_nHardwareThreads.store(nFallBackThreads);
			}
		}
	} // exclusive access ends

	return g_nHardwareThreads.load();
}

/******************************************************************************/
/**
 * This function loads textual (ASCII) data from an external file. Note that this function is
 * not currently meant to be used with binary data, although it should work with that as well
 * (at least in theory -- this is untested). There is no check whether the data loaded represents
 * a text.
 *
 * @param p The name of the file to be loaded
 * @return The data contained in the file
 */
std::string loadTextDataFromFile(const boost::filesystem::path &p) {
	// Check that the file exists
	if (!boost::filesystem::exists(p)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In loadTextDataFromFile(): Error!" << std::endl
				<< "Tried to load data from file " << p.string() << std::endl
				<< "which does not exist" << std::endl
		);
	}

	boost::filesystem::ifstream sourceFileStream(p);
	std::string sourceFile(
		std::istreambuf_iterator<char>(sourceFileStream), (std::istreambuf_iterator<char>())
	);
	return sourceFile;
}

/******************************************************************************/
/**
 * Execute an external command, reacting to possible errors.
 *
 * @param command The command to be executed (possibly including errors)
 * @param arguments The list of arguments to be added to the command
 * @param commandOutputFileName The name of a file to which information should be piped
 * @param fullCommand Allows the caller to find out about the full command
 * @return The error code
 */
int runExternalCommand(
	const boost::filesystem::path &program, const std::vector<std::string> &arguments,
	const boost::filesystem::path &commandOutputFileName, std::string &fullCommand
) {
	// Convert slashes to backslashes on Windows
	boost::filesystem::path p_program = program;
	std::string localCommand = (p_program.make_preferred()).string();

	// Add command line arguments
	std::vector<std::string>::const_iterator cit;
	for (cit = arguments.begin(); cit != arguments.end(); ++cit) {
		localCommand += (std::string(" ") + *cit);
	}

	// If requested by the user, we want to send the command to an external file
	if (boost::filesystem::path() != commandOutputFileName) {
		boost::filesystem::path p_commandOutputFileName = commandOutputFileName;
		std::string localcommandOutputFileName = (p_commandOutputFileName.make_preferred()).string();

		localCommand =
			std::string("(") + localCommand + std::string(") > ") + localcommandOutputFileName + std::string(" 2>&1");
	}

	// MOstly for external debugging
#ifdef GEM_COMMON_PRINT_COMMANDLINE
	std::cout << "Executing external command \"" << localCommand << "\" ...";
#endif /* GEM_COMMON_PRINT_COMMANDLINE */

	// Assign the full command (mostly needed for external error-evaluation)
	fullCommand = localCommand;

	// Run the actual command.
	int errorCode = system(localCommand.c_str());

#ifdef GEM_COMMON_PRINT_COMMANDLINE
	std::cout << "... done." << std::endl;
#endif /* GEM_COMMON_PRINT_COMMANDLINE */

	// The error code will be returned as the function valiue
	return errorCode;
}

/******************************************************************************/
/**
 * Returns a string for a given serialization mode
 *
 * @param s The serialization mode which should be translated to a string
 * @return A string for a given serialization mode
 */
std::string serializationModeToString(const serializationMode &s) {
	switch (s) {
		case Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT:
			return std::string("text mode");
			break;
		case Gem::Common::serializationMode::SERIALIZATIONMODE_XML:
			return std::string("XML mode");
			break;
		case Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY:
			return std::string("binary mode");
			break;
	}

	// Make the compiler happy
	return std::string("");
}

/******************************************************************************/
/**
 * Splits a string into a vector of strings, according to a seperator character.
 * Any trailing or leading white spaces are removed from the result strings.
 *
 * @param str The string to be split
 * @param sep The separator character
 * @return A std::vector holding the fragments
 */
std::vector<std::string> splitString(const std::string &str, const char *sep) {
	std::vector<std::string> result;

#ifdef DEBUG
	if(1 != std::string(sep).size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In splitString(): Error!" << std::endl
				<< "Supplied separator \"" << sep << "\" has invalid size " << std::string(sep).size() << std::endl
		);
	}
#endif /* DEBUG */

	using tokenizer = boost::tokenizer<boost::char_separator<char>>;
	boost::char_separator<char> sep_char(sep);
	tokenizer oaTokenizer(str, sep_char);
	for (tokenizer::iterator oa = oaTokenizer.begin(); oa != oaTokenizer.end(); ++oa) {
		std::string frag = *oa;
		boost::trim(frag); // Remove any leading or trailing white spaces
		if (frag.empty()) continue; // Ignore empty strings
		result.push_back(frag);
	}

	return result;
}

/******************************************************************************/
/**
 * Splits a string into a vector of unsigned int, if possible, or throws
 * an exception. The list must at least contain one entry and must be
 * comma-separated.
 */
std::vector<unsigned int> stringToUIntVec(
	const std::string &raw
	, char sep
) {
	using namespace boost::spirit;

	std::vector<unsigned int> result;
	bool success = false;

	std::string::const_iterator from = raw.begin();
	std::string::const_iterator to = raw.end();

	// Do the actual parsing
	success = qi::phrase_parse(
		from, to, (uint_ % sep), qi::space, result
	);

	if (from != to || !success) {
		std::string rest(from, to);
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In stringToUIntVec(const std::string& raw): Error!" << std::endl
				<< "Parsing failed." << std::endl
				<< "Stopped at: \": " << rest << "\"" << std::endl
		);
	}

	return result;
}

/******************************************************************************/
/**
 * Splits a string into a vector of double values, if possible, or throws
 * an exception. The list must at least contain one entry and must be
 * comma-separated.
 */
std::vector<double> stringToDoubleVec(const std::string &raw) {
	using namespace boost::spirit;

	std::vector<double> result;
	bool success = false;

	std::string::const_iterator from = raw.begin();
	std::string::const_iterator to = raw.end();

	// Do the actual parsing
	success = qi::phrase_parse(
		from, to, (double_ % ','), qi::space, result
	);

	if (from != to || !success) {
		std::string rest(from, to);
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In stringToDoubleVec(const std::string& raw): Error!" << std::endl
				<< "Parsing failed." << std::endl
				<< "Stopped at: \": " << rest << "\"" << std::endl
		);
	}

	return result;
}


/******************************************************************************/
/**
 * Splits a string into a vector of unsigned int-tuples, if possible, or
 * throws an exception. The string should have the form "(1,2), (3,4)" etc.
 */
std::vector<std::tuple<unsigned int, unsigned int>> stringToUIntTupleVec(const std::string &raw) {
	using namespace boost::spirit;

	using cit_type = std::string::const_iterator;
	using res_type = std::vector<std::tuple<unsigned int, unsigned int>>;

	std::vector<std::tuple<unsigned int, unsigned int>> result;
	bool success = false;

	std::string::const_iterator from = raw.begin();
	std::string::const_iterator to = raw.end();

	// Do the actual parsing
	success = qi::phrase_parse(
		from
		, to
		, (('(' >> uint_ >> ',' >> uint_ >> ')') % ',')
		, qi::space
		, result
	);

	if (from != to || !success) {
		std::string rest(from, to);
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In stringToUIntTupleVec(const std::string& raw): Error!" << std::endl
				<< "Parsing failed." << std::endl
				<< "Stopped at: \"" << rest << "\"" << std::endl
		);
	}

	return result;
}

/******************************************************************************/
/**
 * Translates a string of the type "00:10:30" into a std::chrono::duration<double>
 * object denoting hours:minutes:seconds
 */
std::chrono::duration<double> duration_from_string(const std::string& duration_string) {
	std::vector<unsigned int> timings = stringToUIntVec(duration_string, ':');
	std::chrono::duration<double> result;

	switch(timings.size()) {
		case 1:
			result=std::chrono::seconds(timings.at(0));
			break;

		case 2:
			result =
				std::chrono::minutes(timings.at(0))
				+ std::chrono::seconds(timings.at(1));
			break;

		case 3:
			result =
				std::chrono::hours(timings.at(0))
				+ std::chrono::minutes(timings.at(1))
				+ std::chrono::seconds(timings.at(2));
			break;

		default:
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In Gem::Common::duration_from_string(\"" << duration_string << "\"): Error!" << std::endl
					<< "Invalid number of fields present: " << timings.size() << std::endl
			);
			break;
	}

	return result;
}

/******************************************************************************/
/**
 * Converts the current time to a string
 */
std::string currentTimeAsString() {
#if BOOST_COMP_GNUC && (BOOST_COMP_GNUC < BOOST_VERSION_NUMBER(5,0,0))
	return std::string("Dummy (g++ < 5.0 does not support put_time)");
#else
	std::ostringstream oss;
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	oss << std::put_time(std::localtime(&now), "%c");
	return oss.str();
#endif
}

/******************************************************************************/
/**
 * Returns the number of milliseconds since 1.1.1970
 */
std::string getMSSince1970() {
	std::chrono::time_point<std::chrono::system_clock> p1; // 1970
	std::chrono::time_point<std::chrono::system_clock> p2 = std::chrono::system_clock::now();
	std::chrono::milliseconds ms_since_1970 = std::chrono::duration_cast<std::chrono::milliseconds>(p2 - p1);
	return std::to_string(ms_since_1970.count()); // Cannot use Gem::Common::to_string here as we do not want to include GCommonHelperFunctionsT.hpp
}

/******************************************************************************/
/**
 * Converts a std::chrono::high_resolution_clock::time_point into an arithmetic number
 */
std::chrono::milliseconds::rep time_point_to_milliseconds(const std::chrono::high_resolution_clock::time_point& val) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(val.time_since_epoch()).count();
}

/******************************************************************************/
/**
 * Converts an arithmetic number into a std::chrono::high_resolution_clock::time_point
 */
std::chrono::high_resolution_clock::time_point milliseconds_to_time_point(const std::chrono::milliseconds::rep& val) {
	return std::chrono::high_resolution_clock::time_point(std::chrono::milliseconds(val));
}

/******************************************************************************/
/**
 * Raise an exception if a given define wasn't set. "F" stands for "function",
 * "D" for "define".
 */
void condnotset(const std::string &F, const std::string &D) {
	std::ostringstream error;
	error
		<< std::endl
		<< "================================================" << std::endl
		<< "In function " << F << " Error!" << std::endl
		<< "Function was called even though " << D << " hasn't been set." << std::endl
		<< "================================================" << std::endl;                               \
   throw(gemfony_exception(error.str()));
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
