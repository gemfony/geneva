/**
 * @file GCanvasTest.hpp
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

#include <iostream>

#include "common/GCanvas.hpp"

using namespace Gem::Common;

int main(int argc, char**argv) {
	GCanvas<8> gc, gc2;

	// Check loading of the data from file
	gc.loadFromFile(bf::path("./pictures/ml.ppm"));
	gc2 = gc;

	// Check that there is no difference between both
	if(gc2.diff(gc) != 0.) {
		std::cout << "Error: Found difference between objects: " << gc2.diff(gc) << std::endl;
		return 1;
	}

	// Set up a red, semi-transparent triangle ...
	t_circle t_c;
	t_c.middle = coord2D(0.5f,0.5f);
	t_c.radius = 0.1f;
	t_c.angle1 = float(0.0);
	t_c.angle2 = float(0.25);
	t_c.angle3 = float(0.5);
	t_c.r = 1.f;
	t_c.g = 0.f;
	t_c.b = 0.f;
	t_c.a = 0.5f;

	// ... and add it to the canvas
	gc.addTriangle(t_c);

	// Check that there is a difference between gc2 and gc
	if(gc2.diff(gc) <= 0.) {
		std::cout << "Error: Incorrect difference between objects: " << gc2.diff(gc) << std::endl;
		exit(1);
	}

	// Finally save the picture to disk
	gc.toFile(bf::path("./pictures/result.ppm"));

	return 0;
}
