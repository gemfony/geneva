CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-geneva"
	DEPENDS "tests-geneva-unit"
		"tests-geneva-manual"
		"tests-geneva-performance"
	COMMENT "Building all the tests for the Optimization library."
)

ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( PerformanceTests )
ADD_SUBDIRECTORY ( ManualTests )
