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
#
# Check the major/minor/patch strings configured in the top level CMakeLists file
#   and use them to define the GENEVA_VERSION variable in GGlobalDefines.hpp
#
SET ( VERSION_PREFIX "0" )

IF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong major version number! (${VERSION_MAJOR})" )
ENDIF ()

IF ( NOT VERSION_MINOR MATCHES "^(1?[0-9])$" )
	MESSAGE ( AUTHOR_WARNING "Wrong minor version number! (${VERSION_MINOR})" )
ENDIF ()

# Allow additional suffixes like "rc1" in the patch number
IF ( NOT VERSION_PATCH MATCHES "^[0-9]" )
	MESSAGE ( AUTHOR_WARNING "Wrong patch version number! (${VERSION_PATCH})" )
ENDIF ()

# Keep the first character only (a number)
STRING ( SUBSTRING ${VERSION_PATCH} 0 1 VERSION_PATCH_SHORT )

# Get the minimal Boost major and minor versions
STRING ( SUBSTRING ${GENEVA_MIN_BOOST_VERSION} 0 1 MIN_BOOST_VERSION_MAJOR )
STRING ( SUBSTRING ${GENEVA_MIN_BOOST_VERSION} 2 2 MIN_BOOST_VERSION_MINOR )

SET ( GENEVA_API_VERSION ${VERSION_PREFIX}${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH_SHORT} )
SET ( GENEVA_MIN_BOOST_API_VERSION ${MIN_BOOST_VERSION_MAJOR}0${MIN_BOOST_VERSION_MINOR}00)
CONFIGURE_FILE (
	GGlobalDefines.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/GGlobalDefines.hpp
	@ONLY
)

SET ( GENEVACOMMONINCLUDES
	GBarrier.hpp
	GBoundedBufferT.hpp
	GCanvas.hpp
	GCommonEnums.hpp
	GCommonHelperFunctions.hpp
	GCommonHelperFunctionsT.hpp
	GCommonInterfaceT.hpp
	GCommonMathHelperFunctions.hpp
	GCommonMathHelperFunctionsT.hpp
	GDefaultValueT.hpp
	GErrorStreamer.hpp
	GExceptions.hpp
	GExpectationChecksT.hpp
	GFactoryT.hpp
	GFixedSizePriorityQueueT.hpp
	GFormulaParserT.hpp
	GGlobalDefines.hpp
	GGlobalStoreT.hpp
	GLogger.hpp
	GParserBuilder.hpp
	GPODVectorT.hpp
	GPlotDesigner.hpp
	GPtrVectorT.hpp
	GSerializableFunctionObjectT.hpp
	GSerializationHelperFunctionsT.hpp
	GSerializeTupleT.hpp
	GSingletonT.hpp
	GThreadGroup.hpp
	GThreadPool.hpp
	GTupleIO.hpp
	GTypeToStringT.hpp
	GTypeTraitsT.hpp
	GUnitTestFrameworkT.hpp
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_COMMON_HEADERS SOURCES ${GENEVACOMMONINCLUDES})

SET_TARGET_PROPERTIES(
	CLION_ALL_COMMON_HEADERS
	PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)


INSTALL ( FILES ${GENEVACOMMONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/common )
