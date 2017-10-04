CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#
# Check the major/minor/patch strings configured in the top level CMakeLists file
#   and use them to define the GENEVA_VERSION variable in GGlobalDefines.hpp
#
SET ( VERSION_PREFIX "0" )

IF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong major version number! (${VERSION_MAJOR})" )
ENDIF ()

IF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )
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
	GCommonInterfaceT.hpp
    GCommonMathHelperFunctions.hpp
    GCommonMathHelperFunctionsT.hpp
	GDefaultValueT.hpp
	GExceptions.hpp
	GExpectationChecksT.hpp
	GFactoryT.hpp
	GFixedSizePriorityQueueT.hpp
	GFormulaParserT.hpp
	GGlobalDefines.hpp
	GGlobalOptionsT.hpp
        GCommonHelperFunctionsT.hpp
	GLockVarT.hpp
	GLogger.hpp
	GParserBuilder.hpp
	GPlotDesigner.hpp
    GSerializableFunctionObjectT.hpp
	GSerializationHelperFunctionsT.hpp
	GSerializeTupleT.hpp
	GSingletonT.hpp
    GStdThreadGroup.hpp
	GStdSimpleVectorInterfaceT.hpp
	GStdPtrVectorInterfaceT.hpp
	GThreadPool.hpp
    GThreadSafeQueueT.hpp
	GThreadWrapper.hpp
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
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED 14
)


INSTALL ( FILES ${GENEVACOMMONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/common )
