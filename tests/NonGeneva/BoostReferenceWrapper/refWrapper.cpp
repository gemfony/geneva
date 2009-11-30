/**
 * @file refWrapper.cpp
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

/**
 * This example demonstrates how to store and access references to a
 * std::vector<double> in a vector.
 *
 * On OpenSuSE 11, the example can be compiled with the command
 *
 * g++ -g -o refWrapper -I /opt/boost136/include/boost-1_36/
 *                      -L/opt/boost136/lib -lboost_system-gcc43-mt
 *                      -lboost_serialization-gcc43-mt refWrapper.cpp
 *
 * This obviously assumes that the boost libraries have benn installed
 * under /opt/boost136 .
 */

#include <iostream>
#include <vector>

#include <boost/ref.hpp>

main(){
  typedef  std::vector<boost::reference_wrapper<const std::vector<double> > > strangeVector;
  strangeVector testVector;

  std::vector<double> one;
  one.push_back(0);
  one.push_back(1);

  std::vector<double> two = one;
  two.at(0) += 2;
  two.at(1) += 2;

  testVector.push_back(boost::cref(one));
  testVector.push_back(boost::cref(two));

  strangeVector::const_iterator cit;
  for(cit=testVector.begin(); cit!=testVector.end(); ++cit){
    std::vector<double>::const_iterator d_cit;
    const std::vector<double>& vecRef = cit->get();
    for(d_cit=vecRef.begin(); d_cit!=vecRef.end(); ++d_cit){
      std::cout << *d_cit << std::endl;
    }
  }
}
