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

SET ( HAPSOURCES
	GHapEnums.cpp
	GRandomBase.cpp
	GRandomFactory.cpp
	GRandomT.cpp
)

SET_SOURCE_FILES_PROPERTIES(
		${HAPSOURCES}
		PROPERTIES
		LANGUAGE CXX
		CXX_STANDARD 20
)

# If we compile with CUDA support,
# add the appropriate file
IF(GENEVA_USE_CUDA_RNG)
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
ELSE()

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
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
TARGET_COMPILE_FEATURES(${HAP_LIBNAME} PUBLIC cxx_std_20)

# Inter-library dependency: hap builds on common. PUBLIC so that include dirs,
# Boost and the C++20 requirement carried by common propagate to dependents.
TARGET_LINK_LIBRARIES(${HAP_LIBNAME} PUBLIC ${COMMON_LIBNAME})

INSTALL ( TARGETS ${HAP_LIBNAME} EXPORT GenevaTargets DESTINATION ${INSTALL_PREFIX_LIBS} )
