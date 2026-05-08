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

ENABLE_LANGUAGE(CUDA)

# CUDA standard — C++17 is required inside .cu files (lambdas in G_CALLABLE
# functions); the host code is compiled as C++20 by the normal CXX rules.
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# libstdc++ ABI workaround
#
# The CUDA toolkit ships its own bundled host compiler (often an older GCC).
# Its libstdc++.so may lack the CXXABI version required by Geneva's shared
# libraries (built with a newer GCC).  Detect the CXX compiler's own
# libstdc++.so and link it explicitly to keep the right ABI version first.
# ---------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER AND NOT DEFINED GENEVA_CXX_RUNTIME_LIB_DIR)
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libstdc++.so
        OUTPUT_VARIABLE _cxx_libstdcxx_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_cxx_libstdcxx_path AND NOT _cxx_libstdcxx_path STREQUAL "libstdc++.so")
        get_filename_component(_cxx_lib_dir "${_cxx_libstdcxx_path}" DIRECTORY)
        set(GENEVA_CXX_RUNTIME_LIB_DIR "${_cxx_lib_dir}"
            CACHE INTERNAL "CXX runtime lib dir for CUDA link fix")
    endif()
    unset(_cxx_libstdcxx_path)
    unset(_cxx_lib_dir)
endif()

# ---------------------------------------------------------------------------
# Target
# ---------------------------------------------------------------------------
SET(EXECUTABLENAME GCUDAOptBenchmarkTest)

SET(${EXECUTABLENAME}_SRCS
    GBenchmarkCUDATest.cpp
    GBenchmarkBatchEvaluator.cu
)

ADD_EXECUTABLE(${EXECUTABLENAME}
    ${${EXECUTABLENAME}_SRCS}
)

# RTX 4090 = compute capability 8.9; keep 80/86 for broader compatibility.
set_target_properties(${EXECUTABLENAME} PROPERTIES
    CUDA_ARCHITECTURES "80;86;89"
)

# --extended-lambda: required by levy() which captures nothing but uses a
# local lambda inside a __host__ __device__ function.
target_compile_options(${EXECUTABLENAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>
)

# Make the benchmark source directory visible so #include "GBenchmarkBatchEvaluator.cuh"
# resolves without a path prefix.
target_include_directories(${EXECUTABLENAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(LIBRARIES_TO_LINK
    ${GENEVA_LIBRARIES}
    ${Boost_LIBRARIES}
)

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
    SET(LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK} ${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES(${EXECUTABLENAME} PRIVATE
    ${LIBRARIES_TO_LINK}
)

# Link the CXX compiler's libstdc++.so by full path (see ABI note above).
if(DEFINED GENEVA_CXX_RUNTIME_LIB_DIR AND
   EXISTS "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so")
    target_link_libraries(${EXECUTABLENAME} PRIVATE
        "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so"
    )
endif()

# CLion workaround: expose headers as a custom target so the IDE indexes them.
SET(GBENCHMARK_CUDA_HEADERS
    GBenchmarkBatchEvaluator.cuh
    GBenchmarkCUDAConsumer.hpp
)
ADD_CUSTOM_TARGET(CLION_GBENCHMARKCUDA_HEADERS SOURCES ${GBENCHMARK_CUDA_HEADERS})

IF(GENEVA_RUN_BENCHMARKS_AS_TESTS)
    ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
ENDIF()

INSTALL(TARGETS ${EXECUTABLENAME}
    DESTINATION ${INSTALL_PREFIX_DATA}/benchmarks/${EXECUTABLENAME}
)
