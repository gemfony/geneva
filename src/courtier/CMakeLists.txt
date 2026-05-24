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

SET(COMMUNICATIONSOURCES
        GCourtierHelperFunctions.cpp
        GCourtierEnums.cpp
        GDemoProcessingContainers.cpp
        )

# MPI-consumer is optional, only build it when requested
IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
    SET( COMMUNICATIONSOURCES
            ${COMMUNICATIONSOURCES}
            GMPIHelperFunctions.cpp)
ENDIF ()

set_source_files_properties(
        ${COMMUNICATIONSOURCES}
        PROPERTIES
        LANGUAGE CXX
        CXX_STANDARD 20
)

ADD_LIBRARY(${COURTIER_LIBNAME} SHARED ${COMMUNICATIONSOURCES})

SET_TARGET_PROPERTIES(
        ${COURTIER_LIBNAME}
        PROPERTIES
        VERSION ${GENEVA_VERSION}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        EXPORT_NAME courtier
)

# Export the include directory and require C++20 transitively.
TARGET_INCLUDE_DIRECTORIES(
        ${COURTIER_LIBNAME}
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
TARGET_COMPILE_FEATURES(${COURTIER_LIBNAME} PUBLIC cxx_std_20)

# Inter-library dependency: courtier builds on hap (and transitively common).
TARGET_LINK_LIBRARIES(${COURTIER_LIBNAME} PUBLIC ${HAP_LIBNAME})

# MPI is an implementation detail of the consumer sources; keep it PRIVATE so it
# does not leak into the exported interface of consumers that do not use MPI.
IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
    TARGET_LINK_LIBRARIES(${COURTIER_LIBNAME} PRIVATE MPI::MPI_CXX)
ENDIF ()

INSTALL(TARGETS ${COURTIER_LIBNAME} EXPORT GenevaTargets DESTINATION ${INSTALL_PREFIX_LIBS})
