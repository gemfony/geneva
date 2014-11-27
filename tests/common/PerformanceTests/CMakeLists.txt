CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-common-performance"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS GBoundedBufferPerformance
	COMMENT "Building the performance tests for the Common library."
)

ADD_SUBDIRECTORY ( GBoundedBufferPerformance )
