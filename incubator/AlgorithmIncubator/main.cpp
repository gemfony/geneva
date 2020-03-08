/**
 * @file main.cpp
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
#include <random>

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
using real_generator = srd::mt19937;
using real_dist = std::uniform_real_distribution<>;
using Generator = std::variate_generator<real_generator&, real_dist>;

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
