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

SET ( EXECUTABLENAME GGPUFunctionDemo )

ADD_EXECUTABLE ( ${EXECUTABLENAME} GGPUFunctionDemo.cpp )

SET_TARGET_PROPERTIES ( ${EXECUTABLENAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# For the demo-local marshaller header (GFunctionIndividualGPUMarshaller.hpp).
TARGET_INCLUDE_DIRECTORIES ( ${EXECUTABLENAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )

# gemfony-courtier-gpu brings in its public include dir, the courtier libraries and (when present) CUDA/OpenCL.
TARGET_LINK_LIBRARIES ( ${EXECUTABLENAME} PRIVATE
	${COURTIER_GPU_LIBNAME}
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
)

# Runtime assets: the kernels are loaded at run time (NVRTC / clBuildProgram), and the demo's
# consumer-integration part builds GFunctionIndividuals from a factory config. Stage both next to the
# binary so it can be run from its build directory.
ADD_CUSTOM_COMMAND ( TARGET ${EXECUTABLENAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/kernels ${CMAKE_CURRENT_BINARY_DIR}/kernels )
FILE ( COPY ${CMAKE_SOURCE_DIR}/geneva/examples/01_GSimpleOptimizer/config/GFunctionIndividual.json
       DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config )

INSTALL ( TARGETS ${EXECUTABLENAME}
	DESTINATION ${INSTALL_PREFIX_DATA}/experimental/gpu/${EXECUTABLENAME} )
