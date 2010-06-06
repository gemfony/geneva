/**
 * @file vectorInsertion.cpp
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
 * This example demonstrates how to insert an entry into a std::vector
 *
 * Compile with a command line similar to
 * g++ -o vectorInsertion ./vectorInsertion.cpp
 */

#include <iostream>
#include <vector>

int main() {
  std::vector<int> v;

  for(std::size_t i=0; i<5; i++)  v.push_back(i);
  v.push_back(10);
  for(std::size_t i=5; i<10; i++) v.insert(v.begin()+i, i);

  // The following should count from 0 to 10
  for(std::size_t i=0; i<v.size(); i++) std::cout << v[i] << std::endl;
}
