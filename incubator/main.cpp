/**
 * @file main.cpp
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

/*
 * This code is meant as an incubator that facilitates "playing" with new
 * optimization algorithms. It uses some of the conventions common in the
 * Geneva optimization library (see http://launchpad.net/geneva), and thus
 * may be useful if you wish to build new algorithms to be integrated into
 * Geneva. It currently only supports double-type parameters. If you want
 * other parameter types, you can "emulate" them -- 0. is false and 1. is
 * true for boolean types, 0.,1.,2. ... would represent integer types.
 *
 * Some code in this file was contributed by Lisa Schaetzle.
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

#include <tuple>
#include <memory>
#include <type_traits>

#include "boost/random.hpp"

#include "incubator.hpp"

/******************************************************************************/
/**
 * Some more syntactic sugar
 */
const int SUCCESS = 0;
const int FAILURE = 1;
const std::size_t MAXITERATIONS = 10;

/******************************************************************************/
/**
 * The main function (who would have guessed ...)
 */
typedef boost::random::mt19937 real_generator;
typedef boost::random::uniform_real_distribution<> real_dist;
typedef boost::variate_generator<real_generator&, real_dist> Generator;

int main() {
	real_generator mt; // From Boosts own random number suite
	real_dist dist(0, 1);
	Generator rng(mt,dist);

	std::vector<double> startValues;
   startValues.push_back(rng());
   startValues.push_back(rng());
   startValues.push_back(rng());
   startValues.push_back(rng());

   std::vector<double> bestResults;

   // Set up our solver
   solver s(PARABOLA);
   //-----------------------------------------------------
   // We start to optimize with the conjugate gradient
   optimizerPlaceHolder cg(startValues, s, MAXITERATIONS);

   // Run the actual optimization
   bestResults = cg.optimize();

   // Output the result
   print(bestResults, "DummyOA:");

   //-----------------------------------------------------
   // We are done -- let the audience know

   std::cout << "done ..." << std::endl;
   return SUCCESS;
}
