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


################################################################################
#
# This file is part of the Geneva library collection. Apache License 2.0.
# See the NOTICE file in the top-level directory for contributors/copyright.
#
################################################################################
#
# Hap2: public headers (identical class names in Gem::Hap2, include prefix
# <hap2/...>).
#
################################################################################

IF(GENEVA_USE_CUDA_RNG)
	ENABLE_LANGUAGE(CUDA)
	set(CMAKE_CUDA_STANDARD 20)
	set(CMAKE_CUDA_STANDARD_REQUIRED ON)
ENDIF()

SET ( HAP2INCLUDES
	GHapEnums.hpp
	GRandomBase.hpp
	GRandomDistributionsT.hpp
	GRandomDefines.hpp
	GRandomFactory.hpp
	GRandomT.hpp
)

IF(GENEVA_USE_CUDA_RNG)
	SET(HAP2INCLUDES
		${HAP2INCLUDES}
		GCUDARng.hpp
	)
ENDIF ()

# CLion-visibility helper (mirrors the Hap subtree).
ADD_CUSTOM_TARGET(CLION_ALL_HAP2_HEADERS SOURCES ${HAP2INCLUDES})

SET_TARGET_PROPERTIES(
	CLION_ALL_HAP2_HEADERS
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${HAP2INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/hap2 )
