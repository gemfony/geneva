CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-hap-docuprogs"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS DependentDistributionTests
		PlotRNGDistributions
	COMMENT "Building the documentation programs and tests for the Hap library."
)

ADD_CUSTOM_TARGET ( "hap-docu-progs" )

ADD_SUBDIRECTORY ( DependentDistributionTests )
ADD_SUBDIRECTORY ( PlotRNGDistributions )
