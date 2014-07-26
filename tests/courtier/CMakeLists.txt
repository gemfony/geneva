CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "courtier-tests"
	COMMENT "Building the Courtier tests."
)

ADD_DEPENDENCIES ( "courtier-tests"
	${COURTIER_LIBNAME}
	#"courtier-tests-unit"
	"courtier-tests-performance"
)

#ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( PerformanceTests )
