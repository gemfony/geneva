/**
 * @file GThreadWrapper.hpp
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"


// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#if BOOST_VERSION < 104200
#include <boost/exception.hpp> // Deprecated as of Boost 1.42
#else
#include <boost/exception/all.hpp>
#endif


#ifndef GTHREADWRAPPER_HPP_
#define GTHREADWRAPPER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GExceptions.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This struct is used to catch errors of worker tasks, submitted to threads. Exceptions that are
 * thrown inside of threads to not travel beyond the thread's boundary. Hence we need to catch them
 * and emit a message. Catching of exceptions is only performed in DEBUG mode.
 */
struct GThreadWrapper {
	/** @brief The standard constructor for this struct */
	GThreadWrapper(boost::function<void()>);
	/** @brief This is the main function that will be executed by the thread */
	void operator()();

private:
	GThreadWrapper(); ///< Intentionally empty and undefined
	boost::function<void()> f_; ///< Holds the actual worker task
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GTHREADWRAPPER_HPP_ */
