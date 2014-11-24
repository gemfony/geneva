CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET ( "geneva-tests-manual" )

ADD_DEPENDENCIES ( "geneva-tests-manual"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBooleanProbabilityTest
	GConstrainedFPTTest
	GConstrainedIntTTest
	GDoubleBiGaussAdaptorTest
	GDoubleGaussAdaptorTest
	GParameterPropertyParserTest
	GRandomWalk
	GSigmaSigmaAdaptionTest
)

ADD_SUBDIRECTORY ( GBooleanProbabilityTest )
ADD_SUBDIRECTORY ( GConstrainedFPTTest )
ADD_SUBDIRECTORY ( GConstrainedIntegerTTest )
ADD_SUBDIRECTORY ( GDoubleBiGaussAdaptorTest )
ADD_SUBDIRECTORY ( GDoubleGaussAdaptorTest )
ADD_SUBDIRECTORY ( GParameterPropertyParserTest )
ADD_SUBDIRECTORY ( GRandomWalk )
ADD_SUBDIRECTORY ( GSigmaSigmaAdaptionTest )
