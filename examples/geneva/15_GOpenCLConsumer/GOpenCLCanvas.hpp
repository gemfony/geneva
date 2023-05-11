/**
 * @file GOpenCLCanvas.hpp
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

// Standard header files go here
#include <tuple>
#include <functional>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#include "boost/cast.hpp"
#include "boost/shared_array.hpp"

// OpenCL includes
#define __CL_ENABLE_EXCEPTIONS // This will force OpenCL C++ classes to raise exceptions rather than to use an error code
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp" // Use the file in our local directory -- cl.hpp is not delivered by default on MacOS X
#else
#include <CL/opencl.hpp>
#endif

// Geneva header files go here
#include "common/GCanvas.hpp"

namespace Gem::Geneva {

typedef std::tuple<float,float,float> COLORTYPE;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A specialization of Gem::Common::GCanvas8 that allows to emit its data in a
 * form suitable for usage in an OpenCL context.
 */
class GOpenCLCanvas
   : public Gem::Common::GCanvas8
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Gem::Common::GCanvas8);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GOpenCLCanvas();
   /** @brief Initialization with dimensions and colors */
   [[maybe_unused]] explicit GOpenCLCanvas(
         const std::tuple<std::size_t, std::size_t>&
         , const std::tuple<float,float,float>& = COLORTYPE(0.f, 0.f, 0.f)
   );
   /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
   [[maybe_unused]] explicit GOpenCLCanvas(const std::string&);
   /** @brief Copy construction */
   GOpenCLCanvas(const GOpenCLCanvas&);
   /** @brief The destructor */
   ~GOpenCLCanvas() override;

   /** @brief Find out the deviation between this and another canvas */
   [[nodiscard]] virtual float diff(const GOpenCLCanvas&) const;

   /** @brief Emits the canvas data suitable for transfer to an OpenCL context (char-representation) */
   [[maybe_unused]] [[nodiscard]] std::tuple<std::shared_ptr<cl_uchar>, std::size_t> getOpenCLCanvasI() const;
   /** @brief Loads the canvas data from a cl_uchar array */
   [[maybe_unused]] void loadFromOpenCLArrayI(const std::tuple<std::shared_ptr<cl_uchar>, std::size_t>&);

   /** @brief Emits the canvas data suitable for transfer to an OpenCL context (cl_float-representation) */
   [[nodiscard]] std::tuple<std::shared_ptr<cl_float>, std::size_t> getOpenCLCanvasF() const;
   /** @brief Loads the canvas data from a cl_float array */
   [[maybe_unused]] void loadFromOpenCLArrayF(const std::tuple<std::shared_ptr<cl_float>, std::size_t>&);

   /** @brief Emits the canvas data suitable for transfer to an OpenCL context (cl_float4-representation) */
   [[maybe_unused]] [[nodiscard]] auto getOpenCLCanvasF4() const;
   /** @brief Loads the canvas data from a cl_float4 array */
   [[maybe_unused]] void loadFromOpenCLArrayF4(const std::tuple<std::shared_ptr<cl_float4>, std::size_t>&);
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
float operator-(const GOpenCLCanvas&, const GOpenCLCanvas&);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GOpenCLCanvas);

