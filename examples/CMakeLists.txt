CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "examples"
	DEPENDS "examples-common"
		"examples-hap"
		"examples-courtier"
		"examples-geneva"
	COMMENT "Building all the Geneva examples."
)

ADD_SUBDIRECTORY( common )
ADD_SUBDIRECTORY( hap )
ADD_SUBDIRECTORY( courtier )
ADD_SUBDIRECTORY( geneva )
