/**
 * @file GSingletonT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <memory>

// Boost headers go here

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GSINGLETONT_HPP_
#define GSINGLETONT_HPP_

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This function creates a new T object. It can be specialized by the object
 * used inside of Gemfony's singleton, e.g. in case they do not have a default
 * constructor or wish to perform special actions prior to initialization.
 *
 * @return A std::shared_ptr to a newly created T object
 */
template <typename T>
std::shared_ptr<T> TFactory_GSingletonT() {
	return std::shared_ptr<T>(new T());
}

/******************************************************************************/
/**
 * This class implements a singleton pattern, augmented so that it returns
 * a std::shared_ptr. This allows other singletons to store a shared_ptr to
 * T, so that it only gets destroyed once it is no longer needed. Note that
 * the static shared_ptr may long have vanished at that time.
 */
template<typename T>
class GSingletonT
	:boost::noncopyable
{
public:
   typedef T STYPE;

	/***************************************************************************/
	/**
	 * If called for the first time, the function creates a std::shared_ptr
	 * of T and returns it to the caller. Subsequent calls to this function
	 * will return the stored copy of the shared_ptr. Other classes can store
	 * the pointer, so that T doesn't get deleted while it is still needed.
	 *
	 * @param mode Determines the mode in which this function is called
	 */
	static std::shared_ptr<T> Instance(const std::size_t& mode) {
		static std::shared_ptr<T> p;
		static boost::mutex creation_mutex;

		switch(mode) {
		case 0:
			// Several callers can reach the next line simultaneously. Hence, if
			// p is empty, we need to ask again if it is empty after we have acquired the lock
			if(!p) {
				// Prevent concurrent "first" access
				boost::mutex::scoped_lock lk(creation_mutex);
				if(!p) p = Gem::Common::TFactory_GSingletonT<T>();
			}

			return p;
			break;

		case 1:
			p.reset();
			break;
		}

		return std::shared_ptr<T>(); // Make the compiler happy
	}

	/***************************************************************************/

private:
	GSingletonT(); ///< Intentionally left undefined
	~GSingletonT(); ///< Intentionally left undefined
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GSINGLETONT_HPP_ */
