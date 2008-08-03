/**
 * @file
 */

/**
 * GRandom.cpp
 *
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

#include "GRandom.hpp"

namespace Gem {
namespace Util {

  /**
   * This mutex is used for seeding in GSeed. It is not clear whether
   * boost::date_time is thread safe.
   */
  boost::mutex randomseed_mutex;

  /**
   * This function returns a seed based on the current time.
   *
   * Comments on some boost mailing lists (late 2005) seem to indicate that
   * the date_time library's functions are not re-entrant when using gcc and
   * its libraries. It was not possible to determine whether this is still
   * the case, hence we protect calls to date_time with a mutex.
   *
   * @return A seed based on the current time
   */
  uint32_t GSeed(void){
    boost::mutex::scoped_lock lk(randomseed_mutex);
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    return (uint32_t)t1.time_of_day().total_microseconds();
  }

} /* namespace Util */
} /* namespace Gem */
