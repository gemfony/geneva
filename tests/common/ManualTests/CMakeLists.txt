CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "common-tests-manual" )

ADD_DEPENDENCIES ( "common-tests-manual"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GROOTProducerTest
)

ADD_SUBDIRECTORY ( GROOTProducerTest )
