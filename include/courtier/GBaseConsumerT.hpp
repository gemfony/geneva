/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>
#include <sstream>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

// Boost headers go here

#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/vector.hpp>

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
 * boost::noncopyable class. GBaseConsumerT::process() is started in a separate
 * thread by the broker. GBaseConsumer::shutdown() is called by the broker
 * when the consumer is supposed to shut down. Please note that, for the
 * purpose of the calculation of timeouts, the time of the first retrieval
 * of a work item from a GBufferPortT (which is triggered by the actions of
 * the broker) plays a role, so consumers should not retrieve items prior to
 * a dedicated request from a client (in the case of networked execution)
 * or a worker (in the case of multi-threaded work).
 */
template<typename processable_type>
class GBaseConsumerT {
public:
	 //-------------------------------------------------------------------------
	 // Defaulted or deleted constructors, destructor and assignment operators

	 GBaseConsumerT() = default;

	 GBaseConsumerT(const GBaseConsumerT<processable_type> &) = delete; ///< Intentionally left undefined
	 GBaseConsumerT(GBaseConsumerT<processable_type> &&) = delete; ///< Intentionally left undefined

	 virtual ~GBaseConsumerT() BASE = default;

	 GBaseConsumerT<processable_type> &operator=(const GBaseConsumerT<processable_type> &) = delete; ///< Intentionally left undefined
	 GBaseConsumerT<processable_type> &operator=(GBaseConsumerT<processable_type> &&) = delete; ///< Intentionally left undefined

	 //-------------------------------------------------------------------------
	 /**
 	  * Stop execution
     */
	 void shutdown() {
		 this->shutdown_();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Check whether the stop flag has been set
	  */
	 bool stopped() const {
		 return m_server_stopping;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns an indication whether full return can be expected from the consumer.
	  * By default we assume that a full return is not possible.
	  */
	 bool capableOfFullReturn() const {
		 return this->capableOfFullReturn_();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns the (possibly estimated) number of concurrent processing units.
	  * A return value of 0 means "unknown". Note that this function does not
	  * make any assumptions whether processing units are dedicated solely to a
	  * given task.
	  */
	 std::size_t getNProcessingUnitsEstimate(bool& exact) const {
		 return this->getNProcessingUnitsEstimate_(exact);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Parses a given configuration file
	  *
	  * @param configFile The name of a configuration file
	  */
	 void parseConfigFile(boost::filesystem::path const &configFile) {
		 // Create a parser builder object -- local options will be added to it
		 Gem::Common::GParserBuilder gpb;

		 // Add configuration options of this and of derived classes
		 addConfigurationOptions(gpb);

		 // Do the actual parsing. Note that this
		 // will try to write out a default configuration file,
		 // if no existing config file can be found
		 gpb.parseConfigFile(configFile);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Allows to check whether this consumer needs a client to operate. By default
	  * we return false, so that consumers without the need for clients do not need
	  * to re-implement this function.
	  *
	  * @return A boolean indicating whether this consumer needs a client to operate
	  */
	 bool needsClient() const {
		 return this->needsClient_();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * This function returns a client associated with this consumer. By default
	  * it returns an empty smart pointer, so that consumers without the need for
	  * clients do not need to re-implement this function.
	  */
	 std::shared_ptr<GBaseClientT<processable_type>> getClient() const {
		 return this->getClient_();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  * By default we do nothing so that derived classes do not need to re-implement this
	  * function.
	  */
	 void actOnCLOptions(const boost::program_options::variables_map &vm) {
		 this->actOnCLOptions_(vm);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  * By default we do nothing so that derived classes do not need to re-implement this
	  * function.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
	 void addCLOptions(
		 boost::program_options::options_description &visible
		 , boost::program_options::options_description &hidden
	 ) {
		this->addCLOptions_(visible, hidden);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * A unique identifier for a given consumer
	  */
	 std::string getConsumerName() const {
		 return this->getConsumerName_();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns a short identifier for this consumer
	  */
	 std::string getMnemonic() const {
		 return this->getMnemonic_();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * The actual business logic
	  */
	 void async_startProcessing() {
		 this->async_startProcessing_();
	 }

protected:
	 //-------------------------------------------------------------------------
	 /**
	  * Stop execution
	  */
	 virtual void shutdown_() BASE {
		 m_server_stopping.store(true);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Adds local configuration options to a GParserBuilder object. We have no local
	  * data, hence this function is empty. It could have been declared purely virtual,
	  * however, we do not want to force derived classes to implement this function,
	  * as it might not always be needed.
	  *
	  * @param gpb The GParserBuilder object, to which configuration options will be added
	  */
	 virtual void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) BASE { /* nothing -- no local data */ }

private:
	 //-------------------------------------------------------------------------
	 // Some abstract functions

	 /** @brief Adds local command line options to a boost::program_options::options_description object */
	 virtual void addCLOptions_(
		 boost::program_options::options_description&
		 , boost::program_options::options_description&
	 ) BASE = 0;

	 /** @brief Takes a boost::program_options::variables_map object and checks for supplied options */
	 virtual void actOnCLOptions_(const boost::program_options::variables_map&) BASE = 0;

	 /** @brief A unique identifier for a given consumer */
	 virtual std::string getConsumerName_() const BASE = 0;

	 /** @brief Returns a short identifier for this consumer */
	 virtual std::string getMnemonic_() const BASE = 0;

	 /** @brief The actual business logic */
	 virtual void async_startProcessing_() BASE = 0;

	 //-------------------------------------------------------------------------
	 /**
	  * This function returns a client associated with this consumer. By default
	  * it returns an empty smart pointer, so that consumers without the need for
	  * clients do not need to re-implement this function.
	  */
	 virtual std::shared_ptr<GBaseClientT<processable_type>> getClient_() const BASE {
		 return std::shared_ptr<GBaseClientT<processable_type>>();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Allows to check whether this consumer needs a client to operate. By default
	  * we return false, so that consumers without the need for clients do not need
	  * to re-implement this function.
	  *
	  * @return A boolean indicating whether this consumer needs a client to operate
	  */
	 virtual bool needsClient_() const noexcept BASE {
		 return false;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns the (possibly estimated) number of concurrent processing units.
	  * A return value of 0 means "unknown". Note that this function does not
	  * make any assumptions whether processing units are dedicated solely to a
	  * given task.
	  */
	 virtual std::size_t getNProcessingUnitsEstimate_(bool& exact) const BASE = 0;

	 //-------------------------------------------------------------------------
	 /**
	  * Returns an indication whether full return can be expected from the consumer.
	  * By default we assume that a full return is not possible.
	  */
	 virtual bool capableOfFullReturn_() const BASE = 0;

	 //-------------------------------------------------------------------------

	 mutable std::atomic<bool> m_server_stopping{false}; ///< Set to true if we are expected to stop

	 //-------------------------------------------------------------------------
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

