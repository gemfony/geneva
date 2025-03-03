/**
 * @file GImageIndividualEvaluator.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <memory>
#include <vector>
#include <tuple>

// Third party header files go here

// Third party header files go here
#include <png.h>

// CUDA-Headers go here
#include <cuda_runtime.h>

// Boost headers go here

// Geneva headers go here
#include "GImageIndividual.hpp"
#include "GImageHelperFunctions.hpp"

namespace Gem::Geneva
{
    //--------------------------------------------------------------------
    // For GPU-based calculation

    /**
     * Calculates the minimum of three values
     */
    __device__ inline float
    min3(float a, float b, float c) {
        return fminf(a, fminf(b, c));
    }

    /**
     * Calculates the maximum of three values
     */
    __device__ inline float
    max3(float a, float b, float c) {
        return fmaxf(a, fmaxf(b, c));
    }

    /** @brief Retrieval of all corner coordinates of a triangle */
    __device__ inline void
    cuda_calculateCorners(const CircleTriangle& tri,
                          const int& width,
                          const int& height,
                          float& outX1,
                          float& outY1,
                          float& outX2,
                          float& outY2,
                          float& outX3,
                          float& outY3);

    /** @brief Check whether a given point is contained in a triangle */
    __device__ bool
    cuda_pointInTriangle(const float px, const float py,
                         const float x1, const float y1,
                         const float x2, const float y2,
                         const float x3, const float y3);

    /** @brief Simple alpha blending */
    __device__ inline void
    cuda_alphaBlend(float& bgR, float& bgG, float& bgB,
                    const float fgR, const float fgG, const float fgB,
                    const float alpha);

    __global__ void
    cuda_transformTriangleData(const CircleTriangle*,
                               float*,
                               const int, const int,
                               const int);

    template <int ExtractionMode>
    __global__ void
    cuda_renderAndCompareKernel(const float* __restrict__,
                                float* __restrict__,
                                float* __restrict__,
                                const float* __restrict__,
                                const float* __restrict__,
                                const int,
                                const int,
                                const int);

    /******************************************************************************/
    /**
     * This helper-class evaluates GImageIndividual-objects either on the GPU or the CPU
     */
    class GImageIndividualEvaluator
    {
    public:
        GImageIndividualEvaluator(const std::string&,
                                  bool, bool,
                                  const int, const int,
                                  const int, const int);
        virtual ~GImageIndividualEvaluator() = default;

        //------------------------------------------------------------------
        // Render this class non-copyable

        /** @brief Disabled default constructor */
        GImageIndividualEvaluator() = delete;
        /** @brief Disabled copy constructor */
        GImageIndividualEvaluator(const GImageIndividualEvaluator&) = delete;
        /** @brief Disabled move constructor getGPUCandidateImage*/
        GImageIndividualEvaluator(GImageIndividualEvaluator&&) = delete;
        /** @brief Disabled assignment operator */
        GImageIndividualEvaluator& operator=(const GImageIndividualEvaluator&) = delete;
        /** @brief Disabled move-assignment operator */
        GImageIndividualEvaluator& operator=(GImageIndividualEvaluator&&) = delete;

        //------------------------------------------------------------------

        void init(const std::shared_ptr<GImageIndividual>&);
        double evaluate(const std::shared_ptr<GImageIndividual>&);
        void finalize() const;

        /** @brief Retrieval of the candidate image */
        std::vector<float> getCandidateImage(int&, int&) const;
        /** @brief Saves the candidate image to disc */
        void saveCandidateImageToDisc(const std::string&) const;

    private:
        static void checkCuda(cudaError_t err, const char* msg)
        {
            if (err != cudaSuccess)
            {
                fprintf(stderr, "GImageIndividualEvaluator::checkCuda: CUDA Error! %s (%s)\n", msg,
                        cudaGetErrorString(err));
                exit(EXIT_FAILURE);
            }
        }

        /** @brief Converts an individual to an RGB format */
        void transformToRGB(const std::shared_ptr<GImageIndividual>&);

        /** @brief Calculates the deviation of this individual from the target image */
        double cpu_deviation(const std::shared_ptr<GImageIndividual>&);

        /** @brief Resets the candidate data structure to the stored background colors */
        void clearCandidateDataToBG();

        //--------------------------------------------------------------------
        // For CPU-based calculation

        /** @brief Calculate the cartesian coordinates of the image from its circle definition */
        static std::tuple<float, float, float, float, float, float>
        cpu_getCorners(const CircleTriangle&, int, int);

        /** @brief Checks with a cross product whether a given point is contained in a triangle */
        static bool
        cpu_pointInTriangle(float px, float py,
                            float x1, float y1,
                            float x2, float y2,
                            float x3, float y3);

        /** @brief Simple alpha blending in 8 bits */
        static void
        cpu_alphaBlend(float&, float&, float&,
                       const float, const float, const float,
                       const float);

        //--------------------------------------------------------------------
        // Variables

        const std::string targetImageFileName_; ///< The name of the target image
        int width_{0}; ///< The width of the target image
        int height_{0}; ///< The height of the target image
        std::size_t nTriangles_{0}; ///< The number of triangles constituting an image
        std::tuple<float, float, float> bgColor_{0.f, 0.f, 0.f};
        ///< The canvas background color

        // For CUDA evaluation
        const bool useGPU_; ///< Whether the GPU shall be used for the evaluation of the individuals
        const bool getGPUCandidateImage_; ///< Whether to retrieve candidate images back from the GPU

        unsigned int blockSize_x_, blockSize_y_; ///< CUDA block-sizes
        unsigned int gridSize_x_, gridSize_y_; /// CUDA grid-sizes

        float* d_target_{nullptr}; ///< Holds a copy of the target image
        float* d_candidate_{nullptr}; ///< Holds the candidate image assembled from the triangles
        float* d_perPixel_evaluation_{nullptr}; ///< Holds the evaluations for each color channel of each pixel
        float* d_bgcolor_{nullptr}; ///< Holds the current background color, to be transferred to the device
        Geneva::CircleTriangle* d_triangles_{nullptr}; ///< Holds the "raw" triangles described by the individual
        float* d_transformed_triangle_data_{nullptr};
        ///< Holds transformed triangle coordinates relative to the image dimensions
        float* d_result_{nullptr}; ///< Holds the result of the current evaluation
        void* d_temp_storage_{nullptr}; ///< Temporary storage for CUB.
        std::size_t temp_storage_bytes_{0}; ///< Needed for CUB

        cudaStream_t cuda_stream_{};

        // Image data structures
        std::vector<float> targetImageData_vec_{}; ///< Holds the target image data
        std::vector<float> candidateImageData_vec_{}; ///< Holds temporary image data

        // For testing purposes
        std::mutex testMutex_;
    };
} /* namespace Gem::Geneva */
