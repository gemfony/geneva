/**
 * @file GROOTProducerTest.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <cmath>

// Boost headers go here

// Geneva headers go here
#include "common/GROOTProducer.hpp"

int main(int argc, char** argv) {
	Gem::Common::GROOTProducer grp(1,2); // one column of 2 plots
	grp.setCanvasName("sincos");

	for(std::size_t i=0; i<1000; i++) {
		grp & boost::tuple<std::size_t, double>(i, sin(2*M_PI*double(i)/1000.));
	}
	grp.completeSubCanvas("x", "sin(x)", "sinus");

	for(std::size_t i=0; i<1000; i++) {
		grp & boost::tuple<std::size_t, double>(i, cos(2*M_PI*double(i)/1000.));
	}
	grp.completeSubCanvas("x", "cos(x)", "cosinus");

	grp.writeResult("result.C");
}
