CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "tests"
	COMMENT "Building the tests."
)

ADD_DEPENDENCIES ( "tests"
	"dataexchange-tests"
	"hap-tests"
	"courtier-tests"
	"geneva-tests"
)

ADD_SUBDIRECTORY ( dataexchange )
ADD_SUBDIRECTORY ( hap )
ADD_SUBDIRECTORY ( courtier )
ADD_SUBDIRECTORY ( geneva )
