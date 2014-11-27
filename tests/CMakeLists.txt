CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests"
	DEPENDS "tests-common"
		"tests-hap"
		"tests-courtier"
		"tests-geneva"
	COMMENT "Building all the Geneva tests."
)

ADD_SUBDIRECTORY ( hap )
ADD_SUBDIRECTORY ( common )
ADD_SUBDIRECTORY ( courtier )
ADD_SUBDIRECTORY ( geneva )
