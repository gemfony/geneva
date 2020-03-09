CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

ADD_CUSTOM_TARGET( "tests-courtier-performance"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS GBufferPortTTest GConsumerPerformance
	COMMENT "Building the performance tests for the Courtier library."
)

ADD_SUBDIRECTORY (GBufferPortTTest )
# ADD_SUBDIRECTORY (GConsumerPerformance)
