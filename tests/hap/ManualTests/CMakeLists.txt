CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "hap-tests-manual" )

ADD_DEPENDENCIES ( "hap-tests-manual"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GRandomUsage
)

ADD_SUBDIRECTORY ( GRandomUsage )
