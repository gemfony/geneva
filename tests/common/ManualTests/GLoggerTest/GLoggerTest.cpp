/**
 * @file GLoggerTest.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Standard headers go here
#include <cmath>
#include <iostream>

// Boost headers go here
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

// Geneva headers go here
#include "common/GLogger.hpp"

using namespace Gem::Common;

int main(int argc, char** argv) {
   boost::shared_ptr<GBaseLogTarget> gcl_ptr(new GConsoleLogger());
   boost::shared_ptr<GBaseLogTarget> gfl_ptr(new GFileLogger("./somePathToLogFile.txt"));

   glogger.addLogTarget(gcl_ptr);
   glogger.addLogTarget(gfl_ptr);

   // Normal output to all logging targets
   glogger << "Some information " << 1 << " " << 2 << std::endl << GLOGGING;

   // Warning emitted to all targets
   glogger << "Some information " << 3 << " " << 4 << std::endl << GWARNING;

   // Raising an exception. Note that the data will also be written to
   // a file named GENEVA-EXCEPTION.log
   try {
      glogger << "Some information " << 5 << " " << 6 << std::endl << GEXCEPTION;
   } catch(gemfony_error_condition& e) {
      std::cout
      << "Caught exception with message" << std::endl
      << e << std::endl;
   }

   // Output to a specific file
   glogger(boost::filesystem::path("anotherFile")) << "Some other information " << 7 << " " << 8 << std::endl << GFILE;

   // Output to registered logging targets with a given extension
   glogger(std::string("extension")) << "And yet another information " << 9 << " " << 10 << std::endl << GLOGGING;

   // Output to stdout
   glogger << "std::out-information" << std::endl << GSTDOUT;

   // Output to stderr
   glogger << "std::err information" << std::endl << GSTDERR;
}
