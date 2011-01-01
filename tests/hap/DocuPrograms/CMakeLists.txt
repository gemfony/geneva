CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "hap-docu-progs" )

ADD_DEPENDENCIES ( "hap-docu-progs"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GaussianRandomNumbers
)

ADD_SUBDIRECTORY ( PlotRNGDistributions )
