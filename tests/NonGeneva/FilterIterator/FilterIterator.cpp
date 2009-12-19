/** @file FilterIterator.cpp */

// This example is based on code by Jeremy Siek, as shipped
// together with the Boost.Iterator library.
// The original code contained the following statement:
// ---------------------------------------------------------------
// (C) Copyright Jeremy Siek 1999-2004.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------
//
// Just like Jeremy Siek's original code, this code is covered
// by the Boost Software License, Version 1.0.
//
// Please note that:
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Modifications to Jeremy Siek's code by: Ruediger Berlich
//
// This example demonstrates how it is possible to filter out certain
// objects from a vector of these objects, using the filter iterator
// from the Boost.Iterators library.
//
// Compilation should be possible with a command line similar to:
// g++ -o FilterIterator -I/opt/boost141/include/ FilterIterator.cpp

#include <boost/config.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/cstdlib.hpp> // for exit_success
#include <boost/shared_ptr.hpp>

class numberContainer {
public:
  numberContainer(int i) {
    secret_ = i;
  }

  int getSecret() const {
    return secret_;
  }

private:
  int secret_;
};

struct is_positive_number {
  bool operator()(boost::shared_ptr<numberContainer> x) { return (x->getSecret() > 0); }
};

int main()
{
  std::vector<boost::shared_ptr<numberContainer> > ncvec;

  for(int i=-10; i<10; i++) {
    ncvec.push_back(boost::shared_ptr<numberContainer>(new numberContainer(i)));
  }

  typedef std::vector<boost::shared_ptr<numberContainer> >::iterator base_iterator;
  base_iterator numbers = ncvec.begin();

  typedef boost::filter_iterator<is_positive_number, base_iterator>  FilterIter;

  is_positive_number predicate;
  FilterIter it;
  FilterIter filter_iter_first(predicate, numbers, numbers + 20);
  FilterIter filter_iter_last(predicate, numbers + 20, numbers + 20);

  for(it=filter_iter_first; it!=filter_iter_last; ++it) {
    std::cout << (*it)->getSecret() << std::endl;
  }

  return boost::exit_success;
}
