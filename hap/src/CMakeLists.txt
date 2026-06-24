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
# Hap implementation + the gemfony-hap target. Uses the xoshiro256++ engine
# (scalar + SIMD bulk-refill) with an optional cuRAND GPU backend, and is
# exported into the GenevaTargets set so courtier/geneva can consume it.
#
################################################################################

SET ( HAPSOURCES
	GHapEnums.cpp
	GRandomBase.cpp
	GRandomFactory.cpp
	GRandomT.cpp
	GStagedSource.cpp
)

SET_SOURCE_FILES_PROPERTIES(
		${HAPSOURCES}
		PROPERTIES
		LANGUAGE CXX
		CXX_STANDARD 20
)

# If we compile with CUDA support, add the appropriate file
IF(GENEVA_USE_CUDA_RNG_ENABLED)
	SET(HAPSOURCES
		${HAPSOURCES}
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

ADD_LIBRARY ( ${HAP_LIBNAME} SHARED ${HAPSOURCES} )

SET_TARGET_PROPERTIES(
    ${HAP_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	CUDA_ARCHITECTURES 80 86 89
	EXPORT_NAME hap
)

target_compile_options(${HAP_LIBNAME} PRIVATE
		$<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>
)

# Export the include directory and require C++20 transitively.
TARGET_INCLUDE_DIRECTORIES(
    ${HAP_LIBNAME}
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/hap/include>
    $<INSTALL_INTERFACE:include>
)
TARGET_COMPILE_FEATURES(${HAP_LIBNAME} PUBLIC cxx_std_20)

# Inter-library dependency: hap builds on common (exactly like hap).
TARGET_LINK_LIBRARIES(${HAP_LIBNAME} PUBLIC ${COMMON_LIBNAME})

# When CUDA RNG is enabled at build time, the producer can refill containers on
# the GPU via the cuRAND host API at run time (only when a device is actually
# present; otherwise it falls back to the CPU engine). This needs the
# HAP_USE_CUDA compile definition and links the cuRAND + CUDA-runtime host
# libraries. cuRAND is part of the (optional) NVIDIA CUDA Toolkit the user
# provides; it is never vendored into the source tree.
IF(GENEVA_USE_CUDA_RNG_ENABLED)
	FIND_PACKAGE(CUDAToolkit REQUIRED)
	TARGET_COMPILE_DEFINITIONS(${HAP_LIBNAME} PRIVATE HAP_USE_CUDA=1)
	TARGET_LINK_LIBRARIES(${HAP_LIBNAME} PRIVATE CUDA::curand CUDA::cudart)
ENDIF()

################################################################################
# SIMD bulk-refill backend selection. Compile-time only (no runtime
# dispatch). The probes decide which vectorised xoshiro256++ variant (if any)
# the producer uses to refill containers. PRIVATE: the flag/definition apply
# only to the gemfony-hap TUs, never to consumers (the public header's engine
# type stays scalar). On unsupported ISAs only the scalar path is built --
# this is by design, not an error.

# Architecture gate: Geneva/Hap targets ONLY x86-64 and AArch64, because its SIMD
# bulk-refill backend (the universal fallback when the GPU is unavailable) is written
# for AVX2 (x86-64) / NEON (AArch64). Every other ISA is excluded at configure time.
IF ( NOT CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|aarch64|arm64" )
	MESSAGE ( FATAL_ERROR
		"Geneva/Hap supports only x86-64 and AArch64 "
		"(CMAKE_SYSTEM_PROCESSOR='${CMAKE_SYSTEM_PROCESSOR}'). Its SIMD bulk-refill backend "
		"targets AVX2 (x86-64) / NEON (AArch64); other architectures are not supported." )
ENDIF ()

INCLUDE ( CheckCXXSourceCompiles )

SET ( CMAKE_REQUIRED_FLAGS "-mavx2" )
CHECK_CXX_SOURCE_COMPILES (
	"#include <immintrin.h>
	 int main() { __m256i x = _mm256_set1_epi64x(1); return (int)_mm256_extract_epi64(x, 0); }"
	HAP_HAS_AVX2 )
UNSET ( CMAKE_REQUIRED_FLAGS )

CHECK_CXX_SOURCE_COMPILES (
	"#include <arm_neon.h>
	 int main() { uint64x2_t x = vdupq_n_u64(1); return (int)vgetq_lane_u64(x, 0); }"
	HAP_HAS_NEON )

IF ( HAP_HAS_AVX2 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64" )
	TARGET_COMPILE_OPTIONS     ( ${HAP_LIBNAME} PRIVATE -mavx2 )
	TARGET_COMPILE_DEFINITIONS ( ${HAP_LIBNAME} PRIVATE HAP_AVX2_BACKEND=1 )
	MESSAGE ( STATUS "Hap: AVX2 SIMD bulk-refill backend enabled (x86_64)" )
ELSEIF ( HAP_HAS_NEON AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64" )
	TARGET_COMPILE_DEFINITIONS ( ${HAP_LIBNAME} PRIVATE HAP_NEON_BACKEND=1 )
	MESSAGE ( STATUS "Hap: NEON SIMD bulk-refill backend enabled (AArch64)" )
ELSE ()
	# The arch gate above guarantees x86-64 or AArch64; reaching here means the SIMD probe
	# failed (e.g. an x86-64 toolchain that cannot compile AVX2). NEON is baseline-mandatory
	# on AArch64, so this is effectively the pre-AVX2 x86-64 case: warn and fall back to the
	# scalar engine (decided policy -- do not hard-fail), so a non-SIMD build is never silent.
	MESSAGE ( WARNING
		"Hap: no SIMD bulk-refill backend available on this host (x86-64 without AVX2?) -- "
		"falling back to the slower scalar xoshiro256++ refill. AVX2 (Haswell, 2013+) is the "
		"x86-64 SIMD floor." )
ENDIF ()

################################################################################
# Install the library and register it in the GenevaTargets export set, so that
# the exported package config resolves gemfony-hap (courtier/geneva depend on it).
INSTALL ( TARGETS ${HAP_LIBNAME} EXPORT GenevaTargets DESTINATION ${INSTALL_PREFIX_LIBS} )
