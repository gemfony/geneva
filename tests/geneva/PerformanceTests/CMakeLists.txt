CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "geneva-tests-performance" )

ADD_DEPENDENCIES ( "geneva-tests-performance"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBrokerOverhead
	GOptimizationBenchmark
	GParallelisationOverhead
	GSerializationOverhead
)

ADD_SUBDIRECTORY ( GBrokerOverhead )
ADD_SUBDIRECTORY ( GOptimizationBenchmark )
ADD_SUBDIRECTORY ( GParallelisationOverhead )
ADD_SUBDIRECTORY ( GSerializationOverhead )
