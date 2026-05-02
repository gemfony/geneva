/**
 * @file GCUDARng.cpp
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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "hap/GCUDARng.hpp"

#include <cuda_runtime.h>
#include <curand_kernel.h>

namespace Gem::Hap
{
    static constexpr int THREADS_PER_BLOCK = 256;

    // Kernel that initializes PRNG states and generates one random number per thread.
    // For a true Mersenne Twister, curandStateMtgp32 would be used here.
    __global__ void generateRandomKernel(curandState* states, std::uint32_t* out, int n)
    {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < n)
        {
            // Initialize PRNG state (simplified)
            curand_init(/*seed=*/1234ULL, /*sequence=*/idx, /*offset=*/0, &states[idx]);

            // Generate a uint32_t
            out[idx] = curand(&states[idx]);
        }
    }

    // Generates n random numbers on the GPU and copies them into hostBuffer
    void generateGpuRandomNumbers(std::size_t n, std::vector<std::uint32_t>& hostBuffer)
    {
        if (n == 0) return;

        curandState* d_states = nullptr;
        std::uint32_t* d_out = nullptr;
        cudaMalloc(&d_states, n * sizeof(curandState));
        cudaMalloc(&d_out, n * sizeof(std::uint32_t));

        int blocks = static_cast<int>((n + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK);
        generateRandomKernel<<<blocks, THREADS_PER_BLOCK>>>(d_states, d_out, static_cast<int>(n));

        cudaMemcpy(hostBuffer.data(), d_out, n * sizeof(std::uint32_t), cudaMemcpyDeviceToHost);

        cudaFree(d_out);
        cudaFree(d_states);
    }

    // ------------------------------
    // GCudaRNG Implementation
    // ------------------------------

    GCudaRNG::GCudaRNG(std::size_t poolCapacity, std::size_t initialBatchSize)
        : m_poolCapacity(poolCapacity)
          , m_batchSize(initialBatchSize)
          , m_stop(false)
    {
        // Start production thread
        m_productionThread = std::thread(&GCudaRNG::productionLoop, this);
    }

    GCudaRNG::~GCudaRNG()
    {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_stop = true;
            m_cv.notify_all();
        }
        if (m_productionThread.joinable())
        {
            m_productionThread.join();
        }
    }

    GCudaRNG::result_type GCudaRNG::operator()()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [this] { return !m_pool.empty() || m_stop; });

        if (m_stop && m_pool.empty())
        {
            // Return 0 or throw an exception if we're done
            return 0;
        }

        // Get one random number from the pool
        result_type val = m_pool.front();
        m_pool.pop_front();

        // Simple heuristic to increase batch size if we are low
        if (m_pool.size() < m_poolCapacity / 10)
        {
            m_batchSize.store(std::min<std::size_t>(m_batchSize.load() * 2, m_poolCapacity));
        }

        lk.unlock();
        m_cv.notify_all(); // notify producer if waiting
        return val;
    }

    void GCudaRNG::productionLoop()
    {
        std::vector<result_type> tmpBuffer;

        while (true)
        {
            // Check if we should stop
            {
                std::unique_lock<std::mutex> lk(m_mutex);
                if (m_stop)
                {
                    break;
                }
            }

            std::size_t batch = m_batchSize.load();

            // If pool is more than half full, reduce batch size
            {
                std::unique_lock<std::mutex> lk(m_mutex);
                if (m_pool.size() > m_poolCapacity / 2)
                {
                    batch = std::max<std::size_t>(batch / 2, 1);
                    m_batchSize.store(batch);
                }
            }

            // Generate 'batch' numbers on the GPU
            tmpBuffer.resize(batch);
            generateGpuRandomNumbers(batch, tmpBuffer);

            {
                // Wait if the pool is too close to capacity
                std::unique_lock<std::mutex> lk(m_mutex);
                m_cv.wait(lk, [this, batch]
                {
                    return (m_pool.size() + batch) <= m_poolCapacity || m_stop;
                });

                if (m_stop)
                {
                    break;
                }

                // Add new random values to the pool
                for (auto val : tmpBuffer)
                {
                    m_pool.push_back(val);
                }
            }

            // Notify any waiting consumers
            m_cv.notify_all();
        }
    }
} /* namespace Gem::Hap */
