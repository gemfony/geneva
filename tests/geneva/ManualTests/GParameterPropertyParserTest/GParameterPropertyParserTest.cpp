/**
 * @file GParameterPropertyParserTest.cpp
 */

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

// Standard header files go here
#include <iostream>
#include <string>

// Boost header files go here
#include "boost/lexical_cast.hpp"

// Geneva header files go here
#include "geneva/GParameterPropertyParser.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	std::string raw = "d(MY_DPAR_01,-10.3,12.8,100), d(0,-10.3,12.8,100), i(SOME_IPAR_17, 0,5, 20), b(SOME_BPAR, false, true, 10), d(MY_DPAR_02[3], -5, 7, 20), f(MY_FPAR_03, -2, 10), b(MY_BPAR2)";
	GParameterPropertyParser p(raw);

	// Retrieve double parameters
	std::tuple<std::vector<parPropSpec<double>>::const_iterator, std::vector<parPropSpec<double>>::const_iterator> t_d
		= p.getIterators<double>();
	std::vector<parPropSpec<double>>::const_iterator d_cit = std::get<0>(t_d);
	std::vector<parPropSpec<double>>::const_iterator d_end = std::get<1>(t_d);
	for(; d_cit!=d_end; ++d_cit) { // Note: d_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *d_cit << std::endl
		<< std::endl;
	}

	// Retrieve float parameters
	std::tuple<std::vector<parPropSpec<float>>::const_iterator, std::vector<parPropSpec<float>>::const_iterator> t_f
		= p.getIterators<float>();
	std::vector<parPropSpec<float>>::const_iterator f_cit = std::get<0>(t_f);
	std::vector<parPropSpec<float>>::const_iterator f_end = std::get<1>(t_f);
	for(; f_cit!=f_end; ++f_cit) { // Note: f_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *f_cit << std::endl
		<< std::endl;
	}

	// Retrieve integer parameters
	std::tuple<std::vector<parPropSpec<std::int32_t>>::const_iterator, std::vector<parPropSpec<std::int32_t>>::const_iterator> t_i
		= p.getIterators<std::int32_t>();
	std::vector<parPropSpec<std::int32_t>>::const_iterator i_cit = std::get<0>(t_i);
	std::vector<parPropSpec<std::int32_t>>::const_iterator i_end = std::get<1>(t_i);
	for(; i_cit!=i_end; ++i_cit) { // Note: i_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *i_cit << std::endl
		<< std::endl;
	}

	// Retrieve boolean parameters
	std::tuple<std::vector<parPropSpec<bool>>::const_iterator, std::vector<parPropSpec<bool>>::const_iterator> t_b
		= p.getIterators<bool>();
	std::vector<parPropSpec<bool>>::const_iterator b_cit = std::get<0>(t_b);
	std::vector<parPropSpec<bool>>::const_iterator b_end = std::get<1>(t_b);
	for(; b_cit!=b_end; ++b_cit) { // Note: b_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *b_cit << std::endl
		<< std::endl;
	}
}
