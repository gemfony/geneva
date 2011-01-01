CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "hap-tests"
	COMMENT "Building the Hap tests."
)

ADD_DEPENDENCIES ( "hap-tests"
	${HAP_LIBNAME}
	"hap-tests-unit"
	"hap-tests-manual"
)

ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( ManualTests )
ADD_SUBDIRECTORY ( DocuPrograms )
