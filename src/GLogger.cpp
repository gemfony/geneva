/**
 * @file GLogger.cpp
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

#include "GLogger.hpp"

namespace Gem {
	namespace GLogFramework {

		/***********************************************************************************/
		/**
		 * A standard logger. By default we only log CRITICAL. Different log levels
		 * are defined in the file GLogStreamer.h .
		 */
		GLogger::GLogger() {
			addLogLevel(Gem::GLogFramework::CRITICAL);
		}

		/***********************************************************************************/
		/**
		 * The standard destructor. Note that log targets are stored in shared_ptr objects,
		 * so destruction of log targets takes place there. GLogger takes ownership of log
		 * targets, so you do not need to delete them yourself.
		 */
		GLogger::~GLogger() {
			logVector_.clear();
			logLevel_.clear();
		}

		/***********************************************************************************/
		/**
		 * Adds a log target to the GLogger class. Note that log targets are stored in
		 * shared_ptr objects, so you do not need to delete them yourself at the end of
		 * the program.
		 *
		 * @param gblt The log target to be added to the logger
		 */
		void GLogger::addTarget(const boost::shared_ptr<GBaseLogTarget>& gblt) {
			if(!gblt) {
				std::cerr << "In GLogger::addTarget: Error!" << std::endl
				<< "GBaseLogTarget is empty." << std::endl;

				std::terminate();
			}

			logVector_.push_back(gblt);
		}

		/***********************************************************************************/
		/**
		 * This function does the actual logging. As it protects itself with a mutex, it can
		 * be called concurrently from different sources. If no logger is present yet, it will
		 * add the console logger.
		 *
		 * @param msg The message to be logged
		 * @param level The log level of this message
		 */
		void GLogger::log(const std::string& msg, const logLevels& level) {
			// lets protect ourself, so we are the only logger.
			boost::mutex::scoped_lock slock(mutex_logger_);

			if(logVector_.empty()) {
				std::clog << "In GLogger::log: Warning!" << std::endl
				<< "No log targets present. Will" << std::endl
				<< "add a console logger" << std::endl;

				this->addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));
			}

			// Only log if the requested log level is in our list
			if(std::find(logLevel_.begin(), logLevel_.end(), level) != logLevel_.end()) {
				std::vector<boost::shared_ptr<GBaseLogTarget> >::iterator it;
				for(it=logVector_.begin(); it!=logVector_.end(); ++it) (*it)->log(msg);
			}
		}

		/***********************************************************************************/
		/**
		 * In this class log levels are added. It is assumed that set up of log levels is done
		 * before any logging takes place.
		 *
		 * @param val A log level to add to the list
		 */
		void GLogger::addLogLevel(const logLevels& level) {
			// Is the value already contained in the list ?
			if(std::find(logLevel_.begin(), logLevel_.end(), level) == logLevel_.end()) {
				// no - let's add it to the list
				logLevel_.push_back(level);
				// sort the list
				std::sort(logLevel_.begin(), logLevel_.end());
			}
		}

		/***********************************************************************************/
		/**
		 * Here we can check whether any log targets are present.
		 */
		bool GLogger::hasLogTargets() const {
			if(logVector_.empty()) return false;

			return true;
		}

		/***********************************************************************************/

	} /* namespace GLogFramework */
} /* namespace Gem */
