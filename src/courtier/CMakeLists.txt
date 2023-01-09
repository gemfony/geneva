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
# Geneva was started by Dr. Rüdiger Berlich and was later maintained together
# with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
# information on Gemfony scientific, see http://www.gemfomy.eu .
#
# The majority of files in Geneva was released under the Apache license v2.0
# in February 2020.
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
        CXX_STANDARD 17
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF (GENEVA_STATIC)
    ADD_LIBRARY(${COURTIER_LIBNAME} STATIC ${COMMUNICATIONSOURCES})
ELSE ()
    ADD_LIBRARY(${COURTIER_LIBNAME} SHARED ${COMMUNICATIONSOURCES})
ENDIF ()

SET_TARGET_PROPERTIES(
        ${COURTIER_LIBNAME}
        PROPERTIES
        VERSION ${GENEVA_VERSION}
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        MACOSX_RPATH ${INSTALL_PREFIX_LIBS}
)

# Enable API-exporting for this library
ADD_DEFINITIONS("-DGEM_COURTIER_EXPORTS")

# define libraries that need to be linked
SET(COURTIER_LIBRARIES_TO_LINK
        ${HAP_LIBNAME}
        ${COMMON_LIBNAME}
        ${Boost_LIBRARIES})

# if mpi consumer shall be build, also add mpi libraries
IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
    SET(COURTIER_LIBRARIES_TO_LINK
            ${COURTIER_LIBRARIES_TO_LINK}
            ${MPI_LIBRARIES})
ENDIF ()

IF (PLATFORM_NEEDS_LIBRARY_LINKING)
    TARGET_LINK_LIBRARIES(
            ${COURTIER_LIBNAME}
            ${COURTIER_LIBRARIES_TO_LINK}
    )
ENDIF ()

INSTALL(TARGETS ${COURTIER_LIBNAME} DESTINATION ${INSTALL_PREFIX_LIBS})
