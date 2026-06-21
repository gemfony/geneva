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

# GPU-consumer backend sources + device-toolkit detection. This fragment is INCLUDEd from
# courtier/src/CMakeLists.txt when GENEVA_BUILD_WITH_GPU_CONSUMER is ON, so the device-model-agnostic
# GPU consumer ships INSIDE the one gemfony-courtier library (just like the MPI consumer), rather than
# in a separate library. It only sets variables; the caller folds the sources into the courtier target
# and applies the matching link libraries + compile definitions:
#   COURTIER_GPU_SRCS         -- backend .cpp to compile into courtier (paths relative to courtier/src)
#   COURTIER_GPU_HAVE_CUDA    -- TRUE when the CUDA (NVRTC + driver API) backend is included
#   COURTIER_GPU_HAVE_OPENCL  -- TRUE when the OpenCL backend is included
# The always-available CPU backend is header-only; the CUDA/OpenCL headers are confined to these .cpp,
# so folding them in does not leak device headers into courtier's public interface.

SET ( COURTIER_GPU_SRCS
	gpu/GGPUBackendFactory.cpp
)

# --- optional CUDA backend (NVRTC + CUDA driver API; no nvcc compilation here) ----------------
# CUDA is enabled globally by the top-level CMakeLists when a toolkit is found (CMAKE_CUDA_COMPILER)
# and GENEVA_SKIP_CUDA is OFF.
SET ( COURTIER_GPU_HAVE_CUDA FALSE )
IF ( CMAKE_CUDA_COMPILER )
	SET ( COURTIER_GPU_HAVE_CUDA TRUE )
	SET ( COURTIER_GPU_SRCS ${COURTIER_GPU_SRCS} gpu/GCUDABackend.cpp )
ENDIF ()

# --- optional OpenCL backend ------------------------------------------------------------------
# A CUDA toolkit ships its own libOpenCL.so in its library directory, and that directory is on the
# runtime search path (RPATH) because nvrtc and the CUDA driver library live there. If
# find_package(OpenCL) instead resolves to a *different* OpenCL ICD loader (e.g. the system ocl-icd
# loader, which lives in an implicit linker directory), the two same-named libraries collide: CMake
# cannot order them safely and emits "Cannot generate a safe runtime search path", and at run time the
# toolkit's copy shadows the linked one. Pin OpenCL to the *selected* toolkit's loader so link-time and
# run-time resolve to the same file. This is derived from the toolkit (CUDA_nvrtc_LIBRARY's directory),
# never a hardcoded version; it only affects machines that have BOTH a CUDA toolkit and a separate
# system OpenCL, and only when the user has not pinned OpenCL_LIBRARY themselves. Both NVIDIA's bundled
# loader and ocl-icd are ICD loaders that dispatch to the same vendor drivers, so behaviour is unchanged.
IF ( COURTIER_GPU_HAVE_CUDA AND NOT DEFINED OpenCL_LIBRARY AND CUDA_nvrtc_LIBRARY )
	GET_FILENAME_COMPONENT ( _gpu_cuda_libdir "${CUDA_nvrtc_LIBRARY}" DIRECTORY )
	IF ( EXISTS "${_gpu_cuda_libdir}/libOpenCL.so.1" )
		SET ( OpenCL_LIBRARY "${_gpu_cuda_libdir}/libOpenCL.so.1" CACHE FILEPATH
			"OpenCL ICD loader, pinned to the CUDA toolkit's copy so link-time and run-time agree (its dir is already on the RPATH via nvrtc)" )
	ENDIF ()
	UNSET ( _gpu_cuda_libdir )
ENDIF ()
FIND_PACKAGE ( OpenCL QUIET )
SET ( COURTIER_GPU_HAVE_OPENCL FALSE )
IF ( OpenCL_FOUND )
	SET ( COURTIER_GPU_HAVE_OPENCL TRUE )
	SET ( COURTIER_GPU_SRCS ${COURTIER_GPU_SRCS} gpu/GOpenCLBackend.cpp )
ENDIF ()
