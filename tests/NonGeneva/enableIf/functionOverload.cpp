/**
 * @file functionOverload.cpp
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
 * This example demonstrates how it is possible to influence which overload
 * of a template function is being used. The example makes use of boost::enable_if .
 *
 * Compile with a command line similar to
 * g++ -I /opt/boost140/include/ -o functionOverload ./functionOverload.cpp
 */

#include <iostream>
#include <string>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

class printer {
public:
	std::string print() const
	{
		return std::string("I am a free type");
	}
};

template <typename free_type>
void print(const free_type& t,
		typename boost::disable_if<boost::is_arithmetic<free_type> >::type* dummy = 0)
{
	std::cout << "Free type prints: " << t.print() << std::endl;
}

template <typename int_type>
void print(const int_type& i,
		typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0)
{
	std::cout << "Integer variable is " << i << std::endl;
}

template <typename fp_type>
void print(const fp_type& f,
		typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0)
{
	std::cout << "Floating point variable is " << f << std::endl;
}

int main() {
	printer p;
	int i = 1;
	double d = 2.;

	print(p);
	print(i);
	print(d);
}
