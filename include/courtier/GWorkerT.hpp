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
#include <chrono>
#include <functional>
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem::Courtier {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * The base class for a hierarchy of classes that unify the evaluation work
     * inside of consumers.
     */
template <class processable_type>
class GWorkerT // NOLINT(cppcoreguidelines-special-member-functions)
{
    // Make sure processable_type adheres to the GProcessingContainerT interface
    static_assert(
        std::is_base_of_v<
            Gem::Courtier::
                GProcessingContainerT<processable_type, typename processable_type::result_type>,
            processable_type>,
        "GWorkerT: processable_type does not adhere to the "
        "GProcessingContainerT interface"
    );

public:
    /************************************************************************/
    /** @brief The default constructor */
    GWorkerT() = default;

    /************************************************************************/
    /**
         * Initialization with the worker id (a consecutive, unique id to be
         * supplied by the caller.
         */
    explicit GWorkerT(std::size_t workerId)
      : worker_id_(static_cast<std::int32_t>(workerId)) {
        /* nothing */
    }

protected:
    /************************************************************************/
    /**
         * The copy constructor. We do not copy the worker id. It needs to be
         * supplied by the caller. In order to avoid copying inside of the
         * default version, we supply a custom copy constructor here.
         */
    GWorkerT(const GWorkerT<processable_type> &) {
        /* nothing */
    }

public:
    /************************************************************************/
    /**
         * The destructor
         */
    virtual ~GWorkerT() = default;

    /************************************************************************/
    /**
         * Sets the worker id. This id may e.g. be used to let each worker act
         * differently. E.g., the first worker may assume a different role than
         * all the others. Note that we convert the external, unsigned type to
         * an internal, signed representation, so we may detect worker ids that
         * have not been initialized.
         *
         * @param The requested worker id
         */
    void setWorkerId(std::size_t workerId) {
        worker_id_ = Gem::Common::narrow_cast<std::int32_t>(workerId);
    }

    /************************************************************************/
    /**
         * Retrieves the worker id. Calling this function prior to the initialization
         * of the worker id will throw. "0" is an allowed value.
         *
         * @return The current worker id
         */
    [[maybe_unused]] [[nodiscard]] std::size_t getWorkerId() const {
        if(worker_id_ < 0) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GWorkerT<processable_type>::getWorkerId(): Error!" << '\n'
                << "It appears as if the worker id was not set!" << '\n'
            );
        }
                    return Gem::Common::narrow_cast<std::size_t>(worker_id_);
       
    }

    /************************************************************************/
    /**
         * Clones this object (or its derivatives). Each derivative must implement
         * the clone_() function.
         *
         * @return A clone of this object (or its derivatives, camouflaged as a
         * GWorkerT)
         */
    std::shared_ptr<GWorkerT<processable_type>> clone() const {
        return this->clone_();
    }

    /************************************************************************/
    /**
         * The main entry point for the execution
         */
    void run() {
        //---------------------------------------------------------------------
        // For error descriptions
        std::ostringstream error_streamer; // NOLINT(cppcoreguidelines-init-variables)
        // Indicates whether an error was found
        bool has_error = false;

        //---------------------------------------------------------------------
        // Some error checks
        if(-1 == worker_id_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GWorkerT<processable_type>::run(): Error!" << '\n'
                << "It appears as if the worker id was not set!" << '\n'
            );
        }

        //---------------------------------------------------------------------
        // The actual loop. While some error checks are already done in the
        // process()-call, we also try to catch any other error that might occur,
        // so we are sure to detect problems in user-code or our own code early.

        try {
            std::shared_ptr<processable_type> p;
            bool first = true;

            // The main loop
            do {
                // Retrieve an item and check for its validity. Try again if
                // we didn't receive a valid item
                if(not(
                       p = this->retrieve(retrieval_timeout_)
                   )) // NOLINT(bugprone-assignment-in-if-condition)
                {
                    continue;
                }

                if(first) {
                    // Any necessary setup work
                    this->processInit(p);
                    first = false;
                }

                // Initiate the actual processing
                this->process(p);

                // Return the item. Note that the submit function has the freedom
                // to discard items if a submission is not possible.
                this->submit(p, submission_timeout_);
            }
            while(not this->stop_requested());

            // Perform any final work
            this->processFinalize();
        }
        catch(geneva_exception &e) {
            has_error = true;
            error_streamer << "In GWorkerT<processable_type>::run(): Caught "
                              "geneva_exception with message"
                           << '\n'
                           << e.what() << '\n';
        }
        catch(std::exception &e) {
            has_error = true;
            error_streamer << "In GWorkerT<processable_type>::run():" << '\n'
                           << "Caught std::exception with message" << '\n'
                           << e.what() << '\n';
        }
        catch(...) {
            has_error = true;
            error_streamer << "In GWorkerT<processable_type>::run():" << '\n'
                           << "Caught unknown exception." << '\n';
        }

        //---------------------------------------------------------------------
        // Make it known if there was a problem
        if(has_error) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place) << error_streamer.str()
            );
        }

        //---------------------------------------------------------------------
    }

    /************************************************************************/
    /**
         * Allows to treat derived objects as a function object
         */
    void operator()() {
        this->run();
    }

    /************************************************************************/
    /**
         * Parses a given configuration file
         *
         * @param configFile The name of a configuration file
         */
    void parseConfigFile(std::filesystem::path const &configFile) {
        // Create a parser builder object -- local options will be added to it
        Gem::Common::GParserBuilder gpb;

        // Add configuration options of this and of derived classes
        addConfigurationOptions(gpb);

        // Do the actual parsing. Note that this
        // will try to write out a default configuration file,
        // if no existing config file can be found
        gpb.parseConfigFile(configFile);
    }

    /************************************************************************/
    /**
         * Adds local configuration options to a GParserBuilder object. This function
         * only relies to our local implementation, which may be overridden in derived
         * classes. Note that the overriding function should take care to call the
         * parent's addConfigurationOptions_() function.
         *
         * @param gpb The GParserBuilder object, to which configuration options will
         * be added
         */
    void addConfigurationOptions(Gem::Common::GParserBuilder &gpb) {
        this->addConfigurationOptions_(gpb);
    }

protected:
    /************************************************************************/
    /**
         * The actual implementation of adding configuration options. "protected",
         * so it may be called by derived classes and these classes may call this
         * function.´
         *
         * @param gpb The GParserBuilder object, to which configuration options will
         * be added
         */
    virtual void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
        /* nothing -- no local data */
    }

private:
    /************************************************************************/
    /**
         * Initialization code for processing. Specify custom code in the virtual
         * processInit_ function.
         */
    void processInit(std::shared_ptr<processable_type> p) {
        this->processInit_(p);
    }

    /************************************************************************/
    /**
         * Single-pass processing of work items. Specify custom code for processing
         * in the virtual process_ function.
         */
    void process(std::shared_ptr<processable_type> p) {
        // GProcesingContainerT-derivatives may emit a g_processing_exception if
        // they have detected a problem in the actual processing code. This will
        // usually invalidate the work item, but not the entire application. Hence
        // we capture this exception here and leave it to other recipients of the
        // work item to decide on its fate.
        try {
            this->process_(p);
        }
        catch(
            const g_processing_exception &e
        ) // NOLINT(bugprone-empty-catch) — expected; error stored in work item
        {
            glogger << "In GWorkerT<processable_type>::process():" << '\n'
                    << "The work item has flagged a processing exception with the message"
                    << '\n'
                    << e << '\n'
                    << "The item will be returned. It is up to the recipient of the work "
                       "item"
                    << '\n'
                    << "to decide on its fate" << '\n'
                    << GWARNING;
        }
    }

    /************************************************************************/
    /**
         * Finalization code for processing. Specify the actual work in derived
         * classes by overriding processFinalize_()
         */
    void processFinalize() {
        this->processFinalize_();
    }

    /************************************************************************/
    /**
         * Retrieval of work items
         */
    std::shared_ptr<processable_type> retrieve(const std::chrono::milliseconds &timeout) {
        return this->retrieve_(timeout);
    }

    /************************************************************************/
    /**
         * Submission of work items
         */
    void
    submit(std::shared_ptr<processable_type> item_ptr, const std::chrono::milliseconds &timeout) {
        this->submit_(item_ptr, timeout);
    }

    /************************************************************************/
    /**
         * Indicates whether the worker was asked to stop processing
         */
    [[nodiscard]] bool stop_requested() const {
        return this->stop_requested_();
    }

    /************************************************************************/
    /**
         * Setting of the total number of workers
         */
    void setNWorkers(std::size_t nWorkers) {
        n_workers_ = nWorkers;
    }

    /************************************************************************/
    /**
         * Retrieve the total number of workers
         */
    std::size_t getNWorkers() const {
        return n_workers_;
    }

protected:
    /************************************************************************/
    // Some purely virtual functions, to be implemented in derived classes

    /** @brief Initialization code for processing. */
    virtual void processInit_(std::shared_ptr<processable_type> p) = 0;
    /** @brief Actual per-item work is done here -- all error-detection
         * instrumentation is done in the protected "process() function. */
    virtual void process_(std::shared_ptr<processable_type> p) = 0;
    /** @brief Finalization code for processing. */
    virtual void processFinalize_() = 0;

private:
    /** @brief Creation of deep clones of this object('s derivatives) */
    virtual std::shared_ptr<GWorkerT<processable_type>> clone_() const = 0;
    /** @brief Retrieval of work items */
    virtual std::shared_ptr<processable_type> retrieve_(const std::chrono::milliseconds &) = 0;
    /** @brief Submission of work items */
    virtual void submit_(std::shared_ptr<processable_type>, const std::chrono::milliseconds &) = 0;
    /** @brief Indicates whether the worker was asked to stop processing */
    [[nodiscard]] virtual bool stop_requested_() const = 0;

    /************************************************************************/
    // Data

    const std::chrono::milliseconds submission_timeout_ =
        std::chrono::milliseconds(200); ///< Timeout for submit operations
    const std::chrono::milliseconds retrieval_timeout_ =
        std::chrono::milliseconds(200); ///< Timeout for retrieval operations

    std::int32_t worker_id_ = -1; ///< The id of the thread running this class'es operator()

    std::size_t n_workers_{0}; ///< The amount of workers of this type

    /************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * This class acts as a little helper, holding information and functions
     * needed for retrieving and submitting work items as well as termination.
     */
template <class processable_type>
class GBrokerFerryT // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
    /************************************************************************/
    /**
         * Initialization is only allowed with a single constructor, so we only need
         * to check for content once.
         */
    GBrokerFerryT(
        const std::size_t &worker_id,
        std::function<std::shared_ptr<processable_type>(const std::chrono::milliseconds &)>
            retriever,
        std::function<void(std::shared_ptr<processable_type>, const std::chrono::milliseconds &)>
            submitter,
        std::function<bool()> stop_requested
    )
      : worker_id_(worker_id)
      , retriever_(retriever)
      , submitter_(submitter)
      , stop_requested_(stop_requested) {
        if(not retriever_) {
            glogger << "In GLocalConsumerWorkerT<processable_type>::GBrokerFerryT(): "
                       "Error!"
                    << '\n'
                    << "Empty retriever function found!" << '\n'
                    << "We cannot continue" << '\n'
                    << GTERMINATION;
        }

        if(not submitter_) {
            glogger << "In GLocalConsumerWorkerT<processable_type>::GBrokerFerryT(): "
                       "Error!"
                    << '\n'
                    << "Empty submitter function found!" << '\n'
                    << "We cannot continue" << '\n'
                    << GTERMINATION;
        }

        if(not stop_requested_) {
            glogger << "In GLocalConsumerWorkerT<processable_type>::GBrokerFerryT(): "
                       "Error!"
                    << '\n'
                    << "Empty termination function found!" << '\n'
                    << "We cannot continue" << '\n'
                    << GTERMINATION;
        }
    }

    /************************************************************************/
    // Some deleted functions -- this class is non-copyable
    GBrokerFerryT() = delete;
    GBrokerFerryT(const GBrokerFerryT<processable_type> &) = delete;
    GBrokerFerryT(GBrokerFerryT<processable_type> &&) = delete;
    GBrokerFerryT<processable_type> &operator=(const GBrokerFerryT<processable_type> &) = delete;
    GBrokerFerryT<processable_type> &operator=(GBrokerFerryT<processable_type> &&) = delete;

    /************************************************************************/
    /**
         * Retrieval of work items
         */
    std::shared_ptr<processable_type> retrieve(const std::chrono::milliseconds &timeout) {
        return this->retriever_(timeout);
    }

    /************************************************************************/
    /**
         * Submission of work items
         */
    void
    submit(std::shared_ptr<processable_type> item_ptr, const std::chrono::milliseconds &timeout) {
        return this->submitter_(item_ptr, timeout);
    }

    /************************************************************************/
    /**
         * Indicates whether the worker was asked to stop processing
         */
    [[nodiscard]] bool stop_requested() const {
        return this->stop_requested_();
    }

    /************************************************************************/
    /**
         * Access to the worker id
         */
    [[nodiscard]] std::size_t getWorkerId() const {
        return worker_id_;
    }

private:
    /************************************************************************/
    // Data and stored functions

    std::size_t worker_id_ = 0; ///< An id to be assigned to a worker

    std::function<std::shared_ptr<processable_type>(const std::chrono::milliseconds &)>
        retriever_; ///< Retrieval of new work item
    std::function<void(
        std::shared_ptr<processable_type>,
        const std::chrono::milliseconds &
    )>
        submitter_;                        ///< Submission of processed work items
    std::function<bool()> stop_requested_; ///< Termination of the exeecution run

    /************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// An abstract class that provides the registerBrokerFerry function. It
// allows to register derived classes in "local" consumers
template <typename processable_type>
class GWorkerWithRegisterBrokerFerryT : public GWorkerT<processable_type> {
public:
    // Take over all constructors of the parent class
    using GWorkerT<processable_type>::GWorkerT;
    // Default destructor
    ~GWorkerWithRegisterBrokerFerryT() override = default;

    /************************************************************************/
    /**
         * Allows to register a container object for various information
         * needed by this class
         */
    void
    registerBrokerFerry(const std::shared_ptr<GBrokerFerryT<processable_type>> &broker_ferry_ptr) {
        if(not broker_ferry_ptr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In "
                   "GLocalConsumerWorkerT<processable_type>::"
                   "registerBrokerFerry(): Error!"
                << '\n'
                << "Empty broker ferry object found!" << '\n'
            );
        }

        broker_ferry_ptr_ = broker_ferry_ptr;

        // Set the worker id immediately, so the run function does not stumble on
        // an invalid value		 // Extract and set the worker id
        this->setWorkerId(broker_ferry_ptr_->getWorkerId());
    }

private:
    /************************************************************************/
    /** @brief Retrieval of work items */
    std::shared_ptr<processable_type> retrieve_(const std::chrono::milliseconds &timeout) override {
        return this->broker_ferry_ptr_->retrieve(timeout);
    }

    /************************************************************************/
    /** @brief Submission of work items */
    void submit_(
        std::shared_ptr<processable_type> p,
        const std::chrono::milliseconds &timeout
    ) override {
        this->broker_ferry_ptr_->submit(p, timeout);
    }

    /************************************************************************/
    /** @brief Indicates whether the worker was asked to stop processing */
    [[nodiscard]] bool stop_requested_() const override {
        return this->broker_ferry_ptr_->stop_requested();
    }

    /************************************************************************/
    // Data

    /** @brief A pointer to a container object holding information needed by this class */
    std::shared_ptr<GBrokerFerryT<processable_type>> broker_ferry_ptr_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * Unifies the processing of work items inside of consumers that do not
     * submit work for processing to a remote location.
     */
template <class processable_type>
class GLocalConsumerWorkerT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GWorkerWithRegisterBrokerFerryT<processable_type> {
public:
    /************************************************************************/
    /** @brief The default constructor */
    GLocalConsumerWorkerT() = default;

protected:
    /************************************************************************/
    /**
         * The copy constructor.
         */
    GLocalConsumerWorkerT(const GLocalConsumerWorkerT<processable_type> &cp)
      : GWorkerWithRegisterBrokerFerryT<processable_type>(cp) {
        /* nothing */
    }

public:
    /************************************************************************/
    /**
         * The destructor
         */
    ~GLocalConsumerWorkerT() override = default;

protected:
    /************************************************************************/
    /**
         * The actual implementation of adding configuration options. "protected",
         * so it may be called by derived classes.
         *
         * @param gpb The GParserBuilder object, to which configuration options will be added
         */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Make sure any options from our parent class are processed
        GWorkerWithRegisterBrokerFerryT<processable_type>::addConfigurationOptions_(gpb);
    }

    /************************************************************************/
    /**
         * Initialization code for processing.
         */
    void processInit_(std::shared_ptr<processable_type> p) override { /* nothing */
    }

    /************************************************************************/
    /**
         * Only actual per-item work is done here -- Error-detection instrumentation
         * is done in the protected "process()" function of our parent class.
         */
    void process_(std::shared_ptr<processable_type> p) override {
        p->process();
    }

    /************************************************************************/
    /**
         * Finalization code for processing.
         */
    void processFinalize_() override {
        /* nothing */
    }

private:
    /************************************************************************/
    /**
         * Creation of deep clones of this object. Note that a new broker ferry
         * needs to be registered with this object.
         */
    std::shared_ptr<GWorkerT<processable_type>> clone_() const override {
        return std::shared_ptr<GWorkerWithRegisterBrokerFerryT<processable_type>>(
            new GLocalConsumerWorkerT<processable_type>(*this)
        );
    }

    /************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Courtier */
