/**
 * @file tribool.cpp
 *
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

/**
 * This example demonstrates the usage of the boost::logic::tribool type.
 *
 * On OpenSuSE 11, the example can be compiled with the command
 *
 * g++ -g -o tribool tribool.cpp
 */

#include <iostream>
#include <boost/logic/tribool.hpp>

main() {
  boost::logic::tribool x=true;
  boost::logic::tribool y=boost::logic::indeterminate;
  boost::logic::tribool z=false;

  std::cout << x << " " << !x << std::endl
            << y << " " << !y << " " << boost::logic::indeterminate << std::endl
	    << z << " " << !z << std::endl;

  if(x==true) std::cout << "x is true" << std::endl;
  else if(boost::logic::indeterminate(x)) std::cout << "x is boost::logic::indeterminate" << std::endl;
  else if(x==false) std::cout << "x is false" << std::endl;

  if(!x==true) std::cout << "!x is true" << std::endl;
  else if(boost::logic::indeterminate(!x)) std::cout << "!x is boost::logic::indeterminate" << std::endl;
  else if(!x==false) std::cout << "!x is false" << std::endl;

  if(y==true) std::cout << "y is true" << std::endl;
  else if(boost::logic::indeterminate(y)) std::cout << "y is boost::logic::indeterminate" << std::endl;
  else if(y==false) std::cout << "y is false" << std::endl;

  if(!y==true) std::cout << "!y is true" << std::endl;
  else if(boost::logic::indeterminate(!y)) std::cout << "!y is boost::logic::indeterminate" << std::endl;
  else if(!y==false) std::cout << "!y is false" << std::endl;

  if(z==true) std::cout << "z is true" << std::endl;
  else if(boost::logic::indeterminate(z)) std::cout << "z is boost::logic::indeterminate" << std::endl;
  else if(z==false) std::cout << "z is false" << std::endl;

  if(!z==true) std::cout << "!z is true" << std::endl;
  else if(boost::logic::indeterminate(!z)) std::cout << "!z is boost::logic::indeterminate" << std::endl;
  else if(!z==false) std::cout << "!z is false" << std::endl;
}
