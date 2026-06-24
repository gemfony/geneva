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

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

// Boost headers go here

// Geneva headers go here
#include "common/GLogger.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomDefines.hpp"

namespace Gem::Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Access to different random number distributions, whose "raw" material is
 * produced in different ways. We only define the interface here. The actual
 * implementation can be found in the (partial) specializations of this class.
 */
template <Gem::Hap::randomSource s = Gem::Hap::randomSource::QUEUE>
class GRandomT : public Gem::Hap::GRandomBase {
public:
    /***************************************************************************/
    // This class is not meant to be used.

    GRandomT() = delete;

    GRandomT(GRandomT const &) = delete;
    GRandomT(GRandomT &&) = delete;

    GRandomT &operator=(GRandomT const &) = delete;
    GRandomT &operator=(GRandomT &&) = delete;

    /***************************************************************************/

    ~GRandomT() override = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class retrieves random numbers
 * in batches from a global random number factory. The functions provided by
 * GRandomBase then produce different types of random numbers from this raw material.
 * Copy and move are explicitly deleted; it is not possible to assign other
 * objects or use copy constructors.
 */
template <>
class GRandomT<Gem::Hap::randomSource::QUEUE> : public Gem::Hap::GRandomBase {
public:
    /***************************************************************************/
    /**
	 * @brief Default constructor; acquires the factory and a first random number package.
	 *
	 * Note that getNewRandomContainer() may throw.
	 */
    GRandomT() noexcept(false)
      : grf_(randomFactory()) // Make sure we have a local pointer to the factory
    {
        // Make sure we have a first random number package available
        this->getNewRandomContainer();
    }

    /***************************************************************************/
    /**
	 * @brief The standard destructor; returns the held package to the factory for recycling.
	 */
    ~GRandomT() override {
        if(p_) {
            grf_->returnUsedPackage(std::move(p_));
        }
        grf_.reset();
    }

    /***************************************************************************/
    /**
	 * @brief Copy construction is identical to default construction.
	 *
	 * Every instance should hold a unique set of random numbers, so the source
	 * is ignored and a fresh container is obtained via a delegating constructor.
	 *
	 * @param cp The object to be "copied" (unused; present only for interface compatibility)
	 */
    GRandomT([[maybe_unused]] GRandomT<Gem::Hap::randomSource::QUEUE> const & cp) noexcept(false)
      : GRandomT<Gem::Hap::randomSource::QUEUE>() { /* nothing */
    }

    /***************************************************************************/
    /**
	 * @brief Move construction. Note that getNewRandomContainer() may throw.
	 *
	 * Steals the source's random number container, then re-supplies the source
	 * with a fresh container so it remains usable.
	 *
	 * @param cp The object whose random number container is moved from (left in a pristine state)
	 */
    GRandomT(GRandomT<Gem::Hap::randomSource::QUEUE> &&cp) noexcept(false)
      : p_(std::move(cp.p_))
      , grf_(randomFactory()) // Make sure we have a local pointer to the factory
    {
        // Make sure cp is in pristine condition -- we need to give it a new random number container
        cp.getNewRandomContainer();
    }

    /***************************************************************************/
    /**
	 * @brief Copy assignment -- a no-op.
	 *
	 * Each instance keeps its own unique set of random numbers, so nothing is
	 * copied (compare the copy constructor).
	 *
	 * @param cp The object to be "assigned" (unused; present only for interface compatibility)
	 * @return A reference to this object
	 */
    GRandomT<Gem::Hap::randomSource::QUEUE> &
    operator=([[maybe_unused]] GRandomT<Gem::Hap::randomSource::QUEUE> const & cp) noexcept(false) {
        return *this;
    }

    /***************************************************************************/
    /**
	 * @brief Move assignment.
	 *
	 * Takes over the source's random number container (keeping this object's own
	 * factory pointer) and re-supplies the source with a fresh container.
	 *
	 * @param cp The object whose random number container is moved from (re-initialized afterwards)
	 * @return A reference to this object
	 */
    GRandomT<Gem::Hap::randomSource::QUEUE> &
    operator=(GRandomT<Gem::Hap::randomSource::QUEUE> &&cp) noexcept(false) {
        p_ = std::move(cp.p_);
        // We keep our own pointer to the random factory

        // Re-initialize the random-number container of the remote class
        cp.p_.reset();
        cp.getNewRandomContainer();

        return *this;
    }

    /***************************************************************************/
    /**
	 * @brief Retrieves the id of the currently running thread.
	 *
	 * This function exists mostly for debugging purposes.
	 *
	 * @return The std::thread::id of the calling thread
	 */
    static std::thread::id getThreadId() {
        return std::this_thread::get_id();
    }

private:
    /***************************************************************************/
    /**
	 * This function retrieves random number packages from a global
	 * factory and emits them one by one. Once a package has been fully
	 * used, it is discarded and a new package is obtained from the factory.
	 * Essentially this class thus acts as a random number proxy -- to the
	 * caller it appears as if random numbers are created locally. This function
	 * assumes that a valid container is already available.
	 *
	 * @return The next raw random value, transparently refilling from the factory when exhausted
	 */
    GRandomBase::result_type int_random() override {
        if(p_->empty()) {
            // Get rid of the old container ...
            grf_->returnUsedPackage(std::move(p_));
            // ... then get a new one
            getNewRandomContainer();
        }
        return p_->next();
    }

    /***************************************************************************/
    /**
	 * @brief (Re-)Initialization of the local random number container p_.
	 *
	 * Checks (in DEBUG builds) that a valid GRandomFactory still exists, then
	 * retries getNewRandomContainer() on the factory until a valid container is
	 * obtained (each factory call has an internal timeout).
	 */
    void getNewRandomContainer() {
        // Make sure we get rid of the old container
        // p_.reset(); No longer needed with std::unique_ptr

#ifdef DEBUG
        if(not grf_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GRandomT<QUEUE>::getNewRandomContainer(): Error!" << '\n'
                << "No connection to GRandomFactory object." << '\n'
            );
        }
#endif /* DEBUG */

#ifdef DEBUG
        std::uint32_t n_retries = 0;
#endif /* DEBUG */

        // Try until a valid container has been received. new01Container has
        // a timeout of DEFAULTFACTORYGETWAIT internally.
        while(not(p_ = grf_->getNewRandomContainer())) {
#ifdef DEBUG
            n_retries++;
#endif /* DEBUG */
        }

#ifdef DEBUG
        if(n_retries > 1) {
            std::cout << "Info: Had to try " << n_retries
                      << " times to retrieve a valid random number container." << '\n';
        }
#endif /* DEBUG */
    }

    /***************************************************************************/
    /** @brief Holds the container of uniform random numbers */
    std::unique_ptr<random_container> p_;
    /** @brief A local copy of the global GRandomFactory */
    std::shared_ptr<Gem::Hap::GRandomFactory> grf_;
};

/** @brief Convenience typedef */
using GRandom = GRandomT<Gem::Hap::randomSource::QUEUE>;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This specialization of the general GRandomT<> class produces random numbers
 * locally. The functions provided by GRandomBase<> then produce different types
 * of random numbers from this raw material. A seed can be provided either to
 * the constructor, or is taken from the global seed manager (recommended) in
 * case the default constructor is used.
 */
template <>
class GRandomT<Gem::Hap::randomSource::LOCAL> : public Gem::Hap::GRandomBase {
public:
    /***************************************************************************/
    /**
	 * @brief The standard constructor; seeds the local engine from the global seed manager.
	 */
    GRandomT() noexcept(false)
      : rng_(randomFactory()->getSeed()) { /* nothing */
    }

    /***************************************************************************/
    /**
	 * @brief Copy construction does nothing but delegate to the default constructor.
	 *
	 * Each instance gets its own freshly seeded local engine, so the source is ignored.
	 *
	 * @param cp The object to be "copied" (unused; present only for interface compatibility)
	 */
    GRandomT([[maybe_unused]] GRandomT<Gem::Hap::randomSource::LOCAL> const & cp) noexcept(false)
      : GRandomT<Gem::Hap::randomSource::LOCAL>() { /* nothing */
    }

    /***************************************************************************/
    /**
	 * @brief Move construction does nothing but delegate to the default constructor.
	 *
	 * Each instance gets its own freshly seeded local engine, so the source is ignored.
	 *
	 * @param cp The object to be "moved" from (unused; present only for interface compatibility)
	 */
    GRandomT([[maybe_unused]] GRandomT<Gem::Hap::randomSource::LOCAL> && cp) noexcept(false)
      : GRandomT<Gem::Hap::randomSource::LOCAL>() { /* nothing */
    }

    /***************************************************************************/
    /**
	 * The standard destructor
	 */
    ~GRandomT() override = default;

    /***************************************************************************/
    /**
	 * @brief Copy-assignment does nothing.
	 *
	 * Each instance owns its independent, locally seeded engine, so nothing is copied.
	 *
	 * @param cp The object to be "assigned" (unused; present only for interface compatibility)
	 * @return A reference to this object
	 */
    GRandomT<Gem::Hap::randomSource::LOCAL> &
    operator=([[maybe_unused]] GRandomT<Gem::Hap::randomSource::LOCAL> const & cp) noexcept(
        false
    ) // NOLINT(cert-oop54-cpp) — intentionally trivial: each instance owns independent state
    {
        return *this;
    }

    /***************************************************************************/
    /**
	 * @brief Move-assignment does nothing.
	 *
	 * Each instance owns its independent, locally seeded engine, so nothing is moved.
	 *
	 * @param cp The object to be "moved" from (unused; present only for interface compatibility)
	 * @return A reference to this object
	 */
    GRandomT<Gem::Hap::randomSource::LOCAL> &
    operator=([[maybe_unused]] GRandomT<Gem::Hap::randomSource::LOCAL> && cp) noexcept(false) {
        return *this;
    }

private:
    /***************************************************************************/
    /**
	 * @brief This function produces uniform random numbers locally.
	 *
	 * @return One raw random value drawn directly from the instance-local engine
	 */
    GRandomBase::result_type int_random() override {
        return rng_();
    }

    /***************************************************************************/
    /** @brief The actual generator for local random number creation */
    G_CPU_BASE_GENERATOR rng_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Hap */
