/**
 * @file GParameterPropertyParser.cpp
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

#include "geneva/GParameterPropertyParser.hpp"

// Needed for rules to work. Follows http://boost.2283326.n4.nabble.com/hold-multi-pass-backtracking-swap-compliant-ast-td4664679.html
namespace boost {
namespace spirit {

void swap(Gem::Geneva::parPropSpec<double> &a, Gem::Geneva::parPropSpec<double> &b) {
	a.swap(b);
}

void swap(Gem::Geneva::parPropSpec<float> &a, Gem::Geneva::parPropSpec<float> &b) {
	a.swap(b);
}

void swap(Gem::Geneva::parPropSpec<boost::int32_t> &a, Gem::Geneva::parPropSpec<boost::int32_t> &b) {
	a.swap(b);
}

void swap(Gem::Geneva::parPropSpec<bool> &a, Gem::Geneva::parPropSpec<bool> &b) {
	a.swap(b);
}


} /* namespace spirit */
} /* namespace boost */

namespace Gem {
namespace Geneva {

const std::size_t GPP_DEF_NSTEPS = 100; // The default number of steps for a given parameter

/******************************************************************************/
/**
 * The standard constructor -- assignment of the "raw" paramter property string
 */
GParameterPropertyParser::GParameterPropertyParser(const std::string &rw)
	: raw_(rw), parsed_(false) {
	using boost::spirit::ascii::space;
	using boost::spirit::qi::char_;
	using boost::spirit::qi::phrase_parse;
	using boost::spirit::qi::double_;
	using boost::spirit::qi::float_;
	using boost::spirit::qi::uint_;
	using boost::spirit::qi::int_;
	using boost::spirit::qi::bool_;
	using boost::spirit::qi::lit;
	using boost::spirit::ascii::string;
	using boost::spirit::lexeme;
	using boost::spirit::qi::attr;

	using boost::spirit::qi::raw;
	using boost::spirit::qi::alpha;
	using boost::spirit::qi::alnum;
	using boost::spirit::qi::hold;

	varSpec = +char_("0-9a-zA-Z_,.+-[]");;
	varString = char_("dfibs") > '(' > varSpec > ')';

	identifier = raw[(alpha | '_') >> *(alnum | '_')];

	varReference = (hold[attr(0) >> attr("empty") >> uint_] | hold[attr(1) >> identifier >> '[' >> uint_ >> ']'] |
						 (attr(2) >> identifier >> attr(0)));

	simpleScanParser = uint_;
	doubleStringParser = (hold[varReference >> ',' >> double_ >> ',' >> double_ >> ',' >> uint_] |
								 (varReference >> ',' >> double_ >> ',' >> double_ >> attr(GPP_DEF_NSTEPS)));
	floatStringParser = (hold[varReference >> ',' >> float_ >> ',' >> float_ >> ',' >> uint_] |
								(varReference >> ',' >> float_ >> ',' >> float_ >> attr(GPP_DEF_NSTEPS)));
	intStringParser = (hold[varReference >> ',' >> int_ >> ',' >> int_ >> ',' >> uint_] |
							 (varReference >> ',' >> int_ >> ',' >> int_ >> attr(GPP_DEF_NSTEPS)));
	boolStringParser = (hold[varReference >> ',' >> bool_ >> ',' >> bool_ >> ',' >> uint_] |
							  (varReference >> attr(false) >> attr(true) >> attr(GPP_DEF_NSTEPS)));

	try {
		this->parse();
	} catch (const Gem::Common::gemfony_error_condition &e) {
		glogger
		<< "In GParameterPropertyParser::GParameterPropertyParser(const std::string& raw): Error!" << std::endl
		<< "Caught Geneva exception with message " << std::endl
		<< e.what() << std::endl
		<< "Terminating the application" << std::endl
		<< GTERMINATION;
	}
}

/******************************************************************************/
/**
 * Retrieves the raw parameter description
 */
std::string GParameterPropertyParser::getRawParameterDescription() const {
	return raw_;
}

/******************************************************************************/
/**
 * Allows to check whether parsing has already taken place
 */
bool GParameterPropertyParser::isParsed() const {
	return parsed_;
}

/******************************************************************************/
/**
 * Allows to reset the internal structures and to parse a new parameter string
 */
void GParameterPropertyParser::setNewParameterDescription(std::string raw) {
	raw_ = raw;

	sSpecVec.clear();
	dSpecVec.clear();
	fSpecVec.clear();
	iSpecVec.clear();
	bSpecVec.clear();

	parsed_ = false;

	// Update the information
	this->parse();
}

/******************************************************************************/
/**
 * Initiates parsing of the raw_ string
 */
void GParameterPropertyParser::parse() {
	using boost::spirit::ascii::space;
	using boost::spirit::qi::char_;
	using boost::spirit::qi::phrase_parse;
	using boost::spirit::qi::double_;
	using boost::spirit::qi::float_;
	using boost::spirit::qi::uint_;
	using boost::spirit::qi::int_;
	using boost::spirit::qi::bool_;
	using boost::spirit::qi::lit;
	using boost::spirit::lexeme;
	using boost::spirit::qi::attr;

	using boost::phoenix::push_back;
	using boost::spirit::qi::_1;

	// Do nothing if the string has already been parsed
	if (parsed_) return;

	bool success = false;

	std::string::const_iterator from = raw_.begin();
	std::string::const_iterator to = raw_.end();

	std::vector<std::tuple<char, std::string>> variableDescriptions;

	// Dissect the raw string into sub-strings responsible for individual parameters
	success = phrase_parse(
		from, to, (varString % ','), space, variableDescriptions
	);

	if (!success || from != to) {
		std::string rest(from, to);
		glogger
		<< "In GParameterPropertyParser::parse(): Error[1]!" << std::endl
		<< "Parsing of variable descriptions failed. Unparsed fragement: " << rest << std::endl
		<< GEXCEPTION;
	}

	// Process each individual string
	std::vector<std::tuple<char, std::string>>::iterator it;
	for (it = variableDescriptions.begin(); it != variableDescriptions.end(); ++it) {
		std::string varDescr = std::get<1>(*it);

		from = varDescr.begin();
		to = varDescr.end();

		if ('d' == std::get<0>(*it)) {
			success = phrase_parse(
				from, to, doubleStringParser[push_back(boost::phoenix::ref(dSpecVec), _1)], space
			);
		} else if ('f' == std::get<0>(*it)) {
			success = phrase_parse(
				from, to, floatStringParser[push_back(boost::phoenix::ref(fSpecVec), _1)], space
			);
		} else if ('i' == std::get<0>(*it)) {
			success = phrase_parse(
				from, to, intStringParser[push_back(boost::phoenix::ref(iSpecVec), _1)], space
			);
		} else if ('b' == std::get<0>(*it)) {
			success = phrase_parse(
				from, to, boolStringParser[push_back(boost::phoenix::ref(bSpecVec), _1)], space
			);
		} else if ('s' == std::get<0>(*it)) {
			success = phrase_parse(
				from, to, simpleScanParser[push_back(boost::phoenix::ref(sSpecVec), _1)], space
			);
		} else {
			glogger
			<< "In GParameterPropertyParser::parse(): Error!" << std::endl
			<< "Invalid type specifier: " << std::get<0>(*it) << std::endl
			<< GEXCEPTION;
		}

		if (!success || from != to) {
			std::string rest(from, to);
			glogger
			<< "In GParameterPropertyParser::parse(): Error[2]!" << std::endl
			<< "Parsing of variable descriptions failed. Unparsed fragment: " << rest << std::endl
			<< GEXCEPTION;
		}

		// We only accept a single "simple-scan" entry. Complain, if more than one was found
		if (sSpecVec.size() > 1) {
			glogger
			<< "In GParameterPropertyParser::parse(): Error!" << std::endl
			<< "Found " << sSpecVec.size() << "simple scan entries where a" << std::endl
			<< "maximum of 1 is allowed" << std::endl
			<< GEXCEPTION;
		} else if (sSpecVec.size() == 1) { // If we did find a "simple scan" entry, we will discard the other entries.
			if (!dSpecVec.empty()) {
				glogger
				<< "In GParameterPropertyParser::parse(): Warning!" << std::endl
				<< "You have specified both a simple-scan component and " << std::endl
				<< "scan-components for double variables. These entries" << std::endl
				<< "will be discarded" << std::endl
				<< GWARNING;

				dSpecVec.clear();
			}

			if (!fSpecVec.empty()) {
				glogger
				<< "In GParameterPropertyParser::parse(): Warning!" << std::endl
				<< "You have specified both a simple-scan component and " << std::endl
				<< "scan-components for float variables. These entries" << std::endl
				<< "will be discarded" << std::endl
				<< GWARNING;

				fSpecVec.clear();
			}

			if (!iSpecVec.empty()) {
				glogger
				<< "In GParameterPropertyParser::parse(): Warning!" << std::endl
				<< "You have specified both a simple-scan component and " << std::endl
				<< "scan-components for integer variables. These entries" << std::endl
				<< "will be discarded" << std::endl
				<< GWARNING;

				iSpecVec.clear();
			}

			if (!bSpecVec.empty()) {
				glogger
				<< "In GParameterPropertyParser::parse(): Warning!" << std::endl
				<< "You have specified both a simple-scan component and " << std::endl
				<< "scan-components for boolean variables. These entries" << std::endl
				<< "will be discarded" << std::endl
				<< GWARNING;

				bSpecVec.clear();
			}
		}
	}

	// Prevent further use of this function
	parsed_ = true;
}

/******************************************************************************/
/**
 * Retrieve the number of "simple scan" items
 */
std::size_t GParameterPropertyParser::getNSimpleScanItems() const {
	if (sSpecVec.empty()) {
		return std::size_t(0);
	} else { // Return the data of the first item
#ifdef DEBUG
      if(sSpecVec.size() > 1) {
         glogger
         << "In GParameterPropertyParser::getNSimpleScanItems() const: Error!" << std::endl
         << "Found " << sSpecVec.size() << "simple scan entries where a" << std::endl
         << "maximum of 1 is allowed" << std::endl
         << GEXCEPTION;
      }
#endif

		return (sSpecVec.front()).nItems;
	}
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

