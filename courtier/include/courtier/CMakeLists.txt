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

SET ( COMMUNICATIONINCLUDES
    GBaseClientT.hpp
    GCommandContainerT.hpp
    GCourtierEnums.hpp
    GCourtierHelperFunctions.hpp
    GDemoProcessingContainers.hpp
    GExecutorStatusT.hpp
    GProcessingContainerT.hpp
)

# Transport classes (namespace Gem::Courtier::Consumers): the asio/websocket sessions + clients and
# the MPI master/worker nodes that the courtier2 networked consumers reuse. Kept here after the legacy
# submission stack was deleted; named under transport/ so they no longer collide with the (courtier2)
# consumer headers of the same mnemonic.
SET ( TRANSPORT_INCLUDES
    transport/GAsioTransportT.hpp
    transport/GWebsocketTransportT.hpp
)

# MPI-transport is optional, only build it when requested
IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
    SET( COMMUNICATIONINCLUDES
            ${COMMUNICATIONINCLUDES}
            GMPIHelperFunctions.hpp)
    SET( TRANSPORT_INCLUDES
            ${TRANSPORT_INCLUDES}
            transport/GMPITransportT.hpp)
ENDIF ()

set_source_files_properties(
    ${COMMUNICATIONINCLUDES} ${TRANSPORT_INCLUDES}
    PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 20
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_COURTIER_HEADERS SOURCES ${COMMUNICATIONINCLUDES} ${TRANSPORT_INCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_COURTIER_HEADERS
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/courtier )
INSTALL ( FILES ${TRANSPORT_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/courtier/transport )
