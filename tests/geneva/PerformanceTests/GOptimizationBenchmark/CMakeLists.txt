CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(GOptimizationBenchmark)

	# For building the tests independently, we still assume that the
	# current source folder is still part of the Geneva tree structure,
	# i.e., the 'CMakeModules' folder can be found going up... This avoids
	# having to copy the same content over and over again in the tests.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../../../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

ENDIF ()

SET ( EXECUTABLENAME GOptimizationBenchmark )

SET ( ${EXECUTABLENAME}_SRCS
	GOptimizationBenchmark
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
)

ADD_SUBDIRECTORY (config)

ADD_TEST(${EXECUTABLENAME} ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLENAME})

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION examples/${PROJECT_NAME} )
