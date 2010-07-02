/**
 * @file conceptCheck.cpp
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

/*
 * In this example we try to demonstrate how boost::concept_check can be used
 * to limit template classes to a particular type. The example also illustrates
 * how to create a new concept check. Note though that this might need more
 * testing.
 *
 * Compilation can be done similar to
 * g++ -I/where/boos/resides/ -o conceptCheck conceptCheck.cpp
 */

#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/concept_check.hpp>
#include <boost/concept/usage.hpp>
#include <boost/concept/detail/concept_def.hpp>


/*****************************************************************/

namespace boost
{
  BOOST_concept(FloatingPoint, (T))
  {
      BOOST_CONCEPT_USAGE(FloatingPoint)
        {
	  x.error_type_must_be_a_floating_point_type();
        }
   private:
      T x;
  };

  template <> struct FloatingPoint<long double> {};
  template <> struct FloatingPoint<double> {};
  template <> struct FloatingPoint<float> {};
}

/*****************************************************************/

template <class T>
class integerWrapper
{
  BOOST_CONCEPT_ASSERT((boost::Integer<T>));

public:
  integerWrapper() { /* nothing */ }
};

template <class T>
class fpWrapper
{
  BOOST_CONCEPT_ASSERT((boost::FloatingPoint<T>));

public:
  fpWrapper() { /* nothing */ }
};

main() {
  integerWrapper<boost::int32_t> iW;
  fpWrapper<double> fW_double;
  fpWrapper<float> fW_float;
  // fpWrapper<char> fW_char; // fails to compile, just as it should
  // fpWrapper<boost::int32_t> fW_int32; // fails to compile, just as it should
}
