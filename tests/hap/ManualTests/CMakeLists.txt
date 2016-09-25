CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-hap-manual"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS GRandomThroughput GRandomUsage
	COMMENT "Building the manual tests for the Hap library."
)

ADD_SUBDIRECTORY ( GRandomThroughput )
ADD_SUBDIRECTORY ( GRandomUsage )
