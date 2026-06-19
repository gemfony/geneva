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

#include "hap/GRandomFactory.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "hap/GRandomDefines.hpp"
#if defined(HAP_AVX2_BACKEND) || defined(HAP_NEON_BACKEND)
#include "hap/GXoshiro256ppSIMD.hpp" // SIMD bulk-refill engine
#endif
#if defined(HAP_USE_CUDA)
#include "hap/GCUDARng.hpp" // GPU bulk-refill backend (cuRAND host API)
#include <optional>
#endif
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <utility>

namespace Gem::Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization of static data members
 */
std::atomic<bool> GRandomFactory::multiple_call_trap_{false};

/******************************************************************************/
/**
 * @brief The standard constructor.
 *
 * Enforces the single-instantiation contract via the static
 * multiple_call_trap_ flag: a second construction throws a geneva_exception.
 */
GRandomFactory::GRandomFactory() {
    /*
	 * Apparently the entropy() call currently always returns 0 with g++ and clang,
	 * as this call is not fully implemented.
	 *
	// Check whether enough entropy is available. Warn, if this is not the case
	if (0. == multiple_call_trap_.entropy()) {
		glogger
		<< "In GSeedManager::GSeedManager(): Error!" << std::endl
		<< "Source of non-deterministic random numbers" << std::endl
		<< "has entropy 0." << std::endl
		<< GWARNING;
	}
	*/

    if(multiple_call_trap_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "Error in GRandomFactory::GRandomFactory():" << '\n'
            << "Class has been instantiated before." << '\n'
            << "and may be instantiated only once" << '\n'
        );
    }
            multiple_call_trap_.store(true);
   
}

/******************************************************************************/
/**
 * @brief The destructor. All work is done in the finalize() function.
 */
GRandomFactory::~GRandomFactory() {
    // Make sure the finalization code is executed
    // (if this hasn't happened already). Calling
    // finalize() multiple times is safe.
    finalize();
}

/******************************************************************************/
/**
 * @brief Initializes the factory.
 *
 * This function does nothing at this time. Its only purpose is to control
 * initialization of the factory in the singleton.
 */
void GRandomFactory::init() { /* nothing */
}

/******************************************************************************/
/**
 * @brief Finalization code for the GRandomFactory.
 *
 * All producer threads are flagged to stop and the fresh/return buffers are
 * closed to wake any producer parked in a blocking push(); the function then
 * waits for the threads to join. This performs useful work only on the first
 * call and returns immediately on subsequent calls, so it can be called as
 * often as you wish.
 */
void GRandomFactory::finalize() {
    // Only allow one finalization action to be carried out
    if(finalized_) {
        return;
    }

    // Flag all threads to stop
    threads_stop_requested_.store(true);
    // Wake any producer parked in a blocking push() so it observes the stop flag
    // and exits -- otherwise join_all() below would deadlock on a full buffer.
    p_fresh_bfr_.close();
    p_ret_bfr_.close();
    // Wait for all threads to return
    producer_threads_.join_all();

    // Let the audience know
    finalized_.store(true);
}

/******************************************************************************/
/**
 * @brief Allows to retrieve the size of random number arrays.
 *
 * @return The size of a random number array (DEFAULTARRAYSIZE)
 */
std::size_t GRandomFactory::getCurrentArraySize() const {
    return DEFAULTARRAYSIZE;
}

/******************************************************************************/
/**
 * @brief Retrieves the size of the random buffer.
 *
 * The random buffer is the data structure holding the random number packages.
 *
 * @return The size of the random buffer (DEFAULTFACTORYBUFFERSIZE)
 */
std::size_t GRandomFactory::getBufferSize() const {
    return DEFAULTFACTORYBUFFERSIZE;
}

/******************************************************************************/
/**
 * @brief Returns a seed from a pseudo-random sequence.
 *
 * Access is serialised by seeding_mutex_; the local seed collection is
 * (re)generated from the seed_seq_ object on first use and once it has been
 * exhausted.
 *
 * @return A seed taken from a local seed_seq object
 */
seed_type GRandomFactory::getSeed() {
    std::unique_lock<std::mutex> sm_lck(seeding_mutex_);

    // Refill at the start of seeding or when all seeds have been used
    if(not seeding_has_started_ || seed_cit_ == seed_collection_.end()) {
        seed_seq_.generate(seed_collection_.begin(), seed_collection_.end());
        seed_cit_ = seed_collection_.begin();
        seeding_has_started_.store(true);
    }

    seed_type result = *seed_cit_;
    ++seed_cit_;

    return result;
}

/******************************************************************************/
/**
 * @brief Allows recycling of (possibly partially used) packages.
 *
 * This way we avoid the continuous allocation and deletion of new buffers.
 * Note that this function may delete its argument (via reset) if it cannot be
 * added to the return buffer.
 *
 * @param p An rvalue unique_ptr to a partially used work package; ownership is taken (moved into the return buffer or reset)
 */
void GRandomFactory::returnUsedPackage(std::unique_ptr<random_container> &&p) {
    // We try to add the item to the p_ret_bfr_ queue.
    if(not p_ret_bfr_.try_push(std::move(p))) {
        p.reset();
    }
}

/******************************************************************************/
/**
 * @brief Sets the number of producer threads for this factory.
 *
 * See also http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/
 * for the rationale of the double-checked locking pattern. Note that only an
 * increase of the number of threads is allowed when threads are already
 * running; a requested decrease is ignored with a warning, and a request for 0
 * threads falls back to the default DEFAULT01PRODUCERTHREADS.
 *
 * @param n_producer_threads The requested number of threads simultaneously producing random numbers
 */
void GRandomFactory::setNProducerThreads(const std::uint16_t &n_producer_threads) {
    // Threads might already be running, so we need to regulate access
    if(threads_started_) {
        // If we enter this code-path, there is no way threads
        // could go into the "not-running" state, so we do not need
        // to check again using DCLP .
        std::unique_lock<std::mutex> lk(thread_creation_mutex_);
        // Make a suggestion for the number of threads, if requested
        std::uint16_t n_producer_threads_local = DEFAULT01PRODUCERTHREADS;
        if(0 == n_producer_threads) {
            glogger << "In GRandomFactory::setNProducerThreads(n_producer_threads) / 1:" << '\n'
                    << "n_producer_threads == 0 was requested. n_producer_threads_local was set to the "
                       "default "
                    << DEFAULT01PRODUCERTHREADS << '\n'
                    << GWARNING;
        }
        else {
            n_producer_threads_local = n_producer_threads;
        }

        if(n_producer_threads_local > n_producer_threads_.load()) { // start new 01 threads
            for(std::uint16_t i = n_producer_threads_.load(); i < n_producer_threads_local;
                i++) { // NOLINT(cppcoreguidelines-init-variables)
                producer_threads_.create_thread([this]() { this->producer(this->getSeed()); });
            }
        }
        else if(
            n_producer_threads_local < n_producer_threads_.load()
        ) { // We need to remove threads
            glogger
                << "In GRandomFactory::setNProducerThreads(" << n_producer_threads << "): Warning!"
                << '\n'
                << "Attempt to decrease the number of producer threads from "
                << n_producer_threads_.load() << " to " << n_producer_threads << '\n'
                << "while threads were alredy running. The number of threads will remain unchanged."
                << '\n'
                << GWARNING;

            return;
        }
    }
    else { // Double-checked locking pattern
        // Here it appears that no threads were running. We do need to check again, though (DLCP)
        std::unique_lock<std::mutex> tc_lk(thread_creation_mutex_);
        // Make a suggestion for the number of threads, if requested
        std::uint16_t n_producer_threads_local = DEFAULT01PRODUCERTHREADS;
        if(n_producer_threads == 0) {
            glogger << "In GRandomFactory::setNProducerThreads(n_producer_threads) / 2:" << '\n'
                    << "n_producer_threads == 0 was requested. n_producer_threads_local was set to the "
                       "default "
                    << DEFAULT01PRODUCERTHREADS << '\n'
                    << GWARNING;
        }
        else {
            n_producer_threads_local = n_producer_threads;
        }

        if(threads_started_) { // Someone has started the threads in the meantime. Adjust the number of threads
            if(n_producer_threads_local > n_producer_threads_.load()) { // start new 01 threads
                for(std::uint16_t i = n_producer_threads_.load(); i < n_producer_threads_local;
                    i++) { // NOLINT(cppcoreguidelines-init-variables)
                    producer_threads_.create_thread([this]() { this->producer(this->getSeed()); });
                }
            }
            else if(
                n_producer_threads_local < n_producer_threads_.load()
            ) { // We need to remove threads
                glogger << "In GRandomFactory::setNProducerThreads(" << n_producer_threads
                        << "): Warning!" << '\n'
                        << "Attempt to decrease the number of producer threads from "
                        << n_producer_threads_.load() << " to " << n_producer_threads << '\n'
                        << "while threads were alredy running. The number of threads will remain "
                           "unchanged."
                        << '\n'
                        << GWARNING;

                return;
            }
        }

        // Whether they were already running or not -- we may now adjust the number of producer threads
        n_producer_threads_ = n_producer_threads_local;
    }
}

/******************************************************************************/
/**
 * @brief Hands out a new container of random numbers.
 *
 * When objects need a new container of [0,1[ random numbers with the current
 * default size, they call this function. The producer threads are started on
 * first access (double-checked locking; see
 * http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/ for
 * the rationale). A fresh container is popped from the buffer with a bounded
 * wait.
 *
 * @return A packet of new [0,1[ random numbers, or an empty unique_ptr on timeout
 */
std::unique_ptr<random_container> GRandomFactory::getNewRandomContainer() {
    // Start the producer threads upon first access to this function
    if(not threads_started_) {
        std::unique_lock<std::mutex> tc_lk(thread_creation_mutex_);
        if(not threads_started_) { // double checked locking pattern
            //---------------------------------------------------------
            for(std::uint16_t i = 0; i < n_producer_threads_.load(); i++) {
                producer_threads_.create_thread([this]() { this->producer(this->getSeed()); });
            }
            //---------------------------------------------------------

            threads_started_.store(true);
        }
    }

    std::unique_ptr<random_container> p; // empty
    if(auto popped = p_fresh_bfr_.pop_wait(std::chrono::milliseconds(DEFAULTFACTORYGETWAIT))) {
        p = std::move(*popped);
    }
    // On timeout p stays empty -- our way of signaling a time out is an empty std::unique_ptr

    return p;
}

/******************************************************************************/
/**
 * @brief The production of [0,1[ random numbers takes place here.
 *
 * Runs as the body of a producer std::thread: it (re)fills containers from the
 * active backend -- the SIMD engine when an AVX2/NEON backend is compiled in,
 * the GPU (cuRAND) when CUDA support is built and a device is present, else the
 * scalar CPU engine -- and submits them to the fresh buffer with a blocking
 * push until shutdown closes the buffer. As this function is the body of a
 * std::thread, no exception may escape it (that would call std::terminate() and
 * bring the whole process down); every exception is therefore caught, logged as
 * a warning, and this producer thread exits cleanly while the remaining
 * producer threads keep supplying numbers.
 *
 * @param seed A seed for this producer thread's local random number generator
 */
void GRandomFactory::producer(std::uint32_t seed) {
    try {
        // When a SIMD backend is compiled in, the producer refills containers
        // with the vectorised engine (bulk generate()). The public engine type
        // (G_CPU_BASE_GENERATOR) stays scalar so the headers are consistent for
        // every consumer; the SIMD type lives only in this (library-private)
        // translation unit. Without a SIMD backend the scalar engine both
        // refills and serves the local generator.
#if defined(HAP_AVX2_BACKEND) || defined(HAP_NEON_BACKEND)
        using RefillEngine = xoshiro256pp_simd;
#else
        using RefillEngine = G_CPU_BASE_GENERATOR;
#endif
        RefillEngine mt(static_cast<RefillEngine::result_type>(seed));

        // When built with CUDA support and a GPU device is present at run time,
        // containers are refilled on the GPU via cuRAND; otherwise the CPU engine
        // above is used. The choice is made once per producer thread, with a
        // clean fallback when no device is available.
#if defined(HAP_USE_CUDA)
        const bool              useCuda = GCudaRNG::deviceAvailable();
        std::optional<GCudaRNG> cuda;
        if(useCuda) {
            cuda.emplace(static_cast<std::uint64_t>(seed));
            glogger << "In GRandomFactory::producer(): CUDA device present;"
                    << " refilling random-number containers via cuRAND." << '\n'
                    << GLOGGING;
        }
#else
        [[maybe_unused]] constexpr bool useCuda = false;
#endif

        // Fills (fresh=true) or refreshes (fresh=false) a container from the
        // active backend (GPU when useCuda, else the CPU engine).
        auto fill = [&](std::unique_ptr<random_container> &cont, bool fresh) {
#if defined(HAP_USE_CUDA)
            if(useCuda) {
                if(fresh) { cont.reset(new random_container(*cuda));
                } else {      cont->refresh(*cuda);
}
                return;
            }
#endif
            if(fresh) { cont.reset(new random_container(mt));
            } else {      cont->refresh(mt);
}
        };

        std::unique_ptr<random_container> p;

        while(not threads_stop_requested_) {
            // First we try to retrieve a "recycled" item from the p_ret_bfr_ buffer. If this
            // fails (likely because the buffer is empty), we create a new item instead
            if(auto recycled = p_ret_bfr_.try_pop()) {
                p = std::move(*recycled);
                // If we reach this line, we have successfully retrieved a recycled container.
                // First do some error-checking
#ifdef DEBUG
                if(not p) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In RandomFactory::producer(): Error!" << '\n'
                        << "Got empty recycling pointer" << '\n'
                    );
                }

#endif /* DEBUG */

                // Replace "used" random numbers with new ones
                fill(p, /*fresh=*/false);
            }
            else { // O.k., so we need to create a new container
                fill(p, /*fresh=*/true);
            }

            // Blocking submit: sleep on the buffer's not-full condition until a
            // consumer frees space, or until finalize() closes the buffer at
            // shutdown (push() then returns false, and we leave the loop). No
            // timeout polling -- producers stay fully asleep while the buffer is full.
            if(not p_fresh_bfr_.push(std::move(p))) {
                break; // buffer closed at shutdown -- leave the producer loop
            }
        }
    }
    // producer() is the body of a std::thread, so no exception may escape it:
    // an exception leaving a thread's top-level function calls std::terminate()
    // and crashes the whole process. We log the condition (so it is not silent)
    // and return, letting this producer thread exit cleanly; the remaining
    // producer threads keep supplying random numbers.
    catch(const std::exception &e) {
        glogger
            << "In GRandomFactory::producer(): Warning!" << '\n'
            << "Caught an exception with message" << '\n'
            << e.what() << '\n'
            << "This producer thread will now terminate cleanly. Random-number" << '\n'
            << "production continues on the remaining producer threads." << '\n'
            << GWARNING;
    }
    catch(...) {
        glogger
            << "In GRandomFactory::producer(): Warning!" << '\n'
            << "Caught an unknown exception." << '\n'
            << "This producer thread will now terminate cleanly. Random-number" << '\n'
            << "production continues on the remaining producer threads." << '\n'
            << GWARNING;
    }
}

/******************************************************************************/

} /* namespace Gem::Hap */
