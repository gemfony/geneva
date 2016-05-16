CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-hap-manual"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS GRandomUsage
	COMMENT "Building the manual tests for the Hap library."
)

ADD_SUBDIRECTORY ( BoostThreadLocal )
ADD_SUBDIRECTORY ( CXX11ThreadLocal )
ADD_SUBDIRECTORY ( GRandomUsage )
