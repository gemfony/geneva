CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "geneva-tests-performance" )

ADD_DEPENDENCIES ( "geneva-tests-performance"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBrokerOverhead
	GParallelisationOverhead
)

ADD_SUBDIRECTORY ( GBrokerOverhead )
ADD_SUBDIRECTORY ( GParallelisationOverhead )
