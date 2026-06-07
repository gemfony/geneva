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

# Experimental Mona-Lisa (example-15) port. Needs libpng to load the target image; if libpng is not
# present the demo is skipped (no compilation error), like the optional OpenCL backend.
FIND_PACKAGE ( PNG QUIET )
IF ( NOT PNG_FOUND )
	MESSAGE ( STATUS "GGPUMonaLisaDemo: libpng not found -- skipping the Mona-Lisa demo" )
	RETURN ()
ENDIF ()

SET ( EXECUTABLENAME GGPUMonaLisaDemo )

ADD_EXECUTABLE ( ${EXECUTABLENAME}
	GGPUMonaLisaDemo.cpp
	GImageIndividual.cpp
	GImageHelperFunctions.cpp
)

SET_TARGET_PROPERTIES ( ${EXECUTABLENAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

TARGET_INCLUDE_DIRECTORIES ( ${EXECUTABLENAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )

TARGET_LINK_LIBRARIES ( ${EXECUTABLENAME} PRIVATE
	gpugen
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
	PNG::PNG
)

# Runtime assets: the CUDA kernel (loaded via NVRTC at run time), the target image(s), and the factory
# config for building GImageIndividuals. Stage them next to the binary.
FILE ( COPY ${CMAKE_CURRENT_SOURCE_DIR}/kernels  DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
FILE ( COPY ${CMAKE_CURRENT_SOURCE_DIR}/pictures DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
FILE ( COPY ${CMAKE_CURRENT_SOURCE_DIR}/config   DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

INSTALL ( TARGETS ${EXECUTABLENAME}
	DESTINATION ${INSTALL_PREFIX_DATA}/experimental/gpu/${EXECUTABLENAME} )
