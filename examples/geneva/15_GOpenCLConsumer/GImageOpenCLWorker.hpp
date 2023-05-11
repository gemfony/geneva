/**
 * @file GImageOpenCLWorker.hpp
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

// Standard headers go here
#include <memory>
#include <functional>

// OpenCL includes
#define __CL_ENABLE_EXCEPTIONS // This will force OpenCL C++ classes to raise exceptions rather than to use an error code
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp" // Use the file in our local directory -- cl.hpp is not delivered by default on MacOS X
#else
#include <CL/cl2.hpp>
#endif

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "geneva/GParameterSet.hpp"
#include "courtier/GStdThreadConsumerT.hpp"

// Local headers for the image individual and canvas
#include "GOpenCLWorkerT.hpp"
#include "GImageIndividual.hpp"
#include "GOpenCLCanvas.hpp"

namespace Gem::Geneva {

/******************************************************************************/
// Some default settings
const std::string GII_DEF_IMAGEFILE = "./pictures/ml.ppm";
const std::string GII_DEF_CODEFILE  = "./code/monalisa.cl";
const int GII_DEF_WGS               = 192;
const bool GII_DEF_USEGPU           = true;

/******************************************************************************/
// A struct holding condensed triangle specifications
typedef struct t_ocl_spec_c {
  cl_float2 tr_one;
  cl_float2 tr_two;
  cl_float2 tr_three;
  cl_float4 rgba_f;
  cl_float4 dummy1; // padding
  cl_float2 dummy2; // padding
} __attribute__ ((aligned(64))) t_ocl_cart;

/** @brief A convenience-function allowing easier access to the content of this struct */
std::ostream& operator<<(std::ostream&, const t_ocl_cart&);

/******************************************************************************/
/**
 * A struct holding the coordinates, colors and opacity of a single triangle, which is
 * defined via a surrounding circle
 */
typedef struct triangle_ocl_circle_struct {
  /** @brief Assignment of a Gem::Common::triangle_circle_struct struct */
  void operator=(const Gem::Common::triangle_circle_struct&);
  /** @brief Assignment of another t_ocl_circle */
  const triangle_ocl_circle_struct& operator=(const triangle_ocl_circle_struct&);

  cl_float middleX;
  cl_float middleY;
  cl_float radius;
  cl_float angle1;
  cl_float angle2;
  cl_float angle3;
  cl_float4 rgba_f;
  cl_float4 dummy1; // padding
  cl_float2 dummy2; // padding
} __attribute__ ((aligned(64))) t_ocl_circle;

/** @brief Comparison with self */
bool operator==(
      const triangle_ocl_circle_struct&
      , triangle_ocl_circle_struct&
);

/** @brief Comparison with self */
bool operator!=(
      const triangle_ocl_circle_struct&
      , triangle_ocl_circle_struct&
);

/** @brief Output operator for easier access */
std::ostream& operator<<(std::ostream&, const t_ocl_circle&);

/******************************************************************************/
/**
 * A worker class that assembles images from semi-transparent triangles.
 */
class GImageOpenCLWorker
      :public Gem::Courtier::GOpenCLWorkerT<Gem::Geneva::GParameterSet>
{
public:
   /** @brief Initialization with an external OpenCL device and the name of a configuration file. */
   GImageOpenCLWorker(
      const cl::Device&
      , const std::string&
   );

   /** @brief Initialization with the data needed for an optimization run */
   GImageOpenCLWorker(const GImageOpenCLWorker&);

   /** @brief The destructor */
   ~GImageOpenCLWorker() override;

   /** @brief Retrieve the image dimensions */
   [[nodiscard]] std::tuple<std::size_t,std::size_t> getImageDimensions() const;
   /** @brief Sets the amount of triangles constituting each image */
   [[maybe_unused]] void setNTriangles(const std::size_t&);

   /** @brief External evaluation using OpenCL and available devices */
   std::vector<double> openCLCalc(std::shared_ptr<GImageIndividual>);
   /** @brief External evaluation using the CPU alone */
   std::vector<double> cpuCalc(std::shared_ptr<GImageIndividual>);

protected:
   /** @brief Actual per-item work is done here */
   void process_(std::shared_ptr<GParameterSet>) override;

   /** @brief Initialization of everything related to OpenCL */
   void initOpenCL(std::shared_ptr<GParameterSet>) override;
   /** @brief Initialization of kernel objects */
   void initKernels(std::shared_ptr<GParameterSet>) override;

   /** @brief Emits compiler options for OpenCL */
   [[nodiscard]] std::string getCompilerOptions() const override;

   /** @brief Adds local configuration options to a GParserBuilder object */
   void addConfigurationOptions_(Gem::Common::GParserBuilder&) override;

private:
   /** @brief Creates a deep clone of this object, camouflaged as a GWorker */
   std::shared_ptr<Gem::Courtier::GWorkerT<GParameterSet>> clone_() const override;

   /** @brief Loads the target image from file into a local canvas */
   void loadTargetFromFile();

   cl_float *global_results_;
   t_ocl_circle *circle_triangles_;

   std::string imageFile_; ///< The name of the file holding the image data
   GOpenCLCanvas targetCanvas_; ///< Holds the target image

   cl::Image2D target_image_buffer_; // Remains unchanged during the execution
   cl::Image2D candidate_image_buffer_;
   cl::Buffer  circ_triangle_buffer_;
   cl::Buffer  cart_triangle_buffer_;
   cl::Buffer  global_results_buffer_; ///< Will hold results calculated for each candidate image

   cl::Kernel tr_transcode_kernel_;
   cl::Kernel candidate_creator_kernel_;
   cl::Kernel candidate_deviation_kernel_;

   int dimX_, dimY_; ///< The image dimensions (derived from the image file loaded from disk)
   int targetSize_; ///< The number of pixels in the target
   int nWorkGroups_; ///< The number of work groups (derived from the image dimensions and the work group size

   bool useGPU_; ///< Determines whether the GPU should be used for the evaluation (instead of the CPU)
   std::size_t nTriangles_; ///< The amount of triangles constituting each image
};

/******************************************************************************/

} /* namespace Gem::Geneva */

