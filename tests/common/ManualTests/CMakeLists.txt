CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "common-tests-manual" )

ADD_DEPENDENCIES ( "common-tests-manual"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GCanvasTest
	GFormulaParser
	GHelperFunctionsTest
	GLoggerTest
	GPlotDesignerTest
	GThreadPoolTest
)

ADD_SUBDIRECTORY ( GCanvasTest )
ADD_SUBDIRECTORY ( GHelperFunctionsTest )
ADD_SUBDIRECTORY ( GFormulaParser )
ADD_SUBDIRECTORY ( GLoggerTest )
ADD_SUBDIRECTORY ( GPlotDesignerTest )
ADD_SUBDIRECTORY ( GRegressionTest )
ADD_SUBDIRECTORY ( GThreadPoolTest )