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

// Boost header files go here
#include <boost/thread/thread.hpp>

// Geneva header files go here
#include "hap/GRandomT.hpp"
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
	switch(id) {
		case 1:
			for(std::size_t i=0; i<NPROD; i++) {
				producer1_vec.push_back(Gem::Common::tss_ptr<Gem::Hap::GRandom>()->uniform_01<double>());
			}
			break;

		case 2:
			for(std::size_t i=0; i<NPROD; i++) {
				producer2_vec.push_back(Gem::Common::tss_ptr<Gem::Hap::GRandom>()->uniform_01<double>());
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

	for(int i=0; i<NPROD; i++) {
		{
			// Random numbers with an even distribution of
			// double values in the range [0,1[
			double d_even_01 = gr.uniform_01<double>();

			// Note: GRandomBase defines an operator(), hence
			// you could also use gr() to obtain a random number
			// of this type.
		}

		{
			// Random numbers with an even distribution of
			// double values in the range [0.,max[
			double max=10.;
			double d_even_0_max = gr.uniform_real<double>(max);
		}

		{
			// Random numbers with an even distribution of
			// double values in the range [min,max[
			double min=0., max=10.;
			double d_even_min_max = gr.uniform_real<double>(min,max);
		}

		{
			// A normal ("gaussian") distribution of random numbers
			// with mean 0 and sigma 1
			double d_std_gauss = gr.normal_distribution<double>();
		}

		{
			// A normal ("gaussian") distribution of random numbers
			// with mean 0 and sigma "sigma"
			double sigma = 2.;
			double d_gauss_sigma = gr.normal_distribution<double>(sigma);

			// Note: Thanks to the "double" argument you could leave out
			// the <double> here
		}

		{
			// A normal ("gaussian") distribution of random numbers
			// with mean "mean" and sigma "sigma"
			double mean = 1.;
			double sigma = 2.;
			double d_gauss_mean_sigma = gr.normal_distribution<double>(mean, sigma);
		}

		{
			// This function adds two gaussians with sigma "sigma" and a distance
			// "distance" from each other, centered around mean. The idea is to use
			// this function in conjunction with evolutionary strategies, so we avoid
			// searching with the highest likelihood at a location where we already
			// know a good value exists. Rather we want to shift the highest likelihood
			// for probes a bit further away from the candidate solution.
			double mean = 1.;
			double sigma = 2.;
			double distance = 3.;
			double d_bi_gauss = gr.bi_normal_distribution<double>(mean, sigma, distance);
		}

		{
			// This function adds two gaussians with sigmas "sigma1", "sigma2" and a distance
			// of "distance" from each other, centered around mean. The idea is to use
			// this function in conjunction with evolutionary strategies, so we avoid
			// searching with the highest likelihood at a location where we already
			// know a good value exists. Rather we want to shift the highest likelihood
			// for probes a bit further away from the candidate solution.
			double mean = 1.;
			double sigma1 = 2.;
			double sigma2 = 1.;
			double distance = 3.;
			double d_bi_gauss_difsigma = gr.bi_normal_distribution<double>(mean, sigma1, sigma2, distance);
		}

		{
			// This function produces boolean values with a 50% likelihood each for
			// true and false
			bool bool_rnd = gr.uniform_bool();
		}

		{
			// This function returns true with a probability "probability", otherwise false.
			double prob = 0.25; // 25% true values
			bool bool_rnd_weight = gr.weighted_bool(prob);
		}

		{
			// This function produces integer random numbers in the range of [min, max] .
			// Note that max may also be < 0.
			boost::int32_t min = -10, max = 10;
			boost::int32_t int_rand_min_max = gr.uniform_int(min, max);
		}

		{
			// This function produces integer random numbers in the range of [0, max] .
			boost::int32_t max = 10;
			boost::int32_t int_rand_max = gr.uniform_int(max);
		}

		{
			// This function produces integer random numbers in the range of [min, max],
			// using a method optimized for small integers. Note that max may also be < 0.
			boost::int32_t min = -10, max = 10;
			boost::int32_t int_rand_si_min_max = gr.uniform_smallint(min, max);
		}

		{
			// This function produces integer random numbers in the range of [0, max],
			// using a method optimized for small integers.
			boost::int32_t max = 10;
			boost::int32_t int_rand_si_max = gr.uniform_smallint(max);
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
