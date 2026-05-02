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

IF(GENEVA_USE_CUDA_RNG)
	ENABLE_LANGUAGE(CUDA)

	# Setting the CUDA-Standard
	set(CMAKE_CUDA_STANDARD 17)
	set(CMAKE_CUDA_STANDARD_REQUIRED ON)
ENDIF()

SET ( HAPINCLUDES
	GHapEnums.hpp
	GRandomBase.hpp
    GRandomDistributionsT.hpp
	GRandomDefines.hpp
	GRandomFactory.hpp
	GRandomT.hpp
)

# If we compile with CUDA support,
# add the appropriate file
IF(GENEVA_USE_CUDA_RNG)
	SET(HAPINCLUDES
		${HAPINCLUDES}
		GCUDARng.hpp
	)
ENDIF ()

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_HAP_HEADERS SOURCES ${HAPINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_HAP_HEADERS
    PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${HAPINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/hap )
