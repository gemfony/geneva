CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-geneva-performance"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS
        # GBrokerOverhead
        GBrokerSanityChecks
		GOptimizationBenchmark
		# GParallelisationOverhead
		GSerializationOverhead
	COMMENT "Building the performance tests for the Optimization library."
)

# ADD_SUBDIRECTORY ( GBrokerOverhead )
ADD_SUBDIRECTORY ( GBrokerSanityChecks )
ADD_SUBDIRECTORY ( GOptimizationBenchmark )
# ADD_SUBDIRECTORY ( GParallelisationOverhead )
ADD_SUBDIRECTORY ( GSerializationOverhead )
