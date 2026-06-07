################################################################################
#
# This file is part of the Geneva library collection. The following license
# applies to this file:
#
# ------------------------------------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
#
# Note that other files in the Geneva library collection may use a different
# license. Please see the licensing information in each file.
#
################################################################################
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################

# EXPERIMENTAL: a unified, device-model-agnostic GPU consumer framework. Builds a small static
# library (gpugen) carrying the device-agnostic core + the always-available CPU backend, plus the
# CUDA and OpenCL backends when their toolkits are present, and then the demo programs.

SET ( GPUGEN_SRCS
	src/GGPUBackendFactory.cpp
)

# --- optional CUDA backend (NVRTC + CUDA driver API; no nvcc compilation here) ----------------
# CUDA is enabled globally by the top-level CMakeLists when a toolkit is found (CMAKE_CUDA_COMPILER).
SET ( GPUGEN_HAVE_CUDA FALSE )
IF ( CMAKE_CUDA_COMPILER )
	SET ( GPUGEN_HAVE_CUDA TRUE )
	SET ( GPUGEN_SRCS ${GPUGEN_SRCS} src/GCudaBackend.cpp )
ENDIF ()

# --- optional OpenCL backend ------------------------------------------------------------------
FIND_PACKAGE ( OpenCL QUIET )
SET ( GPUGEN_HAVE_OPENCL FALSE )
IF ( OpenCL_FOUND )
	SET ( GPUGEN_HAVE_OPENCL TRUE )
	SET ( GPUGEN_SRCS ${GPUGEN_SRCS} src/GOpenCLBackend.cpp )
ENDIF ()

ADD_LIBRARY ( gpugen STATIC ${GPUGEN_SRCS} )

TARGET_INCLUDE_DIRECTORIES ( gpugen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )

SET_TARGET_PROPERTIES ( gpugen PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE ON
)

TARGET_LINK_LIBRARIES ( gpugen PUBLIC
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
)

IF ( GPUGEN_HAVE_CUDA )
	TARGET_COMPILE_DEFINITIONS ( gpugen PUBLIC GPUGEN_HAVE_CUDA )
	# NVRTC for runtime kernel compilation; the CUDA driver API (cuda) for module load + launch.
	TARGET_LINK_LIBRARIES ( gpugen PUBLIC CUDA::nvrtc CUDA::cuda_driver )
	MESSAGE ( STATUS "gpugen: CUDA backend enabled (NVRTC + CUDA driver API)" )
ELSE ()
	MESSAGE ( STATUS "gpugen: CUDA backend DISABLED (no CUDA toolkit)" )
ENDIF ()

IF ( GPUGEN_HAVE_OPENCL )
	TARGET_COMPILE_DEFINITIONS ( gpugen PUBLIC GPUGEN_HAVE_OPENCL )
	TARGET_LINK_LIBRARIES ( gpugen PUBLIC OpenCL::OpenCL )
	MESSAGE ( STATUS "gpugen: OpenCL backend enabled" )
ELSE ()
	MESSAGE ( STATUS "gpugen: OpenCL backend DISABLED (no OpenCL SDK)" )
ENDIF ()

# The CUDA toolkit's bundled host compiler can inject an older libstdc++ into the link step; link the
# CXX compiler's own libstdc++.so by full path to satisfy the required CXXABI (see top-level notes).
IF ( GPUGEN_HAVE_CUDA AND DEFINED GENEVA_CXX_RUNTIME_LIB_DIR
     AND EXISTS "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so" )
	TARGET_LINK_LIBRARIES ( gpugen PUBLIC "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so" )
ENDIF ()

# Demo programs that drive the framework.
ADD_SUBDIRECTORY ( examples/function_demo )
ADD_SUBDIRECTORY ( examples/image_demo )
ADD_SUBDIRECTORY ( examples/monalisa_demo )
