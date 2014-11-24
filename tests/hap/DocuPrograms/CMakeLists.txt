CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "hap-docu-progs" )

ADD_DEPENDENCIES ( "hap-docu-progs"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	DependentDistributionTests
	PlotRNGDistributions
)

ADD_SUBDIRECTORY ( DependentDistributionTests )
ADD_SUBDIRECTORY ( PlotRNGDistributions )
