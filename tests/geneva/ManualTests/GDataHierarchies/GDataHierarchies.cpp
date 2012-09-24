/**
 * @file GDataHierarchies.cpp
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

// Standard header files go here
#include <iostream>
/*
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
*/

// Boost header files go here
// #include "boost/lexical_cast.hpp"

// Geneva header files go here
// #include "common/GPlotDesigner.hpp"
// #include "geneva-individuals/GTestIndividual3.hpp"

#include "geneva/GConstrainedDoubleObject.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;

// using namespace Gem::Tests;
// using namespace Gem::Geneva;

int main(int argc, char **argv) {
   const double TESTVAL = 0.4;
   boost::shared_ptr<GConstrainedDoubleObject> p_test(new GConstrainedDoubleObject(TESTVAL, 0.3, 0.6));
   std::cout
   << p_test->value() << " / " << TESTVAL << std::endl
   << p_test->value() << " / " << TESTVAL << std::endl;

   /*
   boost::shared_ptr<GTestIndividual3> p = TFactory_GUnitTests<GTestIndividual3>()
                                , p_load = TFactory_GUnitTests<GTestIndividual3>();
   p_load->GObject::load(p);
   */
}
