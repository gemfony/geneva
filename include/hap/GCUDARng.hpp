/**
 * @file GCUDARng.hpp
 */

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
#include <cstdint>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>
#include <iostream>

// Boost headers go here

// Geneva headers go here

namespace Gem::Hap
{
    /**
     * GCudaRNG is a class that provides a C++20-conforming uniform random generator interface.
     * Internally, it produces random numbers on a CUDA-enabled GPU and buffers them in a pool on the host.
     * If more numbers are requested than are available in the pool, the generator will block until enough
     * numbers have been produced. If too many numbers accumulate, production will be reduced.
     */
    class GCudaRNG
    {
    public:
        // Required by the UniformRandomBitGenerator concept
        using result_type = std::uint32_t;

        static constexpr result_type min()
        {
            return std::numeric_limits<result_type>::min();
        }

        static constexpr result_type max()
        {
            return std::numeric_limits<result_type>::max();
        }

        // @param poolCapacity     Maximum number of cached random values on the host
        // @param initialBatchSize Initial number of random values generated per GPU batch
        GCudaRNG(std::size_t poolCapacity = 1'000'000, std::size_t initialBatchSize = 100'000);

        ~GCudaRNG();

        // Retrieves the next random number (may block if the pool is empty).
        result_type operator()();

    private:
        // The background thread that continuously generates random numbers on the GPU.
        void productionLoop();

        // A friend helper function declared here for clarity; defined in GCudaRNG.cpp.
        friend void generateGpuRandomNumbers(std::size_t n, std::vector<std::uint32_t>& hostBuffer);

    private:
        // Host pool of random numbers
        std::deque<result_type> m_pool;
        const std::size_t m_poolCapacity;

        // Dynamic batch size
        std::atomic<std::size_t> m_batchSize;

        // Synchronization
        std::mutex m_mutex;
        std::condition_variable m_cv;

        bool m_stop;
        std::thread m_productionThread;
    };
} /* namespace Gem::Hap */
