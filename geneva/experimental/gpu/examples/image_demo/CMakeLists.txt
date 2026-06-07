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

SET ( EXECUTABLENAME GGPUImageDemo )

ADD_EXECUTABLE ( ${EXECUTABLENAME}
	GGPUImageDemo.cpp
	GExpImageIndividual.cpp
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
)

# Kernels are loaded at run time (NVRTC / clBuildProgram); stage them next to the binary.
ADD_CUSTOM_COMMAND ( TARGET ${EXECUTABLENAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/kernels ${CMAKE_CURRENT_BINARY_DIR}/kernels )

INSTALL ( TARGETS ${EXECUTABLENAME}
	DESTINATION ${INSTALL_PREFIX_DATA}/experimental/gpu/${EXECUTABLENAME} )
