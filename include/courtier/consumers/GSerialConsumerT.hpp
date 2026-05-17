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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here

// Geneva headers go here
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/consumers/GBaseConsumerT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GWorkerT.hpp"

namespace Gem::Courtier::Consumers {

/******************************************************************************/
/**
 * This class adds a serial consumer to the collection of consumers.
 * This allows to use a single implementation of the available
 * optimization algorithms with all available execution modes instead
 * of different implementations of the algorithms for each mode.
 */
template <class processable_type>
class GSerialConsumerT : public GBaseConsumerT<processable_type> {
    // Make sure processable_type adheres to the GProcessingContainerT interface
    static_assert(
        std::is_base_of_v<
            Gem::Courtier::
                GProcessingContainerT<processable_type, typename processable_type::result_type>,
            processable_type>,
        "processable_type does not adhere to the GProcessingContainerT interface"
    );

public:
    /***************************************************************************/
    /**
	  * The default constructor
	  */
    GSerialConsumerT() = default;

    /***************************************************************************/

    GSerialConsumerT(const GSerialConsumerT<processable_type> &) =
        delete; ///< Intentionally left undefined
    GSerialConsumerT(GSerialConsumerT<processable_type> &&) =
        delete; ///< Intentionally left undefined
    GSerialConsumerT<processable_type> &operator=(const GSerialConsumerT<processable_type> &) =
        delete; ///< Intentionally left undefined
    GSerialConsumerT<processable_type> &
    operator=(GSerialConsumerT<processable_type> &&) = delete; ///< Intentionally left undefined

    /***************************************************************************/
    /**
	  * Standard destructor
	  */
    ~GSerialConsumerT() override = default;

    /***************************************************************************/
    /**
	  * Allows to specifcy whether this consumer is capable of full return
	  */
    void setCapableOfFullReturn(bool capableOfFullReturn) {
        glogger << "In GSerialConsumerT<processable_type>::setCapableOfFullReturn():"
                << '\n'
                << "is_capable_of_full_return_ will be set to "
                << (capableOfFullReturn ? "true" : "false") << '\n'
                << GLOGGING;
        is_capable_of_full_return_ = capableOfFullReturn;
    }

    /***************************************************************************/
    /**
	  * Allows to register a single worker template with this class.
	  */
    void registerWorkerTemplate(
        std::shared_ptr<GWorkerWithRegisterBrokerFerryT<processable_type>> workerTemplate
    ) {
#ifdef DEBUG
        if(not workerTemplate) { // Does the template point somewhere ?
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GSerialConsumerT<processable_type>::registerWorkerTemplate(): Error!"
                << '\n'
                << "Found empty worker template pointer" << '\n'
            );
        }
#endif /* DEBUG */

        worker_template_ = workerTemplate;
    }

    /***************************************************************************/
    /**
	  * Sets up a consumer and registers it with the broker. This function accepts
	  * a worker as argument.
	  */
    static void setup(
        const std::string &configFile,
        std::shared_ptr<GWorkerWithRegisterBrokerFerryT<processable_type>> worker_ptr
    ) {
        std::shared_ptr<GSerialConsumerT<processable_type>> consumer_ptr(
            new GSerialConsumerT<processable_type>()
        );

        consumer_ptr->registerWorkerTemplate(worker_ptr);
        consumer_ptr->parseConfigFile(configFile);

        GBROKER(processable_type)->enrol_consumer(consumer_ptr);
    }

protected:
    /***************************************************************************/
    /**
	 * Finalization code. Sends all threads an interrupt signal.
	 */
    void shutdown_() override {
        // This will set the GBaseConsumerT<processable_type>::stop_ flag
        GBaseConsumerT<processable_type>::shutdown_();
        // Wait for our local threads to join
        if(processing_thread_.joinable()) {
            processing_thread_.join();
        }
    }

    /***************************************************************************/
    /**
	  * Adds local configuration options to a GParserBuilder object. We have only
	  * a single local option -- the number of threads
	  *
	  * @param gpb The GParserBuilder object, to which configuration options will be added
	  */
    void addConfigurationOptions(Gem::Common::GParserBuilder &gpb) override {
        // Call our parent class'es function
        GBaseConsumerT<processable_type>::addConfigurationOptions(gpb);

        // Add local data
        // ... no local configuration
    }

private:
    /***************************************************************************/
    /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
    void addCLOptions_(
        boost::program_options::options_description & /*visible*/,
        boost::program_options::options_description &hidden
    ) override {
        namespace po = boost::program_options;

        hidden.add_options()(
            "scCapableOfFullReturn",
            po::value<bool>(&is_capable_of_full_return_)->default_value(is_capable_of_full_return_),
            "\t[sc] A debugging option making the serial consumer use timeouts in the executor"
        );
    }

    /***************************************************************************/
    /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  */
    void actOnCLOptions_(const boost::program_options::variables_map &vm) override { /* nothing */
    }

    /***************************************************************************/
    /**
	 * A unique identifier for a given consumer
	 *
	 * @return A unique identifier for a given consumer
	 */
    std::string getConsumerName_() const override {
        return std::string("GSerialConsumerT");
    }

    /***************************************************************************/
    /**
	  * Returns a short identifier for this consumer
	  */
    std::string getMnemonic_() const override {
        return std::string("sc");
    }

    /***************************************************************************/
    /**
	  * Starts a single worker thread. Termination of the thread is
	  * triggered by a call to GBaseConsumerT<processable_type>::shutdown().
	  */
    void async_startProcessing_() override {
        // Add a default worker if no worker was registered
        if(not worker_template_) {
            std::shared_ptr<GLocalConsumerWorkerT<processable_type>> default_worker(
                new GLocalConsumerWorkerT<processable_type>()
            );
            this->registerWorkerTemplate(default_worker);
        }

        glogger << "Starting single thread in GSerialConsumerT<processable_type>" << '\n'
                << GLOGGING;

        // The actual worker
        std::shared_ptr<GWorkerWithRegisterBrokerFerryT<processable_type>> p_worker =
            std::dynamic_pointer_cast<GWorkerWithRegisterBrokerFerryT<processable_type>>(
                worker_template_->clone()
            );

        // The "broker ferry" holding the connection to the broker
        std::shared_ptr<GBrokerFerryT<processable_type>> broker_ferry_ptr(new GBrokerFerryT<
                                                                          processable_type>(
            //----------------------
            std::size_t(0) // we only have one worker
            //----------------------
            ,
            [this](const std::chrono::milliseconds &timeout) -> std::shared_ptr<processable_type> {
                std::shared_ptr<processable_type> p;
                broker_ptr_->get(p, timeout);
                return p;
            }
            //----------------------
            ,
            [this](std::shared_ptr<processable_type> p, const std::chrono::milliseconds &timeout)
                -> void { broker_ptr_->put(p, timeout); }
            //----------------------
            ,
            [this]() -> bool { return this->stopped(); }
            //----------------------
        ));

        // Register the broker ferry with the worker
        p_worker->registerBrokerFerry(broker_ferry_ptr);

        processing_thread_ = std::thread([p_worker]() -> void { p_worker->run(); });

        // Store the worker for later reference
        worker_ = p_worker;
    }

    /***************************************************************************/
    /**
  	 * Returns the (possibly estimated) number of concurrent processing units.
  	 * A return value of 0 means "unknown".
  	 */
    std::size_t getNProcessingUnitsEstimate_(bool &exact) const override {
        // Mark the answer as exact
        exact = true;
        // Return the result
        return Gem::Common::narrow_cast<std::size_t>(1);
    }

    /***************************************************************************/
    /**
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed in a single thread, we assume that this
	  * is possible and return true. If you believe that this is not the case,
	  * make sure to set is_capable_of_full_return_ to false using the setCapableOfFullReturn()
	  * function. Note that, while processing-errors will likely be caught,
	  * "full return" does not mean "fully processed return", as errors (be it in
	  * user- or Geneva-code) are always possible.
	  */
    bool capableOfFullReturn_() const override {
        return is_capable_of_full_return_;
    }

    /***************************************************************************/

    std::thread processing_thread_; ///< A single thread holding the worker

    bool is_capable_of_full_return_ =
        true; ///< Indicates whether this consumer is capable of full return

    std::shared_ptr<GWorkerWithRegisterBrokerFerryT<processable_type>>
        worker_; ///< Holds the worker assigned to this consumer
    std::shared_ptr<GWorkerWithRegisterBrokerFerryT<processable_type>>
        worker_template_; ///< Holds an external worker assigned to this consumer

    std::shared_ptr<GBrokerT<processable_type>> broker_ptr_ = GBROKER(
        processable_type
    ); ///< A shortcut to the broker so we do not have to go through the singleton
};

/******************************************************************************/

} /* namespace Gem::Courtier::Consumers */
