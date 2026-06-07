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

# Example 15 -- the Mona-Lisa problem, evaluated through the unified courtier GPU consumer
# (gemfony-courtier-gpu). Unlike the old example 15 it does NOT enable the CUDA language: the kernel
# is runtime-compiled by NVRTC at run time, so the example builds with an ordinary C++ toolchain and
# selects CPU / CUDA / OpenCL at run time via config/GGPUConsumer.json. It only needs libpng to load
# the target image and write result pictures; if libpng is missing the example is skipped.

FIND_PACKAGE ( PNG QUIET )
IF ( NOT PNG_FOUND )
	MESSAGE ( STATUS "Example 15 (GCUDAWorker): libpng not found -- skipping" )
	RETURN ()
ENDIF ()

# Headers, listed for CLion indexing.
SET ( MONALISAINCLUDES
	GImageIndividual.hpp
	GImageHelperFunctions.hpp
	GImagePOM.hpp
	GMonaLisaProblem.hpp
	GMonaLisaGPUMarshaller.hpp
)
ADD_CUSTOM_TARGET ( CLION_EXAMPLE15_HEADERS SOURCES ${MONALISAINCLUDES} )
SET_TARGET_PROPERTIES ( CLION_EXAMPLE15_HEADERS PROPERTIES
	CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF )

SET ( EXECUTABLENAME GCUDAWorker )

ADD_EXECUTABLE ( ${EXECUTABLENAME}
	GImageBuilder.cpp
	GImageIndividual.cpp
	GImageHelperFunctions.cpp
)

SET_TARGET_PROPERTIES ( ${EXECUTABLENAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

TARGET_INCLUDE_DIRECTORIES ( ${EXECUTABLENAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )

# The GPU consumer add-on (carries the CUDA/OpenCL backends + NVRTC), the Geneva libraries, Boost and
# libpng. gemfony-courtier-gpu is an OPTIONAL add-on built next to courtier (see courtier/src/gpu/).
TARGET_LINK_LIBRARIES ( ${EXECUTABLENAME} PRIVATE
	${COURTIER_GPU_LIBNAME}
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
	PNG::PNG
)

# Runtime assets. The kernel is runtime-loaded source, so copy it at BUILD time (a POST_BUILD command)
# -- editing the kernel + rebuilding then re-stages it, instead of the stale copy a configure-time
# FILE(COPY) would leave. The target images and config rarely change, so a configure-time copy is fine.
ADD_CUSTOM_COMMAND ( TARGET ${EXECUTABLENAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/kernels ${CMAKE_CURRENT_BINARY_DIR}/kernels )
FILE ( COPY ${CMAKE_CURRENT_SOURCE_DIR}/config   DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
FILE ( COPY ${CMAKE_CURRENT_SOURCE_DIR}/pictures DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

INSTALL ( TARGETS ${EXECUTABLENAME}
	DESTINATION ${INSTALL_PREFIX_DATA}/examples/15_GCUDAWorker )
INSTALL ( DIRECTORY config kernels pictures
	DESTINATION ${INSTALL_PREFIX_DATA}/examples/15_GCUDAWorker )
