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
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GBoundedBufferT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GSingletonT.hpp"
#include "courtier/consumers/GBaseConsumerT.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/** @brief Exception to be thrown as a message in the case of a time-out in GBrokerT */
class buffer_not_present : public std::exception {
public:
    using std::exception::exception;
};

/******************************************************************************/
/**
 * This class acts as the main mediator between producers and consumers.
 */
template <typename processable_type>
class GBrokerT {
    // Make sure processable_type adheres to the GProcessingContainerT interface
    static_assert(
        std::is_base_of_v<
            Gem::Courtier::
                GProcessingContainerT<processable_type, typename processable_type::result_type>,
            processable_type>,
        "GBaseExecutorT: processable_type does not adhere to the GProcessingContainerT<> interface"
    );

    // Syntactic sugar
    using GBUFFERPORT = GBufferPortT<processable_type>;
    using GBUFFERPORT_PTR = typename std::shared_ptr<GBUFFERPORT>;
    using RawBufferPtrMap = typename std::map<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR>;
    using ProcessedBufferPtrMap = typename std::map<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR>;

public:
    /***************************************************************************/
    /**
	  * The standard destructor. Notifies all consumers that they should stop, then waits
	  * for their threads to terminate.
	  */
    ~GBrokerT() {
        // Make sure the finalization code is executed
        // (if this hasn't happened already). Calling
        // finalize() multiple times is safe.
        finalize();
    }

    /***************************************************************************/
    // Defaulted or deleted constructors and assignment operators.
    // Copy and move are explicitly deleted.

    GBrokerT() = default;

    GBrokerT(const GBrokerT<processable_type> &) = delete;
    GBrokerT(GBrokerT<processable_type> &&) = delete;

    GBrokerT<processable_type> &operator=(const GBrokerT<processable_type> &) = delete;
    GBrokerT<processable_type> &operator=(GBrokerT<processable_type> &&) = delete;

    /***************************************************************************/
    /**
	  * Initializes the broker. This function does nothing. Its only purpose is to control
	  * initialization of the factory in the singleton.
	  */
    void init() { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Shuts the broker down, together with all consumers.
	  */
    void finalize() {
        // Only allow one finalization action to be carried out
        if(finalized_) {
            return;
        }

        {
            //-----------------------------------------------------------------------
            // Lock the access to our internal data simultaneously for all mutexes
            std::scoped_lock lk(switch_get_position_mutex_, find_procesed_buffer_mutex_, consumer_enrolment_mutex_);
            //-----------------------------------------------------------------------

            // Shut down all consumers while holding the enrolment lock to prevent
            // a data race with concurrent push_back in enrol_consumer().
            for(auto const &c_ptr : consumer_collection_cnt_) {
                c_ptr->shutdown();
            }

            // Clear raw and processed buffers and the consumer lists
            raw_buffers_.clear();
            processed_buffers_.clear();
            consumer_collection_cnt_.clear();
            buffers_present_.store(false);

            // Make sure this function does not execute code a second time
            finalized_.store(true);
        }
    }

    /***************************************************************************/
    /**
	  * This function is used by producers to register a new GBufferPortT object
	  * with the broker. A GBufferPortT object contains bounded buffers for raw (i.e.
	  * unprocessed) items and for processed items. A producer may at any time decide
	  * to drop a GBufferPortT, but needs to indicate this fact to the buffer port. It is the
	  * task of this function to remove the orphaned shared_ptr<GBufferPortT> pointers.
	  * It thus needs to block access to the entire object during its operation. Note that one of
	  * the effects of this function is that the buffer collections will never run empty,
	  * once the first buffer has been registered.
	  *
	  * @param gbp_ptr A shared pointer to a new GBufferPortT object
	  * @return A boolean which indicates, whether all consumers are capable of full return
	  */
    bool enrol_buffer_port(std::shared_ptr<GBufferPortT<processable_type>> gbp_ptr) {
        //-----------------------------------------------------------------------
        // Lock the access to our internal data simultaneously for all mutexes
        std::scoped_lock lk(switch_get_position_mutex_, find_procesed_buffer_mutex_);
        //-----------------------------------------------------------------------
        // Find orphaned items in the two collections and remove them.
        // Note that, unforunately, g++ < 5.0 does not support auto in lambda statements,
        // otherwise the following statements could be simplified.
        std::size_t nErasedRaw = Gem::Common::erase_if(
            raw_buffers_,
            [](const std::pair<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR> &p) -> bool {
                return (not p.second->is_connected_to_producer());
            }
        ); // raw_buffers_ is a std::map, so items are of type std::pair

#ifdef DEBUG
        if(nErasedRaw > 0) {
            glogger << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr): Removed " << nErasedRaw
                    << " raw buffers" << '\n'
                    << GLOGGING;
        }
#endif

        std::size_t nErasedProc = Gem::Common::erase_if(
            processed_buffers_,
            [](const std::pair<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR> &p) -> bool {
                return (not p.second->is_connected_to_producer());
            }
        ); // processed_buffers_ is a std::map, so items are of type std::pair

#ifdef DEBUG
        if(nErasedProc != nErasedRaw) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr):" << '\n'
                << "nErasedProc (" << nErasedProc << ") != nErasedRaw (" << nErasedRaw << ")"
                << '\n'
            );
        }

        if(nErasedProc > 0) {
            glogger << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr): Removed " << nErasedProc
                    << " processed buffers" << '\n'
                    << GLOGGING;
        }
#endif

        // Update the number of registered buffer ports
#ifdef DEBUG
        if(static_cast<BUFFERPORT_ID_TYPE>(nErasedRaw) > n_registered_buffer_ports_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr):" << '\n'
                << "nErasedRaw (" << nErasedRaw << ") > n_registered_buffer_ports_ ("
                << n_registered_buffer_ports_ << ")" << '\n'
            );
        }
#endif
        n_registered_buffer_ports_ -= static_cast<BUFFERPORT_ID_TYPE>(nErasedRaw);

        // Retrieve a new id for the buffer port.
        auto gbp_tag = getNextBufferPortId();

        // Register the id with the buffer port
        gbp_ptr->set_port_tag(gbp_tag);

        // Attach the new items to the maps
        raw_buffers_[gbp_tag] = gbp_ptr;
        processed_buffers_[gbp_tag] = gbp_ptr;

        // Increment the number of registered buffer ports and check if we have exceeded the allowed amound
        if(++n_registered_buffer_ports_ > MAXREGISTEREDBUFFERPORTS) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr):" << '\n'
                << "Maximum number " << MAXREGISTEREDBUFFERPORTS
                << " of registered buffer ports exceeded" << '\n'
            );
        }

        // Fix the current get-pointer. We simply attach it to the start of the list
        current_get_position_ = raw_buffers_.begin();

        glogger << "Buffer port with id " << gbp_tag << " successfully enrolled" << '\n'
                << GLOGGING;

        // Let the audience know
        buffers_present_.store(true);

        // Let the audience know whether all consumers are capable of full return
        return capable_of_full_return_;
    }

    /***************************************************************************/
    /**
	  * Adds a new consumer to this class and starts its thread.
	  *
	  * @param gc_ptr A pointer to a GBaseConsumerT<processable_type> object
	  */
    void enrol_consumer(std::shared_ptr<cons::GBaseConsumerT<processable_type>> gc_ptr) {
        //-----------------------------------------------------------------------
        std::unique_lock<std::mutex> consumerEnrolmentLock(consumer_enrolment_mutex_);

        // Check whether consumers have already been enrolled. As this may happen
        // only once, we emit a warning and return.
        // Check is inside the lock to prevent a TOCTOU race between concurrent callers.
        if(consumers_present_) {
            glogger << "In GBrokerT<>::enrol_buffer_port(consumer_ptr): One or more consumers have "
                       "already been enrolled."
                    << '\n'
                    << "We will ignore the new enrolment request." << '\n'
                    << GWARNING;

            return;
        }

        // Do nothing if a consumer of this type has already been registered
        if(std::find(
               consumer_types_present_.begin(),
               consumer_types_present_.end(),
               gc_ptr->getConsumerName()
           ) != consumer_types_present_.end()) {
            glogger << "In GBrokerT<>::enrol_buffer_port(consumer):" << '\n'
                    << "Consumer with name " << gc_ptr->getConsumerName() << " aleady exists."
                    << '\n'
                    << "We will ignore the new enrolment request." << '\n'
                    << GWARNING;

            return;
        }

        // Archive the consumer and its name, then start its thread
        consumer_collection_cnt_.push_back(gc_ptr);
        consumer_types_present_.push_back(gc_ptr->getConsumerName());

        // Initiate processing in the consumer. This call will not block.
        gc_ptr->async_startProcessing();

        //-----------------------------------------------------------------------
        // Make it known to subsequent calls that a consumer is already present

        consumers_present_.store(true);

        //-----------------------------------------------------------------------

        // Check whether all registered consumers are capable of full return
        capable_of_full_return_.store(this->checkConsumersCapableOfFullReturn());

        // Notify outside the lock to avoid immediately re-blocking the woken thread
        consumerEnrolmentLock.unlock();
        consumers_enrolled_condition_.notify_all();
    }

    /***************************************************************************/
    /**
	  * Adds multiple consumers to this class and starts their threads.
	  *
	  * @param gc_ptr_cnt A vector of pointers to GBaseConsumerT<processable_type> objects
	  */
    void
    enrol_consumer_vec(std::vector<std::shared_ptr<cons::GBaseConsumerT<processable_type>>> gc_ptr_cnt) {
        //-----------------------------------------------------------------------
        std::unique_lock<std::mutex> consumerEnrolmentLock(consumer_enrolment_mutex_);

        // Check whether consumers have already been enrolled. As this may happen
        // only once, we emit a warning and return.
        // Check is inside the lock to prevent a TOCTOU race between concurrent callers.
        if(consumers_present_) {
            glogger << "In GBrokerT<>::enrol_buffer_port(consumer_ptr_vec): One or more consumers "
                       "have already been enrolled."
                    << '\n'
                    << "We will ignore the new enrolment request." << '\n'
                    << GWARNING;

            return;
        }

        for(auto const &consumer_ptr : gc_ptr_cnt) {
            // Do nothing if a consumer of this type has already been registered
            if(std::find(
                   consumer_types_present_.begin(),
                   consumer_types_present_.end(),
                   consumer_ptr->getConsumerName()
               ) != consumer_types_present_.end()) {
                glogger
                    << "In GBrokerT<>::enrol_buffer_port(consumer_ptr_vec): A consumer with name "
                    << consumer_ptr->getConsumerName() << '\n'
                    << "has already been enrolled. We will ignore the new enrolment request."
                    << '\n'
                    << GWARNING;

                continue;
            }

            // Archive the consumer and its name, then start its thread
            consumer_collection_cnt_.push_back(consumer_ptr);
            consumer_types_present_.push_back(consumer_ptr->getConsumerName());

            // Initiate processing in the consumer. This call will not block.
            consumer_ptr->async_startProcessing();
        }

        //-----------------------------------------------------------------------
        // Make it known to subsequent calls that a consumer is already present

        consumers_present_.store(true);

        //-----------------------------------------------------------------------

        // Check whether all registered consumers are capable of full return
        capable_of_full_return_.store(this->checkConsumersCapableOfFullReturn());

        // Notify outside the lock to avoid immediately re-blocking the woken thread
        consumerEnrolmentLock.unlock();
        consumers_enrolled_condition_.notify_all();
    }

    /***************************************************************************/
    /**
	  * Retrieves a "raw" item from a GBufferPortT. This function will block
	  * if no item can be retrieved.
	  *
	  * @param p Holds the retrieved "raw" item
	  */
    void get(std::shared_ptr<processable_type> &p) {
        // Make sure we are dealing with an empty pointer
        p.reset();

        // Retrieve the current buffer port ...
        auto rawBuffer_ptr = getNextRawBufferPort();
        if(rawBuffer_ptr) {
            // ... and get an item from it. This function is thread-safe.
            rawBuffer_ptr->pop_raw(p);
        }

        // If no raw buffer pointer was registered at the time
        // of the getNextRawBufferPort()-call, p will be empty.
    }

    /***************************************************************************/
    /**
	  * Retrieves a "raw" item from a GBufferPortT, observing a timeout. Note that upon
	  * time-out an exception is thrown.
	  *
	  * @param p Holds the retrieved "raw" item
	  * @param timeout Time after which the function should time out
	  * @return A boolean which indicates whether the operation was successful
	  */
    bool get(std::shared_ptr<processable_type> &p, std::chrono::duration<double> timeout) {
        // Make sure we are dealing with an empty pointer
        p.reset();

        // Retrieve the current buffer port ...
        auto rawBuffer_ptr = getNextRawBufferPort();
        if(rawBuffer_ptr) {
            // ... and get an item from it. This function is thread-safe.
            rawBuffer_ptr->pop_raw(p, timeout); // Note that p might be empty
        }

        // If no raw buffer pointer was registered at the time
        // of the getNextRawBufferPort()-call, p will be empty.

        if(not p) {
            return false;
        }

        return true;
    }

    /***************************************************************************/
    /**
	  * Puts a processed item into the processed queue. Note that the item will simply
	  * be discarded if no target queue with the required id exists. The function will
	  * block otherwise, until it is again possible to submit the item.
	  *
	  * @param p Holds the "raw" item to be submitted to the processed queue
	  */
    void put(std::shared_ptr<processable_type> p) {
        // Retrieve the correct processed buffer for a given id
        auto portId = p->getBufferId();
        auto processedBuffer_ptr = getProcessedBufferPort(portId);

        // Submit the item
        if(processedBuffer_ptr) {
            // This function is thread-safe.
            processedBuffer_ptr->push_processed(p);
        }
        else {
            glogger << "In GBokerT<>::put(1): Warning!" << '\n'
                    << "Did not find buffer with id " << portId << "." << '\n'
                    << "Item will be discarded" << '\n'
                    << GWARNING;

            throw Gem::Courtier::buffer_not_present();
        }
    }

    /***************************************************************************/
    /**
	  * Puts a processed item into the processed queue, observing a timeout. The function
	  * will throw a Gem::Courtier::buffer_not_present exception if the requested buffer
	  * isn't present. The function will return false if no item could be added to the buffer
	  * inside if the allowed time limits.
	  *
	  * @param id A key that uniquely identifies the origin of p
	  * @param p Holds the item to be submitted to the processed queue
	  * @param timeout Time after which the function should time out
	  * @param A boolean indicating whether the item could be added to the queue in time
	  */
    bool put(std::shared_ptr<processable_type> p, std::chrono::duration<double> timeout) {
        // Retrieve the correct processed buffer for our id
        auto portId = p->getBufferId();
        auto processedBuffer_ptr = getProcessedBufferPort(portId);

        // Submit the item
        if(processedBuffer_ptr) {
            // This function is thread-safe.
            return processedBuffer_ptr->push_processed(p, timeout);
        }
                    glogger << "In GBokerT<>::put(1): Warning!" << '\n'
                    << "Did not find buffer with id " << portId << "." << '\n'
                    << "Item will be discarded" << '\n'
                    << GWARNING;

            throw Gem::Courtier::buffer_not_present();
       

        // Make the compiler happy
        return false;
    }

    /***************************************************************************/
    /**
	  * Checks whether any consumers have been enrolled at the time of calling.
	  *
	  * @return A boolean indicating whether any consumers are registered
	  */
    bool hasConsumers() const {
        return consumers_present_;
    }

    /***************************************************************************/
    /**
	  * This function relies on a prior check during the enrolment process whether
	  * all registered consumers are capable of full return. It will block until
	  * a consumer has been registered. The lock will be either released by the
	  * condition variable or when the function is left, so enrolling of consumers
	  * is not prevented.
	  */
    bool capableOfFullReturn() const {
        std::unique_lock<std::mutex> consumerEnrolmentLock(consumer_enrolment_mutex_);
        if(not consumers_present_) {
            consumers_enrolled_condition_.wait(consumerEnrolmentLock, [this]() -> bool {
                return this->hasConsumers();
            });
        }

        return capable_of_full_return_;
    }

private:
    /***************************************************************************/
    /**
	  * Retrieves the next raw buffer port pointer. As we are dealing with a
	  * (not thread-safe) std::map, we need to coordinate the access.
	  */
    GBUFFERPORT_PTR getNextRawBufferPort() {
        // Protect access to the iterator
        std::unique_lock<std::mutex> switchGetPositionLock(switch_get_position_mutex_);

        if(not raw_buffers_.empty()) {
            // Save the current get position
            auto currentGetPosition = current_get_position_;

            // Switch to the next position, if any
            if((raw_buffers_.size() > 1) && (++current_get_position_ == raw_buffers_.end())) {
                current_get_position_ = raw_buffers_.begin();
            }

            // Return the shared_ptr. This will also keep the buffer port alive
            return currentGetPosition->second;
        }
                    return GBUFFERPORT_PTR();
       
    }

    /***************************************************************************/
    /**
	  * Retrieves the processed buffer pointer for a given id. As we are dealing
	  * with a (possibly thread-unsafe) std::map, we need to synchronize the access.
	  */
    GBUFFERPORT_PTR getProcessedBufferPort(BUFFERPORT_ID_TYPE id) {
        // Protect access to the map
        std::unique_lock<std::mutex> findProcessedBufferLock(find_procesed_buffer_mutex_);

        // Find the buffer port (if any)
        try {
            return processed_buffers_.at(id);
        }
        catch(const std::out_of_range &) {
            // Return an empty pointer
            return GBUFFERPORT_PTR();
        }
    }

    /***************************************************************************/
    /**
	  * Checks if all registered consumers are capable of full return. This
	  * function is not thread-safe and must be called in a controlled environment.
	  *
	  * @return A boolean indicating whether all registered consumers are capable of full return
	  */
    bool checkConsumersCapableOfFullReturn() {
        if(consumer_collection_cnt_.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GBrokerT<processable_type>::checkConsumersCapableOfFullReturn(): Error!"
                << '\n'
                << "No consumers registered" << '\n'
            );
        }

        bool capable_of_full_return = true;
        for(auto const &item_ptr : consumer_collection_cnt_) {
            if(not item_ptr->capableOfFullReturn()) {
                capable_of_full_return = false;
                break; // stop the loop
            }
        }

        return capable_of_full_return;
    }

    /***************************************************************************/
    /**
	  * Retrieval of an id to be assigned to the next registered buffer port.
	  * This function is only used within a locked environment, so we can safely
	  * increment and compare the id. As the id may roll over and
	  * buffer ports may have different lifetimes, we need to make sure the chosen
	  * buffer port id hasn't been used yet. We do this by searching through the
	  * list of registered buffer ports
	  */
    BUFFERPORT_ID_TYPE getNextBufferPortId() {
        bool id_in_use = false;
        BUFFERPORT_ID_TYPE next_id =
            current_bufferport_id_; // NOLINT(cppcoreguidelines-init-variables)

        do {
            id_in_use = false;
            next_id = current_bufferport_id_;

            // Reset the id, if we have reached the maximum size
            if(++current_bufferport_id_ == (std::numeric_limits<BUFFERPORT_ID_TYPE>::max)()) {
                current_bufferport_id_ = 0;
            }

            // Check if the id is already being used
            for(const auto &port : raw_buffers_) {
                if(port.first == next_id) {
                    id_in_use = true;
                    break;
                }
            }
        }
        while(id_in_use);

        return next_id;
    }

    /***************************************************************************/
    // Data

    std::atomic<bool> finalized_{
        false
    }; ///< Indicates whether the finalization code has already been executed

    mutable std::mutex consumer_enrolment_mutex_;  ///< Protects the enrolment of consumers
    mutable std::mutex switch_get_position_mutex_;  ///< Protects switches to the next get position
    mutable std::mutex find_procesed_buffer_mutex_; ///< Protects finding a given processed buffer

    mutable std::condition_variable
        consumers_enrolled_condition_; ///< Allows to notify interested parties once consumers have been enrolled

    RawBufferPtrMap raw_buffers_;             ///< Holds a std::map of buffer pointers
    ProcessedBufferPtrMap processed_buffers_; ///< Holds a std::map of buffer pointers

    typename RawBufferPtrMap::iterator current_get_position_{
        raw_buffers_.begin()
    }; ///< The current get position in the raw_buffers_ collection
    std::atomic<bool> buffers_present_{
        false
    }; ///< Set to true once the first buffers have been enrolled

    std::atomic<bool> consumers_present_{
        false
    }; ///< Set to true once one or more consumers have been enrolled
    std::atomic<bool> capable_of_full_return_{
        false
    }; ///< Set to true if all registered consumers are capable of full return, otherwise false

    std::vector<std::shared_ptr<cons::GBaseConsumerT<processable_type>>>
        consumer_collection_cnt_; ///< Holds the actual consumers
    std::vector<std::string>
        consumer_types_present_; ///< Holds identifying strings for each consumer

    std::atomic<BUFFERPORT_ID_TYPE> current_bufferport_id_{
        static_cast<BUFFERPORT_ID_TYPE>(0)
    }; ///< The id assigned to the last registered buffer port
    std::atomic<BUFFERPORT_ID_TYPE> n_registered_buffer_ports_{
        static_cast<BUFFERPORT_ID_TYPE>(0)
    }; ///< The current number of registered buffer ports
};

/******************************************************************************/
/**
 * We require GBrokerT<T> to be a singleton. This ensures that, for a given T, one
 * and only one Broker object exists that is constructed before main begins. All
 * external communication should refer to GBROKER(T).
 */
#define GBROKER(T)      Gem::Common::GSingletonT<Gem::Courtier::GBrokerT<T>>::Instance(0)
#define RESETGBROKER(T) Gem::Common::GSingletonT<Gem::Courtier::GBrokerT<T>>::Instance(1)

/******************************************************************************/

} /* namespace Gem::Courtier */
