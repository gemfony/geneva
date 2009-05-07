/**
 * @file rnrLinearCongruential.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * This example demonstrates how to produce random numbers with the
 * Linear Congruential algorithm. See http://en.wikipedia.org/wiki/Linear_congruential_generator
 *
 * Compile with
 * g++ -o rnrLinearCongruential rnrLinearCongruential.cpp
 *
 * Look at the output with root from root.cern.ch, using the command
 * root -l lc.C
 */

#include <iostream>
#include <fstream>
#include <cmath>

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>

// const choices of glibc, according to Wikipedia
const boost::uint32_t rnr_m = 4294697296; // 2^32
const boost::uint32_t rnr_a = 1103515245;
const boost::uint32_t rnr_c = 12345;
const double rnr_max = double(std::numeric_limits<boost::uint32_t>::max());

boost::uint32_t rnr_last=17;

double d_linear_congruential() {
	rnr_last = (rnr_a * rnr_last + rnr_c) % rnr_m;
	return static_cast<double>(rnr_last)/rnr_max;
}

int main(){
	std::ofstream lc("lc.C");

	lc << "{" << std::endl
	<< "  TH1F *h1 = new TH1F(\"h1\",\"h1\",100,-0.1,1.1);" << std::endl;

	for(int i=0; i<200000; i++) {
		lc << "h1->Fill(" << d_linear_congruential() << ");" << std::endl;
	}

	lc << "  h1->Draw();" << std::endl
	<< "}" << std::endl;

	lc.close();

	return 0;
}
