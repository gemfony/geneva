CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "geneva-tests"
	COMMENT "Building the Geneva tests."
)

ADD_DEPENDENCIES ( "geneva-tests"
	${GENEVA_LIBNAME}
	"geneva-tests-unit"
	"geneva-tests-performance"
	"geneva-tests-manual"
)

ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( PerformanceTests )
ADD_SUBDIRECTORY ( ManualTests )
