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

# The unified, device-model-agnostic GPU consumer add-on to courtier. It builds a small static
# library (gemfony-courtier-gpu) carrying the device-agnostic core + the always-available CPU
# backend, plus the CUDA and OpenCL backends when their toolkits are present. The CUDA/OpenCL
# dependencies stay confined to THIS library: the core courtier shared library never pulls them in,
# and only GPU-aware programs link gemfony-courtier-gpu.

SET ( COURTIER_GPU_SRCS
	GGPUBackendFactory.cpp
)

# --- optional CUDA backend (NVRTC + CUDA driver API; no nvcc compilation here) ----------------
# CUDA is enabled globally by the top-level CMakeLists when a toolkit is found (CMAKE_CUDA_COMPILER).
SET ( COURTIER_GPU_HAVE_CUDA FALSE )
IF ( CMAKE_CUDA_COMPILER )
	SET ( COURTIER_GPU_HAVE_CUDA TRUE )
	SET ( COURTIER_GPU_SRCS ${COURTIER_GPU_SRCS} GCUDABackend.cpp )
ENDIF ()

# --- optional OpenCL backend ------------------------------------------------------------------
FIND_PACKAGE ( OpenCL QUIET )
SET ( COURTIER_GPU_HAVE_OPENCL FALSE )
IF ( OpenCL_FOUND )
	SET ( COURTIER_GPU_HAVE_OPENCL TRUE )
	SET ( COURTIER_GPU_SRCS ${COURTIER_GPU_SRCS} GOpenCLBackend.cpp )
ENDIF ()

set_source_files_properties (
	${COURTIER_GPU_SRCS}
	PROPERTIES
	LANGUAGE CXX
	CXX_STANDARD 20
)

ADD_LIBRARY ( ${COURTIER_GPU_LIBNAME} STATIC ${COURTIER_GPU_SRCS} )

SET_TARGET_PROPERTIES ( ${COURTIER_GPU_LIBNAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE ON
	EXPORT_NAME courtier-gpu
)

# The framework headers live with the courtier headers, and it builds on courtier (and transitively
# hap + common). No dependency on the geneva layer: the marshaller interfaces are processable-generic.
TARGET_LINK_LIBRARIES ( ${COURTIER_GPU_LIBNAME} PUBLIC ${COURTIER_LIBNAME} )
TARGET_COMPILE_FEATURES ( ${COURTIER_GPU_LIBNAME} PUBLIC cxx_std_20 )

IF ( COURTIER_GPU_HAVE_CUDA )
	TARGET_COMPILE_DEFINITIONS ( ${COURTIER_GPU_LIBNAME} PUBLIC GPUGEN_HAVE_CUDA )
	# NVRTC for runtime kernel compilation; the CUDA driver API (cuda) for module load + launch.
	TARGET_LINK_LIBRARIES ( ${COURTIER_GPU_LIBNAME} PUBLIC CUDA::nvrtc CUDA::cuda_driver )
	MESSAGE ( STATUS "${COURTIER_GPU_LIBNAME}: CUDA backend enabled (NVRTC + CUDA driver API)" )
ELSE ()
	MESSAGE ( STATUS "${COURTIER_GPU_LIBNAME}: CUDA backend DISABLED (no CUDA toolkit)" )
ENDIF ()

IF ( COURTIER_GPU_HAVE_OPENCL )
	TARGET_COMPILE_DEFINITIONS ( ${COURTIER_GPU_LIBNAME} PUBLIC GPUGEN_HAVE_OPENCL )
	TARGET_LINK_LIBRARIES ( ${COURTIER_GPU_LIBNAME} PUBLIC OpenCL::OpenCL )
	MESSAGE ( STATUS "${COURTIER_GPU_LIBNAME}: OpenCL backend enabled" )
ELSE ()
	MESSAGE ( STATUS "${COURTIER_GPU_LIBNAME}: OpenCL backend DISABLED (no OpenCL SDK)" )
ENDIF ()

# The CUDA toolkit's bundled host compiler can inject an older libstdc++ into the link step; link the
# CXX compiler's own libstdc++.so by full path to satisfy the required CXXABI (see top-level notes).
IF ( COURTIER_GPU_HAVE_CUDA AND DEFINED GENEVA_CXX_RUNTIME_LIB_DIR
     AND EXISTS "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so" )
	TARGET_LINK_LIBRARIES ( ${COURTIER_GPU_LIBNAME} PUBLIC "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so" )
ENDIF ()

INSTALL ( TARGETS ${COURTIER_GPU_LIBNAME} EXPORT GenevaTargets DESTINATION ${INSTALL_PREFIX_LIBS} )
