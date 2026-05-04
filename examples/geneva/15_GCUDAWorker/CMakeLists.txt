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

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(GCUDAWorker LANGUAGES CXX CUDA)

	# For building the examples independently, we still assume that the
	# 'CMakeModules' folder can be found at the same place as the current
	# source folder... This avoids having to copy the same content over
	# and over again in all the examples.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)
ENDIF ()

ENABLE_LANGUAGE(CUDA)

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
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Search for libpng
FIND_PACKAGE(PNG REQUIRED)
IF(NOT PNG_FOUND)
	message(FATAL_ERROR "libpng could not be found")
ENDIF()

SET ( EXECUTABLENAME GCUDAWorker )
SET ( ${EXECUTABLENAME}_SRCS
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
		${OpenCL_LIBRARIES})

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

SET ( MONALISAINCLUDES
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

ADD_SUBDIRECTORY(pictures)
ADD_SUBDIRECTORY(config)

IF (GENEVA_RUN_EXAMPLES_AS_TESTS)
	ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
ENDIF ()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME} )
