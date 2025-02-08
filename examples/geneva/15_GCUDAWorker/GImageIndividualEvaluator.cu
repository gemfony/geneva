/**
* @file GImageIndividualEvaluator.cu
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

#include <cub/cub.cuh>
#include "GImageIndividualEvaluator.hpp"

namespace Gem::Geneva
{
    /**
     * Initialization with the constant data of the evaluation process
     *
     * @param targetImageFileName The name of the image to which similarity should be created
     * @param useGPU Whether evaluation shall use the GPU
     * @param getGPUCandidateImage Allows to specify whether candidate images shall be returned from the GPU
     * @param blockSize_x The CUDA block-size in x-direction
     * @param blockSize_y The CUDA block-size in y-direction
     * @param gridSize_x The CUDA grid-size in x-direction
     * @param gridSize_y The CUDA grid-size in y-direction
     */
    GImageIndividualEvaluator::GImageIndividualEvaluator(const std::string& targetImageFileName,
                                                         bool useGPU,
                                                         bool getGPUCandidateImage,
                                                         const int blockSize_x, const int blockSize_y,
                                                         const int gridSize_x,  const int gridSize_y)
        : targetImageFileName_(targetImageFileName),
          useGPU_(useGPU),
          getGPUCandidateImage_(getGPUCandidateImage),
          blockSize_x_(blockSize_x), blockSize_y_(blockSize_y),
          gridSize_x_(gridSize_x), gridSize_y_(gridSize_y)
    {
        /* nothing */
    }

    /**
     * Initialization either for CUDA or for local evaluation
     */
    void GImageIndividualEvaluator::init(const std::shared_ptr<GImageIndividual>& individual_ptr)
    {
        // First load the target image into our local data structure,
        // identifying the image dimensions along the way
        if (not Common::loadImageToFloat(targetImageFileName_, targetImageData_vec_, width_, height_))
        {
            throw gemfony_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GImageIndividualEvaluator::init(): Error!" << std::endl
                << "Target image " << targetImageFileName_ << " could not be loaded!" << std::endl
            );
        }

        // Retrieve some further information from the GImageIndividual
        nTriangles_ = individual_ptr->getNTriangles();
        bgColor_ = individual_ptr->getBackGroundColor();

        std::vector<float> bgColor_vec;
        bgColor_vec.push_back(std::get<0>(bgColor_));
        bgColor_vec.push_back(std::get<1>(bgColor_));
        bgColor_vec.push_back(std::get<2>(bgColor_));

        // Fill the candidate image vector with our background color and resize
        clearCandidateDataToBG();

        // Allocate memory and transfer data to the GPU
        if (useGPU_)
        {
            //----------------------------------------------------------------------------------
            // Check that block size is not 0
            if (blockSize_x_ == 0 || blockSize_y_ == 0)
            {
                throw gemfony_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GImageCUDAWorker::init(): Error!" << std::endl
                    << "Invalid block dimensions read: " << blockSize_x_ << " / " << blockSize_y_ << std::endl
                );
            }

            // Calculate grid size automatically if grid is set to 0
            if (gridSize_x_ == 0 && gridSize_y_ == 0)
            {
                gridSize_x_ = (width_ + blockSize_x_ - 1)  / blockSize_x_;
                gridSize_y_ = (height_ + blockSize_y_ - 1) / blockSize_y_;
            }

            //----------------------------------------------------------------------------------
            // Target image
            // Calculate the number of bytes to be allocated on the GPU
            const auto imageSizeBytes = static_cast<int>(width_ * height_ * 3 * sizeof(float));

            // Some error checking
#ifdef DEBUG
            if (imageSizeBytes != targetImageData_vec_.size() * sizeof(float))
            {
                throw gemfony_exception(
                                g_error_streamer(DO_LOG, time_and_place)
                                << "In GImageCUDAWorker::init(): Error!" << std::endl
                                << "Invalid image sizes: " << width_ << " / " << height_ << " / " << (targetImageData_vec_.size() * sizeof(float)) << std::endl
                            );
                );
            }
#endif

            //----------------------------------------------------------------------------------
            // Initialize a CUDA stream
            checkCuda(cudaStreamCreate(&cuda_stream_), "cudaStreamCreate");

            //----------------------------------------------------------------------------------
            // Necessary memory allocations

            // Allocate the appropriate amount of space on the GPU
            checkCuda(cudaMalloc(&d_target_, imageSizeBytes), "cudaMalloc d_target");
            // Allocate the space for the candidate image
            checkCuda(cudaMalloc(&d_candidate_, imageSizeBytes), "cudaMalloc d_candidate_");
            // Allocate space for the evaluations
            checkCuda(cudaMalloc(&d_perPixel_evaluation_, imageSizeBytes), "cudaMalloc d_perpixel_evaluation_");
            // Allocate the space for the background color
            checkCuda(cudaMalloc(&d_bgcolor_, 3 * sizeof(float)), "cudaMalloc d_bgcolor_");
            // Allocate the space for the triangle data
            checkCuda(cudaMalloc(&d_triangles_, static_cast<int>(nTriangles_ * sizeof(Geneva::CircleTriangle))),
                      "cudaMalloc d_triangles_");
            // Storage for transformed triangle data on the device
            checkCuda(cudaMalloc(&d_transformed_triangle_data_, 10 * nTriangles_ * sizeof(float)), "cudaMalloc d_triangle_data");
            // Result data
            checkCuda(cudaMalloc(&d_result_, sizeof(float)), "cudaMalloc d_results");

            // Allocate CUB space for summing up the results
            checkCuda(cub::DeviceReduce::Sum(d_temp_storage_, temp_storage_bytes_, d_perPixel_evaluation_, d_result_,
                                             width_ * height_ * 3), "cub::DeviceReduce::Sum initialization");
            checkCuda(cudaMalloc(&d_temp_storage_, temp_storage_bytes_), "d_temp_storage_");

            //----------------------------------------------------------------------------------
            // Copy and set data

            // Copy the target image data over
            checkCuda(cudaMemcpyAsync(d_target_,
                                      targetImageData_vec_.data(),
                                      imageSizeBytes,
                                      cudaMemcpyHostToDevice, cuda_stream_), "Memcpy target");
            // Copy the current background color over
            checkCuda(cudaMemcpyAsync(d_bgcolor_,
                                      bgColor_vec.data(),
                                      3 * sizeof(unsigned char),
                                      cudaMemcpyHostToDevice, cuda_stream_), "Memcpy bgcolor");

            //----------------------------------------------------------------------------------
            // Wait for operations to finish
            checkCuda(cudaStreamSynchronize(cuda_stream_), "cudaStreamSynchronize");

            //----------------------------------------------------------------------------------
        }
    }

    /**
     * Converts an individual to an RGB format by creating an array with the background color, then
     * looping over all triangles and, where necessary, modifying the associated pixels. The result
     * is stored in the candidateImageData_vec_ vector
     *
     * TODO: Does this use the individual's bg color?
     *
     * @param individual_ptr The GImageIndividual to be evaluated
     */
    void GImageIndividualEvaluator::transformToRGB(const std::shared_ptr<GImageIndividual>& individual_ptr)
    {
        // Clear the candidate image to the background color
        clearCandidateDataToBG();

        // Retrieve our triangles
        const auto triangleData = individual_ptr->getTriangleData();

        // Loop over all triangles, then check which pixels are contained in them.
        // If contained, blend the current color with the pixel-color.
        for (const auto& t : triangleData)
        {
            // Retrieve the triangle corners
            auto [x1,y1,x2,y2,x3,y3] = cpu_getCorners(t, width_, height_);

            // Loop over all pixels
            for (int x = 0; x < width_; x++)
            {
                // columns
                for (int y = 0; y < height_; y++)
                {
                    // rows --> y
                    // Check if the current pixel is contained in the triangle
                    if (cpu_pointInTriangle(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, x1, y1, x2, y2,
                                            x3,
                                            y3))
                    {
                        auto base_index = static_cast<std::size_t>(3 * (y * width_ + x));

                        // Perform the actual alpha-blending
                        cpu_alphaBlend(
#ifdef DEBUG
                            candidateImageData_vec_.at(static_cast<std::size_t>(base_index + 0)),
                            candidateImageData_vec_.at(static_cast<std::size_t>(base_index + 1)),
                            candidateImageData_vec_.at(static_cast<std::size_t>(base_index + 2)),
#else
                            candidateImageData_vec_[static_cast<std::size_t>(base_index + 0)],
                            candidateImageData_vec_[static_cast<std::size_t>(base_index + 1)],
                            candidateImageData_vec_[static_cast<std::size_t>(base_index + 2)],
#endif
                            t.r, t.g, t.b, t.a
                        );
                    }
                }
            }
        }
    }

    /**
     * Calculates the deviation of an individual from the target image
     */
    double GImageIndividualEvaluator::cpu_deviation(const std::shared_ptr<GImageIndividual>& individual_ptr)
    {
        // Extract our own image data. The result will
        // be in our local candidate image vector
        transformToRGB(individual_ptr);

        // Check that the target-image has the right dimension
        if (targetImageData_vec_.size() != candidateImageData_vec_.size())
        {
            std::cout
                << "In GImageIndividual::deviation(): Error! Invalid image dimensions"
                << width_ << " " << height_ << " " << targetImageData_vec_.size()
                << " " << candidateImageData_vec_.size() << std::endl;
            exit(1);
        }

        // Loop over both images and calculate the deviation
        double dev = 0.0;
        for (std::size_t t = 0; t < nTriangles_; t++)
        {
            dev += sqrt(
                pow(static_cast<double>(targetImageData_vec_[t + 0]) - static_cast<double>(candidateImageData_vec_[t +
                        0]), 2.) +
                pow(static_cast<double>(targetImageData_vec_[t + 1]) - static_cast<double>(candidateImageData_vec_[t +
                        1]), 2.) +
                pow(static_cast<double>(targetImageData_vec_[t + 2]) - static_cast<double>(candidateImageData_vec_[t +
                        2]), 2.)
            );
        }

        return dev;
    }

    /**
     * Retrieval of all corner coordinates of a triangle
     */
    __device__ void
    cuda_calculateCorners(const CircleTriangle& tri,
                    const int& width,
                    const int& height,
                    float& outX1,
                    float& outY1,
                    float& outX2,
                    float& outY2,
                    float& outX3,
                    float& outY3)
    {
        // The center in x/y is relative to width/height
        const auto center_x = tri.cx * static_cast<float>(width);
        const auto center_y = tri.cy * static_cast<float>(height);

        // The triangle scale is measured in fractions of the _smaller_ value of width and height
        const auto scale = static_cast<float>(width<height?width:height);

        outX1 = center_x + tri.radius * cosf(tri.angle1 * 2.f * static_cast<float>(M_PI)) * scale;
        outY1 = center_y + tri.radius * sinf(tri.angle1 * 2.f * static_cast<float>(M_PI)) * scale;
        outX2 = center_x + tri.radius * cosf(tri.angle2 * 2.f * static_cast<float>(M_PI)) * scale;
        outY2 = center_y + tri.radius * sinf(tri.angle2 * 2.f * static_cast<float>(M_PI)) * scale;
        outX3 = center_x + tri.radius * cosf(tri.angle3 * 2.f * static_cast<float>(M_PI)) * scale;
        outY3 = center_y + tri.radius * sinf(tri.angle3 * 2.f * static_cast<float>(M_PI)) * scale;
    }

        /**
     * Determines the coordinates of the triangle and fills background-color
     * data and alpha channel into a new array, so that it may be later used without
     * modification for each color channel. Calculation is meant to be done in
     * parallel for each triangle.
     *
     * @param d_triangles The raw triangle data
     * @param d_transformed_triangle_data The transformed values (taking into account width, height, ...)
     * @param width The width of the target image
     * @param height The height of the target image
     * @param nTriangles The number of triangles in the data set
     */
    __global__ void
    cuda_transformTriangleData(const CircleTriangle* d_triangles,
                               float* d_transformed_triangle_data,
                               const int width, const int height,
                               const int nTriangles)
    {
        // Get the id of the triangle we are working on
        const int idx = blockIdx.x * blockDim.x + threadIdx.x;

        // Leave if we are beyond the array boundaries
        if (idx >= nTriangles) return;

        // Calculate the triangle corners and store them in the target array
        const auto baseIndex = 10 * idx;
        cuda_calculateCorners(d_triangles[idx],
                        width, height,
                        d_transformed_triangle_data[baseIndex + 0],
                        d_transformed_triangle_data[baseIndex + 1],
                        d_transformed_triangle_data[baseIndex + 2],
                        d_transformed_triangle_data[baseIndex + 3],
                        d_transformed_triangle_data[baseIndex + 4],
                        d_transformed_triangle_data[baseIndex + 5]);

        // Add background-color and alpha channel
        d_transformed_triangle_data[baseIndex + 6] = d_triangles[idx].r;
        d_transformed_triangle_data[baseIndex + 7] = d_triangles[idx].g;
        d_transformed_triangle_data[baseIndex + 8] = d_triangles[idx].b;
        d_transformed_triangle_data[baseIndex + 9] = d_triangles[idx].a;
    }

    /**
     * Check whether a given point is contained in a triangle. This function
     * acts relative to the image dimensions, i.e. with transformed triangle data
     *
     * @param px The x-coordinate of the point to be checked for its location
     * @param py The y-coordinate of the point to be checked for its location
     * @param x1 The x-coordinate of the first corner of the triangle
     * @param y1 The y-coordinate of the first corner of the triangle * @param x1 The x-coordinate of the first corner of the triangle
     * @param x2 The x-coordinate of the second corner of the triangle
     * @param x2 The y-coordinate of the second corner of the triangle
     * @param x3 The x-coordinate of the third corner of the triangle
     * @param y3 The y-coordinate of the third corner of the triangle
     * @return bool A boolean indicating whether the point is contained in the triangle or not
     */
    __device__ bool
    cuda_pointInTriangle(const float px, const float py,
                         const float x1, const float y1,
                         const float x2, const float y2,
                         const float x3, const float y3)
    {
        // Cross product-Signum
        auto sign = [] __device__ (const float xA, const float yA, const float xB,
                                   const float yB, const float xC, const float yC) -> float
        {
            return (xA - xC) * (yB - yC) - (yA - yC) * (xB - xC);
        };

        const float d1 = sign(px, py, x1, y1, x2, y2);
        const float d2 = sign(px, py, x2, y2, x3, y3);
        const float d3 = sign(px, py, x3, y3, x1, y1);

        const bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        const bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        // Point is contained in triangle if both booleans are the same
        return !(hasNeg && hasPos);
    }

    /**
     * Simple alpha blending
     *
     * @param bgR The red channel of the background color
     * @param bgG The green channel of the background color
     * @param bgB The blue channel of the background color
     * @param fgR The red channel of the resulting foreground color
     * @param fgG The green channel of the resulting foreground color
     * @param fgB The blue channel of the resulting foreground color
     * @param alpha The transparency of the passed color
     */
    __device__ void
    cuda_alphaBlend(float& bgR, float& bgG, float& bgB,
                    const float fgR, const float fgG, const float fgB,
                    const float alpha)
    {
        auto blendChannel = [] __device__ (const float bg,
                                           const float fg,
                                           const float a)
        {
            // Casting needed to avoid overflows
            return static_cast<float>((1. - a) * bg + a * fg);
        };

        bgR = blendChannel(bgR, fgR, alpha);
        bgG = blendChannel(bgG, fgG, alpha);
        bgB = blendChannel(bgB, fgB, alpha);
    }

    /**
     * The actual rendering and evaluation kernel
     *
     * @param d_target The target image, for which each RGB channel is encoded as a 0..1 float
     * @param d_candidate A memory area on the GPU to which a candidate image may be written
     * @param d_perPixel_error The error per pixel, as calculated by this function
     * @param d_bgcolors The background colors of the candidate image
     * @param d_result A memory area to which the deviation of candidate and target image pixel may be added
     * @param d_transformed_triangle_data The data of the nTriangles triangles to be assembled to the candidate image
     * @param width The width of the target (and candidate) image
     * @param height The height of the target (and candidate) image
     * @param nTriangles The number of triangles forming the candidate image
     */
    __global__ void
    cuda_renderAndCompareKernel(const float* __restrict__ d_target,
                                float* __restrict__ d_candidate,
                                float* __restrict__ d_perPixel_error,
                                const float* __restrict__ d_bgcolors,
                                const float* __restrict__ d_transformed_triangle_data,
                                const int width, const int height,
                                const int nTriangles)
    {
        const int x = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
        const int y = static_cast<int>(blockIdx.y * blockDim.y + threadIdx.y);

        const auto x_f = static_cast<float>(x) + 0.5f;
        const auto y_f = static_cast<float>(y) + 0.5f;

        if (x >= width || y >= height) return;

        // Transfer the background color
        float rOut = d_bgcolors[0], gOut = d_bgcolors[1], bOut = d_bgcolors[2];

        // Loop over all triangles
        for (int idx = 0; idx < nTriangles; idx++)
        {
            const auto offset = idx*10;

            // Extract the data of the current triangle
            const auto x1 = d_transformed_triangle_data[offset + 0];
            const auto y1 = d_transformed_triangle_data[offset + 1];
            const auto x2 = d_transformed_triangle_data[offset + 2];
            const auto y2 = d_transformed_triangle_data[offset + 3];
            const auto x3 = d_transformed_triangle_data[offset + 4];
            const auto y3 = d_transformed_triangle_data[offset + 5];
            const auto  r = d_transformed_triangle_data[offset + 6];
            const auto  g = d_transformed_triangle_data[offset + 7];
            const auto  b = d_transformed_triangle_data[offset + 8];
            const auto  a = d_transformed_triangle_data[offset + 9];

            // Check whether the current pixel is located inside
            if (cuda_pointInTriangle(x_f, y_f, x1, y1, x2, y2, x3, y3))
            {
                // Alpha-Blending
                cuda_alphaBlend(rOut, gOut, bOut, r, g, b, a);
            }
        }

        // Write the blended color into the pixel of the candidate image
        const auto pixelIndex = y * width + x;
        const size_t outPos = pixelIndex * 3;

        d_candidate[outPos + 0] = rOut;
        d_candidate[outPos + 1] = gOut;
        d_candidate[outPos + 2] = bOut;

        // Calculate the distance to the target image
        const float rT = d_target[outPos + 0];
        const float gT = d_target[outPos + 1];
        const float bT = d_target[outPos + 2];

        const float dr = rT - rOut;
        const float dg = gT - gOut;
        const float db = bT - bOut;

        const float dr_squared = dr * dr;
        const float dg_squared = dg * dg;
        const float db_squared = db * db;

        constexpr float factor = 0.04;
        const float rsf_dr = dr_squared / (dr_squared + factor); // rsf == rational saturation function
        const float rsf_dg = dg_squared / (dg_squared + factor);
        const float rsf_db = db_squared / (db_squared + factor);

        // Store the data in the result array
        d_perPixel_error[outPos + 0] = rsf_dr;
        d_perPixel_error[outPos + 1] = rsf_dg;
        d_perPixel_error[outPos + 2] = rsf_db;
    }

    /**
     * The actual rendering and evaluation kernel -- ChatGPT o3-mini-high-style
     *
     * @param d_target The target image, for which each RGB channel is encoded as a 0..1 float
     * @param d_candidate A memory area on the GPU to which a candidate image may be written
     * @param d_bgcolors The background colors of the candidate image
     * @param d_result A memory area to which the deviation of candidate and target image pixel may be added
     * @param d_transformed_triangle_data The data of the nTriangles triangles to be assembled to the candidate image
     * @param width The width of the target (and candidate) image
     * @param height The height of the target (and candidate) image
     * @param nTriangles The number of triangles forming the candidate image
     */
    __global__ void
    cuda_renderAndCompareKernelO3MiniHigh(const float* __restrict__ d_target,
                                          float* __restrict__ d_candidate,
                                          const float* __restrict__ d_bgcolors,
                                          float* d_result,
                                          const float* __restrict__ d_transformed_triangle_data,
                                          const int width, const int height,
                                          const int nTriangles)
    {
        // Compute pixel coordinates.
        int x = blockIdx.x * blockDim.x + threadIdx.x;
        int y = blockIdx.y * blockDim.y + threadIdx.y;
        // The color channel for this thread (0: red, 1: green, 2: blue)
        int channel = threadIdx.z;

        if (x >= width || y >= height) return;

        // Each thread starts with the background color for its channel.
        float colorVal = d_bgcolors[channel];

        // Compute the pixel center coordinates.
        float px = static_cast<float>(x) + 0.5f;
        float py = static_cast<float>(y) + 0.5f;

        // Loop over all triangles.
        for (int idx = 0; idx < nTriangles; idx++)
        {
            // Each triangle is stored as 10 floats:
            // [x1, y1, x2, y2, x3, y3, r, g, b, a]
            const int base = idx * 10;
            const float x1 = d_transformed_triangle_data[base + 0];
            const float y1 = d_transformed_triangle_data[base + 1];
            const float x2 = d_transformed_triangle_data[base + 2];
            const float y2 = d_transformed_triangle_data[base + 3];
            const float x3 = d_transformed_triangle_data[base + 4];
            const float y3 = d_transformed_triangle_data[base + 5];
            const float r = d_transformed_triangle_data[base + 6];
            const float g = d_transformed_triangle_data[base + 7];
            const float b = d_transformed_triangle_data[base + 8];
            const float a = d_transformed_triangle_data[base + 9];

            // Check if the pixel lies inside this triangle.
            if (cuda_pointInTriangle(px, py, x1, y1, x2, y2, x3, y3))
            {
                // Select the triangle’s color for the current channel.
                float fgColor = (channel == 0) ? r : (channel == 1) ? g : b;
                // Simple alpha blend for this channel.
                colorVal = (1.0f - a) * colorVal + a * fgColor;
            }
        }

        // Compute the linear index for the pixel.
        const int pixelIndex = y * width + x;
        // Each pixel has three channels stored consecutively.
        const int outPos = pixelIndex * 3 + channel;

        // Write the computed candidate color for this channel.
        d_candidate[outPos] = colorVal;

        // Compare with the target image.
        float targetVal = d_target[outPos];
        float diff = targetVal - colorVal;
        float diff_sq = diff * diff;
        const float factor = 0.04f;
        // Apply the rational saturation function (rsf).
        float rsf = diff_sq / (diff_sq + factor);

        // Atomically add this channel’s error contribution.
        atomicAdd(d_result, rsf);
    }

    // Helper function for single-channel alpha blending
    __device__ float
    cuda_alphaBlendChannel(const float bg, const float fg, const float alpha) {
        return (1.0f - alpha) * bg + alpha * fg;
    }

    /**
     * The actual rendering and evaluation kernel -- DeepThink R1-style
     *
     * @param d_target The target image, for which each RGB channel is encoded as a 0..1 float
     * @param d_candidate A memory area on the GPU to which a candidate image may be written
     * @param d_bgcolors The background colors of the candidate image
     * @param d_result A memory area to which the deviation of candidate and target image pixel may be added
     * @param d_transformed_triangle_data The data of the nTriangles triangles to be assembled to the candidate image
     * @param width The width of the target (and candidate) image
     * @param height The height of the target (and candidate) image
     * @param nTriangles The number of triangles forming the candidate image
     */
    __global__ void
    cuda_renderAndCompareKernelR1(const float* d_target,
                                  float* d_candidate,
                                  const float* d_bgcolors,
                                  float* d_result,
                                  float* d_transformed_triangle_data, // RB: this could be constant
                                  const int width, const int height,
                                  const int nTriangles)
    {
        // 3D thread indexing: x, y, color (0=R, 1=G, 2=B)
        int x = blockIdx.x * blockDim.x + threadIdx.x;
        int y = blockIdx.y * blockDim.y + threadIdx.y;
        int colorIdx = threadIdx.z; // Color channel (0, 1, 2)

        if (x >= width || y >= height) return;

        // Initialize background color for this channel
        float colorOut = d_bgcolors[colorIdx];

        // Loop over all triangles
        for (int idx = 0; idx < nTriangles; idx++)
        {
            // Extract triangle data (shared across all threads)
            const auto x1 = d_transformed_triangle_data[idx * 10 + 0]; // RB: Repeated calculation
            const auto y1 = d_transformed_triangle_data[idx * 10 + 1];
            const auto x2 = d_transformed_triangle_data[idx * 10 + 2];
            const auto y2 = d_transformed_triangle_data[idx * 10 + 3];
            const auto x3 = d_transformed_triangle_data[idx * 10 + 4];
            const auto y3 = d_transformed_triangle_data[idx * 10 + 5];
            const auto r = d_transformed_triangle_data[idx * 10 + 6];
            const auto g = d_transformed_triangle_data[idx * 10 + 7];
            const auto b = d_transformed_triangle_data[idx * 10 + 8];
            const auto a = d_transformed_triangle_data[idx * 10 + 9];

            // Check if pixel is inside the triangle (redundant but necessary)
            if (cuda_pointInTriangle(static_cast<float>(x) + 0.5f, // RB: This calculation could be in front
                                     static_cast<float>(y) + 0.5f,
                                     x1, y1, x2, y2, x3, y3))
            {
                // Get the triangle's color for this channel
                float fgColor;
                switch (colorIdx)
                {
                case 0: fgColor = r;
                    break;
                case 1: fgColor = g;
                    break;
                case 2: fgColor = b;
                    break;
                }
                colorOut = cuda_alphaBlendChannel(colorOut, fgColor, a); // RB: Could be inline
            }
        }

        // Write to candidate image
        const size_t pixelIndex = static_cast<size_t>(y * width + x);
        d_candidate[pixelIndex * 3 + colorIdx] = colorOut;

        // Calculate deviation for this channel
        const float targetVal = d_target[pixelIndex * 3 + colorIdx]; // RB: Repeated index calculation
        const float diff = targetVal - colorOut;
        const float diffSq = diff * diff;
        const float rsf = diffSq / (diffSq + 0.04f);

        // Atomic add to result
        atomicAdd(d_result, rsf);
    }

    /**
     * Evaluation of individuals
     */
    double GImageIndividualEvaluator::evaluate(const std::shared_ptr<GImageIndividual>& individual_ptr)
    {
        float fitness{0.};

        // Perform the evaluation on the GPU
        if (useGPU_)
        {
            const auto imageSizeBytes = static_cast<std::size_t>(width_ * height_ * 3 * sizeof(float));

            // Reset the fitness-counter
            checkCuda(cudaMemsetAsync(d_result_, 0, sizeof(float), cuda_stream_), "Memset fitness");

            // Retrieve and transfer the current background color
            bgColor_ = individual_ptr->getBackGroundColor();
            std::vector<float> bgColor_vec;
            bgColor_vec.push_back(std::get<0>(bgColor_));
            bgColor_vec.push_back(std::get<1>(bgColor_));
            bgColor_vec.push_back(std::get<2>(bgColor_));

            checkCuda(cudaMemcpyAsync(d_bgcolor_, bgColor_vec.data(), 3 * sizeof(float), cudaMemcpyHostToDevice,
                                      cuda_stream_), "Memcpy bgcolor");

            // Retrieve and transfer the current triangle set
            const auto CircleTriangleVec = individual_ptr->getTriangleData();
            checkCuda(cudaMemcpyAsync(d_triangles_, CircleTriangleVec.data(), nTriangles_ * sizeof(CircleTriangle),
                                      cudaMemcpyHostToDevice, cuda_stream_), "Memcpy triangles");

            // Set up the block- and grid-sizes for the corner calculation
            dim3 dimTriangleBlock(256);
            dim3 dimTriangleGrid((nTriangles_ + dimTriangleBlock.x - 1) / dimTriangleBlock.x);

            // Calculate corner coordinates
            cuda_transformTriangleData<<<dimTriangleGrid, dimTriangleBlock, 0, cuda_stream_>>>(
                d_triangles_,
                d_transformed_triangle_data_,
                width_, height_,
                nTriangles_
            );

            // Set up the block- and grid-sizes for the rendering
            dim3 dimRenderBlock(blockSize_x_, blockSize_y_);
            dim3 dimRenderGrid(gridSize_x_, gridSize_y_);

            // Start the actual kernel
            cuda_renderAndCompareKernel<<<dimRenderGrid, dimRenderBlock, 0, cuda_stream_>>>(
                d_target_,
                d_candidate_,
                d_perPixel_evaluation_,
                d_bgcolor_,
                d_transformed_triangle_data_,
                width_, height_,
                static_cast<int>(nTriangles_)
            );

            /*
            // Set up the block- and grid-sizes for the rendering
            dim3 dimRenderBlock(16, 16, 3);
            dim3 dimRenderGrid((width_ + dimRenderBlock.x - 1) / dimRenderBlock.x,
                               (height_ + dimRenderBlock.y - 1) / dimRenderBlock.y);

            // Start the actual kernel
            // cuda_renderAndCompareKernelO3MiniHigh<<<dimRenderGrid, dimRenderBlock, 0, cuda_stream_>>>(
            cuda_renderAndCompareKernelR1<<<dimRenderGrid, dimRenderBlock, 0, cuda_stream_>>>(
                d_target_,
                d_candidate_,
                d_bgcolor_,
                d_result_,
                d_transformed_triangle_data_,
                width_, height_,
                static_cast<int>(nTriangles_)
            );
            */

            // Sum up the result
            checkCuda(cub::DeviceReduce::Sum(d_temp_storage_, temp_storage_bytes_, d_perPixel_evaluation_, d_result_,
                                             width_ * height_ * 3), "cub::DeviceReduce::Sum");

            // Retrieve the fitness of the individual
            checkCuda(cudaMemcpyAsync(&fitness, d_result_, sizeof(float), cudaMemcpyDeviceToHost, cuda_stream_),
                      "Mmcpy result");

            // Store the result in the individual
            std::vector<double> result_vec;
            result_vec.push_back(static_cast<double>(fitness));

            // Copying the images back is an expensive operation.
            // We only want to perform this in selected cases, e.g.
            // for the pluggable optimization monitor.
            if (getGPUCandidateImage_)
            {
                checkCuda(cudaMemcpyAsync(candidateImageData_vec_.data(),
                                          d_candidate_,
                                          imageSizeBytes,
                                          cudaMemcpyDeviceToHost,
                                          cuda_stream_), "Mmcpy candidate image");
            }

            // Wait for all work to finish in this stream
            checkCuda(cudaStreamSynchronize(cuda_stream_), "cudaStreamSynchronize");
        }
        // Run solely on the CPU
        else
        {
            // This is not a multi-criterion optimization
            fitness = cpu_deviation(individual_ptr);
        }

        // individual_ptr->setFitness(std::vector<double>(1, static_cast<double>(fitness)));
        individual_ptr->process(
            std::vector<parameterset_processing_result>(1, parameterset_processing_result(static_cast<double>(fitness)))
        );

        // Let the audience know
        return fitness;
    }

    /**
     * Finalization code
     */
    void GImageIndividualEvaluator::finalize() const
    {
        // Get rid of memory allocated for the target image on the GPU, as well as streams
        if (useGPU_)
        {
            cudaFree(d_target_);
            cudaFree(d_triangles_);
            cudaFree(d_candidate_);
            cudaFree(d_perPixel_evaluation_);
            cudaFree(d_bgcolor_);
            cudaFree(d_result_);
            cudaFree(d_transformed_triangle_data_);
            cudaFree(d_temp_storage_);

            checkCuda(cudaStreamSynchronize(cuda_stream_), "cudaStreamSynchronize");
            checkCuda(cudaStreamDestroy(cuda_stream_), "cudaStreamDestroy");
        }
    }

    /**
     * Retrieval of the candidate image
     *
     * @param width The width of the target image
     * @param height The height of the target image
     * @return The candidate image as float values
     */
    std::vector<float>
    GImageIndividualEvaluator::getCandidateImage(int& width, int& height) const
    {
        // Complain if no valid candidate image exists
        if (useGPU_ and not getGPUCandidateImage_)
        {
            throw gemfony_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GImageIndividualEvaluator::getCandidateImage(): Error!" << std::endl
                << "Asked for candidate image even though image was not meant to " << std::endl
                << "be transferred from the device back to the host" << std::endl
            );
        }

        width = width_;
        height = height_;

        return candidateImageData_vec_;
    }

    /**
     * Saves current the current candidate image to disc
     *
     * @param candidateFile The path and name of the file to which the candidate image shall be saved
     */
    void
    GImageIndividualEvaluator::saveCandidateImageToDisc(const std::string& candidateFile) const
    {
        Common::saveFloatImageToFile(candidateFile, candidateImageData_vec_, width_, height_);
    }

    /**
     * Calculate the cartesian coordinates of the image from its circle definition
     *
     * @param tri The circle definition of the triangle
     * @param width The image width
     * @param height The image height
     * @return A tuple holding the cartesian coordinates
     */
    std::tuple<float, float, float, float, float, float>
    GImageIndividualEvaluator::cpu_getCorners(const CircleTriangle& tri, int width, int height)
    {
        const auto scale = width<=height?width:height;
        return {
            tri.cx * static_cast<float>(width)  + tri.radius * cosf(tri.angle1 * 2 * M_PI) * static_cast<float>(scale), // x1
            tri.cy * static_cast<float>(height) + tri.radius * sinf(tri.angle1 * 2 * M_PI) * static_cast<float>(scale), // y1
            tri.cx * static_cast<float>(width)  + tri.radius * cosf(tri.angle2 * 2 * M_PI) * static_cast<float>(scale), // x2
            tri.cy * static_cast<float>(height) + tri.radius * sinf(tri.angle2 * 2 * M_PI) * static_cast<float>(scale), // y2
            tri.cx * static_cast<float>(width)  + tri.radius * cosf(tri.angle3 * 2 * M_PI) * static_cast<float>(scale), // x3
            tri.cy * static_cast<float>(height) + tri.radius * sinf(tri.angle3 * 2 * M_PI) * static_cast<float>(scale)  // y3
        };
    }

    /**
     * Checks with a cross product whether a given point is contained in
     * a triangle defined by its corner coordinates
     */
    bool
    GImageIndividualEvaluator::cpu_pointInTriangle(float px, float py,
                                                   float x1, float y1,
                                                   float x2, float y2,
                                                   float x3, float y3)
    {
        auto sign = [](float xA, float yA, float xB, float yB, float xC, float yC)
        {
            return (xA - xC) * (yB - yC) - (yA - yC) * (xB - xC);
        };

        const float d1 = sign(px, py, x1, y1, x2, y2);
        const float d2 = sign(px, py, x2, y2, x3, y3);
        const float d3 = sign(px, py, x3, y3, x1, y1);

        const bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        const bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        // The point is only inside of the triangle if the signs are the same
        return !(hasNeg && hasPos);
    }

    /**
     * Simple alpha blending in 8 bits
     *
     * @param bgR The red channel of the background color
     * @param bgG The green channel of the background color
     * @param bgB The blue channel of the background color
     * @param fgR The red channel of the foreground color to be created
     * @param fgG The green channel of the foreground color to be created
     * @param fgB The blue channel of the foreground color to be created
     * @param alpha The transparency level of the triangle
     */
    void
    GImageIndividualEvaluator::cpu_alphaBlend(float& bgR, float& bgG, float& bgB,
                                              const float fgR, const float fgG, const float fgB,
                                              const float alpha)
    {
        auto blendChannel = [](const float bg,
                               const float fg,
                               const float a)
        {
            // Casting needed to avoid overflows
            return static_cast<float>((1. - a) * bg + a * fg);
        };

        bgR = blendChannel(bgR, fgR, alpha);
        bgG = blendChannel(bgG, fgG, alpha);
        bgB = blendChannel(bgB, fgB, alpha);
    }


    /**
     * This function resets the candidateImageData_vec_ to the stored background colors and resize if necessary
     */
    void GImageIndividualEvaluator::clearCandidateDataToBG()
    {
        const std::size_t pixel_size = width_ * height_;
        const std::size_t candidateImageData_vec_size = 3 * pixel_size;

        // Resize candidateImageData_vec_ tp the target size if required
        if (candidateImageData_vec_.size() != (candidateImageData_vec_size))
        {
            candidateImageData_vec_.resize(candidateImageData_vec_size);
        }

        // Consecutively copy bgColor values into the candidateImageData_vec_ vector
        for (std::size_t p = 0; p < pixel_size; p++)
        {
            candidateImageData_vec_.at(3*p + 0) = std::get<0>(bgColor_);
            candidateImageData_vec_.at(3*p + 1) = std::get<1>(bgColor_);
            candidateImageData_vec_.at(3*p + 2) = std::get<2>(bgColor_);
        }
    }
} /* namespace Gem::Geneva */
