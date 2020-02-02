/**
 * @file GCanvasTest.hpp
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
