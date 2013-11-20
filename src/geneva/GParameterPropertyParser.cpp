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

namespace Gem {
namespace Geneva {

const std::size_t GPP_DEF_NSTEPS = 100; // The default number of steps for a given parameter

/******************************************************************************/
/**
 * The standard constructor -- assignment of the "raw" paramter property string
 */
GParameterPropertyParser::GParameterPropertyParser(const std::string& rw)
   : raw_(rw)
   , parsed_(false)
{
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

   varSpec   = +char_("0-9a-zA-Z_,.+-[]");;
   varString = char_("dfib") > '(' > varSpec > ')';

   identifier = raw[(alpha | '_') >> *(alnum | '_')];

   varReference = ( hold[attr(0) >> attr("empty") >> uint_] | hold[attr(1) >> identifier >> '[' >> uint_ >> ']'] | (attr(2) >> identifier >> attr(0)) );

   doubleStringParser = (hold[varReference >> ',' >> double_ >> ',' >> double_ >> ',' >> uint_] | (varReference >> ',' >> double_ >> ',' >> double_ >> attr(GPP_DEF_NSTEPS)));
   floatStringParser  = (hold[varReference >> ',' >> float_  >> ',' >> float_  >> ',' >> uint_] | (varReference >> ',' >> float_  >> ',' >> float_  >> attr(GPP_DEF_NSTEPS)));
   intStringParser    = (hold[varReference >> ',' >> int_    >> ',' >> int_    >> ',' >> uint_] | (varReference >> ',' >> int_    >> ',' >> int_    >> attr(GPP_DEF_NSTEPS)));
   boolStringParser   = (hold[varReference >> ',' >> bool_   >> ',' >> bool_   >> ',' >> uint_] | (varReference >> attr(false) >> attr(true) >> attr(GPP_DEF_NSTEPS)));

   try {
      this->parse();
   } catch (const Gem::Common::gemfony_error_condition& e) {
      glogger
      << "In GParameterPropertyParser::GParameterPropertyParser(const std::string& raw): Error!" << std::endl
      <<"Caught Geneva exception with message " << std::endl
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
   if(parsed_) return;

   bool success = false;

   std::string::const_iterator from = raw_.begin();
   std::string::const_iterator to   = raw_.end();

   std::vector<boost::tuple<char, std::string> > variableDescriptions;

   // Dissect the raw string into sub-strings responsible for individual parameters
   success = phrase_parse(
         from, to
         , (varString % ',')
         , space
         , variableDescriptions
   );

   if(!success || from != to) {
      std::string rest(from, to);
      glogger
      << "In GParameterPropertyParser::parse(): Error[1]!" << std::endl
      << "Parsing of variable descriptions failed. Unparsed fragement: " << rest << std::endl
      << GEXCEPTION;
   }

   // Process each individual string
   std::vector<boost::tuple<char, std::string> >::iterator it;
   for(it=variableDescriptions.begin(); it!=variableDescriptions.end(); ++it) {
      std::string varDescr = boost::get<1>(*it);

      from = varDescr.begin();
      to   = varDescr.end();

      if('d' == boost::get<0>(*it)) {
         success = phrase_parse(
               from, to
               , doubleStringParser[push_back(boost::phoenix::ref(dSpecVec), _1)]
               , space
         );
      } else if('f' == boost::get<0>(*it)) {
         success = phrase_parse(
               from, to
               , floatStringParser[push_back(boost::phoenix::ref(fSpecVec), _1)]
               , space
         );
      } else if('i' == boost::get<0>(*it)) {
         success = phrase_parse(
               from, to
               , intStringParser[push_back(boost::phoenix::ref(iSpecVec), _1)]
               , space
         );
      } else if('b' == boost::get<0>(*it)) {
         success = phrase_parse(
               from, to
               , boolStringParser[push_back(boost::phoenix::ref(bSpecVec), _1)]
               , space
         );
      } else {
         glogger
         << "In GParameterPropertyParser::parse(): Error!" << std::endl
         << "Invalid type specifier: " << boost::get<0>(*it) << std::endl
         << GEXCEPTION;
      }

      if(!success || from != to) {
         std::string rest(from, to);
         glogger
         << "In GParameterPropertyParser::parse(): Error[2]!" << std::endl
         << "Parsing of variable descriptions failed. Unparsed fragement: " << rest << std::endl
         << GEXCEPTION;
      }
   }

   // Prevent further use of this function
   parsed_ = true;
}

/******************************************************************************/
/**
 * This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for double parameters.
 */
template <>
boost::tuple<std::vector<parPropSpec<double> >::const_iterator, std::vector<parPropSpec<double> >::const_iterator> GParameterPropertyParser::getIterators<double>() const {
   // Make sure parsing has happened.
   if(!parsed_) {
      glogger
      << "In GParameterPropertyParser::getIterators<double>(): Error!" << std::endl
      << "Tried to retrieve iterators when parsing hasn't happened yet" << std::endl
      << GEXCEPTION;
   }


   std::vector<parPropSpec<double> >::const_iterator runner_it = dSpecVec.begin();
   std::vector<parPropSpec<double> >::const_iterator end_it    = dSpecVec.end();

   return boost::tuple<std::vector<parPropSpec<double> >::const_iterator, std::vector<parPropSpec<double> >::const_iterator>(runner_it, end_it);
}

/******************************************************************************/
/**
 * This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for float parameters.
 */
template <>
boost::tuple<std::vector<parPropSpec<float> >::const_iterator, std::vector<parPropSpec<float> >::const_iterator> GParameterPropertyParser::getIterators<float>() const {
   // Make sure parsing has happened.
   if(!parsed_) {
      glogger
      << "In GParameterPropertyParser::getIterators<float>(): Error!" << std::endl
      << "Tried to retrieve iterators when parsing hasn't happened yet" << std::endl
      << GEXCEPTION;
   }


   std::vector<parPropSpec<float> >::const_iterator runner_it = fSpecVec.begin();
   std::vector<parPropSpec<float> >::const_iterator end_it    = fSpecVec.end();

   return boost::tuple<std::vector<parPropSpec<float> >::const_iterator, std::vector<parPropSpec<float> >::const_iterator>(runner_it, end_it);
}

/******************************************************************************/
/**
 * This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for boost::int32_t parameters.
 */
template <>
boost::tuple<std::vector<parPropSpec<boost::int32_t> >::const_iterator, std::vector<parPropSpec<boost::int32_t> >::const_iterator> GParameterPropertyParser::getIterators<boost::int32_t>() const {
   // Make sure parsing has happened.
   if(!parsed_) {
      glogger
      << "In GParameterPropertyParser::getIterators<boost::int32_t>(): Error!" << std::endl
      << "Tried to retrieve iterators when parsing hasn't happened yet" << std::endl
      << GEXCEPTION;
   }

   std::vector<parPropSpec<boost::int32_t> >::const_iterator runner_it = iSpecVec.begin();
   std::vector<parPropSpec<boost::int32_t> >::const_iterator end_it    = iSpecVec.end();

   return boost::tuple<std::vector<parPropSpec<boost::int32_t> >::const_iterator, std::vector<parPropSpec<boost::int32_t> >::const_iterator>(runner_it, end_it);
}

/******************************************************************************/
/**
 * This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for bool parameters.
 */
template <>
boost::tuple<std::vector<parPropSpec<bool> >::const_iterator, std::vector<parPropSpec<bool> >::const_iterator> GParameterPropertyParser::getIterators<bool>() const {
   // Make sure parsing has happened.
   if(!parsed_) {
      glogger
      << "In GParameterPropertyParser::getIterators<bool>(): Error!" << std::endl
      << "Tried to retrieve iterators when parsing hasn't happened yet" << std::endl
      << GEXCEPTION;
   }

   std::vector<parPropSpec<bool> >::const_iterator runner_it = bSpecVec.begin();
   std::vector<parPropSpec<bool> >::const_iterator end_it    = bSpecVec.end();

   return boost::tuple<std::vector<parPropSpec<bool> >::const_iterator, std::vector<parPropSpec<bool> >::const_iterator>(runner_it, end_it);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

