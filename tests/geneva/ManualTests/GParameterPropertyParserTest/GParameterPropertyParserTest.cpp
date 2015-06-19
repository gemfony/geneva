/**
 * @file GParameterPropertyParserTest.cpp
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
	boost::tuple<std::vector<parPropSpec<double>>::const_iterator, std::vector<parPropSpec<double>>::const_iterator> t_d
		= p.getIterators<double>();
	std::vector<parPropSpec<double>>::const_iterator d_cit = boost::get<0>(t_d);
	std::vector<parPropSpec<double>>::const_iterator d_end = boost::get<1>(t_d);
	for(; d_cit!=d_end; ++d_cit) { // Note: d_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *d_cit << std::endl
		<< std::endl;
	}

	// Retrieve float parameters
	boost::tuple<std::vector<parPropSpec<float>>::const_iterator, std::vector<parPropSpec<float>>::const_iterator> t_f
		= p.getIterators<float>();
	std::vector<parPropSpec<float>>::const_iterator f_cit = boost::get<0>(t_f);
	std::vector<parPropSpec<float>>::const_iterator f_end = boost::get<1>(t_f);
	for(; f_cit!=f_end; ++f_cit) { // Note: f_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *f_cit << std::endl
		<< std::endl;
	}

	// Retrieve integer parameters
	boost::tuple<std::vector<parPropSpec<boost::int32_t>>::const_iterator, std::vector<parPropSpec<boost::int32_t>>::const_iterator> t_i
		= p.getIterators<boost::int32_t>();
	std::vector<parPropSpec<boost::int32_t>>::const_iterator i_cit = boost::get<0>(t_i);
	std::vector<parPropSpec<boost::int32_t>>::const_iterator i_end = boost::get<1>(t_i);
	for(; i_cit!=i_end; ++i_cit) { // Note: i_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *i_cit << std::endl
		<< std::endl;
	}

	// Retrieve boolean parameters
	boost::tuple<std::vector<parPropSpec<bool>>::const_iterator, std::vector<parPropSpec<bool>>::const_iterator> t_b
		= p.getIterators<bool>();
	std::vector<parPropSpec<bool>>::const_iterator b_cit = boost::get<0>(t_b);
	std::vector<parPropSpec<bool>>::const_iterator b_end = boost::get<1>(t_b);
	for(; b_cit!=b_end; ++b_cit) { // Note: b_cit is already set to the begin of the double parameter arrays
		std::cout
		<< *b_cit << std::endl
		<< std::endl;
	}
}
