CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "common-tests"
	COMMENT "Building the tests for the Common library"
)

ADD_DEPENDENCIES ( "common-tests"
	${COMMON_LIBNAME}
	#"courtier-tests-unit"
	"common-tests-manual"
	"common-tests-performance"
)

#ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( ManualTests )
ADD_SUBDIRECTORY ( PerformanceTests )
