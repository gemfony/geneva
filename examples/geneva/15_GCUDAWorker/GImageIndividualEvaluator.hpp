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

    /** @brief Calculation of a single triangle corner in cartesian coordinates */
    __device__ void
    gpu_getCorner(const CircleTriangle& tri,
                  float angle,
                  float& outX,
                  float& outY,
                  int width,
                  int height);

    /** @brief Check whether a given point is contained in a triangle */
    __device__ bool
    gpu_pointInTriangle(float px, float py,
                        float x1, float y1,
                        float x2, float y2,
                        float x3, float y3);

    /** @brief Simple alpha blending */
    __device__ void
    gpu_alphaBlend(unsigned char& bgR, unsigned char& bgG, unsigned char& bgB,
                   unsigned char fgR, unsigned char fgG, unsigned char fgB,
                   unsigned char alpha);

    __global__ void
    gpu_renderAndCompareKernel(const CircleTriangle*,
                               const unsigned char*,
                               unsigned char*,
                               unsigned char*,
                               float*,
                               int, int,
                               int);

    /******************************************************************************/
    /**
     * This helper-class evaluates GImageIndividual-objects either on the GPU or the CPU
     */
    class GImageIndividualEvaluator
    {
    public:
        GImageIndividualEvaluator(const std::string&,
                                  bool, bool,
                                  int, int,
                                  int, int);
        virtual ~GImageIndividualEvaluator() = default;

        //------------------------------------------------------------------
        // Render this class non-copyable

        /** @brief Disabled default constructor */
        GImageIndividualEvaluator() = delete;
        /** @brief Disabled copy constructor */
        GImageIndividualEvaluator(const GImageIndividualEvaluator&) = delete;
        /** @brief Disabled move constructor */
        GImageIndividualEvaluator(GImageIndividualEvaluator&&) = delete;
        /** @brief Disabled assignment operator */
        GImageIndividualEvaluator& operator=(const GImageIndividualEvaluator&) = delete;
        /** @brief Disabled move-assignment operator */
        GImageIndividualEvaluator& operator=(GImageIndividualEvaluator&&) = delete;

        //------------------------------------------------------------------

        void init(const std::shared_ptr<GImageIndividual>&);
        double evaluate(std::shared_ptr<GImageIndividual>&);
        void finalize();

        /** @brief Retrieval of the candidate image */
        std::vector<unsigned char> getCandidateImage(int&, int&) const;
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
        std::tuple<float, float, float, float, float, float>
        cpu_getCorners(const CircleTriangle&, int, int);

        /** @brief Checks with a cross product whether a given point is contained in a triangle */
        bool cpu_pointInTriangle(float px, float py,
                                 float x1, float y1,
                                 float x2, float y2,
                                 float x3, float y3);

        /** @brief Simple alpha blending in 8 bits */
        void
        cpu_alphaBlend(unsigned char&, unsigned char&, unsigned char&,
                       unsigned char, unsigned char, unsigned char,
                       unsigned char);

        //--------------------------------------------------------------------
        // Variables

        const std::string targetImageFileName_; ///< The name of the target image
        int width_{0}; ///< The width of the target image
        int height_{0}; ///< The height of the target image
        std::size_t nTriangles_{0}; ///< The number of triangles constituting an image
        std::tuple<unsigned char, unsigned char, unsigned char> bgColor_{255, 255, 255};
        ///< The canvas background color

        // For CUDA evaluation
        const bool useGPU_; ///< Whether the GPU shall be used for the evaluation of the individuals
        const bool getGPUCandidateImage_; ///< Whether to retrieve candidate images back from the GPU

        unsigned int blockSize_x_{0}, blockSize_y_{0}; ///< CUDA block-sizes
        unsigned int gridSize_x_{0}, gridSize_y_{0}; /// CUDA grid-sizes

        unsigned char* d_target_{nullptr}; ///< Holds a copy of the target image
        Geneva::CircleTriangle* d_triangles_{nullptr}; ///< Holds the triangles described by the individual
        unsigned char* d_candidate_{nullptr}; ///< Holds the candidate image assembled from the triangles
        unsigned char* d_bgcolor_{nullptr}; ///< Holds the current background color, to be transferred to the device
        double* d_result_{nullptr}; ///< Holds the result of the current evaluation

        cudaStream_t cuda_stream_{};

        // Image data structures
        std::vector<unsigned char> targetImageData_vec_{}; ///< Holds the target image data
        std::vector<unsigned char> candidateImageData_vec_{}; ///< Holds temporary image data

        // For testing purposes
        std::mutex testMutex_;
    };
} /* namespace Gem::Geneva */
