CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "common-tests-performance" )

ADD_DEPENDENCIES ( "common-tests-performance"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBoundedBufferPerformance
)

ADD_SUBDIRECTORY ( GBoundedBufferPerformance )
