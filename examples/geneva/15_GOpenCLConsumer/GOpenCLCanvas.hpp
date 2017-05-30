/**
 * @file GOpenCLCanvas.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva/P library collection. No part of
 * this code may be distributed without prior, written consent by
 * Gemfony scientific.
 */


// Standard header files go here
#include <tuple>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#include "boost/cast.hpp"
#include "boost/shared_array.hpp"

// This will force OpenCL C++ classes to raise exceptions
// rather than to use an error code
#define __CL_ENABLE_EXCEPTIONS

// OpenCL includes
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp" // Use the file in our local directory -- cl.hpp is not delivered by default on MacOS X
#else
#include <CL/cl.hpp>
#endif

#ifndef GOPENCLCANVAS_HPP_
#define GOPENCLCANVAS_HPP_

// Geneva header files go here
#include "common/GCanvas.hpp"

namespace Gem {
namespace Geneva {

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
   GOpenCLCanvas(
         const std::tuple<std::size_t, std::size_t>&
         , const std::tuple<float,float,float>& = COLORTYPE(0.f, 0.f, 0.f)
   );
   /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
   GOpenCLCanvas(const std::string&);
   /** @brief Copy construction */
   GOpenCLCanvas(const GOpenCLCanvas&);
   /** @brief The destructor */
   virtual ~GOpenCLCanvas();

   /** @brief The assignment operator */
   const GOpenCLCanvas& operator=(const GOpenCLCanvas&);

   /** @brief Find out the deviation between this and another canvas */
   virtual float diff(const GOpenCLCanvas&) const;

   /** @brief Emits the canvas data suitable for transfer to an OpenCL context (char-representation) */
   std::tuple<boost::shared_array<cl_uchar>, std::size_t> getOpenCLCanvasI() const;
   /** @brief Loads the canvas data from a cl_uchar array */
   void loadFromOpenCLArrayI(const std::tuple<boost::shared_array<cl_uchar>, std::size_t>&);

   /** @brief Emits the canvas data suitable for transfer to an OpenCL context (cl_float-representation) */
   std::tuple<boost::shared_array<cl_float>, std::size_t> getOpenCLCanvasF() const;
   /** @brief Loads the canvas data from a cl_float array */
   void loadFromOpenCLArrayF(const std::tuple<boost::shared_array<cl_float>, std::size_t>&);

   /** @brief Emits the canvas data suitable for transfer to an OpenCL context (cl_float4-representation) */
   std::tuple<boost::shared_array<cl_float4>, std::size_t> getOpenCLCanvasF4() const;
   /** @brief Loads the canvas data from a cl_float4 array */
   void loadFromOpenCLArrayF4(const std::tuple<boost::shared_array<cl_float4>, std::size_t>&);
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
float operator-(const GOpenCLCanvas&, const GOpenCLCanvas&);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GOpenCLCanvas);

#endif /* GOPENCLCANVAS_HPP_ */
