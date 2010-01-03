/**
 * @file GIntFlipAdaptorT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// Boost headers go here
#include <boost/shared_ptr.hpp>

// These includes hold the actual tests
#include "GAdaptorT_test.cpp"
#include "GEvolutionaryAlgorithm_test.cpp"
#include "GBooleanCollection_test.cpp"
#include "GMultiThreadedEA_test.cpp"
#include "GBoundedDouble_test.cpp"
#include "GBoundedInt32_test.cpp"
#include "GBrokerEA_test.cpp"
#include "GDataExchange_test.cpp"
#include "GDoubleCollection_test.cpp"
#include "GGaussAdaptorT_test.cpp"
#include "GInt32Collection_test.cpp"
#include "GIntFlipAdaptorT_test.cpp"
#include "GNumCollectionT_test.cpp"
#include "GObject_test.cpp"
#include "GParameterSet_test.cpp"
#include "GParameterT_test.cpp"
#include "GParameterTCollectionT_test.cpp"
#include "GRandom_test.cpp"

// test program entry point
test_suite* init_unit_test_suite(int argc, char** argv) {
   framework::master_test_suite().add(new GAdaptorTSuite());
   framework::master_test_suite().add(new GEvolutionaryAlgorithmSuite());
   framework::master_test_suite().add(new GBooleanCollectionSuite());
   framework::master_test_suite().add(new GMultiThreadedEASuite());
   framework::master_test_suite().add(new GBoundedDoubleSuite());
   framework::master_test_suite().add(new GBoundedInt32Suite());
   framework::master_test_suite().add(new GBrokerEASuite());
   framework::master_test_suite().add(new GDataExchangeSuite());
   framework::master_test_suite().add(new GDoubleCollectionSuite());
   framework::master_test_suite().add(new GGaussAdaptorTSuite());
   framework::master_test_suite().add(new GInt32CollectionSuite());
   framework::master_test_suite().add(new GIntFlipAdaptorTSuite());
   framework::master_test_suite().add(new GNumCollectionTSuite());
   framework::master_test_suite().add(new GObjectSuite());
   framework::master_test_suite().add(new GParameterSetSuite());
   framework::master_test_suite().add(new GParameterTSuite());
   framework::master_test_suite().add(new GParameterTCollectionTSuite());

   return 0;
}
