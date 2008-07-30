/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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

#include <fstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>


#ifndef GMANIPULATOR_H_
#define GMANIPULATOR_H_

using namespace std;

#include "GLogTargets.hpp"


namespace GenEvA
{
	
  /***********************************************************************************/
  /** Forward declaration */
  class GLogStreamer;

  /**
   * This class is part of the GenEvA logging architecture, which is built on the
   * concept of streams. Modifiers or "manipulators" (of which the endl construct
   * is a well known example) allow to change the way a stream is processed. E.g., in
   * the iostreams library, it is possible to set a precision for the output of 
   * floating point numbers, and endl inserts a "\n" into the stream. Manipulators
   * are often functions which usually emit a struct, which in turn triggers some
   * functionality. The GenEvA logging mechanism is triggered by GManipulator objects.
   * The streaming architecture of the logging mechanism is modelled after a suggestion
   * by Bjarne Stroustrup in his book "The C++ programming language".
   */
  struct GManipulator{
    /** \brief Constructor that takes a function pointer and the logging level */
    GManipulator(void (*val)(GLogStreamer&, uint16_t), 
		 uint16_t level);
		 
    /** \brief A function pointer, pointing to the logging function */
    void (*man)(GLogStreamer&, uint16_t);
    uint16_t severity; ///< The log level - needed to decide whether any output should be produced
  };

  /***********************************************************************************/

} /* namespace GenEvA */

#endif /*GMANIPULATOR_H_*/
