CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-courtier"
	DEPENDS #"tests-courtier-unit"
		"tests-courtier-performance"
	COMMENT "Building all the tests for the Courtier library."
)

#ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( PerformanceTests )
