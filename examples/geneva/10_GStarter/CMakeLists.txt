CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(GStarter)

	# For building the examples independently, we still assume that the
	# 'CMakeModules' folder can be found at the same place as the current
	# source folder... This avoids having to copy the same content over
	# and over again in all the examples.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

ENDIF ()

SET ( EXECUTABLENAME GStarter )

SET ( ${EXECUTABLENAME}_SRCS
	GStarter
	GStarterIndividual
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
)

ADD_SUBDIRECTORY (config)

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY (Tests)
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION examples/${PROJECT_NAME} )
