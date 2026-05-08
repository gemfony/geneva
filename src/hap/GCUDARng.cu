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
#include "hap/GRandomFactory.hpp"
#include "common/GLogger.hpp"

#include <cuda_runtime.h>
#include <curand_kernel.h>

namespace Gem::Hap
{
    static constexpr int THREADS_PER_BLOCK = 256;

    // One-time initialization: sets up independent PRNG streams per thread using the
    // supplied seed. Each thread receives a distinct sequence number so that all
    // streams are statistically independent.
    __global__ void initStatesKernel(curandState *states, unsigned long long seed, int n)
    {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < n)
        {
            curand_init(seed, static_cast<unsigned long long>(idx), /*offset=*/0, &states[idx]);
        }
    }

    // Per-batch generation: advances the existing PRNG states without re-initialization,
    // producing one independent random uint32 per thread.
    __global__ void generateKernel(curandState *states, std::uint32_t *out, int n)
    {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < n)
        {
            out[idx] = curand(&states[idx]);
        }
    }

    // ------------------------------
    // GCudaRNG Implementation
    // ------------------------------

    GCudaRNG::GCudaRNG(std::size_t poolCapacity, std::size_t initialBatchSize)
        : m_poolCapacity(poolCapacity)
        , m_batchSize(initialBatchSize)
    {
        const int n = static_cast<int>(m_poolCapacity);

        // Dedicated stream so RNG kernel launches never block fitness-evaluation kernels
        // on the default stream, and vice versa.
        cudaStreamCreate(reinterpret_cast<cudaStream_t *>(&m_stream));

        // Allocate persistent GPU buffers sized for the maximum possible batch.
        cudaMalloc(&m_d_states, static_cast<std::size_t>(n) * sizeof(curandState));
        cudaMalloc(&m_d_out,    static_cast<std::size_t>(n) * sizeof(std::uint32_t));

        // Initialize all PRNG states once with a non-deterministic seed drawn from
        // GRandomFactory — the same source used by the CPU-based RNG path.
        const auto seed = static_cast<unsigned long long>(GRANDOMFACTORY->getSeed());
        const int blocks = (n + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        initStatesKernel<<<blocks, THREADS_PER_BLOCK, 0, reinterpret_cast<cudaStream_t>(m_stream)>>>(
            static_cast<curandState *>(m_d_states), seed, n);
        cudaStreamSynchronize(reinterpret_cast<cudaStream_t>(m_stream));

        // Start production thread after GPU state is ready.
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

        cudaStreamDestroy(reinterpret_cast<cudaStream_t>(m_stream));
        m_stream = nullptr;
        cudaFree(m_d_states);
        m_d_states = nullptr;
        cudaFree(m_d_out);
        m_d_out = nullptr;
    }

    GCudaRNG::result_type GCudaRNG::operator()()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [this] { return !m_pool.empty() || m_stop; });

        if (m_stop && m_pool.empty())
        {
            glogger << "In GCudaRNG::operator()(): Warning!" << std::endl
                    << "Generator is shutting down and pool is exhausted — returning 0." << std::endl
                    << GWARNING;
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

    // Runs generateKernel on the persistent m_d_states — no re-initialization.
    // n must be <= m_poolCapacity so the pre-allocated buffers are large enough.
    void GCudaRNG::fillBuffer(std::size_t n, std::vector<result_type> &buf)
    {
        if (n == 0) return;

        const int count  = static_cast<int>(n);
        const int blocks = (count + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

        generateKernel<<<blocks, THREADS_PER_BLOCK, 0, reinterpret_cast<cudaStream_t>(m_stream)>>>(
            static_cast<curandState *>(m_d_states),
            static_cast<std::uint32_t *>(m_d_out),
            count);

        cudaMemcpyAsync(buf.data(), m_d_out, n * sizeof(result_type),
                        cudaMemcpyDeviceToHost, reinterpret_cast<cudaStream_t>(m_stream));
        cudaStreamSynchronize(reinterpret_cast<cudaStream_t>(m_stream));
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

            // Generate 'batch' numbers on the GPU using the persistent PRNG states
            tmpBuffer.resize(batch);
            fillBuffer(batch, tmpBuffer);

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
