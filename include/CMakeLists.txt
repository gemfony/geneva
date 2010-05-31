CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#
# Check the major/minor/patch strings configure in the top level CMakeLists file
#   and use them to define the GENEVAVERSION variable in GGlobalDefines.hpp
#
SET ( VERSION_PREFIX "0" )

IF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong major version number! (${VERSION_MAJOR})" )
ENDIF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )

IF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong minor version number! (${VERSION_MINOR})" )
ENDIF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )

# Allow additional sufixes like "rc1" in the patch number
IF ( NOT VERSION_PATCH MATCHES "^[0-9]" )
	MESSAGE ( AUTHOR_WARNING "Wrong patch version number! (${VERSION_PATCH})" )
ENDIF ( NOT VERSION_PATCH MATCHES "^[0-9]" )

# Keep the first character only (a number)
STRING ( SUBSTRING ${VERSION_PATCH} 0 1 VERSION_PATCH_SHORT )

SET ( GENEVA_API_VERSION ${VERSION_PREFIX}${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH_SHORT} )
CONFIGURE_FILE (
	GGlobalDefines.hpp.in ${PROJECT_SOURCE_DIR}/include/GGlobalDefines.hpp
	@ONLY
)


ADD_SUBDIRECTORY ( thirdparty )

SET ( GENEVASHAREDINCLUDES
	GBoundedBufferT.hpp
	GBoundedBufferWithIdT.hpp
	GBufferPortT.hpp
	GEnums.hpp
	GExceptions.hpp
	GGlobalDefines.hpp
	GGlobalOptionsT.hpp
	GHelperFunctionsT.hpp
	GObject.hpp
	GPODExpectationChecksT.hpp
	GSerializationHelperFunctions.hpp
	GSingletonT.hpp
	GStdPtrVectorInterfaceT.hpp
	GStdSimpleVectorInterfaceT.hpp
	GThreadGroup.hpp
	GUnitTestFrameworkT.hpp
)

ADD_SUBDIRECTORY ( dataexchange )
ADD_SUBDIRECTORY ( random )
ADD_SUBDIRECTORY ( communication )
ADD_SUBDIRECTORY ( optimization )

INSTALL ( FILES ${GENEVASHAREDINCLUDES} DESTINATION include
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
