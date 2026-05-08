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
        : poolCapacity_(poolCapacity)
        , batchSize_(initialBatchSize)
    {
        const int n = static_cast<int>(poolCapacity_);

        // Dedicated stream so RNG kernel launches never block fitness-evaluation kernels
        // on the default stream, and vice versa.
        cudaStreamCreate(reinterpret_cast<cudaStream_t *>(&stream_));

        // Allocate persistent GPU buffers sized for the maximum possible batch.
        cudaMalloc(&d_states_, static_cast<std::size_t>(n) * sizeof(curandState));
        cudaMalloc(&d_out_,    static_cast<std::size_t>(n) * sizeof(std::uint32_t));

        // Initialize all PRNG states once with a non-deterministic seed drawn from
        // GRandomFactory — the same source used by the CPU-based RNG path.
        const auto seed = static_cast<unsigned long long>(GRANDOMFACTORY->getSeed());
        const int blocks = (n + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        initStatesKernel<<<blocks, THREADS_PER_BLOCK, 0, reinterpret_cast<cudaStream_t>(stream_)>>>(
            static_cast<curandState *>(d_states_), seed, n);
        cudaStreamSynchronize(reinterpret_cast<cudaStream_t>(stream_));

        // Start production thread after GPU state is ready.
        productionThread_ = std::thread(&GCudaRNG::productionLoop, this);
    }

    GCudaRNG::~GCudaRNG()
    {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            stop_ = true;
            cv_.notify_all();
        }
        if (productionThread_.joinable())
        {
            productionThread_.join();
        }

        cudaStreamDestroy(reinterpret_cast<cudaStream_t>(stream_));
        stream_ = nullptr;
        cudaFree(d_states_);
        d_states_ = nullptr;
        cudaFree(d_out_);
        d_out_ = nullptr;
    }

    GCudaRNG::result_type GCudaRNG::operator()()
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this] { return !pool_.empty() || stop_; });

        if (stop_ && pool_.empty())
        {
            glogger << "In GCudaRNG::operator()(): Warning!" << std::endl
                    << "Generator is shutting down and pool is exhausted — returning 0." << std::endl
                    << GWARNING;
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GCudaRNG::operator()(): Error!" << std::endl
                << "Generator is shutting down and pool is exhausted." << std::endl
            );
        }

        // Get one random number from the pool
        result_type val = pool_.front();
        pool_.pop_front();

        // Simple heuristic to increase batch size if we are low
        if (pool_.size() < poolCapacity_ / 10)
        {
            batchSize_.store(std::min<std::size_t>(batchSize_.load() * 2, poolCapacity_));
        }

        lk.unlock();
        cv_.notify_all(); // notify producer if waiting
        return val;
    }

    // Runs generateKernel on the persistent d_states_ — no re-initialization.
    // n must be <= poolCapacity_ so the pre-allocated buffers are large enough.
    void GCudaRNG::fillBuffer(std::size_t n, std::vector<result_type> &buf)
    {
        if (n == 0) return;

        const int count  = static_cast<int>(n);
        const int blocks = (count + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

        generateKernel<<<blocks, THREADS_PER_BLOCK, 0, reinterpret_cast<cudaStream_t>(stream_)>>>(
            static_cast<curandState *>(d_states_),
            static_cast<std::uint32_t *>(d_out_),
            count);

        cudaMemcpyAsync(buf.data(), d_out_, n * sizeof(result_type),
                        cudaMemcpyDeviceToHost, reinterpret_cast<cudaStream_t>(stream_));
        cudaStreamSynchronize(reinterpret_cast<cudaStream_t>(stream_));
    }

    void GCudaRNG::productionLoop()
    {
        std::vector<result_type> tmpBuffer;

        while (true)
        {
            // Check if we should stop
            {
                std::unique_lock<std::mutex> lk(mutex_);
                if (stop_)
                {
                    break;
                }
            }

            std::size_t batch = batchSize_.load();

            // If pool is more than half full, reduce batch size
            {
                std::unique_lock<std::mutex> lk(mutex_);
                if (pool_.size() > poolCapacity_ / 2)
                {
                    batch = std::max<std::size_t>(batch / 2, 1);
                    batchSize_.store(batch);
                }
            }

            // Generate 'batch' numbers on the GPU using the persistent PRNG states
            tmpBuffer.resize(batch);
            fillBuffer(batch, tmpBuffer);

            {
                // Wait if the pool is too close to capacity
                std::unique_lock<std::mutex> lk(mutex_);
                cv_.wait(lk, [this, batch]
                {
                    return (pool_.size() + batch) <= poolCapacity_ || stop_;
                });

                if (stop_)
                {
                    break;
                }

                // Add new random values to the pool
                for (auto val : tmpBuffer)
                {
                    pool_.push_back(val);
                }
            }

            // Notify any waiting consumers
            cv_.notify_all();
        }
    }
} /* namespace Gem::Hap */
