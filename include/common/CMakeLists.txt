CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#
# Check the major/minor/patch strings configured in the top level CMakeLists file
#   and use them to define the GENEVA_VERSION variable in GGlobalDefines.hpp
#
SET ( VERSION_PREFIX "0" )

IF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong major version number! (${VERSION_MAJOR})" )
ENDIF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )

IF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong minor version number! (${VERSION_MINOR})" )
ENDIF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )

# Allow additional suffixes like "rc1" in the patch number
IF ( NOT VERSION_PATCH MATCHES "^[0-9]" )
	MESSAGE ( AUTHOR_WARNING "Wrong patch version number! (${VERSION_PATCH})" )
ENDIF ( NOT VERSION_PATCH MATCHES "^[0-9]" )

# Keep the first character only (a number)
STRING ( SUBSTRING ${VERSION_PATCH} 0 1 VERSION_PATCH_SHORT )

SET ( GENEVA_API_VERSION ${VERSION_PREFIX}${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH_SHORT} )
CONFIGURE_FILE (
	GGlobalDefines.hpp.in ${PROJECT_SOURCE_DIR}/include/common/GGlobalDefines.hpp
	@ONLY
)

SET ( GENEVACOMMONINCLUDES
	GBoundedBufferT.hpp
	GBoundedBufferWithIdT.hpp
	GCanvas.hpp
	GCommonEnums.hpp
	GDefaultValueT.hpp
	GExceptions.hpp
	GFactoryT.hpp
	GFormulaParser.hpp
	GGlobalDefines.hpp
	GGlobalOptionsT.hpp
	GHelperFunctions.hpp
	GHelperFunctionsT.hpp
	GLogger.hpp
	GMathHelperFunctions.hpp
	GMathHelperFunctionsT.hpp
	GNumericParameterT.hpp
	GParserBuilder.hpp
	GPlotDesigner.hpp
	GPODExpectationChecksT.hpp
	GSerializableI.hpp
	GSerializationHelperFunctionsT.hpp
	GSerializeTupleT.hpp
	GSingletonT.hpp
	GThreadGroup.hpp
	GThreadPool.hpp
	GThreadWrapper.hpp
	GTriboolSerialization.hpp
	GUnitTestFrameworkT.hpp
)

INSTALL ( FILES ${GENEVACOMMONINCLUDES} DESTINATION include/common
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
