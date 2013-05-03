/**
 * @file GBaseConsumer.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "courtier/GBaseConsumer.hpp"

namespace Gem
{
namespace Courtier
{

/******************************************************************************/
/**
 * The default constructor
 */
GBaseConsumer::GBaseConsumer()
   : stop_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GBaseConsumer::~GBaseConsumer()
{ /* nothing */ }

/******************************************************************************/
/**
 * Stop execution. Note that this function requires unique access to the lock.
 */
void GBaseConsumer::shutdown() {
   boost::unique_lock<boost::shared_mutex> lock(stopMutex_);
   stop_=true;
   lock.unlock();
}

/******************************************************************************/
/**
 * Check whether the stop flag has been set. Since we only read the flag, a
 * shared_lock suffices.
 */
bool GBaseConsumer::stopped() const {
   boost::shared_lock<boost::shared_mutex> lock(stopMutex_);
   return stop_;
}

/******************************************************************************/
/**
 * Returns an indication whether a full return of work items can be expected
 * from the consonumer. By default we asusme that a full return is not possible.
 */
bool GBaseConsumer::capableOfFullReturn() const {
   return false;
}

/******************************************************************************/
/**
 * Parses a given configuration file
 *
 * @param configFile The name of a configuration file
 */
void GBaseConsumer::parseConfigFile(const std::string& configFile) {
   // Create a parser builder object -- local options will be added to it
   Gem::Common::GParserBuilder gpb;

   // Add configuration options of this and of derived classes
   addConfigurationOptions(gpb, true);

   // Do the actual parsing. Note that this
   // will try to write out a default configuration file,
   // if no existing config file can be found
   gpb.parseConfigFile(configFile);
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object. We have no local
 * data, hence this function is empty. It could have been declared purely virtual,
 * however, we do not want to force derived classes to implement this function,
 * as it might not always be needed.
 *
 * @param gpb The GParserBuilder object, to which configuration options will be added
 * @param showOrigin Indicates, whether the origin of a configuration option should be shown in the configuration file
 */
void GBaseConsumer::addConfigurationOptions(
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
){
   /* nothing -- no local data */
}

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

