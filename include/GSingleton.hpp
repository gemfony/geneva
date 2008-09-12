/**
 * @file GSingleton.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Standard headers go here

// Boost headers go here

#include <boost/version.hpp>

// We require at least Boost version 1.36 because of the usage of system_time
// in the boost::thread code below
#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GSINGLETON_HPP_
#define GSINGLETON_HPP_

namespace Gem {
namespace Util {

/************************************************************************/
/**
 * This class implements the singleton patter.
 */
template<typename T>
class GSingleton
	:boost::noncopyable
{
public:
	/*******************************************************************/
	/**
	 * The default constructor
	 */
	GSingleton()
		:first_(true)
	{ /* nothing */	}

	/*******************************************************************/
	/**
	 * The standard destructor
	 */
	~GSingleton()
	{ /* nothing */	}

	/*******************************************************************/
	/**
	 * If called for the first time, the function creates a boost::shared_ptr
	 * of T and returns it to the caller. Subsequent calls to this function
	 * will return a stored copy of the shared_ptr. Other singletons can store
	 * the pointer, so that T doesn't get deleted while it is still needed.
	 */
	boost::shared_ptr<T> getInstance() {
		if(first_) {
			boost::shared_ptr<T> p(new T());
			p_=p;

			first_=false;
		}

		return p_;
	}

	/*******************************************************************/

private:
	boost::shared_ptr<T> p_;
	bool first_;
};

/************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GSINGLETON_HPP_ */
