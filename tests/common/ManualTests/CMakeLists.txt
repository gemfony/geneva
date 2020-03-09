CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

ADD_CUSTOM_TARGET( "tests-common-manual"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS
        GCanvasTest
        GFormulaParserTest
        GHelperFunctionsTest
        GLoggerTest
        GPlotDesignerTest
        GThreadPoolTest
        GThreadSafeQueueTest
	COMMENT "Building the manual tests for the Common library."
)

ADD_SUBDIRECTORY ( GCanvasTest )
ADD_SUBDIRECTORY ( GHelperFunctionsTest )
ADD_SUBDIRECTORY ( GFormulaParserTest )
ADD_SUBDIRECTORY ( GLoggerTest )
ADD_SUBDIRECTORY ( GPlotDesignerTest )
ADD_SUBDIRECTORY ( GThreadPoolTest )
