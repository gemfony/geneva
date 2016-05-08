/**
 * @file GHapUsagePatterns.cpp
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
#include <random>

// Boost header files go here
#include <boost/thread/thread.hpp>

// Geneva header files go here
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "common/GTSSAccessT.hpp"

const int NPROD = 1000;

// All GRandom-related code is in the namespace Gem::Hap
using namespace Gem::Hap;

/***********************************************************************************/
// Containers for the two producer threads
std::vector<double> producer1_vec;
std::vector<double> producer2_vec;

/***********************************************************************************/
/**
 * Test of GRandom-access through thread-specific pointer
 */
void produceNumbers(int id) {
    std::uniform_real_distribution<double> uniform_real_distribution(0.,1.);

	switch(id) {
		case 1:
			for(std::size_t i=0; i<NPROD; i++) {
				producer1_vec.push_back(uniform_real_distribution(GRANDOM_TLS));
			}
			break;

		case 2:
			for(std::size_t i=0; i<NPROD; i++) {
				producer2_vec.push_back(uniform_real_distribution(GRANDOM_TLS));
			}
			break;

		default:
			glogger
			<< "Unkown id " << id << std::endl
			<< GEXCEPTION;
			break;
	}
}

/***********************************************************************************/
/**
 * The main function
 */
int main(int argc, char **argv) {
	// Instantiate a random number generator
	// NOTE: You could use GRandomT<RANDOMPROXY> instead
	GRandom gr;

    std::uniform_real_distribution<double> uniform_real_distribution(0.,1.);
    std::normal_distribution<double> normal_distribution;
    Gem::Hap::bi_normal_distribution<double> bi_normal_distribution(1. /*mean*/, 2. /*sigma1*/, 1. /*sigma2*/, 3. /*distance*/);
    std::bernoulli_distribution uniform_bool; // defaults to a probability of 0.5
    std::bernoulli_distribution weighted_bool(0.25);
    std::uniform_int_distribution<std::int32_t> uniform_int_distribution;

	for(int i=0; i<NPROD; i++) {
		{
			// Random numbers with an even distribution of
			// double values in the range [0,1[
			double d_even_01 = uniform_real_distribution(gr);

			// Note: GRandomBase defines an operator(), hence
			// you could also use gr() to obtain a random number
			// of this type.
		}

        {
            // Random numbers with an even distribution of
            // double values in the range [0.,max[
            double max = 10.;
            double d_even_0_max = uniform_real_distribution(gr, std::uniform_real_distribution<double>::param_type(0., max));
        }

		{
			// Random numbers with an even distribution of
			// double values in the range [min,max[
			double min=-10., max=10.;
            double d_even_min_max = uniform_real_distribution(gr, std::uniform_real_distribution<double>::param_type(min, max));
        }

		{
			// A normal ("gaussian") distribution of random numbers
			// with mean 0 and sigma 1
			double d_std_gauss = normal_distribution(gr);
	  	}

		{
			// A normal ("gaussian") distribution of random numbers
			// with mean "mean" and sigma "sigma"
			double mean = 1.;
			double sigma = 2.;
			double d_gauss_mean_sigma = normal_distribution(gr, std::normal_distribution<double>::param_type(mean, sigma));
		}

		{
			// This function adds two gaussians with sigmas "sigma1", "sigma2" and a distance
			// of "distance" from each other, centered around mean. The idea is to use
			// this function in conjunction with evolutionary strategies, so we avoid
			// searching with the highest likelihood at a location where we already
			// know a good value exists. Rather we want to shift the highest likelihood
			// for probes a bit further away from the candidate solution.
			double d_bi_gauss_difsigma  = bi_normal_distribution(gr);
			double d_bi_gauss_difsigma2 = bi_normal_distribution(gr, bi_normal_distribution.param());
		}

		{
			// This function produces boolean values with a 50% likelihood each for true and false
			bool bool_rnd = uniform_bool(gr);
		}

		{
			// This function returns true with a probability "0.25", otherwise false.
			bool bool_rnd_weight = weighted_bool(gr);
		}

		{
			// This function produces integer random numbers in the range of [min, max] .
			// Note that max may also be < 0.
			std::int32_t min = -10, max = 10;
			std::int32_t int_rand_min_max = uniform_int_distribution(gr, std::uniform_int_distribution<std::int32_t>::param_type(min, max));
		}
	}

	// Try thread specific storage access to GRandomT
	boost::thread t1(produceNumbers, 1);
	boost::thread t2(produceNumbers, 2);
	t1.join();
	t2.join();

	std::cout << "producer1_vec.size() = " << producer1_vec.size() << std::endl;
	std::cout << "producer2_vec.size() = " << producer2_vec.size() << std::endl;

	return 0;
}
