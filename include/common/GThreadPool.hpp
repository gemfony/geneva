/**
 * @file GThreadPool.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard header files go here

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#ifndef GTHREADPOOL_HPP_
#define GTHREADPOOL_HPP_


// Geneva header files go here
#include "common/GThreadGroup.hpp"
#include "common/GHelperFunctions.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class implements a simple thread pool, based on the facilities already
 * provided by Boost.ASIO . This is meant as a drop-in replacement, until a more
 * versatile thread pool becomes commonly available in Boost.
 */
class GThreadPool
	:private boost::noncopyable // make sure the pool cannot be copied
{
public:
	/** @brief Initialization with the "native" number of threads for this architecture */
	GThreadPool();
	/** @brief Initialization with a number of threads */
	GThreadPool(const std::size_t&);
	/** @brief The destructor */
	~GThreadPool();

   /** @brief Sets the number of threads currently used */
   void setNThreads(std::size_t);
   /** @brief Retrieves the current number of threads being used in the pool */
   std::size_t getNThreads() const;

	/** @brief Blocks until all submitted jobs have been cleared from the pool */
	bool wait();

	/** @brief Allows to check whether any errors have occurred */
	bool hasErrors() const;
	/** @brief Retrieves the errors */
	void getErrors(std::vector<std::string>&);
	/** @brief Clears the error logs */
	void clearErrors();

	/***************************************************************************/
	/**
	 * Submits the task to Boost.ASIO's io_service. This function will return immediately.
	 *
	 * @param f The function to be executed by the threads in the pool
	 */
	template <typename F>
	void async_schedule(F f) {
		io_service_.post(
			boost::bind(&GThreadPool::taskWrapper<F>, this, f)
		);

		{ // Update the submission counter
			boost::unique_lock<boost::mutex> lck(job_counter_mutex_);
			submittedJobs_++;
		}
	}

private:
	/***************************************************************************/
	/**
	 * A wrapper for the thread execution that takes care of exceptions thrown by our
	 * function and allows to track how many jobs are still pending
	 *
	 * @param f A function to be executed by the threads in the pool
	 */
	template <typename F>
	void taskWrapper(F f) {
		try {
			// Execute the actual worker task
			f();
		} catch(Gem::Common::gemfony_error_condition& e) {
			// Extract the error
			std::ostringstream error;
			error << "In GThreadWrapper::operator(): Caught Gem::Common::gemfony_error_condition with message" << std::endl
				  << e.what() << std::endl;

			{ // Store the error for later reference
				boost::unique_lock<boost::shared_mutex> lck(error_mutex_); // We protect against concurrent write access
				errorCounter_++;
				errorLog_.push_back(error.str());
			}
		} catch(std::exception& e) {
			// Extract the error
			std::ostringstream error;
			error << "In GThreadWrapper::operator(): Caught std::exception with message" << std::endl
				  << e.what() << std::endl;

			{ // Store the error for later reference
				boost::unique_lock<boost::shared_mutex> lck(error_mutex_); // We protect against concurrent write access
				errorCounter_++;
				errorLog_.push_back(error.str());
			}
		} catch(boost::exception& e) {
			// Extract the error
			std::ostringstream error;
			error << "In GThreadWrapper::operator(): Caught boost::exception" << std::endl;

			{ // Store the error for later reference
				boost::unique_lock<boost::shared_mutex> lck(error_mutex_); // We protect against concurrent write access
				errorCounter_++;
				errorLog_.push_back(error.str());
			}
		} catch(...) {
			std::ostringstream error;
			error << "GThreadWrapper::operator(): Caught unknown exception" << std::endl;

			{ // Store the error for later reference
				boost::unique_lock<boost::shared_mutex> lck(error_mutex_); // We protect against concurrent write access
				errorCounter_++;
				errorLog_.push_back(error.str());
			}
		}

		{ // Update the submission counter -- we need an external means to check whether the pool has run empty
			boost::unique_lock<boost::mutex> lck(job_counter_mutex_);
			completedJobs_++;
			condition_.notify_all();
		}
	}

	/***************************************************************************/

   /** @brief Resets the entire thread pool */
   void clear();
   /** @brief Adds the desired number of work items */
   void setup(std::size_t);

	boost::asio::io_service io_service_; ///< Manages the concurrent thread execution
	boost::shared_ptr<boost::asio::io_service::work> work_; ///< A place holder ensuring that the io_service doesn't stop prematurely

	GThreadGroup gtg_; ///< Holds the actual threads

	boost::uint32_t errorCounter_; ///< The number of exceptions thrown by the pay load
	std::vector<std::string> errorLog_; ///< Holds error descriptions emitted by the work load
	mutable boost::shared_mutex error_mutex_; ///< Protects access to the error log and error counter

	boost::uint32_t submittedJobs_;  ///< The number of jobs that have been submitted in this round
	boost::uint32_t completedJobs_;  ///< The number of jobs that have been completed in this round
	mutable boost::mutex job_counter_mutex_; ///< Protects access to the "completed" job counter

	boost::condition_variable_any condition_;
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GTHREADPOOL_HPP_ */
