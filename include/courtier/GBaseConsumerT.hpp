/**
 * @file GBaseConsumerT.hpp
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
#include <string>
#include <sstream>

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread.hpp>
#include <boost/type_traits.hpp>

#ifndef GCONSUMER_HPP_
#define GCONSUMER_HPP_

// Geneva headers go here
#include "common/GParserBuilder.hpp"
#include "courtier/GBaseClientT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes that take
 * objects from GBrokerT and process them, either locally or remotely.
 * Derived classes such as the GAsioTCPConsumerT form the single point
 * of contact for remote clients. We do not want this class and its
 * derivatives to be copyable, hence we derive it from the
 * boost::noncopyable class. GBaseConsumer::process() is started in a separate
 * thread by the broker. GBaseConsumer::shutdown() is called by the broker
 * when the consumer is supposed to shut down.
 */
template <typename pl_type> // pl stands for "pay load"
class GBaseConsumerT
	:private boost::noncopyable
{
public:
	typedef pl_type payload;

	/***************************************************************************/
   /**
    * The default constructor
    */
	GBaseConsumerT()
      : stop_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The standard destructor
    */
	virtual ~GBaseConsumerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Stop execution
	 */
	virtual void shutdown() {
	   boost::unique_lock<boost::shared_mutex> lock(stopMutex_);
	   stop_=true;
	   lock.unlock();
	}

	/***************************************************************************/
	/**
	 * Check whether the stop flag has been set
	 */
	bool stopped() const {
	   boost::shared_lock<boost::shared_mutex> lock(stopMutex_);
	   return stop_;
	}

	/***************************************************************************/
	/**
	 * Returns an indication whether full return can be expected from the consumer.
	 * By default we assume that a full return is not possible.
	 */
	virtual bool capableOfFullReturn() const {
	   return false;
	}

	/***************************************************************************/
	/**
	 * Parses a given configuration file
	 *
	 * @param configFile The name of a configuration file
	 */
	void parseConfigFile(const std::string& configFile) {
      // Create a parser builder object -- local options will be added to it
      Gem::Common::GParserBuilder gpb;

      // Add configuration options of this and of derived classes
      addConfigurationOptions(gpb);

      // Do the actual parsing. Note that this
      // will try to write out a default configuration file,
      // if no existing config file can be found
      gpb.parseConfigFile(configFile);
   }

   /***************************************************************************/
   /**
    * Allows to check whether this consumer needs a client to operate. By default
    * we return false, so that consumers without the need for clients do not need
    * to re-implement this function.
    *
    * @return A boolean indicating whether this consumer needs a client to operate
    */
   virtual bool needsClient() const {
      return false;
   }

   /***************************************************************************/
   /**
    * This function returns a client associated with this consumer. By default
    * it returns an empty smart pointer, so that consumers without the need for
    * clients do not need to re-implement this function.
    */
   virtual boost::shared_ptr<GBaseClientT<pl_type> > getClient() const {
      return boost::shared_ptr<GBaseClientT<pl_type> >();
   }

   /***************************************************************************/
   /**
    * Adds local command line options to a boost::program_options::options_description object.
    * By default we do nothing so that derived classes do not need to re-implement this
    * function.
    *
    * @param visible Command line options that should always be visible
    * @param hidden Command line options that should only be visible upon request
    */
   virtual void addCLOptions(
      boost::program_options::options_description& visible
      , boost::program_options::options_description& hidden
   ) BASE { /* nothing */ }

   /***************************************************************************/
   /**
    * Takes a boost::program_options::variables_map object and checks for supplied options.
    * By default we do nothing so that derived classes do not need to re-implement this
    * function.
    */
   virtual void actOnCLOptions(const boost::program_options::variables_map& vm)
   { /* nothing */ }

   /***************************************************************************/
   // Some abstract functions

   /** @brief A unique identifier for a given consumer */
   virtual std::string getConsumerName() const = 0;
   /** @brief Returns a short identifier for this consumer */
   virtual std::string getMnemonic() const = 0;

   /** @brief The actual business logic */
   virtual void async_startProcessing() = 0;

protected:
   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object. We have no local
    * data, hence this function is empty. It could have been declared purely virtual,
    * however, we do not want to force derived classes to implement this function,
    * as it might not always be needed.
    *
    * @param gpb The GParserBuilder object, to which configuration options will be added
    */
   virtual void addConfigurationOptions(
         Gem::Common::GParserBuilder& gpb
   ){ /* nothing -- no local data */ }

private:
   /***************************************************************************/
   mutable boost::shared_mutex stopMutex_; ///< Regulate access to the stop_ variable
   mutable bool stop_; ///< Set to true if we are expected to stop
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /*GCONSUMER_HPP_*/
