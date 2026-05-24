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

# Headers listed here for CLion indexing — this target carries no CUDA dependency
# and must remain before ENABLE_LANGUAGE(CUDA) so CLion sees it even when CUDA
# is not configured in the IDE toolchain (mirrors the CLION_ALL_COMMON_HEADERS
# pattern used by the Geneva libraries).
SET ( MONALISAINCLUDES
	GCanvas.hpp
    GImageIndividual.hpp
    GImagePOM.hpp
	GImageCUDAWorker.hpp
	GImageHelperFunctions.hpp
	GImageIndividualEvaluator.hpp
	GCUDAFitnessIndividual.hpp
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_MONALISA_HEADERS SOURCES ${MONALISAINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_MONALISA_HEADERS
    PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

ENABLE_LANGUAGE(CUDA)

# Make CUDA:: imported targets (e.g. CUDA::cudart) available.  This is needed
# so CXX source files that call the CUDA runtime API (e.g. cudaGetDeviceProperties,
# which resolves to cudaGetDeviceProperties_v2 with CUDA 12+ headers) resolve
# correctly at link time regardless of how many CUDA toolkit versions are installed.
find_package(CUDAToolkit REQUIRED)

# The CUDA toolkit's bundled g++ wrapper selects an older GCC whose library
# directory is injected into the link command before the system CXX compiler's
# directory. When Geneva's shared libraries are built with a newer GCC they
# require a newer CXXABI that the older GCC's libstdc++.so does not provide.
# Detect the CXX compiler's own libstdc++.so directory and store it so the
# GCUDAWorker target can prepend it (see target_link_directories below).
if(CMAKE_CXX_COMPILER AND NOT DEFINED GENEVA_CXX_RUNTIME_LIB_DIR)
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libstdc++.so
        OUTPUT_VARIABLE _cxx_libstdcxx_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_cxx_libstdcxx_path AND NOT _cxx_libstdcxx_path STREQUAL "libstdc++.so")
        get_filename_component(_cxx_lib_dir "${_cxx_libstdcxx_path}" DIRECTORY)
        set(GENEVA_CXX_RUNTIME_LIB_DIR "${_cxx_lib_dir}" CACHE INTERNAL "CXX runtime lib dir for CUDA fix")
    endif()
    unset(_cxx_libstdcxx_path)
    unset(_cxx_lib_dir)
endif()

# Setting the CUDA-Standard
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Search for libpng
FIND_PACKAGE(PNG REQUIRED)
IF(NOT PNG_FOUND)
	message(FATAL_ERROR "libpng could not be found")
ENDIF()

SET ( EXECUTABLENAME GCUDAWorker )
SET ( ${EXECUTABLENAME}_SRCS
	GCanvas.cpp
	GImageBuilder.cpp
	GImageCUDAWorker.cpp
    GImageIndividual.cpp
	GImageHelperFunctions.cpp
	GImageIndividualEvaluator.cu
	GCUDAFitnessIndividual.cu
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

set_target_properties(${EXECUTABLENAME} PROPERTIES
		CUDA_ARCHITECTURES 80 86 89
)

target_compile_options(${EXECUTABLENAME} PRIVATE
		$<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>
)

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES}
		${OpenCL_LIBRARIES}
		CUDA::cudart)

# Add MPI Libraries in case building with MPI-consumer
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK
			${LIBRARIES_TO_LINK}
			${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME} PRIVATE
		${LIBRARIES_TO_LINK}
		PNG::PNG
)

# The CUDA toolkit's bundled host compiler (GCC 13) causes its older
# libstdc++.so (missing CXXABI_1.3.15) to be found first during linking.
# CMake filters -L flags from target_link_directories (implicit path
# deduplication) and target_link_options (-L suppressed). Linking the
# CXX compiler's libstdc++.so by its full path bypasses all filtering
# and gives the linker a file that satisfies the required CXXABI version.
if(DEFINED GENEVA_CXX_RUNTIME_LIB_DIR AND
   EXISTS "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so")
    target_link_libraries(${EXECUTABLENAME} PRIVATE
        "${GENEVA_CXX_RUNTIME_LIB_DIR}/libstdc++.so"
    )
endif()

ADD_SUBDIRECTORY(pictures)
ADD_SUBDIRECTORY(config)

IF (GENEVA_BUILD_TESTS)
	ADD_SUBDIRECTORY(tests)
ENDIF ()

IF (GENEVA_RUN_EXAMPLES_AS_TESTS)
	ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
ENDIF ()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME} )
