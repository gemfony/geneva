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


################################################################################
#
# This file is part of the Geneva library collection. Apache License 2.0.
# See the NOTICE file in the top-level directory for contributors/copyright.
#
################################################################################
#
# Hap2 implementation + the gemfony-hap2 target. Built in parallel to Hap so
# both libraries coexist (libgemfony-hap.so vs libgemfony-hap2.so). Not exported
# into the installed GenevaTargets set; Hap2 is consumed in-tree (e.g. by the
# GHapVsHap2 benchmark).
#
################################################################################

SET ( HAP2SOURCES
	GHapEnums.cpp
	GRandomBase.cpp
	GRandomFactory.cpp
	GRandomT.cpp
)

SET_SOURCE_FILES_PROPERTIES(
		${HAP2SOURCES}
		PROPERTIES
		LANGUAGE CXX
		CXX_STANDARD 20
)

# If we compile with CUDA support, add the appropriate file
IF(GENEVA_USE_CUDA_RNG)
	SET(HAP2SOURCES
		${HAP2SOURCES}
		GCUDARng.cu
	)

	SET_SOURCE_FILES_PROPERTIES(
		GCUDARng.cu
		PROPERTIES
		LANGUAGE CUDA
		CMAKE_CUDA_STANDARD 20
		CMAKE_CUDA_STANDARD_REQUIRED ON
	)
ENDIF ()

ADD_LIBRARY ( ${HAP2_LIBNAME} SHARED ${HAP2SOURCES} )

SET_TARGET_PROPERTIES(
    ${HAP2_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	CUDA_ARCHITECTURES 80 86 89
	EXPORT_NAME hap2
)

target_compile_options(${HAP2_LIBNAME} PRIVATE
		$<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>
)

# Export the include directory and require C++20 transitively.
TARGET_INCLUDE_DIRECTORIES(
    ${HAP2_LIBNAME}
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/hap2/include>
    $<INSTALL_INTERFACE:include>
)
TARGET_COMPILE_FEATURES(${HAP2_LIBNAME} PUBLIC cxx_std_20)

# Inter-library dependency: hap2 builds on common (exactly like hap).
TARGET_LINK_LIBRARIES(${HAP2_LIBNAME} PUBLIC ${COMMON_LIBNAME})

# When CUDA RNG is enabled at build time, the producer can refill containers on
# the GPU via the cuRAND host API at run time (only when a device is actually
# present; otherwise it falls back to the CPU engine). This needs the
# HAP2_USE_CUDA compile definition and links the cuRAND + CUDA-runtime host
# libraries. cuRAND is part of the (optional) NVIDIA CUDA Toolkit the user
# provides; it is never vendored into the source tree.
IF(GENEVA_USE_CUDA_RNG)
	FIND_PACKAGE(CUDAToolkit REQUIRED)
	TARGET_COMPILE_DEFINITIONS(${HAP2_LIBNAME} PRIVATE HAP2_USE_CUDA=1)
	TARGET_LINK_LIBRARIES(${HAP2_LIBNAME} PRIVATE CUDA::curand CUDA::cudart)
ENDIF()

################################################################################
# SIMD bulk-refill backend selection. Compile-time only (no runtime
# dispatch). The probes decide which vectorised xoshiro256++ variant (if any)
# the producer uses to refill containers. PRIVATE: the flag/definition apply
# only to the gemfony-hap2 TUs, never to consumers (the public header's engine
# type stays scalar). On unsupported ISAs only the scalar path is built --
# this is by design, not an error.

INCLUDE ( CheckCXXSourceCompiles )

SET ( CMAKE_REQUIRED_FLAGS "-mavx2" )
CHECK_CXX_SOURCE_COMPILES (
	"#include <immintrin.h>
	 int main() { __m256i x = _mm256_set1_epi64x(1); return (int)_mm256_extract_epi64(x, 0); }"
	HAP2_HAS_AVX2 )
UNSET ( CMAKE_REQUIRED_FLAGS )

CHECK_CXX_SOURCE_COMPILES (
	"#include <arm_neon.h>
	 int main() { uint64x2_t x = vdupq_n_u64(1); return (int)vgetq_lane_u64(x, 0); }"
	HAP2_HAS_NEON )

IF ( HAP2_HAS_AVX2 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64" )
	TARGET_COMPILE_OPTIONS     ( ${HAP2_LIBNAME} PRIVATE -mavx2 )
	TARGET_COMPILE_DEFINITIONS ( ${HAP2_LIBNAME} PRIVATE HAP2_AVX2_BACKEND=1 )
	MESSAGE ( STATUS "Hap2: AVX2 SIMD bulk-refill backend enabled (x86_64)" )
ELSEIF ( HAP2_HAS_NEON AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64" )
	TARGET_COMPILE_DEFINITIONS ( ${HAP2_LIBNAME} PRIVATE HAP2_NEON_BACKEND=1 )
	MESSAGE ( STATUS "Hap2: NEON SIMD bulk-refill backend enabled (AArch64)" )
ELSE ()
	MESSAGE ( STATUS "Hap2: scalar xoshiro256++ refill (no SIMD backend for this ISA)" )
ENDIF ()
