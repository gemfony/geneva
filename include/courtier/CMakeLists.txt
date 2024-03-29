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

SET ( COMMUNICATIONINCLUDES
    GAsioConsumerT.hpp
    GBaseClientT.hpp
    GBaseConsumerT.hpp
    GBrokerT.hpp
    GBufferPortT.hpp
    GCommandContainerT.hpp
    GCourtierEnums.hpp
    GCourtierHelperFunctions.hpp
    GExecutorT.hpp
    GProcessingContainerT.hpp
    GSerialConsumerT.hpp
    GStdThreadConsumerT.hpp
    GWebsocketConsumerT.hpp
    GWorkerT.hpp
)

# MPI-consumer is optional, only build it when requested
IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
    SET( COMMUNICATIONINCLUDES
            ${COMMUNICATIONINCLUDES}
            GMPIConsumerT.hpp
            GMPIHelperFunctions.hpp)
ENDIF ()

set_source_files_properties(
    ${COMMUNICATIONINCLUDES}
    PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_COURTIER_HEADERS SOURCES ${COMMUNICATIONINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_COURTIER_HEADERS
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/courtier )
