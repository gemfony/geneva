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

SET ( COMMONSOURCES
    GCommonEnums.cpp
    GCommonHelperFunctions.cpp
    GExceptions.cpp
	GExpectationChecksT.cpp
	GFormulaParserT.cpp
	GLogger.cpp
	GParserBuilder.cpp
	GPlotDesigner.cpp
	GThreadGroup.cpp
	GThreadPool.cpp
)


set_source_files_properties(
    ${COMMONSOURCES}
    PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 20
)

ADD_LIBRARY ( ${COMMON_LIBNAME} SHARED ${COMMONSOURCES} )

SET_TARGET_PROPERTIES(
    ${COMMON_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
    MACOSX_RPATH ${INSTALL_PREFIX_LIBS}
)

# Enable API-exporting for this library
ADD_DEFINITIONS("-DGEM_COMMON_EXPORTS")

IF ( PLATFORM_NEEDS_LIBRARY_LINKING )
	TARGET_LINK_LIBRARIES(
		${COMMON_LIBNAME}
		${Boost_LIBRARIES}
	)
ENDIF ()

INSTALL ( TARGETS ${COMMON_LIBNAME} DESTINATION ${INSTALL_PREFIX_LIBS} )
