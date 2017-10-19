CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "tests-geneva-manual"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS
        GBooleanProbabilityTest
		GConstrainedFPTTest
		GConstrainedIntegerTTest
		GDoubleBiGaussAdaptorTest
		GDoubleGaussAdaptorTest
		GParameterPropertyParserTest
		GRandomWalk
        GResetToOptimizationStartTest
		GSigmaSigmaAdaptionTest
	COMMENT "Building the manual tests for the Optimization library."
)

ADD_SUBDIRECTORY ( GBooleanProbabilityTest )
ADD_SUBDIRECTORY ( GConstrainedFPTTest )
ADD_SUBDIRECTORY ( GConstrainedIntegerTTest )
ADD_SUBDIRECTORY ( GDoubleBiGaussAdaptorTest )
ADD_SUBDIRECTORY ( GDoubleGaussAdaptorTest )
ADD_SUBDIRECTORY ( GParameterPropertyParserTest )
ADD_SUBDIRECTORY ( GRandomWalk )
ADD_SUBDIRECTORY ( GResetToOptimizationStartTest )
ADD_SUBDIRECTORY ( GSigmaSigmaAdaptionTest )
