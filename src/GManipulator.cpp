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

#include "GManipulator.hpp"

namespace GenEvA
{

  /***********************************************************************************/
  /**
   * This constructor takes a function pointer to a function which does the actual logging.
   * It also accepts a parameter indicating the log level. This in turn affects the output
   * of data. If the loglevel provided here is higher than a value specified by the user, the 
   * no data is logged.
   */
  GManipulator::GManipulator(void (*val)(GLogStreamer&, uint16_t), uint16_t level)
    :man(val), severity(level)
  {  /* nothing */ }

  /***********************************************************************************/

} /* namespace GenEvA */

