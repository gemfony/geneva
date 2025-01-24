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
                gridSize_x_ = (width_ + blockSize_x_ - 1) / blockSize_x_;
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
            // Allocate the space for the background color
            checkCuda(cudaMalloc(&d_bgcolor_, 3 * sizeof(float)), "cudaMalloc d_bgcolor_");
            // Allocate the space for the triangle data
            checkCuda(cudaMalloc(&d_triangles_, static_cast<int>(nTriangles_ * sizeof(Geneva::CircleTriangle))),
                      "cudaMalloc d_triangles_");
            // Storage for transformed triangle data on the device
            checkCuda(cudaMalloc(&d_transformed_triangle_data_, 10 * nTriangles_ * sizeof(float)), "cudaMalloc d_triangle_data");
            // Result data
            checkCuda(cudaMalloc(&d_result_, sizeof(float)), "cudaMalloc d_results");

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
     * Check whether a given point is contained in a triangle. This function
     * acts relative to the image dimensions, i.e. with transformed triangle data
     */
    __device__ bool
    cuda_pointInTriangle(const float px, const float py,
                         const float x1, const float y1,
                         const float x2, const float y2,
                         const float x3, const float y3)
    {
        // Cross product-Signum
        auto sign = [] __device__ (const float xA, const float yA, const float xB,
                                   const float yB, const float xC, const float yC)
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
     * The actual rendering and evaluation kernel
     */
    __global__ void
    cuda_renderAndCompareKernel(const float* d_target,
                                float* d_candidate,
                                const float* d_bgcolors,
                                float* d_result,
                                float* d_transformed_triangle_data,
                                const int width, const int height,
                                const int nTriangles)
    {
        int x = blockIdx.x * blockDim.x + threadIdx.x;
        int y = blockIdx.y * blockDim.y + threadIdx.y;

        if (x >= width || y >= height) return;

        // Transfer the background color
        float rOut = d_bgcolors[0], gOut = d_bgcolors[1], bOut = d_bgcolors[2];

        // Loop over all triangles
        for (int idx = 0; idx < nTriangles; idx++)
        {
            // Extract the data of the current triangle
            const auto x1 = d_transformed_triangle_data[idx * 10 + 0];
            const auto y1 = d_transformed_triangle_data[idx * 10 + 1];
            const auto x2 = d_transformed_triangle_data[idx * 10 + 2];
            const auto y2 = d_transformed_triangle_data[idx * 10 + 3];
            const auto x3 = d_transformed_triangle_data[idx * 10 + 4];
            const auto y3 = d_transformed_triangle_data[idx * 10 + 5];
            const auto  r = d_transformed_triangle_data[idx * 10 + 6];
            const auto  g = d_transformed_triangle_data[idx * 10 + 7];
            const auto  b = d_transformed_triangle_data[idx * 10 + 8];
            const auto  a = d_transformed_triangle_data[idx * 10 + 9];

            // Check whether the current pixel is located inside of a triangle
            if (cuda_pointInTriangle(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, x1, y1, x2, y2, x3, y3))
            {
                // Alpha-Blending
                cuda_alphaBlend(rOut, gOut, bOut, r, g, b, a);
            }
        }

        // Write the blended color into the candidate image
        const size_t pixelIndex = static_cast<size_t>(y * width + x);
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

        // Sum up the result
        atomicAdd(d_result, dr * dr + dg * dg + db * db);
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
                d_bgcolor_,
                d_result_,
                d_transformed_triangle_data_,
                width_, height_,
                static_cast<int>(nTriangles_)
            );

            // Retrieve the fitness of the individual
            checkCuda(cudaMemcpyAsync(&fitness, d_result_, sizeof(float), cudaMemcpyDeviceToHost, cuda_stream_), "Mmcpy result");

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
            cudaFree(d_bgcolor_);
            cudaFree(d_result_);
            cudaFree(d_transformed_triangle_data_);

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
