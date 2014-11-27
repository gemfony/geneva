CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-hap"
	DEPENDS "tests-hap-unit"
		"tests-hap-manual"
	COMMENT "Building all the tests for the Hap library."
)

ADD_SUBDIRECTORY ( UnitTests )
ADD_SUBDIRECTORY ( ManualTests )
ADD_SUBDIRECTORY ( DocuPrograms )
