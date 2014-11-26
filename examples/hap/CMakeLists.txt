CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "examples_hap"
	# Add all the available example's targets here
	DEPENDS 05_GHapUsagePatterns
	COMMENT "Building the Geneva examples for the Hap library."
)

ADD_SUBDIRECTORY( 05_GHapUsagePatterns )
