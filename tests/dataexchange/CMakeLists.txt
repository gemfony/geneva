CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "dataexchange-tests"
	COMMENT "Building the Dataexchange tests."
)

ADD_DEPENDENCIES ( "dataexchange-tests"
	${DATAEXCHANGE_LIBNAME}
	"dataexchange-tests-unit"
)

ADD_SUBDIRECTORY ( UnitTests )
