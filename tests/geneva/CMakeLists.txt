CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "geneva-tests"
	COMMENT "Building only the Geneva tests."
)

ADD_DEPENDENCIES ( "geneva-tests"
	${GENEVA_LIBNAME}
	"geneva-tests-manual"
	"geneva-tests-performance"
	"geneva-tests-unit"
)

ADD_SUBDIRECTORY (
	TestIndividuals
)

ADD_SUBDIRECTORY (
	ManualTests
)

ADD_SUBDIRECTORY (
	PerformanceChecks
)

ADD_SUBDIRECTORY (
	UnitTests
)
