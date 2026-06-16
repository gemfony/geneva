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
#
# GGlobalDefines.hpp is a regular, manually maintained header — it is no longer
# generated from a .in template.  When bumping the Geneva version or the minimum
# Boost version, update the GENEVA_VERSION and MIN_BOOST_VERSION macros in that
# file directly (see the comments there for the encoding rules).
# The authoritative version numbers are VERSION_MAJOR / VERSION_MINOR /
# VERSION_PATCH in the top-level CMakeLists.txt and GENEVA_MIN_BOOST_VERSION in
# CMakeModules/CommonGenevaBuild.cmake.
#

SET ( GENEVACOMMONINCLUDES
	GBlockingMPMCQueueT.hpp
	GCommonEnums.hpp
	GCommonHelperFunctions.hpp
	GCommonHelperFunctionsT.hpp
	GCommonInterfaceT.hpp
	GCommonMathHelperFunctionsT.hpp
	GContainerT.hpp
	GDefaultValueT.hpp
	GErrorStreamer.hpp
	GExceptions.hpp
	GExpectationChecksT.hpp
	GFactoryT.hpp
	GFixedSizePriorityQueueT.hpp
	GGlobalDefines.hpp
	GGlobalOptionsT.hpp
	GLogger.hpp
	GMPMCQueueT.hpp
	GMemberReflectionT.hpp
	GParserBuilder.hpp
	GPlotDesigner.hpp
	GPreallocatedMPMCQueueT.hpp
	GProviderT.hpp
	GQueueCommon.hpp
	GSerializableFunctionObjectT.hpp
	GSerializationHelperFunctionsT.hpp
	GSerializeTupleT.hpp
	GSingletonT.hpp
	GStdFilesystemPathSerialization.hpp
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
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)


INSTALL ( FILES ${GENEVACOMMONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/common )

# The plotting subsystem (formerly the monolithic GPlotDesigner.hpp) lives in focused headers under
# common/plotting/; common/GPlotDesigner.hpp remains as an umbrella that includes them.
SET ( GENEVACOMMONPLOTTINGINCLUDES
	plotting/GPlotEnums.hpp
	plotting/GDecorators.hpp
	plotting/GBasePlotter.hpp
	plotting/GPlots.hpp
	plotting/GPlotDesigner.hpp
)

INSTALL ( FILES ${GENEVACOMMONPLOTTINGINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/common/plotting )
