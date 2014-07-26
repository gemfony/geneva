CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( HAPSOURCES
	GHapEnums
	GRandomBase
	GRandomFactory
	GSeedManager
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF ( GENEVA_STATIC )
	ADD_LIBRARY ( ${HAP_LIBNAME} STATIC ${HAPSOURCES} )
ELSE ( GENEVA_STATIC )
	ADD_LIBRARY ( ${HAP_LIBNAME} SHARED ${HAPSOURCES} )
ENDIF ( GENEVA_STATIC )

SET_TARGET_PROPERTIES( ${HAP_LIBNAME}
	PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
)

################################################################################
# Add additional libraries if this is MacOS
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	TARGET_LINK_LIBRARIES(
		${HAP_LIBNAME}
		${COMMON_LIBNAME}
	)
ENDIF()

################################################################################

INSTALL ( TARGETS ${HAP_LIBNAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
