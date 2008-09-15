/**
 * @file GLogger.hpp
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

/*
 * Todo: The log function could write log messages into a bounded buffer. A thread
 * then writes the messages to the various log targets. The user may associate each
 * log target with specific log levels and may address specific log targets with keys.
 * This way all user-output would go through this framework, and all output is
 * thread-safe.
 */

#include <iostream>
#include <exception>
#include <string>
#include <sstream>
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
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>

#ifndef GLOGGER_HPP_
#define GLOGGER_HPP_

#include "GLogTargets.hpp"
#include "GSingleton.hpp"

namespace Gem
{
namespace GLogFramework
{

/***********************************************************************************/
/** Definition of log levels */
enum logLevels {
	CRITICAL=0, // Warning: it is possible that the application might produce bad results
	WARNING=1, // The flow of the operation could be affected and this should be reported
	INFORMATIONAL=2, // Useful information
	PROGRESS=3, // Information useful for progress checks
	DEBUGGING=4, // Debug information
	CUSTOM_1=5, // Custom log level 1
	CUSTOM_2=6, // Custom log level 2
	CUSTOM_3=7, // Custom log level 3
	CUSTOM_4=8, // Custom log level 4
	CUSTOM_5=9, // Custom log level 5
	CUSTOM_6=10, // Custom log level 6
	CUSTOM_7=11, // Custom log level 7
	CUSTOM_8=12, // Custom log level 8
	CUSTOM_9=13, // Custom log level 9
	CUSTOM_10=14 // Custom log level 10
};

/***********************************************************************************/
/**
 * Every class in GenEvA should be able to log events, to targets defined by the
 * user. Different log targets, such as console or disk can thus be added to this
 * class. Logging will be done to all registered targets simultaneously. The class
 * allows concurrent access from different sources. Log events are performed in one
 * go, so that different log events do not interfere. By design it is impossible for
 * the GLogger to be copied.
 */
class GLogger: boost::noncopyable {
public:
	/** @brief The default constructor - needed for the singleton*/
	GLogger();
	/** @brief A standard destructor */
	virtual ~GLogger();

	/** @brief Adds a log target, such as console or file */
	void addTarget(const boost::shared_ptr<GBaseLogTarget>& gblt);
	/** @brief Does the actual logging */
	void log(const std::string& msg, const logLevels& level);
	/** @brief Sets a log level that should be observed */
	void addLogLevel(const logLevels& level);
	/** @brief Adds log levels <= a given threshold */
	void addLogLevelsUpTo(const logLevels& level);

	/** @brief Checks whether any log targets are present */
	bool hasLogTargets() const;

private:
	std::vector<boost::shared_ptr<GBaseLogTarget> > logVector_; ///< Contains the log targets
	std::vector<logLevels> logLevel_; ///< Contains the log levels to be observed

	boost::mutex mutex_logger_; ///< Needed for concurrent access
};

/***********************************************************************************/

} /* namespace GLogFramework */
} /* namespace Gem */

/**
 * We currently require the global GLogger object to be a singleton
 */
typedef Gem::Util::GSingleton<Gem::GLogFramework::GLogger>  logger;
#define LOGGER logger::getInstance()

#endif /* GLOGGER_HPP_ */
