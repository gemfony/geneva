CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-common"
	DEPENDS #"tests-common-unit"
		"tests-common-manual"
		"tests-common-performance"
	COMMENT "Building all the tests for the Common library."
)

#ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( ManualTests )
ADD_SUBDIRECTORY ( PerformanceTests )
