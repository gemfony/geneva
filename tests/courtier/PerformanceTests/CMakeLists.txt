CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "courtier-tests-performance" )

ADD_DEPENDENCIES ( "courtier-tests-performance"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBrokerSelfCommunication
	GBufferPortTTest
)

ADD_SUBDIRECTORY ( GBrokerSelfCommunication )
ADD_SUBDIRECTORY ( GBufferPortTTest )
